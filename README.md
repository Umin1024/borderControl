# borderControl
amaze 2026
![intallation example](6c9ce398668a3fd84eecf0d062fcfc6e.png)

### Index
#### 一 游戏流程介绍
#### 二 设备完整接线图（和电子器件清单）
#### 三 接口细节
#### 四 一些常见问题的解决方式和可能原因
#### 五 代码链接。代码的架构（如何修改调整排版）
#### 六 新电脑如何安装库与注意事项
---


### 一 游戏流程与页面说明

#### 1. 总流程

`START` → `INTRO 1~5` → `LEVEL 1~8`（每关 2 页：`PROMPT` + `RESULT`）→ `FINAL` → `ACCESS RESULT` → `LEADERBOARD` → `START`

#### 2. 详细规则

1. 等待玩家：
显示 START 页面，玩家站上称激活游戏（检测到重量从低于 25 kg 变为高于 25 kg 时进入 INTRO）。
调试模式（`TEST_MODE 1`）跳过重量判定，直接进入 INTRO。
2. 游戏介绍：
玩家在称上站立不动，屏幕自动播放 5 页游戏介绍，每页停留 4 秒。
3. 进入关卡：
依次进入八个关卡。每关随机从 3 个候选题目中抽取 1 个，题目页右下角显示红色倒计时，共 12 秒（先从 10 倒数到 0，最后 2 秒 "0s" 闪烁三次后自动翻页）。进入第一关题目页时记录当前重量为基准值。
4. 关卡结算：
倒计时结束后冻结测量重量，计算与上一次记录重量的差值，和题目标准克重比较得到本关误差分数。RESULT 页显示误差，停留 6 秒后进入下一关。
5. 所有关卡结束：
八关结束后进入 FINAL 页，显示累计总误差，红色字，停留 10 秒后进入审核页。
6. 审核判定：
ACCESS RESULT 页判断总误差是否小于 5000 g。通过则绿色字并亮绿灯；不通过则红色字并亮红灯。停留 10 秒。
7. 排行榜与重置：
进入排行榜页，大字显示 `BEST`，右侧三行显示历史最佳 3 个分数，空位显示 `xxx`。停留 20 秒并等待玩家离开承重区域（重量降至 25 kg 以下）后，回到 START 页。

#### 3. 各页面显示内容

| 页面 | 实际显示文字 | 颜色 | 停留时间 |
| --- | --- | --- | --- |
| `START` | `Please step onto the platform, with all your belongings.` | 红 | 等待触发 |
| `INTRO 1` | `Welcome to the Great Nation's Border Control!` | 红 | 4 秒 |
| `INTRO 2` | `Please step on the platform with all your belongings.` | 红 | 4 秒 |
| `INTRO 3` | `Do not remove any items until instructed.` | 红 | 4 秒 |
| `INTRO 4` | `Detecting ...`（第一行空，第二行起） | 红 | 4 秒 |
| `INTRO 5` | `Prohibited items found! ☹`（第一行空，第二行起） | 红 | 4 秒 |
| `LEVEL n / PROMPT` | `Please discard the following item:  <item name>` | 红 | 12 秒（含 2 秒闪烁） |
| `LEVEL n / RESULT` | `The weight discarded was off by:  <误差> g` | 红 | 6 秒 |
| `FINAL` | `Cumulative weight discrepancy so far:  <总误差> g` | 红 | 10 秒 |
| `ACCESS RESULT` | `Congratulations. Your entry has been approved.` / `Unfortunately. the weight discrepancy was too large.` | 绿 / 红 | 10 秒 |
| `LEADERBOARD` | 大字 `BEST`，右侧 `1.` `2.` `3.` 各行分数 | 黄 | 20 秒 + 等离场 |

#### 4. 通过条件

- 通过阈值在 `config.h` 的 `PASS_THRESHOLD_G`，当前值为 `5000`（g）。
- 规则：`累计总误差 < PASS_THRESHOLD_G` 即通过，ACCESS RESULT 页显示绿色并亮绿灯。
---




### 二 设备完整接线图（和电子器件清单）

R1_PIN_DEFAULT 4 <br>
G1_PIN_DEFAULT 5 <br>
B1_PIN_DEFAULT 6 <br>
R2_PIN_DEFAULT 7<br>
G2_PIN_DEFAULT 15<br>
B2_PIN_DEFAULT 16<br>
A_PIN_DEFAULT  18<br>
B_PIN_DEFAULT  8<br>
C_PIN_DEFAULT  3<br>
D_PIN_DEFAULT  42<br>
E_PIN_DEFAULT  17 // required for 1/32 scan panels<br>
LAT_PIN_DEFAULT 40<br>
OE_PIN_DEFAULT  2<br>
CLK_PIN_DEFAULT 41<br>

称的部分:<br>
VCC 需要接5v<br>
HX711_SCK 13  // PORTA0 (esp32 S3)<br>
HX711_DT 14   // PORTL (esp32 S3)<br>
GND

---

### 三 图片

![分压板电路图](2e3213f481d069c567e93a28c88c5a1f.jpg)
![relay-light](relay-5v-1-channel-arduino.jpg)
![HUB75](HUB75.PNG)
---

### 四 一些常见问题的解决方式和可能原因

---

### 五 代码链接。代码的架构（如何修改调整排版）

