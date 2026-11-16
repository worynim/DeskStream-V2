# DeskStream V2 Release Notes

## [v0.1.0] - 2026-11-17
### 🧱 V1 저장소 구조 이관 + 첫 포팅 (OLED_Wide_test)
- `libraries/DeskStreamV2Core` v0.7.0: V1 `DeskStreamCore` v0.6.1의 보드 정의(`ds_board.h`)와 화면 전송 계층을
  4버스 동시 BitBang(SH1106 128x64 × 4)으로 바꾼 공통 라이브러리. 호스트 테스트 9종 통과.
- `scripts/link-arduino-lib.sh`: 스케치북에 `DeskStreamV2Core` 심볼릭 링크를 만든다.
- `OLED_Wide_test` v1.4.0: V1 v1.3.1을 DeskStreamV2Core로 이식. 실기 확인 — 4화면 정방향·순서 정상, 30fps.
- 실기에서 정한 것: 방향은 V1과 같다(`U8G2_R2` + `setFlip(false)`가 정방향). 화면 0~3 = 왼쪽부터 OLED1~4.
- `AI_GUIDE.md`: V2 보드·DeskStreamV2Core 기준 작업 가이드 (V1 프로젝트 이관 절차 포함).
- `README.md`: V2 저장소 소개·프로젝트 목록·빌드 방법.
