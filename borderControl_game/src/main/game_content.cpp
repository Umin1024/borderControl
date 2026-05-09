#include "game_content.h"

// --- UI strings ---
const char *const TEXT_START = "PRESS START!";
const char *const TEXT_PROMPT_PREFIX = "discard ";
const char *const TEXT_RESULT_PREFIX = "close! \n off by: ";
const char *const TEXT_FINAL_PREFIX = "FINAL ERROR: \n";
const char *const TEXT_LEADERBOARD_PREFIX = "BEST: ";

// --- Intro messages (shown before levels begin) ---
// Edit text here; count must equal NUM_INTRO_PAGES (config.h).
const char *const INTRO_TEXTS[NUM_INTRO_PAGES] = {
    "WELCOME TO THE GREAT NATION'S PORT OF ENTRY", // intro 0
    "step onto the scale with your things",        // intro 1
    "do not remove anything",                      // intro 2
    "...",                                         // intro 3
    "forbidden items found",                       // intro 4
};

// --- Level data ---
// Each level has QUESTIONS_PER_LEVEL (3) variants.
// At game start, one variant is randomly selected for that run.
// To add a new level: increase NUM_LEVELS in config.h and append an entry here.
// To fill in variants: replace the TODO placeholders below.
const Level LEVELS[NUM_LEVELS] = {

    {// Level 0 — small spice
     {
         {"jar of spice", 740},
         /* TODO */ {"jar of spice B", 800},
         /* TODO */ {"jar of spice C", 650},
     }},

    {// Level 1 — kettle
     {
         {"your kettle", 1200},
         /* TODO */ {"your kettle B", 1100},
         /* TODO */ {"your kettle C", 1300},
     }},

    {// Level 2 — blanket
     {
         {"blanket", 1500},
         /* TODO */ {"blanket B", 1400},
         /* TODO */ {"blanket C", 1600},
     }},

    {// Level 3 — wok
     {
         {"wok", 2110},
         /* TODO */ {"wok B", 2000},
         /* TODO */ {"wok C", 2200},
     }},

    {// Level 4 — tent
     {
         {"tent", 2500},
         /* TODO */ {"tent B", 2300},
         /* TODO */ {"tent C", 2700},
     }},

    {// Level 5 — rice cooker
     {
         {"rice cooker", 4500},
         /* TODO */ {"rice cooker B", 4200},
         /* TODO */ {"rice cooker C", 4800},
     }},

    {// Level 6 — blood
     {
         {"400cc blood", 400},
         /* TODO */ {"400cc blood B", 380},
         /* TODO */ {"400cc blood C", 420},
     }},

    {// Level 7 — organ
     {
         {"kidney", 150},
         /* TODO */ {"kidney B", 140},
         /* TODO */ {"kidney C", 160},
     }},

};
