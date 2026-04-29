// testshapes demo for RGBmatrixPanel library.
// Demonstrates the drawing abilities of the RGBmatrixPanel library.
// For 64x64 RGB LED matrix.

// WILL NOT FIT on ARDUINO UNO -- requires a Mega, M0 or M4 board

#include "RGBmatrixPanel.h"

#include "bit_bmp.h"
#include "fonts.h"
#include <string.h>
#include <stdlib.h>
#include "Hx711.h"

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

// 按钮定义
#define BUTTON_PIN 50  // 按钮连接到引脚 50，可根据需要修改

// 翻页控制相关定义
#define PAGE_START     0    // 第 0 页：初始页面
#define PAGE_MEASURE   1    // 新增：测量当前重量页（放在 START 之后）
#define PAGE_DISCARD_1 2    // 第 2 页：丢弃提示 1
#define PAGE_DISCARD_2 3    // 第 3 页：丢弃提示 2
#define PAGE_DISCARD_3 4    // 第 4 页：丢弃提示 3
#define PAGE_DISCARD_4 5    // 第 5 页：丢弃提示 4
#define PAGE_DISCARD_5 6    // 第 6 页：丢弃提示 5
#define TOTAL_PAGES    7    // 总页数（更新为 7）

RGBmatrixPanel matrix(A, B, C, D, E, CLK, LAT, OE, false, WIDTH, _HEIGHT);
unsigned long Weight = 0;
unsigned long new_Weight = 0;
// 新增：重量显示节流相关
unsigned long lastWeightMillis = 0;
const unsigned long WEIGHT_INTERVAL_MS = 500; // 500 ms

// 新增：显示抑制/阈值控制
const unsigned long DISPLAY_DEADZONE = 40; // 小于 40g 显示为 0g
unsigned long lastDisplayedWeight = 0xFFFFFFFFUL; // 保证首次更新

// 新增：player score / 初始重量 / 钱包参考重量
unsigned long initialWeight = 0;
long player_score = 0;
long current_score = 0;
const unsigned long WALLET_WEIGHT = 100; // 100 g

// 翻页状态管理
volatile uint8_t currentPage = PAGE_START;
unsigned long lastPageChangeTime = 0;
#define PAGE_DISPLAY_TIME 3000  // 每页显示 3 秒

// 按钮状态
boolean buttonPressed = false;
boolean pageStarted = false;  // 是否已启动页面切换  -> 改为 false（不自动定时）

void setup() {
  matrix.begin();
  Serial.begin(9600);
  
  // 初始化按钮引脚
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Init_Hx711();
  Get_Maopi();
  // 初始显示动画和初始页面
  matrix.setTextSize(0);
  //start_animation();
  displayPage(PAGE_START);  // 显示第一页
  
  Serial.println("Press button to start");
}

void loop() {
  // 检测按钮按下（低电平有效），按下时翻到下一页（边沿触发）
  if(digitalRead(BUTTON_PIN) == LOW) {  // 按钮按下（低电平）
    if(!buttonPressed) {
      buttonPressed = true;
      nextPage();                         // 立即翻页
      Serial.println("Button pressed - Next page");
      delay(50);  // 简单防抖
    }
  } else {
    buttonPressed = false;
  }

  // 每 500ms 更新一次重量并在变化时刷新显示
  if (millis() - lastWeightMillis >= WEIGHT_INTERVAL_MS) {
    lastWeightMillis = millis();
    unsigned long measured = Get_Weight();   // 读取并计算重量（单位：g）
    // 应用死区：40g 以下视为 0g
    if (measured < DISPLAY_DEADZONE) measured = 0;

    // 仅当待显示重量与上次不同才刷新显示
    if (measured != lastDisplayedWeight) {
      lastDisplayedWeight = measured;
      Weight = measured;   // 更新全局用于显示的值
      show_weight();       // 只有在数值改变时才调用刷新显示函数
    }
  }

  delay(20); // 保持短延时，避免占用过多 CPU
}



// ========== 翻页控制函数 ==========
/**
 * @brief 切换到下一页
 * @param None
 * @return None
 */
