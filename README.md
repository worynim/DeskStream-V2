# 🖥️ DeskStream V2: ESP32-C3 4-OLED (1.3") Board

DeskStream V2는 ESP32-C3 SuperMini와 **1.3" SH1106 128x64 OLED 4개**(가로로 놓으면 512x64)를 쓰는 보드입니다.
1.3" 모듈은 I2C 주소를 바꿀 수 없어, SCL 1개를 공유하고 SDA만 화면별로 나눈 **4버스 BitBang I2C**로 구동합니다.
DeskStream(V1) 저장소의 구조(공통 라이브러리 + 프로젝트별 스케치 폴더)를 그대로 가져와,
프로젝트를 하나씩 V2 하드웨어에 맞게 옮기고 있습니다.

## 🛠️ 하드웨어 사양
- **MCU**: ESP32-C3 SuperMini (`esp32:esp32:nologo_esp32c3_super_mini`)
- **Display**: 1.3" SH1106 128x64 OLED × 4 (주소 0x3C, 변경 불가)
- **I2C**: 4버스 BitBang — SCL GPIO 0 공유, SDA GPIO 2 / 1 / 10 / 7 (OLED1~4)
- **Buttons**: GPIO 3 / 5 / 21 / 9 (BTN1~4)
- **Buzzer**: GPIO 6 · **MIC**: GPIO 4 · **NeoPixel**: GPIO 8 · **GPS**: RX 20 / TX 21
- 상세 정보: [HARDWARE.md](./HARDWARE.md)

