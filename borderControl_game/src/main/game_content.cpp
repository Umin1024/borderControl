#include "game_content.h"
#include "display.h"

// --- UI strings ---
const char *const TEXT_START = "Please step onto the platform, with all your belongings.";
const char *const TEXT_PROMPT_PREFIX = "Please discard the following item: ";
const char *const TEXT_RESULT_PREFIX = "The weight discarded was off by: ";
const char *const TEXT_FINAL_PREFIX = "Cumulative weight discrepancy so far: ";
const char *const TEXT_LEADERBOARD_PREFIX = "Leaderboard (top 3 scores)";

// --- Intro messages (shown before levels begin) ---
// Edit text here; count must equal NUM_INTRO_PAGES (config.h).
const char *const INTRO_TEXTS[NUM_INTRO_PAGES] = {
    "Welcome to the Great Nation's Border Control!",
    "Please step on the platform with all your belongings.",
    "Do not remove any items until instructed.",
    "\nDetecting ...",
    "\nProhibited items found! ☹",
};

// --- Level data ---
// Each level has QUESTIONS_PER_LEVEL (3) variants.
// At game start, one variant is randomly selected for that run.
// To add a new level: increase NUM_LEVELS in config.h and append an entry here.
// To fill in variants: replace the TODO placeholders below.
// ============================================================
// Page UI helpers
// ============================================================

String pageLabel(uint8_t pageNum)
{
    if (pageNum == PAGE_START)
        return String("START");
    if (pageNum >= PAGE_INTRO_BASE && pageNum < PAGE_LEVEL_BASE)
        return String("INTRO ") + String(pageNum - PAGE_INTRO_BASE + 1);
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

String pageText(uint8_t pageNum, long current_score, long player_score,
                const uint8_t selectedQuestions[], bool passed)
{
    if (pageNum == PAGE_START)
        return String(TEXT_START);

    if (pageNum >= PAGE_INTRO_BASE && pageNum < PAGE_LEVEL_BASE)
        return String(INTRO_TEXTS[pageNum - PAGE_INTRO_BASE]);

    if (pageNum >= PAGE_LEVEL_BASE && pageNum < PAGE_FINAL)
    {
        uint8_t levelPage = pageNum - PAGE_LEVEL_BASE;
        uint8_t levelIdx = levelPage / 2;
        bool isResult = (levelPage % 2 == 1);
        const Question &q = LEVELS[levelIdx].questions[selectedQuestions[levelIdx]];
        if (isResult)
            return String(TEXT_RESULT_PREFIX) + " <" + String(current_score) + "> g";
        return String(TEXT_PROMPT_PREFIX) + " <" + String(q.itemName) + ">";
    }

    if (pageNum == PAGE_FINAL)
        return String(TEXT_FINAL_PREFIX) + " <" + String(player_score) + "> g";

    if (pageNum == PAGE_ACCESS_RESULT)
        return passed
                   ? String("Congratulations. Your entry has been approved.")
                   : String("Unfortunately. the weight discrepancy was too large.");

    if (pageNum == PAGE_LEADERBOARD)
        return String(TEXT_LEADERBOARD_PREFIX);

    return String();
}

uint16_t pageColor(uint8_t pageNum, bool passed)
{
    if (pageNum == PAGE_FINAL)
        return color333(7, 0, 0);
    if (pageNum == PAGE_ACCESS_RESULT)
        return passed ? color333(0, 7, 0) : color333(7, 0, 0);
    if (pageNum == PAGE_LEADERBOARD)
        return color333(7, 0, 0);
    return color333(7, 0, 0);
}

// ============================================================
// Level data
// ============================================================

const Level LEVELS[NUM_LEVELS] = {

    {// Level 0 — Home-cooked Food
     {
         {"a jar of homemade spice", 200},
         {"a bottle of pickled cabbage", 850},
         {"a bag of dried shiitake mushrooms", 296},
     }},

    {// Level 1 — Cultural Tools
     {
         {"a kettle", 1200},
         {"a bamboo steamer", 294},
         {"a plushie", 531},
     }},

    {// Level 2 — Household Items
     {
         {"a blanket", 1100},
         {"a mosquito net", 698},
         {"an electric blanket", 1756},
     }},

    {// Level 3 — Outdoor Cooking
     {
         {"a wok", 2110},
         {"a piece of hardtack", 78},
         {"a portable gas stove", 3260},
     }},

    {// Level 4 — Hippie Shelter
     {
         {"a tent", 2670},
         {"a sleeping bag", 1840},
         {"a portable washing machine", 1992},
     }},

    {// Level 5 — Hometown Items
     {
         {"a rice cooker", 4521},
         {"an incense burner", 1500},
         {"a Mahjong set", 4113},
     }},

    {// Level 6 — Blood
     {
         {"400cc of blood", 410},
         {"500cc of blood", 520},
         {"300cc of blood", 310},
     }},

    {// Level 7 — Organs
     {
         {"a kidney", 158},
         {"a heart", 397},
         {"a liver", 1521},
     }},
};
