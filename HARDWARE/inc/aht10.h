/*-------------------------------------------------*/
/*            超纬电子STM32系列开发板              */
/*-------------------------------------------------*/
/*                                                 */
/*              实现AHT10功能的头文件              */
/*                                                 */
/*-------------------------------------------------*/

#ifndef __AHT10_H
#define __AHT10_H

void AHT10_Reset(void);
void AHT10_ReadData(void);
char AHT10_ReadStatus(void);
char AHT10_Read_CalEnable(void); 
char AHT10_Init(void);
void AHT10_Data(double *temp, double *humi);	

#endif
















