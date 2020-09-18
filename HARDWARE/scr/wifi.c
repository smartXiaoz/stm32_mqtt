
#include "main.h"
#include "stm32f10x.h"  //包含需要的头文件
#include "wifi.h"	    //包含需要的头文件
#include "iic.h"        //包含需要的头文件
#include "24c02.h" 		//包含需要的头文件
#include "delay.h"	    //包含需要的头文件
#include "usart1.h"	    //包含需要的头文件
#include "led.h"        //包含需要的头文件
#include "mqtt.h"       //包含需要的头文件
#include "key.h"        //包含需要的头文件
#include "stmflash.h"

char wifi_mode = 0;     //联网模式 0：SSID和密码写在程序里   1：Smartconfig方式用APP发送
unsigned char WIFI_CONNECT_INFORMATION[2][50];
unsigned char WIFI_CONNECT_FLAG[2] = {'0','0'}; //0 未得到密码,1 得到密码

/*-------------------------------------------------*/
/*函数名：初始化WiFi的复位IO                       */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void WiFi_ResetIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;                      //定义一个设置IO端口参数的结构体
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA , ENABLE);   //使能PA端口时钟
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;                 //准备设置PA4
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;         //速率50Mhz
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   		  //推免输出方式
	GPIO_Init(GPIOA, &GPIO_InitStructure);            		  //设置PA4
	RESET_IO(1);                                              //复位IO拉高电平
}

