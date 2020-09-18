#include "process.h"
#include "stm32f10x.h"  //������Ҫ��ͷ�ļ�
#include "main.h"       //������Ҫ��ͷ�ļ�
#include "delay.h"      //������Ҫ��ͷ�ļ�
#include "usart1.h"     //������Ҫ��ͷ�ļ�
#include "usart2.h"     //������Ҫ��ͷ�ļ�
#include "timer1.h"     //������Ҫ��ͷ�ļ�
#include "timer2.h"     //������Ҫ��ͷ�ļ�
#include "timer3.h"     //������Ҫ��ͷ�ļ�
#include "timer4.h"     //������Ҫ��ͷ�ļ�
#include "wifi.h"	    //������Ҫ��ͷ�ļ�
#include "led.h"        //������Ҫ��ͷ�ļ�
#include "mqtt.h"       //������Ҫ��ͷ�ļ�
#include "key.h"        //������Ҫ��ͷ�ļ�
#include "dht11.h"
#include "iic.h"        //������Ҫ��ͷ�ļ�
#include "adc.h"
#include "ds18b20.h"
#include "cJSON.h"
#include "stmflash.h"
#include "rtc.h"
char PUMP_CONNECT_STATUS[8] = {0,0,0,0,0,0,0,0};            //ˮ��״̬
u16 PUMP_SET_TIM[8] = {0,0,0,0,0,0,0,0};    //���ö�ʱʱ��s
u16 PUMP_SET_AlarmTim[8][18];//��ǰ������ʱ�֣����������շ��룬������շ֣���ʱ���룬17���� У�飨��ֻ����λ2020��20��
u32 PUMP_SET_AlarmTim_interval[8][2];
//PUMP_SET_AlarmTim[0][0] ��ǰ��� PUMP_SET_AlarmTim[0][1] ��ǰ�·�
//PUMP_SET_AlarmTim[0][2] ��ǰ��   PUMP_SET_AlarmTim[0][3] ��ǰʱ
//PUMP_SET_AlarmTim[0][4] ��ǰ���� PUMP_SET_AlarmTim[0][5] ��ǰ��
//PUMP_SET_AlarmTim[0][6] �趨��� PUMP_SET_AlarmTim[0][7] �趨�·�
//PUMP_SET_AlarmTim[0][8] �趨��   PUMP_SET_AlarmTim[0][9] �趨ʱ
//PUMP_SET_AlarmTim[0][10] �趨���� PUMP_SET_AlarmTim[0][11] �趨��
//PUMP_SET_AlarmTim[0][12] ����� 
//PUMP_SET_AlarmTim[0][13] ���ʱ 
//PUMP_SET_AlarmTim[0][14] ����� 
//PUMP_SET_AlarmTim[0][15] ����ʱ�� 
//PUMP_SET_AlarmTim[0][16] ����ʱ��
//PUMP_SET_AlarmTim[0][17] ��־λ

