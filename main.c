#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


typedef unsigned char uint8_t;
typedef unsigned long uint32_t;
typedef unsigned short uint16_t;

typedef long int32_t;
typedef short int16_t;

//定义一个与数据库中站点表匹配的结构体
typedef struct module_info
{
	char comm_id[15];				//通讯模块的设备SN：“20100101+01+xxxx"
	char hydro_id[15];			
	char pressure_id[15];
	char sensor1_id[15];		//对应盒子上水流量ADC1_9
	char sensor2_id[15];		//ADC1_10

	char sensor3_id[15];
	char sensor4_id[15];
	//char sensor1_id[15];
} WT_Module_Info;

//定义一个与数据库中站点表匹配的结构体
typedef struct st_device_info
{
	char comm_id[15];				//通讯模块的设备SN：“20100101+01+xxxx"
	char hydro_id[15];			
	char pressure_id[15];
	char sensor1_id[15];		//对应盒子上水流量ADC1_9
	char sensor2_id[15];		//ADC1_10
	char sensor3_id[15];		//ADC1_15
	char sensor4_id[15];		//ADC1_11
}WT_Device_Info;


//设备的采样设置，以及发送时间设置等等
typedef struct st_sample_setting
{
	uint8_t HYDRO_sample_rate;			//采样水听器采样率
	uint8_t HYDRO_sample_channels;		//采样水听器采样通道
	uint8_t	hydro_sample_length;		//采样时长
	uint8_t	hydro_sample_interval;		//采样间隔(分钟)
	int32_t	hydro_gain;					//增益
	
	uint8_t pressure_sample_interval;	//采样间隔ms
	uint8_t sensors_sample_interval;	//sensor2-4的采样设置
	uint8_t sensors_send_interval;		//发动数据的间隔时间
	uint8_t	rtc_sysnc_hour;				//RTC同步的时间点（小时点）
} WT_Device_Setting;

//通讯服务器ip和port
typedef struct comm_server {
	char ip[15];			//IP address
	uint16_t lte_Port;	    //端口
} WT_Server_Setting;

//站点的信息，设置等总结构
typedef struct st_station_info {
	uint32_t stn_id;			//站点ID;这个ID对应数据库中的stn_id。
	char stn_name[40];			//数据库中的站点名称
	uint8_t stm32_id[12];		//STM32L4 MCU ID
	//经纬度信息
	uint16_t jinDu;
	uint16_t jinFen;
	uint16_t weiDu;
	uint16_t weiFen;
	//设备信息
	WT_Device_Info dev;
	//站点设置
	WT_Device_Setting setting;
	uint8_t netNumber[12];	        //站点的通讯号码
	WT_Server_Setting server[2];	//两个服务器设置
} WT_Station_Setting;

WT_Station_Setting stn_information;

enum UART_CMD_TYPE { 
	CMD_NONE, CMD_SYSINFO, CMD_GPS, 
	CMD_LTE, CMD_SENSOR, CMD_HYDRO, 
	CMD_TEST
};

enum UART_OPT_TYPE {
	OPT_NONE, OPT_READ, OPT_WRITE, OPT_CMD 
};

typedef struct {
    uint8_t key_idx;
    char *key_Str;
} value_string;

