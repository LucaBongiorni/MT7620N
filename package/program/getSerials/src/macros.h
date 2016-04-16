//=============================================================================
// FILE: Macros.h
// DESC:
//=============================================================================

#ifndef __MACROS_H__
#define __MACROS_H__
typedef unsigned char u8;
typedef char s8;
typedef unsigned short u16;
typedef short s16 ;
//typedef unsigned long u32;
//typedef long s32;
typedef unsigned int u32;
typedef int s32;

//sys

//litte-endian
#define LITTE_ENDIAN




//定义发送到接收的等待最大时间0.5s REV_WAITE_CNT*WATIE_CYC
#define REV_WAIT_CNT 500
//串口中有数据时到结束的等待时间us
//经测试1000可以，但短了一点，这对扫描数据
//复位，不知道为什么要0.5S
#define REV_WAIT_FINISH 10000

//串口循环等待时间us
#define WATI_CYC 1000

//定义接收帧中读取串口的数据的延时us 
#define FRAME_WAIT 10000 
//定义每次读取最大长度
#define READ_DATA_LEN 200 
#define HEX_DATA_LEN ((READ_DATA_LEN)/2)
//定义一帧最大读取次数 字节长度=READ_DATA_LEN*FRAME_READ_CNT
#define FRAME_READ_CNT 5 
//命令宏定义
#define CONNON_ACK_CMD  0x0101
#define RST_CLOUD_BCN_CMD 0x0102
#define SCAN_BLE_CMD 0x0103
#define STOP_BLE_CMD 0x0104
#define CONNECT_BLE_CMD 0x0106
#define DISCONNECT_BLE_CMD 0x0107
#define CONFIG_BLE_CMD 0x0108
#define READ_CONFIG_BLE_CMD 0x0109
#define ANSWER_SCAN_CMD 0x0105
#define GET_MARK_BLE_CB_CMD 0x010b
#define ACK_MARK_BLE_CB_CMD 0x010c

//扫描数据结构体相关宏定义
//名字字符长度
#define LEN_ADR_BLE 6	

//return error
#define ERR_SYS_BUSY  1
#define ERR_CHECK  2
#define ERR_OTHER    255



//调试宏
//#define TTY_DEBUG 
//#ifdef SERIAL_PORT_OWN_DEBUG



#ifndef IFNAMESIZ
#define IFNAMESIZ   16
#endif

#define IFCON_NAME "br-lan" 










//=============================================================================
#endif /* __BEACON_H__ */
