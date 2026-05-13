#pragma once

// --- Hardware pins ---
#define PANEL_RES_X 96
#define PANEL_RES_Y 48
#define PANEL_CHAIN 2
#define TOTAL_DISPLAY_WIDTH (PANEL_RES_X * PANEL_CHAIN)  // 192 for dual screen
#define PIN_E       17
#define PIN_RED     11
#define PIN_GREEN   12

// --- Sensor ---
#define HX711_SCK        13
#define HX711_DT         14
#define HX711_GAP        20.1f   // calibration: smaller = lighter, larger = heavier

// --- Timing ---
#define WEIGHT_INTERVAL_MS         500
#define INTRO_PAGE_MS             4000
#define PROMPT_PAGE_MS           12000   // 10s countdown + 2s blink
#define PROMPT_COUNTDOWN_MS      10000   // visible countdown portion
#define RESULT_PAGE_MS            6000
#define FINAL_PAGE_MS            10000
#define ACCESS_RESULT_PAGE_MS     6000
#define DENIED_PAGE_MS            5000
#define LEADERBOARD_PAGE_MS      20000
#define DISPLAY_DEADZONE           40
#define DISPLAY_WEIGHT_MIN_G      500   // weights below this show as 0 on screen
#define PLAYER_WEIGHT_THRESHOLD_G 25000UL

// --- Test mode ---
// Set to 1 to skip weight checks so you can browse all pages without the scale.
// Serial command 'n' advances to the next page manually.
#define TEST_MODE 1

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
#define PAGE_DENIED        (PAGE_ACCESS_RESULT + 1)  // only shown on failure
#define PAGE_LEADERBOARD   (PAGE_DENIED + 1)
#define TOTAL_PAGES        (PAGE_LEADERBOARD + 1)
