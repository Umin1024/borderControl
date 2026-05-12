#pragma once
#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "config.h"
#include "game_content.h"

// Shared display object — defined in main.ino.
extern MatrixPanel_I2S_DMA* dma_display;

// Map 0-7 RGB to 16-bit color565.
uint16_t color333(uint8_t r, uint8_t g, uint8_t b);

// --- Display templates ---
// Each function clears the screen and draws one page layout.
// They are pure display: no game-state side effects.

void disp_start_screen();
void disp_intro(uint8_t introIdx);
void disp_level_prompt(const char* itemName);
void disp_level_result(long errorGrams);
void disp_final(long totalErrorGrams);
void disp_leaderboard(long scores[], uint8_t count);

// Debug: raw weight overlay drawn over any page (size-1 font, bottom-right).
void disp_debug_weight(long rawWeight);
