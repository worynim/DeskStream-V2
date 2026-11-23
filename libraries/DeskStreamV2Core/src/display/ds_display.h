// worynim@gmail.com
/**
 * @file ds_display.h
 * @brief 4-OLED 화면 엔진 (DisplayCore) — SH1106 128x64 × 4, 4버스 동시 BitBang
 * @details U8G2 4개(화면별 개별 1KB 버퍼)를 만들고, push()가 셰도 버퍼와 diff한 페이지 구간만
 *          전송한다. 페이지마다 바뀐 화면들을 모아 U8g2가 만든 바이트를 캡처한 뒤
 *          **4버스에 같은 클럭으로 동시에** 보낸다 (ds_i2c_platform.h).
 *
 *          사용 예:
 *          @code
 *          ds::DisplayCore disp;
 *          disp.begin();                       // 기본값 = 반주기 150ns · U8G2_R2
 *          disp.screen(0).drawStr(0, 10, "hi");
 *          disp.push();
 *          @endcode
 * @note API는 DeskStream(V1) DeskStreamCore와 같다. 전송 계층은 Hangeul_Clock_bitbang_i2c
 *       v1.1.2의 display_manager.cpp pushParallel()·begin() 배선을 옮겼다 (DeskStreamV3Core v0.7.0 경유).
 */
#ifndef DS_DISPLAY_H
#define DS_DISPLAY_H

#include <Arduino.h>
#include <U8g2lib.h>
#include "../ds_board.h"
#include "ds_display_config.h"
#include "ds_panel_type.h"
#include "ds_i2c_platform.h"

namespace ds {

class DisplayCore {
public:
    DisplayCore();

    /** 버스 스캔·BitBang 드라이버·U8G2 4개를 초기화하고 화면을 지운다 */
    void begin(const DisplayConfig& cfg = DisplayConfig());

    U8G2& screen(uint8_t idx) { return *_screens[idx]; }
    /** U8G2* 배열 (렌더러 등이 인덱스로 접근할 때) */
    U8G2** screens() { return _screens; }

    /**
     * @brief 바뀐 페이지만 4화면에 동시 전송한다
     * @details 전송은 이 함수 안에서 끝난다(동기). 그래서 반환 직후 버퍼를 다시 그려도 된다.
     *          셰도를 우회하는 `sendBuffer()`를 직접 부르면 셰도와 화면이 어긋난다 — 항상 이 함수를 쓴다.
     *          동시 재생에 실패한 버스는 그 화면만 순차 경로로 다시 보낸다.
     */
    void push();

    /**
     * @brief 512x64 한 장 캔버스(행 우선·MSB 먼저 1bpp)를 4화면 버퍼로 전치한 뒤 push()한다
     * @param canvas       LovyanGFX 1bit 스프라이트 버퍼 등 (ds_canvas.h 형식, 폭 512)
     * @param reverseOrder true면 화면 0에 캔버스 오른쪽 끝 열을 보낸다 (180° 장착 시 화면 순서 뒤집기)
     * @details 전치 결과를 U8g2 버퍼에 **직접** 쓰므로 소프트웨어 회전(DisplayConfig::rotation)은
     *          적용되지 않는다. 방향은 setFlip()·패널 명령으로 맞춘다.
     */
    void pushCanvas(const uint8_t* canvas, bool reverseOrder = false);

    /** 4화면 버퍼를 지우고 셰도를 무효화해 **전 화면을 강제 전송**한다 (잔상 제거) */
    void clearAll();

    /** 한 화면을 다음 push()에서 통째로 다시 보내게 한다 */
    void invalidate(uint8_t idx) { _bus.invalidateShadow(idx); }

    /** V1 API 호환용 — 전송이 push() 안에서 끝나므로 기다릴 것이 없다 */
    void waitIdle() {}

    void setBrightness(uint8_t contrast);
    void setInversion(bool invert);   ///< 0xA7(반전) / 0xA6(정상)

    /**
     * @brief 하드웨어 flip 명령 (setFlipMode)
     * @details false = 0xA1/0xC8, true = 0xA0/0xC0 (U8g2 SH1106 드라이버).
     *          V1과 같다: 기본 회전 U8G2_R2에서는 **false가 정방향**, true면 180° 뒤집힌다.
     *          버퍼에 직접 쓰는 경우(pushCanvas·U8G2_R0)는 반대로 **true가 정방향**이다
     *          (2026-11-17 V2 PCB 실기 확인).
     *          setFlipMode()는 열 보정값을 U8g2 기본값으로 되돌리므로 여기서 다시 적용한다.
     */
    void setFlip(bool flip);

    /**
     * @brief 풀 프레임 전송 FPS를 측정해 시리얼과 화면 0에 보고한다
     * @details 화면 내용을 덮어쓰므로 부팅 중에만 부른다. 0xAA/0x55를 번갈아 채워 모든 바이트가
     *          바뀌게 해 풀 프레임 전송을 강제한다.
     *            · A: 4화면 동시  · B: 화면 0만 — A와 시간이 같으면 동시 전송이 제대로 된 것
     */
    void runFpsBenchmark(uint32_t frames = 50, uint32_t showMs = 8000);

    I2CPlatform& bus() { return _bus; }

private:
    /** 한 화면을 diff해 바뀐 페이지 비트(_dirty)와 구간(_first/_count)을 기록한다 */
    void diffScreen(uint8_t idx);
    void applyColOffset();
    /** OLED 종류를 바꾼 직후 4화면에 알린다 */
    void showPanelType();

    DisplayConfig _cfg;
    PanelType _panelType = PanelType::kSH1106;   ///< NVS에 저장된 OLED 종류 (ds_panel_type.h)
    I2CPlatform _bus;
    U8G2_SH1106_128X64_NONAME_F_SW_I2C _u8g2_0, _u8g2_1, _u8g2_2, _u8g2_3;
    U8G2* _screens[kNumScreens];

    /**
     * U8g2 F(풀 버퍼) 생성자는 인스턴스끼리 **같은 정적 버퍼**를 공유한다.
     * 화면마다 따로 그려 두려면 tile_buf_ptr를 개별 버퍼로 바꿔 끼워야 한다.
     * 4바이트 정렬 — ds_frame_diff.h가 워드 단위로 비교한다.
     */
    alignas(4) uint8_t _buffers[kNumScreens][kScreenBufBytes];

    // diff 결과 (화면 인덱스) — push() 한 번 동안만 쓴다
    uint16_t _dirty[kNumScreens];                   ///< bit p = 페이지 p가 바뀜
    uint8_t  _first[kNumScreens][kPagesPerScreen];  ///< 페이지별 첫 바뀐 타일
    uint8_t  _count[kNumScreens][kPagesPerScreen];  ///< 페이지별 바뀐 타일 수
    bool     _reportedFallback = false;
};

}  // namespace ds

#endif
