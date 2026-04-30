
// Core logic ported from alt_ctrl_weight.ino, adapted to ESP32-HUB75-MatrixPanel-I2S-DMA.
// 删除了原物理按钮逻辑，改为每 3 秒自动触发“虚拟按钮”。

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "Hx711.h"
#include "config.h"

MatrixPanel_I2S_DMA *dma_display = nullptr;

unsigned long Weight = 0;
unsigned long new_Weight = 0;
unsigned long lastWeightMillis = 0;
unsigned long lastPageMillis = 0;
unsigned long lastDisplayedWeight = 0xFFFFFFFFUL;
unsigned long initialWeight = 0;
long player_score = 0;
long current_score = 0;
long leaderboard[3] = {0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF};
uint8_t currentPage = PAGE_START;

uint16_t color333(uint8_t r, uint8_t g, uint8_t b) {
  // 将 0-7 范围映射到 0-255 8bit 颜色
  return dma_display->color565(r * 36, g * 36, b * 36);
}

void setup() {
  Init_Hx711();
  Get_Maopi();

  HUB75_I2S_CFG mxconfig;
  mxconfig.mx_height = PANEL_RES_Y;
  mxconfig.mx_width = PANEL_RES_X;
  mxconfig.chain_length = PANEL_CHAIN;
  mxconfig.gpio.e = PIN_E;

  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(255);
  dma_display->fillScreen(dma_display->color565(0, 0, 0));
  dma_display->setTextWrap(false);
  dma_display->setTextSize(1);

  displayPage(PAGE_START);
  show_weight();
  lastPageMillis = millis();
}

void loop() {
  // 虚拟按钮：每 3 秒自动翻页一次
  if (millis() - lastPageMillis >= PAGE_AUTO_INTERVAL_MS) {
    lastPageMillis = millis();
    nextPage();
  }

  // 每 500ms 读取一次重量，变化时刷新显示
  if (millis() - lastWeightMillis >= WEIGHT_INTERVAL_MS) {
    lastWeightMillis = millis();
    unsigned long measured = Get_Weight();
    if (measured < DISPLAY_DEADZONE) measured = 0;

    if (measured != lastDisplayedWeight) {
      lastDisplayedWeight = measured;
      Weight = measured;
      show_weight();
    }
  }

  delay(20);
}

void nextPage(void) {
  currentPage++;
  if (currentPage >= TOTAL_PAGES) {
    currentPage = PAGE_START;
  }
  displayPage(currentPage);
  show_weight();
}

void previousPage(void) {
  if (currentPage == 0) {
    currentPage = TOTAL_PAGES - 1;
  } else {
    currentPage--;
  }
  displayPage(currentPage);
}

void show_current_score(unsigned long item_weight) {
  new_Weight = Weight;
  unsigned long diff = (new_Weight >= initialWeight) ? (new_Weight - initialWeight) : (initialWeight - new_Weight);
  long score = (long)diff - (long)item_weight;
  if (score < 0) score = -score;
  current_score = score;
  player_score += current_score;
  initialWeight = new_Weight;
}

void displayPage(uint8_t pageNum) {
  switch(pageNum) {
    case PAGE_START:
      page_start_0();
      break;
    case PAGE_MSG1:
      page_msg1();
      break;
    case PAGE_MSG2:
      page_msg2();
      break;
    case PAGE_MSG3:
      page_msg3();
      break;
    case PAGE_MSG4:
      page_msg4();
      break;
    case PAGE_MSG5:
      page_msg5();
      break;
    case PAGE_DISCARD_1:
      initialWeight = Weight;
      lastDisplayedWeight = Weight;
      page_discard_1();
      break;
    case PAGE_DISCARD_2:
      show_current_score(740);
      page_discard_2();
      break;
    case PAGE_DISCARD_3:
      page_discard_3();
      break;
    case PAGE_DISCARD_4:
      show_current_score(1200);
      page_discard_4();
      break;
    case PAGE_DISCARD_5:
      page_discard_5();
      break;
    case PAGE_DISCARD_6:
      show_current_score(1500);
      page_discard_6();
      break;
    case PAGE_DISCARD_7:
      page_discard_7();
      break;
    case PAGE_DISCARD_8:
      show_current_score(2110);
      page_discard_8();
      break;
    case PAGE_DISCARD_9:
      page_discard_9();
      break;
    case PAGE_DISCARD_10:
      show_current_score(2500);
      page_discard_10();
      break;
    case PAGE_DISCARD_11:
      page_discard_11();
      break;
    case PAGE_DISCARD_12:
      show_current_score(4500);
      page_discard_12();
      break;
    case PAGE_DISCARD_13:
      page_discard_13();
      break;
    case PAGE_DISCARD_14:
      show_current_score(400);
      page_discard_14();
      break;
    case PAGE_DISCARD_15:
      page_discard_15();
      break;
    case PAGE_DISCARD_16:
      show_current_score(150);
      page_discard_16();
      break;
    case PAGE_DISCARD_FINAL:
      page_discard_final();
      break;
    case PAGE_LEADERBOARD:
      updateLeaderboard(player_score);
      page_leaderboard();
      break;
    default:
      currentPage = PAGE_START;
      page_start_0();
      break;
  }
}

