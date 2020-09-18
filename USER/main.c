#include "stm32f10x.h"  //������Ҫ��ͷ�ļ�
#include "main.h"       //������Ҫ��ͷ�ļ�
#include "delay.h"      //������Ҫ��ͷ�ļ�
#include "usart1.h"     //������Ҫ��ͷ�ļ�
#include "usart2.h"     //������Ҫ��ͷ�ļ�
#include "timer1.h"     //������Ҫ��ͷ�ļ�
#include "timer2.h"     //������Ҫ��ͷ�ļ�
#include "timer3.h"     //������Ҫ��ͷ�ļ�
#include "timer4.h"     //������Ҫ��ͷ�ļ�
#include "wifi.h"	    //������Ҫ��ͷ�ļ�
#include "led.h"        //������Ҫ��ͷ�ļ�
#include "mqtt.h"       //������Ҫ��ͷ�ļ�
#include "key.h"        //������Ҫ��ͷ�ļ�
#include "dht11.h"
#include "iic.h"        //������Ҫ��ͷ�ļ�
#include "adc.h"
#include "ds18b20.h"
#include "process.h"
#include "cJSON.h"
#include "stmflash.h"
#include "rtc.h"

//���������λ
void SYS_RST(void) {
	__set_FAULTMASK(1);
	NVIC_SystemReset();
}

int main(void) 
{

    Delay_Init();                   //��ʱ���ܳ�ʼ��              
	Usart1_Init(115200);              //����1���ܳ�ʼ����������9600
	Usart2_Init(115200);            //����2���ܳ�ʼ����������115200	
	TIM4_Init(300,7200);            //TIM4��ʼ������ʱʱ�� 300*7200*1000/72000000 = 30ms
    
    LED_Init();	                    //LED��ʼ��
    EXTIX_Init();
    Adc_Init();
	RTC_Init();	  			//RTC��ʼ��
    while(DS18B20_Init())	//DS18B20��ʼ��	
	{
 		delay_ms(200);
        u1_printf("�¶ȴ�������ʼ��\r\n");
    }		
	u1_printf("success\r\n");
    WiFi_ResetIO_Init();            //��ʼ��WiFi�ĸ�λIO
    MQTT_Buff_Init();               //��ʼ������,����,�������ݵ� ������ �Լ���״̬����
	AliIoT_Parameter_Init();	    //��ʼ�����Ӱ�����IoTƽ̨MQTT�������Ĳ���
    TIM5_ENABLE_1S();
	u1_printf("success\r\n");
	
    while(1)                        //��ѭ��
	{		

		/*--------------------------------------------------------------------*/
		/*   Connect_flag=1ͬ����������������,���ǿ��Է������ݺͽ���������    */
		/*--------------------------------------------------------------------*/
		if(Connect_flag==1){			
            mqtt_txbuf_process();//�����ͻ���������
			
            mqtt_rxbuf_process();//������ܻ���������
			
            mqtt_cmdbuf_process();//���������������
        }	
		// /*--------------------------------------------------------------------*/
		// /*      Connect_flag=0ͬ�������Ͽ�������,����Ҫ�������ӷ�����         */
		// /*--------------------------------------------------------------------*/
		else{
            connect_toRouter();
        }
    
	}	
}
/*-------------------------------------------------*/
/*���������ɼ���ʪ�ȣ���������������               */
/*��  ������                                       */
/*����ֵ����                                       */
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
	u1_printf("�¶ȣ�%.2f  ����%.2f\r\n",temperature, lux);
	u1_printf("ʪ��0��%.2f  ʪ��1%.2f\r\n",humidity[0], humidity[1]);
	u1_printf("ʪ��2��%.2f  ʪ��3%.2f\r\n",humidity[2], humidity[3]);
	u1_printf("ʪ��4��%.2f  ʪ��5%.2f\r\n",humidity[4], humidity[5]);
	u1_printf("ʪ��6��%.2f  ʪ��7%.2f\r\n",humidity[6], humidity[7]);
	sprintf(str, "%d%d%d%d%d%d%d%d", PUMP_CONNECT_STATUS[0], PUMP_CONNECT_STATUS[1], PUMP_CONNECT_STATUS[2], PUMP_CONNECT_STATUS[3],
            PUMP_CONNECT_STATUS[4], PUMP_CONNECT_STATUS[5], PUMP_CONNECT_STATUS[6], PUMP_CONNECT_STATUS[7]);
	sprintf(temp,"{\"method\":\"thing.event.property.post\",\n\"id\":\"203302322\",\n	 	\
	\"params\":{\n\"humdity0\":%2f,\"humdity1\":%2f,\"humdity2\":%2f,\"humdity3\":%2f,\n 	\
	\"humdity4\":%2f,\"humdity5\":%2f,\"humdity6\":%2f,\"humdity7\":%2f,			 	\
	\"temperature\":%2f,\"env_lux\":%2f,\n\"pumpConnectStatus\":\"%s\"},												\
	\"version\":\"1.0.0\"}",humidity[0],humidity[1],humidity[2],humidity[3],humidity[4],\
	humidity[5],humidity[6],humidity[7],temperature,lux,str);  //�����ظ�ʪ���¶�����
    // setPost_JSON_str();
    MQTT_PublishQs0(P_TOPIC_NAME,temp,strlen(temp));   //������ݣ�������������
}
