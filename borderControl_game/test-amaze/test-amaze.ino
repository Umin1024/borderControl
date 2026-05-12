#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
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

    pinMode(PIN_RED, OUTPUT);
    pinMode(PIN_GREEN, OUTPUT);
    digitalWrite(PIN_RED, LOW);
    digitalWrite(PIN_GREEN, LOW);

    displayPage(PAGE_START);
    delay(3000);
    HX711_CH0.begin();
    delay(3000);
    HX711_CH0.begin(); // re-tare after sensor warm-up
    lastPageMillis = millis();
}

void loop()
{
    if (millis() - lastPageMillis >= PAGE_AUTO_INTERVAL_MS)
    {
        lastPageMillis = millis();
        currentPage++;
        if (currentPage >= TOTAL_PAGES)
            currentPage = PAGE_START;
        displayPage(currentPage);
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
        disp_start_screen();
        return;
    }

    // INTRO pages
    if (pageNum >= PAGE_INTRO_BASE && pageNum < PAGE_LEVEL_BASE)
    {
        disp_intro(pageNum - PAGE_INTRO_BASE);
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
            disp_level_result(current_score);
        }
        else
        {
            if (levelIdx == 0)
            {
                initialWeight = Weight; // baseline: player is on scale with all luggage
                lastDisplayedWeight = Weight;
            }
            disp_level_prompt(q.itemName);
        }
        return;
    }

    // FINAL
    if (pageNum == PAGE_FINAL)
    {
        disp_final(player_score);
        digitalWrite(PIN_RED, HIGH);
        digitalWrite(PIN_GREEN, HIGH);
        return;
    }

    // LEADERBOARD
    if (pageNum == PAGE_LEADERBOARD)
    {
        updateLeaderboard(player_score);
        disp_leaderboard(leaderboard, LEADERBOARD_SIZE);
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