/*-------------------------------------------------*/
/*�����������ӵ�·����                              */
/*��  ������                                       */
/*����ֵ����                                       */
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
        u1_printf("��Ҫ���ӷ�����\r\n");                 //���������Ϣ
        TIM_Cmd(TIM4,DISABLE);                           //�ر�TIM4 
        TIM_Cmd(TIM3,DISABLE);                           //�ر�TIM3  
        WiFi_RxCounter=0;                                //WiFi������������������                        
        memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);          //���WiFi���ջ����� 
        MQTT_Buff_Init();
        if(WiFi_Connect_IoTServer()==0){   			     //���WiFi�����Ʒ�������������0����ʾ��ȷ������if
            u1_printf("����TCP���ӳɹ�\r\n");            //���������Ϣ
            Connect_flag = 1;                            //Connect_flag��1����ʾ���ӳɹ�	
            WiFi_RxCounter=0;                            //WiFi������������������                        
            memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);      //���WiFi���ջ����� 
            MQTT_Buff_ReInit();                          //���³�ʼ�����ͻ�����                    
        }	       
    }else
    {
        WIFI_AP_CONNECT();//ap������������
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
                    WiFi_RxCounter=0;                                //WiFi������������������                        
                    memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);          //���WiFi���ջ�����
                    Wifi_send_str((const char *)temp_buf[0]);
                    Wifi_send_str((const char *)temp_buf[1]);
                    return 1;
                }else
                {
                    WiFi_RxCounter=0;                                //WiFi������������������                        
                    memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);          //���WiFi���ջ����� 
                }                   
            }             
        }         
    }
    return 0;
}
/*-------------------------------------------------------------*/
/*                     �����ͻ���������                      */
/*-------------------------------------------------------------*/
void mqtt_txbuf_process(void) {
    if (MQTT_TxDataOutPtr != MQTT_TxDataInPtr) { //if�����Ļ���˵�����ͻ�������������
        //3������ɽ���if
        //��1�֣�0x10 ���ӱ���
        //��2�֣�0x82 ���ı��ģ���ConnectPack_flag��λ����ʾ���ӱ��ĳɹ�
        //��3�֣�SubcribePack_flag��λ��˵�����ӺͶ��ľ��ɹ����������Ŀɷ�
        if ((MQTT_TxDataOutPtr[2] == 0x10) || ((MQTT_TxDataOutPtr[2] == 0x82) && (ConnectPack_flag == 1)) || (SubcribePack_flag == 1)) {
            u1_printf("��������:0x%x\r\n", MQTT_TxDataOutPtr[2]); //������ʾ��Ϣ
            MQTT_TxData(MQTT_TxDataOutPtr);                       //��������
            MQTT_TxDataOutPtr += BUFF_UNIT;                       //ָ������
            if (MQTT_TxDataOutPtr == MQTT_TxDataEndPtr)           //���ָ�뵽������β����
                MQTT_TxDataOutPtr = MQTT_TxDataBuf[0];            //ָ���λ����������ͷ
        } 				
    }//�����ͻ��������ݵ�else if��֧��β
}

