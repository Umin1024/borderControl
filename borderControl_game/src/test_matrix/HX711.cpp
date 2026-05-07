#include "hx711.h"
#include <freertos/portmacro.h>

static portMUX_TYPE hx711Mux = portMUX_INITIALIZER_UNLOCKED;
unsigned long HX711_Buffer = 0;
unsigned long Weight_Maopi = 0;
long Weight_Shiwu = 0;
bool Flag_Error = 0;

//****************************************************
//初始化HX711
//****************************************************
void Init_Hx711()
{
	pinMode(HX711_SCK, OUTPUT);	
	pinMode(HX711_DT, INPUT);      // 确保 DT 是输入模式
	digitalWrite(HX711_SCK, LOW);  // 初始状态
}


//****************************************************
//获取毛皮重量
//****************************************************
void Get_Maopi()
{
	Weight_Maopi = HX711_Read();		
} 

//****************************************************
//称重
//****************************************************
unsigned long Get_Weight()
{
	HX711_Buffer = HX711_Read();
	if(HX711_Buffer >= Weight_Maopi)			
	{
		Weight_Shiwu = HX711_Buffer;
		Weight_Shiwu = Weight_Shiwu - Weight_Maopi;				//获取实物的AD采样数值。
	
		Weight_Shiwu = (unsigned long)((float)Weight_Shiwu/21.5+0.05); 	
			//计算实物的实际重量
			//因为不同的传感器特性曲线不一样，因此，每一个传感器需要矫正这里的40这个除数。
			//当发现测试出来的重量偏大时，增加该数值。
			//如果测试出来的重量偏小时，减小改数值。
			//该数值一般在20-50之间。因传感器不同而定。
			//+0.05是为了四舍五入百分位
                if(Weight_Shiwu > 100000)
        	{
        		Flag_Error = 1;
        		Serial.print("Error1");
        	}
                else
        	{
        		Flag_Error = 0;	
        	}	
	}
        else if(HX711_Buffer > Weight_Maopi - 3000)
        {
            Flag_Error = 0;	
            Weight_Shiwu = 0;
        }
        else
        {
           Flag_Error = 1;
           Serial.print("Error2"); 
        }

	
	
	return Weight_Shiwu;
}

//****************************************************
//读取HX711
//****************************************************
unsigned long HX711_Read(void)	//增益128
{
	unsigned long count; 
	unsigned char i;
	bool Flag = 0;

	digitalWrite(HX711_DT, HIGH);
	delayMicroseconds(1);

	digitalWrite(HX711_SCK, LOW);
	delayMicroseconds(1);

  	count=0; 
  	// 等待 DATA 线变为低电平（数据就绪）
  	while(digitalRead(HX711_DT)); 

  	// 关键的 24 位时序读出：在此段禁用中断，避免矩阵库的 Timer1 ISR 干扰
  	taskENTER_CRITICAL(&hx711Mux);
  	for(i=0;i<24;i++)
	{ 
	  	digitalWrite(HX711_SCK, HIGH); 
		delayMicroseconds(1);
	  	count=count<<1; 
		digitalWrite(HX711_SCK, LOW); 
		delayMicroseconds(1);
	  	if(digitalRead(HX711_DT))
			count++; 
	} 
 	digitalWrite(HX711_SCK, HIGH); 
	delayMicroseconds(1);
	digitalWrite(HX711_SCK, LOW); 
	delayMicroseconds(1);
	taskEXIT_CRITICAL(&hx711Mux); // 恢复中断

	count ^= 0x800000;
	return(count);
}
