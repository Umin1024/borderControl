#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <stdio.h>
#include "HX711.h"
#include "config.h"
#include "game_content.h"
#include "display.h"

HX711 HX711_CH0(HX711_SCK, HX711_DT, HX711_GAP);
MatrixPanel_I2S_DMA *dma_display = nullptr;

// --- Game state ---
unsigned long Weight = 0;
long debugWeight = 0; // raw signed reading for debug overlay
unsigned long lastWeightMillis = 0;
unsigned long lastPageMillis = 0;
unsigned long lastDisplayedWeight = 0xFFFFFFFFUL;
unsigned long initialWeight = 0;
long player_score = 0;
long current_score = 0;
long leaderboard[LEADERBOARD_SIZE];
uint8_t currentPage = PAGE_START;
uint8_t selectedQuestions[NUM_LEVELS]; // which of the 3 variants is active this run

// --- Forward declarations ---
void displayPage(uint8_t pageNum);
void resetGame();
void calcScore(unsigned long itemWeight);
void updateLeaderboard(long score);
String pageLabel(uint8_t pageNum);
String pageText(uint8_t pageNum);
uint16_t pageColor(uint8_t pageNum);

// ============================================================

void setup()
{
    for (int i = 0; i < LEADERBOARD_SIZE; i++)
        leaderboard[i] = 0x7FFFFFFF;

    HUB75_I2S_CFG mxconfig;
    mxconfig.mx_height = PANEL_RES_Y;
    mxconfig.mx_width = PANEL_RES_X;
    mxconfig.chain_length = PANEL_CHAIN;
    mxconfig.gpio.e = PIN_E;
    mxconfig.min_refresh_rate = 30;

    dma_display = new MatrixPanel_I2S_DMA(mxconfig);
    dma_display->begin();
    dma_display->setBrightness8(255); // （10/256）
    dma_display->fillScreen(dma_display->color565(0, 0, 0));
    dma_display->setTextWrap(false);

    Serial.begin(115200);
    printf("\n[stdio] ready @115200\n");
    printf("[stdio] page preview prints on page change\n");
    printf("[stdio] send 'p' in serial monitor to print the current page\n");

    pinMode(PIN_RED, OUTPUT);
    pinMode(PIN_GREEN, OUTPUT);
    digitalWrite(PIN_RED, LOW);
    digitalWrite(PIN_GREEN, LOW);

    displayPage(PAGE_START);
    disp_print_serial_preview();
    delay(3000);
    HX711_CH0.begin();
    delay(3000);
    HX711_CH0.begin(); // re-tare after sensor warm-up
    printf("[stdio] sensors ready, reprinting current page\n");
    disp_print_serial_preview();
    lastPageMillis = millis();
}

void loop()
{
    while (Serial.available() > 0)
    {
        char command = (char)Serial.read();
        if (command == 'p' || command == 'P')
        {
            printf("[stdio] manual page preview\n");
            disp_print_serial_preview();
        }
    }

    if (millis() - lastPageMillis >= PAGE_AUTO_INTERVAL_MS)
    {
        lastPageMillis = millis();
        currentPage++;
        if (currentPage >= TOTAL_PAGES)
            currentPage = PAGE_START;
        displayPage(currentPage);
        disp_print_serial_preview();
        disp_debug_weight(debugWeight); // redraw overlay after page clear
    }

    if (millis() - lastWeightMillis >= WEIGHT_INTERVAL_MS)
    {
        lastWeightMillis = millis();
        long raw = (long)HX711_CH0.Get_Weight();
        debugWeight = raw;
        unsigned long measured = (raw > 0) ? (unsigned long)raw : 0;
        if (measured < DISPLAY_DEADZONE)
            measured = 0;
        if (measured != lastDisplayedWeight)
        {
            lastDisplayedWeight = measured;
            Weight = measured;
        }
        disp_debug_weight(debugWeight);
    }

    delay(20);
}

// ============================================================
// Page router
// ============================================================

void displayPage(uint8_t pageNum)
{
    // START
    if (pageNum == PAGE_START)
    {
        resetGame();
        disp_show_page(pageLabel(pageNum), pageText(pageNum), pageColor(pageNum));
        return;
    }

    // INTRO pages
    if (pageNum >= PAGE_INTRO_BASE && pageNum < PAGE_LEVEL_BASE)
    {
        disp_show_page(pageLabel(pageNum), pageText(pageNum), pageColor(pageNum));
        return;
    }

    // LEVEL pages  (pairs: prompt / result)
    if (pageNum >= PAGE_LEVEL_BASE && pageNum < PAGE_FINAL)
    {
        uint8_t levelPage = pageNum - PAGE_LEVEL_BASE;
        uint8_t levelIdx = levelPage / 2;
        bool isResult = (levelPage % 2 == 1);
        const Question &q = LEVELS[levelIdx].questions[selectedQuestions[levelIdx]];

        if (isResult)
        {
            calcScore(q.weightGrams);
        }
        else if (levelIdx == 0)
        {
            initialWeight = Weight; // baseline: player is on scale with all luggage
            lastDisplayedWeight = Weight;
        }

        disp_show_page(pageLabel(pageNum), pageText(pageNum), pageColor(pageNum));
        return;
    }

    // FINAL
    if (pageNum == PAGE_FINAL)
    {
        disp_show_page(pageLabel(pageNum), pageText(pageNum), pageColor(pageNum));
        digitalWrite(PIN_RED, HIGH);
        digitalWrite(PIN_GREEN, HIGH);
        return;
    }

    // LEADERBOARD
    if (pageNum == PAGE_LEADERBOARD)
    {
        updateLeaderboard(player_score);
        disp_show_page(pageLabel(pageNum), pageText(pageNum), pageColor(pageNum));
        return;
    }
}

