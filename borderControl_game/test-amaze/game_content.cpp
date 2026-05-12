#include "game_content.h"

// --- UI strings ---
const char *const TEXT_START = "Please step onto the platform，with all your belongings.";
const char *const TEXT_PROMPT_PREFIX = "\nPlease discard the following item: ";
const char *const TEXT_RESULT_PREFIX = "\nThe weight discarded was off by: \n";
const char *const TEXT_FINAL_PREFIX = "\nCumulative weight discrepancy so far: \n";
const char *const TEXT_LEADERBOARD_PREFIX = "\nLeaderboard (top 3 scores)";

// --- Intro messages (shown before levels begin) ---
// Edit text here; count must equal NUM_INTRO_PAGES (config.h).
const char *const INTRO_TEXTS[NUM_INTRO_PAGES] = {
    "\n\nWelcome to the Great Nation’s Border Control!",
    "\n\nPlease step on the platform with all your belongings.",
    "\n\nDo not remove any items until instructed.",
    "\n\nDetecting ...",
    "\n\nProhibited items found! ☹",
};

// --- Level data ---
// Each level has QUESTIONS_PER_LEVEL (3) variants.
// At game start, one variant is randomly selected for that run.
// To add a new level: increase NUM_LEVELS in config.h and append an entry here.
// To fill in variants: replace the TODO placeholders below.
const Level LEVELS[NUM_LEVELS] = {

        {// Level 0 — Home-cooked Food
     {
         {"a jar of homemade spice", 740},
         {"a bottle of pickled cabbage", 850},
         {"a bag of dried shiitake mushrooms", 500},
     }},

    {// Level 1 — Cultural Tools
     {
         {"a kettle", 1200},
         {"a bamboo steamer", 800},
         {"a bag of home made dumpling skins", 500},
     }},

    {// Level 2 — Household Items
     {
         {"a blanket", 1500},
         {"a mosquito net", 800},
         {"an electric blanket", 1800},
     }},

    {// Level 3 — Outdoor Cooking
     {
         {"wok", 2110},
         {"a piece of hardtack", 2000},
         {"a portable gas stove", 1200},
     }},

    {// Level 4 — Hippie Shelter
     {
         {"a tent", 2500},
         {"a sleeping bag", 1800},
         {"a portable washing machine", 1600},
     }},

    {// Level 5 — Hometown Items
     {
         {"a rice cooker", 3500},
         {"a incense burner", 1500},
         {"a Mahjong set", 3500},
     }},

    {// Level 6 — Blood
     {
         {"400cc blood", 450},
         {"500cc blood", 550},
         {"300cc blood", 340},
     }},

    {// Level 7 — Organs
     {
         {"a kidney", 150},
         {"a heart", 320},
         {"a liver", 1500},
     }},
};
