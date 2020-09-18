

#ifndef __WIFI_H
#define __WIFI_H

#include "usart2.h"	    //������Ҫ��ͷ�ļ�

#define RESET_IO(x)    GPIO_WriteBit(GPIOA, GPIO_Pin_4, (BitAction)x)  //PA4����WiFi�ĸ�λ

#define WiFi_printf       u2_printf           //����2���� WiFi
#define WiFi_RxCounter    Usart2_RxCounter    //����2���� WiFi
#define WiFi_RX_BUF       Usart2_RxBuff       //����2���� WiFi
#define WiFi_RXBUFF_SIZE  USART2_RXBUFF_SIZE  //����2���� WiFi
#define WIFI_FLASH_SAVE_ADDR_SSID  0X08070300		//���ñ���wifi FLASH �����ַ(����Ϊż��������ֵҪ���ڱ�������ռ��FLASH�Ĵ�С+0X08000000)
#define WIFI_FLASH_SAVE_ADDR_PASSWORD  0X08070200
#define WIFI_CONNECT_FLAG_ADDR  0X08070100		//���ñ���wifi FLASH �����ַ(����Ϊż��������ֵҪ���ڱ�������ռ��FLASH�Ĵ�С+0X08000000)

#define SSID   "SciShine000"                     //·����SSID����
#define PASS   "ss85560552"                 //·��������
// #define SSID   "Redmi"                     //·����SSID����
// #define PASS   "12345678"                 //·��������
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


