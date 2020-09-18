#include "rtc.h"
#include "process.h"
#include "stm32f10x.h"     //������Ҫ��ͷ�ļ�
#include "stm32f10x_it.h"  //������Ҫ��ͷ�ļ�
#include "main.h"          //������Ҫ��ͷ�ļ�
#include "delay.h"         //������Ҫ��ͷ�ļ�
#include "usart1.h"        //������Ҫ��ͷ�ļ�
#include "usart2.h"        //������Ҫ��ͷ�ļ�
#include "timer1.h"        //������Ҫ��ͷ�ļ�
#include "timer4.h"        //������Ҫ��ͷ�ļ�
#include "wifi.h"	       //������Ҫ��ͷ�ļ�
#include "timer3.h"        //������Ҫ��ͷ�ļ�
#include "led.h"           //������Ҫ��ͷ�ļ�
#include "mqtt.h"          //������Ҫ��ͷ�ļ�

void USART2_IRQHandler(void)   
{                      
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET){  //���USART_IT_RXNE��־��λ����ʾ�����ݵ��ˣ�����if��֧
		if(Connect_flag==0){                                //���Connect_flag����0����ǰ��û�����ӷ�����������ָ������״̬
			if(USART2->DR){                                 //����ָ������״̬ʱ������ֵ�ű��浽������	
				Usart2_RxBuff[Usart2_RxCounter]=USART2->DR; //���浽������	
				Usart2_RxCounter ++;                        //ÿ����1���ֽڵ����ݣ�Usart2_RxCounter��1����ʾ���յ���������+1 
			}		
		}else{		                                        //��֮Connect_flag����1�������Ϸ�������	
			Usart2_RxBuff[Usart2_RxCounter] = USART2->DR;   //�ѽ��յ������ݱ��浽Usart2_RxBuff��				
			if(Usart2_RxCounter == 0){    					//���Usart2_RxCounter����0����ʾ�ǽ��յĵ�1�����ݣ�����if��֧				
				TIM_Cmd(TIM4,ENABLE); 
			}else{                        					//else��֧����ʾ��Usart2_RxCounter������0�����ǽ��յĵ�һ������
				TIM_SetCounter(TIM4,0);  
			}	
			Usart2_RxCounter ++;         				    //ÿ����1���ֽڵ����ݣ�Usart2_RxCounter��1����ʾ���յ���������+1 
		}
	}
} 
/*-------------------------------------------------*/
/*����������ʱ��4�жϷ�����                      */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void TIM4_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET){                //���TIM_IT_Update��λ����ʾTIM4����жϣ�����if

        u1_printf("counter:%d\r\n", Usart2_RxCounter);
        memcpy(&MQTT_RxDataInPtr[2],Usart2_RxBuff,Usart2_RxCounter);  //�������ݵ����ջ�����
		MQTT_RxDataInPtr[0] = Usart2_RxCounter/256;                   //��¼���ݳ��ȸ��ֽ�
		MQTT_RxDataInPtr[1] = Usart2_RxCounter%256;                   //��¼���ݳ��ȵ��ֽ�
		MQTT_RxDataInPtr+=BUFF_UNIT;                                  //ָ������
		if(MQTT_RxDataInPtr==MQTT_RxDataEndPtr)                       //���ָ�뵽������β����
			MQTT_RxDataInPtr = MQTT_RxDataBuf[0];                     //ָ���λ����������ͷ
		Usart2_RxCounter = 0;                                         //����2������������������
		TIM_SetCounter(TIM3, 0);                                      //���㶨ʱ��6�����������¼�ʱping������ʱ��
		TIM_Cmd(TIM4, DISABLE);                        				  //�ر�TIM4��ʱ��
		TIM_SetCounter(TIM4, 0);                        			  //���㶨ʱ��4������
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);     			  //���TIM4����жϱ�־ 	
	}
}
/*-------------------------------------------------*/
/*����������ʱ��3�жϷ�����                      */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET){   //���TIM_IT_Update��λ����ʾTIM3����жϣ�����if	
		switch(Ping_flag){                               //�ж�Ping_flag��״̬
			case 0:										 //���Ping_flag����0����ʾ����״̬������Ping����  
					MQTT_PingREQ(); 					 //���Ping���ĵ����ͻ�����  
					break;
			case 1:										 //���Ping_flag����1��˵����һ�η��͵���ping���ģ�û���յ��������ظ�������1û�б����Ϊ0�������������쳣������Ҫ��������pingģʽ
					TIM3_ENABLE_2S(); 					 //���ǽ���ʱ��6����Ϊ2s��ʱ,���ٷ���Ping����
					MQTT_PingREQ();  					 //���Ping���ĵ����ͻ�����  
					break;
			case 2:										 //���Ping_flag����2��˵����û���յ��������ظ�
			case 3:				                         //���Ping_flag����3��˵����û���յ��������ظ�
			case 4:				                         //���Ping_flag����4��˵����û���յ��������ظ�	
					MQTT_PingREQ();  					 //���Ping���ĵ����ͻ����� 
					break;
			case 5:										 //���Ping_flag����5��˵�����Ƿ����˶��ping�����޻ظ���Ӧ�������������⣬������������
					Connect_flag = 0;                    //����״̬��0����ʾ�Ͽ���û���Ϸ�����
					SYS_RST();
					TIM_Cmd(TIM3,DISABLE);               //��TIM3 				
					break;			
		}
		Ping_flag++;           		             		 //Ping_flag����1����ʾ�ַ�����һ��ping���ڴ��������Ļظ�
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);      //���TIM3����жϱ�־ 	
	}
}
/*-------------------------------------------------*/
/*����������ʱ��2�жϷ�����                      */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void TIM2_IRQHandler(void)
{	
	if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET){  //���TIM_IT_Update��λ����ʾTIM2����жϣ�����if	
		TempHumi_State();
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);     //���TIM2����жϱ�־ 	
	}
}

