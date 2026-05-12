#include "game_content.h"

// --- UI strings ---
const char *const TEXT_START = "BORDER CONTROL\nPLEASE STEP ON\nTHE SCALE";
const char *const TEXT_PROMPT_PREFIX = "Please discard the following item:";
const char *const TEXT_RESULT_PREFIX = "The weight discarded was off by <.";
const char *const TEXT_FINAL_PREFIX = "final error";
const char *const TEXT_LEADERBOARD_PREFIX = "TOP3";

// --- Intro messages (shown before levels begin) ---
const char *const INTRO_TEXTS[NUM_INTRO_PAGES] = {
    "Welcome to the Great Nation’s Border Control!",
    "Please step on the platform with all your belongings.",
    "Do not remove any items until instructed.",
    "Detecting ...",
    "Prohibited items found! ☹",
};

// --- Level data ---
const Level LEVELS[NUM_LEVELS] = {

    {// Level 0 — Home-cooked Food
     {
         {"home spice jar", 740},
         {"pickled cabbage", 850},
         {"dried shiitake", 500},
     }},

    {// Level 1 — Cultural Tools
     {
         {"kettle", 1200},
         {"bamboo steamer", 800},
         {"dumpling skins", 500},
     }},

    {// Level 2 — Household Items
     {
         {"blanket", 1500},
         {"mosquito net", 800},
         {"electric blanket", 1800},
     }},

    {// Level 3 — Outdoor Cooking
     {
         {"wok", 2110},
         {"hardtack box", 2000},
         {"gas stove", 1200},
     }},

    {// Level 4 — Hippie Shelter
     {
         {"tent", 2500},
         {"sleeping bag", 1800},
         {"wash machine", 1600},
     }},

    {// Level 5 — Hometown Items
     {
         {"rice cooker", 3500},
         {"incense burner", 1500},
         {"Mahjong set", 3500},
     }},

    {// Level 6 — Blood
     {
         {"400cc blood", 450},
         {"500cc blood", 550},
         {"300cc blood", 340},
     }},

    {// Level 7 — Organs
     {
         {"kidney", 150},
         {"heart", 320},
         {"liver", 1500},
     }},

};