//消息关键字
const value_string vals_message_type[] = {
	/* STN INFO*/
	{ 1,    "STN_ID" },
	{ 2,    "STN_NAME" },
	{ 3,    "STM_ID" },
	{ 4,	"JIN_DU" },
	{ 5,	"JIN_FEN" },
	{ 6,	"WEI_DU" },
	{ 7,	"WEI_FEN" },
	//dev info
	{ 10,	"COMM_ID" },
	{ 11,	"HD_ID" },
	{ 12,	"PRE_ID" },
	{ 13,	"S2_ID" },
	{ 14,	"S3_ID" },
	{ 15,	"S4_ID" },
	//dev sample setting
	{ 20,	"HD_SAMP" },
	{ 21,	"PRE_SAMP" },
	{ 22,	"SENS_SAMP" },
	// { 23,	"SEN3_SAMP" },
	// { 24,	"SEN4_SAMP" },
	//dev sending setting
	{ 30, 	"HD_SEND" },
	{ 31,	"DATA_SEND" },
    { 32,   "RTC_SYNC"},
	//HD setting
	{ 40,	"HD_GAIN" },
	{ 41,	"SAMP_RATE" },
	{ 42,	"SAMP_LEN" },
	{ 43,	"SEN4_SAMP" },
		
	//LTE
	{ 50, "LTE_CSQ" },
		{51, "LTE_SIM" },
		{52, "COMM0_IP"},
		{53, "COMM0_PT"},
		{54, "COMM_TYPE" },
		{55, "ACCESS_MODE" },
		{56, "COMM_BAUD" },
		{57, "COMM0_IP"},
		{58, "COMM0_PT"},

	//OTHERS
		{60, "GPS_VER" },
		{61, "xxxxx" },
    { 0x00, NULL },
};

//命令行键值
const value_string cmd_message_type[] = {
	{1, "up"},
	{2, "reset"},
	{3, "down"},
	{4, "sleep"},
	{5, "close"},
	{6, "+++"},
	{7, "init"},
	{8, "baud"},
    { 0x00, NULL },

};

uint8_t parse_utility_cmd(char * cmdstring, WT_Station_Setting * stn_info);
uint8_t handle_sysinfo_cmd(char * cmdstring, uint8_t optType);

//实验数据
char * sys_info_test = "CMD01:w=STN_ID:1001,STN_NAME:stm_name_001,COMM_ID:20201222000001,STM_ID:123456789009,JIN_DU:100,JIN_FEN:2222,WEI_DU:33,WEI_FEN:4444,HD_ID:20201010001112,HD_SAMP:30,PRE_ID:20201212010001,PRE_SAMP:25,S2_ID:20201010001000,S3_ID:20201010111112,S4_ID:20201010001234,SENS_SAMP:1234,HD_SEND:30,DATA_SEND:5,COMM0_IP:201.212.256.231,COMM0_PT:8005,COMM0_IP:201.212.256.111,COMM0_PT:8115,RTC_SYNC:8\r\n";

char * cmd_info_test = "CMD02:c=reset\r\n";
//处理system info的数据请求，主要将站点基本信息，装配信息，配置信息返回给
//system info的请求包含了大量的key和value，这个处理上有些繁琐
//
uint8_t handle_sysinfo_cmd(char * cmdstring, uint8_t optType)
{
	return 0;
}

//从事先定义的命令串/字段串中匹配，返回index
uint8_t match_keyString(char * matchStr, value_string * list){
	uint8_t len = strlen(matchStr);
	value_string * ptr = list;
	while(ptr->key_idx != 0x0){
		char * strMatch = ptr->key_Str;
		if(strstr(matchStr, strMatch)) {
			return ptr->key_idx;
		}
        ptr++;
    }
	return 0;
	
}