/*-------------------------------------------------*/
/*����������ʱ��1�жϷ�����                      */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void TIM5_IRQHandler(void)
{
    u8 i = 0;
    if(TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET){  //���TIM_IT_Update��λ����ʾTIM2����жϣ�����if
        for (i = 0; i < 8; i++) {
            if (PUMP_SET_TIM[0] > 0) {
                PAout(8) = 0;
            }else
			{
                PAout(8) = 1;
            }
			
            if (PUMP_SET_TIM[i] > 0) {
				u1_printf("i--%d--------data:%d\r\n",i,PUMP_SET_TIM[i]);
                PUMP_SET_TIM[i]--;
                PUMP(i) = 0;
            }else
			{			
                PUMP(i) = 1;
            }
        }
        TIM_ClearITPendingBit(TIM5, TIM_IT_Update); //���TIM2����жϱ�־ 		
	}
}

//RTCʱ���ж�
//ÿ�봥��һ��  
//extern u16 tcnt; 
void RTC_IRQHandler(void)
{
    u8 i = 0;
    double settim = 0;
    double pretim = 0;
    if (RTC_GetITStatus(RTC_IT_SEC) != RESET)//�����ж�
	{
		RTC_Get();//����ʱ�� 
        for (i = 0; i < 8; i++) {
			
            if (PUMP_SET_AlarmTim[i][17] == 1) {
                u1_printf("rtc---------\r\n");
                settim = PUMP_SET_AlarmTim[i][7] * 100000000 + PUMP_SET_AlarmTim[i][8] * 1000000 + \
				PUMP_SET_AlarmTim[i][9] * 10000 + PUMP_SET_AlarmTim[i][10] * 100 + PUMP_SET_AlarmTim[i][11];
				pretim = calendar.w_month * 100000000 + calendar.w_date * 1000000 + \
				calendar.hour * 10000 + calendar.min * 100 + calendar.sec;
                u1_printf("�趨ֵ:%lf\r\n", settim);
                u1_printf("��ǰֵ:%lf\r\n", pretim);
                if (settim <= pretim) {
                    PUMP_SET_AlarmTim[i][17] = 2;
                    PUMP_SET_TIM[i] = PUMP_SET_AlarmTim[i][15] * 60 + PUMP_SET_AlarmTim[i][16];
					u1_printf("PUMP_SET_TIM:index:%d---%d", i,PUMP_SET_TIM[i]);
                }
            }
            if (PUMP_SET_AlarmTim[i][17] == 2) {
				u1_printf("PUMP_SET_AlarmTim_interval:index:%d---%d\r\n",i, PUMP_SET_AlarmTim_interval[i][0]);
                if (PUMP_SET_AlarmTim_interval[i][0] > 0) {
					PUMP_SET_AlarmTim_interval[i][0]--;
                } else {
                    PUMP_SET_AlarmTim_interval[i][0] = PUMP_SET_AlarmTim_interval[i][1];
					PUMP_SET_TIM[i] = PUMP_SET_AlarmTim[i][15] * 60 + PUMP_SET_AlarmTim[i][16];
					u1_printf("PUMP_SET_TIM:%d", PUMP_SET_TIM[i]);
                }
            }
        }
    }
	if(RTC_GetITStatus(RTC_IT_ALR)!= RESET)//�����ж�
	{
		RTC_ClearITPendingBit(RTC_IT_ALR);		//�������ж�	  	
	  	RTC_Get();				//����ʱ��   
  		u1_printf("Alarm Time:%d-%d-%d %d:%d:%d\n",calendar.w_year,calendar.w_month,calendar.w_date,calendar.hour,calendar.min,calendar.sec);//�������ʱ��	
		
  	} 				  								 
	RTC_ClearITPendingBit(RTC_IT_SEC|RTC_IT_OW);		//�������ж�
	RTC_WaitForLastTask();	  	    						 	   	 
}

