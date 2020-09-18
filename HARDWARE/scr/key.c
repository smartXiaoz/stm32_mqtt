/*-------------------------------------------------*/
/*            超纬电子STM32系列开发板              */
/*-------------------------------------------------*/
/*                                                 */
/*            实现扫描按键功能的源文件             */
/*                                                 */
/*-------------------------------------------------*/

#include "stm32f10x.h"  //包含需要的头文件
#include "delay.h"      //包含需要的头文件
#include "key.h"      //包含需要的头文件

/*-------------------------------------------------*/
/*函数名：初始化KEY按键函数                        */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void KEY_Init(void)
{	
	GPIO_InitTypeDef GPIO_InitStructure;                       //定义一个设置IO的结构体
	
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA , ENABLE);    //使能PA端口时钟
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0;                 //准备设置PA0
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;   		   //下拉输入，默认状态是低电平
	GPIO_Init(GPIOA, &GPIO_InitStructure);            		   //设置PA0
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_1;                 //准备设置PA1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;   		   //上拉输入，默认状态是高电平
	GPIO_Init(GPIOA, &GPIO_InitStructure);            		   //设置PA1
} 

/*-------------------------------------------------*/
/*函数名：按键扫描函数                             */
/*参  数：无                                       */
/*返回值：有无按键按下，有的话是哪个按键按下       */
/*-------------------------------------------------*/
char KEY_Scan(void)
{	 	  
	if(KEY1_IN_STA==1){                  //判断控制KEY1的IO输入电平，是否是高电平，如果是进入if	
		delay_ms(5);                     //延时5ms，消除抖动的误判
		if(KEY1_IN_STA==1){              //消除抖动的影响后，再次判断控制KEY1的IO输入电平，是否是高电平，如果是进入if，说明按键按下了	
			while(1){                    //等待按键抬起
				if(KEY1_IN_STA==0){      //判断控制KEY1的IO输入电平，是否是低电平，如果是进入if	
					delay_ms(5);         //延时5ms，消除抖动的误判
					if(KEY1_IN_STA==0){  //消除抖动的影响后，再次判断控制KEY1的IO输入电平，是否是低电平，如果是进入if，说明按键抬起了
						return 1;        //返回1，表示KEY1按下了
					}
				}
			}
		}
	}else if(KEY2_IN_STA==0){            //判断控制KEY2的IO输入电平，是否是低电平，如果是进入if	
		delay_ms(5);                     //延时5ms，消除抖动的误判
		if(KEY2_IN_STA==0){              //消除抖动的影响后，再次判断控制KEY2的IO输入电平，是否是低电平，如果是进入if，说明按键按下了	
			while(1){                    //等待按键抬起
				if(KEY2_IN_STA==1){      //判断控制KEY2的IO输入电平，是否是高电平，如果是进入if
					delay_ms(5);         //延时5ms，消除抖动的误判
					if(KEY2_IN_STA==1){  //消除抖动的影响后，再次判断控制KEY2的IO输入电平，是否是高电平，如果是进入if，说明按键抬起了
						return 2;        //返回2，表示KEY2按下了
					}
				}
			}
		}
	}	
	return 0;                            //如果没有按键按下，返回0
}

/*-------------------------------------------------*/
/*函数名：按键外部中断初始化程序.                  */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/	   
void KEY_Exti_Init(void)
{
	NVIC_InitTypeDef   NVIC_InitStructure; //定义一个设置IO的结构体
	EXTI_InitTypeDef   EXTI_InitStructure; //定义一个设置中断的结构体 

  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	          //使能GPIO复用功能时钟
  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource0);    //配置KEY1-PA0中断线
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource1);    //配置KEY2-PA1中断线

  	EXTI_InitStructure.EXTI_Line = EXTI_Line0;	                  //准备设置外部0中断线
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	          //外部中断
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;        //上升沿触发中断
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;                     //使能
  	EXTI_Init(&EXTI_InitStructure);	 	                          //设置外部中断0线
	
  	EXTI_InitStructure.EXTI_Line = EXTI_Line1;	                  //准备设置KEY2-PA1外部1中断线
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	          //外部中断
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;       //下降沿触发中断
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;                     //使能
  	EXTI_Init(&EXTI_InitStructure);	 	                          //设置外部中断1线

	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;			  //准备设置外部中断0
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;     //抢占优先级3 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		      //子优先级0
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			      //使能外部中断0通道
  	NVIC_Init(&NVIC_InitStructure);                               //设置外部中断0

    NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;			  //准备设置外部中断1
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;     //抢占优先级3
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		      //子优先级1
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				  //使能外部中断1通道
  	NVIC_Init(&NVIC_InitStructure);                               //设置外部中断1
}




