/****************************************************************************
//将数据字段写入结构体中
//key:val
****************************************************************************/
uint8_t update_sysinfo(WT_Station_Setting * stn_info, uint8_t keyIndex, char * value){
    char * matched = strchr(value, ':');
    if(matched == NULL)
        return 0;
    matched ++;
    switch (keyIndex) {
	    /* STN INFO*/
        case 1: //"STN_ID"
            stn_info->stn_id = atol(matched);
            printf("stn_info->stn_id = %d\r\n", stn_info->stn_id);
            break;
        case 2: //"STN_NAME"
            strcpy(stn_info->stn_name, matched);
            printf("stn_info->stn_name = %s\r\n", stn_info->stn_name);
            break;
        case 3: //"STM_ID"
            strcpy(stn_info->stm32_id, matched);
            printf("stn_info->stm32_id = %x-%x-%x-%x-%x-%x-%x-%x-%x\r\n", stn_info->stm32_id[0],
                    stn_info->stm32_id[1], stn_info->stm32_id[2], stn_info->stm32_id[3], 
                    stn_info->stm32_id[4], stn_info->stm32_id[5], stn_info->stm32_id[6], 
                    stn_info->stm32_id[7], stn_info->stm32_id[8], stn_info->stm32_id[9]);
            break;
        case 4:    //"JIN_DU" 
            stn_info->jinDu = atol(matched);
            printf("stn_info->jinDu = (%d)\r\n", stn_info->jinDu);
            break;
        case 5:    //"JIN_FEN" 
            stn_info->jinFen = atol(matched);
            printf("stn_info->jinFen = (%d)\r\n", stn_info->jinFen);
            break;
        case 6:    //"WEI_DU"
            stn_info->weiDu = atol(matched);
            printf("stn_info->weiDu = (%d)\r\n", stn_info->weiDu);
            break;
        case 7:    //"WEI_FEN"
            stn_info->weiFen = atol(matched);
            printf("stn_info->weiFen = (%d)\r\n", stn_info->weiFen);
            break;
	//dev info
        case 10:    //"COMM_ID"
            memset(stn_info->dev.comm_id, 0, sizeof(stn_info->dev.comm_id));
            strcpy(stn_info->dev.comm_id , matched);
            printf("stn_info->weiFen = (%d)\r\n", stn_info->weiFen);
            break;
        case 11:    //"HD_ID"
            memset(stn_info->dev.hydro_id, 0, sizeof(stn_info->dev.hydro_id));
            strcpy(stn_info->dev.hydro_id , matched);
            printf("stn_info->dev.hydro_id = %s\r\n", stn_info->dev.hydro_id);
            break;
        case 12:    //"PRE_ID"
            memset(stn_info->dev.pressure_id, 0, sizeof(stn_info->dev.pressure_id));
            strcpy(stn_info->dev.pressure_id , matched);
            printf("stn_info->dev.pressure_id = %s\r\n", stn_info->dev.pressure_id);
            break;
        case 13:    //"S2_ID"
            memset(stn_info->dev.sensor2_id, 0, sizeof(stn_info->dev.sensor2_id));
            strcpy(stn_info->dev.sensor2_id , matched);
            printf("stn_info->dev.sensor2_id = %s\r\n", stn_info->dev.sensor2_id);
            break;
        case 14:    //"S3_ID"
            memset(stn_info->dev.sensor3_id, 0, sizeof(stn_info->dev.sensor3_id));
            strcpy(stn_info->dev.sensor3_id , matched);
            printf("stn_info->dev.sensor3_id = %s\r\n", stn_info->dev.sensor3_id);
            break;
        case 15:    //"S4_ID"
            memset(stn_info->dev.sensor4_id, 0, sizeof(stn_info->dev.sensor4_id));
            strcpy(stn_info->dev.sensor4_id , matched);
            printf("stn_info->dev.sensor4_id = %s\r\n", stn_info->dev.sensor4_id);
            break;
        //dev sample setting
        case 20:    //"HD_SAMP"
            stn_info->setting.hydro_sample_interval = atol(matched);
            printf("stn_info->setting.hydro_sample_interval = %d\r\n", stn_info->setting.hydro_sample_interval);
            break;
        case 21:    //"PRE_SAMP"
            stn_info->setting.pressure_sample_interval = atol(matched);
            printf("stn_info->setting.pressure_sample_interval = %d\r\n", stn_info->setting.pressure_sample_interval);
            break;
        case 22:    //"SEN2_SAMP"
            stn_info->setting.sensors_sample_interval = atol(matched);
            printf("stn_info->setting.sensors_sample_interval = %d\r\n", stn_info->setting.sensors_sample_interval);
            break;
        // case 23:    //"SEN3_SAMP"
        //     stn_info->setting.sensors_sample_interval = atol(matched);
        //     break;
        // case 24:    //"SEN4_SAMP"
        //     stn_info->setting.sensors_sample_interval = atol(matched);
        //     break;
        //dev sending setting
        // case 30:    //"HD_SEND"
        //     stn_info->setting.sensors_send_interval = atol(matched);
        //     printf("stn_info->setting.sensors_send_interval = %x(%d)\r\n", stn_info->setting.sensors_send_interval);
        //     break;
        case 31:    //"DATA_SEND"
            stn_info->setting.sensors_send_interval = atol(matched);
            printf("stn_info->setting.sensors_send_interval = %d\r\n", stn_info->setting.sensors_send_interval);
            break;
        case 32:
            stn_info->setting.rtc_sysnc_hour  = atol(matched);
            printf("stn_info->setting.rtc_sysnc_hour = %d\r\n", stn_info->setting.rtc_sysnc_hour);
            break;
    	//HD setting
        case 40:    //"HD_GAIN"
            stn_info->setting.hydro_gain = atol(matched);
            printf("stn_info->setting.hydro_gain = %d\r\n", stn_info->setting.hydro_gain);
            break;
        case 41:    //"SAMP_RATE"
            stn_info->setting.HYDRO_sample_rate = atol(matched);
            printf("stn_info->setting.HYDRO_sample_rate = %d\r\n", stn_info->setting.HYDRO_sample_rate);
            break;
        case 42:    //"SAMP_LEN"
            stn_info->setting.hydro_sample_length = atol(matched);
            printf("stn_info->setting.hydro_sample_length = %d\r\n", stn_info->setting.hydro_sample_length);
            break;
        // case 43:    //"SEN4_SAMP" },
        //     stn_info->jinFen = atol(matched);
	    //     break;
        //LTE
        case 50: //"LTE_CSQ"
            break;
        case 51: //"LTE_SIM"
            break;
        case 52: //"COMM0_IP"
            memset(stn_info->server[0].ip, 0, sizeof(stn_info->server[0].ip));
            strcpy(stn_info->server[0].ip , matched);
            printf("stn_info->server[0].ip = %s\r\n", stn_info->server[0].ip);
            break;
        case 53:    //"COMM0_PT"
            stn_info->server[0].lte_Port = atol(matched);
            printf("stn_info->server[0].lte_Port = %d\r\n", stn_info->server[0].lte_Port);
            break;
        case 57: //"COMM1_IP"
            memset(stn_info->server[1].ip, 0, sizeof(stn_info->server[1].ip));
            strcpy(stn_info->server[1].ip , matched);
            printf("stn_info->server[1].ip = %s\r\n", stn_info->server[1].ip);
            break;
        case 58:    //"COMM1_PT"
            stn_info->server[1].lte_Port = atol(matched);
            printf("stn_info->server[1].lte_Port = %d\r\n", stn_info->server[1].lte_Port);
            break;
        case 54:    //"COMM_TYPE"
            break;
        case 55:    //"ACCESS_MODE"
            break;
        case 56:    //"COMM_BAUD" },
            break;
	//OTHERS
        case 60:    //"GPS_VER"
            break;
        case 61:    //"xxxxx"
            break;
    }    
    return 0;
}