//�ⲿ�жϷ������ 
void EXTI9_5_IRQHandler(void)
{
    char temp[200];
    char str[20];
    if (EXTI_GetITStatus(EXTI_Line6)) {
		delay_ms(10);//����
        if (INPUMP0 == 0) {
            PUMP_CONNECT_STATUS[0] = 1;
            u1_printf("6k\r\n");
		}else if(INPUMP0 == 1)
		{
			PUMP_CONNECT_STATUS[0] = 0;
            u1_printf("6c\r\n");
		}	  
		EXTI_ClearITPendingBit(EXTI_Line6); //���LINE6�ϵ��жϱ�־λ  
    }
    
	if (EXTI_GetITStatus(EXTI_Line7)) {
		delay_ms(10);//����
        if (INPUMP1 == 0) {
			PUMP_CONNECT_STATUS[1] = 1;
			u1_printf("7\r\n");
		}else if(INPUMP1 == 1){
			PUMP_CONNECT_STATUS[1] = 0;
            u1_printf("7c\r\n");
		}  
		EXTI_ClearITPendingBit(EXTI_Line7); //���LINE6�ϵ��жϱ�־λ  
    }

	if (EXTI_GetITStatus(EXTI_Line8)) {
		delay_ms(10);//����
        if (INPUMP2 == 0) {
			PUMP_CONNECT_STATUS[2] = 1;
			u1_printf("8\r\n");
		}else if(INPUMP2 == 1){
			PUMP_CONNECT_STATUS[2] = 0;
            u1_printf("8c\r\n");
		}   
		EXTI_ClearITPendingBit(EXTI_Line8); //���LINE6�ϵ��жϱ�־λ  
    }

	if (EXTI_GetITStatus(EXTI_Line9)) {
		delay_ms(10);//����
        if (INPUMP3 == 0) {
			PUMP_CONNECT_STATUS[3] = 1;
			u1_printf("9k\r\n");
		}else if(INPUMP3 == 1){
			PUMP_CONNECT_STATUS[3] = 0;
            u1_printf("9c\r\n");
		}   
		EXTI_ClearITPendingBit(EXTI_Line9); //���LINE6�ϵ��жϱ�־λ  
    }
    sprintf(str, "%d%d%d%d%d%d%d%d", PUMP_CONNECT_STATUS[0], PUMP_CONNECT_STATUS[1], PUMP_CONNECT_STATUS[2], PUMP_CONNECT_STATUS[3],
            PUMP_CONNECT_STATUS[4], PUMP_CONNECT_STATUS[5], PUMP_CONNECT_STATUS[6], PUMP_CONNECT_STATUS[7]);
    sprintf(temp,"{\"method\":\"thing.event.property.post\",\n\"id\":\"203302322\",\n	 	\
	\"params\":{\n\"pumpConnectStatus\":\"%s\"},												\
	\"version\":\"1.0.0\"}",str);  //�����ظ�ʪ���¶�����
    u1_printf(temp);
	
    MQTT_PublishQs0(P_TOPIC_NAME,temp,strlen(temp));   //������ݣ�������������
}