void Wifi_send_str(const char *str) {
    u8 p[50];
    sprintf((char *)p, "AT+CIPSEND=0,%d", strlen(str));
    WIFI_SEND_CMD(p, "OK", 20);
    WiFi_printf("%s", str);
    delay_ms(100);
}
/*-------------------------------------------------*/
/*函数名：WiFi发送设置指令                         */
/*参  数：cmd：指令                                */
/*参  数：timeout：超时时间（100ms的倍数）         */
/*返回值：0：正确   其他：错误                     */
/*-------------------------------------------------*/
char WiFi_SendCmd(char *cmd, int timeout)
{
	WiFi_RxCounter=0;                           //WiFi接收数据量变量清零                        
	memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);     //清空WiFi接收缓冲区 
	WiFi_printf("%s\r\n",cmd);                  //发送指令
	while(timeout--){                           //等待超时时间到0
		delay_ms(100);                          //延时100ms
		if(strstr(WiFi_RX_BUF,"OK"))            //如果接收到OK表示指令成功
			break;       						//主动跳出while循环
		u1_printf("%d ",timeout);               //串口输出现在的超时时间
	}
	u1_printf("\r\n");                          //串口输出信息
	if(timeout<=0)return 1;                     //如果timeout<=0，说明超时时间到了，也没能收到OK，返回1
	else return 0;		         				//反之，表示正确，说明收到OK，通过break主动跳出while
}
/*-------------------------------------------------*/
/*函数名：WiFi复位                                 */
/*参  数：timeout：超时时间（100ms的倍数）         */
/*返回值：0：正确   其他：错误                     */
/*-------------------------------------------------*/
char WiFi_Reset(int timeout)
{
	RESET_IO(0);                                    //复位IO拉低电平
	delay_ms(500);                                  //延时500ms
	RESET_IO(1);                                    //复位IO拉高电平	
	while(timeout--){                               //等待超时时间到0
		delay_ms(100);                              //延时100ms
		if(strstr(WiFi_RX_BUF,"ready"))             //如果接收到ready表示复位成功
			break;       						    //主动跳出while循环
		u1_printf("%d ",timeout);                   //串口输出现在的超时时间
	}
	u1_printf("\r\n");                              //串口输出信息
	if(timeout<=0)return 1;                         //如果timeout<=0，说明超时时间到了，也没能收到ready，返回1
	else return 0;		         				    //反之，表示正确，说明收到ready，通过break主动跳出while
}
/*-------------------------------------------------*/
/*函数名：WiFi加入路由器指令                       */
/*参  数：timeout：超时时间（1s的倍数）            */
/*返回值：0：正确   其他：错误                     */
/*-------------------------------------------------*/
char WiFi_JoinAP(int timeout)
{		
	WiFi_RxCounter=0;                               //WiFi接收数据量变量清零                        
	memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);         //清空WiFi接收缓冲区
    u1_printf("conss:%s\r\n", (char *)WIFI_CONNECT_INFORMATION[0]);
    u1_printf("passs:%s\r\n", (char *)WIFI_CONNECT_INFORMATION[1]);
    WiFi_printf("AT+CWJAP=\"%s\",\"%s\"\r\n",(char *)WIFI_CONNECT_INFORMATION[0],(char *)WIFI_CONNECT_INFORMATION[1]); //发送指令	
	while(timeout--){                               //等待超时时间到0
		delay_ms(1000);                             //延时1s
		if(strstr(WiFi_RX_BUF,"\n\r\nOK")) //如果接收到WIFI GOT IP表示成功，但是正点原子输出形式是		WIFI GOT IP\n\r\nOK，老版本输出形式是WIFI GOT IP\r\n\r\nOK，为了通用，此处这样设置
			break;       						    //主动跳出while循环
		u1_printf("%d ",timeout);                   //串口输出现在的超时时间
	}
	u1_printf("\r\n");                              //串口输出信息
	if(timeout<=0)return 1;                         //如果timeout<=0，说明超时时间到了，也没能收到WIFI GOT IP，返回1
	return 0;                                       //正确，返回0
}
/*-------------------------------------------------*/
/*函数名：连接TCP服务器，并进入透传模式            */
/*参  数：timeout： 超时时间（100ms的倍数）        */
/*返回值：0：正确  其他：错误                      */
/*-------------------------------------------------*/
char WiFi_Connect_Server(int timeout)
{	
	WiFi_RxCounter=0;                               //WiFi接收数据量变量清零                        
	memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);         //清空WiFi接收缓冲区   
	WiFi_printf("AT+CIPSTART=\"TCP\",\"%s\",%d\r\n",ServerIP,ServerPort);//发送连接服务器指令
	while(timeout--){                               //等待超时与否
		delay_ms(100);                              //延时100ms	
		if(strstr(WiFi_RX_BUF ,"CONNECT"))          //如果接受到CONNECT表示连接成功
			break;                                  //跳出while循环
		if(strstr(WiFi_RX_BUF ,"CLOSED"))           //如果接受到CLOSED表示服务器未开启
			return 1;                               //服务器未开启返回1
		if(strstr(WiFi_RX_BUF ,"ALREADY CONNECTED"))//如果接受到ALREADY CONNECTED已经建立连接
			break;                               //已经建立连接返回2
		u1_printf("%d ",timeout);                   //串口输出现在的超时时间  
	}
	u1_printf("\r\n");                        //串口输出信息
	if(timeout<=0)return 3;                   //超时错误，返回3
	else                                      //连接成功，准备进入透传
	{
		u1_printf("连接服务器成功，准备进入透传\r\n");  //串口显示信息
		WiFi_RxCounter=0;                               //WiFi接收数据量变量清零                        
		memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);         //清空WiFi接收缓冲区     
		WiFi_printf("AT+CIPSEND\r\n");                  //发送进入透传指令
		while(timeout--){                               //等待超时与否
			delay_ms(100);                              //延时100ms	
			if(strstr(WiFi_RX_BUF,"\r\nOK\r\n\r\n>"))   //如果成立表示进入透传成功
				break;                          //跳出while循环
			u1_printf("%d ",timeout);           //串口输出现在的超时时间  
		}
		if(timeout<=0)return 4;                 //透传超时错误，返回4	
	}
	return 0;	                                //成功返回0	
}
/*-------------------------------------------------*/
/*函数名：                           */
/*参  数：char *cmd 命令 ,check 校验值,timeout：超时时间（1s的倍数）            */
/*返回值： 0成功,1失败                    */
/*-------------------------------------------------*/
u8 WIFI_SEND_CMD(char *cmd, char *check, int timeout) {
	WiFi_RxCounter=0;                               //WiFi接收数据量变量清零                        
	memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);         //清空WiFi接收缓冲区   
	WiFi_printf("%s\r\n",cmd);//发送连接服务器指令
	while(timeout--){                               //等待超时与否
		delay_ms(100);                              //延时100ms	
		if(strstr(WiFi_RX_BUF ,check))          //如果接受到CONNECT表示连接成功
			break;                                  //跳出while循环
		u1_printf("%d ",timeout);                   //串口输出现在的超时时间  
	}
    if (timeout < 0) {
		u1_printf("失败\r\n");
		return 1;
	}
        
    u1_printf("成功\r\n");
    return 0;
}
/*-------------------------------------------------*/
/*函数名：ap模式                           */
/*参  数：timeout：超时时间（1s的倍数）            */
/*返回值：                     */
/*-------------------------------------------------*/
void WIFI_AP_CONNECT(void) {
	// while (WiFi_Reset(50)==0)
	// {
	// 	u1_printf("重启成功\r\n"); 
    //     break;
    // }
	// delay_ms(1000);
	while (WIFI_SEND_CMD("AT+CWMODE=2", "OK", 20)==0)
	{
		u1_printf("cwmode\r\n"); 
        break;
	}
	delay_ms(100);
    while (WiFi_Reset(50)==0)
	{
		u1_printf("重启成功\r\n"); 
        break;
    }
    WIFI_SEND_CMD("AT+CWSAP=\"红宇\",\"12345678\",6,4", "", 20);
	delay_ms(100);
	WIFI_SEND_CMD("AT+RST", "OK", 20);
	delay_ms(1000);
	WIFI_SEND_CMD("AT+CIPMUX=1", "OK", 20);
	delay_ms(100);
	WIFI_SEND_CMD("AT+CIPSERVER=1,333", "OK", 20);
	delay_ms(100);	
}

