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

// 翻页控制相关定义（在 PAGE_DISCARD_6 后插入 10 页）
#define PAGE_START           0
#define PAGE_MSG1            1
#define PAGE_MSG2            2
#define PAGE_MSG3            3
#define PAGE_MSG4            4
#define PAGE_MSG5            5
#define PAGE_DISCARD_1       6
#define PAGE_DISCARD_2       7
#define PAGE_DISCARD_3       8
#define PAGE_DISCARD_4       9
#define PAGE_DISCARD_5       10
#define PAGE_DISCARD_6       11
// 新增：wok（两页）
#define PAGE_DISCARD_7       12  // wok 显示
#define PAGE_DISCARD_8       13  // wok 结果
// 新增：tent（两页）
#define PAGE_DISCARD_9       14  // tent 显示
#define PAGE_DISCARD_10      15  // tent 结果
// 新增：rice cooker（两页）
#define PAGE_DISCARD_11      16  // rice cooker 显示
#define PAGE_DISCARD_12      17  // rice cooker 结果
// 新增：400cc blood（两页）
#define PAGE_DISCARD_13      18  // blood 显示
#define PAGE_DISCARD_14      19  // blood 结果
// 新增：kidney（两页）
#define PAGE_DISCARD_15      20  // kidney 显示
#define PAGE_DISCARD_16      21  // kidney 结果
#define PAGE_DISCARD_FINAL   22  // Final Deviation
#define PAGE_LEADERBOARD     23
#define TOTAL_PAGES          24

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

// 物品重量定义
const unsigned long CHILI_SAUCE_JAR_WEIGHT = 740;      // 740 g
const unsigned long KETTLE_WEIGHT = 1200;              // 1200 g
const unsigned long BLANKET_WEIGHT = 1500;             // 1500 g
const unsigned long WOK_WEIGHT = 2110;                 // 2110 g
const unsigned long TENT_WEIGHT = 2500;                // 2500 g
const unsigned long RICE_COOKER_WEIGHT = 4500;         // 4500 g
const unsigned long BLOOD_WEIGHT = 400;                // 400 g
const unsigned long KIDNEY_WEIGHT = 150;               // 150 g

// 新增：排行榜（保存3个最高分数）
// long leaderboard[3] = {0, 0, 0}; // 原来：高分优先且 0 被占位
long leaderboard[3] = {0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF}; // 使用哨兵值表示空位（越小越好）

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

