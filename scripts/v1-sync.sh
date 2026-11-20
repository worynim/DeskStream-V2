#!/usr/bin/env bash
# DeskStream(V1)의 변경 중 V2에 아직 반영하지 않은 것을 찾고, 폴더 단위로 V2에 적용한다.
#
# 기준 커밋·제외 목록은 V1_SYNC.md 의 ```sync 블록에서 읽는다.
# V1 저장소 위치는 기본 ../DeskStream, 다르면 V1_DIR 환경변수로 준다.
#
# 사용:
#   bash scripts/v1-sync.sh [status]        기준 커밋 이후 V1 커밋과 바뀐 폴더를 분류해 보여준다 (읽기 전용)
#   bash scripts/v1-sync.sh diff <폴더>     그 폴더의 V1 변경을 V2 경로로 바꾼 패치로 출력한다 (읽기 전용)
#   bash scripts/v1-sync.sh apply <폴더>    그 패치를 V2에 3-way로 적용한다 — 결과는 인덱스에 올라가고,
#                                           V2에서 고친 부분과 겹치면 충돌 표시(<<<<<<<)가 남는다
#   bash scripts/v1-sync.sh set-base [커밋]  V1_SYNC.md 의 기준 커밋을 바꾼다 (기본: V1 HEAD)
set -euo pipefail

REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SYNC_MD="$REPO/V1_SYNC.md"
V1="${V1_DIR:-$REPO/../DeskStream}"

die() { echo "❌ $*" >&2; exit 1; }

[ -f "$SYNC_MD" ] || die "V1_SYNC.md 가 없습니다: $SYNC_MD"
git -C "$V1" rev-parse --git-dir >/dev/null 2>&1 || die "V1 저장소를 찾을 수 없습니다: $V1 (V1_DIR 로 지정)"
V1="$(cd "$V1" && pwd)"

# ```sync 블록에서 "키: 값" 한 줄을 읽는다
sync_value() {
  awk -v key="$1" '
    /^```sync/ { inblk = 1; next }
    /^```/     { inblk = 0 }
    inblk && $1 == key ":" { $1 = ""; sub(/^ +/, ""); print; exit }
  ' "$SYNC_MD"
}

BASE="$(sync_value base)"
EXCLUDE=" $(sync_value exclude) "
HOLD=" $(sync_value hold) "
[ -n "$BASE" ] || die "V1_SYNC.md sync 블록에 base 가 없습니다"
git -C "$V1" rev-parse --verify -q "$BASE^{commit}" >/dev/null || die "V1에 기준 커밋 $BASE 가 없습니다"

v1git() { git -C "$V1" -c core.quotepath=false "$@"; }

# V1 최상위 이름 → 분류
classify() {
  local top="$1"
  case "$EXCLUDE" in *" $top "*) echo "exclude"; return ;; esac
  case "$HOLD" in *" $top "*) echo "hold"; return ;; esac
  if [ "$top" = "libraries" ]; then echo "library"
  elif [ -d "$V1/$top" ] && [ -d "$REPO/$top" ]; then echo "app"
  elif [ -d "$V1/$top" ]; then echo "new"
  else echo "root"
  fi
}

# V1 패치의 경로를 V2 경로로 바꾼다 (라이브러리 이름만 다르다)
remap() {
  sed -E '/^(diff --git |--- |\+\+\+ |rename from |rename to |copy from |copy to )/{
    s#libraries/DeskStreamCore/#libraries/DeskStreamV2Core/#g
    s#/DeskStreamCore\.h#/DeskStreamV2Core.h#g
  }'
}

patch_for() {
  v1git diff --full-index --binary "$BASE"..HEAD -- "$1" | remap
}

