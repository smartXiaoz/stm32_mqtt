/*-------------------------------------------------*/
/*            ��γ����STM32ϵ�п�����              */
/*-------------------------------------------------*/
/*                                                 */
/*              ʵ��AHT10���ܵ�ͷ�ļ�              */
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
















