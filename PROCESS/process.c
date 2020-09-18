#include "process.h"
#include "stm32f10x.h"  //包含需要的头文件
#include "main.h"       //包含需要的头文件
#include "delay.h"      //包含需要的头文件
#include "usart1.h"     //包含需要的头文件
#include "usart2.h"     //包含需要的头文件
#include "timer1.h"     //包含需要的头文件
#include "timer2.h"     //包含需要的头文件
#include "timer3.h"     //包含需要的头文件
#include "timer4.h"     //包含需要的头文件
#include "wifi.h"	    //包含需要的头文件
#include "led.h"        //包含需要的头文件
#include "mqtt.h"       //包含需要的头文件
#include "key.h"        //包含需要的头文件
#include "dht11.h"
#include "iic.h"        //包含需要的头文件
#include "adc.h"
#include "ds18b20.h"
#include "cJSON.h"
#include "stmflash.h"
#include "rtc.h"
char PUMP_CONNECT_STATUS[8] = {0,0,0,0,0,0,0,0};            //水泵状态
u16 PUMP_SET_TIM[8] = {0,0,0,0,0,0,0,0};    //设置定时时间s
u16 PUMP_SET_AlarmTim[8][18];//当前年月日时分，触发年月日分秒，间隔月日分，定时分秒，17设置 校验（年只发两位2020发20）
u32 PUMP_SET_AlarmTim_interval[8][2];
//PUMP_SET_AlarmTim[0][0] 当前年份 PUMP_SET_AlarmTim[0][1] 当前月份
//PUMP_SET_AlarmTim[0][2] 当前日   PUMP_SET_AlarmTim[0][3] 当前时
//PUMP_SET_AlarmTim[0][4] 当前分钟 PUMP_SET_AlarmTim[0][5] 当前秒
//PUMP_SET_AlarmTim[0][6] 设定年份 PUMP_SET_AlarmTim[0][7] 设定月份
//PUMP_SET_AlarmTim[0][8] 设定日   PUMP_SET_AlarmTim[0][9] 设定时
//PUMP_SET_AlarmTim[0][10] 设定分钟 PUMP_SET_AlarmTim[0][11] 设定秒
//PUMP_SET_AlarmTim[0][12] 间隔日 
//PUMP_SET_AlarmTim[0][13] 间隔时 
//PUMP_SET_AlarmTim[0][14] 间隔分 
//PUMP_SET_AlarmTim[0][15] 倒计时分 
//PUMP_SET_AlarmTim[0][16] 倒计时秒
//PUMP_SET_AlarmTim[0][17] 标志位

/*-------------------------------------------------*/
/*函数名：连接到路由器                              */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void connect_toRouter(void) {
    u1_printf("flag:%c ,%c\r\n", WIFI_CONNECT_FLAG[0],WIFI_CONNECT_FLAG[1]);
    STMFLASH_Read(WIFI_CONNECT_FLAG_ADDR, (u16 *)WIFI_CONNECT_FLAG, sizeof(u16)*4);
    STMFLASH_Read(WIFI_FLASH_SAVE_ADDR_SSID, (u16*)WIFI_CONNECT_INFORMATION[0], sizeof(u16)*30);
    STMFLASH_Read(WIFI_FLASH_SAVE_ADDR_PASSWORD, (u16*)WIFI_CONNECT_INFORMATION[1], sizeof(u16)*30);
    u1_printf("pass:%s\r\n", WIFI_CONNECT_INFORMATION[1]);
    u1_printf("ssid:%s\r\n", WIFI_CONNECT_INFORMATION[0]);
    u1_printf("flag:%c ,%c\r\n", WIFI_CONNECT_FLAG[0],WIFI_CONNECT_FLAG[1]);
    if (WIFI_CONNECT_FLAG[0] == 'O' && WIFI_CONNECT_FLAG[1] == 'K') {        
        u1_printf("需要连接服务器\r\n");                 //串口输出信息
        TIM_Cmd(TIM4,DISABLE);                           //关闭TIM4 
        TIM_Cmd(TIM3,DISABLE);                           //关闭TIM3  
        WiFi_RxCounter=0;                                //WiFi接收数据量变量清零                        
        memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);          //清空WiFi接收缓冲区 
        MQTT_Buff_Init();
        if(WiFi_Connect_IoTServer()==0){   			     //如果WiFi连接云服务器函数返回0，表示正确，进入if
            u1_printf("建立TCP连接成功\r\n");            //串口输出信息
            Connect_flag = 1;                            //Connect_flag置1，表示连接成功	
            WiFi_RxCounter=0;                            //WiFi接收数据量变量清零                        
            memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);      //清空WiFi接收缓冲区 
            MQTT_Buff_ReInit();                          //重新初始化发送缓冲区                    
        }	       
    }else
    {
        WIFI_AP_CONNECT();//ap连接输入密码
        while (WIFI_receive_passwad()==0);
    }
    
    			
}

