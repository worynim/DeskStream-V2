# DeskStream V2 Release Notes

## [v0.5.0] - 2026-11-17
### 🕐 Chinese_Clock 이식
- `Chinese_Clock` v1.2.0: V1 v1.1.1을 DeskStreamV2Core로 이식. 코드 변경은 include와 주석, 테스트가 읽는 라이브러리 경로뿐이다.
- 호스트 테스트·JS 교차 검증 통과. 실기 확인 — 화면·버튼·웹 설정 정상.

## [v0.4.0] - 2026-11-17
### 🕐 ENG_Clock 이식
- `ENG_Clock` v1.2.0: V1 v1.1.1을 DeskStreamV2Core로 이식. 코드 변경은 include와 주석, 테스트가 읽는 라이브러리 경로뿐이다.
- JS ↔ 펌웨어 교차 검증 통과. 실기 확인 — 화면·버튼·웹 설정 정상.

## [v0.3.0] - 2026-11-17
### 🕐 Hangeul_Clock 이식
- `Hangeul_Clock` v2.12.0: V1 v2.11.1을 DeskStreamV2Core로 이식. 코드 변경은 include와 주석, 테스트가 읽는 라이브러리 경로뿐이다.
  설정(NVS `clock`)·폰트가 이어진다.
- 호스트 테스트·웹 페이지 JS 검증 통과. 실기 확인 — 시간 표시·버튼·웹 설정 정상 (WiFi와 BitBang 전송을 함께 쓰는 첫 프로젝트).
- `Hangeul_Clock_bitbang_i2c`(v1.1.2)를 저장소에서 삭제했다 — `Hangeul_Clock` v2.12.0이 대체한다. 함께 들어 있던 Multi_BitBang(GPLv3) 소스도 없어졌다.
- `HARDWARE.md`: 출처 표기를 `ds_board.h`로 바꿨다.

## [v0.2.0] - 2026-11-17
### 🧩 Game_Of_Life · Universal_Timer 이식
- `Game_Of_Life` v1.2.0: V1 v1.1.1을 DeskStreamV2Core로 이식. 난수 시드를 `analogRead(0)` → `esp_random()`으로 바꿨다(GPIO 0 = OLED SCL).
- `Universal_Timer` v1.3.0: V1 v1.2.0을 DeskStreamV2Core로 이식. 코드 변경은 include뿐이다.
- 둘 다 실기 확인 — 4화면 표시·버튼 4개(GPS와 선을 공유하는 BTN3 포함)·부저 정상.

## [v0.1.0] - 2026-11-17
### 🧱 V1 저장소 구조 이관 + 첫 포팅 (OLED_Wide_test)
- `libraries/DeskStreamV2Core` v0.7.0: V1 `DeskStreamCore` v0.6.1의 보드 정의(`ds_board.h`)와 화면 전송 계층을
  4버스 동시 BitBang(SH1106 128x64 × 4)으로 바꾼 공통 라이브러리. 호스트 테스트 9종 통과.
- `scripts/link-arduino-lib.sh`: 스케치북에 `DeskStreamV2Core` 심볼릭 링크를 만든다.
- `OLED_Wide_test` v1.4.0: V1 v1.3.1을 DeskStreamV2Core로 이식. 실기 확인 — 4화면 정방향·순서 정상, 30fps.
- 실기에서 정한 것: 방향은 V1과 같다(`U8G2_R2` + `setFlip(false)`가 정방향). 화면 0~3 = 왼쪽부터 OLED1~4.
- `AI_GUIDE.md`: V2 보드·DeskStreamV2Core 기준 작업 가이드 (V1 프로젝트 이관 절차 포함).
- `README.md`: V2 저장소 소개·프로젝트 목록·빌드 방법.
