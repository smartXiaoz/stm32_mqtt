/*-------------------------------------------------*/
/*            ��γ����STM32ϵ�п�����              */
/*-------------------------------------------------*/
/*                                                 */
/*            ʵ��ɨ�谴�����ܵ�ͷ�ļ�             */
/*                                                 */
/*-------------------------------------------------*/
#ifndef __KEY_H
#define __KEY_H	

#define KEY1_IN_STA  GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) //PA0����KEY1,��ȡ��ƽ״̬,�����ж�KEY1�Ƿ���
#define KEY2_IN_STA  GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1) //PA1����KEY2,��ȡ��ƽ״̬,�����ж�KEY2�Ƿ���

void KEY_Init(void);	   //������ʼ��
char KEY_Scan(void);       //����ɨ�躯��	
void KEY_Exti_Init(void);  //�����ⲿ�жϳ�ʼ������

#endif
