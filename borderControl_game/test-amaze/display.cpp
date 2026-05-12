#include <Arduino.h>
#include "display.h"

// --- Helpers ---

uint16_t color333(uint8_t r, uint8_t g, uint8_t b) {
    return dma_display->color565(r * 36, g * 36, b * 36);
}

static void clear() {
    dma_display->fillScreen(dma_display->color565(0, 0, 0));
}

// Default text setup used by most pages.
// x starts at 2 to skip the leftmost dead pixel column.
static void textSetup(uint16_t color, uint8_t size = 2, int16_t x = 2, int16_t y = 0) {
    dma_display->setTextSize(size);
    dma_display->setTextWrap(true);
    dma_display->setCursor(x, y);
    dma_display->setTextColor(color);
}

// --- Page templates ---

void disp_start_screen() {
    clear();
    textSetup(color333(7, 0, 0), 2, 20, 16);
    dma_display->println(TEXT_START);
}

void disp_intro(uint8_t introIdx) {
    clear();
    textSetup(color333(7, 0, 0));
    dma_display->println(INTRO_TEXTS[introIdx]);
}

void disp_level_prompt(const char* itemName) {
    clear();
    textSetup(color333(7, 0, 0));
    dma_display->print(TEXT_PROMPT_PREFIX);
    dma_display->println(itemName);
}

void disp_level_result(long errorGrams) {
    clear();
    textSetup(color333(7, 0, 0));
    dma_display->println(String(TEXT_RESULT_PREFIX) + String(errorGrams) + "g");
}

void disp_final(long totalErrorGrams) {
    clear();
    textSetup(color333(0, 7, 0));
    dma_display->println(String(TEXT_FINAL_PREFIX) + String(totalErrorGrams) + "g");
}

void disp_leaderboard(long scores[], uint8_t count) {
    clear();
    textSetup(color333(7, 7, 0));
    String text = String(TEXT_LEADERBOARD_PREFIX);
    for (uint8_t i = 0; i < count; i++) {
        text += (scores[i] == 0x7FFFFFFF) ? "--" : String(scores[i]) + "g";
        if (i < count - 1) text += " ";
    }
    dma_display->println(text);
}

// Debug overlay: raw weight in bottom-right corner, size-1 font.
// Draws over whatever is currently on screen; call after any page draw.
void disp_debug_weight(long rawWeight) {
    String label = String(rawWeight) + "g";
    // Size-1 font: 6px wide × 8px tall per character.
    int16_t textW = label.length() * 6;
    int16_t x     = dma_display->width()  - textW - 2;
    int16_t y     = dma_display->height() - 8;
    // Clear just the overlay area first.
    dma_display->fillRect(x - 1, y, textW + 2, 8, dma_display->color565(0, 0, 0));
    dma_display->setTextSize(1);
    dma_display->setTextWrap(false);
    dma_display->setCursor(x, y);
    dma_display->setTextColor(color333(0, 4, 4));
    dma_display->print(label);
}