//解析工具发送过来的操作命令;一般只有一个命令串，所以不用循环
uint8_t parser_cmd_string(char * sysStr, WT_Station_Setting * stn_info){
    char keyVal[100];
    char * ptr = sysStr;
    char * post = sysStr;
    uint8_t idx = 0;
    //截取分隔符
    char * pMatched = strchr(ptr, '\r');
    if(pMatched){
        post = pMatched + 1;
        memset(keyVal, 0, sizeof(keyVal));
        memcpy(keyVal,  ptr, post - ptr - 1);
        idx = match_keyString(keyVal, (char *)cmd_message_type);
        printf("Get sub string:[%s]  idx:%d\r\n", keyVal, idx);
    }
    return 0;
}


//用于解析好有信息字段的
uint8_t parser_sysinfo_string(char * sysStr, WT_Station_Setting * stn_info){
    char keyVal[100];
    char * ptr = sysStr;
    char * post = sysStr;
    uint8_t idx = 0;
    //截取分隔符
    char * pMatched = strchr(ptr, ',');
    if(pMatched == NULL)
        //判断结束符'\r\n'
        pMatched = strchr(ptr, '\r');
    while(pMatched){
        post = pMatched + 1;
        memset(keyVal, 0, sizeof(keyVal));
        memcpy(keyVal,  ptr, post - ptr - 1);
        idx = match_keyString(keyVal, (char *)vals_message_type);
        printf("Get sub string:[%s]  idx:%d\r\n", keyVal, idx);
        if(idx != 0){
            update_sysinfo(stn_info, idx, keyVal);
        }
        //当前子串处理结束
        ptr = post;
        //判断间隔符','
        pMatched = strchr(ptr, ',');
        if(pMatched == NULL)
            //判断结束符'\r\n'
            pMatched = strchr(ptr, '\r');
        
    }
    return 0;
}

