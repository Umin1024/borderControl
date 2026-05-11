#pragma once

// --- Hardware pins ---
#define PANEL_RES_X 96
#define PANEL_RES_Y 48
#define PANEL_CHAIN 2
#define PIN_E       17
#define PIN_RED     11
#define PIN_GREEN   12

// --- Sensor ---
#define HX711_SCK        13
#define HX711_DT         14
#define HX711_GAP        9.65f   // calibration: smaller = lighter, larger = heavier

// --- Timing ---
#define WEIGHT_INTERVAL_MS         500
#define INTRO_PAGE_MS             4000
#define PROMPT_PAGE_MS           10000
#define RESULT_PAGE_MS            6000
#define FINAL_PAGE_MS            10000
#define ACCESS_RESULT_PAGE_MS    10000
#define LEADERBOARD_PAGE_MS      20000
#define DISPLAY_DEADZONE           40
#define PLAYER_WEIGHT_THRESHOLD_G 25000UL

// --- Game ---
#define PASS_THRESHOLD_G       5000
#define NUM_LEVELS             8
#define QUESTIONS_PER_LEVEL    3   // each level has this many question variants
#define NUM_INTRO_PAGES        5
#define LEADERBOARD_SIZE       3

// --- Page layout (auto-computed, do not edit) ---
// Flow: START → INTRO(x5) → [PROMPT + RESULT](x NUM_LEVELS) → FINAL → ACCESS RESULT → LEADERBOARD
#define PAGE_START         0
#define PAGE_INTRO_BASE    1
#define PAGE_LEVEL_BASE    (PAGE_INTRO_BASE + NUM_INTRO_PAGES)
#define PAGE_FINAL         (PAGE_LEVEL_BASE + NUM_LEVELS * 2)
#define PAGE_ACCESS_RESULT (PAGE_FINAL + 1)
#define PAGE_LEADERBOARD   (PAGE_ACCESS_RESULT + 1)
#define TOTAL_PAGES        (PAGE_LEADERBOARD + 1)
