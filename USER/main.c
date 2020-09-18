#include "stm32f10x.h"  //包含需要的头文件
#include "main.h"       //包含需要的头文件
#include "delay.h"      //包含需要的头文件
#include "usart1.h"     //包含需要的头文件
#include "usart2.h"     //包含需要的头文件
#include "timer1.h"     //包含需要的头文件
#include "timer2.h"     //包含需要的头文件
#include "timer3.h"     //包含需要的头文件
#include "timer4.h"     //包含需要的头文件
#include "wifi.h"	    //包含需要的头文件
#include "led.h"        //包含需要的头文件
#include "mqtt.h"       //包含需要的头文件
#include "key.h"        //包含需要的头文件
#include "dht11.h"
#include "iic.h"        //包含需要的头文件
#include "adc.h"
#include "ds18b20.h"
#include "process.h"
#include "cJSON.h"
#include "stmflash.h"
#include "rtc.h"

//程序软件复位
void SYS_RST(void) {
	__set_FAULTMASK(1);
	NVIC_SystemReset();
}

int main(void) 
{

    Delay_Init();                   //延时功能初始化              
	Usart1_Init(115200);              //串口1功能初始化，波特率9600
	Usart2_Init(115200);            //串口2功能初始化，波特率115200	
	TIM4_Init(300,7200);            //TIM4初始化，定时时间 300*7200*1000/72000000 = 30ms
    
    LED_Init();	                    //LED初始化
    EXTIX_Init();
    Adc_Init();
	RTC_Init();	  			//RTC初始化
    while(DS18B20_Init())	//DS18B20初始化	
	{
 		delay_ms(200);
        u1_printf("温度传感器初始化\r\n");
    }		
	u1_printf("success\r\n");
    WiFi_ResetIO_Init();            //初始化WiFi的复位IO
    MQTT_Buff_Init();               //初始化接收,发送,命令数据的 缓冲区 以及各状态参数
	AliIoT_Parameter_Init();	    //初始化连接阿里云IoT平台MQTT服务器的参数
    TIM5_ENABLE_1S();
	u1_printf("success\r\n");
	
    while(1)                        //主循环
	{		

		/*--------------------------------------------------------------------*/
		/*   Connect_flag=1同服务器建立了连接,我们可以发布数据和接收推送了    */
		/*--------------------------------------------------------------------*/
		if(Connect_flag==1){			
            mqtt_txbuf_process();//处理发送缓冲区数据
			
            mqtt_rxbuf_process();//处理接受缓冲区数据
			
            mqtt_cmdbuf_process();//处理命令缓冲区数据
        }	
		// /*--------------------------------------------------------------------*/
		// /*      Connect_flag=0同服务器断开了连接,我们要重启连接服务器         */
		// /*--------------------------------------------------------------------*/
		else{
            connect_toRouter();
        }
    
	}	
}
/*-------------------------------------------------*/
/*函数名：采集温湿度，并发布给服务器               */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void TempHumi_State(void)
{
    u8 i = 0;
    double lux = 0;
    double temperature = 0;
    double humidity[8];
	char temp[512];
    char str[10];
    lux = getEnvirmentLux();
    temperature = DS18B20_Get_ActualTemp();
    getAllHimidity(humidity);  
	u1_printf("温度：%.2f  光照%.2f\r\n",temperature, lux);
	u1_printf("湿度0：%.2f  湿度1%.2f\r\n",humidity[0], humidity[1]);
	u1_printf("湿度2：%.2f  湿度3%.2f\r\n",humidity[2], humidity[3]);
	u1_printf("湿度4：%.2f  湿度5%.2f\r\n",humidity[4], humidity[5]);
	u1_printf("湿度6：%.2f  湿度7%.2f\r\n",humidity[6], humidity[7]);
	sprintf(str, "%d%d%d%d%d%d%d%d", PUMP_CONNECT_STATUS[0], PUMP_CONNECT_STATUS[1], PUMP_CONNECT_STATUS[2], PUMP_CONNECT_STATUS[3],
            PUMP_CONNECT_STATUS[4], PUMP_CONNECT_STATUS[5], PUMP_CONNECT_STATUS[6], PUMP_CONNECT_STATUS[7]);
	sprintf(temp,"{\"method\":\"thing.event.property.post\",\n\"id\":\"203302322\",\n	 	\
	\"params\":{\n\"humdity0\":%2f,\"humdity1\":%2f,\"humdity2\":%2f,\"humdity3\":%2f,\n 	\
	\"humdity4\":%2f,\"humdity5\":%2f,\"humdity6\":%2f,\"humdity7\":%2f,			 	\
	\"temperature\":%2f,\"env_lux\":%2f,\n\"pumpConnectStatus\":\"%s\"},												\
	\"version\":\"1.0.0\"}",humidity[0],humidity[1],humidity[2],humidity[3],humidity[4],\
	humidity[5],humidity[6],humidity[7],temperature,lux,str);  //构建回复湿度温度数据
    // setPost_JSON_str();
    MQTT_PublishQs0(P_TOPIC_NAME,temp,strlen(temp));   //添加数据，发布给服务器
}
