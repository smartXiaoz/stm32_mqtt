#include "rtc.h"
#include "process.h"
#include "stm32f10x.h"     //包含需要的头文件
#include "stm32f10x_it.h"  //包含需要的头文件
#include "main.h"          //包含需要的头文件
#include "delay.h"         //包含需要的头文件
#include "usart1.h"        //包含需要的头文件
#include "usart2.h"        //包含需要的头文件
#include "timer1.h"        //包含需要的头文件
#include "timer4.h"        //包含需要的头文件
#include "wifi.h"	       //包含需要的头文件
#include "timer3.h"        //包含需要的头文件
#include "led.h"           //包含需要的头文件
#include "mqtt.h"          //包含需要的头文件

void USART2_IRQHandler(void)   
{                      
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET){  //如果USART_IT_RXNE标志置位，表示有数据到了，进入if分支
		if(Connect_flag==0){                                //如果Connect_flag等于0，当前还没有连接服务器，处于指令配置状态
			if(USART2->DR){                                 //处于指令配置状态时，非零值才保存到缓冲区	
				Usart2_RxBuff[Usart2_RxCounter]=USART2->DR; //保存到缓冲区	
				Usart2_RxCounter ++;                        //每接收1个字节的数据，Usart2_RxCounter加1，表示接收的数据总量+1 
			}		
		}else{		                                        //反之Connect_flag等于1，连接上服务器了	
			Usart2_RxBuff[Usart2_RxCounter] = USART2->DR;   //把接收到的数据保存到Usart2_RxBuff中				
			if(Usart2_RxCounter == 0){    					//如果Usart2_RxCounter等于0，表示是接收的第1个数据，进入if分支				
				TIM_Cmd(TIM4,ENABLE); 
			}else{                        					//else分支，表示果Usart2_RxCounter不等于0，不是接收的第一个数据
				TIM_SetCounter(TIM4,0);  
			}	
			Usart2_RxCounter ++;         				    //每接收1个字节的数据，Usart2_RxCounter加1，表示接收的数据总量+1 
		}
	}
} 
/*-------------------------------------------------*/
/*函数名：定时器4中断服务函数                      */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void TIM4_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET){                //如果TIM_IT_Update置位，表示TIM4溢出中断，进入if

        u1_printf("counter:%d\r\n", Usart2_RxCounter);
        memcpy(&MQTT_RxDataInPtr[2],Usart2_RxBuff,Usart2_RxCounter);  //拷贝数据到接收缓冲区
		MQTT_RxDataInPtr[0] = Usart2_RxCounter/256;                   //记录数据长度高字节
		MQTT_RxDataInPtr[1] = Usart2_RxCounter%256;                   //记录数据长度低字节
		MQTT_RxDataInPtr+=BUFF_UNIT;                                  //指针下移
		if(MQTT_RxDataInPtr==MQTT_RxDataEndPtr)                       //如果指针到缓冲区尾部了
			MQTT_RxDataInPtr = MQTT_RxDataBuf[0];                     //指针归位到缓冲区开头
		Usart2_RxCounter = 0;                                         //串口2接收数据量变量清零
		TIM_SetCounter(TIM3, 0);                                      //清零定时器6计数器，重新计时ping包发送时间
		TIM_Cmd(TIM4, DISABLE);                        				  //关闭TIM4定时器
		TIM_SetCounter(TIM4, 0);                        			  //清零定时器4计数器
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);     			  //清除TIM4溢出中断标志 	
	}
}
/*-------------------------------------------------*/
/*函数名：定时器3中断服务函数                      */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET){   //如果TIM_IT_Update置位，表示TIM3溢出中断，进入if	
		switch(Ping_flag){                               //判断Ping_flag的状态
			case 0:										 //如果Ping_flag等于0，表示正常状态，发送Ping报文  
					MQTT_PingREQ(); 					 //添加Ping报文到发送缓冲区  
					break;
			case 1:										 //如果Ping_flag等于1，说明上一次发送到的ping报文，没有收到服务器回复，所以1没有被清除为0，可能是连接异常，我们要启动快速ping模式
					TIM3_ENABLE_2S(); 					 //我们将定时器6设置为2s定时,快速发送Ping报文
					MQTT_PingREQ();  					 //添加Ping报文到发送缓冲区  
					break;
			case 2:										 //如果Ping_flag等于2，说明还没有收到服务器回复
			case 3:				                         //如果Ping_flag等于3，说明还没有收到服务器回复
			case 4:				                         //如果Ping_flag等于4，说明还没有收到服务器回复	
					MQTT_PingREQ();  					 //添加Ping报文到发送缓冲区 
					break;
			case 5:										 //如果Ping_flag等于5，说明我们发送了多次ping，均无回复，应该是连接有问题，我们重启连接
					Connect_flag = 0;                    //连接状态置0，表示断开，没连上服务器
					SYS_RST();
					TIM_Cmd(TIM3,DISABLE);               //关TIM3 				
					break;			
		}
		Ping_flag++;           		             		 //Ping_flag自增1，表示又发送了一次ping，期待服务器的回复
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);      //清除TIM3溢出中断标志 	
	}
}
/*-------------------------------------------------*/
/*函数名：定时器2中断服务函数                      */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void TIM2_IRQHandler(void)
{	
	if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET){  //如果TIM_IT_Update置位，表示TIM2溢出中断，进入if	
		TempHumi_State();
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);     //清除TIM2溢出中断标志 	
	}
}