void page_start_0() {
  player_score = 0;
  current_score = 0;
  Weight = 0;
  initialWeight = 0;
  new_Weight = 0;

  dma_display->fillScreen(dma_display->color565(0, 0, 0));
  dma_display->setTextSize(1);
  dma_display->setTextWrap(false);
  dma_display->setCursor(0, 0);
  dma_display->setTextColor(color333(7, 0, 0));
  dma_display->println("press start!");
}

void page_msg1() {
  dma_display->fillScreen(dma_display->color565(0,0,0));
  dma_display->setTextSize(1);
  dma_display->setTextWrap(true);
  dma_display->setCursor(0, 0);
  dma_display->setTextColor(color333(7,0,0));
  dma_display->println("welcome to the");
  dma_display->println("great nation's");
  dma_display->println("port of entry");
}

void page_msg2() {
  dma_display->fillScreen(dma_display->color565(0,0,0));
  dma_display->setTextSize(1);
  dma_display->setTextWrap(true);
  dma_display->setCursor(0, 0);
  dma_display->setTextColor(color333(7,0,0));
  dma_display->println("step onto the");
  dma_display->println("scale with all");
  dma_display->println("your belongings");
}

void page_msg3() {
  dma_display->fillScreen(dma_display->color565(0,0,0));
  dma_display->setTextSize(1);
  dma_display->setTextWrap(true);
  dma_display->setCursor(0, 0);
  dma_display->setTextColor(color333(7,0,0));
  dma_display->println("do not remove");
  dma_display->println("anything until");
  dma_display->println("instructed");
}

void page_msg4() {
  dma_display->fillScreen(dma_display->color565(0,0,0));
  dma_display->setTextSize(1);
  dma_display->setTextWrap(true);
  dma_display->setCursor(0, 0);
  dma_display->setTextColor(color333(7,0,0));
  dma_display->println("...");
}

void page_msg5() {
  dma_display->fillScreen(dma_display->color565(0,0,0));
  dma_display->setTextSize(1);
  dma_display->setTextWrap(true);
  dma_display->setCursor(0, 0);
  dma_display->setTextColor(color333(7,0,0));
  dma_display->println("foreign and");
  dma_display->println("prohibited items");
  dma_display->println("detected");
}

void page_discard_1() {
  dma_display->fillScreen(dma_display->color565(0, 0, 0));
  dma_display->setTextSize(1);
  dma_display->setTextWrap(false);
  dma_display->setCursor(0, 0);
  dma_display->setTextColor(color333(7,0,0));
  dma_display->println("please discard");
  dma_display->setCursor(0, 8);
  dma_display->println("your jar of spice");
}

void page_discard_2() {
  dma_display->fillScreen(dma_display->color565(0, 0, 0));
  dma_display->setTextSize(1);
  dma_display->setTextWrap(false);
  dma_display->setCursor(0, 0);
  dma_display->setTextColor(color333(7,0,0));
  dma_display->println("... close, but");
  dma_display->setCursor(0, 8);
  dma_display->println("you are still");
  dma_display->setCursor(0, 16);
  dma_display->println(String(current_score) + " grams off.");
}

