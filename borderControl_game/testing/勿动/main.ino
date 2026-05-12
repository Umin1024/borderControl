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
unsigned long frozenWeight = 0;
unsigned long leaderboardPageShownMillis = 0;
bool startPlayerArmed = false;
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
bool playerPassed();
unsigned long pageDuration(uint8_t pageNum);
bool isPromptPage(uint8_t pageNum);
bool isResultPage(uint8_t pageNum);
void updatePageProgress();
void updatePageOverlay();

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
    }

    if (currentPage == PAGE_START)
    {
        if (Weight < PLAYER_WEIGHT_THRESHOLD_G)
            startPlayerArmed = true;
        if (startPlayerArmed && Weight >= PLAYER_WEIGHT_THRESHOLD_G)
        {
            displayPage(PAGE_INTRO_BASE);
            disp_print_serial_preview();
            delay(20);
            return;
        }
    }

    updatePageProgress();
    updatePageOverlay();

    delay(20);
}

// ============================================================
// Page router
// ============================================================

void displayPage(uint8_t pageNum)
{
    digitalWrite(PIN_RED, LOW);
    digitalWrite(PIN_GREEN, LOW);

    currentPage = pageNum;
    lastPageMillis = millis();

    // START
    if (pageNum == PAGE_START)
    {
        resetGame();
        startPlayerArmed = false;
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
            frozenWeight = Weight;
            calcScore(q.weightGrams);
        }
        else if (levelIdx == 0)
        {
            initialWeight = Weight;
        }

        disp_show_page(pageLabel(pageNum), pageText(pageNum), pageColor(pageNum));
        return;
    }

    // FINAL
    if (pageNum == PAGE_FINAL)
    {
        disp_show_page(pageLabel(pageNum), pageText(pageNum), pageColor(pageNum));
        return;
    }

    // ACCESS RESULT
    if (pageNum == PAGE_ACCESS_RESULT)
    {
        disp_show_page(pageLabel(pageNum), pageText(pageNum), pageColor(pageNum));
        if (playerPassed())
            digitalWrite(PIN_GREEN, HIGH);
        else
            digitalWrite(PIN_RED, HIGH);
        return;
    }

    // LEADERBOARD
    if (pageNum == PAGE_LEADERBOARD)
    {
        updateLeaderboard(player_score);
        leaderboardPageShownMillis = millis();
        disp_show_leaderboard(leaderboard, LEADERBOARD_SIZE, pageColor(pageNum));
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
    frozenWeight = 0;
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
    unsigned long diff = (frozenWeight >= initialWeight)
                             ? (frozenWeight - initialWeight)
                             : (initialWeight - frozenWeight);
    long error = (long)diff - (long)itemWeight;
    if (error < 0)
        error = -error;
    current_score = error;
    player_score += current_score;
    initialWeight = frozenWeight;
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

    if (pageNum == PAGE_ACCESS_RESULT)
        return String("ACCESS RESULT");

    if (pageNum == PAGE_LEADERBOARD)
        return String("LEADERBOARD");

    return String("UNKNOWN");
}

uint16_t pageColor(uint8_t pageNum)
{
    if (pageNum == PAGE_FINAL)
        return color333(0, 7, 0);

    if (pageNum == PAGE_ACCESS_RESULT)
        return playerPassed() ? color333(0, 7, 0) : color333(7, 0, 0);

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

    if (pageNum == PAGE_ACCESS_RESULT)
        return playerPassed() ? String("Access Granted.") : String("Access Denied.");

    if (pageNum == PAGE_LEADERBOARD)
    {
        String text = String(TEXT_LEADERBOARD_PREFIX);
        for (uint8_t i = 0; i < LEADERBOARD_SIZE; i++)
        {
            text += (leaderboard[i] == 0x7FFFFFFF) ? "xxx" : String(leaderboard[i]) + "g";
            if (i < LEADERBOARD_SIZE - 1)
                text += " ";
        }
        return text;
    }

    return String();
}

bool playerPassed()
{
    return player_score < PASS_THRESHOLD_G;
}

unsigned long pageDuration(uint8_t pageNum)
{
    if (pageNum >= PAGE_INTRO_BASE && pageNum < PAGE_LEVEL_BASE)
        return INTRO_PAGE_MS;
    if (isPromptPage(pageNum))
        return PROMPT_PAGE_MS;
    if (isResultPage(pageNum))
        return RESULT_PAGE_MS;
    if (pageNum == PAGE_FINAL)
        return FINAL_PAGE_MS;
    if (pageNum == PAGE_ACCESS_RESULT)
        return ACCESS_RESULT_PAGE_MS;
    return 0;
}

bool isPromptPage(uint8_t pageNum)
{
    if (pageNum < PAGE_LEVEL_BASE || pageNum >= PAGE_FINAL)
        return false;
    return ((pageNum - PAGE_LEVEL_BASE) % 2) == 0;
}

bool isResultPage(uint8_t pageNum)
{
    if (pageNum < PAGE_LEVEL_BASE || pageNum >= PAGE_FINAL)
        return false;
    return ((pageNum - PAGE_LEVEL_BASE) % 2) == 1;
}

void updatePageProgress()
{
    if (currentPage == PAGE_START)
        return;

    if (currentPage == PAGE_LEADERBOARD)
    {
        if (millis() - leaderboardPageShownMillis >= LEADERBOARD_PAGE_MS && Weight < PLAYER_WEIGHT_THRESHOLD_G)
        {
            displayPage(PAGE_START);
            disp_print_serial_preview();
        }
        return;
    }

    unsigned long duration = pageDuration(currentPage);
    if (duration == 0)
        return;

    if (millis() - lastPageMillis >= duration)
    {
        uint8_t nextPage = currentPage + 1;
        if (nextPage >= TOTAL_PAGES)
            nextPage = PAGE_START;
        displayPage(nextPage);
        disp_print_serial_preview();
    }
}

void updatePageOverlay()
{
    if (isPromptPage(currentPage))
    {
        unsigned long elapsed = millis() - lastPageMillis;
        unsigned long remainingMs = (elapsed >= PROMPT_PAGE_MS) ? 0 : (PROMPT_PAGE_MS - elapsed);
        unsigned long remainingSeconds = (remainingMs + 999) / 1000;
        disp_draw_corner_label(String(remainingSeconds) + "s", color333(7, 7, 7));
        return;
    }

    if (isResultPage(currentPage))
    {
        disp_draw_corner_label(String(frozenWeight) + "g", color333(0, 4, 4));
        return;
    }
}

