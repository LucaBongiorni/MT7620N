//=============================================================================
// FILE  : ble_defines.h
// DESC  :
// AUTHOR: PINBO
// CREATE: 2015-06-02
// MODIFY:
//=============================================================================

#ifndef __BLE_DEFINES_H__
#define __BLE_DEFINES_H__

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#define __BLE_DEBUG__

#ifdef  __BLE_DEBUG__
#define BLE_DEBUG(fmt,...)  printf("[%d]"fmt"\n",__LINE__, ##__VA_ARGS__)
#else
#define BLE_DEBUG(fmt,...)
#endif

#ifdef WIN32
#include <windows.h>
#define NOMINMAX
#define SLEEP(msec)         Sleep(msec)
#else
#include <unistd.h>
#include <stdbool.h>
#define SLEEP(msec)         usleep(msec * 1000)
#endif


//=============================================================================
//=============================================================================
#define BLE_MAX_ADDR_LEN                    6
#define BLE_MAX_UUID_LEN                    16
#define BLE_MAX_SKEY_LEN                    16
#define BLE_MAX_EURL_LEN                    18
#define BLE_MAX_NAME_LEN                    20
#define BLE_MAX_DATA_LEN                    20
#define BLE_MAX_SCAN_LEN                    31

#define DFU_MAX_BUFFER_LEN                  20
#define DFU_MAX_PACKET_LEN                  16

#define USR_MAX_BUFFER_LEN                  512
#define USR_MAX_PACKET_LEN                  16

//=============================================================================
//=============================================================================
#define COM_MAX_BUF_SIZE                    512
#define BLE_MAX_MSG_SIZE                    256
#define NET_MAX_MSG_SIZE                    1400

#define MSG_TX_BEG_FLAG                     0x02
#define MSG_TX_END_FLAG                     0x03

//=============================================================================
//=============================================================================
#define BLE_GENERAL_ACK                     0x0101
#define BLE_CENTRAL_RESET                   0x0102
#define BLE_SCANNING_START                  0x0103
#define BLE_SCANNING_REPORT                 0x0104
#define BLE_SCANNING_STOP                   0x0105
#define BLE_BEACON_CONNECT                  0x0106
#define BLE_BEACON_DISCONN                  0x0107
#define BLE_CHARACT_QUERY                   0x0108
#define BLE_CHARACT_REPORT                  0x0109
#define BLE_CHARACT_SETUP                   0x010A
#define BLE_CENTRAL_QUERY                   0x010B
#define BLE_CENTRAL_REPORT                  0x010C
#define BLE_CENTRAL_SETUP                   0x010D
#define BLE_CENTRAL_UPDATE                  0x010E
#define BLE_USRDATA_SETUP                   0x010F  // this --> BLE
#define BLE_USRDATA_REPORT                  0x0110  // this <-- BLE


//=============================================================================
// WIFI Information Structure:
// EN(1)+CH(1)+SEC(1)+ENCRYPT(1)+ssid('\0')+key('\0')
//=============================================================================
#define USR_MSG_SETUP_STA_INFO              0x40    // SEND: SEQ(1)+OPT(1)+MSG(1)+NUM(1)+DATA(0~16), BACK: ACK(1)+OPT(1)+ERR(1)+NUM(1)
#define USR_MSG_QUERY_STA_INFO              0x41    // SEND: SEQ(1)+OPT(1)+MSG(1)+NUM(1),            BACK: ACK(1)+OPT(1)+ERR(1)+NUM(1)+DATA(0~16)
#define USR_MSG_SETUP_AP_INFO               0x42    // SEND: SEQ(1)+OPT(1)+MSG(1)+NUM(1)+DATA(0~16), BACK: ACK(1)+OPT(1)+ERR(1)+NUM(1)
#define USR_MSG_QUERY_AP_INFO               0x43    // SEND: SEQ(1)+OPT(1)+MSG(1)+NUM(1),            BACK: ACK(1)+OPT(1)+ERR(1)+NUM(1)+DATA(0~16)

// Message Options:
//   bit: 7  6  5  4  3  2  1  0
//   val: X  X  X  0  0  0  0  0
//   bit[0:4]: message length
//   bit[7]  : package end
#define USR_MSG_LENGTH_MASK                 0x1F
#define USR_MSG_OPTION_MASK                 0xE0
#define USR_MSG_OPTION_END_PACKAGE          (1<<7)