/*-------------------------------------------------*/
/*函数名：定时器1中断服务函数                      */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void TIM5_IRQHandler(void)
{
    u8 i = 0;
    if(TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET){  //如果TIM_IT_Update置位，表示TIM2溢出中断，进入if
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
        TIM_ClearITPendingBit(TIM5, TIM_IT_Update); //清除TIM2溢出中断标志 		
	}
}

//RTC时钟中断
//每秒触发一次  
//extern u16 tcnt; 
void RTC_IRQHandler(void)
{
    u8 i = 0;
    double settim = 0;
    double pretim = 0;
    if (RTC_GetITStatus(RTC_IT_SEC) != RESET)//秒钟中断
	{
		RTC_Get();//更新时间 
        for (i = 0; i < 8; i++) {
			
            if (PUMP_SET_AlarmTim[i][17] == 1) {
                u1_printf("rtc---------\r\n");
                settim = PUMP_SET_AlarmTim[i][7] * 100000000 + PUMP_SET_AlarmTim[i][8] * 1000000 + \
				PUMP_SET_AlarmTim[i][9] * 10000 + PUMP_SET_AlarmTim[i][10] * 100 + PUMP_SET_AlarmTim[i][11];
				pretim = calendar.w_month * 100000000 + calendar.w_date * 1000000 + \
				calendar.hour * 10000 + calendar.min * 100 + calendar.sec;
                u1_printf("设定值:%lf\r\n", settim);
                u1_printf("当前值:%lf\r\n", pretim);
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
	if(RTC_GetITStatus(RTC_IT_ALR)!= RESET)//闹钟中断
	{
		RTC_ClearITPendingBit(RTC_IT_ALR);		//清闹钟中断	  	
	  	RTC_Get();				//更新时间   
  		u1_printf("Alarm Time:%d-%d-%d %d:%d:%d\n",calendar.w_year,calendar.w_month,calendar.w_date,calendar.hour,calendar.min,calendar.sec);//输出闹铃时间	
		
  	} 				  								 
	RTC_ClearITPendingBit(RTC_IT_SEC|RTC_IT_OW);		//清闹钟中断
	RTC_WaitForLastTask();	  	    						 	   	 
}

//外部中断服务程序 
void EXTI9_5_IRQHandler(void)
{
    char temp[200];
    char str[20];
    if (EXTI_GetITStatus(EXTI_Line6)) {
		delay_ms(10);//消抖
        if (INPUMP0 == 0) {
            PUMP_CONNECT_STATUS[0] = 1;
            u1_printf("6k\r\n");
		}else if(INPUMP0 == 1)
		{
			PUMP_CONNECT_STATUS[0] = 0;
            u1_printf("6c\r\n");
		}	  
		EXTI_ClearITPendingBit(EXTI_Line6); //清除LINE6上的中断标志位  
    }
    
	if (EXTI_GetITStatus(EXTI_Line7)) {
		delay_ms(10);//消抖
        if (INPUMP1 == 0) {
			PUMP_CONNECT_STATUS[1] = 1;
			u1_printf("7\r\n");
		}else if(INPUMP1 == 1){
			PUMP_CONNECT_STATUS[1] = 0;
            u1_printf("7c\r\n");
		}  
		EXTI_ClearITPendingBit(EXTI_Line7); //清除LINE6上的中断标志位  
    }

	if (EXTI_GetITStatus(EXTI_Line8)) {
		delay_ms(10);//消抖
        if (INPUMP2 == 0) {
			PUMP_CONNECT_STATUS[2] = 1;
			u1_printf("8\r\n");
		}else if(INPUMP2 == 1){
			PUMP_CONNECT_STATUS[2] = 0;
            u1_printf("8c\r\n");
		}   
		EXTI_ClearITPendingBit(EXTI_Line8); //清除LINE6上的中断标志位  
    }

	if (EXTI_GetITStatus(EXTI_Line9)) {
		delay_ms(10);//消抖
        if (INPUMP3 == 0) {
			PUMP_CONNECT_STATUS[3] = 1;
			u1_printf("9k\r\n");
		}else if(INPUMP3 == 1){
			PUMP_CONNECT_STATUS[3] = 0;
            u1_printf("9c\r\n");
		}   
		EXTI_ClearITPendingBit(EXTI_Line9); //清除LINE6上的中断标志位  
    }
    sprintf(str, "%d%d%d%d%d%d%d%d", PUMP_CONNECT_STATUS[0], PUMP_CONNECT_STATUS[1], PUMP_CONNECT_STATUS[2], PUMP_CONNECT_STATUS[3],
            PUMP_CONNECT_STATUS[4], PUMP_CONNECT_STATUS[5], PUMP_CONNECT_STATUS[6], PUMP_CONNECT_STATUS[7]);
    sprintf(temp,"{\"method\":\"thing.event.property.post\",\n\"id\":\"203302322\",\n	 	\
	\"params\":{\n\"pumpConnectStatus\":\"%s\"},												\
	\"version\":\"1.0.0\"}",str);  //构建回复湿度温度数据
    u1_printf(temp);
	
    MQTT_PublishQs0(P_TOPIC_NAME,temp,strlen(temp));   //添加数据，发布给服务器
}

//外部中断服务程序 
void EXTI15_10_IRQHandler(void)
{
	char temp[200];
    char str[20];
    if (EXTI_GetITStatus(EXTI_Line10)) {
		delay_ms(10);//消抖
        if (INPUMP4 == 0) {
			PUMP_CONNECT_STATUS[4] = 1;
			u1_printf("10k\r\n");
		}else if(INPUMP4 == 1){
			PUMP_CONNECT_STATUS[4] = 0;
            u1_printf("10c\r\n");
		}  
		EXTI_ClearITPendingBit(EXTI_Line10); //清除LINE6上的中断标志位  
    }
    
	if (EXTI_GetITStatus(EXTI_Line11)) {
		delay_ms(10);//消抖
        if (INPUMP5 == 0) {
			PUMP_CONNECT_STATUS[5] = 1;
			u1_printf("11k\r\n");
		}else if(INPUMP4 == 1){
			PUMP_CONNECT_STATUS[5] = 0;
            u1_printf("11c\r\n");
		}  
		EXTI_ClearITPendingBit(EXTI_Line11); //清除LINE6上的中断标志位  
    }

	if (EXTI_GetITStatus(EXTI_Line12)) {
		delay_ms(10);//消抖
        if (INPUMP6 == 0) {
			PUMP_CONNECT_STATUS[6] = 1;
			u1_printf("12k\r\n");
		}else if(INPUMP6 == 1){
			PUMP_CONNECT_STATUS[6] = 0;
            u1_printf("12c\r\n");
		}   
		EXTI_ClearITPendingBit(EXTI_Line12); //清除LINE6上的中断标志位  
    }

	if (EXTI_GetITStatus(EXTI_Line13)) {
		delay_ms(10);//消抖
        if (INPUMP7 == 0) {
			PUMP_CONNECT_STATUS[7] = 1;
			u1_printf("12k\r\n");
		}else if(INPUMP7 == 1){
			PUMP_CONNECT_STATUS[7] = 0;
            u1_printf("12c\r\n");
		}   
		EXTI_ClearITPendingBit(EXTI_Line13); //清除LINE6上的中断标志位  
    }	
	sprintf(str, "%d%d%d%d%d%d%d%d", PUMP_CONNECT_STATUS[0], PUMP_CONNECT_STATUS[1], PUMP_CONNECT_STATUS[2], PUMP_CONNECT_STATUS[3],
            PUMP_CONNECT_STATUS[4], PUMP_CONNECT_STATUS[5], PUMP_CONNECT_STATUS[6], PUMP_CONNECT_STATUS[7]);
    sprintf(temp,"{\"method\":\"thing.event.property.post\",\n\"id\":\"203302322\",\n	 	\
	\"params\":{\n\"pumpConnectStatus\":\"%s\"},												\
	\"version\":\"1.0.0\"}",str);  //构建回复湿度温度数据
    u1_printf(temp);
	
    MQTT_PublishQs0(P_TOPIC_NAME,temp,strlen(temp));   //添加数据，发布给服务器
}

/*-------------------------------------------------*/
/*函数名：不可屏蔽中断处理函数                     */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void NMI_Handler(void)
{

}

/*-------------------------------------------------*/
/*函数名：硬件出错后进入的中断处理函数             */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void HardFault_Handler(void)
{

}

/*-------------------------------------------------*/
/*函数名：内存管理中断处理函数                     */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void MemManage_Handler(void)
{

}

/*-------------------------------------------------*/
/*函数名：预取指失败，存储器访问失败中断处理函数   */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void BusFault_Handler(void)
{

}

/*-------------------------------------------------*/
/*函数名：未定义的指令或非法状态处理函数           */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void UsageFault_Handler(void)
{

}

/*-------------------------------------------------*/
/*函数名：软中断，SWI 指令调用的处理函数           */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void SVC_Handler(void)
{
	
}

/*-------------------------------------------------*/
/*函数名：调试监控器处理函数                       */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void DebugMon_Handler(void)
{
	
}

/*-------------------------------------------------*/
/*函数名：可挂起的系统服务处理函数                 */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void PendSV_Handler(void)
{
}

/*-------------------------------------------------*/
/*函数名：SysTic系统嘀嗒定时器处理函数             */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void SysTick_Handler(void)
{
 
}