void nextPage(void) {
  currentPage++;
  if(currentPage >= TOTAL_PAGES) {
    currentPage = PAGE_START;  // 循环回到第一页
  }
  displayPage(currentPage);
}

/**
 * @brief 切换到上一页
 * @param None
 * @return None
 */
void previousPage(void) {
  if(currentPage == 0) {
    currentPage = TOTAL_PAGES - 1;  // 循环到最后一页
  } else {
    currentPage--;
  }
  displayPage(currentPage);
}

/**
 * @brief 根据页码显示对应页面
 * @param pageNum 页码（0-TOTAL_PAGES-1）
 * @return None
 */
void displayPage(uint8_t pageNum) {
  switch(pageNum) {
    case PAGE_START:
      page_start_0();
      Serial.println("Page: Start");
      break;
    case PAGE_MEASURE:
      page_measure_current();      // 新增：测量当前重量页
      Serial.println("Page: Measure");
      break;
    case PAGE_DISCARD_1:
      page_discard_1();
      Serial.println("Page: Discard");
      break;
    case PAGE_DISCARD_2:
      {
        // 进入 PAGE_DISCARD_2 时记录新重量并计算 current_score
        new_Weight = Weight; // 单位：g（已稳定、死区处理）
        unsigned long diff = (new_Weight >= initialWeight) ? (new_Weight - initialWeight) : (initialWeight - new_Weight);
        long score = (long)diff - (long)WALLET_WEIGHT;
        if (score < 0) score = -score;
        current_score = score;
        player_score = player_score+current_score; // 累加总分
        // 可选日志
        initialWeight = new_Weight; // 更新初始重量为当前重量，便于下一次比较 
        Serial.print("Initial: "); Serial.print(initialWeight);
        Serial.print("  New: "); Serial.print(new_Weight);
        Serial.print("  diff: "); Serial.print(diff);
        Serial.print("  current_score: "); Serial.println(current_score);
      }
      page_discard_2();
      Serial.println("Page: Discard 2");
      break;
    case PAGE_DISCARD_3:
      page_discard_3();
      Serial.println("Page: Discard 3");
      break;
    case PAGE_DISCARD_4:
          {
        // 进入 PAGE_DISCARD_4 时记录新重量并计算 current_score
        new_Weight = Weight; // 单位：g（已稳定、死区处理）
        unsigned long diff = (new_Weight >= initialWeight) ? (new_Weight - initialWeight) : (initialWeight - new_Weight);
        long score = (long)diff - (long)WALLET_WEIGHT;
        if (score < 0) score = -score;
        current_score = score;
        player_score = player_score+current_score; // 累加总分
        initialWeight = new_Weight; // 更新初始重量为当前重量，便于下一次比较 
        // 可选日志
        Serial.print("Initial: "); Serial.print(initialWeight);
        Serial.print("  New: "); Serial.print(new_Weight);
        Serial.print("  diff: "); Serial.print(diff);
        Serial.print("  current_score: "); Serial.println(current_score);
      }
      page_discard_4();
      Serial.println("Page: Discard 4");
      break;
    case PAGE_DISCARD_5:
      page_discard_5();
      Serial.println("Page: Discard 5");
      break;
    default:
      currentPage = PAGE_START;
      page_start_0();
      break;
  }
}

// ========== 页面显示函数 ==========
void page_start_0()
{
  matrix.fillScreen(matrix.Color333(0, 0, 0));
  matrix.setCursor(10, 1*8);
  matrix.setTextColor(matrix.Color333(7,7,0));
  matrix.println("press start!");
}
// void page_offer_1()
// {
//   matrix.fillScreen(matrix.Color333(0, 0, 0));
//   matrix.setTextSize(1);
//   matrix.setTextWrap(false);

//   matrix.setCursor(0, 1*8);
//   uint8_t w0 = 0;
//   char *str0 = "what do you have";
//   for (w0=0; w0<16; w0++) {
//     matrix.setTextColor(Wheel(w0));
//     matrix.print(str0[w0]);
//   }

