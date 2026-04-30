# borderControl
amaze 2026

一 游戏流程介绍
二 设备完整接线图（和电子器件清单）
三 接口细节
四 一些常见问题的解决方式和可能原因
五 代码链接。代码的架构（如何修改调整排版）
六 新电脑如何安装库与注意事项



一 游戏流程 

1. 玩家站到称上激活游戏（检测到大于25kg的重量开始游戏界面）
2. 玩家站立不动，自动播放游戏介绍跟流程，每个页面停留4秒（三选一主题）
3. 进入关卡：每关播放题目，右下角倒数10秒。
4. 关卡结算：10秒后冻结重量，计算差值，播放结果停留4秒后，进入下一关卡。
5. 所有关卡流程结束后，计算总分，总分界面停留10秒，如果通过游戏，亮起绿灯；如果不能通过，亮起红灯
6. 最后进入排行榜界面

二 接线：

#define R1_PIN_DEFAULT 4
#define G1_PIN_DEFAULT 5
#define B1_PIN_DEFAULT 6
#define R2_PIN_DEFAULT 7
#define G2_PIN_DEFAULT 15
#define B2_PIN_DEFAULT 16
#define A_PIN_DEFAULT  18
#define B_PIN_DEFAULT  8
#define C_PIN_DEFAULT  3
#define D_PIN_DEFAULT  42
#define E_PIN_DEFAULT  17 // required for 1/32 scan panels, like 64x64. Any available pin would do, i.e. IO32
#define LAT_PIN_DEFAULT 40
#define OE_PIN_DEFAULT  2
#define CLK_PIN_DEFAULT 41

// #define HX711_SCK 13  // PORTA0 (esp32 S3)
// #define HX711_DT 14   // PORTL (esp32 S3)