//cmd format: CMDxx:(r/w)[=[key1]:[val1],[key2]:[val2],[keyx]:[valx];]\r\n
uint8_t parse_utility_cmd(char * cmdstring, WT_Station_Setting * stn_info) {
    uint8_t cmdType = CMD_NONE;
	uint8_t optType = OPT_NONE;
	char *ptr = cmdstring;
	uint8_t len = 0;

	len = strlen(cmdstring);
	//命令头：CMDxx:r/w
	if ( len <= 7)
		return CMD_NONE;
	//判断指令开头
	if( ptr[0] != 'C' || ptr[1] != 'M' || ptr[2] != 'D')
		return CMD_NONE;
	//判断指令号00
	ptr = ptr + 3;
	if( (ptr[0] >= '0' && ptr[0] <= '9')
			&& (ptr[1] >= '0' && ptr[1] <= '9')) {
				cmdType = (ptr[0] - '0') * 10 + (ptr[1] - '0'); 
	}else 
		return CMD_NONE;
    printf("%s\r\n", ptr);	
	//move pointer to command content
	//判断命令格式":"
	if(ptr[2] != ':')
		return CMD_NONE;
	else
		ptr = ptr + 3;
	
    printf("%s\r\n", ptr);	
	//check r/w bit
	if(ptr[0] == 'r')
		optType = OPT_READ;
	else if(ptr[0] == 'w')
		optType = OPT_WRITE;
	else if(ptr[0] == 'c')
		optType = OPT_CMD;
	else
		return CMD_NONE;
	
    if(OPT_READ != optType)
        ptr = ptr + 2;  //move next 2 char "w/c="
    //分析命令行后面的参数
    switch (optType) {
        case OPT_CMD:       //要执行命令
            parser_sysinfo_string(ptr, stn_info);
            break;
        case OPT_WRITE:     //对于终端write操作是要更新结构体的
            parser_sysinfo_string(ptr, stn_info);
            break;
        case OPT_READ:
            break;
    }
	switch (cmdType) {
		case CMD_SYSINFO:{
			break;
		}
		case CMD_GPS:{
			break;
		}
		case CMD_LTE:{
			break;
		}
		case CMD_SENSOR:{
			break;
		}
		case CMD_HYDRO:{
			break;
		}
		case CMD_TEST: {
			break;
		}
	}						
}

/******************************************************************************
 * testing code 
 * ***************************************************************************/
void test_parser(void){
    // printf("%s\r\n", sys_info_test);
    // parse_utility_cmd(sys_info_test);
}


/*****************************************************************************/
// void wt_read_setting(WT_Station_Setting * stn_info){
// 	WT_Station_Setting *ptr = (WT_Station_Setting *)g_setting_address;
// 	memcpy(stn_info, ptr, sizeof(WT_Station_Setting));
// 	return;