void show_current_score(unsigned long item_weight){
 // 进入 PAGE_DISCARD_2 时记录新重量并计算 current_score
        new_Weight = Weight; // 单位：g（已稳定、死区处理）
        unsigned long diff = (new_Weight >= initialWeight) ? (new_Weight - initialWeight) : (initialWeight - new_Weight);
        long score = (long)diff - (long)item_weight;
        if (score < 0) score = -score;
        current_score = score;
        player_score = player_score+current_score; // 累加总分
        // 可选日志
        Serial.print("Initial: "); Serial.print(initialWeight);

        initialWeight = new_Weight; // 更新初始重量为当前重量，便于下一次比较 
        Serial.print("  New: "); Serial.print(new_Weight);
        Serial.print("  diff: "); Serial.print(diff);
        Serial.print("  current_score: "); Serial.println(current_score);
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
    case PAGE_MSG1:
      page_msg1();
      Serial.println("Page: Msg1");
      break;
    case PAGE_MSG2:
      page_msg2();
      Serial.println("Page: Msg2");
      break;
    case PAGE_MSG3:
      page_msg3();
      Serial.println("Page: Msg3");
      break;
    case PAGE_MSG4:
      page_msg4();
      Serial.println("Page: Msg4");
      break;
    case PAGE_MSG5:
      page_msg5();
      Serial.println("Page: Msg5");
      break;
    case PAGE_DISCARD_1:
      {
        initialWeight = Weight; // 设置为初始重量
        lastDisplayedWeight = Weight; // 防止与周期刷新冲突
        Serial.print("Measured weight page: ");
        Serial.println(initialWeight);
      }
      page_discard_1();
      Serial.println("Page: Discard");
      break;
    case PAGE_DISCARD_2:
      show_current_score(CHILI_SAUCE_JAR_WEIGHT);
      page_discard_2();
      Serial.println("Page: Discard 2");
      break;
    case PAGE_DISCARD_3:
      page_discard_3();
      Serial.println("Page: Discard 3");
      break;
    case PAGE_DISCARD_4:
      show_current_score(KETTLE_WEIGHT);
      page_discard_4();
      Serial.println("Page: Discard 4");
      break;

    // 新插入：PAGE_DISCARD_5（与 page_discard_3 类似，但显示 blanket）
    case PAGE_DISCARD_5:
      page_discard_5();
      Serial.println("Page: Discard 5 (blanket)");
      break;

    // 新插入：PAGE_DISCARD_6（功能同 PAGE_DISCARD_4，计算偏差并显示）
    case PAGE_DISCARD_6:
      show_current_score(BLANKET_WEIGHT);
      page_discard_6();
      Serial.println("Page: Discard 6");
      break;

    // 新增：wok
    case PAGE_DISCARD_7:
      page_discard_7();
      Serial.println("Page: Discard 7 (wok display)");
      break;
    case PAGE_DISCARD_8:
      show_current_score(WOK_WEIGHT);
      page_discard_8();
      Serial.println("Page: Discard 8 (wok result)");
      break;

    // 新增：tent
    case PAGE_DISCARD_9:
      page_discard_9();
      Serial.println("Page: Discard 9 (tent display)");
      break;
    case PAGE_DISCARD_10:
      show_current_score(TENT_WEIGHT);
      page_discard_10();
      Serial.println("Page: Discard 10 (tent result)");
      break;

    // 新增：rice cooker
    case PAGE_DISCARD_11:
      page_discard_11();
      Serial.println("Page: Discard 11 (rice cooker display)");
      break;
    case PAGE_DISCARD_12:
      show_current_score(RICE_COOKER_WEIGHT);
      page_discard_12();
      Serial.println("Page: Discard 12 (rice cooker result)");
      break;

    // 新增：400cc blood
    case PAGE_DISCARD_13:
      page_discard_13();
      Serial.println("Page: Discard 13 (blood display)");
      break;
    case PAGE_DISCARD_14:
      show_current_score(BLOOD_WEIGHT);
      page_discard_14();
      Serial.println("Page: Discard 14 (blood result)");
      break;

    // 新增：kidney
    case PAGE_DISCARD_15:
      page_discard_15();
      Serial.println("Page: Discard 15 (kidney display)");
      break;
    case PAGE_DISCARD_16:
      show_current_score(KIDNEY_WEIGHT);
      page_discard_16();
      Serial.println("Page: Discard 16 (kidney result)");
      break;

    case PAGE_DISCARD_FINAL:
      page_discard_final();
      Serial.println("Page: Final Deviation");
      break;

    case PAGE_LEADERBOARD:
      updateLeaderboard(player_score);
      page_leaderboard();
      Serial.println("Page: Leaderboard");
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
  player_score = 0; // 重置总分
  current_score = 0;
  Weight = 0;
  initialWeight = 0;
  new_Weight = 0;

  matrix.fillScreen(matrix.Color333(0, 0, 0));
  matrix.setCursor(0, 0*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.println("press start!");
}
      //welcome to the g
      //reat nation’s po
      //rt of entry

      //welcome to the 
      //great nation’s 
      //port of entry
      
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


// 新增：在 page_start_0() 与 page_measure_current() 之间插入的五页（最小字号、红色、左上角起始）
void page_msg1()
{
  matrix.fillScreen(matrix.Color333(0,0,0));
  matrix.setTextSize(0);           // 最小字号（保持与 setup 中一致）
  matrix.setTextWrap(true);
  matrix.setCursor(0, 0);          // 左上角第一个像素
  matrix.setTextColor(matrix.Color333(7,0,0)); // 红色
  matrix.println("welcome to the");
  matrix.println("great nation's");
  matrix.println("port of entry");
}

void page_msg2()
{
  matrix.fillScreen(matrix.Color333(0,0,0));
  matrix.setTextSize(0);
  matrix.setTextWrap(true);
  matrix.setCursor(0, 0);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.println("step onto the");
  matrix.println("scale with all");
  matrix.println("your belongings");
}

void page_msg3()
{
  matrix.fillScreen(matrix.Color333(0,0,0));
  matrix.setTextSize(0);
  matrix.setTextWrap(true);
  matrix.setCursor(0, 0);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.println("do not remove");
  matrix.println("anything until");
  matrix.println("instructed");
}

void page_msg4()
{
  matrix.fillScreen(matrix.Color333(0,0,0));
  matrix.setTextSize(0);
  matrix.setTextWrap(true);
  matrix.setCursor(0, 0);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.println("...");
}

void page_msg5()
{
  matrix.fillScreen(matrix.Color333(0,0,0));
  matrix.setTextSize(0);
  matrix.setTextWrap(true);
  matrix.setCursor(0, 0);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.println("foreign and");
  matrix.println("prohibited items");
  matrix.println("detected");
}

// 新增：测量并立即显示当前重量的页面
// void page_measure_current()
// {
//   matrix.fillScreen(matrix.Color333(0, 0, 0));
//   matrix.setTextSize(1);
//   matrix.setTextWrap(false);

//   matrix.setCursor(8, 1*8);
//   matrix.setTextColor(matrix.Color333(7,7,0));
//   matrix.println("MEASUREING...");

// }

void page_discard_1()
{
  matrix.fillScreen(matrix.Color333(0, 0, 0));
  
  matrix.setCursor(0, 0*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.println("please discard");
  matrix.setCursor(0, 1*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.println("your jar of spice"); 
}

void page_discard_2()
{
  matrix.fillScreen(matrix.Color333(0, 0, 0));
  matrix.setCursor(0, 0*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.println("... close, but");
  matrix.setCursor(0, 1*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.println("you are still");
  // 在第二行显示分数信息
  matrix.setCursor(0, 2*8);
  matrix.setTextColor(matrix.Color333(7,0,0));

matrix.print(current_score);
matrix.print(" grams off."); 
}

void page_discard_3()
{
  matrix.fillScreen(matrix.Color333(0, 0, 0));
  matrix.setCursor(0, 0*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.println("please discard");
  matrix.setCursor(0, 1*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.println("your kettle"); 
}

void page_discard_4()
{
  matrix.fillScreen(matrix.Color333(0, 0, 0));
  matrix.setCursor(0, 0*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.println("... close, but");
  matrix.setCursor(0, 1*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.println("you are still");
  // 在第二行显示分数信息
  matrix.setCursor(0, 2*8);
  matrix.setTextColor(matrix.Color333(7,0,0));

matrix.print(current_score);
matrix.print(" grams off."); 
}

void page_discard_5()
{
  matrix.fillScreen(matrix.Color333(0, 0, 0));
  matrix.setCursor(0, 0*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.println("please discard");
  matrix.setCursor(0, 1*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.println("your blanket"); 
}

void page_discard_6()
{
  matrix.fillScreen(matrix.Color333(0, 0, 0));
  matrix.setCursor(0, 0*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.println("... close, but");
  matrix.setCursor(0, 1*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.println("you are still");
  // 在第二行显示分数信息
  matrix.setCursor(0, 2*8);
  matrix.setTextColor(matrix.Color333(7,0,0));

matrix.print(current_score);
matrix.print(" grams off."); 
}

// 新增：page_discard_5（please discard your blanket）
// 与 page_discard_3 类似但显示 BLANKET_NAME
void page_discard_7()
{
  matrix.fillScreen(matrix.Color333(0, 0, 0));
  matrix.setCursor(0, 0*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.println("please discard");
  matrix.setCursor(0, 1*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.println("your wok");
  // 若需显示常量名也可替换为： matrix.println(BLANKET_NAME);
}

// 新增：page_discard_6（与 page_discard_4 显示格式相同，显示偏差）
void page_discard_8()
{
  matrix.fillScreen(matrix.Color333(0, 0, 0));
  matrix.setCursor(0, 0*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.println("... close, but");
  matrix.setCursor(0, 1*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.println("you are still");
  matrix.setCursor(0, 2*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.print(current_score);
  matrix.print(" grams off.");
}

void page_discard_9()
{
  matrix.fillScreen(matrix.Color333(0, 0, 0));
  matrix.setCursor(0, 0*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.println("please discard");
  matrix.setCursor(0, 1*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.println("your tent");
  // 若需显示常量名也可替换为： matrix.println(BLANKET_NAME);
}

void page_discard_10()
{
  matrix.fillScreen(matrix.Color333(0, 0, 0));
  matrix.setCursor(0, 0*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.println("... close, but");
  matrix.setCursor(0, 1*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.println("you are still");
  matrix.setCursor(0, 2*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.print(current_score);
  matrix.print(" grams off.");
}

void page_discard_11()
{
  matrix.fillScreen(matrix.Color333(0, 0, 0));
  matrix.setCursor(0, 0*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.println("please discard");
  matrix.setCursor(0, 1*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.println("your rice cooker");
  // 若需显示常量名也可替换为： matrix.println(BLANKET_NAME);
}

void page_discard_12()
{
  matrix.fillScreen(matrix.Color333(0, 0, 0));
  matrix.setCursor(0, 0*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.println("... close, but");
  matrix.setCursor(0, 1*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.println("you are still");
  matrix.setCursor(0, 2*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.print(current_score);
  matrix.print(" grams off.");
}

void page_discard_13()
{
  matrix.fillScreen(matrix.Color333(0, 0, 0));
  matrix.setCursor(0, 0*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.println("please discard");
  matrix.setCursor(0, 1*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.println("your 400cc blood");
  // 若需显示常量名也可替换为： matrix.println(BLANKET_NAME);
}

void page_discard_14()
{
  matrix.fillScreen(matrix.Color333(0, 0, 0));
  matrix.setCursor(0, 0*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.println("... close, but");
  matrix.setCursor(0, 1*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.println("you are still");
  matrix.setCursor(0, 2*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.print(current_score);
  matrix.print(" grams off.");
}

void page_discard_15()
{
  matrix.fillScreen(matrix.Color333(0, 0, 0));
  matrix.setCursor(0, 0*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.println("please discard");
  matrix.setCursor(0, 1*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.println("your kidney");
  // 若需显示常量名也可替换为： matrix.println(BLANKET_NAME);
}

void page_discard_16()
{
  matrix.fillScreen(matrix.Color333(0, 0, 0));
  matrix.setCursor(0, 0*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.println("... close, but");
  matrix.setCursor(0, 1*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.println("you are still");
  matrix.setCursor(0, 2*8);
  matrix.setTextColor(matrix.Color333(7,0,0));
  matrix.print(current_score);
  matrix.print(" grams off.");
}



void page_discard_final()
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



void show_weight()
{
  matrix.fillRect(0, 5*8, 30, 8, matrix.Color333(0, 0, 0));
  matrix.setCursor(0, 5*8); 
  matrix.setTextColor(matrix.Color333(7,9,7));

  //String wstr = String(Weight);
  matrix.println(Weight);
  Serial.print(Weight);	//串口显示重量
  Serial.print(" g\n");	//显示单位
  Serial.print("\n");		//显示单位
}

// 新增：更新排行榜函数（小分优先，0 不入榜）
void updateLeaderboard(long score)
{
  if (score <= 0) return; // 0 或负数不进入排名

  // 如果比当前第3名（最差）更好（更小），放入第3名位置
  if (score < leaderboard[2]) {
    leaderboard[2] = score;
  } else {
    return; // 不是更好也不需要排序
  }

  // 冒泡式向上调整，确保升序排列（越小越靠前）
  if (leaderboard[2] < leaderboard[1]) {
    long temp = leaderboard[1];
    leaderboard[1] = leaderboard[2];
    leaderboard[2] = temp;
  }
  if (leaderboard[1] < leaderboard[0]) {
    long temp = leaderboard[0];
    leaderboard[0] = leaderboard[1];
    leaderboard[1] = temp;
  }

  Serial.print("Leaderboard updated: ");
  for (int i = 0; i < 3; ++i) {
    if (leaderboard[i] == 0x7FFFFFFF) Serial.print("--");
    else Serial.print(leaderboard[i]);
    if (i < 2) Serial.print(" ");
  }
  Serial.println();
}

// 新增：排行榜页面显示函数（未占位显示为 "--"）
void page_leaderboard()
{
  matrix.fillScreen(matrix.Color333(0, 0, 0));
  
  matrix.setCursor(8, 0*8);
  matrix.setTextColor(matrix.Color333(7, 7, 0));
  matrix.println("LEADERBOARD");
  
  matrix.setCursor(4, 1*8);
  matrix.setTextColor(matrix.Color333(0, 7, 7));
  matrix.print("1: ");
  if (leaderboard[0] == 0x7FFFFFFF) matrix.println("--");
  else matrix.println(leaderboard[0]);
  
  matrix.setCursor(4, 2*8);
  matrix.setTextColor(matrix.Color333(7, 7, 0));
  matrix.print("2: ");
  if (leaderboard[1] == 0x7FFFFFFF) matrix.println("--");
  else matrix.println(leaderboard[1]);
  
  matrix.setCursor(4, 3*8);
  matrix.setTextColor(matrix.Color333(7, 0, 7));
  matrix.print("3: ");
  if (leaderboard[2] == 0x7FFFFFFF) matrix.println("--");
  else matrix.println(leaderboard[2]);
  
  matrix.setCursor(4, 5*8);
  matrix.setTextColor(matrix.Color333(7, 7, 7));
  matrix.println("Press for next");
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
