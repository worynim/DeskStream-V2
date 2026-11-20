# DeskStream V2 Release Notes

## [v0.16.0] - 2026-11-20
### 📖 Page_Turner 이식 · V1 동기화 도구
- `Page_Turner` v1.2.0: V1 v1.1.0(V1 v4.44.0, `9c96d88`)을 DeskStreamV2Core로 이식. 코드 변경은 include·주석·버전 문자열과
  화면 끄기 전 `waitIdle()` 제거(V2 `push()`는 동기)뿐이다. 딥슬립 깨우기 버튼 BTN1·BTN2는 V2에서 GPIO 3·5로 0~5 범위 안이다.
- 호스트 테스트 통과, 컴파일 745,379 B(공통 `partitions.csv`). 실기 확인 — 세로 화면·BLE 페어링·넘기기·설정 저장·화면 끄기·딥슬립과 깨우기 정상.
- V1 동기화: `V1_SYNC.md`(기준 커밋·앱별 버전 대응표·반영 이력), `scripts/v1-sync.sh`(새 V1 커밋을 폴더별로 분류, 앱 단위 3-way 적용),
  Claude Code `/sync-v1` 스킬(`.claude/skills/sync-v1/`). 지난 동기화(v0.14.0·v0.15.0)를 다시 적용해 코드 결과가 같음을 확인했다.

## [v0.15.0] - 2026-11-19
### 💾 Universal_Timer 1.4.0 — V1 1.3.0 반영
- `Universal_Timer` v1.4.0: V1 v1.3.0(V1 v4.42.0)의 변경을 가져왔다 — 버튼으로 바꾼 시간을 START 시 NVS에 저장해 다시 켜도 유지, 일시정지 후 재개해도 TIME OVER 해제 시 처음 START한 시간으로 복원. 이력은 `Universal_Timer/RELEASE_NOTES.md`.
- 컴파일 확인(442,582 B). 실기 확인 — 재부팅 후 설정 시간 유지, 일시정지 후 알람 복원 정상.

## [v0.14.0] - 2026-11-18
### 📊 Agent_Monitor PC 데몬 0.8.0 — V1 변경 반영
- `Agent_Monitor` PC 데몬 v0.8.0: V1 v4.40.2 · v4.41.0(`c58516b`)의 PC 데몬 0.7.1 · 0.8.0을 그대로 가져왔다 — 상세 화면에서 닫은 세션이 다시 나타나던 문제 수정, `[usage] poll` 로 Claude 사용량 직접 조회(macOS, 기본 꺼짐). 펌웨어는 0.8.0 그대로(주석만 변경). 이력은 `Agent_Monitor/RELEASE_NOTES.md`.
- 파이썬 테스트 65개 통과. 실기 확인은 아직.

## [v0.13.0] - 2026-11-17
### 📡 Wireless_Analyzer 이식
- `Wireless_Analyzer` v0.6.0: V1 0.5.1을 DeskStreamV2Core로 이식. 코드 변경은 include·주석·버전 문자열뿐이다.
- 호스트 테스트 통과. 실기 확인 — WiFi 스캔·채널 활동량·BLE 스캔과 화면 전송이 함께 정상, 6개 화면·버튼 정상.

## [v0.12.0] - 2026-11-17
### 🎤 Sound_Visualizer_4_OLEDs 이식
- `Sound_Visualizer_4_OLEDs` v3.7.0: V1 v3.6.0을 DeskStreamV2Core로 이식. 마이크 입력을 GPIO 0(ADC1_CH0) → GPIO 4(ADC1_CH4)로 바꿨다.
- `DeskStreamV2Core` v0.8.0: 마이크 핀 상수 `ds::kMicPin`·`ds::kMicAdcChannel` 추가 (기존 API 변경 없음).
- 라이브러리 호스트 테스트 통과. 실기 확인 — GPIO 4 마이크 입력에 스펙트럼 반응, 화면·버튼 정상.

## [v0.11.0] - 2026-11-17
### ⌨️ DS_DECK 이식
- `DS_DECK` v1.7.0: V1 v1.6.1을 DeskStreamV2Core로 이식. 코드 변경은 include와 주석뿐이다.
- 실기 확인 — BLE 키보드 페어링·매크로·아이콘·웹 설정 정상.

## [v0.10.0] - 2026-11-17
### 🤖 Agent_Monitor 이식
- `Agent_Monitor` 펌웨어 v0.8.0: V1 펌웨어 0.7.1을 DeskStreamV2Core로 이식. 코드 변경은 include·주석·버전 문자열뿐이다.
  PC 데몬(0.7.0)·`p0_spike/`는 그대로 복사했다.
- 호스트 테스트·파이썬 테스트 통과. 실기 확인 — PC 데몬 연결·키 등록·세션 표시·승인·버튼 정상.

## [v0.9.0] - 2026-11-17
### 🎬 Screen_To_OLEDs 이식
- `Screen_To_OLEDs` 펌웨어 v2.4.0: V1 펌웨어 v2.3.1을 DeskStreamV2Core로 이식. 코드 변경은 include와 주석뿐이다. PC 스트리머(2.3.4)는 그대로 복사했다.
- `Screen_To_OLEDs`·`OLED_Wide_test`(v1.4.1): 플립 기본값을 OFF(정방향)로 바꿨다 — 저장값이 없는 보드가 뒤집혀 시작하지 않는다.
- 실기 확인 — 스트리밍(약 17~30fps)·방향·화면 순서·버튼 정상.

## [v0.8.0] - 2026-11-17
### 📊 Stats_Monitor 이식
- `Stats_Monitor` v1.7.0: V1 펌웨어 v1.6.1을 DeskStreamV2Core로 이식. 코드 변경은 include와 주석뿐이다. PC 클라이언트(2.4.2)는 그대로 복사했다.
- 실기 확인 — PC 클라이언트와 BLE 연결, 지표 표시·버튼 정상 (BLE와 BitBang 전송을 함께 쓰는 첫 프로젝트).

## [v0.7.0] - 2026-11-17
### 🌤️ Smart_Info_Station 이식
- `Smart_Info_Station` v2.11.0: V1 v2.10.1을 DeskStreamV2Core로 이식. 코드 변경은 include와 주석뿐이다.
- 실기 확인 — 데이터 수신·화면·반전 4종(패널 명령 직접 전송)·버튼 정상.

## [v0.6.0] - 2026-11-17
### 🌍 World_Clock 이식
- `World_Clock` v0.2.0: V1 v0.1.0을 DeskStreamV2Core로 이식. 코드 변경은 include와 버전 문자열뿐이다.
- 호스트 테스트 통과. 실기 확인 — 화면·도시 설정·버튼 정상 (V1의 0.1.0은 실제 보드 확인 전 배포본이었고, 이번이 첫 실기 확인이다).

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
