#include "HX711.h"
#include <freertos/portmacro.h>

// 定义 ESP32 双核自旋锁（解决 LED 时序冲突的关键！）
portMUX_TYPE hx711_mux = portMUX_INITIALIZER_UNLOCKED;
HX711::HX711(int SCK_PIN, int DT_PIN, float GapValueIn)
{
	HX711_SCK = SCK_PIN;
	HX711_DT = DT_PIN;
	ValueGap = GapValueIn;
}

//****************************************************
// 初始化HX711
//****************************************************
void HX711::begin()
{
	pinMode(HX711_SCK, OUTPUT);
	pinMode(HX711_DT, INPUT);

	Get_Maopi();
}

//****************************************************
// 获取毛皮重量
//****************************************************
void HX711::Get_Maopi()
{
	Weight_Maopi = HX711_Read();
}

//****************************************************
// 称重
//****************************************************
long HX711::Get_Weight()
{
	HX711_Buffer = HX711_Read();

	Weight_Shiwu = HX711_Buffer;
	Weight_Shiwu = Weight_Shiwu - Weight_Maopi; // 获取实物的AD采样数值。

	Weight_Shiwu = (long)((float)Weight_Shiwu / ValueGap - 0.05); // 修改

	return Weight_Shiwu;
}

//****************************************************
// 读取HX711 (安全版，防 LED 干扰 + 防死机)
//****************************************************
unsigned long HX711::HX711_Read()
{
	unsigned long count = 0;
	unsigned char i;
	int timeout = 10000; // 超时防止死机

	digitalWrite(HX711_DT, HIGH);
	delayMicroseconds(1);
	digitalWrite(HX711_SCK, LOW);
	delayMicroseconds(1);

	// 1. 等待数据准备好，加入超时机制，防止拔掉传感器后死机
	while (digitalRead(HX711_DT))
	{
		timeout--;
		if (timeout == 0)
			return 0; // 如果超时没读到，返回0
		delayMicroseconds(1);
	}

	// 2. ⚡开启中断保护，防止被 LED 的 DMA 打断！⚡
	portENTER_CRITICAL(&hx711_mux);

	// 3. 严格的 24 bit 读取时序
	for (i = 0; i < 24; i++)
	{
		digitalWrite(HX711_SCK, HIGH);
		delayMicroseconds(1);
		count = count << 1;
		digitalWrite(HX711_SCK, LOW);
		delayMicroseconds(1);
		if (digitalRead(HX711_DT))
			count++;
	}
	digitalWrite(HX711_SCK, HIGH);
	delayMicroseconds(1);
	digitalWrite(HX711_SCK, LOW);
	delayMicroseconds(1);

	// 4. ⚡关闭中断保护，让 LED 恢复工作⚡
	portEXIT_CRITICAL(&hx711_mux);

	count ^= 0x800000;
	return count;
}
