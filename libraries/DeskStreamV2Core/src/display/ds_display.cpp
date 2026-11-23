// worynim@gmail.com
/**
 * @file ds_display.cpp
 * @brief 4-OLED 화면 엔진 구현 — 페이지 diff + 4버스 동시 전송
 * @note 출처: Hangeul_Clock_bitbang_i2c/display_manager.cpp (v1.1.2) begin()·pushParallel()·clearAll(),
 *       V1 DeskStreamCore ds_display.cpp pushCanvas()·runFpsBenchmark()
 */
#include "ds_display.h"
#include "ds_canvas.h"
#include "ds_frame_diff.h"
#include "ds_panel_type.h"

namespace ds {

// 핀은 U8X8_PIN_NONE으로 둔다 — U8g2가 핀을 만지지 않고, 버스별 byte_cb가 전송한다.
DisplayCore::DisplayCore()
    : _u8g2_0(U8G2_R0, U8X8_PIN_NONE, U8X8_PIN_NONE, U8X8_PIN_NONE),
      _u8g2_1(U8G2_R0, U8X8_PIN_NONE, U8X8_PIN_NONE, U8X8_PIN_NONE),
      _u8g2_2(U8G2_R0, U8X8_PIN_NONE, U8X8_PIN_NONE, U8X8_PIN_NONE),
      _u8g2_3(U8G2_R0, U8X8_PIN_NONE, U8X8_PIN_NONE, U8X8_PIN_NONE) {
    _screens[0] = &_u8g2_0; _screens[1] = &_u8g2_1; _screens[2] = &_u8g2_2; _screens[3] = &_u8g2_3;
}

void DisplayCore::applyColOffset() {
    const uint8_t off = _cfg.colOffset >= 0 ? (uint8_t)_cfg.colOffset : panelColOffset(_panelType);
    for (int i = 0; i < kNumScreens; i++) _screens[i]->getU8x8()->x_offset = off;
}

void DisplayCore::showPanelType() {
    for (int i = 0; i < kNumScreens; i++) {
        U8G2& s = *_screens[i];
        s.clearBuffer();
        s.setFont(u8g2_font_6x10_tf);
        s.drawStr(0, 12, "OLED:");
        s.drawStr(36, 12, panelTypeName(_panelType));
        s.drawStr(0, 36, "Saved.");
        s.drawStr(0, 58, "BTN2+RESET 2s: change");
    }
    push();
}

void DisplayCore::begin(const DisplayConfig& cfg) {
    _cfg = cfg;

    // 1. 버스 스캔 + 동시 전송 드라이버
    _bus.begin(cfg);

    // 2. 화면별 byte_cb 등록 후 U8g2 초기화 시퀀스 전송 (즉시 전송 경로)
    //    화면 i가 쓸 **버스**는 kBusOfScreen[i]가 정한다 (ds_board.h).
    static u8x8_msg_cb const kBbCb[kNumBuses] = {
        ds_u8x8_byte_bb_0, ds_u8x8_byte_bb_1, ds_u8x8_byte_bb_2, ds_u8x8_byte_bb_3
    };
    for (int i = 0; i < kNumScreens; i++) {
        _screens[i]->getU8g2()->tile_buf_ptr = _buffers[i];
        _screens[i]->setDisplayRotation(cfg.rotation);
        _screens[i]->getU8x8()->byte_cb = kBbCb[kBusOfScreen[i]];
        _screens[i]->begin();
    }
    _panelType = loadPanelType();
    applyColOffset();

    clearAll();

    // 3. BTN2를 누른 채 리셋해 2초 유지하면 OLED 종류(SH1106 ↔ SSD1306)를 바꿔 저장하고 알린다
    if (panelToggleRequested()) {
        _panelType = otherPanelType(_panelType);
        const bool saved = savePanelType(_panelType);
        Serial.printf("[OLED] 종류 -> %s%s\n", panelTypeName(_panelType), saved ? "" : " (저장 실패)");
        applyColOffset();
        showPanelType();
        while (digitalRead(kButtonPins[1]) == LOW) delay(10);   // BTN2를 뗄 때까지 (앱이 눌림으로 받지 않게)
        delay(1000);
        clearAll();
    }
}

void DisplayCore::diffScreen(uint8_t idx) {
    const uint8_t* buf = _screens[idx]->getBufferPtr();
    uint8_t* shadow = _bus.shadow(idx);
    _dirty[idx] = 0;
    for (int p = 0; p < kPagesPerScreen; p++) {
        const int off = p * kScreenWidth;
        if (fdDiffPage(buf + off, shadow + off, kTilesPerPage, &_first[idx][p], &_count[idx][p])) {
            _dirty[idx] |= (uint16_t)(1u << p);
        }
    }
}

void DisplayCore::push() {
    // 1) diff — 바뀐 페이지와 구간을 모은다 (셰도 버퍼도 여기서 갱신된다)
    for (int s = 0; s < kNumScreens; s++) diffScreen(s);

    // 2) 페이지마다 참여 화면을 모아 4버스에 동시 전송
    for (uint8_t p = 0; p < kPagesPerScreen; p++) {
        // ⚠ 두 인덱스 공간을 섞지 않는다.
        //    · _dirty / _first / _count / _screens[] = **화면** 인덱스
        //    · replayCaptured(busMask) / 캡처 버퍼    = **버스** 인덱스
        uint8_t busMask = 0;
        uint8_t first[kNumScreens];
        uint8_t count[kNumScreens];
        for (uint8_t s = 0; s < kNumScreens; s++) {
            const bool d = (_dirty[s] & (uint16_t)(1u << p)) != 0;
            first[s] = d ? _first[s][p] : 0;
            count[s] = d ? _count[s][p] : 0;
            if (d) busMask |= (uint8_t)(1u << kBusOfScreen[s]);
        }
        if (busMask == 0) continue;

        // SCL을 공유하므로 참여 화면은 **같은 구간**을 보내야 한다 → 합집합 (같은 값을 다시 쓰는 것뿐)
        uint8_t uFirst = 0, uCount = 0;
        bbUnionColumnRange(first, count, kNumScreens, &uFirst, &uCount);
        if (uCount == 0) continue;

        // ① U8g2가 만들 바이트를 캡처한다 (전송하지 않는다)
        _bus.captureBegin();
        for (uint8_t s = 0; s < kNumScreens; s++) {
            if (count[s]) _screens[s]->updateDisplayArea(uFirst, p, uCount, 1);
        }
        _bus.captureEnd();

        // ② 트랜잭션끼리 짝지어 4버스에 동시 전송 (마스크는 **버스** 기준)
        const uint8_t okBusMask = _bus.replayCaptured(busMask);

        // ③ 동시 재생에 실패한 버스는 그 화면만 순차 경로로 다시 보낸다
        //    (한 화면의 배선 고장이 다른 화면까지 멈추지 않게 하는 안전망)
        if (okBusMask != busMask) {
            if (!_reportedFallback) {
                _reportedFallback = true;
                Serial.printf("[I2C] 동시 재생 실패 -> 순차 폴백 (bus=0x%02X ok=0x%02X) err=%u\n",
                              busMask, okBusMask, (unsigned)_bus.errorCount());
            }
            for (uint8_t s = 0; s < kNumScreens; s++) {
                if (!count[s] || (okBusMask & (uint8_t)(1u << kBusOfScreen[s]))) continue;
                _screens[s]->updateDisplayArea(uFirst, p, uCount, 1);
            }
        }
    }

    // 오류가 누적되면 버스를 자가 복구한다 — BitBang은 stuck-low를 ACK로 오독할 수 있어
    //   오류 카운터가 유일한 단서다.
    _bus.recoverIfNeeded();
}

void DisplayCore::pushCanvas(const uint8_t* canvas, bool reverseOrder) {
    if (!canvas) return;
    for (int s = 0; s < kNumScreens; s++) {
        const uint8_t column = reverseOrder ? (uint8_t)(kNumScreens - 1 - s) : (uint8_t)s;
        canvasToScreenPages(canvas, kScreenWidth * kNumScreens, column, _buffers[s]);
    }
    push();
}

void DisplayCore::clearAll() {
    for (int i = 0; i < kNumScreens; i++) {
        _screens[i]->clearBuffer();
        _bus.invalidateShadow(i);   // 모든 0픽셀을 강제로 전송하게 한다
    }
    push();
}

void DisplayCore::setBrightness(uint8_t contrast) {
    for (int i = 0; i < kNumScreens; i++) _screens[i]->setContrast(contrast);
}

void DisplayCore::setInversion(bool invert) {
    for (int i = 0; i < kNumScreens; i++) _screens[i]->sendF("c", invert ? 0xA7 : 0xA6);
}

void DisplayCore::setFlip(bool flip) {
    for (int i = 0; i < kNumScreens; i++) _screens[i]->setFlipMode(flip);
    applyColOffset();
    // 열 보정·스캔 방향이 바뀌었으니 화면 전체를 새 배치로 다시 보낸다
    for (int i = 0; i < kNumScreens; i++) _bus.invalidateShadow(i);
}

// ─────────────────────────────────────────────────────────────────────────────
// FPS 벤치마크 — 호출하지 않으면 링커가 제거한다 (--gc-sections)
// ─────────────────────────────────────────────────────────────────────────────

/** 한 케이스의 측정 결과 */
struct FpsBenchResult {
    uint32_t frames;
    uint32_t totalUs;   // N프레임 전체
    uint64_t busUs;     // BitBang 전송 시간
    uint64_t busBits;   // BitBang 비트 주기 수
    uint32_t busTx;     // 트랜잭션 수
};

/** screenMask에 속한 화면만 매 프레임 전부 바뀌게 그리고 push()를 잰다 */
static FpsBenchResult fpsBenchCase(DisplayCore& dc, uint8_t screenMask, uint32_t frames) {
    for (int s = 0; s < kNumScreens; s++) dc.screen(s).clearBuffer();
    dc.push();

    BitBangI2C& bb = dc.bus().bitBang();
    bb.resetStats();

    FpsBenchResult r = {};
    r.frames = frames;
    const uint32_t t0 = micros();
    for (uint32_t f = 0; f < frames; f++) {
        const uint8_t pat = (f & 1u) ? 0x55 : 0xAA;
        for (int s = 0; s < kNumScreens; s++) {
            if (screenMask & (1u << s)) memset(dc.screen(s).getBufferPtr(), pat, kScreenBufBytes);
        }
        dc.push();
    }
    r.totalUs = (uint32_t)(micros() - t0);
    r.busUs = bb.statUs(); r.busBits = bb.statBits(); r.busTx = bb.statTx();
    return r;
}

static float benchKHz(uint64_t bits, uint64_t us) { return us ? (float)bits * 1000.0f / (float)us : 0.0f; }
static float benchFps(const FpsBenchResult& r) { return r.totalUs ? 1e6f * r.frames / r.totalUs : 0.0f; }
static float benchMs(const FpsBenchResult& r)  { return r.frames ? r.totalUs / 1000.0f / r.frames : 0.0f; }

static void fpsBenchPrint(const char* name, const FpsBenchResult& r) {
    const float n = (float)(r.frames ? r.frames : 1);
    Serial.printf("[FPS] %s\n", name);
    Serial.printf("[FPS]   %u프레임, 평균 %.2f ms/frame -> %.1f FPS\n", (unsigned)r.frames, benchMs(r), benchFps(r));
    Serial.printf("[FPS]   버스: %.2f ms/frame, 실효 SCL %.0f kHz (트랜잭션 %u/frame)\n",
                  (float)r.busUs / 1000.0f / n, benchKHz(r.busBits, r.busUs), (unsigned)(r.busTx / n));
}

void DisplayCore::runFpsBenchmark(uint32_t frames, uint32_t showMs) {
    Serial.printf("[FPS] 벤치마크 시작 — 반주기 %u ns(실측 %u), CPU %u MHz\n",
                  (unsigned)_cfg.halfBitNs, (unsigned)_bus.bitBang().measuredHalfBitNs(),
                  (unsigned)getCpuFrequencyMhz());
    const uint32_t err0 = _bus.errorCount();

    const FpsBenchResult a = fpsBenchCase(*this, 0x0F, frames);   // 4화면
    const FpsBenchResult b = fpsBenchCase(*this, 0x01, frames);   // 1화면

    fpsBenchPrint("A) 4화면 풀 프레임 (동시 전송)", a);
    fpsBenchPrint("B) 화면 0만 (1화면)", b);
    const uint32_t err = _bus.errorCount() - err0;
    Serial.printf("[FPS] 전송 오류: %u\n", (unsigned)err);

    // 화면에도 띄운다 — USB CDC는 포트를 늦게 열면 부팅 로그를 놓친다
    char line[32];
    for (int s = 0; s < kNumScreens; s++) _screens[s]->clearBuffer();
    U8G2* u = _screens[0];
    u->setFont(u8g2_font_6x10_tf);
    u->drawStr(0, 10, "FPS BENCH (BB x4)");
    snprintf(line, sizeof(line), "4scr %.1fms %.0ffps", benchMs(a), benchFps(a)); u->drawStr(0, 24, line);
    snprintf(line, sizeof(line), "1scr %.1fms %.0ffps", benchMs(b), benchFps(b)); u->drawStr(0, 36, line);
    snprintf(line, sizeof(line), "SCL %.0fkHz e%u", benchKHz(a.busBits, a.busUs), (unsigned)err);
    u->drawStr(0, 48, line);
    push();
    delay(showMs);

    for (int s = 0; s < kNumScreens; s++) _screens[s]->clearBuffer();
    push();
}

}  // namespace ds
