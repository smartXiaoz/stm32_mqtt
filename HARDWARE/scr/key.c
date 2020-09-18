/*-------------------------------------------------*/
/*            ��γ����STM32ϵ�п�����              */
/*-------------------------------------------------*/
/*                                                 */
/*            ʵ��ɨ�谴�����ܵ�Դ�ļ�             */
/*                                                 */
/*-------------------------------------------------*/

#include "stm32f10x.h"  //������Ҫ��ͷ�ļ�
#include "delay.h"      //������Ҫ��ͷ�ļ�
#include "key.h"      //������Ҫ��ͷ�ļ�

/*-------------------------------------------------*/
/*����������ʼ��KEY��������                        */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void KEY_Init(void)
{	
	GPIO_InitTypeDef GPIO_InitStructure;                       //����һ������IO�Ľṹ��
	
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA , ENABLE);    //ʹ��PA�˿�ʱ��
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0;                 //׼������PA0
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;   		   //�������룬Ĭ��״̬�ǵ͵�ƽ
	GPIO_Init(GPIOA, &GPIO_InitStructure);            		   //����PA0
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_1;                 //׼������PA1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;   		   //�������룬Ĭ��״̬�Ǹߵ�ƽ
	GPIO_Init(GPIOA, &GPIO_InitStructure);            		   //����PA1
} 

/*-------------------------------------------------*/
/*������������ɨ�躯��                             */
/*��  ������                                       */
/*����ֵ�����ް������£��еĻ����ĸ���������       */
/*-------------------------------------------------*/
char KEY_Scan(void)
{	 	  
	if(KEY1_IN_STA==1){                  //�жϿ���KEY1��IO�����ƽ���Ƿ��Ǹߵ�ƽ������ǽ���if	
		delay_ms(5);                     //��ʱ5ms����������������
		if(KEY1_IN_STA==1){              //����������Ӱ����ٴ��жϿ���KEY1��IO�����ƽ���Ƿ��Ǹߵ�ƽ������ǽ���if��˵������������	
			while(1){                    //�ȴ�����̧��
				if(KEY1_IN_STA==0){      //�жϿ���KEY1��IO�����ƽ���Ƿ��ǵ͵�ƽ������ǽ���if	
					delay_ms(5);         //��ʱ5ms����������������
					if(KEY1_IN_STA==0){  //����������Ӱ����ٴ��жϿ���KEY1��IO�����ƽ���Ƿ��ǵ͵�ƽ������ǽ���if��˵������̧����
						return 1;        //����1����ʾKEY1������
					}
				}
			}
		}
	}else if(KEY2_IN_STA==0){            //�жϿ���KEY2��IO�����ƽ���Ƿ��ǵ͵�ƽ������ǽ���if	
		delay_ms(5);                     //��ʱ5ms����������������
		if(KEY2_IN_STA==0){              //����������Ӱ����ٴ��жϿ���KEY2��IO�����ƽ���Ƿ��ǵ͵�ƽ������ǽ���if��˵������������	
			while(1){                    //�ȴ�����̧��
				if(KEY2_IN_STA==1){      //�жϿ���KEY2��IO�����ƽ���Ƿ��Ǹߵ�ƽ������ǽ���if
					delay_ms(5);         //��ʱ5ms����������������
					if(KEY2_IN_STA==1){  //����������Ӱ����ٴ��жϿ���KEY2��IO�����ƽ���Ƿ��Ǹߵ�ƽ������ǽ���if��˵������̧����
						return 2;        //����2����ʾKEY2������
					}
				}
			}
		}
	}	
	return 0;                            //���û�а������£�����0
}

/*-------------------------------------------------*/
/*�������������ⲿ�жϳ�ʼ������.                  */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/	   
void KEY_Exti_Init(void)
{
	NVIC_InitTypeDef   NVIC_InitStructure; //����һ������IO�Ľṹ��
	EXTI_InitTypeDef   EXTI_InitStructure; //����һ�������жϵĽṹ�� 

  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	          //ʹ��GPIO���ù���ʱ��
  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource0);    //����KEY1-PA0�ж���
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource1);    //����KEY2-PA1�ж���

  	EXTI_InitStructure.EXTI_Line = EXTI_Line0;	                  //׼�������ⲿ0�ж���
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	          //�ⲿ�ж�
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;        //�����ش����ж�
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;                     //ʹ��
  	EXTI_Init(&EXTI_InitStructure);	 	                          //�����ⲿ�ж�0��
	
  	EXTI_InitStructure.EXTI_Line = EXTI_Line1;	                  //׼������KEY2-PA1�ⲿ1�ж���
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	          //�ⲿ�ж�
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;       //�½��ش����ж�
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;                     //ʹ��
  	EXTI_Init(&EXTI_InitStructure);	 	                          //�����ⲿ�ж�1��

	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;			  //׼�������ⲿ�ж�0
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;     //��ռ���ȼ�3 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		      //�����ȼ�0
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			      //ʹ���ⲿ�ж�0ͨ��
  	NVIC_Init(&NVIC_InitStructure);                               //�����ⲿ�ж�0

    NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;			  //׼�������ⲿ�ж�1
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;     //��ռ���ȼ�3
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		      //�����ȼ�1
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				  //ʹ���ⲿ�ж�1ͨ��
  	NVIC_Init(&NVIC_InitStructure);                               //�����ⲿ�ж�1
}




