/*-------------------------------------------------------------*/
/*                     ������ջ���������                      */
/*-------------------------------------------------------------*/
void mqtt_rxbuf_process(void) {
    if(MQTT_RxDataOutPtr != MQTT_RxDataInPtr){  //if�����Ļ���˵�����ջ�������������														
        u1_printf("���յ�����:");
        /*-----------------------------------------------------*/
        /*                    ����CONNACK����                  */
        /*-----------------------------------------------------*/				
        //if�жϣ������һ���ֽ���0x20����ʾ�յ�����CONNACK����
        //��������Ҫ�жϵ�4���ֽڣ�����CONNECT�����Ƿ�ɹ�
        if(MQTT_RxDataOutPtr[2]==0x20){             			
            switch(MQTT_RxDataOutPtr[5]){					
                case 0x00 : u1_printf("CONNECT���ĳɹ�\r\n");                            //���������Ϣ	
                            ConnectPack_flag = 1;                                        //CONNECT���ĳɹ������ı��Ŀɷ�
                            break;                                                       //������֧case 0x00                                              
                case 0x01 : u1_printf("�����Ѿܾ�����֧�ֵ�Э��汾��׼������\r\n");     //���������Ϣ
                            Connect_flag = 0;                                            //Connect_flag���㣬��������
                            break;                                                       //������֧case 0x01   
                case 0x02 : u1_printf("�����Ѿܾ������ϸ�Ŀͻ��˱�ʶ����׼������\r\n"); //���������Ϣ
                            Connect_flag = 0;                                            //Connect_flag���㣬��������
                            break;                                                       //������֧case 0x02 
                case 0x03 : u1_printf("�����Ѿܾ�������˲����ã�׼������\r\n");         //���������Ϣ
                            Connect_flag = 0;                                            //Connect_flag���㣬��������
                            break;                                                       //������֧case 0x03
                case 0x04 : u1_printf("�����Ѿܾ�����Ч���û��������룬׼������\r\n");   //���������Ϣ
                            Connect_flag = 0;                                            //Connect_flag���㣬��������						
                            break;                                                       //������֧case 0x04
                case 0x05 : u1_printf("�����Ѿܾ���δ��Ȩ��׼������\r\n");               //���������Ϣ
                            Connect_flag = 0;                                            //Connect_flag���㣬��������						
                            break;                                                       //������֧case 0x05 		
                default   : u1_printf("�����Ѿܾ���δ֪״̬��׼������\r\n");             //���������Ϣ 
                            Connect_flag = 0;                                            //Connect_flag���㣬��������					
                            break;                                                       //������֧case default 								
            }				
        }			
        //if�жϣ���һ���ֽ���0x90����ʾ�յ�����SUBACK����
        //��������Ҫ�ж϶��Ļظ��������ǲ��ǳɹ�
        else if(MQTT_RxDataOutPtr[2]==0x90){ 
                switch(MQTT_RxDataOutPtr[6]){					
                case 0x00 :
                case 0x01 : u1_printf("���ĳɹ�\r\n");            //���������Ϣ
                            SubcribePack_flag = 1;                //SubcribePack_flag��1����ʾ���ı��ĳɹ����������Ŀɷ���
                            Ping_flag = 0;                        //Ping_flag����
                            TIM3_ENABLE_30S();                    //����30s��PING��ʱ��
                            TIM2_ENABLE_30S();                    //����30s���ϴ����ݵĶ�ʱ��
                            TempHumi_State();                     //�ȷ�һ������
                            break;                                //������֧                                             
                default   : u1_printf("����ʧ�ܣ�׼������\r\n");  //���������Ϣ 
                            Connect_flag = 0;                     //Connect_flag���㣬��������
                            break;                                //������֧ 								
            }					
        }
        //if�жϣ���һ���ֽ���0xD0����ʾ�յ�����PINGRESP����
        else if(MQTT_RxDataOutPtr[2]==0xD0){ 
            u1_printf("PING���Ļظ�\r\n"); 		  //���������Ϣ 
            if(Ping_flag==1){                     //���Ping_flag=1����ʾ��һ�η���
                    Ping_flag = 0;    				  //Ҫ���Ping_flag��־
            }else if(Ping_flag>1){ 				  //���Ping_flag>1����ʾ�Ƕ�η����ˣ�������2s����Ŀ��ٷ���
                Ping_flag = 0;     				  //Ҫ���Ping_flag��־
                TIM3_ENABLE_30S(); 				  //PING��ʱ���ػ�30s��ʱ��
            }				
        }	
        //if�жϣ������һ���ֽ���0x30����ʾ�յ����Ƿ�������������������
        //����Ҫ��ȡ��������
        else if((MQTT_RxDataOutPtr[2]==0x30)){ 
            u1_printf("�������ȼ�0����\r\n"); 		   //���������Ϣ
            u1_printf(MQTT_RxDataOutPtr);
            u1_printf("\r\n"); 		   //���������Ϣ
            MQTT_DealPushdata_Qs0(MQTT_RxDataOutPtr);  //����ȼ�0��������
        }				
                        
        MQTT_RxDataOutPtr += BUFF_UNIT;                     //ָ������
        if(MQTT_RxDataOutPtr==MQTT_RxDataEndPtr)            //���ָ�뵽������β����
            MQTT_RxDataOutPtr = MQTT_RxDataBuf[0];          //ָ���λ����������ͷ                        
    }//������ջ��������ݵ�else if��֧��β
}
/*-------------------------------------------------------------*/
/*                     ���������������                      */
/*-------------------------------------------------------------*/
void mqtt_cmdbuf_process(void){  
    if(MQTT_CMDOutPtr != MQTT_CMDInPtr){                             //if�����Ļ���˵�����������������			       
        u1_printf("����:%s\r\n",&MQTT_CMDOutPtr[2]);                 //���������Ϣ
        mqtt_CMD_analyzing(&MQTT_CMDOutPtr[2]);
        MQTT_CMDOutPtr += BUFF_UNIT;                             	 //ָ������
        if(MQTT_CMDOutPtr==MQTT_CMDEndPtr)           	             //���ָ�뵽������β����
            MQTT_CMDOutPtr = MQTT_CMDBuf[0];          	             //ָ���λ����������ͷ				
    }//��������������ݵ�else if��֧��β	
}

