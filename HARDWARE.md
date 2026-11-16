# 🛠 Hardware Specification: DeskStream V2 board

DeskStream V2 보드의 하드웨어 구성과 핀 맵입니다. 새 프로젝트를 시작하거나 배선을 확인할 때 참조하세요.
(출처: `Hangeul_Clock_bitbang_i2c/config.h`, `README.md` — 2026-11-15 V2 PCB 핀맵)

## 1. 메인 컨트롤러 (Controller)
| 항목 | 사양 | 비고 |
| :--- | :--- | :--- |
| **MCU** | **ESP32-C3 SuperMini** | RISC-V Single-core |
| **FQBN** | `esp32:esp32:nologo_esp32c3_super_mini` | |
| **Operating Voltage** | 3.3V | 모든 로직 레벨 동일 |

## 2. 디스플레이 모듈 (Display)
1.3" 모듈은 I2C 주소를 바꿀 수 없어(전부 0x3C) **SCL 1개를 공유하고 SDA만 화면별로 분리**합니다.
5핀으로 4개의 독립 BitBang I2C 버스를 만듭니다.

| 모듈 ID | 모델 | 인터페이스 | SDA (Pin) | SCL (Pin) | I2C Addr | 비고 |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **OLED 1** | SH1106 | BitBang I2C | **GPIO 2** | **GPIO 0** (공유) | 0x3C | SDA가 스트래핑 핀 |
| **OLED 2** | SH1106 | BitBang I2C | **GPIO 1** | **GPIO 0** (공유) | 0x3C | |
| **OLED 3** | SH1106 | BitBang I2C | **GPIO 10** | **GPIO 0** (공유) | 0x3C | |
| **OLED 4** | SH1106 | BitBang I2C | **GPIO 7** | **GPIO 0** (공유) | 0x3C | |

- **패널**: 1.3" 128×64 모노, 4핀(VCC/GND/SCL/SDA), 리셋 핀 없음
- **SH1106**: 132열 RAM이라 가시 영역이 2열부터 시작 → 열 오프셋 **+2**
- 모듈마다 컨트롤러가 다를 수 있고(SH1106/SSD1315/SSD1306) 소프트웨어로 구별할 수 없습니다. 실기에서 SH1106으로 확인했습니다.

```
   GPIO  0 ──┬── OLED1 SCL
             ├── OLED2 SCL          (4개 모듈 공통)
             ├── OLED3 SCL
             └── OLED4 SCL
   GPIO  2 ────── OLED1 SDA
   GPIO  1 ────── OLED2 SDA
   GPIO 10 ────── OLED3 SDA
   GPIO  7 ────── OLED4 SDA
```

## 3. 사용자 입력 (Buttons)
모든 버튼은 내부 풀업(`INPUT_PULLUP`)을 사용하며, GND와 연결 시 동작합니다.

| 버튼 ID | 연결 핀 (GPIO) | 비고 |
| :--- | :--- | :--- |
| **BTN 1** | **GPIO 3** | |
| **BTN 2** | **GPIO 5** | |
| **BTN 3** | **GPIO 21** | GPS RX(ESP TX)와 공유, 버튼 쪽에 1kΩ 직렬 저항 |
| **BTN 4** | **GPIO 9** | 보드 BOOT 버튼 (스트래핑 핀) |

## 4. 기타 주변장치 (Peripherals)
| 장치 | 연결 핀 (GPIO) | 비고 |
| :--- | :--- | :--- |
| **Buzzer** | **GPIO 6** | |
| **MIC** | **GPIO 4** | ADC1_CH4 (C3의 ADC2는 continuous 모드 미지원) |
| **NeoPixel** | **GPIO 8** | 데이터 핀 (스트래핑 핀) |
| **GPS** | **RX 20 / TX 21** | GPS TX → ESP RX(20), ESP TX(21) → GPS RX. TX는 BTN 3과 공유 |

## ⚠ 주의 사항
- **GPIO 0~10, 20, 21을 전부 사용**합니다. 빈 핀이 없으므로 다른 용도로 쓰지 않습니다.
- **GPIO 2 (OLED1 SDA)** 는 스트래핑 핀(부팅 시 High)입니다. 모듈 풀업으로 통상 문제없지만, 부팅이 불안정하면 외부 4.7kΩ 풀업을 추가합니다.
- **GPIO 9 (BTN 4)** 를 누른 채 리셋하면 다운로드 모드로 들어갑니다. 부팅 시 버튼 판정에는 쓰지 않습니다.
- **GPIO 21** 은 평소 BTN 3 입력이고, GPS 설정을 보낼 때만 잠시 UART TX로 씁니다.
- 빌드 옵션 **USB CDC On Boot = Enabled** 를 유지합니다. 꺼지면 `Serial`이 UART0(20/21)로 나가 BTN 3·GPS 선과 충돌합니다.

## 📍 전체 핀 맵 요약 (Pin Map Summary)
- **GPIO 0**: BitBang I2C SCL (OLED 1~4 공유)
- **GPIO 1**: OLED 2 SDA
- **GPIO 2**: OLED 1 SDA (스트래핑)
- **GPIO 3**: BTN 1
- **GPIO 4**: MIC (ADC1_CH4)
- **GPIO 5**: BTN 2
- **GPIO 6**: Buzzer
- **GPIO 7**: OLED 4 SDA
- **GPIO 8**: NeoPixel (스트래핑)
- **GPIO 9**: BTN 4 / BOOT (스트래핑)
- **GPIO 10**: OLED 3 SDA
- **GPIO 20 (RX)**: GPS TX → ESP RX
- **GPIO 21 (TX)**: ESP TX → GPS RX / BTN 3 (공유)
