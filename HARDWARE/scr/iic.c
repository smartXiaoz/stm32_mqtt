/*-------------------------------------------------*/
/*            ��γ����STM32ϵ�п�����              */
/*-------------------------------------------------*/
/*                                                 */
/*            ʵ��IIc���߹��ܵ�Դ�ļ�              */
/*                                                 */
/*-------------------------------------------------*/

#include "stm32f10x.h"  //������Ҫ��ͷ�ļ�
#include "delay.h"      //������Ҫ��ͷ�ļ�
#include "iic.h"        //������Ҫ��ͷ�ļ�

/*-------------------------------------------------*/
/*����������ʼ��IIC�ӿ�                            */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void IIC_Init(void)
{			
	GPIO_InitTypeDef  GPIO_InitStructure;                      //����һ������IO�ı���

	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE );    //ʹ��GPIOBʱ��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 ;    //׼������PB6 PB7
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;           //���������ʽ  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;          //IO����50M
	GPIO_Init(GPIOB, &GPIO_InitStructure);                     //����PB6 PB7
	
	IIC_SCL_H;   //SCL����
	IIC_SDA_H;   //SDA����
}
/*-------------------------------------------------*/
/*��������IIC��ʼ�ź�                              */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void IIC_Start(void)
{
	SDA_OUT();     //SDA�����ģʽ
    IIC_SDA_H;	   //SDA����  
	IIC_SCL_H;     //SCL����	  
	Delay_Us(4);   //��ʱ
 	IIC_SDA_L;     //SCL�ߵ�ƽ��ʱ��SDA�ɸߵ��ͣ���ʼ�ź�
}
/*-------------------------------------------------*/
/*��������IICֹͣ�ź�                              */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void IIC_Stop(void)
{
	SDA_OUT();     //SDA�����ģʽ
	IIC_SDA_L;     //SDA����
	IIC_SCL_H;     //SCL����
 	Delay_Us(4);   //��ʱ
	IIC_SDA_H;     //SCL�ߵ�ƽ��ʱ��SDA�ɵ͵��ߣ�����ֹͣ�ź�					   	
}
/*-------------------------------------------------*/
/*���������ȴ�Ӧ��                                 */
/*��  ������                                       */
/*����ֵ��0���ɹ�  1��ʧ��                         */
/*-------------------------------------------------*/
char IIC_Wait_Ack(void)
{
	unsigned char timeout=0; //����һ�����������ڼ��㳬ʱʱ��
	
	IIC_SDA_H;	             //SDA����
	SDA_IN();                //SDA������ģʽ 
	IIC_SCL_H;	             //SCL����
	 
	while(READ_SDA){         //�ȴ�SDA��͵�ƽ����ʾӦ��������Ȼһֱwhileѭ����ֱ����ʱ	
		timeout++;           //��ʱ����+1
		if(timeout>250){     //�������250	
			IIC_Stop();      //����ֹͣ�ź�
			return 1;        //����1����ʾʧ��
		}
		Delay_Us(2);         //��ʱ
	}
	IIC_SCL_L;               //SCL����
	Delay_Us(20);            //��ʱ	
	return 0;                //����0����ʾ�ɹ�
} 	
/*-------------------------------------------------*/
/*������������һ���ֽ�                             */
/*��  ����txd�����͵��ֽ�                          */
/*����ֵ����                                       */
/*-------------------------------------------------*/ 			  
void IIC_Send_Byte(unsigned char txd)
{                        
    unsigned char t;                   //����һ����������forѭ�� 
	
	SDA_OUT(); 	                       //SDA�����ģʽ 
    IIC_SCL_L;                         //SCL���ͣ���ʼ���ݴ���
    for(t=0;t<8;t++){                  //forѭ����һλһλ�ķ��ͣ������λ λ7��ʼ                 
        IIC_SDA_OUT(((txd&0x80)>>7));  //��SDA������
        txd<<=1; 	                   //����һλ��׼����һ�η���
		Delay_Us(2);                   //��ʱ
		IIC_SCL_H;                     //SCL����
		Delay_Us(2);                   //��ʱ
		IIC_SCL_L;	                   //SCL����
    }	 
} 	    
/*-------------------------------------------------*/
/*����������ȡһ���ֽ�                             */
/*��  ����ack���Ƿ���Ӧ�� 1:�� 0������           */
/*����ֵ����ȡ������                               */
/*-------------------------------------------------*/   
unsigned char IIC_Read_Byte(unsigned char ack)
{
	unsigned char i;           //����һ����������forѭ�� 
	unsigned char receive;     //����һ���������ڱ�����յ�������
		
	receive = 0;               //���receive��׼����������
	SDA_IN();                  //SDA����Ϊ����
    IIC_SCL_L;                 //SCL����
    Delay_Us(20);               //��ʱ	
	for(i=0;i<8;i++){          //forѭ����һλһλ�Ķ�ȡ	 
		IIC_SCL_H;             //SCL���� 
		Delay_Us(20);           //��ʱ
        receive<<=1;           //����һλ��׼���´εĶ�ȡ
        if(READ_SDA)receive++; //��ȡ����
		Delay_Us(20);           //��ʱ
		IIC_SCL_L;             //SCL����  
		Delay_Us(20);           //��ʱ 		
    }					 
    if(!ack){        //����Ҫ����
		SDA_OUT();   //SDA�����ģʽ 
		IIC_SDA_H;   //ʱ������ĸߵ�ƽ�ڼ䱣���ȶ��ĸߵ�ƽ
		IIC_SCL_H;   //SCL����
		Delay_Us(2); //��ʱ
		IIC_SCL_L;   //SCL���� 
	}else{           //��Ҫ����
		SDA_OUT();   //SDA�����ģʽ 
		IIC_SDA_L;   //ʱ������ĸߵ�ƽ�ڼ䱣���ȶ��ĵ͵�ƽ
		IIC_SCL_H;   //SCL����
		Delay_Us(2); //��ʱ
		IIC_SCL_L;   //SCL����
		IIC_SDA_H;   //SDA����
	}
    return receive;
}
