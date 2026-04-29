#ifndef __HX711__H__
#define __HX711__H__

#include <Arduino.h>

#define HX711_SCK 42  // PORTA0 (安全，作为输出)
#define HX711_DT 43   // PORTL (不受 RGB 矩阵影响，推荐改为 44-49 之间的任意引脚)

extern void Init_Hx711();
extern unsigned long HX711_Read(void);
extern unsigned long Get_Weight();
extern void Get_Maopi();
extern bool Flag_Error;

#endif
