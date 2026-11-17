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

## 📁 프로젝트
| 프로젝트 | 버전 | 상태 |
|:--|:--|:--|
| [OLED_Wide_test](./OLED_Wide_test/) — 512x64 초광폭 디스플레이 테스트 | v1.4.1 | DeskStreamV2Core 사용 · 실기 확인 (2026-11-17) |
| [Game_Of_Life](./Game_Of_Life/) — 512x64 생명 게임 | v1.2.0 | DeskStreamV2Core 사용 · 실기 확인 (2026-11-17) |
| [Universal_Timer](./Universal_Timer/) — 4-OLED 범용 타이머 | v1.3.0 | DeskStreamV2Core 사용 · 실기 확인 (2026-11-17) |
| [Hangeul_Clock](./Hangeul_Clock/) — 고화질 폰트 한글 시계 | v2.12.0 | DeskStreamV2Core 사용 · 실기 확인 (2026-11-17) |
| [ENG_Clock](./ENG_Clock/) — 고화질 폰트 영어 시계 | v1.2.0 | DeskStreamV2Core 사용 · 실기 확인 (2026-11-17) |
| [Chinese_Clock](./Chinese_Clock/) — 고화질 폰트 중국어 시계 | v1.2.0 | DeskStreamV2Core 사용 · 실기 확인 (2026-11-17) |
| [World_Clock](./World_Clock/) — 세계 4곳 시계 | v0.2.0 | DeskStreamV2Core 사용 · 실기 확인 (2026-11-17) |
| [Smart_Info_Station](./Smart_Info_Station/) — 스마트 정보 스테이션 | v2.11.0 | DeskStreamV2Core 사용 · 실기 확인 (2026-11-17) |
| [Stats_Monitor](./Stats_Monitor/) — PC 시스템 상태 모니터 (BLE) | v1.7.0 | DeskStreamV2Core 사용 · 실기 확인 (2026-11-17) |
| [Screen_To_OLEDs](./Screen_To_OLEDs/) — 실시간 화면 스트리머 (WiFi UDP) | 펌웨어 v2.4.0 | DeskStreamV2Core 사용 · 실기 확인 (2026-11-17) |
| [Agent_Monitor](./Agent_Monitor/) — AI 개발 에이전트 모니터 (USB·BLE) | 펌웨어 v0.8.0 | DeskStreamV2Core 사용 · 실기 확인 (2026-11-17) |

V1에서 옮겨 올 프로젝트:
DS_DECK, Sound_Visualizer_4_OLEDs, GPS_DASH, Wireless_Analyzer.

## 📚 공통 라이브러리
- [libraries/DeskStreamV2Core](./libraries/DeskStreamV2Core/) (v0.7.0) — 화면 엔진(4버스 동시 BitBang)·버튼·부저·WiFi·NTP·설정 저장.
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