unsigned char WIFI_receive_passwad(void) {
    char*str0 = strstr((const char*)WiFi_RX_BUF,(const char*)"$$");
    unsigned char temp_buf[2][50];
    unsigned char connect_flag[2];
    u8 i = 0;
    u8 temp = 0;
    u8 ssid_len = 0;
    u8 password_len = 0; 
    if (str0) {
        u1_printf("enter1\r\n");
        while (WiFi_RX_BUF[i++] != '\0') {
            if (WiFi_RX_BUF[i] == '$'&& WiFi_RX_BUF[i+1] == '$') {
                i += 2;
                ssid_len = ((WiFi_RX_BUF[i] - '0') * 10) + (WiFi_RX_BUF[i+1] - '0');
                i += 2;
                u1_printf("ssidlen:%d\r\n", ssid_len);
                while (ssid_len--)
                {
                    temp_buf[0][temp++] = WiFi_RX_BUF[i++];
                }
                temp_buf[0][temp++] = '\0';
                temp = 0;
                u1_printf("ssid:%s\r\n", temp_buf[0]);
                password_len = ((WiFi_RX_BUF[i] - '0') * 10) + (WiFi_RX_BUF[i+1] - '0');
                i += 2;
                u1_printf("password_len:%d\r\n", password_len);
                while (password_len--)
                {
                    temp_buf[1][temp++] = WiFi_RX_BUF[i++];
                }
                temp_buf[1][temp++] = '\0';
                u1_printf("pass:%s\r\n", temp_buf[1]);
                if (WiFi_RX_BUF[i] == '$') {
                    STMFLASH_Write(WIFI_FLASH_SAVE_ADDR_SSID, (u16*)temp_buf[0], sizeof(temp_buf[0]));
                    STMFLASH_Write(WIFI_FLASH_SAVE_ADDR_PASSWORD, (u16*)temp_buf[1], sizeof(temp_buf[1]));
                    connect_flag[0] = 'O';
                    connect_flag[1] = 'K';
                    STMFLASH_Write(WIFI_CONNECT_FLAG_ADDR, (u16*)connect_flag, sizeof(u16)*4);
                    WiFi_RxCounter=0;                                //WiFi接收数据量变量清零                        
                    memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);          //清空WiFi接收缓冲区
                    Wifi_send_str((const char *)temp_buf[0]);
                    Wifi_send_str((const char *)temp_buf[1]);
                    return 1;
                }else
                {
                    WiFi_RxCounter=0;                                //WiFi接收数据量变量清零                        
                    memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);          //清空WiFi接收缓冲区 
                }                   
            }             
        }         
    }
    return 0;
}
/*-------------------------------------------------------------*/
/*                     处理发送缓冲区数据                      */
/*-------------------------------------------------------------*/
void mqtt_txbuf_process(void) {
    if (MQTT_TxDataOutPtr != MQTT_TxDataInPtr) { //if成立的话，说明发送缓冲区有数据了
        //3种情况可进入if
        //第1种：0x10 连接报文
        //第2种：0x82 订阅报文，且ConnectPack_flag置位，表示连接报文成功
        //第3种：SubcribePack_flag置位，说明连接和订阅均成功，其他报文可发
        if ((MQTT_TxDataOutPtr[2] == 0x10) || ((MQTT_TxDataOutPtr[2] == 0x82) && (ConnectPack_flag == 1)) || (SubcribePack_flag == 1)) {
            u1_printf("发送数据:0x%x\r\n", MQTT_TxDataOutPtr[2]); //串口提示信息
            MQTT_TxData(MQTT_TxDataOutPtr);                       //发送数据
            MQTT_TxDataOutPtr += BUFF_UNIT;                       //指针下移
            if (MQTT_TxDataOutPtr == MQTT_TxDataEndPtr)           //如果指针到缓冲区尾部了
                MQTT_TxDataOutPtr = MQTT_TxDataBuf[0];            //指针归位到缓冲区开头
        } 				
    }//处理发送缓冲区数据的else if分支结尾
}