void page_discard_3() {
  dma_display->fillScreen(dma_display->color565(0, 0, 0));
  dma_display->setTextSize(1);
  dma_display->setTextWrap(false);
  dma_display->setCursor(0, 0);
  dma_display->setTextColor(color333(7,0,0));
  dma_display->println("please discard");
  dma_display->setCursor(0, 8);
  dma_display->println("your kettle");
}

void page_discard_4() {
  dma_display->fillScreen(dma_display->color565(0, 0, 0));
  dma_display->setTextSize(1);
  dma_display->setTextWrap(false);
  dma_display->setCursor(0, 0);
  dma_display->setTextColor(color333(7,0,0));
  dma_display->println("... close, but");
  dma_display->setCursor(0, 8);
  dma_display->println("you are still");
  dma_display->setCursor(0, 16);
  dma_display->println(String(current_score) + " grams off.");
}

void page_discard_5() {
  dma_display->fillScreen(dma_display->color565(0, 0, 0));
  dma_display->setTextSize(1);
  dma_display->setTextWrap(false);
  dma_display->setCursor(0, 0);
  dma_display->setTextColor(color333(7,0,0));
  dma_display->println("please discard");
  dma_display->setCursor(0, 8);
  dma_display->println("your blanket");
}

void page_discard_6() {
  dma_display->fillScreen(dma_display->color565(0, 0, 0));
  dma_display->setTextSize(1);
  dma_display->setTextWrap(false);
  dma_display->setCursor(0, 0);
  dma_display->setTextColor(color333(7,0,0));
  dma_display->println("... close, but");
  dma_display->setCursor(0, 8);
  dma_display->println("you are still");
  dma_display->setCursor(0, 16);
  dma_display->println(String(current_score) + " grams off.");
}

void page_discard_7() {
  dma_display->fillScreen(dma_display->color565(0, 0, 0));
  dma_display->setTextSize(1);
  dma_display->setTextWrap(false);
  dma_display->setCursor(0, 0);
  dma_display->setTextColor(color333(7,0,0));
  dma_display->println("please discard");
  dma_display->setCursor(0, 8);
  dma_display->println("your wok");
}

void page_discard_8() {
  dma_display->fillScreen(dma_display->color565(0, 0, 0));
  dma_display->setTextSize(1);
  dma_display->setTextWrap(false);
  dma_display->setCursor(0, 0);
  dma_display->setTextColor(color333(7,0,0));
  dma_display->println("... close, but");
  dma_display->setCursor(0, 8);
  dma_display->println("you are still");
  dma_display->setCursor(0, 16);
  dma_display->println(String(current_score) + " grams off.");
}

void page_discard_9() {
  dma_display->fillScreen(dma_display->color565(0, 0, 0));
  dma_display->setTextSize(1);
  dma_display->setTextWrap(false);
  dma_display->setCursor(0, 0);
  dma_display->setTextColor(color333(7,0,0));
  dma_display->println("please discard");
  dma_display->setCursor(0, 8);
  dma_display->println("your tent");
}

void page_discard_10() {
  dma_display->fillScreen(dma_display->color565(0, 0, 0));
  dma_display->setTextSize(1);
  dma_display->setTextWrap(false);
  dma_display->setCursor(0, 0);
  dma_display->setTextColor(color333(7,0,0));
  dma_display->println("... close, but");
  dma_display->setCursor(0, 8);
  dma_display->println("you are still");
  dma_display->setCursor(0, 16);
  dma_display->println(String(current_score) + " grams off.");
}

void page_discard_11() {
  dma_display->fillScreen(dma_display->color565(0, 0, 0));
  dma_display->setTextSize(1);
  dma_display->setTextWrap(false);
  dma_display->setCursor(0, 0);
  dma_display->setTextColor(color333(7,0,0));
  dma_display->println("please discard");
  dma_display->setCursor(0, 8);
  dma_display->println("your rice cooker");
}

void page_discard_12() {
  dma_display->fillScreen(dma_display->color565(0, 0, 0));
  dma_display->setTextSize(1);
  dma_display->setTextWrap(false);
  dma_display->setCursor(0, 0);
  dma_display->setTextColor(color333(7,0,0));
  dma_display->println("... close, but");
  dma_display->setCursor(0, 8);
  dma_display->println("you are still");
  dma_display->setCursor(0, 16);
  dma_display->println(String(current_score) + " grams off.");
}

