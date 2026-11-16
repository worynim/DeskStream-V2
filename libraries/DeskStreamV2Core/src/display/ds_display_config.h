// worynim@gmail.com
/**
 * @file ds_display_config.h
 * @brief 화면 엔진 런타임 설정 (DisplayCore::begin()에 넘긴다)
 * @details 라이브러리는 스케치의 config.h를 볼 수 없으므로 프로젝트별 값은 이 구조체로 받는다.
 *          BitBang 타이밍 기본값은 Hangeul_Clock_bitbang_i2c v1.1.2 실측 채택값이다
 *          (SH1106 4화면 풀 프레임 10.3ms / 97.3fps, 실효 SCL 1,170kHz, 오류 0).
 */
#ifndef DS_DISPLAY_CONFIG_H
#define DS_DISPLAY_CONFIG_H

#include <U8g2lib.h>

namespace ds {

struct DisplayConfig {
    /** 화면 회전. U8G2_R2 + setFlip(false)가 이 보드의 정방향이다 (DisplayCore::setFlip 주석 참조) */
    const u8g2_cb_t* rotation = U8G2_R2;

    /**
     * 열 주소 보정값. -1 = U8g2 기본값(SH1106 = 2)을 그대로 쓴다.
     * @details SH1106은 132열 RAM이라 가시 영역이 2열부터 시작한다. 1.3" 모듈 중에는 컨트롤러가
     *          SSD1306/SSD1315인 것도 있는데(겉모습·주소가 같다), 그때는 화면이 2픽셀 밀리므로 0을 준다.
     */
    int16_t colOffset = -1;

    /**
     * SCL **반주기** 지연 (ns). 0 = 지연 없음. 실효 비트 주기 ≈ 650ns + 2 × 이 값.
     * @details SH1106 실측 (Hangeul_Clock_bitbang_i2c):
     *            150 → 실효 1,170kHz, 오류 0 ← 기본값
     *            250 → 848kHz, 오류 0
     *            0   → 약 1.5MHz, ✗ 오류·노이즈·한 화면 밝기 이상
     *          화면이 깨지거나 오류가 보이면 키운다.
     */
    uint32_t halfBitNs = 150;
    uint32_t sclSpinMax = 32;            ///< SCL 상승을 레지스터로 다시 읽는 횟수 (넘으면 micros() 경로)
    uint32_t stretchTimeoutUs = 3000;    ///< SCL이 이 시간 넘게 LOW면 트랜잭션 실패 (stuck)
    uint32_t errorThreshold = 50;        ///< 누적 오류가 이를 넘으면 버스 자가 복구
};

}  // namespace ds

#endif