// }

void init_setting1(WT_Station_Setting * stn_info){
    stn_info->stn_id =  12;
    strcpy(stn_info->stn_name, "STN_0001");
    strcpy(stn_info->stm32_id, "1234567890");
    stn_info->jinDu = 100;
    stn_info->jinFen = 110;
    stn_info->weiDu = 120;
    stn_info->weiFen = 140;
    memset(stn_info->dev.comm_id, 0, sizeof(stn_info->dev.comm_id));
    strcpy(stn_info->dev.comm_id , "2020121200001");
    memset(stn_info->dev.hydro_id, 0, sizeof(stn_info->dev.hydro_id));
    strcpy(stn_info->dev.hydro_id , "2020121209001");
    memset(stn_info->dev.pressure_id, 0, sizeof(stn_info->dev.pressure_id));
    strcpy(stn_info->dev.pressure_id , "2020121201001");
    memset(stn_info->dev.sensor2_id, 0, sizeof(stn_info->dev.sensor2_id));
    strcpy(stn_info->dev.sensor2_id , "2020121202001");
    memset(stn_info->dev.sensor3_id, 0, sizeof(stn_info->dev.sensor3_id));
    strcpy(stn_info->dev.sensor3_id , "2020121203001");
    memset(stn_info->dev.sensor4_id, 0, sizeof(stn_info->dev.sensor4_id));
    strcpy(stn_info->dev.sensor4_id , "2020121205001");
    stn_info->setting.hydro_sample_interval = 30;
    stn_info->setting.pressure_sample_interval = 25;
    stn_info->setting.sensors_sample_interval = 1;
    stn_info->setting.sensors_send_interval = 5;
    stn_info->setting.hydro_gain = -10;
    stn_info->setting.HYDRO_sample_rate = 8000;
    stn_info->setting.hydro_sample_length = 10;
}

void init_setting2(WT_Station_Setting * stn_info){
    stn_info->stn_id =  11;
    strcpy(stn_info->stn_name, "STN_0xxx");
    strcpy(stn_info->stm32_id, "AAAAAAAAA");
    stn_info->jinDu = 100;
    stn_info->jinFen = 110;
    stn_info->weiDu = 120;
    stn_info->weiFen = 140;
    memset(stn_info->dev.comm_id, 0, sizeof(stn_info->dev.comm_id));
    strcpy(stn_info->dev.comm_id , "2020121100001");
    memset(stn_info->dev.hydro_id, 0, sizeof(stn_info->dev.hydro_id));
    strcpy(stn_info->dev.hydro_id , "2020120109001");
    memset(stn_info->dev.pressure_id, 0, sizeof(stn_info->dev.pressure_id));
    strcpy(stn_info->dev.pressure_id , "2020122201001");
    memset(stn_info->dev.sensor2_id, 0, sizeof(stn_info->dev.sensor2_id));
    strcpy(stn_info->dev.sensor2_id , "2021121202001");
    memset(stn_info->dev.sensor3_id, 0, sizeof(stn_info->dev.sensor3_id));
    strcpy(stn_info->dev.sensor3_id , "2022121203001");
    memset(stn_info->dev.sensor4_id, 0, sizeof(stn_info->dev.sensor4_id));
    strcpy(stn_info->dev.sensor4_id , "2023121205001");
    stn_info->setting.hydro_sample_interval = 30;
    stn_info->setting.pressure_sample_interval = 25;
    stn_info->setting.sensors_sample_interval = 1;
    stn_info->setting.sensors_send_interval = 5;
    stn_info->setting.hydro_gain = -10;
    stn_info->setting.HYDRO_sample_rate = 8000;
    stn_info->setting.hydro_sample_length = 10;
}

WT_Station_Setting setting1;
WT_Station_Setting setting2;
WT_Station_Setting setting3;

void save_setting(WT_Station_Setting * dest, WT_Station_Setting * src)
{
    memcpy(dest, src, sizeof(WT_Station_Setting));

    return;
}

