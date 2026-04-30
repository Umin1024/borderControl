
// Minimal example: display a single large line of text
// on a 96x48 panel chain (192x48 total)

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "Hx711.h"

#define PANEL_RES_X 96    // Width of each individual panel
#define PANEL_RES_Y 48     // Height of each individual panel
#define PANEL_CHAIN 2      // Number of panels chained together
#define PIN_E 17
#define PIN_RED 38
#define PIN_GREEN 39

MatrixPanel_I2S_DMA *dma_display = nullptr;

void setup() {
  Init_Hx711();
  Get_Maopi();

  HUB75_I2S_CFG mxconfig;
  mxconfig.mx_height = PANEL_RES_Y;      // we have 64 pix heigh panels
  mxconfig.mx_width = PANEL_RES_X;
  mxconfig.chain_length = PANEL_CHAIN;
  mxconfig.gpio.e = PIN_E;                // we MUST assign pin e to some free pin on a board to drive 64 pix height panels with 1/32 scan
  //mxconfig.driver = HUB75_I2S_CFG::FM6126A;     // in case that we use panels based on FM6126A chip, we can change that


  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(255);
  dma_display->fillScreen(dma_display->color565(0, 0, 0));
  dma_display->setTextColor(dma_display->color565(255, 0, 0));
  dma_display->setTextSize(2);
  dma_display->setTextWrap(false);
  dma_display->setCursor(0, 0);
  dma_display->print("welcome to the");
  dma_display->setCursor(0, 17);
  dma_display->print("great nation");

  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  digitalWrite(PIN_RED, LOW);
  digitalWrite(PIN_GREEN, LOW);

  delay(1000);
}

void loop() {
  unsigned long weight = Get_Weight();
  unsigned long now = millis();
  int countdown = 10 - ((now / 1000) % 10);
  bool redPhase = ((now / 10000) % 2) == 0;
  if (redPhase) {
    digitalWrite(PIN_RED, HIGH);
    digitalWrite(PIN_GREEN, LOW);
  } else {
    digitalWrite(PIN_RED, LOW);
    digitalWrite(PIN_GREEN, HIGH);
  }

  // 左下角重量显示
  dma_display->setTextSize(1);
  dma_display->fillRect(0, 33, 96, 15, dma_display->color565(0, 0, 0));
  dma_display->setCursor(1, 40);
  dma_display->print("W:");
  dma_display->print(weight);
  dma_display->print("g");

  // 右下角倒计时显示
  dma_display->fillRect(150, 33, 40, 15, dma_display->color565(0, 0, 0));
  dma_display->setCursor(150, 40);
  dma_display->print(countdown);
  dma_display->print('s');

  delay(200);
}