void page_discard_13() {
  dma_display->fillScreen(dma_display->color565(0, 0, 0));
  dma_display->setTextSize(1);
  dma_display->setTextWrap(false);
  dma_display->setCursor(0, 0);
  dma_display->setTextColor(color333(7,0,0));
  dma_display->println("please discard");
  dma_display->setCursor(0, 8);
  dma_display->println("your 400cc blood");
}

void page_discard_14() {
  dma_display->fillScreen(dma_display->color565(0, 0, 0));
  dma_display->setTextSize(1);
  dma_display->setTextWrap(false);
  dma_display->setCursor(0, 0);
  dma_display->setTextColor(color333(7,0,0));
  dma_display->println("... close, but");
  dma_display->setCursor(0, 8);
  dma_display->println("you are still");
  dma_display->setCursor(0, 16);
  dma_display->println(String(current_score) + " grams off.");
}

void page_discard_15() {
  dma_display->fillScreen(dma_display->color565(0, 0, 0));
  dma_display->setTextSize(1);
  dma_display->setTextWrap(false);
  dma_display->setCursor(0, 0);
  dma_display->setTextColor(color333(7,0,0));
  dma_display->println("please discard");
  dma_display->setCursor(0, 8);
  dma_display->println("your kidney");
}

void page_discard_16() {
  dma_display->fillScreen(dma_display->color565(0, 0, 0));
  dma_display->setTextSize(1);
  dma_display->setTextWrap(false);
  dma_display->setCursor(0, 0);
  dma_display->setTextColor(color333(7,0,0));
  dma_display->println("... close, but");
  dma_display->setCursor(0, 8);
  dma_display->println("you are still");
  dma_display->setCursor(0, 16);
  dma_display->println(String(current_score) + " grams off.");
}

void page_discard_final() {
  dma_display->fillScreen(dma_display->color565(0, 0, 0));
  dma_display->setTextSize(1);
  dma_display->setTextWrap(false);
  dma_display->setCursor(0, 0);
  dma_display->setTextColor(color333(0, 7, 0));
  dma_display->println("Final Deviation:");
  dma_display->setCursor(0, 16);
  dma_display->setTextColor(color333(7, 7, 0));
  dma_display->print(player_score);
  dma_display->println(" g");
  dma_display->setCursor(0, 32);
  dma_display->setTextColor(color333(7, 0, 7));
  dma_display->println("The screening has");
  dma_display->println("been completed.");
}

void show_weight() {
  dma_display->setTextSize(1);
  dma_display->setTextWrap(false);
  dma_display->fillRect(0, 40, dma_display->width(), 8, dma_display->color565(0, 0, 0));
  dma_display->setCursor(0, 40);
  //dma_display->setTextColor(color333(0, 7, 0));
  dma_display->print(Weight);
  dma_display->print(" g");
}

void updateLeaderboard(long score) {
  if (score <= 0) return;
  if (score < leaderboard[2]) {
    leaderboard[2] = score;
  } else {
    return;
  }
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
}

void page_leaderboard() {
  dma_display->fillScreen(dma_display->color565(0, 0, 0));
  dma_display->setTextSize(1);
  dma_display->setTextWrap(false);
  dma_display->setCursor(0, 0);
  dma_display->setTextColor(color333(7, 7, 0));
  dma_display->println("LEADERBOARD");
  dma_display->setCursor(4, 8);
  dma_display->setTextColor(color333(0, 7, 7));
  dma_display->print("1: ");
  dma_display->println(leaderboard[0] == 0x7FFFFFFF ? String("--") : String(leaderboard[0]));
  dma_display->setCursor(4, 16);
  dma_display->setTextColor(color333(7, 7, 0));
  dma_display->print("2: ");
  dma_display->println(leaderboard[1] == 0x7FFFFFFF ? String("--") : String(leaderboard[1]));
  dma_display->setCursor(4, 24);
  dma_display->setTextColor(color333(7, 0, 7));
  dma_display->print("3: ");
  dma_display->println(leaderboard[2] == 0x7FFFFFFF ? String("--") : String(leaderboard[2]));
  dma_display->setCursor(4, 40);
  dma_display->setTextColor(color333(7, 7, 7));
  dma_display->println("Next page in 3s");
}
