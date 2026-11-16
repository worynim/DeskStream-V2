// worynim@gmail.com
/**
 * @file ds_i2c_platform.cpp
 * @brief 4버스 BitBang 전송 계층 구현 — 캡처 후 재생
 * @details 명령 바이트는 **U8g2가 만든 것을 그대로** 쓰고(캡처), 전송만 4버스에
 *          동시에 한다(재생). 배경은 ds_i2c_platform.h 참조.
 * @note 출처: V2 Hangeul_Clock_bitbang_i2c/i2c_platform.cpp (v1.1.2).
 *       부팅 버스 스캔은 V2의 내장 Multi_BitBang 대신 BitBangI2C::probe()로 한다.
 */
#include "ds_i2c_platform.h"

namespace ds {

/** byte_cb가 찾아갈 인스턴스 — begin()에서 정한다 (DisplayCore는 보드당 하나다) */
static I2CPlatform* g_platform = nullptr;

I2CPlatform::I2CPlatform() {
    memset(_shadow_buffer, 0xFF, sizeof(_shadow_buffer));
    memset(_packet_ptr, 0, sizeof(_packet_ptr));
    memset(_cap_len, 0, sizeof(_cap_len));
    memset(_cap_tx_cnt, 0, sizeof(_cap_tx_cnt));
    memset(_cap_cur, 0, sizeof(_cap_cur));
    memset(_cap_in_tx, 0, sizeof(_cap_in_tx));
}

void I2CPlatform::begin(const DisplayConfig& cfg) {
    _cfg = cfg;
    g_platform = this;

    _bb.begin(kBusSdaPins, kSclPin, kNumBuses, (uint16_t)cfg.halfBitNs,
              cfg.sclSpinMax, cfg.stretchTimeoutUs);

    // 클럭 설정을 보고한다 — "왜 느린가"를 추측하지 않기 위한 계측값이다.
    const uint32_t measured = _bb.measuredHalfBitNs();
    if (cfg.halfBitNs == 0) {
        Serial.println("[I2C] 반주기 지연: 없음 (최대 속도 — SCL 상승을 기다려 자연 페이싱)");
    } else {
        Serial.printf("[I2C] 반주기 지연: 목표 %uns / 실측 %uns\n",
                      (unsigned)cfg.halfBitNs, (unsigned)measured);
    }

    // 배선 진단 — 화면이 안 나올 때 코드를 의심할지 배선을 의심할지 여기서 갈린다.
    for (uint8_t b = 0; b < kNumBuses; b++) {
        const bool found = _bb.probe(b, kOledAddr);
        Serial.printf("[I2C] 버스%u (SDA %u): 0x%02X %s\n", (unsigned)b, (unsigned)kBusSdaPins[b],
                      (unsigned)kOledAddr, found ? "OK" : "NOT FOUND");
    }
}

void I2CPlatform::invalidateShadow(uint8_t screenIdx) {
    if (screenIdx < kNumScreens) {
        // 0xEE는 0x00(Clear)이나 0xFF(Initial)와 다른 특수 값으로,
        // 다음 diff가 모든 타일을 변경으로 잡게 한다.
        memset(_shadow_buffer[screenIdx], 0xEE, sizeof(_shadow_buffer[screenIdx]));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 캡처
// ─────────────────────────────────────────────────────────────────────────────

void I2CPlatform::captureBegin() {
    for (uint8_t b = 0; b < kNumBuses; b++) {
        _cap_len[b] = 0;
        _cap_tx_cnt[b] = 0;
        _cap_cur[b] = 0;
        _cap_in_tx[b] = false;
    }
    _cap_overflow = false;
    _capturing = true;
}

void I2CPlatform::captureEnd() {
    for (uint8_t b = 0; b < kNumBuses; b++) {
        if (_cap_in_tx[b]) captureCloseTransaction(b);   // END 없이 끝난 경우 방어
    }
    _capturing = false;
}

/**
 * @brief START~END 사이에 담긴 바이트를 한 트랜잭션으로 확정한다
 * @details `_cap_cur[bus]`가 이번 트랜잭션이 누적한 길이,
 *          `_cap_len[bus]`가 전체 스트림 길이다. 시작 오프셋 = 전체 - 이번 길이.
 */
void I2CPlatform::captureCloseTransaction(uint8_t busIdx) {
    const uint16_t cur = _cap_cur[busIdx];
    _cap_cur[busIdx] = 0;
    if (cur == 0) return;   // 담긴 바이트가 없는 START/END 쌍은 트랜잭션이 아니다

    if (_cap_tx_cnt[busIdx] >= kCapTxMax) { _cap_overflow = true; return; }
    const uint8_t idx = _cap_tx_cnt[busIdx];
    _cap_tx_off[busIdx][idx] = (uint16_t)(_cap_len[busIdx] - cur);
    _cap_tx_len[busIdx][idx] = cur;
    _cap_tx_cnt[busIdx] = (uint8_t)(idx + 1);
}

uint8_t I2CPlatform::handleByteCb(uint8_t busIdx, uint8_t msg, uint8_t arg_int, void* arg_ptr) {
    if (busIdx >= kNumBuses) return 0;

    if (_capturing) {
        switch (msg) {
            case U8X8_MSG_BYTE_INIT:
            case U8X8_MSG_BYTE_SET_DC:      // I2C에서는 무시된다
                break;
            case U8X8_MSG_BYTE_START_TRANSFER:
                if (_cap_in_tx[busIdx]) captureCloseTransaction(busIdx);  // START 겹침 방어
                _cap_in_tx[busIdx] = true;
                break;
            case U8X8_MSG_BYTE_SEND:
                if ((uint32_t)_cap_len[busIdx] + arg_int > kCapBufSize) {
                    _cap_overflow = true;
                    break;
                }
                memcpy(&_cap_buf[busIdx][_cap_len[busIdx]], arg_ptr, arg_int);
                _cap_len[busIdx] = (uint16_t)(_cap_len[busIdx] + arg_int);
                _cap_cur[busIdx] = (uint16_t)(_cap_cur[busIdx] + arg_int);
                break;
            case U8X8_MSG_BYTE_END_TRANSFER:
                captureCloseTransaction(busIdx);
                _cap_in_tx[busIdx] = false;
                break;
            default:
                return 0;
        }
        return 1;
    }

    // ── 즉시 전송 경로 (초기화 시퀀스·명령) ──
    switch (msg) {
        case U8X8_MSG_BYTE_INIT:
        case U8X8_MSG_BYTE_SET_DC:
            break;
        case U8X8_MSG_BYTE_START_TRANSFER:
            _packet_ptr[busIdx] = 0;
            break;
        case U8X8_MSG_BYTE_SEND:
            if ((uint32_t)_packet_ptr[busIdx] + arg_int > kPacketBufSize) {
                // 절반만 나간 명령 시퀀스는 화면을 알 수 없는 상태로 만든다 → 버린다
                _packet_ptr[busIdx] = 0;
                return 0;
            }
            memcpy(&_packet_buf[busIdx][_packet_ptr[busIdx]], arg_ptr, arg_int);
            _packet_ptr[busIdx] = (uint16_t)(_packet_ptr[busIdx] + arg_int);
            break;
        case U8X8_MSG_BYTE_END_TRANSFER:
            return flushPacket(busIdx);
        default:
            return 0;
    }
    return 1;
}

/** @brief 누적된 패킷을 해당 버스로 즉시 내보낸다 */
uint8_t I2CPlatform::flushPacket(uint8_t busIdx) {
    const uint16_t len = _packet_ptr[busIdx];
    _packet_ptr[busIdx] = 0;
    if (len == 0) return 1;

    const uint8_t* payloads[kNumBuses] = {nullptr, nullptr, nullptr, nullptr};
    payloads[busIdx] = _packet_buf[busIdx];

    const uint8_t ok = _bb.writeConcurrent(kOledAddr, payloads, len, (uint8_t)(1u << busIdx));
    if (!ok) _error_count++;
    return ok ? 1 : 0;
}

/**
 * @brief 캡처한 트랜잭션들을 참여 버스에 동시 재생한다
 * @details 트랜잭션 **번호끼리** 짝지어 한 번에 쏜다. SCL을 공유하므로 참여 버스는
 *          같은 개수·같은 길이의 트랜잭션을 가져야 한다 — 다르면 정렬이 깨지므로
 *          재생하지 않고 0을 돌려준다(호출부가 순차 경로로 다시 보낸다).
 */
uint8_t I2CPlatform::replayCaptured(uint8_t busMask) {
    busMask &= (uint8_t)((1u << kNumBuses) - 1u);
    if (busMask == 0) return 0;
    if (_cap_overflow) { _error_count++; return 0; }

    // 참여 버스의 트랜잭션 수가 모두 같아야 한다
    uint8_t txCount = 0;
    bool first = true;
    for (uint8_t b = 0; b < kNumBuses; b++) {
        if (!(busMask & (uint8_t)(1u << b))) continue;
        if (first) { txCount = _cap_tx_cnt[b]; first = false; }
        else if (_cap_tx_cnt[b] != txCount) { _error_count++; return 0; }
    }
    if (txCount == 0) return 0;

    for (uint8_t t = 0; t < txCount; t++) {
        // 길이도 같아야 한다 (SCL을 공유하므로 비트 수가 어긋나면 안 된다)
        uint16_t len = 0;
        bool lenFirst = true;
        bool lenOk = true;
        for (uint8_t b = 0; b < kNumBuses; b++) {
            if (!(busMask & (uint8_t)(1u << b))) continue;
            if (lenFirst) { len = _cap_tx_len[b][t]; lenFirst = false; }
            else if (_cap_tx_len[b][t] != len) { lenOk = false; }
        }
        if (!lenOk || len == 0) { _error_count++; return 0; }

        const uint8_t* payloads[kNumBuses];
        for (uint8_t b = 0; b < kNumBuses; b++) {
            payloads[b] = (busMask & (uint8_t)(1u << b)) ? &_cap_buf[b][_cap_tx_off[b][t]] : nullptr;
        }

        if (!_bb.writeConcurrent(kOledAddr, payloads, len, busMask)) {
            _error_count++;
            return 0;
        }
    }
    return busMask;
}

void I2CPlatform::recoverBus() {
    Serial.println("[I2C] BitBang bus recovery (clock pulsing)...");
    _bb.recover();
    _error_count = 0;
}

}  // namespace ds

// ─────────────────────────────────────────────────────────────────────────────
// U8g2 콜백 래퍼 (버스 인덱스)
// ─────────────────────────────────────────────────────────────────────────────

static inline uint8_t bbCb(uint8_t bus, uint8_t msg, uint8_t arg_int, void* arg_ptr) {
    return ds::g_platform ? ds::g_platform->handleByteCb(bus, msg, arg_int, arg_ptr) : 0;
}
extern "C" uint8_t ds_u8x8_byte_bb_0(u8x8_t*, uint8_t msg, uint8_t arg_int, void* arg_ptr) { return bbCb(0, msg, arg_int, arg_ptr); }
extern "C" uint8_t ds_u8x8_byte_bb_1(u8x8_t*, uint8_t msg, uint8_t arg_int, void* arg_ptr) { return bbCb(1, msg, arg_int, arg_ptr); }
extern "C" uint8_t ds_u8x8_byte_bb_2(u8x8_t*, uint8_t msg, uint8_t arg_int, void* arg_ptr) { return bbCb(2, msg, arg_int, arg_ptr); }
extern "C" uint8_t ds_u8x8_byte_bb_3(u8x8_t*, uint8_t msg, uint8_t arg_int, void* arg_ptr) { return bbCb(3, msg, arg_int, arg_ptr); }
