// worynim@gmail.com
/**
 * @file ds_panel_type.h
 * @brief OLED 컨트롤러 종류 (SH1106 / SSD1306) — 보드 전체 설정
 * @details V2 보드에는 1.3" SH1106 모듈과 V1의 SSD1306 모듈을 끼울 수 있다. 두 컨트롤러는 열 주소만 다르다
 *          (SH1106은 132열 RAM이라 보이는 영역이 2열부터). 소프트웨어로 구별할 수 없어 사용자가 고른다:
 *          **BTN2를 누른 채 리셋하고 2초 유지**하면 종류가 바뀌어 NVS에 저장되고, 이후 모든 앱과 런처가 그 값을 쓴다.
 *          저장 위치는 NVS 이름공간 `ds_board`, 키 `oled`(u8: 0 = SH1106, 1 = SSD1306) — 런처(esp-launcher)도 같은 키를 읽는다.
 *          값이 없거나 모르는 값이면 SH1106.
 *
 *          순수 부분(값 변환)은 호스트 테스트 대상이고, NVS·버튼 함수는 ds_panel_type.cpp(Arduino)에 있다.
 */
#ifndef DS_PANEL_TYPE_H
#define DS_PANEL_TYPE_H

#include <stdint.h>

namespace ds {

enum class PanelType : uint8_t {
    kSH1106 = 0,
    kSSD1306 = 1,
};

constexpr const char* kPanelNvsNamespace = "ds_board";   ///< 런처와 같이 쓰는 NVS 이름공간
constexpr const char* kPanelNvsKey = "oled";
constexpr uint32_t kPanelToggleHoldMs = 2000;            ///< BTN2를 이만큼 누르고 있어야 바뀐다

/** NVS에 저장된 값 → 종류 (모르는 값은 SH1106) */
inline PanelType panelTypeFromStored(uint8_t v) {
    return v == (uint8_t)PanelType::kSSD1306 ? PanelType::kSSD1306 : PanelType::kSH1106;
}

inline PanelType otherPanelType(PanelType t) {
    return t == PanelType::kSH1106 ? PanelType::kSSD1306 : PanelType::kSH1106;
}

/** 열 주소 보정값 (U8g2 x_offset) */
inline uint8_t panelColOffset(PanelType t) {
    return t == PanelType::kSSD1306 ? 0 : 2;
}

inline const char* panelTypeName(PanelType t) {
    return t == PanelType::kSSD1306 ? "SSD1306" : "SH1106";
}

/** NVS에서 읽는다. 저장된 값이 없으면 SH1106 */
PanelType loadPanelType();

/** NVS에 저장한다. 실패하면 false */
bool savePanelType(PanelType t);

/**
 * 전원·RESET 버튼으로 켜졌고 BTN2가 holdMs 동안 계속 눌려 있으면 true.
 * 딥슬립 깨우기(Page_Turner는 BTN2로 깨운다)·소프트웨어 재시작 등 다른 리셋은 false.
 * 눌려 있지 않으면 바로 돌아온다.
 */
bool panelToggleRequested(uint32_t holdMs = kPanelToggleHoldMs);

}  // namespace ds

#endif
