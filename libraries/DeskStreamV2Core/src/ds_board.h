// worynim@gmail.com
/**
 * @file ds_board.h
 * @brief DeskStream V2 보드 공통 핀 맵 · 화면 치수 (HARDWARE.md 기준)
 * @details 모든 서브 프로젝트가 같은 보드를 쓰므로 핀 정의를 한 곳에 둔다.
 *          Arduino 의존이 없어 호스트 테스트에서도 include 할 수 있다.
 *          OLED는 1.3" SH1106 128x64 × 4.
 */
#ifndef DS_BOARD_H
#define DS_BOARD_H

#include <stdint.h>

namespace ds {

// === 버튼 (INPUT_PULLUP, GND 연결 시 눌림) ===
//   BTN3(GPIO 21)은 ESP TX → GPS RX와 공유한다 (버튼 쪽 1kΩ 직렬 저항).
//   BTN4(GPIO 9)는 BOOT 스트래핑 핀 — 누른 채 리셋하면 다운로드 모드다.
constexpr uint8_t kNumButtons = 4;
constexpr uint8_t kButtonPins[kNumButtons] = {3, 5, 21, 9};   // BTN1~BTN4

// === 부저 ===
constexpr uint8_t kBuzzerPin = 6;

// === 마이크 (아날로그) ===
//   C3의 ADC2(GPIO 5)는 continuous(DMA) 모드를 지원하지 않아 ADC1 핀에 둔다.
constexpr uint8_t kMicPin = 4;
constexpr uint8_t kMicAdcChannel = 4;   // ADC1_CH4 (= ADC_CHANNEL_4, ADC_UNIT_1)

// === I2C 버스 (4버스 BitBang — SCL 1개 공유, SDA는 화면별) ===
//   1.3" 모듈은 주소를 바꿀 수 없어(전부 0x3C) 버스를 4개 만든다.
constexpr uint8_t kNumBuses = 4;
constexpr uint8_t kSclPin = 0;
/** 버스 번호 → SDA 핀 (물리 OLED1~4). Hangeul_Clock_bitbang_i2c에서 검증된 대응 그대로 둔다 */
constexpr uint8_t kBusSdaPins[kNumBuses] = {2, 1, 10, 7};
/**
 * 화면 번호 → 버스 번호.
 * @details 화면 0~3 = 정방향으로 봤을 때 왼쪽부터 물리 OLED1~4 (2026-11-17 V2 PCB 실기 확인).
 *          화면 순서를 바꿀 때는 위 핀 배열이 아니라 이 표만 바꾼다.
 */
constexpr uint8_t kBusOfScreen[kNumBuses] = {0, 1, 2, 3};
constexpr uint8_t kOledAddr = 0x3C;   // 7비트, 4화면 공통

// === 화면 (SH1106 128x64 × 4) ===
constexpr uint8_t  kNumScreens     = 4;
constexpr uint16_t kScreenWidth    = 128;
constexpr uint16_t kScreenHeight   = 64;
constexpr uint8_t  kPagesPerScreen = 8;     // 8px 높이 페이지
constexpr uint8_t  kTilesPerPage   = 16;    // 8px 폭 타일
constexpr uint16_t kScreenBufBytes = kScreenWidth * kPagesPerScreen;   // 1024

}  // namespace ds

#endif
