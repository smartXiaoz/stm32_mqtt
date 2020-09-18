

#ifndef __WIFI_H
#define __WIFI_H

#include "usart2.h"	    //包含需要的头文件

#define RESET_IO(x)    GPIO_WriteBit(GPIOA, GPIO_Pin_4, (BitAction)x)  //PA4控制WiFi的复位

#define WiFi_printf       u2_printf           //串口2控制 WiFi
#define WiFi_RxCounter    Usart2_RxCounter    //串口2控制 WiFi
#define WiFi_RX_BUF       Usart2_RxBuff       //串口2控制 WiFi
#define WiFi_RXBUFF_SIZE  USART2_RXBUFF_SIZE  //串口2控制 WiFi
#define WIFI_FLASH_SAVE_ADDR_SSID  0X08070300		//设置保存wifi FLASH 保存地址(必须为偶数，且其值要大于本代码所占用FLASH的大小+0X08000000)
#define WIFI_FLASH_SAVE_ADDR_PASSWORD  0X08070200
#define WIFI_CONNECT_FLAG_ADDR  0X08070100		//设置保存wifi FLASH 保存地址(必须为偶数，且其值要大于本代码所占用FLASH的大小+0X08000000)

#define SSID   "SciShine000"                     //路由器SSID名称
#define PASS   "ss85560552"                 //路由器密码
// #define SSID   "Redmi"                     //路由器SSID名称
// #define PASS   "12345678"                 //路由器密码
void WiFi_ResetIO_Init(void);
char WiFi_SendCmd(char *cmd, int timeout);
char WiFi_Reset(int timeout);
char WiFi_JoinAP(int timeout);
char WiFi_Connect_Server(int timeout);
char WiFi_Smartconfig(int timeout);
char WiFi_WaitAP(int timeout);
char WiFi_Connect_IoTServer(void);
u8 WIFI_SEND_CMD(char *cmd, char *check, int timeout);
void WIFI_AP_CONNECT(void);
void Wifi_send_str(const char *str);

extern unsigned char WIFI_CONNECT_INFORMATION[2][50];
extern unsigned char WIFI_CONNECT_FLAG[2];

#endif


