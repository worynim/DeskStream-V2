// worynim@gmail.com
/**
 * @file ds_i2c_platform.h
 * @brief 4버스 BitBang 전송 계층 — 셰도 버퍼 · 캡처 후 재생 · 버스 자가 복구
 *
 * @details ## 전송 방식 — 캡처 후 재생
 *
 *          **명령 바이트를 우리가 만들지 않는다.** 그게 이 구조의 핵심이다.
 *
 *          ① 각 화면의 `updateDisplayArea()`를 **캡처 모드**로 호출하면, byte_cb가
 *             전송하는 대신 버스별 버퍼에 **트랜잭션 단위로** 담는다.
 *             (명령 순서·열 오프셋·제어바이트 규칙이 전부 U8g2 책임이 된다)
 *          ② 캡처분을 트랜잭션 번호끼리 짝지어 **4버스에 동시 전송**한다.
 *             SCL을 공유하므로 참여 화면은 같은 트랜잭션 구조·같은 길이여야 한다.
 *
 *          V2 이력 요약: 명령·데이터 페이로드를 손으로 만든 버전(v1.0.0~v1.0.1)과 dirty 장부를
 *          거친 버전(v1.0.4)은 화면이 깨졌고, **캡처·재생 + 장부 없음**(v1.0.9~)이 정상 동작했다.
 *
 * @note 출처: V2 Hangeul_Clock_bitbang_i2c/i2c_platform.* (v1.1.2).
 *       인덱스 공간 — 셰도는 **화면** 인덱스, 캡처·재생·byte_cb는 **버스** 인덱스다.
 *       DisplayCore 내부 부품이다 — 스케치는 보통 DisplayCore만 쓴다.
 */
#ifndef DS_I2C_PLATFORM_H
#define DS_I2C_PLATFORM_H

#include <Arduino.h>
#include <U8g2lib.h>
#include "../ds_board.h"
#include "ds_display_config.h"
#include "ds_bitbang_i2c.h"

namespace ds {

class I2CPlatform {
public:
    /** 캡처 버퍼 크기 — 한 페이지(명령 트랜잭션 3개 + 데이터 ≤24B 청크 6개)에 충분하다 */
    static constexpr uint16_t kCapBufSize = 256;
    /** 한 페이지에서 캡처할 최대 트랜잭션 수 */
    static constexpr uint8_t kCapTxMax = 16;
    /** 즉시 전송 경로(초기화·명령)가 누적하는 패킷 버퍼 크기 */
    static constexpr uint16_t kPacketBufSize = 256;

    I2CPlatform();

    /** 버스 스캔(시리얼 보고) 후 동시 전송 드라이버를 초기화한다 */
    void begin(const DisplayConfig& cfg);

    // ── 셰도 버퍼 (diff 기준, 화면 인덱스) ──────────────────────────────
    /** 화면의 셰도 버퍼 시작 (kScreenBufBytes, 4바이트 정렬 — ds_frame_diff.h 요구) */
    uint8_t* shadow(uint8_t screenIdx) { return _shadow_buffer[screenIdx]; }
    void invalidateShadow(uint8_t screenIdx);

    // ── 캡처 후 재생 (버스 인덱스) ─────────────────────────────────────
    /** 캡처 모드 시작 — 이후 byte_cb는 전송하지 않고 버퍼에 담는다. captureEnd()와 짝을 맞춘다 */
    void captureBegin();
    void captureEnd();

    /**
     * @brief 캡처한 트랜잭션을 참여 버스들에 **동시에** 재생한다
     * @param busMask 참여 버스 마스크
     * @return 성공한 버스 마스크 (0 = 전부 실패)
     * @details 트랜잭션 구조가 버스마다 다르면 동시 재생이 불가능하므로 0을 돌려준다.
     */
    uint8_t replayCaptured(uint8_t busMask);

    /** @brief U8g2 byte_cb 본체 */
    uint8_t handleByteCb(uint8_t busIdx, uint8_t msg, uint8_t arg_int, void* arg_ptr);

    /** @brief 버스 자가 복구 — SCL 펄스로 붙잡힌 SDA를 풀어준다 */
    void recoverBus();

    /** 누적 오류가 설정 한도를 넘었으면 자가 복구한다 (DisplayCore::push()가 매번 부른다) */
    void recoverIfNeeded() { if (_error_count > _cfg.errorThreshold) recoverBus(); }

    uint32_t errorCount() const { return _error_count; }
    BitBangI2C& bitBang() { return _bb; }

private:
    uint8_t flushPacket(uint8_t busIdx);
    void captureCloseTransaction(uint8_t busIdx);

    DisplayConfig _cfg;
    BitBangI2C _bb;

    /** 셰도 버퍼 — "화면에 실제로 들어가 있는 값"의 사본 (페이지 diff 기준) */
    alignas(4) uint8_t _shadow_buffer[kNumScreens][kScreenBufBytes];

    /** U8g2가 START_TRANSFER~END_TRANSFER 사이에 누적하는 패킷 (즉시 전송 경로용) */
    uint8_t  _packet_buf[kNumBuses][kPacketBufSize];
    uint16_t _packet_ptr[kNumBuses];

    // ── 캡처 상태 (버스 인덱스) ─────────────────────────────────────────
    bool     _capturing = false;
    uint8_t  _cap_buf[kNumBuses][kCapBufSize];   ///< 트랜잭션들이 이어 붙은 바이트 스트림
    uint16_t _cap_len[kNumBuses];
    uint16_t _cap_tx_off[kNumBuses][kCapTxMax];  ///< 트랜잭션 목록 — _cap_buf 안의 (시작, 길이)
    uint16_t _cap_tx_len[kNumBuses][kCapTxMax];
    uint8_t  _cap_tx_cnt[kNumBuses];
    uint16_t _cap_cur[kNumBuses];                ///< 열려 있는(START~END) 트랜잭션의 누적 길이
    bool     _cap_in_tx[kNumBuses];              ///< 버스별 START~END 짝이 열려 있는가
    bool     _cap_overflow = false;

    uint32_t _error_count = 0;
};

}  // namespace ds

// U8g2 콜백 — U8g2는 "몇 번 버스인지"를 넘겨주지 않으므로 버스마다 따로 등록한다
extern "C" uint8_t ds_u8x8_byte_bb_0(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
extern "C" uint8_t ds_u8x8_byte_bb_1(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
extern "C" uint8_t ds_u8x8_byte_bb_2(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
extern "C" uint8_t ds_u8x8_byte_bb_3(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);

#endif
