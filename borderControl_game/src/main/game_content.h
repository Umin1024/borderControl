#pragma once
#include <Arduino.h>
#include "config.h"

// One measurable question: what item to discard and its expected weight in grams.
struct Question {
    const char*   itemName;     // displayed as "discard <itemName>"
    unsigned long weightGrams;  // expected weight change for scoring
};

// Each level holds QUESTIONS_PER_LEVEL variants; one is picked randomly each game.
struct Level {
    Question questions[QUESTIONS_PER_LEVEL];
};

extern const Level       LEVELS[NUM_LEVELS];
extern const char* const INTRO_TEXTS[NUM_INTRO_PAGES];

// --- UI strings (edit here to change displayed text) ---
extern const char* const TEXT_START;
extern const char* const TEXT_PROMPT_PREFIX;   // prepended to itemName
extern const char* const TEXT_RESULT_PREFIX;   // prepended to error value
extern const char* const TEXT_FINAL_PREFIX;    // prepended to total score
extern const char* const TEXT_LEADERBOARD_PREFIX;

// --- Page UI helpers ---
// These encapsulate all display text and color decisions so main.ino stays logic-only.
String pageLabel(uint8_t pageNum);
String pageText(uint8_t pageNum, long current_score, long player_score,
                const uint8_t selectedQuestions[], bool passed);
uint16_t pageColor(uint8_t pageNum, bool passed);
