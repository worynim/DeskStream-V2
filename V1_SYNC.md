# V1 → V2 동기화 기준

DeskStream(V1, `../DeskStream`)의 변경을 V2에 맞춰 반영할 때 쓰는 기준 문서다.
`scripts/v1-sync.sh`가 아래 `sync` 블록을 읽어 반영할 변경을 찾는다. 절차는 Claude Code `/sync-v1` 스킬
(`.claude/skills/sync-v1/SKILL.md`)에 있다.

## 스크립트 설정

```sync
base: 9c96d88
exclude: web firmware tools .github Chiptune_Buzzer I2C_Scanner
hold: GPS_DASH
```

- `base` — **동기화 기준 V1 커밋.** 이 커밋까지의 V1 변경은 V2에 반영됐다. 반영을 마치면
  `bash scripts/v1-sync.sh set-base`로 V1 HEAD로 올린다.
- `exclude` — V2 대상 아님 (웹 플래셔 `web/`·`firmware/`·`tools/`·`.github/`, 테스트용 앱).
- `hold` — 이식 보류 (GPS_DASH: 하드웨어 준비 후 BTN3 공유 처리 설계 필요).
- 위 목록에 없고 V2에도 없는 V1 폴더는 **미이식 앱**으로 표시된다 — 이식할지 먼저 정한다.

## 앱별 버전 대응표

V2 각 앱이 V1의 어느 버전에서 이식·동기화됐는지 기록한다. V1 변경을 가져오면 V2 앱 버전은 V2 체계로 올린다.

| 앱 | V1 기준 | V2 버전 |
|:--|:--|:--|
| DeskStreamCore → DeskStreamV2Core | 0.6.1 | 0.8.0 |
| OLED_Wide_test | 1.3.1 | 1.4.1 |
| Game_Of_Life | 1.1.1 | 1.2.0 |
| Universal_Timer | 1.3.0 | 1.4.0 |
| Hangeul_Clock | 2.11.1 | 2.12.0 |
| ENG_Clock | 1.1.1 | 1.2.0 |
| Chinese_Clock | 1.1.1 | 1.2.0 |
| World_Clock | 0.1.0 | 0.2.0 |
| Smart_Info_Station | 2.10.1 | 2.11.0 |
| Stats_Monitor 펌웨어 / PC 클라이언트 | 1.6.1 / 2.4.2 | 1.7.0 / 2.4.2 |
| Screen_To_OLEDs 펌웨어 / PC 스트리머 | 2.3.1 / 2.3.4 | 2.4.0 / 2.3.4 |
| Agent_Monitor 펌웨어 / PC 데몬 | 0.7.1 / 0.8.0 | 0.8.0 / 0.8.0 |
| DS_DECK | 1.6.1 | 1.7.0 |
| Sound_Visualizer_4_OLEDs | 3.6.0 | 3.7.0 |
| Wireless_Analyzer | 0.5.1 | 0.6.0 |
| Page_Turner | 1.1.0 | 1.2.0 |

- PC 쪽(`Stats_Monitor/client_py`, `Screen_To_OLEDs/` 파이썬, `Agent_Monitor/bridge_py`·`p0_spike`)은
  V1과 같은 파일이라 패치가 그대로 적용된다.
- `libraries/DeskStreamCore`는 V2에서 `libraries/DeskStreamV2Core`로 이름이 바뀌었고 화면 전송 계층이 다르다
  (스크립트가 경로를 바꿔 적용하지만 충돌이 많다 — 하나씩 판단한다).

## 반영 이력

| 날짜 | V1 커밋 범위 | 내용 |
|:--|:--|:--|
| 2026-11-17 | ~`69afbac` (v4.40.1) | 첫 이식 |
| 2026-11-19 | `69afbac`..`1779bab` (v4.42.0) | Agent_Monitor PC 데몬 0.8.0, Universal_Timer 1.3.0. `89ec08a`(v4.41.1)는 웹 플래셔만 바꿔 대상 아님 |
| 2026-11-20 | `1779bab`..`9c96d88` (v4.44.0) | Page_Turner 1.1.0 이식(V2 1.2.0). 라이브러리 README 변경은 V2 README "사용 중인 스케치"에 반영 |