void setPost_JSON_str() {
    cJSON * json =  cJSON_CreateObject();//�������ݰ�
    cJSON * item =  cJSON_CreateObject();//�������ݰ���������
    char *temp;
	
    cJSON_AddItemToObject(json, "method", cJSON_CreateString("thing.event.property.post"));//�������
    cJSON_AddItemToObject(json, "id", cJSON_CreateString("203302322"));//�������
    cJSON_AddItemToObject(json, "params", item);//�������
    cJSON_AddItemToObject(item, "temperature", cJSON_CreateNumber(10.000000));//�������
    cJSON_AddItemToObject(item, "humdity0", cJSON_CreateNumber(40.000000));//�������
    cJSON_AddItemToObject(json, "version", cJSON_CreateString("1.0.0"));//�������
    
    temp = cJSON_Print(json);
    
    MQTT_PublishQs0(P_TOPIC_NAME, temp, strlen(temp));

    cJSON_Delete(json);//�ͷ��ڴ�
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
    json = cJSON_Parse(cmd_json); //������json��ʽ
    json_reset_connect = cJSON_GetObjectItem(json, "reset_conncet");
    json_setPumpStatus      = cJSON_GetObjectItem( json , "setPumpStatus");
    json_setPumpTim      = cJSON_GetObjectItem( json , "setPumpTim");
    json_humidity_threshold = cJSON_GetObjectItem( json , "humidity_threshold");
    if (json_reset_connect != NULL) {
        Connect_flag = 0;
        STMFLASH_Write(WIFI_CONNECT_FLAG_ADDR, (u16*)"EE", sizeof(u16)*4);
        WiFi_RxCounter=0;                                //WiFi������������������                        
        memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);          //���WiFi���ջ�����
        SYS_RST();
    }
    if (json_setPumpStatus != NULL) {
        temp_str = json_setPumpStatus->valuestring;
        u1_printf(temp_str);
        for (i = 0; i < 8; i++) {
            if (temp_str[i] == '0') {
                u1_printf("i:%d ��", i);
                PUMP(i) = 0;
                PAout(8) = 0;
            }
            else if (temp_str[i] == '1') {
                u1_printf("i:%d ��", i);
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
    cJSON_Delete(json);  //�ͷ��ڴ� 
    cJSON_Delete(json_reset_connect);  //�ͷ��ڴ�
    cJSON_Delete(json_setPumpStatus);  //�ͷ��ڴ� 
    cJSON_Delete(json_humidity_threshold);  //�ͷ��ڴ� 
    cJSON_Delete(json_setPumpTim);  //�ͷ��ڴ� 
    WiFi_RxCounter=0;                                //WiFi������������������                        
    memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);          //���WiFi���ջ����� 
}

void pumpSetTim(char *str) {
    u8 ch[50];
    u8 index = 0;
    u8 i;
    u8 j = 0;
    
    index = str[0] - '0';
    PUMP_SET_AlarmTim[index][17] = 1;//��ʾ��Ŀ�걻��ʼ������
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
    u1_printf("���:%d-%d-%d--------%d\r\n", PUMP_SET_AlarmTim[index][12],PUMP_SET_AlarmTim[index][13],PUMP_SET_AlarmTim[index][14],PUMP_SET_AlarmTim_interval[index][0]);
    u1_printf("alarm:%d-%d\r\n", PUMP_SET_AlarmTim[index][15],PUMP_SET_AlarmTim[index][16]);
    RTC_Set(PUMP_SET_AlarmTim[index][0] + 2000, PUMP_SET_AlarmTim[index][1], PUMP_SET_AlarmTim[index][2], PUMP_SET_AlarmTim[index][3], PUMP_SET_AlarmTim[index][4], PUMP_SET_AlarmTim[index][5]);
}