cmd_status() {
  local head commits tops
  head="$(git -C "$V1" rev-parse --short HEAD)"
  echo "V1: $V1"
  echo "기준 커밋: $BASE → V1 HEAD: $head"
  [ -z "$(git -C "$V1" status --porcelain)" ] || echo "⚠️  V1에 커밋하지 않은 변경이 있습니다 (커밋된 변경만 비교합니다)"

  commits="$(v1git log --reverse --format='  %h %s' "$BASE"..HEAD)"
  if [ -z "$commits" ]; then
    echo "✅ 반영할 V1 변경이 없습니다."
    return
  fi
  echo
  echo "## V1 새 커밋"
  echo "$commits"

  tops="$(v1git diff --name-only "$BASE"..HEAD | cut -d/ -f1 | sort -u)"
  local cls label top n hashes
  for cls in app library new hold exclude root; do
    case "$cls" in
      app)     label="이식된 앱 — apply 로 반영" ;;
      library) label="공통 라이브러리 — apply 시 DeskStreamV2Core 로 경로 변환, 충돌 많음" ;;
      new)     label="미이식 앱 — 이식할지 먼저 결정" ;;
      hold)    label="이식 보류" ;;
      exclude) label="V2 대상 아님" ;;
      root)    label="루트 파일 — 참고만 (V2 문서는 직접 작성)" ;;
    esac
    local printed=0
    while IFS= read -r top; do
      [ -n "$top" ] || continue
      [ "$(classify "$top")" = "$cls" ] || continue
      if [ "$printed" = 0 ]; then echo; echo "## $label"; printed=1; fi
      n="$(v1git diff --name-only "$BASE"..HEAD -- "$top" | wc -l | tr -d ' ')"
      hashes="$(v1git log --format=%h "$BASE"..HEAD -- "$top" | paste -sd ' ' -)"
      echo "  $top  (파일 $n개 · 커밋 $hashes)"
    done <<< "$tops"
  done
}

cmd_diff() {
  [ $# -ge 1 ] || die "폴더를 지정하세요: diff <폴더>"
  patch_for "$1"
}

cmd_apply() {
  [ $# -ge 1 ] || die "폴더를 지정하세요: apply <폴더>"
  local top="${1%%/*}" cls patch
  cls="$(classify "$top")"
  case "$cls" in
    exclude) die "$top 은(는) V2 대상이 아닙니다 (V1_SYNC.md exclude)" ;;
    hold)    die "$top 은(는) 이식 보류입니다 (V1_SYNC.md hold)" ;;
    root)    die "루트 파일은 적용하지 않습니다 — V2 문서는 직접 고칩니다" ;;
    new)     echo "⚠️  $top 은(는) V2에 없는 앱입니다. 기준 커밋 이후 변경만 적용되므로, 이식이면 AI_GUIDE.md 'V1 프로젝트를 옮길 때' 절차를 따르세요." >&2 ;;
  esac

  patch="$(patch_for "$1")"
  [ -n "$patch" ] || { echo "✅ $1: 기준 커밋 이후 V1 변경이 없습니다."; return; }

  # V1 객체를 빌려 와야 3-way 병합이 패치의 원본 blob 을 찾는다
  local objects
  objects="$(cd "$V1" && cd "$(git rev-parse --git-common-dir)" && pwd)/objects"
  if printf '%s\n' "$patch" | (cd "$REPO" && GIT_ALTERNATE_OBJECT_DIRECTORIES="$objects" git apply --3way --whitespace=nowarn); then
    echo "✅ $1: 충돌 없이 적용했습니다 (인덱스에 올라감 — git diff --cached 로 확인)."
  else
    echo "⚠️  $1: 일부 충돌이 있습니다. 충돌 파일:" >&2
    (cd "$REPO" && git -c core.quotepath=false diff --name-only --diff-filter=U -- "$1") >&2
    return 1
  fi
}

cmd_set_base() {
  local target="${1:-HEAD}" short tmp
  short="$(git -C "$V1" rev-parse --short "$target^{commit}")" || die "V1에 $target 커밋이 없습니다"
  tmp="$(mktemp)"
  awk -v new="$short" '
    /^```sync/ { inblk = 1 }
    /^```$/    { inblk = 0 }
    inblk && $1 == "base:" { print "base: " new; next }
    { print }
  ' "$SYNC_MD" > "$tmp"
  cat "$tmp" > "$SYNC_MD" && rm -f "$tmp"
  echo "✅ 기준 커밋: $BASE → $short (V1_SYNC.md 의 대응표·반영 이력은 직접 갱신하세요)"
}

case "${1:-status}" in
  status)   cmd_status ;;
  diff)     shift; cmd_diff "$@" ;;
  apply)    shift; cmd_apply "$@" ;;
  set-base) shift; cmd_set_base "$@" ;;
  *) die "알 수 없는 명령: $1 (status | diff <폴더> | apply <폴더> | set-base [커밋])" ;;
esac
