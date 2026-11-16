# ESP32-C3 Ultra-Wide OLED (512x64) Matrix — DeskStream V2

4개의 1.3" 128x64 OLED(SH1106)를 가로로 배치해 하나의 **512x64 초광폭 디스플레이**로 제어하는 테스트 프로젝트입니다.
DeskStream V2 보드에서 **4개 OLED와 공통 라이브러리(DeskStreamV2Core)가 문제없이 동작하는지 확인**하는 첫 번째 포팅 대상입니다.
DeskStream(V1) 저장소의 OLED_Wide_test v1.3.1을 옮겨 왔습니다.

https://www.youtube.com/playlist?list=PL2He47zwR3XjQv_0kjdzW76SOO_vlLyVU

## 🚀 주요 특징

### 1. 4버스 동시 BitBang I2C
*   1.3" 모듈은 주소가 전부 0x3C라 **SCL 1개를 공유하고 SDA만 화면별로 분리**한 4개 버스를 씁니다.
*   SCL을 공유한 채 4개 SDA에 서로 다른 비트를 실어 **4화면에 같은 클럭으로 동시에** 전송합니다 (DeskStreamV2Core).

### 2. 지능형 렌더링 엔진 (Smart Flush)
*   **Shadow Buffering**: 512x64 가상 캔버스의 내용을 그림자 버퍼와 비교해 **바뀐 부분(Dirty Page)만** 전송합니다.
*   **LovyanGFX + DeskStreamV2Core 조합**: 그리기는 LovyanGFX가, 전치·바뀐 타일만 4화면 동시 전송은 `ds::DisplayCore::pushCanvas()`가 담당합니다.

### 3. 안정적인 프레임 제어 (Frame Pacing)
*   **Target FPS (30 FPS)**: 프레임 제한기로 애니메이션 속도를 일정하게 유지합니다.
*   **smartDelay**: 지연 시간(Pause) 중에도 화면 갱신 엔진을 계속 구동합니다.
*   **FPS 모니터링**: Ticker로 1초마다 시리얼 모니터(115200)에 실시간 FPS를 보고합니다.

## 🛠 하드웨어 구성
- **MCU**: ESP32-C3 SuperMini
- **Display**: SH1106 128x64 OLED x 4 (주소 0x3C)
  - SCL(공유): GPIO 0
  - SDA: OLED1 GPIO 2 / OLED2 GPIO 1 / OLED3 GPIO 10 / OLED4 GPIO 7
- **BTN 1 (GPIO 3)**: 화면 180도 회전 (Flip)
- **Buzzer (GPIO 6)**: 조작 피드백
- 상세: 루트 [HARDWARE.md](../HARDWARE.md)

## 📦 필요한 라이브러리
*   **U8g2**: OLED 컨트롤러 초기화·명령 (라이브러리 매니저)
*   **LovyanGFX**: 고속 그리기 및 가상 캔버스 (라이브러리 매니저)
*   **DeskStreamV2Core**: 저장소 `libraries/DeskStreamV2Core` — 4-OLED 전송·버튼·부저 (빌드 방법은 루트 [README](../README.md#빌드))

## 🕹️ 조작 및 버튼 기능
*   **BTN 1 (GPIO 3) 짧게 누름**: 화면 180도 회전(Flip) 토글.
    *   화면이 뒤집힐 때 OLED 전송 순서도 자동 스왑(`1-2-3-4` ↔ `4-3-2-1`)되어 가로 방향 그래픽의 연속성을 유지합니다.
*   조작 시 3000Hz의 짧은 비프음이 납니다. 설정은 NVS(`wide_set`)에 저장되어 재부팅 시 복원됩니다.

## 📺 포함된 데모
- 가로 512픽셀 전 영역 선 그리기 애니메이션
- 사각형 및 채우기 그래픽 테스트
- 중앙 기준 동심원 및 채우기 테스트
- 둥근 사각형 및 삼각형 데모
- 여러 크기의 텍스트와 512픽셀 너비의 흐르는 자막(Ticker)

## ✅ 실기 확인 순서
1. 시리얼(115200)에서 부팅 버스 스캔을 확인합니다. 4개 버스 모두 0x3C가 `OK`여야 합니다.
   `NOT FOUND`면 그 버스의 배선·전원(SDA·SCL·VCC·GND)을 확인합니다.
2. 데모가 4화면에 이어져 보이는지, 위아래·좌우가 맞는지 봅니다. 저장값이 없는 보드는 뒤집힌 상태로 시작하므로 BTN 1로 바꿉니다(저장됨).
3. 화면이 **가로로 2픽셀 밀려** 보이면 모듈 컨트롤러가 SH1106이 아닙니다 — `DisplayConfig::colOffset`을 0으로 줍니다.
4. 노이즈·깨짐이 보이면 `DisplayConfig::halfBitNs`를 키웁니다(150 → 250). 시리얼의 `[I2C] 동시 재생 실패` 로그도 확인합니다.
