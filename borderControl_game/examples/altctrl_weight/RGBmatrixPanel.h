/*!
 * @file RGBmatrixPanel.h
 *
 * This is the documentation for Adafruit's RGB LED Matrix Panel library
 * for the Arduino platform.  It is designed to work with 16x32, 32x32 and
 * 32x64 panels.
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 * Written by Limor Fried/Ladyada & Phil Burgess/PaintYourDragon for
 * Adafruit Industries.
 *
 * BSD license, all text here must be included in any redistribution.
 *
 */

#ifndef RGBMATRIXPANEL_H
#define RGBMATRIXPANEL_H

#if ARDUINO >= 100
 #include <Arduino.h>
#else
 #include <WProgram.h>
 #include <pins_arduino.h>
#endif
#include "Adafruit_GFX.h"

class RGBmatrixPanel : public Adafruit_GFX {

 public:
 
  /**
   * @fn RGBmatrixPanel
   *  @brief Constructor
   */ 
  RGBmatrixPanel(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e,
    uint8_t sclk, uint8_t latch, uint8_t oe, boolean dbuf, uint8_t width, uint8_t high);

  /**
   * @fn customizeZH
   * @brief Chinese character display
   * @param arr Display contents
   * @param fontSize font size (only 16,32 and 64 three options)
   * @param x x-axis
   * @param y  y-axis
   * @param color  color
   */   
  void customizeZH(const uint8_t *arr, uint8_t fontSize, uint8_t x, uint8_t y, uint16_t color);

  /**
   * @fn begin
   * @brief init function
   * @return None
   */ 
  void  begin(void);
  
  /**
   * @fn drawPixel
   * @brief Draw pixel
   * @param x pixel X-coordinate
   * @param y pixel Y-coordinate
   * @param c pixel color
   */ 
  void drawPixel(int16_t x, int16_t y, uint16_t c);

  /**
   * @fn fillScreen
   * @brief Fill the screen with a color
   * @param c the color filled in the screen
   * @return None 
   */
  void fillScreen(uint16_t c);

  /**
   * @brief 
   * @param copy
   * @return None 
   */
  void swapBuffers(boolean copy);

  /**
   * @fn dumpMatrix
   * @brief Dump the displayed data into the serial port
   * @return None
   */
  void dumpMatrix(void);

  /**
   * @fn backBuffer
   * @brief Return address of back buffer
   * @return Return address of back buffer
   */
  uint8_t* backBuffer(void);

  /**
   * @fn Color333
   * @brief Convert RGB333 to RGB565
   * @param r red
   * @param g green
   * @param b blue
   * @return Return the converted color
   */
  uint16_t  Color333(uint8_t r, uint8_t g, uint8_t b);

  /**
   * @fn Color444
   * @brief Convert RGB444 to RGB565
   * @param r red
   * @param g green
   * @param b blue
   * @return Return the converted color
   */
  uint16_t  Color444(uint8_t r, uint8_t g, uint8_t b);

  /**
   * @fn Color888
   * @brief Convert RGB888 to RGB565
   * @param r red
   * @param g green
   * @param b blue
   * @return Return the converted color
   */
  uint16_t  Color888(uint8_t r, uint8_t g, uint8_t b);

  /**
   * @fn Color888
   * @brief Convert RGB888 to RGB565
   * @param r red
   * @param g green
   * @param b blue
   * @param gflag whether to correct the color
   * @return Return the converted color
   */
  uint16_t  Color888(uint8_t r, uint8_t g, uint8_t b, boolean gflag);

  /**
   * @fn ColorHSV
   * @brief Set color saturation
   * @param hue hue
   * @param sat saturation
   * @param val value
   * @param gflag whether to correct the color
   * @return Return the converted color
   */
  uint16_t  ColorHSV(long hue, uint8_t sat, uint8_t val, boolean gflag);

  /**
   * @fn updateDisplay
   * @brief Display update
   * @return None
   */
  void setFont(const GFXfont * f);
   
  void updateDisplay(void);

 private:

  uint8_t           *matrixbuff[2];
  uint8_t           nRows;
  int16_t         _width, _high;
  volatile uint8_t  backindex;
  volatile boolean  swapflag;

  // PORT register pointers, pin bitmasks, pin numbers:
  volatile uint8_t
    *latport, *oeport, *addraport, *addrbport, *addrcport, *addrdport, *addreport;
  uint8_t
    sclkpin, latpin, oepin, addrapin, addrbpin, addrcpin, addrdpin, addrepin,
    _sclk, _latch, _oe, _a, _b, _c, _d, _e;

  // Counters/pointers for interrupt handler:
  volatile uint8_t row, plane;
  volatile uint8_t *buffptr;
  void FM6126_Init();
  void Write_REG1(int A,int B,int C,int D,int E,int F,uint16_t REG_DATA);
  void Write_REG2(int A,int B,int C,int D,int E,int F,uint16_t REG_DATA);
  
};

#endif // RGBMATRIXPANEL_H
