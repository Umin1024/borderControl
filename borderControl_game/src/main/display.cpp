#include <Arduino.h>
#include "display.h"

static constexpr uint8_t PAGE_TEXT_SIZE = 1;
static constexpr int16_t PAGE_TEXT_X = 5;           // 5px left margin (M mod 6 = 5, avoids DMA dead columns at x=48/96/144)
static constexpr int16_t PAGE_TEXT_Y = 2;           // 2px top margin
static constexpr int16_t PAGE_LINE_HEIGHT = 16;
static constexpr uint8_t PAGE_MAX_LINES = 3;
static constexpr uint8_t PAGE_MAX_COLS = 30;        // floor((192-5-2)/6) = 30
static constexpr uint8_t FIRST_LINE_INDENT = 0;
static constexpr int16_t LEADERBOARD_ROW_H = 11;   // px between leaderboard rows

static String currentPageTag;
static String currentPageLines[PAGE_MAX_LINES];

uint16_t color333(uint8_t r, uint8_t g, uint8_t b) {
    return dma_display->color565(r * 36, g * 36, b * 36);
}

static void clear() {
    dma_display->fillScreen(dma_display->color565(0, 0, 0));
}

static void padPageLine(String& line, uint8_t maxLen) {
    while (line.length() < maxLen) line += ' ';
    if (line.length() > maxLen) line = line.substring(0, maxLen);
}

// firstContentDone: set to true once the first non-empty segment is being laid out.
// The first content line always gets the indent width, even if it's not line index 0
// (e.g. texts that begin with \n have an empty line 0 and content on line 1).
static void appendWrappedSegment(const String& segment, String lines[PAGE_MAX_LINES],
                                  int& lineCount, bool& firstContentDone) {
    if (lineCount >= PAGE_MAX_LINES) return;

    String text = segment;
    text.trim();

    if (text.length() == 0) {
        lines[lineCount++] = "";
        return;
    }

    int start = 0;
    while (start < (int)text.length() && lineCount < PAGE_MAX_LINES) {
        while (start < (int)text.length() && text[start] == ' ') start++;
        if (start >= (int)text.length()) break;

        int colsAvail = !firstContentDone ? (PAGE_MAX_COLS - FIRST_LINE_INDENT) : PAGE_MAX_COLS;
        firstContentDone = true;
        int end = start + colsAvail;

        if (end >= (int)text.length()) {
            lines[lineCount++] = text.substring(start);
            break;
        }

        // If 'end' lands on punctuation followed by a space, keep the punctuation
        // on this line and start the next line after the space.
        int split;
        bool trailingPunct = (text[end] == ',' || text[end] == '.' || text[end] == '!'
                           || text[end] == '?' || text[end] == ';' || text[end] == ':')
                          && (end + 1 >= (int)text.length() || text[end + 1] == ' ');
        if (trailingPunct) {
            split = end + 1; // substring(start, end+1) includes the punctuation
        } else {
            // Find last space at or before end to avoid splitting a word
            split = end;
            while (split > start && text[split] != ' ') split--;
            if (split == start) split = end; // single word longer than line, hard-break
        }

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
    bool firstContentDone = false;

    for (int i = 0; i <= text.length() && lineCount < PAGE_MAX_LINES; i++) {
        if (i == text.length() || text[i] == '\n') {
            appendWrappedSegment(text.substring(segmentStart, i), lines, lineCount, firstContentDone);
            segmentStart = i + 1;
        }
    }

    while (lineCount < PAGE_MAX_LINES) {
        lines[lineCount++] = "";
    }

    // Pad each line; the first content line uses the indented (narrower) width.
    bool firstPadded = false;
    for (int i = 0; i < PAGE_MAX_LINES; i++) {
        bool hasContent = false;
        for (int j = 0; j < (int)lines[i].length(); j++) {
            if (lines[i][j] != ' ') { hasContent = true; break; }
        }
        if (!firstPadded && hasContent) {
            padPageLine(lines[i], PAGE_MAX_COLS - FIRST_LINE_INDENT);
            firstPadded = true;
        } else {
            padPageLine(lines[i], PAGE_MAX_COLS);
        }
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

    bool indented = false;
    for (int i = 0; i < PAGE_MAX_LINES; i++) {
        bool hasContent = false;
        for (int j = 0; j < (int)currentPageLines[i].length(); j++) {
            if (currentPageLines[i][j] != ' ') { hasContent = true; break; }
        }
        int16_t xPos = PAGE_TEXT_X + (!indented && hasContent ? FIRST_LINE_INDENT * 6 : 0);
        if (!indented && hasContent) indented = true;
        dma_display->setCursor(xPos, PAGE_TEXT_Y + i * PAGE_LINE_HEIGHT);
        dma_display->print(currentPageLines[i]);
    }
}


void disp_show_leaderboard(const long scores[], uint8_t count, uint16_t color) {
    currentPageTag = String("LEADERBOARD");
    for (int i = 0; i < PAGE_MAX_LINES; i++) currentPageLines[i] = "";

    clear();
    dma_display->setTextSize(1);
    dma_display->setTextWrap(false);
    dma_display->setTextColor(color);

    // 4 rows in 44px usable height (2px top margin): LEADERBOARD_ROW_H px per row
    int16_t W = dma_display->width();

    // Row 0: title
    int16_t titleLen = (int16_t)strlen(TEXT_LEADERBOARD_PREFIX);
    dma_display->setCursor((W - titleLen * 6) / 2, PAGE_TEXT_Y);
    dma_display->print(TEXT_LEADERBOARD_PREFIX);

    // Rows 1-3: scores
    const char* labels[] = {"1.", "2.", "3."};
    for (uint8_t i = 0; i < count && i < 3; i++) {
        String value = (scores[i] == 0x7FFFFFFF) ? String("xxx") : String(scores[i]) + "g";
        String line = String(labels[i]) + " " + value;
        currentPageLines[i] = line;
        dma_display->setCursor((W - (int16_t)(line.length() * 6)) / 2,
                               PAGE_TEXT_Y + (i + 1) * LEADERBOARD_ROW_H);
        dma_display->print(line);
    }
}

void disp_blink_leaderboard_row(int rank, bool visible, uint16_t color) {
    if (rank < 0 || rank >= PAGE_MAX_LINES) return;
    int16_t y = PAGE_TEXT_Y + (rank + 1) * LEADERBOARD_ROW_H;
    int16_t W = dma_display->width();
    dma_display->fillRect(0, y, W, 8, 0);
    if (visible) {
        dma_display->setTextSize(1);
        dma_display->setTextWrap(false);
        dma_display->setTextColor(color);
        const String& line = currentPageLines[rank];
        dma_display->setCursor((W - (int16_t)(line.length() * 6)) / 2, y);
        dma_display->print(line);
    }
}

void disp_draw_corner_label(const String& label, uint16_t color, int16_t yFromBottom) {
    int16_t textW = label.length() * 6;
    int16_t x = dma_display->width() - textW - 2;
    int16_t y = dma_display->height() - yFromBottom;
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
    printf("+---------------------------------+\n");
    for (int i = 0; i < PAGE_MAX_LINES; i++) {
        if (i == 0)
            printf("|  %s|\n", currentPageLines[i].c_str());
        else
            printf("|%s|\n", currentPageLines[i].c_str());
    }
    printf("+---------------------------------+\n");
}