// ============================================================
// Game logic
// ============================================================

void resetGame()
{
    player_score = 0;
    current_score = 0;
    Weight = 0;
    initialWeight = 0;
    digitalWrite(PIN_RED, LOW);
    digitalWrite(PIN_GREEN, LOW);

    // Pick one random question variant per level for this game run.
    randomSeed(millis());
    for (uint8_t i = 0; i < NUM_LEVELS; i++)
    {
        selectedQuestions[i] = random(QUESTIONS_PER_LEVEL);
    }
}

// Calculate how far off the player was and accumulate to player_score.
void calcScore(unsigned long itemWeight)
{
    unsigned long diff = (Weight >= initialWeight)
                             ? (Weight - initialWeight)
                             : (initialWeight - Weight);
    long error = (long)diff - (long)itemWeight;
    if (error < 0)
        error = -error;
    current_score = error;
    player_score += current_score;
    initialWeight = Weight; // new baseline for next level
}

void updateLeaderboard(long score)
{
    if (score <= 0)
        return;
    if (score >= leaderboard[LEADERBOARD_SIZE - 1])
        return;
    leaderboard[LEADERBOARD_SIZE - 1] = score;
    // bubble up
    for (int i = LEADERBOARD_SIZE - 1; i > 0; i--)
    {
        if (leaderboard[i] < leaderboard[i - 1])
        {
            long tmp = leaderboard[i - 1];
            leaderboard[i - 1] = leaderboard[i];
            leaderboard[i] = tmp;
        }
    }
}

String pageLabel(uint8_t pageNum)
{
    if (pageNum == PAGE_START)
        return String("START");

    if (pageNum >= PAGE_INTRO_BASE && pageNum < PAGE_LEVEL_BASE)
    {
        return String("INTRO ") + String(pageNum - PAGE_INTRO_BASE + 1);
    }

    if (pageNum >= PAGE_LEVEL_BASE && pageNum < PAGE_FINAL)
    {
        uint8_t levelPage = pageNum - PAGE_LEVEL_BASE;
        uint8_t levelIdx = levelPage / 2;
        bool isResult = (levelPage % 2 == 1);
        return String("LEVEL ") + String(levelIdx + 1) + (isResult ? " RESULT" : " PROMPT");
    }

    if (pageNum == PAGE_FINAL)
        return String("FINAL");

    if (pageNum == PAGE_LEADERBOARD)
        return String("LEADERBOARD");

    return String("UNKNOWN");
}

uint16_t pageColor(uint8_t pageNum)
{
    if (pageNum == PAGE_FINAL)
        return color333(0, 7, 0);

    if (pageNum == PAGE_LEADERBOARD)
        return color333(7, 7, 0);

    return color333(7, 0, 0);
}

String pageText(uint8_t pageNum)
{
    if (pageNum == PAGE_START)
        return String(TEXT_START);

    if (pageNum >= PAGE_INTRO_BASE && pageNum < PAGE_LEVEL_BASE)
    {
        return String(INTRO_TEXTS[pageNum - PAGE_INTRO_BASE]);
    }

    if (pageNum >= PAGE_LEVEL_BASE && pageNum < PAGE_FINAL)
    {
        uint8_t levelPage = pageNum - PAGE_LEVEL_BASE;
        uint8_t levelIdx = levelPage / 2;
        bool isResult = (levelPage % 2 == 1);
        const Question &q = LEVELS[levelIdx].questions[selectedQuestions[levelIdx]];

        if (isResult)
            return String(TEXT_RESULT_PREFIX) + " " + String(current_score) + "g";

        return String(TEXT_PROMPT_PREFIX) + " " + String(q.itemName);
    }

    if (pageNum == PAGE_FINAL)
        return String(TEXT_FINAL_PREFIX) + " " + String(player_score) + "g";

    if (pageNum == PAGE_LEADERBOARD)
    {
        String text = String(TEXT_LEADERBOARD_PREFIX);
        for (uint8_t i = 0; i < LEADERBOARD_SIZE; i++)
        {
            text += (leaderboard[i] == 0x7FFFFFFF) ? "--" : String(leaderboard[i]) + "g";
            if (i < LEADERBOARD_SIZE - 1)
                text += " ";
        }
        return text;
    }

    return String();
}