/*-------------------------------------------------*/
/*函数名：WiFi_Smartconfig                         */
/*参  数：timeout：超时时间（1s的倍数）            */
/*返回值：0：正确   其他：错误                     */
/*-------------------------------------------------*/
char WiFi_Smartconfig(int timeout)
{
	
	WiFi_RxCounter=0;                           //WiFi接收数据量变量清零                        
	memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);     //清空WiFi接收缓冲区     
	while(timeout--){                           //等待超时时间到0
		delay_ms(1000);                         //延时1s
		if(strstr(WiFi_RX_BUF,"connected"))     //如果串口接受到connected表示成功
			break;                              //跳出while循环  
		u1_printf("%d ",timeout);               //串口输出现在的超时时间  
	}	
	u1_printf("\r\n");                          //串口输出信息
	if(timeout<=0)return 1;                     //超时错误，返回1
	return 0;                                   //正确返回0
}

/*-------------------------------------------------*/
/*函数名：等待加入路由器                           */
/*参  数：timeout：超时时间（1s的倍数）            */
/*返回值：0：正确   其他：错误                     */
/*-------------------------------------------------*/
char WiFi_WaitAP(int timeout)
{		
	while(timeout--){                               //等待超时时间到0
		delay_ms(1000);                             //延时1s
		if(strstr(WiFi_RX_BUF,"WIFI GOT IP"))       //如果接收到WIFI GOT IP表示成功
			break;       						    //主动跳出while循环
		u1_printf("%d ",timeout);                   //串口输出现在的超时时间
	}
	u1_printf("\r\n");                              //串口输出信息
	if(timeout<=0)return 1;                         //如果timeout<=0，说明超时时间到了，也没能收到WIFI GOT IP，返回1
	return 0;                                       //正确，返回0
}
/*-------------------------------------------------*/
/*函数名：WiFi连接服务器                           */
/*参  数：无                                       */
/*返回值：0：正确   其他：错误                     */
/*-------------------------------------------------*/
char WiFi_Connect_IoTServer(void)
{	
	u1_printf("准备复位模块\r\n");                     //串口提示数据
	if(WiFi_Reset(50)){                                //复位，100ms超时单位，总计5s超时时间
		u1_printf("复位失败，准备重启\r\n");           //返回非0值，进入if，串口提示数据
		return 1;                                      //返回1
	}else u1_printf("复位成功\r\n");                   //串口提示数据
	
	u1_printf("准备设置STA模式\r\n");                  //串口提示数据
	if(WiFi_SendCmd("AT+CWMODE=1",50)){                //设置STA模式，100ms超时单位，总计5s超时时间
		u1_printf("设置STA模式失败，准备重启\r\n");    //返回非0值，进入if，串口提示数据
		return 2;                                      //返回2
	}else u1_printf("设置STA模式成功\r\n");            //串口提示数据
	
	if(wifi_mode==0){                                      //如果联网模式=0：SSID和密码写在程序里 
		u1_printf("准备取消自动连接\r\n");                 //串口提示数据
		if(WiFi_SendCmd("AT+CWAUTOCONN=0",50)){            //取消自动连接，100ms超时单位，总计5s超时时间
			u1_printf("取消自动连接失败，准备重启\r\n");   //返回非0值，进入if，串口提示数据
			return 3;                                      //返回3
		}else u1_printf("取消自动连接成功\r\n");           //串口提示数据
				
		u1_printf("准备连接路由器\r\n");                   //串口提示数据	
		if(WiFi_JoinAP(30)){                               //连接路由器,1s超时单位，总计30s超时时间
			u1_printf("连接路由器失败，准备重启\r\n");     //返回非0值，进入if，串口提示数据
			STMFLASH_Write(WIFI_CONNECT_FLAG_ADDR, (u16*)"EE", sizeof(u16)*4);
            return 4;                                      //返回4	
		}else u1_printf("连接路由器成功\r\n");             //串口提示数据			
	}
	
	u1_printf("准备设置透传\r\n");                     //串口提示数据
	if(WiFi_SendCmd("AT+CIPMODE=1",50)){               //设置透传，100ms超时单位，总计5s超时时间
		u1_printf("设置透传失败，准备重启\r\n");       //返回非0值，进入if，串口提示数据
		return 8;                                      //返回8
	}else u1_printf("设置透传成功\r\n");               //串口提示数据
	
	u1_printf("准备关闭多路连接\r\n");                 //串口提示数据
	if(WiFi_SendCmd("AT+CIPMUX=0",50)){                //关闭多路连接，100ms超时单位，总计5s超时时间
		u1_printf("关闭多路连接失败，准备重启\r\n");   //返回非0值，进入if，串口提示数据
		return 9;                                      //返回9
	}else u1_printf("关闭多路连接成功\r\n");           //串口提示数据
	
	u1_printf("准备连接服务器\r\n");                   //串口提示数据
	if(WiFi_Connect_Server(100)){                      //连接服务器，100ms超时单位，总计10s超时时间
		u1_printf("连接服务器失败，准备重启\r\n");     //返回非0值，进入if，串口提示数据
		
		return 10;                                     //返回10
	}else u1_printf("连接服务器成功\r\n");             //串口提示数据	
	
	return 0;                                          //正确返回0
	
}