//=============================================================================
//=============================================================================
#define NET_SCANCFG_QUERY_REQ               0x0101
#define NET_SCANCFG_QUERY_ACK               0x0102
#define NET_SCANNING_START_REQ              0x0107
#define NET_SCANNING_START_ACK              0x0108
#define NET_SCANNING_STOP_REQ               0x0109
#define NET_SCANNING_STOP_ACK               0x010A
#define NET_SCANNING_REPORT                 0x010B
#define NET_BEACON_QUERY_REQ                0x010C
#define NET_BEACON_QUERY_ACK                0x010D
#define NET_BEACON_SETUP_REQ                0x010E
#define NET_BEACON_SETUP_ACK                0x010F
#define NET_CENTRAL_QUERY_REQ               0x0110
#define NET_CENTRAL_QUERY_ACK               0x0111
#define NET_CENTRAL_SETUP_REQ               0x0112
#define NET_CENTRAL_SETUP_ACK               0x0113
#define NET_SCANNING_POST                   0x0114

//=============================================================================
//=============================================================================
#define ACK_RESULT_SUCCEED                  0
#define ACK_RESULT_FAILED                   1
#define ACK_RESULT_ERROR                    2
#define ACK_RESULT_BUSY                     3
#define ACK_RESULT_TIMEOUT                  4
#define ACK_RESULT_UNKNOWN                  5
#define ACK_RESULT_FORBIDDEN                6
#define ACK_RESULT_INVALID_PARAM            7
#define ACK_RESULT_INVALID_STATE            8
#define ACK_RESULT_INVALID_VALUE            9
#define ACK_RESULT_INVALID_LENGTH           10

//=============================================================================
//=============================================================================
char* string_check(char *p_str);

uint8_t  uint16_encode(uint16_t value, uint8_t *p_encoded_data);
uint8_t  uint32_encode(uint32_t value, uint8_t *p_encoded_data);
uint16_t uint16_decode(const uint8_t *p_encoded_data);
uint32_t uint32_decode(const uint8_t *p_encoded_data);

uint8_t  big_uint16_encode(uint16_t value, uint8_t *p_encoded_data);
uint8_t  big_uint32_encode(uint32_t value, uint8_t *p_encoded_data);
uint16_t big_uint16_decode(const uint8_t *p_encoded_data);
uint32_t big_uint32_decode(const uint8_t *p_encoded_data);

uint8_t MsgChecksum(const uint8_t *p_msg, uint8_t msg_len);
uint8_t MsgHex2ASCII(char *p_out, const uint8_t *p_in, uint8_t msg_len);
uint8_t MsgASCII2Hex(uint8_t *p_out, const char *p_in, uint8_t msg_len);

uint8_t Hex2EddystoneURL(char *p_out, const uint8_t *p_in, uint8_t hex_len);
uint8_t EddystoneURL2Hex(uint8_t *p_out, const char *p_in, uint8_t str_len);

//=============================================================================
//=============================================================================
struct BleCommand
{
    uint32_t    ble_seq_no;
    uint16_t    ble_msg_id;
    uint16_t    net_seq_no;
    uint16_t    net_msg_id;
    uint32_t    net_phone;
    uint8_t     net_flags;
    uint16_t    ret_value;
};


extern int CentralNetRead(BleCommand &cmd, char *p_msg, int msg_len);
extern int CentralNetWrite(BleCommand cmd, char *p_msg, int msg_len);
extern int CentralComRead(BleCommand &cmd, char *p_msg, int msg_len);
extern int CentralComWrite(BleCommand cmd, char *p_msg, int msg_len);

extern bool GetScanningEnabled();
extern void SetScanningEnabled(bool enable);

extern bool GetReportEnabled();
extern int  GetReportInterval();
extern int  GetReturnTimeout();

extern bool  GetPostEnabled();
extern char* GetWifiLanAddr();
extern char* GetDevSerialNo();

#include "dealWithOpenwrt.h"

//=============================================================================
#endif /* __BLE_DEFINES_H__ */
