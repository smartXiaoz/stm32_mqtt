#ifndef __LED_H
#define __LED_H	 
#include "sys.h"

#define PUMP(x) PBout(pump_convert[x])
extern char pump_convert[8];
#define INPUMP0  GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_6)
#define INPUMP1  GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_7)
#define INPUMP2  GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_8)
#define INPUMP3  GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_9)
#define INPUMP4  GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_10)
#define INPUMP5  GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_11)
#define INPUMP6  GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_12)
#define INPUMP7  GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13)


void LED_Init(void);//≥ı ºªØ
void EXTIX_Init(void);

#endif
