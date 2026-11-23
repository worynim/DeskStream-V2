// worynim@gmail.com
/**
 * @file ds_panel_type.cpp
 * @brief OLED 컨트롤러 종류 — NVS 저장과 BTN2 리셋 판정 (Arduino)
 */
#include "ds_panel_type.h"

#include <Arduino.h>
#include <Preferences.h>
#include <esp_system.h>
#include "../ds_board.h"

namespace ds {

PanelType loadPanelType() {
    Preferences prefs;
    if (!prefs.begin(kPanelNvsNamespace, true)) return PanelType::kSH1106;   // 이름공간이 아직 없음
    const uint8_t v = prefs.getUChar(kPanelNvsKey, (uint8_t)PanelType::kSH1106);
    prefs.end();
    return panelTypeFromStored(v);
}

bool savePanelType(PanelType t) {
    Preferences prefs;
    if (!prefs.begin(kPanelNvsNamespace, false)) return false;
    const bool ok = prefs.putUChar(kPanelNvsKey, (uint8_t)t) == 1;
    prefs.end();
    return ok;
}

bool panelToggleRequested(uint32_t holdMs) {
    const esp_reset_reason_t why = esp_reset_reason();
    if (why != ESP_RST_POWERON && why != ESP_RST_EXT) return false;

    const uint8_t pin = kButtonPins[1];   // BTN2
    pinMode(pin, INPUT_PULLUP);
    const uint32_t start = millis();
    while (digitalRead(pin) == LOW) {
        if (millis() - start >= holdMs) return true;
        delay(10);
    }
    return false;
}

}  // namespace ds
