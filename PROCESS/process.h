#ifndef __PROCESS_H
#define __PROCESS_H
#include "sys.h"
extern char PUMP_CONNECT_STATUS[8];
void connect_toRouter(void);
void mqtt_txbuf_process(void);
void mqtt_rxbuf_process(void);
void mqtt_cmdbuf_process(void);
void setPost_JSON_str(void);
void mqtt_CMD_analyzing(unsigned char *cmd_json);
unsigned char WIFI_receive_passwad(void);
void pumpSetTim(char *str);

extern u16 PUMP_SET_TIM[8];
extern char PUMP_CONNECT_STATUS[8];
extern u16 PUMP_SET_AlarmTim[8][18];
extern u32 PUMP_SET_AlarmTim_interval[8][2];
#endif
