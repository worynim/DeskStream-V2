// worynim@gmail.com
/**
 * @file ds_bitbang_i2c.h
 * @brief ESP32-C3 전용 **4버스 동시(lock-step) BitBang I2C** 드라이버
 * @details SCL 1개를 공유하고 SDA만 화면별로 분리한 구성에서, 4개 화면에
 *          **서로 다른 바이트를 같은 클럭에 동시에** 실어 보낸다.
 *
 *          왜 되는가: I2C 슬레이브는 SCL 상승 에지에서 **자기 SDA만** 래치한다.
 *          따라서 SDA_N에 각자 다른 비트를 실어도 4개가 동시에 자기 바이트를 받는다.
 *          ACK도 슬레이브마다 자기 SDA를 당기므로 한 번의 클럭으로 4개를 모두 읽는다.
 *
 *          "어떤 핀을 HIGH/LOW로 둘 것인가"의 계산은 ds_bitbang_core.h(순수, 호스트 테스트 대상)가 맡는다.
 * @note 출처: V2 Hangeul_Clock_bitbang_i2c/bitbang_i2c.* (v1.1.2).
 *       config.h 매크로(BB_SCL_SPIN_MAX·BB_SCL_STRETCH_TIMEOUT_US) → begin() 인자.
 *       DisplayCore 내부 부품이다 — 스케치는 보통 DisplayCore만 쓴다.
 */
#ifndef DS_BITBANG_I2C_H
#define DS_BITBANG_I2C_H

#include <Arduino.h>
#include <driver/gpio.h>
#include <soc/gpio_struct.h>
#include "esp_cpu.h"     // esp_cpu_get_cycle_count() — 부팅 시 지연 보정
#include "ds_bitbang_core.h"

namespace ds {

class BitBangI2C {
public:
    /**
     * @brief 버스와 핀을 초기화한다
     * @param sdaPins          버스 순서대로의 SDA 핀 번호 (길이 >= busCount)
     * @param sclPin           공유 SCL 핀
     * @param busCount         버스 수 (<= BB_CORE_MAX_BUSES)
     * @param halfBitNs        SCL 반주기 지연 (ns). 0이면 보정하지 않고 최대 속도
     * @param sclSpinMax       SCL 상승을 micros() 없이 다시 읽는 최대 횟수
     * @param stretchTimeoutUs SCL이 이 시간 넘게 LOW면 트랜잭션 실패
     */
    void begin(const uint8_t* sdaPins, uint8_t sclPin, uint8_t busCount, uint16_t halfBitNs,
               uint32_t sclSpinMax, uint32_t stretchTimeoutUs);

    /** @brief 보정된 반주기 지연의 **실측값** (ns) — 시리얼 보고용 */
    uint32_t measuredHalfBitNs() const { return _measuredNs; }

    /**
     * @brief 참여하는 모든 버스에 **같은 길이**의 서로 다른 데이터를 동시에 보낸다
     * @param addr     I2C 7비트 주소
     * @param payloads 버스별 페이로드 포인터 배열. 비참여 버스는 nullptr 가능
     * @param length   페이로드 길이 (전 참여 버스 공통 — 호출부가 맞춘다)
     * @param busMask  참여 버스 마스크 (bit N = 버스 N)
     * @return 1 = 전 참여 버스가 전 구간 ACK, 0 = 실패(호출부가 재시도)
     * @details 페이로드에는 제어바이트(0x00 명령 / 0x40 데이터)가 **포함**되어야 한다.
     *          U8g2의 byte_cb가 넘겨주는 패킷과 같은 형식이다.
     */
    uint8_t writeConcurrent(uint8_t addr, const uint8_t* const* payloads,
                            uint16_t length, uint8_t busMask);

    /** @brief 해당 버스에 주소가 응답하는가 (브링업 진단용) */
    uint8_t probe(uint8_t bus, uint8_t addr);

    /**
     * @brief 버스 자가 복구 — SCL을 펄스해 SDA를 붙잡고 있는 슬레이브를 밀어낸다
     * @details BitBang에서는 stuck-low가 **ACK 성공으로 오독**되므로, 타임아웃으로
     *          이상을 감지한 뒤 반드시 이 함수로 버스를 유휴 상태로 되돌려야 한다.
     */
    void recover();

    uint32_t errorCount() const { return _errors; }

