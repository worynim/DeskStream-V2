---
name: sync-v1
description: DeskStream(V1, ../DeskStream)의 새 변경을 DeskStream V2에 반영한다. 사용자가 "V1 동기화", "V1 변경 반영", "/sync-v1"을 말하거나 V1을 고친 뒤 V2도 맞추자고 할 때 쓴다.
---

# V1 → V2 동기화

기준과 대응표는 `V1_SYNC.md`, 도구는 `scripts/v1-sync.sh`다. 커밋·업로드 같은 단계마다 사용자 지시를 받는다.

## 1. 현황 파악

```bash
bash scripts/v1-sync.sh
```

- 결과(새 커밋, 분류별 폴더)를 사용자에게 요약해 보여 준다. "반영할 V1 변경이 없습니다"면 끝낸다.
- **미이식 앱**은 이식할지 묻는다. 이식하기로 하면 이 스킬이 아니라 `AI_GUIDE.md` "V1 프로젝트를 옮길 때" 절차로 진행하고,
  안 하면 `V1_SYNC.md`의 `exclude` 또는 `hold`에 넣을지 묻는다.
- V2 대상 아님·보류·루트 파일은 반영하지 않는다. 루트 파일 변경은 V2 문서에 옮길 내용이 있는지만 참고한다.

## 2. 앱별 반영 (한 앱씩)

1. V1 앱의 `RELEASE_NOTES.md`에서 무엇이 바뀌었는지 읽는다. 필요하면 `bash scripts/v1-sync.sh diff <앱>`.
2. `bash scripts/v1-sync.sh apply <앱>` — 결과는 인덱스에 올라간다.
3. 충돌(`<<<<<<<`)을 푼다. 대부분 V2에서 따로 고친 README·RELEASE_NOTES·버전 문자열이다.
   - V2 쪽 내용(V2 버전, 핀, 하드웨어 설명)을 지키고 V1의 새 기능 설명만 가져온다.
4. 적용된 코드에서 V2와 다른 점을 확인한다 — 핀·ADC·UART 직접 사용, `#include <DeskStreamCore.h>`,
   V1 핀 번호가 적힌 주석 (`AI_GUIDE.md` §1 9·10번 규칙).
5. **V2 앱 버전을 올린다** (V2 자체 체계 — 기능 추가면 minor). `config.h` 등 버전 문자열과 앱 `RELEASE_NOTES.md`에
   "V1 x.y.z 반영"을 쓴다.
6. 검증: 앱에 `test/`가 있으면 호스트 테스트, 파이썬이면 그 테스트, 펌웨어는 저장소 밖 임시 폴더로 컴파일
   (V1과 같은 PartitionScheme — `AI_GUIDE.md` §2).
7. 펌웨어가 바뀌었으면 업로드 여부를 사용자에게 묻고, 업로드 후 **사용자 실기 확인을 기다린다.**
   PC 쪽만 바뀐 경우는 테스트 결과로 갈음하되 실기 미확인임을 알린다.

`libraries`(DeskStreamCore) 변경은 경로를 바꿔 적용되지만 V2 화면 계층과 다르므로, 변경마다 V2에 필요한지 사용자와 정한다.
라이브러리를 고치면 `AI_GUIDE.md` §7(버전·사용 중인 스케치 재빌드)을 따른다.

## 3. 마무리

1. 문서: 앱 README·RELEASE_NOTES, 루트 `README.md` 프로젝트 표의 버전, 루트 `RELEASE_NOTES.md`에 새 V2 버전 항목
   (V1 커밋·버전, 테스트·실기 확인 결과).
2. `V1_SYNC.md`: 대응표의 V1 기준·V2 버전, 반영 이력 한 줄. 반영하지 않기로 한 변경도 이력에 이유를 적는다.
3. 이번 범위를 모두 처리했으면 `bash scripts/v1-sync.sh set-base` (V1 HEAD로). 일부만 처리했으면 기준을 올리지 않는다.
4. 사용자가 "커밋"이라고 하면 커밋한다. 푸시는 따로 지시를 받는다.