//�ⲿ�жϷ������ 
void EXTI15_10_IRQHandler(void)
{
	char temp[200];
    char str[20];
    if (EXTI_GetITStatus(EXTI_Line10)) {
		delay_ms(10);//����
        if (INPUMP4 == 0) {
			PUMP_CONNECT_STATUS[4] = 1;
			u1_printf("10k\r\n");
		}else if(INPUMP4 == 1){
			PUMP_CONNECT_STATUS[4] = 0;
            u1_printf("10c\r\n");
		}  
		EXTI_ClearITPendingBit(EXTI_Line10); //���LINE6�ϵ��жϱ�־λ  
    }
    
	if (EXTI_GetITStatus(EXTI_Line11)) {
		delay_ms(10);//����
        if (INPUMP5 == 0) {
			PUMP_CONNECT_STATUS[5] = 1;
			u1_printf("11k\r\n");
		}else if(INPUMP4 == 1){
			PUMP_CONNECT_STATUS[5] = 0;
            u1_printf("11c\r\n");
		}  
		EXTI_ClearITPendingBit(EXTI_Line11); //���LINE6�ϵ��жϱ�־λ  
    }

	if (EXTI_GetITStatus(EXTI_Line12)) {
		delay_ms(10);//����
        if (INPUMP6 == 0) {
			PUMP_CONNECT_STATUS[6] = 1;
			u1_printf("12k\r\n");
		}else if(INPUMP6 == 1){
			PUMP_CONNECT_STATUS[6] = 0;
            u1_printf("12c\r\n");
		}   
		EXTI_ClearITPendingBit(EXTI_Line12); //���LINE6�ϵ��жϱ�־λ  
    }

	if (EXTI_GetITStatus(EXTI_Line13)) {
		delay_ms(10);//����
        if (INPUMP7 == 0) {
			PUMP_CONNECT_STATUS[7] = 1;
			u1_printf("12k\r\n");
		}else if(INPUMP7 == 1){
			PUMP_CONNECT_STATUS[7] = 0;
            u1_printf("12c\r\n");
		}   
		EXTI_ClearITPendingBit(EXTI_Line13); //���LINE6�ϵ��жϱ�־λ  
    }	
	sprintf(str, "%d%d%d%d%d%d%d%d", PUMP_CONNECT_STATUS[0], PUMP_CONNECT_STATUS[1], PUMP_CONNECT_STATUS[2], PUMP_CONNECT_STATUS[3],
            PUMP_CONNECT_STATUS[4], PUMP_CONNECT_STATUS[5], PUMP_CONNECT_STATUS[6], PUMP_CONNECT_STATUS[7]);
    sprintf(temp,"{\"method\":\"thing.event.property.post\",\n\"id\":\"203302322\",\n	 	\
	\"params\":{\n\"pumpConnectStatus\":\"%s\"},												\
	\"version\":\"1.0.0\"}",str);  //�����ظ�ʪ���¶�����
    u1_printf(temp);
	
    MQTT_PublishQs0(P_TOPIC_NAME,temp,strlen(temp));   //������ݣ�������������
}

/*-------------------------------------------------*/
/*�����������������жϴ�����                     */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void NMI_Handler(void)
{

}

/*-------------------------------------------------*/
/*��������Ӳ������������жϴ�����             */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void HardFault_Handler(void)
{

}

/*-------------------------------------------------*/
/*���������ڴ�����жϴ�����                     */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void MemManage_Handler(void)
{

}

/*-------------------------------------------------*/
/*��������Ԥȡָʧ�ܣ��洢������ʧ���жϴ�����   */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void BusFault_Handler(void)
{

}

/*-------------------------------------------------*/
/*��������δ�����ָ���Ƿ�״̬������           */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void UsageFault_Handler(void)
{

}

/*-------------------------------------------------*/
/*�����������жϣ�SWI ָ����õĴ�����           */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void SVC_Handler(void)
{
	
}

/*-------------------------------------------------*/
/*�����������Լ����������                       */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void DebugMon_Handler(void)
{
	
}

/*-------------------------------------------------*/
/*���������ɹ����ϵͳ��������                 */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void PendSV_Handler(void)
{
}

/*-------------------------------------------------*/
/*��������SysTicϵͳ��શ�ʱ��������             */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void SysTick_Handler(void)
{
 
}