void print_info(WT_Station_Setting *stn_info){
    printf("======================================================================================\r\n");
    printf("stn_info->stn_id =      %d\r\n", stn_info->stn_id);
    printf("stn_info->stn_name =    %s\r\n", stn_info->stn_name);
    printf("stn_info->stm32_id =    %x-%x-%x-%x-%x-%x-%x-%x-%x\r\n", stn_info->stm32_id[0],
            stn_info->stm32_id[1], stn_info->stm32_id[2], stn_info->stm32_id[3], 
            stn_info->stm32_id[4], stn_info->stm32_id[5], stn_info->stm32_id[6], 
            stn_info->stm32_id[7], stn_info->stm32_id[8], stn_info->stm32_id[9]);
    printf("stn_info->jinDu =   (%d)\r\n", stn_info->jinDu);
    printf("stn_info->jinFen =  (%d)\r\n", stn_info->jinFen);
    printf("stn_info->weiDu =   (%d)\r\n", stn_info->weiDu);
    printf("stn_info->weiFen =  (%d)\r\n", stn_info->weiFen);
    printf("stn_info->weiFen =  (%d)\r\n", stn_info->weiFen);
    printf("stn_info->dev.hydro_id =    %s\r\n", stn_info->dev.hydro_id);
    printf("stn_info->dev.pressure_id = %s\r\n", stn_info->dev.pressure_id);
    printf("stn_info->dev.sensor2_id =  %s\r\n", stn_info->dev.sensor2_id);
    printf("stn_info->dev.sensor3_id =  %s\r\n", stn_info->dev.sensor3_id);
    printf("stn_info->dev.sensor4_id =  %s\r\n", stn_info->dev.sensor4_id);
    printf("stn_info->setting.hydro_sample_interval =       %d\r\n", stn_info->setting.hydro_sample_interval);
    printf("stn_info->setting.pressure_sample_interval =    %d\r\n", stn_info->setting.pressure_sample_interval);
    printf("stn_info->setting.sensors_sample_interval =     %d\r\n", stn_info->setting.sensors_sample_interval);
    printf("stn_info->setting.sensors_send_interval =       %d\r\n", stn_info->setting.sensors_send_interval);
    printf("stn_info->setting.rtc_sysnc_hour =  %d\r\n", stn_info->setting.rtc_sysnc_hour);
    printf("stn_info->setting.hydro_gain =      %d\r\n", stn_info->setting.hydro_gain);
    printf("stn_info->setting.HYDRO_sample_rate =   %d\r\n", stn_info->setting.HYDRO_sample_rate);
    printf("stn_info->setting.hydro_sample_length = %d\r\n", stn_info->setting.hydro_sample_length);
    printf("stn_info->server[0].ip =        %s\r\n", stn_info->server[0].ip);
    printf("stn_info->server[0].lte_Port =  %d\r\n", stn_info->server[0].lte_Port);
    printf("stn_info->server[1].ip =        %s\r\n", stn_info->server[1].ip);
    printf("stn_info->server[1].lte_Port =  %d\r\n", stn_info->server[1].lte_Port);
    printf("======================================================================================\r\n\r\n\r\n");
}

int main()
{
    //cout << "Hello world!" <<endl;
    init_setting1(&setting1);
    printf("========SETTING1===========\r\n");
    print_info(&setting1);

    init_setting2(&setting2);
    save_setting(&setting3, &setting2);
    printf("========SETTING2===========\r\n");
    print_info(&setting2);
    printf("========SETTING3==========\r\n");
    print_info(&setting3);
    printf("%s\r\n", sys_info_test);
    parse_utility_cmd(sys_info_test, &setting1);    
    print_info(&setting1); 
    save_setting(&setting3, &setting1);
    printf("========SETTING3===========\r\n");
    print_info(&setting3); 
    //test_parser();
    return 0;
}
