//=============================================================================
// FILE  : ble_central_task.cpp
// DESC  :
// AUTHOR: PINBO
// CREATE: 2015-06-02
// MODIFY:
//=============================================================================

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ble_defines.h"
#include "ble_device.h"
#include "ble_advert.h"
#include "ble_central.h"

#include "cJSON.h"
#include "tinyxml.h"
#include "SerialCom.h"
#include "pthreadCom.h"
#include "cbProjMain.h"
#include "defCom.h"

//=============================================================================
//=============================================================================
int CentralNetRead(BleCommand &cmd, char *p_msg, int msg_len)
{
    int stay_len = 0;
    int read_len = ReadDataFromNet(p_msg, msg_len,
                                   &cmd.net_msg_id,
                                   &cmd.net_phone,
                                   (char*)&cmd.net_flags,
                                   &stay_len, 100);
    if (read_len >= 0)
    {
        // make sure the message end with '\0'!!
        p_msg[read_len] = '\0';
#if 1
        printf("NET message read[id=0x%04X, flags=%d, phone=%d, len=%dB]:",
               cmd.net_msg_id, cmd.net_flags, cmd.net_phone, read_len);
        if (read_len <= 1024)
            printf("\n%s\n", p_msg);
        else
            printf("(...)\n");
#endif
    }

    return read_len;
}

int CentralNetWrite(BleCommand cmd, char *p_msg, int msg_len)
{
    int send_len = WriteDataToNet(p_msg, msg_len,
                                  cmd.net_msg_id,
                                  cmd.net_flags,
                                  cmd.net_phone);
    if (send_len > 0)
    {
#if 1
        printf("NET message send[id=0x%04X, flags=%d, phone=%d, len=%dB]:",
               cmd.net_msg_id, cmd.net_flags, cmd.net_phone, send_len);
        if (send_len <= 1024)
            printf("\n%s\n", p_msg);
        else
            printf("(...)\n");
#endif
    }
    return send_len;
}

//=============================================================================
//=============================================================================
int CentralComRead(BleCommand &cmd, char *p_msg, int msg_len)
{
    int read_len = 0;
    read_len = RecvOneFrame(p_msg, msg_len, &read_len);
    return read_len;
}

int CentralComWrite(BleCommand cmd, char *p_msg, int msg_len)
{
    return SendDataToSerialsCom(p_msg, msg_len, 100);
}

//=============================================================================
//=============================================================================
bool GetScanningEnabled()
{
    IbeaconConfig *pConfig = GetConfigPosition();
    return pConfig->getIsStartScanDev();
}

void SetScanningEnabled(bool enable)
{
    IbeaconConfig *pConfig = GetConfigPosition();
    pConfig->setIsStartScanDev(enable);
}

bool GetReportEnabled()
{
    IbeaconConfig *pConfig = GetConfigPosition();
    return pConfig->getTCPBeaconSerOpenVal();
}

int GetReportInterval()
{
    IbeaconConfig *pConfig = GetConfigPosition();
    return pConfig->getBeaconInterval();
}

int GetReturnTimeout()
{
    return 35;
}

bool GetPostEnabled()
{
    IbeaconConfig *pConfig = GetConfigPosition();
    return pConfig->getIsOpenBeaconSer();
}

char* GetWifiLanAddr()
{
    return GetLanMac();
}

char* GetDevSerialNo()
{
    IbeaconConfig *pConfig = GetConfigPosition();
    return (char *)pConfig->getSerials();
}

