#pragma once
#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "config.h"
#include "game_content.h"

// Shared display object — defined in main.ino.
extern MatrixPanel_I2S_DMA* dma_display;

// Map 0-7 RGB to 16-bit color565.
uint16_t color333(uint8_t r, uint8_t g, uint8_t b);

void disp_show_page(const String& pageTag, const String& pageText, uint16_t color);
void disp_show_leaderboard(const long scores[], uint8_t count, uint16_t color);
void disp_draw_corner_label(const String& label, uint16_t color);

// Debug: raw weight overlay drawn over any page (size-1 font, bottom-right).
void disp_debug_weight(long rawWeight);
void disp_print_serial_preview();