> 🌐 **웹 플래셔**: [https://worynim.github.io/DeskStream-V2/](https://worynim.github.io/DeskStream-V2/) — USB로 연결한 V2 보드에 원하는 프로젝트를 브라우저에서 바로 업로드합니다. ([사용법](#-웹-플래셔-web-flasher) · [소스](./web/))

## 📁 프로젝트
| 프로젝트 | 버전 | 상태 |
|:--|:--|:--|
| [OLED_Wide_test](./OLED_Wide_test/) — 512x64 초광폭 디스플레이 테스트 | v1.4.1 | DeskStreamV2Core 사용 · 실기 확인 (2026-11-17) |
| [Game_Of_Life](./Game_Of_Life/) — 512x64 생명 게임 | v1.2.0 | DeskStreamV2Core 사용 · 실기 확인 (2026-11-17) |
| [Universal_Timer](./Universal_Timer/) — 4-OLED 범용 타이머 | v1.4.0 | DeskStreamV2Core 사용 · 실기 확인 (2026-11-19) |
| [Hangeul_Clock](./Hangeul_Clock/) — 고화질 폰트 한글 시계 | v2.12.0 | DeskStreamV2Core 사용 · 실기 확인 (2026-11-17) |
| [ENG_Clock](./ENG_Clock/) — 고화질 폰트 영어 시계 | v1.2.0 | DeskStreamV2Core 사용 · 실기 확인 (2026-11-17) |
| [Chinese_Clock](./Chinese_Clock/) — 고화질 폰트 중국어 시계 | v1.2.0 | DeskStreamV2Core 사용 · 실기 확인 (2026-11-17) |
| [World_Clock](./World_Clock/) — 세계 4곳 시계 | v0.2.0 | DeskStreamV2Core 사용 · 실기 확인 (2026-11-17) |
| [Smart_Info_Station](./Smart_Info_Station/) — 스마트 정보 스테이션 | v2.11.0 | DeskStreamV2Core 사용 · 실기 확인 (2026-11-17) |
| [Stats_Monitor](./Stats_Monitor/) — PC 시스템 상태 모니터 (BLE) | v1.7.0 | DeskStreamV2Core 사용 · 실기 확인 (2026-11-17) |
| [Screen_To_OLEDs](./Screen_To_OLEDs/) — 실시간 화면 스트리머 (WiFi UDP) | 펌웨어 v2.4.0 | DeskStreamV2Core 사용 · 실기 확인 (2026-11-17) |
| [Agent_Monitor](./Agent_Monitor/) — AI 개발 에이전트 모니터 (USB·BLE) | 펌웨어 v0.8.0 | DeskStreamV2Core 사용 · 실기 확인 (2026-11-17) |
| [DS_DECK](./DS_DECK/) — 프로그래머블 매크로 패드 (BLE HID) | v1.7.0 | DeskStreamV2Core 사용 · 실기 확인 (2026-11-17) |
| [Sound_Visualizer_4_OLEDs](./Sound_Visualizer_4_OLEDs/) — 오디오 스펙트럼 비주얼라이저 (MIC) | v3.7.0 | DeskStreamV2Core 사용 · 실기 확인 (2026-11-17) |
| [Wireless_Analyzer](./Wireless_Analyzer/) — 2.4GHz 무선 환경 분석기 | v0.6.0 | DeskStreamV2Core 사용 · 실기 확인 (2026-11-17) |
| [Page_Turner](./Page_Turner/) — BLE 페이지 터너 (웹소설·웹툰·쇼츠 넘기기) | v1.2.0 | DeskStreamV2Core 사용 · 실기 확인 (2026-11-20) |

V1에서 옮겨 올 프로젝트: GPS_DASH (GPS TX와 BTN3 공유 처리 설계 필요 — 하드웨어 준비 후 진행).

### V1 변경 동기화
V1이 바뀌면 [V1_SYNC.md](./V1_SYNC.md)의 기준 커밋 이후 변경을 찾아 반영합니다 (Claude Code에서는 `/sync-v1`).

```bash
bash scripts/v1-sync.sh                    # 반영할 V1 커밋과 바뀐 폴더를 분류해 보기
```

```bash
bash scripts/v1-sync.sh apply <앱 폴더>    # 그 앱의 V1 변경을 3-way로 적용 (겹치는 곳은 충돌 표시)
```

## 🌐 웹 플래셔 (Web Flasher)

**https://worynim.github.io/DeskStream-V2/** — Chrome/Edge 데스크톱 또는 Android Chrome 148+(USB OTG)에서 V2 보드에 15개 프로젝트를 바로 플래시합니다.
V1 웹 플래셔(1.9.0)를 그대로 가져와 V2 앱·주소만 바꿨습니다 — 사용법은 V1 [README 웹 플래셔](https://github.com/worynim/DeskStream#-웹-플래셔-web-flasher)와 같습니다.

- 기본 플래시는 NVS(WiFi·설정) 보존, 필요하면 "전체 지우기". SHA-256(다운로드)·MD5(쓰기 후) 검증 후 자동 재부팅.
- V1 보드(0.96" SSD1306)용 펌웨어와 섞이지 않도록 V2 보드에는 이 주소만 씁니다. GPS_DASH는 V2 이식 전이라 목록에 없습니다.
- PC 클라이언트(Stats_Monitor·Screen_To_OLEDs·Agent_Monitor) 다운로드는 V1과 같은 프로그램이라 V1 저장소 Release를 가리킵니다.

### 🚀 런처 (esp-launcher)

PC 없이 보드에서 프로젝트를 바꿀 수 있게 해 주는 런처입니다. 런처는 factory 영역에, 고른 프로젝트는 앱 영역(ota_0)에 들어갑니다.

1. 웹 플래셔에서 **런처 (esp-launcher)** 를 골라 한 번 굽습니다. 지금 앱은 지워지고 Wi-Fi 설정·글꼴(LittleFS)은 남습니다.
2. 런처 화면의 **Wi-Fi** 에서 폰으로 QR을 찍어 설정 포털에 접속하고 AP를 저장합니다 (앱과 같은 저장소라 앱에서도 그대로 씁니다).
3. **Firmware** 에서 목록([`firmware/manifest.launcher.json`](./firmware/manifest.launcher.json))을 받아 프로젝트를 골라 설치하면 그 앱으로 재부팅합니다.
4. 런처로 돌아가려면 **BTN1(GPIO 3)을 누른 채 RESET** 을 눌렀다 떼고 BTN1을 **2초** 더 누르고 있습니다.
5. 런처가 새로 나오면 런처의 **Info → Update** 로 기기에서 바로 업데이트합니다 (설치된 앱은 지워지므로 다시 설치).

- 런처가 설치된 보드에 웹 플래셔로 다른 프로젝트를 구우면 **런처는 남기고 앱만** ota_0에 설치합니다. 런처를 없애려면 **전체 지우기**를 켜고 굽습니다.
- 앱 코드에서 런처로 돌아가려면 DeskStreamV2Core 0.9.0의 `ds::returnToLauncher()`를 부릅니다.
- 런처 소스는 별도 저장소 `esp-launcher`(V2 보드 ID `deskstream_v2_c3`)에 있습니다. 배포본은 `tools/mkbundle.py deskstream_v2_c3 <DeskStream_V2>/firmware/Launcher` 로 내보낸 번들입니다.

### 관련 파일

| 항목 | 링크 |
|---|---|
| 웹 플래셔 앱 (Vite + TypeScript + esptool-js) · 릴리즈 노트 | [`web/`](./web/) · [`web/RELEASE_NOTES.md`](./web/RELEASE_NOTES.md) |
| 펌웨어 수집·manifest 생성 스크립트 | [`tools/collect-firmware.mjs`](./tools/collect-firmware.mjs) |
| 프로젝트 표시 메타데이터 (이름·버전·아이콘·설명) | [`firmware/projects.json`](./firmware/projects.json) |
| 배포된 펌웨어 목록·오프셋·SHA-256 / 런처용 목록 (자동 생성) | [`firmware/manifest.json`](./firmware/manifest.json) · [`firmware/manifest.launcher.json`](./firmware/manifest.launcher.json) |
| 웹 플래셔에 표시할 PC 클라이언트 목록 | [`firmware/clients.json`](./firmware/clients.json) |
| GitHub Pages 배포 워크플로 | [`.github/workflows/deploy-web.yml`](./.github/workflows/deploy-web.yml) |

### 펌웨어 업데이트 방법

1. 앱을 export 빌드합니다 — Arduino IDE `Sketch → Export Compiled Binary`, 또는 같은 결과를 내는 arduino-cli (`partitions.csv`가 있는 앱은 `:PartitionScheme=huge_app`):

   ```bash
   arduino-cli compile --fqbn esp32:esp32:nologo_esp32c3_super_mini --build-path <저장소 밖 임시 폴더> --export-binaries <스케치폴더>
   ```

2. 저장소 루트에서 수집 스크립트를 실행해 `firmware/<project>/`와 `firmware/manifest.json`·`manifest.launcher.json`을 갱신합니다.

   ```bash
   node tools/collect-firmware.mjs
   ```

3. `firmware/` 변경분을 커밋해 `main`에 push → GitHub Actions가 웹 플래셔를 자동 빌드·배포합니다.

- 로컬 미리보기: `cd web && npm install && npm run dev` → http://localhost:5173/DeskStream-V2/
- 최초 1회: 저장소 **Settings → Pages → Source**를 **GitHub Actions**로 지정합니다.

## 📚 공통 라이브러리
- [libraries/DeskStreamV2Core](./libraries/DeskStreamV2Core/) (v0.9.0) — 화면 엔진(4버스 동시 BitBang)·버튼·부저·WiFi·NTP·설정 저장.
  V1 `DeskStreamCore`와 API가 같고(`namespace ds`), 보드 정의와 화면 전송 계층만 V2용입니다.
  이름이 달라 V1·V3 라이브러리와 스케치북에 함께 둘 수 있습니다 — 스케치는 `#include <DeskStreamV2Core.h>`.
- 새 프로젝트를 만들거나 V1 프로젝트를 옮길 때는 [AI_GUIDE.md](./AI_GUIDE.md)를 먼저 봅니다.

## 빌드
필요한 외부 라이브러리: **U8g2**, (캔버스를 쓰는 프로젝트는) **LovyanGFX** — Arduino 라이브러리 매니저.

처음 한 번, 라이브러리를 스케치북(`~/Documents/Arduino/libraries`)에 심볼릭 링크로 연결합니다 (PC마다 1회).

```bash
bash scripts/link-arduino-lib.sh
```

```bash
arduino-cli compile --fqbn esp32:esp32:nologo_esp32c3_super_mini OLED_Wide_test
```

- 빌드 옵션 **USB CDC On Boot = Enabled**(보드 기본값)를 유지합니다. 꺼지면 `Serial`이 UART0(GPIO 20/21)로 나가 BTN3·GPS 선과 충돌합니다.

## 🧪 테스트
```bash
bash libraries/DeskStreamV2Core/test/run_all.sh
```
