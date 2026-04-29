
// Minimal example: display a single large line of text
// on a 96x48 panel chain (192x48 total)

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

#define PANEL_RES_X 96    // Width of each individual panel
#define PANEL_RES_Y 48     // Height of each individual panel
#define PANEL_CHAIN 1      // Number of panels chained together
#define PIN_E 17

MatrixPanel_I2S_DMA *dma_display = nullptr;

void setup() {


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
  dma_display->setTextColor(dma_display->color565(255, 255, 255));
  dma_display->setTextSize(2);
  dma_display->setTextWrap(false);
  dma_display->setCursor(0, 0);
dma_display->print("welcome to the gr");
dma_display->setCursor(0, 17);
dma_display->print("eat nation's por");
dma_display->setCursor(0, 33);
dma_display->print("t of entry");
  
}

void loop() {
  // Nothing else is needed. Keep the text on-screen.
  delay(1000);
}
