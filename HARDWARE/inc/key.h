/*-------------------------------------------------*/
/*            超纬电子STM32系列开发板              */
/*-------------------------------------------------*/
/*                                                 */
/*            实现扫描按键功能的头文件             */
/*                                                 */
/*-------------------------------------------------*/
#ifndef __KEY_H
#define __KEY_H	

#define KEY1_IN_STA  GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) //PA0控制KEY1,读取电平状态,可以判断KEY1是否按下
#define KEY2_IN_STA  GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1) //PA1控制KEY2,读取电平状态,可以判断KEY2是否按下

void KEY_Init(void);	   //按键初始化
char KEY_Scan(void);       //按键扫描函数	
void KEY_Exti_Init(void);  //按键外部中断初始化程序

#endif