/*-------------------------------------------------------------*/
/*                     处理接收缓冲区数据                      */
/*-------------------------------------------------------------*/
void mqtt_rxbuf_process(void) {
    if(MQTT_RxDataOutPtr != MQTT_RxDataInPtr){  //if成立的话，说明接收缓冲区有数据了														
        u1_printf("接收到数据:");
        /*-----------------------------------------------------*/
        /*                    处理CONNACK报文                  */
        /*-----------------------------------------------------*/				
        //if判断，如果第一个字节是0x20，表示收到的是CONNACK报文
        //接着我们要判断第4个字节，看看CONNECT报文是否成功
        if(MQTT_RxDataOutPtr[2]==0x20){             			
            switch(MQTT_RxDataOutPtr[5]){					
                case 0x00 : u1_printf("CONNECT报文成功\r\n");                            //串口输出信息	
                            ConnectPack_flag = 1;                                        //CONNECT报文成功，订阅报文可发
                            break;                                                       //跳出分支case 0x00                                              
                case 0x01 : u1_printf("连接已拒绝，不支持的协议版本，准备重启\r\n");     //串口输出信息
                            Connect_flag = 0;                                            //Connect_flag置零，重启连接
                            break;                                                       //跳出分支case 0x01   
                case 0x02 : u1_printf("连接已拒绝，不合格的客户端标识符，准备重启\r\n"); //串口输出信息
                            Connect_flag = 0;                                            //Connect_flag置零，重启连接
                            break;                                                       //跳出分支case 0x02 
                case 0x03 : u1_printf("连接已拒绝，服务端不可用，准备重启\r\n");         //串口输出信息
                            Connect_flag = 0;                                            //Connect_flag置零，重启连接
                            break;                                                       //跳出分支case 0x03
                case 0x04 : u1_printf("连接已拒绝，无效的用户名或密码，准备重启\r\n");   //串口输出信息
                            Connect_flag = 0;                                            //Connect_flag置零，重启连接						
                            break;                                                       //跳出分支case 0x04
                case 0x05 : u1_printf("连接已拒绝，未授权，准备重启\r\n");               //串口输出信息
                            Connect_flag = 0;                                            //Connect_flag置零，重启连接						
                            break;                                                       //跳出分支case 0x05 		
                default   : u1_printf("连接已拒绝，未知状态，准备重启\r\n");             //串口输出信息 
                            Connect_flag = 0;                                            //Connect_flag置零，重启连接					
                            break;                                                       //跳出分支case default 								
            }				
        }			
        //if判断，第一个字节是0x90，表示收到的是SUBACK报文
        //接着我们要判断订阅回复，看看是不是成功
        else if(MQTT_RxDataOutPtr[2]==0x90){ 
                switch(MQTT_RxDataOutPtr[6]){					
                case 0x00 :
                case 0x01 : u1_printf("订阅成功\r\n");            //串口输出信息
                            SubcribePack_flag = 1;                //SubcribePack_flag置1，表示订阅报文成功，其他报文可发送
                            Ping_flag = 0;                        //Ping_flag清零
                            TIM3_ENABLE_30S();                    //启动30s的PING定时器
                            TIM2_ENABLE_30S();                    //启动30s的上传数据的定时器
                            TempHumi_State();                     //先发一次数据
                            break;                                //跳出分支                                             
                default   : u1_printf("订阅失败，准备重启\r\n");  //串口输出信息 
                            Connect_flag = 0;                     //Connect_flag置零，重启连接
                            break;                                //跳出分支 								
            }					
        }
        //if判断，第一个字节是0xD0，表示收到的是PINGRESP报文
        else if(MQTT_RxDataOutPtr[2]==0xD0){ 
            u1_printf("PING报文回复\r\n"); 		  //串口输出信息 
            if(Ping_flag==1){                     //如果Ping_flag=1，表示第一次发送
                    Ping_flag = 0;    				  //要清除Ping_flag标志
            }else if(Ping_flag>1){ 				  //如果Ping_flag>1，表示是多次发送了，而且是2s间隔的快速发送
                Ping_flag = 0;     				  //要清除Ping_flag标志
                TIM3_ENABLE_30S(); 				  //PING定时器重回30s的时间
            }				
        }	
        //if判断，如果第一个字节是0x30，表示收到的是服务器发来的推送数据
        //我们要提取控制命令
        else if((MQTT_RxDataOutPtr[2]==0x30)){ 
            u1_printf("服务器等级0推送\r\n"); 		   //串口输出信息
            u1_printf(MQTT_RxDataOutPtr);
            u1_printf("\r\n"); 		   //串口输出信息
            MQTT_DealPushdata_Qs0(MQTT_RxDataOutPtr);  //处理等级0推送数据
        }				
                        
        MQTT_RxDataOutPtr += BUFF_UNIT;                     //指针下移
        if(MQTT_RxDataOutPtr==MQTT_RxDataEndPtr)            //如果指针到缓冲区尾部了
            MQTT_RxDataOutPtr = MQTT_RxDataBuf[0];          //指针归位到缓冲区开头                        
    }//处理接收缓冲区数据的else if分支结尾
}
/*-------------------------------------------------------------*/
/*                     处理命令缓冲区数据                      */
/*-------------------------------------------------------------*/
void mqtt_cmdbuf_process(void){  
    if(MQTT_CMDOutPtr != MQTT_CMDInPtr){                             //if成立的话，说明命令缓冲区有数据了			       
        u1_printf("命令:%s\r\n",&MQTT_CMDOutPtr[2]);                 //串口输出信息
        mqtt_CMD_analyzing(&MQTT_CMDOutPtr[2]);
        MQTT_CMDOutPtr += BUFF_UNIT;                             	 //指针下移
        if(MQTT_CMDOutPtr==MQTT_CMDEndPtr)           	             //如果指针到缓冲区尾部了
            MQTT_CMDOutPtr = MQTT_CMDBuf[0];          	             //指针归位到缓冲区开头				
    }//处理命令缓冲区数据的else if分支结尾	
}

