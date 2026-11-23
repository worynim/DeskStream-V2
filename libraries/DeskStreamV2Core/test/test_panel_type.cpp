// worynim@gmail.com
/**
 * @file test_panel_type.cpp
 * @brief OLED 종류 값 변환 테스트 (ds_panel_type.h 순수 부분)
 * @details 실행: bash test/run_all.sh
 *          NVS 값은 런처(esp-launcher)도 읽으므로 0 = SH1106, 1 = SSD1306 대응이 바뀌면 안 된다.
 */
#include <cstdio>
#include <cstring>
#include "display/ds_panel_type.h"

using namespace ds;

static int g_pass = 0;
static int g_fail = 0;

static void expect(const char* label, bool ok) {
    if (ok) {
        g_pass++;
    } else {
        g_fail++;
        printf("  FAIL  %s\n", label);
    }
}

int main() {
    expect("저장값 0 = SH1106", panelTypeFromStored(0) == PanelType::kSH1106);
    expect("저장값 1 = SSD1306", panelTypeFromStored(1) == PanelType::kSSD1306);
    expect("모르는 값 2 = SH1106", panelTypeFromStored(2) == PanelType::kSH1106);
    expect("지운 플래시 값 0xFF = SH1106", panelTypeFromStored(0xFF) == PanelType::kSH1106);

    expect("SH1106 → SSD1306", otherPanelType(PanelType::kSH1106) == PanelType::kSSD1306);
    expect("SSD1306 → SH1106", otherPanelType(PanelType::kSSD1306) == PanelType::kSH1106);

    expect("SH1106 열 보정 2", panelColOffset(PanelType::kSH1106) == 2);
    expect("SSD1306 열 보정 0", panelColOffset(PanelType::kSSD1306) == 0);

    expect("이름 SH1106", strcmp(panelTypeName(PanelType::kSH1106), "SH1106") == 0);
    expect("이름 SSD1306", strcmp(panelTypeName(PanelType::kSSD1306), "SSD1306") == 0);
    expect("NVS 키 (런처와 같아야 함)",
           strcmp(kPanelNvsNamespace, "ds_board") == 0 && strcmp(kPanelNvsKey, "oled") == 0);

    printf("test_panel_type: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
