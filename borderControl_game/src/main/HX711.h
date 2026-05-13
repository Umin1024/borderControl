#ifndef __HX711__H__
#define __HX711__H__

#include <Arduino.h>

class HX711
{
public:
    HX711(int SCK_PIN, int DT_PIN, float GapValueIn = 20);
    long Get_Weight();
    void begin();

    int HX711_SCK;
    int HX711_DT;
    float ValueGap;
    long HX711_Buffer;
    long Weight_Maopi;
    long Weight_Shiwu;

private:
    void Get_Maopi();
    unsigned long HX711_Read();
};

#endif
