#include <Arduino.h>
#include "display.h"

static constexpr uint8_t PAGE_TEXT_SIZE = 1;
static constexpr int16_t PAGE_TEXT_X = 0;
static constexpr int16_t PAGE_TEXT_Y = 0;
static constexpr int16_t PAGE_LINE_HEIGHT = 16;
static constexpr uint8_t PAGE_MAX_LINES = 3;
static constexpr uint8_t PAGE_MAX_COLS = 16;

static String currentPageTag;
static String currentPageLines[PAGE_MAX_LINES];

uint16_t color333(uint8_t r, uint8_t g, uint8_t b) {
    return dma_display->color565(r * 36, g * 36, b * 36);
}

static void clear() {
    dma_display->fillScreen(dma_display->color565(0, 0, 0));
}

static void padPageLine(String& line) {
    while (line.length() < PAGE_MAX_COLS) line += ' ';
    if (line.length() > PAGE_MAX_COLS) line = line.substring(0, PAGE_MAX_COLS);
}

static void appendWrappedSegment(const String& segment, String lines[PAGE_MAX_LINES], int& lineCount) {
    if (lineCount >= PAGE_MAX_LINES) return;

    String text = segment;
    text.trim();

    if (text.length() == 0) {
        lines[lineCount++] = "";
        return;
    }

    int start = 0;
    while (start < text.length() && lineCount < PAGE_MAX_LINES) {
        while (start < text.length() && text[start] == ' ') start++;
        if (start >= text.length()) break;

        int end = start + PAGE_MAX_COLS;
        if (end >= text.length()) {
            lines[lineCount++] = text.substring(start);
            break;
        }

        int split = end;
        while (split > start && text[split] != ' ') split--;
        if (split == start) split = end;

        lines[lineCount++] = text.substring(start, split);
        start = split;
    }
}

void disp_split_page_text(const String& rawText, String lines[3]) {
    for (int i = 0; i < PAGE_MAX_LINES; i++) lines[i] = "";

    String text = rawText;
    text.replace("\r", "");

    int lineCount = 0;
    int segmentStart = 0;

    for (int i = 0; i <= text.length() && lineCount < PAGE_MAX_LINES; i++) {
        if (i == text.length() || text[i] == '\n') {
            appendWrappedSegment(text.substring(segmentStart, i), lines, lineCount);
            segmentStart = i + 1;
        }
    }

    while (lineCount < PAGE_MAX_LINES) {
        lines[lineCount++] = "";
    }

    for (int i = 0; i < PAGE_MAX_LINES; i++) {
        padPageLine(lines[i]);
    }
}

static void textSetup(uint16_t color, uint8_t size = PAGE_TEXT_SIZE) {
    dma_display->setTextSize(size);
    dma_display->setTextWrap(false);
    dma_display->setTextColor(color);
}

void disp_show_page(const String& pageTag, const String& pageText, uint16_t color) {
    disp_split_page_text(pageText, currentPageLines);
    currentPageTag = pageTag;

    clear();
    textSetup(color);

    for (int i = 0; i < PAGE_MAX_LINES; i++) {
        dma_display->setCursor(PAGE_TEXT_X, PAGE_TEXT_Y + i * PAGE_LINE_HEIGHT);
        dma_display->print(currentPageLines[i]);
    }
}

void disp_show_leaderboard(const long scores[], uint8_t count, uint16_t color) {
    currentPageTag = String("LEADERBOARD");
    for (int i = 0; i < PAGE_MAX_LINES; i++) currentPageLines[i] = "";

    clear();
    textSetup(color);

    dma_display->setTextSize(3);
    dma_display->setTextWrap(false);
    dma_display->setTextColor(color);
    dma_display->setCursor(0, 8);
    dma_display->print("BEST");

    dma_display->setTextSize(1);
    dma_display->setTextColor(color);

    const char* labels[] = {"1.", "2.", "3."};
    int16_t x = 42;
    int16_t y = 4;
    for (uint8_t i = 0; i < count && i < 3; i++) {
        String value = (scores[i] == 0x7FFFFFFF) ? String("xxx") : String(scores[i]) + "g";
        currentPageLines[i] = String(labels[i]) + " " + value;
        dma_display->setCursor(x, y + i * 14);
        dma_display->print(labels[i]);
        dma_display->print(" ");
        dma_display->print(value);
    }
}

void disp_draw_corner_label(const String& label, uint16_t color) {
    int16_t textW = label.length() * 6;
    int16_t x = dma_display->width() - textW - 2;
    int16_t y = dma_display->height() - 8;
    dma_display->fillRect(x - 1, y, textW + 2, 8, dma_display->color565(0, 0, 0));
    dma_display->setTextSize(1);
    dma_display->setTextWrap(false);
    dma_display->setCursor(x, y);
    dma_display->setTextColor(color);
    dma_display->print(label);
}

void disp_debug_weight(long rawWeight) {
    disp_draw_corner_label(String(rawWeight) + "g", color333(0, 4, 4));
}

void disp_print_serial_preview() {
    printf("\n================ PAGE CHANGE ================\n");
    printf("PAGE: %s\n", currentPageTag.c_str());
    printf("+----------------+\n");
    for (int i = 0; i < PAGE_MAX_LINES; i++) {
        printf("|%s|\n", currentPageLines[i].c_str());
    }
    printf("+----------------+\n");
}