void setPost_JSON_str() {
    cJSON * json =  cJSON_CreateObject();//创建数据包
    cJSON * item =  cJSON_CreateObject();//创建数据包（二级）
    char *temp;
	
    cJSON_AddItemToObject(json, "method", cJSON_CreateString("thing.event.property.post"));//添加数据
    cJSON_AddItemToObject(json, "id", cJSON_CreateString("203302322"));//添加数据
    cJSON_AddItemToObject(json, "params", item);//添加数据
    cJSON_AddItemToObject(item, "temperature", cJSON_CreateNumber(10.000000));//添加数据
    cJSON_AddItemToObject(item, "humdity0", cJSON_CreateNumber(40.000000));//添加数据
    cJSON_AddItemToObject(json, "version", cJSON_CreateString("1.0.0"));//添加数据
    
    temp = cJSON_Print(json);
    
    MQTT_PublishQs0(P_TOPIC_NAME, temp, strlen(temp));

    cJSON_Delete(json);//释放内存
    cJSON_Delete(item);
}

void mqtt_CMD_analyzing(unsigned char *cmd_json) {
    u8 i = 0;
    char *temp_str;
    char *temp_str1;
    cJSON *json;
    cJSON *json_reset_connect;
    cJSON *json_setPumpStatus;
    cJSON *json_setPumpTim;
    cJSON *json_humidity_threshold;
    json = cJSON_Parse(cmd_json); //解析成json形式
    json_reset_connect = cJSON_GetObjectItem(json, "reset_conncet");
    json_setPumpStatus      = cJSON_GetObjectItem( json , "setPumpStatus");
    json_setPumpTim      = cJSON_GetObjectItem( json , "setPumpTim");
    json_humidity_threshold = cJSON_GetObjectItem( json , "humidity_threshold");
    if (json_reset_connect != NULL) {
        Connect_flag = 0;
        STMFLASH_Write(WIFI_CONNECT_FLAG_ADDR, (u16*)"EE", sizeof(u16)*4);
        WiFi_RxCounter=0;                                //WiFi接收数据量变量清零                        
        memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);          //清空WiFi接收缓冲区
        SYS_RST();
    }
    if (json_setPumpStatus != NULL) {
        temp_str = json_setPumpStatus->valuestring;
        u1_printf(temp_str);
        for (i = 0; i < 8; i++) {
            if (temp_str[i] == '0') {
                u1_printf("i:%d 关", i);
                PUMP(i) = 0;
                PAout(8) = 0;
            }
            else if (temp_str[i] == '1') {
                u1_printf("i:%d 开", i);
                PUMP(i) = 1;
                PAout(8) = 1;
            }
        }
    }
    
    if (json_humidity_threshold != NULL) {
        u1_printf("\r\njson_humidity_threshold:%s\r\n", json_humidity_threshold->valuestring);
    }

    if (json_setPumpTim != NULL) {
        u1_printf("\r\nPUMP_SET_TIM:%d\r\n", json_setPumpTim->valuestring);
        pumpSetTim(json_setPumpTim->valuestring);
    }
    cJSON_Delete(json);  //释放内存 
    cJSON_Delete(json_reset_connect);  //释放内存
    cJSON_Delete(json_setPumpStatus);  //释放内存 
    cJSON_Delete(json_humidity_threshold);  //释放内存 
    cJSON_Delete(json_setPumpTim);  //释放内存 
    WiFi_RxCounter=0;                                //WiFi接收数据量变量清零                        
    memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);          //清空WiFi接收缓冲区 
}

