/*-------------------------------------------------*/
/*            超纬电子STM32系列开发板              */
/*-------------------------------------------------*/
/*                                                 */
/*              实现DHT12功能的源文件              */
/*                                                 */
/*-------------------------------------------------*/

#include "stm32f10x.h"  //包含需要的头文件
#include "aht10.h"      //包含需要的头文件
#include "delay.h" 	    //包含需要的头文件			 
#include "iic.h"        //包含需要的头文件

/*-------------------------------------------------*/
/*函数名：向AHT10发送软复位命令                    */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void AHT10_Reset(void)
{
	IIC_Start();                    //IIC开始信号
	IIC_Send_Byte(0x70);            //发送器件地址+写操作	   	
	IIC_Wait_Ack();                 //等待应答
    IIC_Send_Byte(0xBA);            //发送复位命令 
	IIC_Wait_Ack();	                //等待应答 
    IIC_Stop();                     //停止信号
}
/*-------------------------------------------------*/
/*函数名：向AHT10发送读温湿度数据的命令            */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void AHT10_ReadData(void) 
{
	IIC_Start();                    //IIC开始信号
	IIC_Send_Byte(0x70);            //发送器件地址+写操作	   	
	IIC_Wait_Ack();                 //等待应答
    IIC_Send_Byte(0xAC);            //发送命令
	IIC_Wait_Ack();	                //等待应答 
	IIC_Send_Byte(0x33);            //发送数据	   	
	IIC_Wait_Ack();                 //等待应答
    IIC_Send_Byte(0x00);            //发送需要读数据的地址 
	IIC_Wait_Ack();	                //等待数据 
    IIC_Stop();                     //停止信号
}
/*-------------------------------------------------*/
/*函数名：读取AHT10的状态寄存器                    */
/*参  数：无                                       */
/*返回值：寄存器值                                 */
/*-------------------------------------------------*/
char AHT10_ReadStatus(void)
{
	char Status;
	
	IIC_Start();                    //IIC开始信号
	IIC_Send_Byte(0x71);            //发送器件地址+读操作
	IIC_Wait_Ack();	                //等待应答 
	Status = IIC_Read_Byte(0);      //读一字节数据，不发送ACK信号 
    IIC_Stop();                     //停止信号

	return Status;                  //返回状态寄存器的值
}
/*-------------------------------------------------*/
/*函数名：查询工作模式和校准位正确与否             */
/*参  数：无                                       */
/*返回值：0正确 1失败                              */
/*-------------------------------------------------*/
char AHT10_Read_CalEnable(void)  
{
	char val = 0;
 
	val = AHT10_ReadStatus();  //读取状态寄存器
	if((val & 0x68)==0x08)     //判断工作模式和校准输出是否有效
		return 0;              //正确返回0
	else  
		return 1;              //失败返回1
 }
/*-------------------------------------------------*/
/*函数名：初始化AHT10                              */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
char AHT10_Init(void)   
{	   
	char times;
	
	IIC_Start();                    //IIC开始信号
	IIC_Send_Byte(0x70);            //发送器件地址+写操作	   	
	IIC_Wait_Ack();                 //等待应答
    IIC_Send_Byte(0xE1);            //发送初始化命令 
	IIC_Wait_Ack();	                //等待应答 
	IIC_Send_Byte(0x08);            //发送数据	   	
	IIC_Wait_Ack();                 //等待应答
    IIC_Send_Byte(0x00);            //发送数据
	IIC_Wait_Ack();	                //等待应答 
    IIC_Stop();                     //停止信号
	delay_ms(500);                  //延时
	
	while(AHT10_Read_CalEnable()==1)   //查询工作模式和校准位正确与否 
	{   
		AHT10_Reset();                 //复位AHT10
		delay_ms(100);                 //延时

		IIC_Send_Byte(0x70);           //发送器件地址+写操作	   	
		IIC_Wait_Ack();                //等待应答
		IIC_Send_Byte(0xE1);           //发送初始化命令 
		IIC_Wait_Ack();	               //等待应答 
		IIC_Send_Byte(0x08);           //发送数据	   	
		IIC_Wait_Ack();                //等待应答
		IIC_Send_Byte(0x00);           //发送数据
		IIC_Wait_Ack();	               //等待应答 
		IIC_Stop();                    //停止信号

		times++;	                   //重复次数+1
		delay_ms(500);                 //延时
		if(times>=10)return 1;         //重复此时超过10了，失败返回1
	}
	return 0;                          //正确返回0
}
/*-------------------------------------------------*/
/*函数名：读传感器数据                             */
/*参  数：temp 保存温度数据                        */
/*参  数：humi 保存湿度数据                        */
/*返回值：读到的数据                               */
/*-------------------------------------------------*/
void AHT10_Data(double *temp, double *humi)
{				  		
	int     tdata;
	char    Redata[6];
		
	AHT10_ReadData();                 //读数据
	delay_ms(100);                    //延迟	
	IIC_Start();                      //IIC开始信号
	IIC_Send_Byte(0x71);              //发送器件地址+读操作				   
	IIC_Wait_Ack();	                  //等待应答
	Redata[0]=IIC_Read_Byte(1);       //读一字节数据，发送ACK信号 
	Redata[1]=IIC_Read_Byte(1);       //读一字节数据，发送ACK信号 
	Redata[2]=IIC_Read_Byte(1);       //读一字节数据，发送ACK信号 
	Redata[3]=IIC_Read_Byte(1);       //读一字节数据，发送ACK信号 
	Redata[4]=IIC_Read_Byte(1);       //读一字节数据，发送ACK信号
	Redata[5]=IIC_Read_Byte(0);       //读一字节数据，不发送ACK信号 	
    IIC_Stop();                       //停止信号	
	
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