//   matrix.setCursor(0, 2*8);
//   matrix.setTextColor(matrix.Color333(7,7,7));
//   matrix.println("a pen ask you");

//   matrix.setCursor(0, 3*8);
//   uint8_t w1 = 0;
//   char *str1 = "what do you have";
//   for (w1=0; w1<16; w1++) {
//     matrix.setTextColor(Wheel(w1));
//     matrix.print(str1[w1]);
//   }

//   matrix.setCursor(36, 5*8);
//   matrix.setTextColor(matrix.Color333(7,0,7));
//   matrix.println("g OFFERED");
// }

void page_discard_1()
{
  matrix.fillScreen(matrix.Color333(0, 0, 0));
  
  matrix.setCursor(10, 1*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.println("PLEASE DISCARD");
  matrix.setCursor(20, 3*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.println("YOUR WALLETS"); 
}

void page_discard_2()
{
  matrix.fillScreen(matrix.Color333(0, 0, 0));
  matrix.setCursor(10, 1*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.println("CLOSE");
  matrix.setCursor(10, 2*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.println("BUT");
  // 在第二行显示分数信息
  matrix.setCursor(10, 3*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.print("YOUR ARE "); 
matrix.print(current_score);
matrix.print("g OFF"); 
}

void page_discard_3()
{
  matrix.fillScreen(matrix.Color333(0, 0, 0));
  matrix.setCursor(10, 1*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.println("PLEASE DISCARD");
  matrix.setCursor(20, 3*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.println("YOUR GLOCK"); 
}

void page_discard_4()
{
  matrix.fillScreen(matrix.Color333(0, 0, 0));
  matrix.setCursor(10, 1*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.println("CLOSE");
  matrix.setCursor(10, 2*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.println("BUT");
  // 在第二行显示分数信息
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.print("YOUR ARE"); 
matrix.print(current_score); 
matrix.print("g OFF"); 
}


void page_discard_5()
{
  matrix.fillScreen(matrix.Color333(0, 0, 0));
  matrix.setCursor(0, 0*8);
  matrix.setTextColor(matrix.Color333(0, 7, 0));
  matrix.println("Final Deviation:");
  
  matrix.setCursor(0, 2*8);
  matrix.setTextColor(matrix.Color333(7, 7, 0));
  matrix.print(player_score);
  matrix.print(" g");
  
  matrix.setCursor(0, 4*8);
  matrix.setTextColor(matrix.Color333(7, 0, 7));
  char *str = "The screening has been completed.";
  for (int i = 0; str[i] != '\0'; ++i) {
    if (str[i] == '\n') {
      matrix.println();
    } else {
      matrix.print(str[i]);
    }
  }
  
  Serial.print("Final player_score: ");
  Serial.println(player_score);
}

// 新增：测量并立即显示当前重量的页面
void page_measure_current()
{
  matrix.fillScreen(matrix.Color333(0, 0, 0));
  matrix.setTextSize(1);
  matrix.setTextWrap(false);

  matrix.setCursor(8, 1*8);
  matrix.setTextColor(matrix.Color333(7,7,0));
  matrix.println("MEASUREING...");

  // 读取稳定重量（Get_Weight 内已有多次采样/稳定处理）
  unsigned long measured = Get_Weight();
  if (measured < DISPLAY_DEADZONE) measured = 0; // 死区处理（<40g 为 0）

  // 更新全局并显示
  Weight = measured;
  lastDisplayedWeight = measured; // 防止与周期刷新冲突



  // 日志
  Serial.print("Measured weight page: ");
  Serial.println(measured);
}

void show_weight()
{
  matrix.fillRect(0, 5*8, 24, 8, matrix.Color333(0, 0, 0));
  matrix.setCursor(0, 5*8); 
  matrix.setTextColor(matrix.Color333(7,9,7));

  //String wstr = String(Weight);
  matrix.println(Weight);
  Serial.print(Weight);	//串口显示重量
  Serial.print(" g\n");	//显示单位
  Serial.print("\n");		//显示单位
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

void start_animation()
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
}