void pumpSetTim(char *str) {
    u8 ch[50];
    u8 index = 0;
    u8 i;
    u8 j = 0;
    
    index = str[0] - '0';
    PUMP_SET_AlarmTim[index][17] = 1;//表示该目标被初始化设置
    // u16 year, month, day, hour, minute, month_add, day_add, hour_add, minute_add, sec_add;
    for (i = 0; i < strlen(str); i++) {
        ch[i] = str[i] - '0';    
    }
    // RTC_Set();
    i = 1;
    for (j = 0; i < strlen(str); j++) {
        PUMP_SET_AlarmTim[index][j] = ch[i] * 10 + ch[i + 1];
        i += 2;
    }
    PUMP_SET_AlarmTim_interval[index][0] = (PUMP_SET_AlarmTim[index][12] * 24 * 60 + PUMP_SET_AlarmTim[index][13] * 60 + PUMP_SET_AlarmTim[index][14])*60;
    PUMP_SET_AlarmTim_interval[index][1] = (PUMP_SET_AlarmTim[index][12] * 24 * 60 + PUMP_SET_AlarmTim[index][13] * 60 + PUMP_SET_AlarmTim[index][14])*60;
    u1_printf("index:%d\r\n", index);
    u1_printf("present:%d-%d-%d-%d-%d-%d\r\n",PUMP_SET_AlarmTim[index][0] + 2000,PUMP_SET_AlarmTim[index][1],PUMP_SET_AlarmTim[index][2],PUMP_SET_AlarmTim[index][3],PUMP_SET_AlarmTim[index][4],PUMP_SET_AlarmTim[index][5]);
    u1_printf("setTim:%d-%d-%d-%d-%d-%d\r\n", PUMP_SET_AlarmTim[index][6] + 2000,PUMP_SET_AlarmTim[index][7],PUMP_SET_AlarmTim[index][8],PUMP_SET_AlarmTim[index][9],PUMP_SET_AlarmTim[index][10],PUMP_SET_AlarmTim[index][11]);
    u1_printf("间隔:%d-%d-%d--------%d\r\n", PUMP_SET_AlarmTim[index][12],PUMP_SET_AlarmTim[index][13],PUMP_SET_AlarmTim[index][14],PUMP_SET_AlarmTim_interval[index][0]);
    u1_printf("alarm:%d-%d\r\n", PUMP_SET_AlarmTim[index][15],PUMP_SET_AlarmTim[index][16]);
    RTC_Set(PUMP_SET_AlarmTim[index][0] + 2000, PUMP_SET_AlarmTim[index][1], PUMP_SET_AlarmTim[index][2], PUMP_SET_AlarmTim[index][3], PUMP_SET_AlarmTim[index][4], PUMP_SET_AlarmTim[index][5]);
}
