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
int8_t playerLeaderboardRank = -1;  // index in leaderboard[], -1 if not ranked
uint8_t currentPage = PAGE_START;
uint8_t selectedQuestions[NUM_LEVELS]; // which of the 3 variants is active this run

// --- Forward declarations ---
void displayPage(uint8_t pageNum);
void resetGame();
void calcScore(unsigned long itemWeight);
void updateLeaderboard(long score);
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
#if TEST_MODE
        if (command == 'n' || command == 'N')
        {
            uint8_t nextPage = currentPage + 1;
            if (nextPage >= TOTAL_PAGES) nextPage = PAGE_START;
            printf("[TEST] advancing to page %d\n", nextPage);
            displayPage(nextPage);
            disp_print_serial_preview();
        }
#endif
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
#if TEST_MODE
        displayPage(PAGE_INTRO_BASE);
        disp_print_serial_preview();
        delay(20);
        return;
#else
        if (Weight < PLAYER_WEIGHT_THRESHOLD_G)
            startPlayerArmed = true;
        if (startPlayerArmed && Weight >= PLAYER_WEIGHT_THRESHOLD_G)
        {
            displayPage(PAGE_INTRO_BASE);
            disp_print_serial_preview();
            delay(20);
            return;
        }
#endif
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
        disp_show_page(pageLabel(pageNum), pageText(pageNum, current_score, player_score, selectedQuestions, playerPassed()), pageColor(pageNum, playerPassed()));
        return;
    }

    // INTRO pages
    if (pageNum >= PAGE_INTRO_BASE && pageNum < PAGE_LEVEL_BASE)
    {
        disp_show_page(pageLabel(pageNum), pageText(pageNum, current_score, player_score, selectedQuestions, playerPassed()), pageColor(pageNum, playerPassed()));
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

        disp_show_page(pageLabel(pageNum), pageText(pageNum, current_score, player_score, selectedQuestions, playerPassed()), pageColor(pageNum, playerPassed()));
        return;
    }

    // FINAL
    if (pageNum == PAGE_FINAL)
    {
        disp_show_page(pageLabel(pageNum), pageText(pageNum, current_score, player_score, selectedQuestions, playerPassed()), pageColor(pageNum, playerPassed()));
        return;
    }

    // ACCESS RESULT
    if (pageNum == PAGE_ACCESS_RESULT)
    {
        disp_show_page(pageLabel(pageNum), pageText(pageNum, current_score, player_score, selectedQuestions, playerPassed()), pageColor(pageNum, playerPassed()));
        if (playerPassed())
            digitalWrite(PIN_GREEN, HIGH);
        else
            digitalWrite(PIN_RED, HIGH);
        return;
    }

    // DENIED (failure only — skip to leaderboard if player passed)
    if (pageNum == PAGE_DENIED)
    {
        if (playerPassed())
        {
            displayPage(PAGE_LEADERBOARD);
            return;
        }
        disp_show_page("DENIED", "Your entry has been denied.", color333(7, 0, 0));
        return;
    }

    // LEADERBOARD
    if (pageNum == PAGE_LEADERBOARD)
    {
        updateLeaderboard(player_score);
        leaderboardPageShownMillis = millis();
        playerLeaderboardRank = -1;
        for (int i = 0; i < LEADERBOARD_SIZE; i++) {
            if (leaderboard[i] == player_score) { playerLeaderboardRank = i; break; }
        }
        disp_show_leaderboard(leaderboard, LEADERBOARD_SIZE, pageColor(pageNum, playerPassed()));
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
    if (pageNum == PAGE_DENIED)
        return DENIED_PAGE_MS;
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
        if (elapsed < PROMPT_COUNTDOWN_MS)
        {
            unsigned long remainingSeconds = (PROMPT_COUNTDOWN_MS - elapsed) / 1000;
            disp_draw_corner_label(String(remainingSeconds) + "s", color333(7, 0, 0));
        }
        else
        {
            // Blink "0s" 3 times over the final 2 seconds (6 half-cycles × 333ms each)
            unsigned long blinkElapsed = elapsed - PROMPT_COUNTDOWN_MS;
            uint8_t halfCycle = blinkElapsed / 333;
            bool visible = (halfCycle % 2 == 0);
            disp_draw_corner_label("0s", visible ? color333(7, 0, 0) : color333(0, 0, 0));
        }
        return;
    }

    if (isResultPage(currentPage))
    {
        // disp_draw_corner_label(String(frozenWeight) + "g", color333(7, 7, 7));
        return;
    }

    if (currentPage == PAGE_LEADERBOARD && playerLeaderboardRank >= 0)
    {
        bool visible = (millis() / 500) % 2 == 0;
        disp_blink_leaderboard_row(playerLeaderboardRank, visible,
                                   pageColor(PAGE_LEADERBOARD, playerPassed()));
        return;
    }
}

