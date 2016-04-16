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




//���巢�͵����յĵȴ����ʱ��0.5s REV_WAITE_CNT*WATIE_CYC
#define REV_WAIT_CNT 500
//������������ʱ�������ĵȴ�ʱ��us
//������1000���ԣ�������һ�㣬���ɨ������
//��λ����֪��ΪʲôҪ0.5S
#define REV_WAIT_FINISH 10000

//����ѭ���ȴ�ʱ��us
#define WATI_CYC 1000

//�������֡�ж�ȡ���ڵ����ݵ���ʱus 
#define FRAME_WAIT 10000 
//����ÿ�ζ�ȡ��󳤶�
#define READ_DATA_LEN 200 
#define HEX_DATA_LEN ((READ_DATA_LEN)/2)
//����һ֡����ȡ���� �ֽڳ���=READ_DATA_LEN*FRAME_READ_CNT
#define FRAME_READ_CNT 5 
//����궨��
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

//ɨ�����ݽṹ����غ궨��
//�����ַ�����
#define LEN_ADR_BLE 6	

//return error
#define ERR_SYS_BUSY  1
#define ERR_CHECK  2
#define ERR_OTHER    255



//���Ժ�
//#define TTY_DEBUG 
//#ifdef SERIAL_PORT_OWN_DEBUG



#ifndef IFNAMESIZ
#define IFNAMESIZ   16
#endif

#define IFCON_NAME "br-lan" 










//=============================================================================
#endif /* __BEACON_H__ */