#### 1. 从哪里开始看
1. 只测试屏幕动画：`borderControl\borderControl_game\src\test_pix\2_PatternPlasma.ino`
2. 测试屏幕、灯、称是否都正常：`borderControl\borderControl_game\src\test_matrix\test_matrix.ino`
3. 正式游戏程序入口：`borderControl\borderControl_game\src\main\main.ino`
4. `borderControl\borderControl_game\src\test\test.ino` 是较早的一体化版本，现在主要作为旧逻辑参考，不是当前主入口。

#### 2. 正式程序的文件分工
- `borderControl_game\src\main\main.ino`
  - 程序入口。
  - `setup()` 负责初始化屏幕、红绿灯、HX711，并做两次 tare（传感器预热后重新归零）。
  - `loop()` 有两条主线：按 `WEIGHT_INTERVAL_MS`（500ms）读取重量；按页面计时规则自动切页。
  - `displayPage()` 是总路由，根据页码调用对应的显示函数。
  - `calcScore()` 负责计算当前关卡误差，`updateLeaderboard()` 负责更新排行榜。
  - `updatePageOverlay()` 负责在关卡题目页右下角绘制红色倒计时（10→0s，最后 2 秒闪烁）。
- `borderControl_game\src\main\config.h`
  - 所有全局参数：引脚、屏幕尺寸、称校准系数 `HX711_GAP`、各页面停留时间、关卡数量、排行榜大小、通过阈值 `PASS_THRESHOLD_G`。
  - `TEST_MODE`：设为 1 时跳过称重触发，直接进入游戏；串口发送 `n` 手动跳到下一页。
  - 想改节奏、页数、硬件参数，优先看这个文件。
- `borderControl_game\src\main\game_content.h / game_content.cpp`
  - 游戏内容与页面文字的唯一入口。
  - `INTRO_TEXTS`：5 页开场文案。
  - `LEVELS`：8 关题目和标准克重，每关 `QUESTIONS_PER_LEVEL`（3）个变体，开局随机抽一个。
  - `pageLabel()` / `pageText()` / `pageColor()`：每个页面显示什么文字、什么颜色，全在这里决定。
  - 想改文案、物品名字、标准克重、页面颜色，改这里。
- `borderControl_game\src\main\display.h / display.cpp`
  - 纯显示层，不做分数和流程判断。
  - 负责文字换行、边距（上下左右各 2px）、首行缩进（2 字符）、排行榜专用排版。
  - 想改排版、字号、换行方式、文字位置，改这里。
- `borderControl_game\src\main\HX711.h / HX711.cpp`
  - 称重驱动。`begin()` 记录空载基准，`Get_Weight()` 返回当前克重（g）。
  - 内含临界区保护，避免 LED DMA 刷屏打断 HX711 时序读数。
  - 称不稳、读数异常，优先看这里。

#### 3. 当前程序实际流程（以 `borderControl_game\src\main\main.ino` 为准）

`PAGE_START` → 5 页 INTRO（各 4s）→ 8 关 × （PROMPT 12s + RESULT 6s）→ FINAL 10s → ACCESS RESULT 10s → LEADERBOARD 20s → 回到 `PAGE_START`

各页面停留时间由 `config.h` 中独立常量控制：

| 常量 | 默认值 | 对应页面 |
| --- | --- | --- |
| `INTRO_PAGE_MS` | 4000 ms | 每页 INTRO |
| `PROMPT_PAGE_MS` | 12000 ms | 关卡题目页（含 2s 闪烁） |
| `PROMPT_COUNTDOWN_MS` | 10000 ms | 倒计时显示的时长部分 |
| `RESULT_PAGE_MS` | 6000 ms | 关卡结果页 |
| `FINAL_PAGE_MS` | 10000 ms | 总分页 |
| `ACCESS_RESULT_PAGE_MS` | 10000 ms | 审核结果页 |
| `LEADERBOARD_PAGE_MS` | 20000 ms | 排行榜页（另需等玩家离场） |

#### 4. 常见修改对应改哪里
- 改文案、页面颜色：`game_content.cpp`
- 改题目和标准克重：`game_content.cpp` 里的 `LEVELS`
- 改页数、停留时间、硬件参数、校准系数、调试开关：`config.h`
- 改排版、边距、换行方式、字体大小、文字位置：`display.cpp`
- 改流程推进方式、计分逻辑、排行榜逻辑、倒计时动画：`main.ino`
- 改称重底层读取方式：`HX711.cpp`

#### 5. 哪些文件通常不要动
- `borderControl_game\src\ESP32-HUB75-MatrixPanel-I2S-DMA.*`
- `borderControl_game\src\ESP32-HUB75-VirtualMatrixPanel_T.hpp`
- `borderControl_game\src\ESP32-VirtualMatrixPanel-I2S-DMA.h`
- `borderControl_game\src\platforms\...`
- `borderControl_game\src\cie_luts.h`

这些基本是 HUB75 屏幕驱动库源码。除非你在修底层驱动问题，否则平时改 `src\main` 下面几个文件就够了。
---

### 六 新电脑如何安装库与注意事项 



todo
<!-- 1. 称校准。
    称的安装（30min）
    系数校准（30min）
    解决走线过长数据不稳问题（1h）
    电路是否增加电容（1h）（优先级低）
2. 整体架构（能快速找出问题，方便模块化修改）(优先级低)
   硬件功能测试（1h）


3. 游戏功能测试（30min-1h）
4. 扩展游戏，增加承重选项（非编程，1h）
5. 整体游戏测试，全游戏流程测试（1h）
6. 布线设计 优化（2h）
7. 模块化文档（2h）
