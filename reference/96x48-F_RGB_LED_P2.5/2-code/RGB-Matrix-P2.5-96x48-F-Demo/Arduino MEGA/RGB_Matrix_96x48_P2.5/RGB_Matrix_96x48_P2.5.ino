// testshapes demo for RGBmatrixPanel library.
// Demonstrates the drawing abilities of the RGBmatrixPanel library.
// For 64x64 RGB LED matrix.

// WILL NOT FIT on ARDUINO UNO -- requires a Mega, M0 or M4 board

#include "RGBmatrixPanel.h"

#include "bit_bmp.h"
#include "fonts.h"
#include <string.h>
#include <stdlib.h>

#define OE     9
#define LAT   10
#define CLK   11
#define A     A0
#define B     A1
#define C     A2
#define D     A3
#define E     A4
#define WIDTH 96
#define _HEIGHT 48

RGBmatrixPanel matrix(A, B, C, D, E, CLK, LAT, OE, false, WIDTH, _HEIGHT);

void setup() {
  
  matrix.begin();
  Demo_0();

}

void loop() {
  // do nothing
   
}


// Input a value 0 to 24 to get a color value.
// The colours are a transition r - g - b - back to r.
uint16_t Wheel(byte WheelPos) {
  if(WheelPos < 8) {
   return matrix.Color333(7 - WheelPos, WheelPos, 0);
  } else if(WheelPos < 16) {
   WheelPos -= 8;
   return matrix.Color333(0, 7-WheelPos, WheelPos);
  } else {
   WheelPos -= 16;
   return matrix.Color333(0, WheelPos, 7 - WheelPos);
  }
}


void Demo_0()
{
  // draw a pixel in solid white
  matrix.drawPixel(0, 0, matrix.Color333(7, 7, 7)); 
  delay(500);

    // fix the screen with red
  matrix.fillRect(0, 0, matrix.width(), matrix.height(), matrix.Color333(7, 0, 0));
  delay(500);

    // fix the screen with green
  matrix.fillRect(0, 0, matrix.width(), matrix.height(), matrix.Color333(0, 7, 0));
  delay(500);

  // fix the screen with blue
  matrix.fillRect(0, 0, matrix.width(), matrix.height(), matrix.Color333(0, 0, 7));
  delay(500);

    // fill the screen with 'black'
  matrix.fillScreen(matrix.Color333(0, 0, 0));

  // draw a box in yellow
  matrix.drawRect(0, 0, matrix.width(), matrix.height(), matrix.Color333(7, 7, 0));
  delay(500);
  
  // draw an 'X' in red
  matrix.drawLine(0, 0, matrix.width()-1, matrix.height()-1, matrix.Color333(7, 0, 0));
  matrix.drawLine(matrix.width()-1, 0, 0, matrix.height()-1, matrix.Color333(7, 0, 0));
  delay(500);
  
  // draw a blue circle
  matrix.drawCircle(10, 10, 10, matrix.Color333(0, 0, 7));
  delay(500);
  
  // fill a violet circle
  matrix.fillCircle(40, 21, 10, matrix.Color333(7, 0, 7));
  delay(500);
  
  // fill the screen with 'black'
  matrix.fillScreen(matrix.Color333(0, 0, 0));
  
  // draw some text!
  matrix.setTextSize(1);     // size 1 == 8 pixels high
  matrix.setTextWrap(false); // Don't wrap at end of line - will do ourselves

  matrix.setCursor(20, 0*8);    // start at top left, with 8 pixel of spacing
  uint8_t w = 0;
  char *str = "WaveshareElectronics";
  for (w=0; w<9; w++) {
    matrix.setTextColor(Wheel(w));
    matrix.print(str[w]);
  }
  matrix.setCursor(13, 1*8);    // next line
  for (w=9; w<20; w++) {
    matrix.setTextColor(Wheel(w));
    matrix.print(str[w]);
  }
  
  matrix.setCursor(6, 2*8);    // next line
  matrix.setTextColor(matrix.Color333(7,7,7));
  matrix.println("RGB LED MATRIX");

  
  matrix.setCursor(0, 3*8);    // next line
  uint8_t w1 = 0;
  char *str1 = "Flexible Display";
  for (w1=0; w1<16; w1++) {
    matrix.setTextColor(Wheel(w1));
    matrix.print(str1[w1]);
  }

  matrix.setCursor(6, 4*8);    // next line
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.print('9');
  matrix.setTextColor(matrix.Color333(7,4,0)); 
  matrix.print('6');
  matrix.setTextColor(matrix.Color333(7,7,0));
  matrix.print('x');
  matrix.setTextColor(matrix.Color333(4,7,0)); 
  matrix.print('4');
  matrix.setTextColor(matrix.Color333(0,7,0));  
  matrix.print('8');
  matrix.setTextColor(matrix.Color333(0,7,7)); 
  matrix.print(" ");
  matrix.setTextColor(matrix.Color333(7,0,0)); 
  matrix.print('R');
  matrix.setTextColor(matrix.Color333(0,7,0));
  matrix.print('G');
  matrix.setTextColor(matrix.Color333(0,0,7)); 
  matrix.print("B");
  matrix.setTextColor(matrix.Color333(7,0,4)); 
  matrix.print(" ");
  matrix.setTextColor(matrix.Color333(0,7,7));
  matrix.print('L');
  matrix.setTextColor(matrix.Color333(4,4,4)); 
  matrix.print('E');
  matrix.setTextColor(matrix.Color333(7,7,0));
  matrix.print('D');
  matrix.setTextColor(matrix.Color333(7,0,0)); 
  matrix.print(' ');

  matrix.setCursor(12, 5*8);    // next line
  matrix.setTextColor(matrix.Color333(4,4,4)); 
  matrix.print('2');
  matrix.setTextColor(matrix.Color333(7,7,0));
  matrix.print('.');
  matrix.setTextColor(matrix.Color333(7,0,0)); 
  matrix.print('5');
  matrix.setTextColor(matrix.Color333(0,7,0));
  matrix.print('m');
  matrix.setTextColor(matrix.Color333(0,0,7)); 
  matrix.print("m");
  matrix.print(' ');
  matrix.setTextColor(matrix.Color333(7,7,0));
  matrix.print('P');
  matrix.setTextColor(matrix.Color333(4,7,0)); 
  matrix.print('i');
  matrix.setTextColor(matrix.Color333(0,7,0));  
  matrix.print('t');
  matrix.setTextColor(matrix.Color333(0,7,7)); 
  matrix.print("c");
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.print('h');
  
}
