/*-------------------------------------------------*/
/*            ��γ����STM32ϵ�п�����              */
/*-------------------------------------------------*/
/*                                                 */
/*              ʵ��DHT12���ܵ�Դ�ļ�              */
/*                                                 */
/*-------------------------------------------------*/

#include "stm32f10x.h"  //������Ҫ��ͷ�ļ�
#include "aht10.h"      //������Ҫ��ͷ�ļ�
#include "delay.h" 	    //������Ҫ��ͷ�ļ�			 
#include "iic.h"        //������Ҫ��ͷ�ļ�

/*-------------------------------------------------*/
/*����������AHT10������λ����                    */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void AHT10_Reset(void)
{
	IIC_Start();                    //IIC��ʼ�ź�
	IIC_Send_Byte(0x70);            //����������ַ+д����	   	
	IIC_Wait_Ack();                 //�ȴ�Ӧ��
    IIC_Send_Byte(0xBA);            //���͸�λ���� 
	IIC_Wait_Ack();	                //�ȴ�Ӧ�� 
    IIC_Stop();                     //ֹͣ�ź�
}
/*-------------------------------------------------*/
/*����������AHT10���Ͷ���ʪ�����ݵ�����            */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void AHT10_ReadData(void) 
{
	IIC_Start();                    //IIC��ʼ�ź�
	IIC_Send_Byte(0x70);            //����������ַ+д����	   	
	IIC_Wait_Ack();                 //�ȴ�Ӧ��
    IIC_Send_Byte(0xAC);            //��������
	IIC_Wait_Ack();	                //�ȴ�Ӧ�� 
	IIC_Send_Byte(0x33);            //��������	   	
	IIC_Wait_Ack();                 //�ȴ�Ӧ��
    IIC_Send_Byte(0x00);            //������Ҫ�����ݵĵ�ַ 
	IIC_Wait_Ack();	                //�ȴ����� 
    IIC_Stop();                     //ֹͣ�ź�
}
/*-------------------------------------------------*/
/*����������ȡAHT10��״̬�Ĵ���                    */
/*��  ������                                       */
/*����ֵ���Ĵ���ֵ                                 */
/*-------------------------------------------------*/
char AHT10_ReadStatus(void)
{
	char Status;
	
	IIC_Start();                    //IIC��ʼ�ź�
	IIC_Send_Byte(0x71);            //����������ַ+������
	IIC_Wait_Ack();	                //�ȴ�Ӧ�� 
	Status = IIC_Read_Byte(0);      //��һ�ֽ����ݣ�������ACK�ź� 
    IIC_Stop();                     //ֹͣ�ź�

	return Status;                  //����״̬�Ĵ�����ֵ
}
/*-------------------------------------------------*/
/*����������ѯ����ģʽ��У׼λ��ȷ���             */
/*��  ������                                       */
/*����ֵ��0��ȷ 1ʧ��                              */
/*-------------------------------------------------*/
char AHT10_Read_CalEnable(void)  
{
	char val = 0;
 
	val = AHT10_ReadStatus();  //��ȡ״̬�Ĵ���
	if((val & 0x68)==0x08)     //�жϹ���ģʽ��У׼����Ƿ���Ч
		return 0;              //��ȷ����0
	else  
		return 1;              //ʧ�ܷ���1
 }
/*-------------------------------------------------*/
/*����������ʼ��AHT10                              */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
char AHT10_Init(void)   
{	   
	char times;
	
	IIC_Start();                    //IIC��ʼ�ź�
	IIC_Send_Byte(0x70);            //����������ַ+д����	   	
	IIC_Wait_Ack();                 //�ȴ�Ӧ��
    IIC_Send_Byte(0xE1);            //���ͳ�ʼ������ 
	IIC_Wait_Ack();	                //�ȴ�Ӧ�� 
	IIC_Send_Byte(0x08);            //��������	   	
	IIC_Wait_Ack();                 //�ȴ�Ӧ��
    IIC_Send_Byte(0x00);            //��������
	IIC_Wait_Ack();	                //�ȴ�Ӧ�� 
    IIC_Stop();                     //ֹͣ�ź�
	delay_ms(500);                  //��ʱ
	
	while(AHT10_Read_CalEnable()==1)   //��ѯ����ģʽ��У׼λ��ȷ��� 
	{   
		AHT10_Reset();                 //��λAHT10
		delay_ms(100);                 //��ʱ

		IIC_Send_Byte(0x70);           //����������ַ+д����	   	
		IIC_Wait_Ack();                //�ȴ�Ӧ��
		IIC_Send_Byte(0xE1);           //���ͳ�ʼ������ 
		IIC_Wait_Ack();	               //�ȴ�Ӧ�� 
		IIC_Send_Byte(0x08);           //��������	   	
		IIC_Wait_Ack();                //�ȴ�Ӧ��
		IIC_Send_Byte(0x00);           //��������
		IIC_Wait_Ack();	               //�ȴ�Ӧ�� 
		IIC_Stop();                    //ֹͣ�ź�

		times++;	                   //�ظ�����+1
		delay_ms(500);                 //��ʱ
		if(times>=10)return 1;         //�ظ���ʱ����10�ˣ�ʧ�ܷ���1
	}
	return 0;                          //��ȷ����0
}
/*-------------------------------------------------*/
/*��������������������                             */
/*��  ����temp �����¶�����                        */
/*��  ����humi ����ʪ������                        */
/*����ֵ������������                               */
/*-------------------------------------------------*/
void AHT10_Data(double *temp, double *humi)
{				  		
	int     tdata;
	char    Redata[6];
		
	AHT10_ReadData();                 //������
	delay_ms(100);                    //�ӳ�	
	IIC_Start();                      //IIC��ʼ�ź�
	IIC_Send_Byte(0x71);              //����������ַ+������				   
	IIC_Wait_Ack();	                  //�ȴ�Ӧ��
	Redata[0]=IIC_Read_Byte(1);       //��һ�ֽ����ݣ�����ACK�ź� 
	Redata[1]=IIC_Read_Byte(1);       //��һ�ֽ����ݣ�����ACK�ź� 
	Redata[2]=IIC_Read_Byte(1);       //��һ�ֽ����ݣ�����ACK�ź� 
	Redata[3]=IIC_Read_Byte(1);       //��һ�ֽ����ݣ�����ACK�ź� 
	Redata[4]=IIC_Read_Byte(1);       //��һ�ֽ����ݣ�����ACK�ź�
	Redata[5]=IIC_Read_Byte(0);       //��һ�ֽ����ݣ�������ACK�ź� 	
    IIC_Stop();                       //ֹͣ�ź�	
	
	tdata = 0;
	tdata = (tdata|Redata[1])<<8;
	tdata = (tdata|Redata[2])<<8;
	tdata = (tdata|Redata[3]);
	tdata = (tdata >>4);
	*humi =  tdata*100.0/1024.0/1024.0;
	
	tdata = 0;
	tdata = (tdata|Redata[3])<<8;
	tdata = (tdata|Redata[4])<<8;
	tdata = (tdata|Redata[5]);
	tdata =  tdata&0xfffff;
	*temp =  tdata*200.0/1024.0/1024.0-50.0;
}