    // ── 전송 통계 (FPS 벤치마크용) ──────────────────────────────────────
    /**
     * @brief writeConcurrent()가 실제로 선 위에 낸 **비트 주기 수**와 그 시간을 누적한다
     * @details 비트 주기 = START 1 + (주소 1 + 페이로드 N) × 9(8비트+ACK) + STOP 1.
     *          동시 전송은 4버스가 같은 클럭을 쓰므로 **버스 수와 무관하게 한 번만** 센다.
     *          → `statBits / statUs` = 실효 SCL 주파수 (바이트 사이 계산 오버헤드 포함).
     */
    void     resetStats() { _statBits = 0; _statUs = 0; _statTx = 0; }
    uint64_t statBits() const { return _statBits; }
    uint64_t statUs()   const { return _statUs; }
    uint32_t statTx()   const { return _statTx; }

private:
    // ── 저수준 핀 조작 ──────────────────────────────────────────────────
    // 모든 SDA/SCL은 Open-Drain: 래치 1 = Hi-Z(외부 풀업이 HIGH로 끌어올림), 0 = LOW로 당김.
    // GPIO_MODE_INPUT_OUTPUT_OD 로 두면 출력과 동시에 입력(ACK 읽기)도 살아 있다.
    //   ⚠ GPIO_MODE_OUTPUT_OD 만 쓰면 입력 버퍼가 꺼져 ACK를 못 읽는다.

    /**
     * @brief 반주기 지연 — 부팅 시 실측 보정한 NOP 루프
     * @details `delayMicroseconds()`는 호출 오버헤드가 커서 1us를 요청해도 실효 1.3us가
     *          된다. 여기서는 CPU 사이클을 한 번 재서 목표 ns에 맞춘 횟수만큼 NOP를 돈다.
     * @note `_delayIters == 0`이면 지연 없이 **최대 속도**로 돈다.
     */
    inline void bitDelay() const {
        for (uint32_t i = 0; i < _delayIters; i++) { __asm__ __volatile__("nop"); }
    }
    inline void sclLow()   const { GPIO.out_w1tc.val = _sclBit; }
    inline void sclHigh()  const { GPIO.out_w1ts.val = _sclBit; }
    inline void sdaDrive(uint32_t ones, uint32_t zeros) const {
        GPIO.out_w1ts.val = ones;
        GPIO.out_w1tc.val = zeros;
    }
    inline void sdaReleaseAll() const { GPIO.out_w1ts.val = _sdaMask; }
    inline uint32_t sdaReadAll() const { return GPIO.in.val & _sdaMask; }

    /**
     * @brief SCL을 HIGH로 놓고 실제로 HIGH가 될 때까지 기다린다
     * @return 0 = 타임아웃 (stretchTimeoutUs 초과)
     * @details 풀업 RC 때문에 SCL 상승은 수백 ns가 걸린다. 먼저 레지스터만 몇 번 다시 읽고,
     *          그래도 LOW일 때만(진짜 스트레칭·stuck) micros() 타임아웃 경로로 간다.
     */
    inline uint8_t sclWaitHigh() const {
        sclHigh();
        for (uint32_t n = 0; n < _spinMax; n++) {
            if (GPIO.in.val & _sclBit) return 1;
        }
        const uint32_t t0 = micros();
        while (!(GPIO.in.val & _sclBit)) {
            if ((uint32_t)(micros() - t0) > _stretchUs) return 0;
        }
        return 1;
    }

    inline uint32_t pinMask(uint8_t busMask) const {
        uint32_t m = 0;
        for (uint8_t b = 0; b < _busCount; b++) {
            if (busMask & (uint8_t)(1u << b)) m |= (uint32_t)1u << _sda[b];
        }
        return m;
    }

    uint8_t  _sda[BB_CORE_MAX_BUSES];
    uint8_t  _busCount   = 0;
    uint8_t  _scl        = 0;
    uint16_t _halfBitNs  = 0;   // 목표 반주기 (ns)
    uint32_t _delayIters = 0;   // 보정된 NOP 루프 반복 횟수 (0 = 지연 없음)
    uint32_t _measuredNs = 0;   // 실측 반주기 (ns) — 보고용
    uint32_t _spinMax    = 32;
    uint32_t _stretchUs  = 3000;
    uint8_t  _lastAck    = 0;
    uint32_t _errors     = 0;
    uint32_t _sdaMask    = 0;
    uint32_t _sclBit     = 0;
    /** 니블 → 비트별 HIGH 핀 마스크 표 (bbBuildNibbleLut, 1KB) */
    uint32_t _nib[BB_CORE_MAX_BUSES][16][4];
    uint64_t _statBits   = 0;   // 누적 비트 주기 수 (FPS 벤치마크)
    uint64_t _statUs     = 0;   // 누적 전송 시간 (us)
    uint32_t _statTx     = 0;   // 누적 트랜잭션 수

    uint8_t writeConcurrentImpl(uint8_t addr, const uint8_t* const* payloads,
                                uint16_t length, uint8_t busMask);
    /** 핫 경로 — 니블 표로 4버스 바이트 1개 + ACK. 전 참여 버스 ACK면 1 */
    inline uint8_t sendByteLut(const uint8_t* bytes, uint32_t pm) __attribute__((always_inline));

    void    calibrateBitDelay(uint16_t targetNs);
    uint8_t start(uint8_t busMask);
    void    stop(uint8_t busMask);
    uint8_t writeBytePerBus(const uint8_t* bytes, uint8_t busMask);
};

}  // namespace ds

#endif
