

#include "stm32f10x.h"  //������Ҫ��ͷ�ļ�
#include "timer1.h"
/*-------------------------------------------------*/
/*����������ʱ��1ʹ��1���Ӷ�ʱ                     */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void TIM1_ENABLE_60S(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;            //����һ�����ö�ʱ���ı���
	NVIC_InitTypeDef NVIC_InitStructure;                          //����һ�������жϵı���
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);               //�����ж��������飺��2�� �������ȼ���0 1 2 3 �����ȼ���0 1 2 3		
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);           //ʹ��TIM1ʱ��	
	
    TIM_DeInit(TIM1);                                             //��ʱ��1�Ĵ����ָ�Ĭ��ֵ
	TIM_TimeBaseInitStructure.TIM_Period = 60000-1; 	          //�����Զ���װ��ֵ
	TIM_TimeBaseInitStructure.TIM_Prescaler=36000-1;              //���ö�ʱ��Ԥ��Ƶ��
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter=2-1;          //���ö�ʱ���ظ�����2�Σ�TIM1��8 ����������ܣ�
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //���ϼ���ģʽ
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1;     //1��Ƶ
	TIM_TimeBaseInit(TIM1,&TIM_TimeBaseInitStructure);            //����TIM1
	
	TIM_ClearITPendingBit(TIM1,TIM_IT_Update);                    //�������жϱ�־λ
	TIM_ITConfig(TIM1,TIM_IT_Update,ENABLE);                      //ʹ��TIM1����ж�    
                       	
	NVIC_InitStructure.NVIC_IRQChannel=TIM1_UP_IRQn;              //����TIM1�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;       //��ռ���ȼ�1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=1;              //�����ȼ�1
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;                 //�ж�ͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);                               //�����ж�
	
	TIM_Cmd(TIM1,ENABLE);                                         //��TIM1   
}

void TIM1_ENABLE_1S(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;            //����һ�����ö�ʱ���ı���
	NVIC_InitTypeDef NVIC_InitStructure;                          //����һ�������жϵı���
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);               //�����ж��������飺��2�� �������ȼ���0 1 2 3 �����ȼ���0 1 2 3		
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);           //ʹ��TIM1ʱ��	
	
    TIM_DeInit(TIM1);                                             //��ʱ��1�Ĵ����ָ�Ĭ��ֵ
	TIM_TimeBaseInitStructure.TIM_Period = 10000-1; 	          //�����Զ���װ��ֵ
	TIM_TimeBaseInitStructure.TIM_Prescaler=3600-1;              //���ö�ʱ��Ԥ��Ƶ��
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter=2-1;          //���ö�ʱ���ظ�����2�Σ�TIM1��8 ����������ܣ�
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //���ϼ���ģʽ
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1;     //1��Ƶ
	TIM_TimeBaseInit(TIM1,&TIM_TimeBaseInitStructure);            //����TIM1
	
	TIM_ClearITPendingBit(TIM1,TIM_IT_Update);                    //�������жϱ�־λ
	TIM_ITConfig(TIM1,TIM_IT_Update,ENABLE);                      //ʹ��TIM1����ж�    
                       	
	NVIC_InitStructure.NVIC_IRQChannel=TIM1_UP_IRQn;              //����TIM1�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;       //��ռ���ȼ�1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=1;              //�����ȼ�1
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;                 //�ж�ͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);                               //�����ж�
	
	TIM_Cmd(TIM1,ENABLE);                                         //��TIM1   
}
