//=============================================================================
// FILE  : ble_central.cpp
// DESC  :
// AUTHOR: PINBO
// CREATE: 2015-06-02
// MODIFY:
//=============================================================================

#include "ble_defines.h"
#include "ble_advert.h"
#include "ble_device.h"
#include "ble_central.h"
#include "cJSON.h"
#include "tinyxml.h"
#include <algorithm>

//=============================================================================
//=============================================================================
BleCentral::BleCentral()
{
    memset(&mCentralAddr, 0, sizeof(mCentralAddr));
    mCentralType = 0;
    mFirmwareVer = 0;

    mConfigPath = "";
    mConfigVer  = "";
    mAdvertList.clear();
    pthread_mutex_init(&mAdvertLock,  NULL);

    mDeviceList.clear();
    pthread_mutex_init(&mDeviceLock,  NULL);

    memset(&mCmdRequest, 0, sizeof(BleCommand));
    mCmdCounter = 1;
    mIsHandling = false;
    mHandleTime = 0;

    mIsScanning = false;
    mReportTime = 0;

    memset(&mConnectAddr, 0, sizeof(mConnectAddr));
    memset(&mConnectSkey[0], 0, BLE_MAX_SKEY_LEN);
    mConnectUUID = 0xFFFF;
    mCharRWIndex = 0xFFFF;
    mCharRWArray.clear();

#ifdef WIN32
    mNetRdThread.p = NULL;
    mComRdThread.p = NULL;
#else
    mNetRdThread = -1;
    mComRdThread = -1;
#endif
    mNetRdRunning= false;
    mComRdRunning= false;
}

BleCentral::~BleCentral()
{
    Stop();
    pthread_mutex_destroy(&mAdvertLock);
    pthread_mutex_destroy(&mDeviceLock);
}

bool BleCentral::ReloadConfig(const char *pConfigPath)
{
    unsigned type = 0;
    unsigned offs = 0;
    unsigned size = 0;
    unsigned order= 0;

    // clear configurations
    UnloadConfig();

    // loading configure file
    TiXmlDocument cfgDoc;
    if (!cfgDoc.LoadFile(pConfigPath))
    {
        BLE_DEBUG("BleCentral::ReloadConfig(): load configure file(%s) failed!",
                  pConfigPath);
        return false;
    }

    TiXmlElement *pRoot = cfgDoc.RootElement();
    if (!pRoot)
    {
        BLE_DEBUG("BleCentral::ReloadConfig(): invalid format of configure file!");
        return false;
    }

    mConfigVer = pRoot->Attribute("version");
    BLE_DEBUG("BleCentral::ReloadConfig(): loading configurations(verion=%s)...",
              mConfigVer.c_str());

    // parsing configure file
    pthread_mutex_lock(&mAdvertLock);
    TiXmlElement *pXmlAdvert = pRoot->FirstChildElement();
    for (; pXmlAdvert; pXmlAdvert = pXmlAdvert->NextSiblingElement())
    {
        BleAdvert *pBLEAdvert = new BleAdvert();
        if (!pBLEAdvert)
        {
            BLE_DEBUG("BleCentral::ReloadConfig(): create advert object failed!");
            pthread_mutex_unlock(&mAdvertLock);
            return false;
        }

        // add advertising format
        mAdvertList.push_back(pBLEAdvert);

        // get advertising info
        unsigned adv_type = BLE_ADV_TYPE_ADVERTIS;
        pXmlAdvert->QueryUnsignedAttribute("type", &adv_type);
        pBLEAdvert->SetAdvertType(adv_type);

        unsigned adv_uuid = 0;
        pXmlAdvert->QueryUnsignedAttribute("uuid", &adv_uuid);
        pBLEAdvert->SetAdvertUUID(adv_uuid);

        // get advertising conditions
        TiXmlElement *pJudgements = pXmlAdvert->FirstChildElement("Judgements");
        for (; pJudgements; pJudgements = pJudgements->NextSiblingElement("Judgements"))
        {
            TiXmlElement *pCondition = pJudgements->FirstChildElement("Condition");
            for (; pCondition; pCondition = pCondition->NextSiblingElement("Condition"))
            {
                BleCondInfo advCond;
                memset(&advCond, 0, sizeof(advCond));

                pCondition->QueryUnsignedAttribute("type",   &type);
                pCondition->QueryUnsignedAttribute("offset", &offs);
                pCondition->QueryUnsignedAttribute("size",   &size);
                pCondition->QueryUnsignedAttribute("order",  &order);

                if ((offs >= BLE_MAX_SCAN_LEN) || 
                    (size >  BLE_MAX_DATA_LEN) || 
                    ((offs + size) > BLE_MAX_SCAN_LEN))
                {
                    BLE_DEBUG("BleCentral::ReloadConfig(): invalid condition offset[%d] and size[%d]!",
                              offs, size);
                    pthread_mutex_unlock(&mAdvertLock);
                    return false;
                }

                const char *pCondData = pCondition->Attribute("value");
                if (pCondData == NULL)
                {
                    BLE_DEBUG("BleCentral::ReloadConfig(): invalid condition data(NULL)!");
                    pthread_mutex_unlock(&mAdvertLock);
                    return false;
                }

                if ((type == BLE_TYPE_SIGNED_NUMBER) || 
                    (type == BLE_TYPE_UNSIGNED_NUMBER))
                {
                    long val = strtol(pCondData, NULL, 0);
                    if (size == 1)
                    {
                        advCond.data[0] = (uint8_t)(val & 0xFF);
                    }
                    if (size == 2)
                    {
                        if (order == 0)
                            big_uint16_encode(val, advCond.data);
                        else
                            uint16_encode(val, advCond.data);
                    }
                    if (size == 4)
                    {
                        if (order == 0)
                            big_uint32_encode(val, advCond.data);
                        else
                            uint32_encode(val, advCond.data);
                    }
                }
                if (type == BLE_TYPE_BYTE_ARRAY)
                {
                    int len = std::min<int>(strlen(pCondData), BLE_MAX_DATA_LEN * 2);
                    MsgASCII2Hex(advCond.data, pCondData, len);
                }

                advCond.type = (uint8_t)type;
                advCond.offs = (uint8_t)offs;
                advCond.size = (uint8_t)size;
                pBLEAdvert->AddCondition(advCond);

            }
        }

        // get advetising parameters
        TiXmlElement *pParameters = pXmlAdvert->FirstChildElement("Parameters");
        for (; pParameters; pParameters = pParameters->NextSiblingElement("Parameters"))
        {
            TiXmlElement *pParameter = pParameters->FirstChildElement("Parameter");
            for (; pParameter; pParameter = pParameter->NextSiblingElement("Parameter"))
            {
                BleParaInfo advPara;
                memset(&advPara, 0, sizeof(advPara));

                pParameter->QueryUnsignedAttribute("type",   &type);
                pParameter->QueryUnsignedAttribute("offset", &offs);
                pParameter->QueryUnsignedAttribute("size",   &size);

                const char *pParaName = pParameter->Attribute("name");
                if (!pParaName)
                {
                    BLE_DEBUG("BleCentral::ReloadConfig(): invalid parameter name!");
                    pthread_mutex_unlock(&mAdvertLock);
                    return false;
                }

                int len = std::min<int>(strlen(pParaName), BLE_MAX_NAME_LEN);
                memcpy(advPara.name, pParaName, len);

                if ((offs >= BLE_MAX_SCAN_LEN) || 
                    (size >  BLE_MAX_DATA_LEN) ||
                    ((offs + size) > BLE_MAX_SCAN_LEN))
                {
                    BLE_DEBUG("BleCentral::ReloadConfig(): invalid parameter offset[%d] and size[%d]!",
                              offs, size);
                    pthread_mutex_unlock(&mAdvertLock);
                    return false;
                }

                advPara.type = (uint8_t)type;
                advPara.offs = (uint8_t)offs;
                advPara.size = (uint8_t)size;
                pBLEAdvert->AddParameter(advPara);
            }
        }
    }

    // load success, remember configure file
    mConfigPath = pConfigPath;
    pthread_mutex_unlock(&mAdvertLock);
    return true;
}

void BleCentral::UnloadConfig()
{
    ADVERT_ITER iter;
    pthread_mutex_lock(&mAdvertLock);

    iter = mAdvertList.begin();
    while (iter != mAdvertList.end())
    {
        delete (*iter);
        mAdvertList.erase(iter);
        iter = mAdvertList.begin();
    }

    pthread_mutex_unlock(&mAdvertLock);
}

bool BleCentral::Start()
{
    // create network message read thread
    mNetRdRunning = false;
    if (pthread_create(&mNetRdThread, NULL, CentralNetReadTask, this) != 0)
    {
        BLE_DEBUG("BleCentral::Start(): create network message read thread failed!");
        return false;
    }

    // create BLE com message read thread
    mComRdRunning = false;
    if (pthread_create(&mComRdThread, NULL, CentralComReadTask, this) != 0)
    {
        BLE_DEBUG("BleCentral::Start(): create BLE com message read thread failed!");
        return false;
    }

    // make sure stop scanning
    mIsScanning = false;
    BleScanningStop(mCmdRequest);
    SLEEP(500);

    // OK, mBLECentral task is running now
    BLE_DEBUG("BleCentral::Start(): BLE central starting...");
    return true;
}

void BleCentral::Stop()
{
    mCmdRequest.ble_seq_no = mCmdCounter++;
    mCmdRequest.ble_msg_id = 0;
    mCmdRequest.net_seq_no = 0;
    mCmdRequest.net_msg_id = 0;
    mCmdCounter = 1;

    // disconnect device for sure
    BleDisconnect(mCmdRequest, mConnectAddr);
    SLEEP(200);

    memset(&mConnectAddr, 0, sizeof(mConnectAddr));
    memset(&mConnectSkey[0], 0, BLE_MAX_SKEY_LEN);
    mConnectUUID = 0xFFFF;
    mCharRWIndex = 0xFFFF;
    mCharRWArray.clear();

    // stop scanning for sure
    BleScanningStop(mCmdRequest);
    mIsScanning = false;
    mReportTime = 0;
    SLEEP(200);

    // waiting network read thread to quit
    mNetRdRunning = false;
#ifdef WIN32
    if (mNetRdThread.p != NULL)
#else
    if (mNetRdThread != (pthread_t)-1)
#endif
    {
        BLE_DEBUG("BleCentral::Stop(): waiting network message read thread to quit...");
        pthread_join(mNetRdThread, NULL);
        SLEEP(200);
#ifdef WIN32
        mNetRdThread.p = NULL;
#else
        mNetRdThread = -1;
#endif
    }

    // waiting BLE com thread to quit
    mComRdRunning = false;
#ifdef WIN32
    if (mComRdThread.p != NULL)
#else
    if (mComRdThread != (pthread_t)-1)
#endif
    {
        BLE_DEBUG("BleCentral::Stop(): waiting BLE com message read thread to quit...");
        pthread_join(mComRdThread, NULL);
        SLEEP(200);
#ifdef WIN32
        mComRdThread.p = NULL;
#else
        mComRdThread = -1;
#endif
    }

    // deleting all devices
    DeleteAllDevices();

    // unload advertising configurations
    UnloadConfig();

    // reset command handling
    mIsHandling = false;
    mHandleTime = 0;
}

bool BleCentral::GetBLEAddress(BleAddrInfo &bleAddr)
{
    int retries = 0;
    int counter = 0;

    BleCommand bleCmd;
    bleCmd.ble_seq_no = mCmdCounter++;
    bleCmd.ble_msg_id = 0;

    while (!mCentralAddr.type && retries < 10)
    {
        BleQueryCentral(bleCmd);
        retries++;

        counter = 0;
        while (!mCentralAddr.type && counter < 100)
        {
            SLEEP(10);
            counter++;
        }
    }

    bleAddr = mCentralAddr;
    return (bleAddr.type != 0);
}

uint16_t BleCentral::GetCentralType()
{
    char buf[8] = {0};
    sprintf(buf, "%4X", mCentralType);
    return atoi(buf);
}

uint16_t BleCentral::GetFirmwareVer()
{
    return mFirmwareVer;
}

bool BleCentral::BleResetCentral(BleCommand &bleCmd)
{
    uint8_t msgLen = 0;
    uint8_t msgBuf[BLE_MAX_MSG_SIZE];

    // making BLE command
    bleCmd.ble_msg_id = BLE_CENTRAL_RESET;
    msgLen+= big_uint16_encode(bleCmd.ble_msg_id, &msgBuf[msgLen]);
    msgLen+= big_uint32_encode(bleCmd.ble_seq_no, &msgBuf[msgLen]);

    // checksum
    msgBuf[msgLen] = MsgChecksum(msgBuf,  msgLen);
    msgLen++;

    // sending BLE command
    return (CentralComWrite(bleCmd, (char *)msgBuf, msgLen) > 0);
}

bool BleCentral::BleScanningStart(BleCommand &bleCmd)
{
    uint8_t msgLen = 0;
    uint8_t msgBuf[BLE_MAX_MSG_SIZE];

    // making BLE command
    bleCmd.ble_msg_id = BLE_SCANNING_START;
    msgLen+= big_uint16_encode(bleCmd.ble_msg_id, &msgBuf[msgLen]);
    msgLen+= big_uint32_encode(bleCmd.ble_seq_no, &msgBuf[msgLen]);

    // checksum
    msgBuf[msgLen] = MsgChecksum(msgBuf,  msgLen);
    msgLen++;

    // sending BLE command
    return (CentralComWrite(bleCmd, (char *)msgBuf, msgLen) > 0);
}

bool BleCentral::BleScanningStop(BleCommand &bleCmd)
{
    uint8_t msgLen = 0;
    uint8_t msgBuf[BLE_MAX_MSG_SIZE];

    // making BLE command
    bleCmd.ble_msg_id = BLE_SCANNING_STOP;
    msgLen+= big_uint16_encode(bleCmd.ble_msg_id, &msgBuf[msgLen]);
    msgLen+= big_uint32_encode(bleCmd.ble_seq_no, &msgBuf[msgLen]);

    // checksum
    msgBuf[msgLen] = MsgChecksum(msgBuf,  msgLen);
    msgLen++;

    // sending BLE command
    return (CentralComWrite(bleCmd, (char *)msgBuf, msgLen) > 0);
}

bool BleCentral::BleConnect(BleCommand &bleCmd, BleAddrInfo &bleAddr)
{
    uint8_t msgLen = 0;
    uint8_t msgBuf[BLE_MAX_MSG_SIZE];

    // making BLE command
    bleCmd.ble_msg_id = BLE_BEACON_CONNECT;
    msgLen+= big_uint16_encode(bleCmd.ble_msg_id, &msgBuf[msgLen]);
    msgLen+= big_uint32_encode(bleCmd.ble_seq_no, &msgBuf[msgLen]);

    // BLE address
    memcpy(&msgBuf[msgLen], bleAddr.addr, BLE_MAX_ADDR_LEN);
    msgLen+= BLE_MAX_ADDR_LEN;
    msgBuf[msgLen++] = bleAddr.type;

    // checking sum
    msgBuf[msgLen] = MsgChecksum(msgBuf, msgLen);
    msgLen++;

    // sending BLE command
    return (CentralComWrite(bleCmd, (char *)msgBuf, msgLen) > 0);
}

bool BleCentral::BleDisconnect(BleCommand &bleCmd, BleAddrInfo &bleAddr)
{
    uint8_t msgLen = 0;
    uint8_t msgBuf[BLE_MAX_MSG_SIZE];

    // making BLE command
    bleCmd.ble_msg_id = BLE_BEACON_DISCONN;
    msgLen+= big_uint16_encode(bleCmd.ble_msg_id, &msgBuf[msgLen]);
    msgLen+= big_uint32_encode(bleCmd.ble_seq_no, &msgBuf[msgLen]);

    // BLE address
    memcpy(&msgBuf[msgLen], bleAddr.addr, BLE_MAX_ADDR_LEN);
    msgLen+= BLE_MAX_ADDR_LEN;
    msgBuf[msgLen++] = bleAddr.type;

    // checksum
    msgBuf[msgLen] = MsgChecksum(msgBuf, msgLen);
    msgLen++;

    // sending BLE command
    return (CentralComWrite(bleCmd, (char *)msgBuf, msgLen) > 0);
}

bool BleCentral::BleQueryChar(BleCommand &bleCmd, BleAddrInfo &bleAddr, BleCharInfo &bleChar)
{
    uint8_t msgLen = 0;
    uint8_t msgBuf[BLE_MAX_MSG_SIZE];

    // making BLE command
    bleCmd.ble_msg_id = BLE_CHARACT_QUERY;
    msgLen+= big_uint16_encode(bleCmd.ble_msg_id, &msgBuf[msgLen]);
    msgLen+= big_uint32_encode(bleCmd.ble_seq_no, &msgBuf[msgLen]);

    // BLE address
    memcpy(&msgBuf[msgLen], bleAddr.addr, BLE_MAX_ADDR_LEN);
    msgLen+= BLE_MAX_ADDR_LEN;
    msgBuf[msgLen++] = bleAddr.type;

    // characteristic UUID
    msgLen+= big_uint16_encode(bleChar.uuid, &msgBuf[msgLen]);

    // checksum
    msgBuf[msgLen] = MsgChecksum(msgBuf, msgLen);
    msgLen++;

    // sending BLE command
    return (CentralComWrite(bleCmd, (char *)msgBuf, msgLen) > 0);
}

bool BleCentral::BleSetupChar(BleCommand &bleCmd, BleAddrInfo &bleAddr, BleCharInfo &bleChar)
{
    uint8_t msgLen = 0;
    uint8_t msgBuf[BLE_MAX_MSG_SIZE];

    // making BLE command
    bleCmd.ble_msg_id = BLE_CHARACT_SETUP;
    msgLen+= big_uint16_encode(bleCmd.ble_msg_id, &msgBuf[msgLen]);
    msgLen+= big_uint32_encode(bleCmd.ble_seq_no, &msgBuf[msgLen]);

    // BLE address
    memcpy(&msgBuf[msgLen], bleAddr.addr, BLE_MAX_ADDR_LEN);
    msgLen+= BLE_MAX_ADDR_LEN;
    msgBuf[msgLen++] = bleAddr.type;

    // characteristic UUID
    msgLen+= big_uint16_encode(bleChar.uuid, &msgBuf[msgLen]);

    // characteristic offset and size
    msgBuf[msgLen++] = bleChar.offs;
    msgBuf[msgLen++] = bleChar.size;

    // characteristic data
    memcpy(&msgBuf[msgLen], bleChar.data, bleChar.size);
    msgLen+= bleChar.size;

    // checksum
    msgBuf[msgLen] = MsgChecksum(msgBuf, msgLen);
    msgLen++;

    // sending BLE command
    return (CentralComWrite(bleCmd, (char *)msgBuf, msgLen) > 0);
}

bool BleCentral::BleQueryCentral(BleCommand &bleCmd)
{
    uint8_t msgLen = 0;
    uint8_t msgBuf[BLE_MAX_MSG_SIZE];

    // query central info message

    // making BLE command
    bleCmd.ble_msg_id = BLE_CENTRAL_QUERY;
    msgLen+= big_uint16_encode(bleCmd.ble_msg_id, &msgBuf[msgLen]);
    msgLen+= big_uint32_encode(bleCmd.ble_seq_no, &msgBuf[msgLen]);

    // checksum
    msgBuf[msgLen] = MsgChecksum(msgBuf, msgLen);
    msgLen++;

    // sending BLE command
    return (CentralComWrite(bleCmd, (char *)msgBuf, msgLen) > 0);
}

bool BleCentral::BleSetupCentral(BleCommand &bleCmd, BleChipInfo &bleChip)
{
    uint8_t msgLen = 0;
    uint8_t msgBuf[BLE_MAX_MSG_SIZE];

    // making BLE command
    bleCmd.ble_msg_id = BLE_CENTRAL_SETUP;
    msgLen+= big_uint16_encode(bleCmd.ble_msg_id, &msgBuf[msgLen]);
    msgLen+= big_uint32_encode(bleCmd.ble_seq_no, &msgBuf[msgLen]);

    memcpy(&msgBuf[msgLen], bleChip.uuid, BLE_MAX_UUID_LEN);
    msgLen+= BLE_MAX_UUID_LEN;

    memcpy(&msgBuf[msgLen], bleChip.skey, BLE_MAX_SKEY_LEN);
    msgLen+= BLE_MAX_SKEY_LEN;

    msgLen+= big_uint16_encode(bleChip.major,  &msgBuf[msgLen]);
    msgLen+= big_uint16_encode(bleChip.minor,  &msgBuf[msgLen]);
    msgLen+= big_uint16_encode(bleChip.mode,   &msgBuf[msgLen]);
    msgBuf[msgLen++] = (uint8_t)bleChip.ms_power;
    msgBuf[msgLen++] = (uint8_t)bleChip.tx_power;
    msgLen+= big_uint16_encode(bleChip.adv_interval, &msgBuf[msgLen]);
    msgLen+= big_uint16_encode(bleChip.bat_interval, &msgBuf[msgLen]);
    msgLen+= big_uint16_encode(bleChip.tmp_interval, &msgBuf[msgLen]);
    msgLen+= big_uint16_encode(bleChip.asw_interval, &msgBuf[msgLen]);

    memcpy(&msgBuf[msgLen], bleChip.eurl, BLE_MAX_EURL_LEN);
    msgLen+= BLE_MAX_EURL_LEN;
    msgBuf[msgLen++] = bleChip.eurl[BLE_MAX_EURL_LEN];

    memcpy(&msgBuf[msgLen], bleChip.name, BLE_MAX_NAME_LEN);
    msgLen+= BLE_MAX_NAME_LEN;

    // checksum
    msgBuf[msgLen] = MsgChecksum(msgBuf, msgLen);
    msgLen++;

    // sending BLE command
    return (CentralComWrite(bleCmd, (char *)msgBuf, msgLen) > 0);
}

bool BleCentral::BleUpdateCentral(BleCommand &bleCmd, uint8_t *data, uint8_t len)
{
    uint8_t msgLen = 0;
    uint8_t msgBuf[BLE_MAX_MSG_SIZE];

    // making BLE command
    bleCmd.ble_msg_id = BLE_CENTRAL_UPDATE;
    msgLen+= big_uint16_encode(bleCmd.ble_msg_id, &msgBuf[msgLen]);
    msgLen+= big_uint32_encode(bleCmd.ble_seq_no, &msgBuf[msgLen]);

    // packet data
    memcpy(&msgBuf[msgLen], data, len);
    msgLen+= len;

    // checking sum
    msgBuf[msgLen] = MsgChecksum(msgBuf, msgLen);
    msgLen++;

    // sending BLE command
    return (CentralComWrite(bleCmd, (char *)msgBuf, msgLen) > 0);
}

bool BleCentral::BleSetupUserData(BleCommand &bleCmd, uint8_t *data, uint8_t len)
{
    uint8_t msgLen = 0;
    uint8_t msgBuf[BLE_MAX_MSG_SIZE];

    // making BLE command
    bleCmd.ble_msg_id = BLE_USRDATA_SETUP;
    msgLen+= big_uint16_encode(bleCmd.ble_msg_id, &msgBuf[msgLen]);
    msgLen+= big_uint32_encode(bleCmd.ble_seq_no, &msgBuf[msgLen]);

    // packet data
    msgBuf[msgLen++] = len;
    memcpy(&msgBuf[msgLen], data, len);
    msgLen+= len;

    // checking sum
    msgBuf[msgLen] = MsgChecksum(msgBuf, msgLen);
    msgLen++;

    // sending BLE command
    return (CentralComWrite(bleCmd, (char *)msgBuf, msgLen) > 0);
}

void* BleCentral::CentralNetReadTask(void *arg)
{
    BleCentral *pCentral = (BleCentral *)arg;
    assert(pCentral != NULL);

    int         msgLen;
    char        msgBuf[NET_MAX_MSG_SIZE];
    BleCommand  bleCmd;

    pCentral->mNetRdRunning = true;
    while (pCentral->mNetRdRunning)
    {
        // operation timeout(35 seconds)
        if (pCentral->mIsHandling &&
            (GetReturnTimeout() <= (time(0) - pCentral->mHandleTime)))
        {
            BLE_DEBUG("BleCentral::CentralNetReadTask(): (msg=0x%04X, seq=%d)handle timeout!",
                      pCentral->mCmdRequest.net_msg_id,
                      pCentral->mCmdRequest.net_seq_no);

            pCentral->mCmdRequest.ret_value = ACK_RESULT_TIMEOUT;
            pCentral->GeneralReporting(pCentral->mCmdRequest);

            pCentral->mCmdRequest.ble_seq_no = 0;
            pCentral->mCmdRequest.net_seq_no = 0;
            pCentral->mIsHandling = false;
        }

        // report scanning devices
        if (pCentral->mIsScanning && !pCentral->mIsHandling &&
            (GetReportInterval() <= (time(0) - pCentral->mReportTime)))
        {
            BLE_DEBUG("BleCentral::CentralNetReadTask(): reporting all devices[%d]...",
                      pCentral->mDeviceList.size());

            bleCmd.net_msg_id = NET_SCANNING_REPORT;
            bleCmd.net_seq_no = pCentral->mCmdCounter++;
            bleCmd.net_flags  = 1;
            bleCmd.net_phone  = 0;
            pCentral->ReportAllDevices(bleCmd);

            if (GetPostEnabled())
            {
                bleCmd.net_msg_id = NET_SCANNING_POST;
                bleCmd.net_seq_no = pCentral->mCmdCounter++;
                bleCmd.net_flags  = 1;
                bleCmd.net_phone  = 0;
                pCentral->PostAllDevices(bleCmd);
            }

            pCentral->DeleteAllDevices();
            pCentral->mReportTime = time(0);
        }

        // get network message
        msgLen = CentralNetRead(bleCmd, msgBuf, NET_MAX_MSG_SIZE);
        if (msgLen <= 0) continue;

        // handle message
        switch (bleCmd.net_msg_id)
        {
        case NET_SCANCFG_QUERY_REQ:
            BLE_DEBUG("BleCentral:CentralNetReadTask(): (msg=0x%04X, nflags=%d)query scanning configure...",
                      bleCmd.net_msg_id, bleCmd.net_flags);
            pCentral->HandleQueryScanCfg(bleCmd, msgBuf, msgLen);
            break;
        case NET_SCANNING_START_REQ:
            BLE_DEBUG("BleCentral:CentralNetReadTask(): (msg=0x%04X, nflags=%d)request start scanning...",
                      bleCmd.net_msg_id, bleCmd.net_flags);
            pCentral->HandleScanningStart(bleCmd, msgBuf, msgLen);
            break;
        case NET_SCANNING_STOP_REQ:
            BLE_DEBUG("BleCentral:CentralNetReadTask(): (msg=0x%04X, nflags=%d)request stop scanning...",
                      bleCmd.net_msg_id, bleCmd.net_flags);
            pCentral->HandleScanningStop(bleCmd, msgBuf, msgLen);
            break;
        case NET_BEACON_QUERY_REQ:
            BLE_DEBUG("BleCentral:CentralNetReadTask(): (msg=0x%04X, nflags=%d)query beacon services...",
                      bleCmd.net_msg_id, bleCmd.net_flags);
            pCentral->HandleQueryBeacon(bleCmd, msgBuf, msgLen);
            break;
        case NET_BEACON_SETUP_REQ:
            BLE_DEBUG("BleCentral:CentralNetReadTask(): (msg=0x%04X, nflags=%d)setup beacon services...",
                      bleCmd.net_msg_id, bleCmd.net_flags);
            pCentral->HandleSetupBeacon(bleCmd, msgBuf, msgLen);
            break;
        case NET_CENTRAL_QUERY_REQ:
            BLE_DEBUG("BleCentral:CentralNetReadTask(): (msg=0x%04X, nflags=%d)query central BLE info...",
                      bleCmd.net_msg_id, bleCmd.net_flags);
            pCentral->HandleQueryCentral(bleCmd, msgBuf, msgLen);
            break;
        case NET_CENTRAL_SETUP_REQ:
            BLE_DEBUG("BleCentral:CentralNetReadTask(): (msg=0x%04X, nflags=%d)setup central BLE info...",
                      bleCmd.net_msg_id, bleCmd.net_flags);
            pCentral->HandleSetupCentral(bleCmd, msgBuf, msgLen);
        default:
            BLE_DEBUG("BleCentral::CentralNetReadTask(): (msg=0x%04X, nflags=%d)not supported message!",
                      bleCmd.net_msg_id, bleCmd.net_flags);
            break;
        }
    }

    BLE_DEBUG("BleCentral::CentralNetReadTask(): thread quit now.");
    return 0;
}

bool BleCentral::HandleQueryScanCfg(BleCommand &bleCmd, const char *msgBuf, int msgLen)
{
    // parsing message data
    cJSON *pMsgRoot = cJSON_Parse(msgBuf);
    cJSON *pMsgSeqN = cJSON_GetObjectItem(pMsgRoot, "seq_no");
    if (pMsgSeqN == NULL)
    {
        BLE_DEBUG("BleCentral::HandleQueryScanCfg(): invalid NET message format!");
        cJSON_Delete(pMsgRoot);

        bleCmd.net_msg_id = NET_SCANCFG_QUERY_ACK;
        bleCmd.ret_value  = ACK_RESULT_ERROR;
        return GeneralReporting(bleCmd);
    }

    bleCmd.net_msg_id = NET_SCANCFG_QUERY_ACK;
    bleCmd.net_seq_no = pMsgSeqN->valueint;
    bleCmd.ret_value  = ACK_RESULT_SUCCEED;

    // making NET acknowledge message
    cJSON *pOutRoot = cJSON_CreateObject();
    cJSON_AddNumberToObject(pOutRoot, "ack_no", bleCmd.net_seq_no);
    cJSON_AddNumberToObject(pOutRoot, "ret_no", bleCmd.ret_value);
    cJSON_AddNumberToObject(pOutRoot, "is_scanning_now", mIsScanning ? 1 : 0);
    cJSON_AddNumberToObject(pOutRoot, "enable_scanning", GetScanningEnabled() ? 1 : 0);
    cJSON_AddNumberToObject(pOutRoot, "report_interval", GetReportInterval());
    cJSON_AddNumberToObject(pOutRoot, "return_timeout",  GetReturnTimeout());
    cJSON_AddStringToObject(pOutRoot, "config_version",  mConfigVer.c_str());

    // send NET acknowledge message
    char *pOutBuff = cJSON_PrintUnformatted(pOutRoot);
    int   nOutSize = CentralNetWrite(bleCmd, pOutBuff, strlen(pOutBuff));

    // do clear work
    if (pOutBuff) free(pOutBuff);
    if (pOutRoot) cJSON_Delete(pOutRoot);
    if (pMsgRoot) cJSON_Delete(pMsgRoot);
    return (nOutSize > 0);
}

bool BleCentral::HandleScanningStart(BleCommand &bleCmd, const char *msgBuf, int msgLen)
{
    // parsing message data
    cJSON *pMsgRoot = cJSON_Parse(msgBuf);
    cJSON *pMsgSeqN = cJSON_GetObjectItem(pMsgRoot, "seq_no");
    if (pMsgSeqN == NULL)
    {
        BLE_DEBUG("BleCentral::HandleScanningStart(): invalid NET message format!");
        cJSON_Delete(pMsgRoot);

        bleCmd.net_msg_id = NET_SCANNING_START_ACK;
        bleCmd.ret_value  = ACK_RESULT_ERROR;
        return GeneralReporting(bleCmd);
    }

    bleCmd.net_seq_no = pMsgSeqN->valueint;
    bleCmd.ret_value  = ACK_RESULT_SUCCEED;

    // make sure not handling other message
    if (mIsHandling)
    {
        BLE_DEBUG("BleCentral::HandleScanningStart(): message handling busy now!");
        cJSON_Delete(pMsgRoot);

        bleCmd.net_msg_id = NET_SCANNING_START_ACK;
        bleCmd.ret_value  = ACK_RESULT_BUSY;
        return GeneralReporting(bleCmd);
    }

    // start handling
    mIsHandling = true;
    mHandleTime = time(0);
    mCmdRequest = bleCmd;
    mCmdRequest.net_msg_id = NET_SCANNING_START_ACK;
    mCmdRequest.ble_seq_no = mCmdCounter++;

    // enable BLE scanning
    SetScanningEnabled(true);

    // send BLE command
    cJSON_Delete(pMsgRoot);
    return BleScanningStart(mCmdRequest);
}


bool BleCentral::ResumeScanning()
{
    if (!GetScanningEnabled())
        return true;

    BLE_DEBUG("BleCentral::ResumeScanning(): restart BLE scanning...");

    BleCommand bleCmd;
    bleCmd.ble_seq_no = mCmdCounter++;
    bleCmd.ble_msg_id = BLE_SCANNING_START;

    BleScanningStart(bleCmd);
    SLEEP(200);

    mIsScanning = true;
    mReportTime = time(0);
    return true;
}


bool BleCentral::HandleScanningStop(BleCommand &bleCmd, const char *msgBuf, int msgLen)
{
    // parsing message data
    cJSON *pMsgRoot = cJSON_Parse(msgBuf);
    cJSON *pMsgSeqN = cJSON_GetObjectItem(pMsgRoot, "seq_no");
    if (pMsgSeqN == NULL)
    {
        BLE_DEBUG("BleCentral::HandleScanningStop(): invalid NET message format!");
        cJSON_Delete(pMsgRoot);

        bleCmd.net_msg_id = NET_SCANNING_STOP_ACK;
        bleCmd.ret_value  = ACK_RESULT_ERROR;
        return GeneralReporting(bleCmd);
    }

    bleCmd.net_seq_no = pMsgSeqN->valueint;
    bleCmd.ret_value  = ACK_RESULT_SUCCEED;

    // make sure not handling other message
    if (mIsHandling)
    {
        BLE_DEBUG("BleCentral::HandleScanningStop(): message handling busy now!");
        cJSON_Delete(pMsgRoot);

        bleCmd.net_msg_id = NET_SCANNING_STOP_ACK;
        bleCmd.ret_value  = ACK_RESULT_BUSY;
        return GeneralReporting(bleCmd);
    }

    // start handling
    bleCmd.net_msg_id = NET_SCANNING_STOP_ACK;
    bleCmd.ble_seq_no = mCmdCounter++;
    bleCmd.ret_value  = ACK_RESULT_SUCCEED;

    // disable BLE scanning immediately
    SetScanningEnabled(false);
    mIsScanning = false;

    // stop BLE scanner
    BleScanningStop(bleCmd);

    // acknowledge imediately
    cJSON_Delete(pMsgRoot);
    return GeneralReporting(bleCmd);
}

bool BleCentral::HandleQueryBeacon(BleCommand &bleCmd, const char *msgBuf, int msgLen)
{
    // parsing message data
    cJSON *pMsgRoot = cJSON_Parse(msgBuf);
    cJSON *pMsgSeqN = cJSON_GetObjectItem(pMsgRoot, "seq_no");
    cJSON *pBleAddr = cJSON_GetObjectItem(pMsgRoot, "ble_addr");
    cJSON *pBleType = cJSON_GetObjectItem(pMsgRoot, "addr_type");
    cJSON *pKeyUUID = cJSON_GetObjectItem(pMsgRoot, "key_uuid");
    cJSON *pConnKey = cJSON_GetObjectItem(pMsgRoot, "conn_key");
    cJSON *pCharLst = cJSON_GetObjectItem(pMsgRoot, "char_list");
    if (!pMsgSeqN || !pBleAddr || !pBleType || !pConnKey || !pCharLst)
    {
        BLE_DEBUG("BleCentral::HandleQueryBeacon(): invalid NET message format!");
        cJSON_Delete(pMsgRoot);

        bleCmd.net_msg_id = NET_BEACON_QUERY_ACK;
        bleCmd.ret_value  = ACK_RESULT_ERROR;
        return GeneralReporting(bleCmd);
    }

    bleCmd.net_seq_no = pMsgSeqN->valueint;
    bleCmd.ret_value  = ACK_RESULT_SUCCEED;

    // make sure not handling other message
    if (mIsHandling)
    {
        BLE_DEBUG("BleCentral::HandleQueryBeacon(): message handling busy now!");
        cJSON_Delete(pMsgRoot);

        bleCmd.net_msg_id = NET_BEACON_QUERY_ACK;
        bleCmd.ret_value  = ACK_RESULT_BUSY;
        return GeneralReporting(bleCmd);
    }

    // start handling
    mIsHandling = true;
    mHandleTime = time(0);
    mCmdRequest = bleCmd;
    mCmdRequest.net_msg_id = NET_BEACON_QUERY_ACK;
    mCmdRequest.ble_seq_no = mCmdCounter++;

    // parsing BLE address
    int len = std::min<int>(strlen(pBleAddr->valuestring), BLE_MAX_ADDR_LEN * 2);
    MsgASCII2Hex(mConnectAddr.addr, pBleAddr->valuestring, len);
    mConnectAddr.type = (uint8_t)pBleType->valueint;

    // parsing security key
    len = std::min<int>(strlen(pConnKey->valuestring), BLE_MAX_SKEY_LEN * 2);
    MsgASCII2Hex(mConnectSkey, pConnKey->valuestring, len);
    mConnectUUID = (uint16_t)pKeyUUID->valueint;

    // parsing characteristics
    mCharRWArray.clear();
    int count = cJSON_GetArraySize(pCharLst);
    for (int i = 0; i < count; i++)
    {
        cJSON *pChar = cJSON_GetArrayItem(pCharLst, i);
        if (pChar == NULL)
        {
            BLE_DEBUG("BleCentral::HandleQueryBeacon(): invalid NET message format!");
            cJSON_Delete(pMsgRoot);

            mCmdRequest.ret_value = ACK_RESULT_ERROR;
            GeneralReporting(mCmdRequest);

            mCmdRequest.ble_seq_no = 0;
            mCmdRequest.net_seq_no = 0;
            mIsHandling = false;
            return true;
        }

        cJSON *pUUID = cJSON_GetObjectItem(pChar, "uuid");
        cJSON *pType = cJSON_GetObjectItem(pChar, "type");
        if (!pUUID || !pType)
        {
            BLE_DEBUG("BleCentral::HandleQueryBeacon(): invalid NET message format!");
            cJSON_Delete(pMsgRoot);

            mCmdRequest.ret_value = ACK_RESULT_ERROR;
            GeneralReporting(mCmdRequest);

            mCmdRequest.ble_seq_no = 0;
            mCmdRequest.net_seq_no = 0;
            mIsHandling = false;
            return true;
        }

        BleCharInfo bleChar;
        memset(&bleChar, 0, sizeof(bleChar));
        bleChar.stat = ACK_RESULT_FAILED;
        bleChar.uuid = (uint16_t)pUUID->valueint;
        bleChar.type = (uint8_t)pType->valueint;
        bleChar.offs = 0;
        bleChar.size = 0;
        mCharRWArray.push_back(bleChar);

        BLE_DEBUG("characteristic to be query: uuid=0x%04X, type=%d",
                  bleChar.uuid, bleChar.type);
    }

    BLE_DEBUG("BleCentral::HandleQueryBeacon(): connecting to device[%02X:%02X:%02X:%02X:%02X:%02X]...",
              mConnectAddr.addr[0], mConnectAddr.addr[1], mConnectAddr.addr[2],
              mConnectAddr.addr[3], mConnectAddr.addr[4], mConnectAddr.addr[5]);

    // send BLE connect command
    cJSON_Delete(pMsgRoot);
    mCharRWIndex = 0xFFFF;
    return BleConnect(mCmdRequest,
                      mConnectAddr);
}

bool BleCentral::HandleSetupBeacon(BleCommand &bleCmd, const char *msgBuf, int msgLen)
{
    // parsing message data
    cJSON *pMsgRoot = cJSON_Parse(msgBuf);
    cJSON *pMsgSeqN = cJSON_GetObjectItem(pMsgRoot, "seq_no");
    cJSON *pBleAddr = cJSON_GetObjectItem(pMsgRoot, "ble_addr");
    cJSON *pBleType = cJSON_GetObjectItem(pMsgRoot, "addr_type");
    cJSON *pKeyUUID = cJSON_GetObjectItem(pMsgRoot, "key_uuid");
    cJSON *pConnKey = cJSON_GetObjectItem(pMsgRoot, "conn_key");
    cJSON *pCharLst = cJSON_GetObjectItem(pMsgRoot, "char_list");
    if (!pMsgSeqN || !pBleAddr || !pBleType || !pConnKey || !pCharLst)
    {
        BLE_DEBUG("BleCentral::HandleSetupBeacon(): invalid NET message format!");
        cJSON_Delete(pMsgRoot);

        bleCmd.net_msg_id = NET_BEACON_SETUP_ACK;
        bleCmd.ret_value  = ACK_RESULT_ERROR;
        return GeneralReporting(bleCmd);
    }

    bleCmd.net_seq_no = pMsgSeqN->valueint;
    bleCmd.ret_value  = ACK_RESULT_SUCCEED;

    // make sure not handling other message
    if (mIsHandling)
    {
        BLE_DEBUG("BleCentral::HandleSetupBeacon(): message handling busy now!");
        cJSON_Delete(pMsgRoot);

        bleCmd.net_msg_id = NET_BEACON_SETUP_ACK;
        bleCmd.ret_value  = ACK_RESULT_BUSY;
        return GeneralReporting(bleCmd);
    }

    // start handling
    mIsHandling = true;
    mHandleTime = time(0);
    mCmdRequest = bleCmd;
    mCmdRequest.net_msg_id = NET_BEACON_SETUP_ACK;
    mCmdRequest.ble_seq_no = mCmdCounter++;

    // parsing BLE address
    int len = std::min<int>(strlen(pBleAddr->valuestring), BLE_MAX_ADDR_LEN * 2);
    MsgASCII2Hex(mConnectAddr.addr, pBleAddr->valuestring, len);
    mConnectAddr.type = (uint8_t)pBleType->valueint;

    // parsing security key
    len = std::min<int>(strlen(pConnKey->valuestring), BLE_MAX_SKEY_LEN * 2);
    MsgASCII2Hex(mConnectSkey, pConnKey->valuestring, len);
    mConnectUUID = (uint16_t)pKeyUUID->valueint;

    // parsing characteristics
    mCharRWArray.clear();
    int count = cJSON_GetArraySize(pCharLst);
    for (int i = 0; i < count; i++)
    {
        cJSON *pChar = cJSON_GetArrayItem(pCharLst, i);
        if (pChar == NULL)
        {
            BLE_DEBUG("BleCentral::HandleSetupBeacon(): invalid NET message format!");
            cJSON_Delete(pMsgRoot);

            mCmdRequest.ret_value = ACK_RESULT_ERROR;
            GeneralReporting(mCmdRequest);

            mCmdRequest.ble_seq_no = 0;
            mCmdRequest.net_seq_no = 0;
            mIsHandling = false;
            return true;
        }

        cJSON *pUUID = cJSON_GetObjectItem(pChar, "uuid");
        cJSON *pType = cJSON_GetObjectItem(pChar, "type");
        cJSON *pOffs = cJSON_GetObjectItem(pChar, "offs");
        cJSON *pSize = cJSON_GetObjectItem(pChar, "size");
        cJSON *pData = cJSON_GetObjectItem(pChar, "data");
        if (!pUUID || !pType || !pOffs || !pSize || !pData)
        {
            BLE_DEBUG("BleCentral::HandleSetupBeacon(): invalid NET message format!");
            cJSON_Delete(pMsgRoot);

            mCmdRequest.ret_value = ACK_RESULT_ERROR;
            GeneralReporting(mCmdRequest);

            mCmdRequest.ble_seq_no = 0;
            mCmdRequest.net_seq_no = 0;
            mIsHandling = false;
            return true;
        }

        BleCharInfo bleChar;
        memset(&bleChar, 0, sizeof(bleChar));
        bleChar.stat = ACK_RESULT_FAILED;
        bleChar.uuid = (uint16_t)pUUID->valueint;
        bleChar.type = (uint8_t)pType->valueint;
        bleChar.offs = (uint8_t)pOffs->valueint;
        bleChar.size = (uint8_t)pSize->valueint;

        if (bleChar.type != BLE_TYPE_STRING)
        {
            len = std::min<int>(strlen(pData->valuestring), BLE_MAX_DATA_LEN * 2);
            MsgASCII2Hex(bleChar.data, pData->valuestring, len);
        }
        else
        {
            len = std::min<int>(strlen(pData->valuestring), BLE_MAX_DATA_LEN);
            memcpy(bleChar.data, string_check(pData->valuestring), len);
        }

        mCharRWArray.push_back(bleChar);
        BLE_DEBUG("characteristic to be setup: uuid=0x%04X, type=%d, offs=%d, size=%d",
                   bleChar.uuid, bleChar.type, bleChar.offs, bleChar.size);
    }

    BLE_DEBUG("BleCentral::HandleSetupBeacon(): connecting to device[%02X:%02X:%02X:%02X:%02X:%02X]...",
              mConnectAddr.addr[0], mConnectAddr.addr[1], mConnectAddr.addr[2],
              mConnectAddr.addr[3], mConnectAddr.addr[4], mConnectAddr.addr[5]);

    // send BLE connect command
    cJSON_Delete(pMsgRoot);
    mCharRWIndex = 0xFFFF;
    return BleConnect(mCmdRequest,
                      mConnectAddr);
}

bool BleCentral::HandleQueryCentral(BleCommand &bleCmd, const char *msgBuf, int msgLen)
{
    // parsing message data
    cJSON *pMsgRoot = cJSON_Parse(msgBuf);
    cJSON *pMsgSeqN = cJSON_GetObjectItem(pMsgRoot, "seq_no");
    if (pMsgSeqN == NULL)
    {
        BLE_DEBUG("BleCentral::HandleQueryCentral(): invalid NET message format!");
        cJSON_Delete(pMsgRoot);

        bleCmd.net_msg_id = NET_CENTRAL_QUERY_ACK;
        bleCmd.ret_value  = ACK_RESULT_ERROR;
        return GeneralReporting(bleCmd);
    }

    bleCmd.net_seq_no = pMsgSeqN->valueint;
    bleCmd.ret_value  = ACK_RESULT_SUCCEED;

    // make sure not handling other message
    if (mIsHandling)
    {
        BLE_DEBUG("BleCentral::HandleQueryCentral(): message handling busy now!");
        cJSON_Delete(pMsgRoot);

        bleCmd.net_msg_id = NET_CENTRAL_QUERY_ACK;
        bleCmd.ret_value  = ACK_RESULT_BUSY;
        return GeneralReporting(bleCmd);
    }

    // start handling
    mIsHandling = true;
    mHandleTime = time(0);
    mCmdRequest = bleCmd;
    mCmdRequest.net_msg_id = NET_CENTRAL_QUERY_ACK;
    mCmdRequest.ble_seq_no = mCmdCounter++;

    // send BLE central query command
    cJSON_Delete(pMsgRoot);
    return BleQueryCentral(mCmdRequest);
}

bool BleCentral::HandleSetupCentral(BleCommand &bleCmd, const char *msgBuf, int msgLen)
{
    // parsing message data
    cJSON *pMsgRoot     = cJSON_Parse(msgBuf);
    cJSON *pMsgSeqN     = cJSON_GetObjectItem(pMsgRoot, "seq_no");
    cJSON *pBleUUID     = cJSON_GetObjectItem(pMsgRoot, "ble_uuid");
    cJSON *pDevSKey     = cJSON_GetObjectItem(pMsgRoot, "dev_skey");
    cJSON *pMajor       = cJSON_GetObjectItem(pMsgRoot, "major");
    cJSON *pMinor       = cJSON_GetObjectItem(pMsgRoot, "minor");
    cJSON *pDevMode     = cJSON_GetObjectItem(pMsgRoot, "dev_mode");
    cJSON *pMsPower     = cJSON_GetObjectItem(pMsgRoot, "ms_power");
    cJSON *pTxPower     = cJSON_GetObjectItem(pMsgRoot, "tx_power");
    cJSON *pAdvInterval = cJSON_GetObjectItem(pMsgRoot, "adv_interval");
    cJSON *pBatInterval = cJSON_GetObjectItem(pMsgRoot, "batt_check_interval");
    cJSON *pTmpInterval = cJSON_GetObjectItem(pMsgRoot, "temp_check_interval");
    cJSON *pAswInterval = cJSON_GetObjectItem(pMsgRoot, "adv_switch_interval");
    cJSON *pEddystoneUrl= cJSON_GetObjectItem(pMsgRoot, "eddystone_url");
    cJSON *pBleName  = cJSON_GetObjectItem(pMsgRoot, "ble_name");
    if (!pMsgSeqN || !pBleUUID || !pDevSKey || !pMajor || !pMinor || 
        !pBleName || !pDevMode || !pMsPower || !pTxPower || !pAdvInterval || 
        !pBatInterval || !pTmpInterval || !pAswInterval || !pEddystoneUrl)
    {
        BLE_DEBUG("BleCentral::HandleSetupCentral(): invalid NET message format!");
        cJSON_Delete(pMsgRoot);

        bleCmd.net_msg_id = NET_CENTRAL_SETUP_ACK;
        bleCmd.ret_value  = ACK_RESULT_ERROR;
        return GeneralReporting(bleCmd);
    }

    bleCmd.net_seq_no = pMsgSeqN->valueint;
    bleCmd.ret_value  = ACK_RESULT_SUCCEED;

    // make sure not handling other message
    if (mIsHandling)
    {
        BLE_DEBUG("BleCentral::HandleSetupCentral(): message handling busy now!");
        cJSON_Delete(pMsgRoot);

        bleCmd.net_msg_id = NET_CENTRAL_SETUP_ACK;
        bleCmd.ret_value  = ACK_RESULT_BUSY;
        return GeneralReporting(bleCmd);
    }

    // start handling
    mIsHandling = true;
    mHandleTime = time(0);
    mCmdRequest = bleCmd;
    mCmdRequest.net_msg_id = NET_CENTRAL_SETUP_ACK;
    mCmdRequest.ble_seq_no = mCmdCounter++;

    // parsing BLE information
    BleChipInfo bleChip;
    memset(&bleChip, 0, sizeof(bleChip));
    bleChip.major        = (uint16_t)pMajor->valueint;
    bleChip.minor        = (uint16_t)pMinor->valueint;
    bleChip.mode         = (uint16_t)pDevMode->valueint;
    bleChip.ms_power     = (int8_t)pMsPower->valueint;
    bleChip.tx_power     = (int8_t)pTxPower->valueint;
    bleChip.adv_interval = (uint16_t)pAdvInterval->valueint;
    bleChip.bat_interval = (uint16_t)pBatInterval->valueint;
    bleChip.tmp_interval = (uint16_t)pBatInterval->valueint;
    bleChip.asw_interval = (uint16_t)pAswInterval->valueint;

    // parsing BLE uuid
    int len = std::min<int>(strlen(pBleUUID->valuestring), BLE_MAX_UUID_LEN * 2);
    MsgASCII2Hex(bleChip.uuid, pBleUUID->valuestring, len);

    // parsing BLE security key
    len = std::min<int>(strlen(pDevSKey->valuestring), BLE_MAX_SKEY_LEN * 2);
    MsgASCII2Hex(bleChip.skey, pDevSKey->valuestring, len);

    // parsing eddystone URL
    len = strlen(pEddystoneUrl->valuestring);
    len = EddystoneURL2Hex(bleChip.eurl, pEddystoneUrl->valuestring, len);
    bleChip.eurl[BLE_MAX_EURL_LEN] = len;

    // parsing BLE device name
    len = std::min<int>(strlen(pBleName->valuestring), BLE_MAX_NAME_LEN);
    memcpy(bleChip.name, string_check(pBleName->valuestring), len);

    // send BLE command
    cJSON_Delete(pMsgRoot);
    return BleSetupCentral(mCmdRequest, bleChip);
}

bool BleCentral::GeneralReporting(BleCommand &bleCmd)
{
    // making NET acknowledge message
    cJSON *pOutRoot = cJSON_CreateObject();
    cJSON_AddNumberToObject(pOutRoot, "ack_no", bleCmd.net_seq_no);
    cJSON_AddNumberToObject(pOutRoot, "ret_no", bleCmd.ret_value);

    // append device address when acknowledge service
    // querying or setuping.
    if ((bleCmd.net_msg_id == NET_BEACON_QUERY_REQ) ||
        (bleCmd.net_msg_id == NET_BEACON_QUERY_ACK) ||
        (bleCmd.net_msg_id == NET_BEACON_SETUP_REQ) ||
        (bleCmd.net_msg_id == NET_BEACON_SETUP_ACK))
    {
        char buffer[64] = {0};
        MsgHex2ASCII(buffer, mConnectAddr.addr, BLE_MAX_ADDR_LEN);
        cJSON_AddStringToObject(pOutRoot, "ble_addr",  buffer);
        cJSON_AddNumberToObject(pOutRoot, "addr_type", mConnectAddr.type);
    }

    // sending NET message
    char *pOutBuff = cJSON_PrintUnformatted(pOutRoot);
    int   nOutSize = CentralNetWrite(bleCmd, pOutBuff, strlen(pOutBuff));

    // do clear work
    if (pOutBuff) free(pOutBuff);
    if (pOutRoot) cJSON_Delete(pOutRoot);
    return (nOutSize > 0);
}

void* BleCentral::CentralComReadTask(void *arg)
{
    BleCentral *pCentral = (BleCentral *)arg;
    assert(pCentral != NULL);

    uint8_t     msgCRC;
    int         msgLen;
    char        msgBuf[COM_MAX_BUF_SIZE];
    BleCommand  bleCmd;

    pCentral->mComRdRunning = true;
    while (pCentral->mComRdRunning)
    {
        // get BLE com message
        msgLen = CentralComRead(bleCmd, msgBuf, COM_MAX_BUF_SIZE);
        if (msgLen <= 0) continue;

        // CRC checking
        msgLen--;
        msgCRC = MsgChecksum((uint8_t *)msgBuf, msgLen);
        if (msgCRC != (uint8_t)msgBuf[msgLen])
            continue;

        // parsing message header
        bleCmd.ble_msg_id = big_uint16_decode((uint8_t *)&msgBuf[0]);
        bleCmd.ble_seq_no = big_uint32_decode((uint8_t *)&msgBuf[2]);
        msgLen -= 6;

#if 0
        static int count1 = 0;
        static int count2 = 0;
        static time_t time1 = time(0);
        static time_t time2 = time(0);

        if (bleCmd.ble_msg_id == BLE_SCANNING_REPORT)
        {
            count1++;
            count2++;
        }
        if (1 < (time(0) - time1))
        {
            printf("scanned count in 1 second: %d\n", count1 / 2);
            count1 = 0;
            time1  = time(0);
        }
        if (10 < (time(0) - time2))
        {
            printf("scanned count in 10 seconds: %d\n", count2 / 2);
            count2 = 0;
            time2  = time(0);
        }
#endif
        // handle message
        switch (bleCmd.ble_msg_id)
        {
        case BLE_GENERAL_ACK:
            BLE_DEBUG("BleCentral::CentralComReadTask(): (msg=0x%04X, seq=%d)general acknowledge...",
                      bleCmd.ble_msg_id, bleCmd.ble_seq_no);
            pCentral->HandleGeneralAck(bleCmd, &msgBuf[6], msgLen);
            break;
        case BLE_SCANNING_REPORT:
            //BLE_DEBUG("BleCentral::CentralComReadTask(): (msg=0x%04X, seq=%d)scanning reporting...",
            //          bleCmd.ble_msg_id, bleCmd.ble_seq_no);
            pCentral->HandleScannedReport(bleCmd, &msgBuf[6], msgLen);
            break;
        case BLE_CHARACT_REPORT:
            BLE_DEBUG("BleCentral::CentralComReadTask(): (msg=0x%04X, seq=%d)character reporting...",
                      bleCmd.ble_msg_id, bleCmd.ble_seq_no);
            pCentral->HandleCharactReport(bleCmd, &msgBuf[6], msgLen);
            break;
        case BLE_CENTRAL_REPORT:
            BLE_DEBUG("BleCentral::CentralComReadTask(): (msg=0x%04X, seq=%d)central info reporting...",
                      bleCmd.ble_msg_id, bleCmd.ble_seq_no);
            pCentral->HandleCentralReport(bleCmd, &msgBuf[6], msgLen);
            break;
        case BLE_USRDATA_REPORT:
            BLE_DEBUG("BleCentral::CentralComReadTask(): (msg=0x%04X, seq=%d)user BLE data reporting...",
                      bleCmd.ble_msg_id, bleCmd.ble_seq_no);
            pCentral->HandleUsrDataReport(bleCmd, &msgBuf[6], msgLen);
            break;
        default:
            BLE_DEBUG("BleCentral::CentralComReadTask(): (msg=0x%04X, seq=%d)not supported message!",
                      bleCmd.ble_msg_id, bleCmd.ble_seq_no);
            break;
        }
    }

    BLE_DEBUG("BleCentral::CentralComReadTask(): thread quit now.");
    return 0;
}

bool BleCentral::HandleGeneralAck(BleCommand &bleCmd, const char *msgBuf, int msgLen)
{
    bleCmd.ble_seq_no = big_uint32_decode((uint8_t *)msgBuf); msgBuf += sizeof(uint32_t);
    bleCmd.ble_msg_id = big_uint16_decode((uint8_t *)msgBuf); msgBuf += sizeof(uint16_t);
    bleCmd.ret_value  = big_uint16_decode((uint8_t *)msgBuf); msgBuf += sizeof(uint16_t);

    // scanning start acknowledged
    if (bleCmd.ble_msg_id == BLE_SCANNING_START)
    {
        // only handling request BLE message
        if (bleCmd.ble_seq_no != mCmdRequest.ble_seq_no)
            return false;

        // message error, retransmit
        mCmdRequest.ret_value = bleCmd.ret_value;
        if (mCmdRequest.ret_value == ACK_RESULT_ERROR)
        {
            BLE_DEBUG("BleCentral::HandleGeneralAck(): message error, retransmit it(msg=0x%04X, seq=%d)!",
                      mCmdRequest.ble_msg_id, mCmdRequest.ble_seq_no);
            return (CentralComWrite(mCmdRequest, (char *)msgBuf, msgLen) > 0);
        }

        // handling acknowledge
        if (mCmdRequest.ret_value == ACK_RESULT_SUCCEED)
        {
            mIsScanning = true;
            mReportTime = time(0);
        }

        BLE_DEBUG("BleCentral::HandleGeneralAck(): start scanning %s!",
                   mIsScanning ? "SUCCESS" : "FAILED");

        // handling finish, acknowledge
        GeneralReporting(mCmdRequest);
        mCmdRequest.ble_seq_no = 0;
        mCmdRequest.net_seq_no = 0;
        mIsHandling = false;
        return true;
    }

    // scanning stop acknowledged
    if (bleCmd.ble_msg_id == BLE_SCANNING_STOP)
    {
        // stop any way
        BLE_DEBUG("BleCentral::HandleGeneralAck(): stop scanning SUCCESS!");
        mIsScanning = false;
        mReportTime = time(0);
        return true;
    }

    // connection acknowledged
    if (bleCmd.ble_msg_id == BLE_BEACON_CONNECT)
    {
        // only handling request BLE message
        if (bleCmd.ble_seq_no != mCmdRequest.ble_seq_no)
            return false;

        // message error, retransmit
        mCmdRequest.ret_value = bleCmd.ret_value;
        if (mCmdRequest.ret_value == ACK_RESULT_ERROR)
        {
            BLE_DEBUG("BleCentral::HandleGeneralAck(): message error, retransmit it(msg=0x%04X, seq=%d)!",
                      mCmdRequest.ble_msg_id, mCmdRequest.ble_seq_no);
            return (CentralComWrite(mCmdRequest, (char *)msgBuf, msgLen) > 0);
        }

        // handling acknowledge
        if (mCmdRequest.ret_value != ACK_RESULT_SUCCEED)
        {
            // connection failed
            BLE_DEBUG("BleCentral::HandleGeneralAck(): connecting to device[%02X:%02X:%02X:%02X:%02X:%02X] failed!",
                      mConnectAddr.addr[0], mConnectAddr.addr[1], mConnectAddr.addr[2],
                      mConnectAddr.addr[3], mConnectAddr.addr[4], mConnectAddr.addr[5]);

            // acknowledge
            GeneralReporting(mCmdRequest);

            // clear connection info
            memset(&mConnectAddr, 0, sizeof(mConnectAddr));
            memset(&mConnectSkey[0], 0, BLE_MAX_SKEY_LEN);
            mConnectUUID = 0xFFFF;
            mCharRWIndex = 0xFFFF;
            mCharRWArray.clear();

            // handling finish
            mCmdRequest.ble_seq_no = 0;
            mCmdRequest.net_seq_no = 0;
            mIsHandling = false;
            return true;
        }
        else
        {
            // sending security key at first
            BLE_DEBUG("BleCentral::HandleGeneralAck(): connected, writing key[%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X]...",
                      mConnectSkey[0],  mConnectSkey[1],  mConnectSkey[2],  mConnectSkey[3],
                      mConnectSkey[4],  mConnectSkey[5],  mConnectSkey[6],  mConnectSkey[7],
                      mConnectSkey[8],  mConnectSkey[9],  mConnectSkey[10], mConnectSkey[11],
                      mConnectSkey[12], mConnectSkey[13], mConnectSkey[14], mConnectSkey[15]);

            // prepare security key characteristic data
            BleCharInfo bleChar;
            bleChar.uuid = mConnectUUID;
            bleChar.type = BLE_TYPE_BYTE_ARRAY;
            bleChar.offs = 0;
            bleChar.size = BLE_MAX_SKEY_LEN;
            memset(bleChar.data, 0, BLE_MAX_DATA_LEN);
            memcpy(bleChar.data, mConnectSkey, BLE_MAX_SKEY_LEN);

            // writing security key characteristic
            mHandleTime  = time(0);
            mCharRWIndex = 0xFFFF;
            return BleSetupChar(mCmdRequest,
                                mConnectAddr,
                                bleChar);
        }
    }

    // disconnection acknowledged
    if (bleCmd.ble_msg_id == BLE_BEACON_DISCONN)
    {
        // only handling request BLE message
        if (bleCmd.ble_seq_no != mCmdRequest.ble_seq_no)
            return false;

        // message error, retransmit
        mCmdRequest.ret_value = bleCmd.ret_value;
        if (mCmdRequest.ret_value == ACK_RESULT_ERROR)
        {
            BLE_DEBUG("BleCentral::HandleGeneralAck(): message error, retransmit it(msg=0x%04X, seq=%d)!",
                      mCmdRequest.ble_msg_id, mCmdRequest.ble_seq_no);
            return (CentralComWrite(mCmdRequest, (char *)msgBuf, msgLen) > 0);
        }

        // handling acknowledge
        if (mCharRWIndex == 0xFFFF)
        {
            // connection terminated
            BLE_DEBUG("BleCentral::HandleGeneralAck(): device[%02X:%02X:%02X:%02X:%02X:%02X] is terminated!",
                      mConnectAddr.addr[0], mConnectAddr.addr[1], mConnectAddr.addr[2],
                      mConnectAddr.addr[3], mConnectAddr.addr[4], mConnectAddr.addr[5]);

            // acknowledge
            mCmdRequest.ret_value = ACK_RESULT_FAILED;
            GeneralReporting(mCmdRequest);

            // clear connection info
            memset(&mConnectAddr, 0, sizeof(mConnectAddr));
            memset(&mConnectSkey[0], 0, BLE_MAX_SKEY_LEN);
            mConnectUUID = 0xFFFF;
            mCharRWIndex = 0xFFFF;
            mCharRWArray.clear();

            // handling finish
            mCmdRequest.ble_seq_no = 0;
            mCmdRequest.net_seq_no = 0;
            mIsHandling = false;
            return true;
        }
        else
        {
            // connection disconnected
            BLE_DEBUG("BleCentral::HandleGeneralAck(): device[%02X:%02X:%02X:%02X:%02X:%02X] is disconnected!",
                      mConnectAddr.addr[0], mConnectAddr.addr[1], mConnectAddr.addr[2],
                      mConnectAddr.addr[3], mConnectAddr.addr[4], mConnectAddr.addr[5]);

            // report query results
            mCmdRequest.ret_value = ACK_RESULT_SUCCEED;
            ReportCharacters(mCmdRequest);

            // clear connection info
            memset(&mConnectAddr, 0, sizeof(mConnectAddr));
            memset(&mConnectSkey[0], 0, BLE_MAX_SKEY_LEN);
            mConnectUUID = 0xFFFF;
            mCharRWIndex = 0xFFFF;
            mCharRWArray.clear();

            // handling finish
            mCmdRequest.ble_seq_no = 0;
            mCmdRequest.net_seq_no = 0;
            mIsHandling = false;
            return true;
        }
    }

    // characteristic query acknowledged
    if (bleCmd.ble_msg_id == BLE_CHARACT_QUERY)
    {
        // only handling request BLE message
        if (bleCmd.ble_seq_no != mCmdRequest.ble_seq_no)
            return false;

        // message error, retransmit
        mCmdRequest.ret_value = bleCmd.ret_value;
        if (mCmdRequest.ret_value == ACK_RESULT_ERROR)
        {
            BLE_DEBUG("BleCentral::HandleGeneralAck(): message error, retransmit it(msg=0x%04X, seq=%d)!",
                      mCmdRequest.ble_msg_id, mCmdRequest.ble_seq_no);
            return (CentralComWrite(mCmdRequest, (char *)msgBuf, msgLen) > 0);
        }

        // handling acknowledge
        if (mCharRWIndex < mCharRWArray.size())
        {
            mCharRWArray[mCharRWIndex].stat = mCmdRequest.ret_value;
            mCharRWIndex++;
        }

        // querying next characterisitcs
        mHandleTime = time(0);
        if (mCharRWIndex < mCharRWArray.size())
        {
            BLE_DEBUG("BleCentral::HandleGeneralAck(): query next characteristic[0x%04X]...",
                      mCharRWArray[mCharRWIndex].uuid);

            // query the next characteristic
            return BleQueryChar(mCmdRequest,
                                mConnectAddr,
                                mCharRWArray[mCharRWIndex]);
        }
        else
        {
            BLE_DEBUG("BleCentral::HandleGeneralAck(): query completed, disconnecting...");

            // query characteristic finished, disconnect
            return BleDisconnect(mCmdRequest,
                                 mConnectAddr);
        }
    }

    // setup characteristic acknowledged
    if (bleCmd.ble_msg_id == BLE_CHARACT_SETUP)
    {
        // only handling request BLE message
        if (bleCmd.ble_seq_no != mCmdRequest.ble_seq_no)
            return false;

        // message error, retransmit
        mCmdRequest.ret_value = bleCmd.ret_value;
        if (mCmdRequest.ret_value == ACK_RESULT_ERROR)
        {
            BLE_DEBUG("BleCentral::HandleGeneralAck(): message error, retransmit it(msg=0x%04X, seq=%d)!",
                      mCmdRequest.ble_msg_id, mCmdRequest.ble_seq_no);
            return (CentralComWrite(mCmdRequest, (char *)msgBuf, msgLen) > 0);
        }

        // handling acknowledge
        if (mCharRWIndex < mCharRWArray.size())
        {
            mCharRWArray[mCharRWIndex].stat = mCmdRequest.ret_value;
            mCharRWIndex++;
        }

        // start writing first characteristic after
        // security key be wrote.
        if (mCharRWIndex == 0xFFFF)
        {
            BLE_DEBUG("BleCentral::HandleGeneralAck(): security key write %s!",
                      (mCmdRequest.ret_value == 0) ? "SUCCESS" : "FAILED");
            mCharRWIndex = 0;
        }

        // query or setup the next characteristic
        mHandleTime = time(0);
        if (mCharRWIndex < mCharRWArray.size())
        {
            // querying the first characteristic
            if (mCmdRequest.net_msg_id == NET_BEACON_QUERY_ACK)
            {
                BLE_DEBUG("BleCentral::HandleGeneralAck(): query next characteristic[0x%04X]...",
                          mCharRWArray[mCharRWIndex].uuid);

                return BleQueryChar(mCmdRequest,
                                    mConnectAddr,
                                    mCharRWArray[mCharRWIndex]);
            }

            // setuping the next characteristic
            if (mCmdRequest.net_msg_id == NET_BEACON_SETUP_ACK)
            {
                BLE_DEBUG("BleCentral::HandleGeneralAck(): setup next characteristic[0x%04X]...",
                          mCharRWArray[mCharRWIndex].uuid);

                return BleSetupChar(mCmdRequest,
                                    mConnectAddr,
                                    mCharRWArray[mCharRWIndex]);
            }
        }
        else
        {
            BLE_DEBUG("BleCentral::HandleGeneralAck(): setup completed, disconnecting...");

            // setup characteristic finished
            return BleDisconnect(mCmdRequest,
                                 mConnectAddr);
        }
    }

    // handling other request message
    if (bleCmd.ble_seq_no == mCmdRequest.ble_seq_no)
    {
        // message error, retransmit
        mCmdRequest.ret_value = bleCmd.ret_value;
        if (mCmdRequest.ret_value == ACK_RESULT_ERROR)
        {
            BLE_DEBUG("BleCentral::HandleGeneralAck(): message error, retransmit it(msg=0x%04X, seq=%d)!",
                      mCmdRequest.ble_msg_id, mCmdRequest.ble_seq_no);
            return (CentralComWrite(mCmdRequest, (char *)msgBuf, msgLen) > 0);
        }
        else
        {
            // handling acknowledge
            GeneralReporting(mCmdRequest);

            // handle finish
            mCmdRequest.ble_seq_no = 0;
            mCmdRequest.net_seq_no = 0;
            mIsHandling = false;
            return true;
        }
    }

    // invalid acknowledge
    return false;
}

bool BleCentral::HandleScannedReport(BleCommand &bleCmd, const char *msgBuf, int msgLen)
{
    // scanning not start, stop it for sure
    if (!mIsScanning || !GetReportEnabled())
    {
        bleCmd.ble_seq_no = mCmdCounter++;
        return BleScanningStop(bleCmd);
    }

    // parsing scanned info
    bleCmd.ble_seq_no = big_uint32_decode((uint8_t *)msgBuf);
    msgBuf += sizeof(uint32_t);
    msgLen -= sizeof(uint32_t);

    BleAddrInfo bleAddr;
    memcpy(bleAddr.addr, msgBuf, BLE_MAX_ADDR_LEN);
    msgBuf += BLE_MAX_ADDR_LEN;
    msgLen -= BLE_MAX_ADDR_LEN;

    bleAddr.type = *msgBuf++;
    msgLen--;

    BleRssiInfo devRssi;
    devRssi.rssi = (int8_t)*msgBuf++;
    devRssi.time = time(0);
    msgLen--;

    uint8_t advType = (uint8_t)*msgBuf++;
    msgLen--;

    // analysis and parsing advertising data
    pthread_mutex_lock(&mAdvertLock);
    for (ADVERT_ITER advIter = mAdvertList.begin();
         advIter!= mAdvertList.end(); advIter++)
    {
        bool        valid = true;
        int         index = 0;
        BleCondInfo advCond;
        BleParaInfo advPara;

        // checking the advertising data is valid
        BleAdvert  *pAdvert = *advIter;
        while (pAdvert->GetCondition(index++, advCond))
        {
            if ((advType!= pAdvert->GetAdvertType())        ||
                (msgLen <= advCond.offs)                    ||
                (msgLen < (advCond.offs + advCond.size))    ||
                memcmp(advCond.data, &msgBuf[advCond.offs], advCond.size))
            {
                valid = false;
                break;
            }
        }

        // unrecognized
        if (!valid)
            continue;

        // get the device
        BleDevice  *pDevice = NULL;
        pthread_mutex_lock(&mDeviceLock);
        for (DEVICE_ITER devIter = mDeviceList.begin();
             devIter!= mDeviceList.end(); devIter++)
        {
            if ((*devIter)->CmpAddress(bleAddr))
            {
                pDevice = *devIter;
                mDeviceList.erase(devIter);
                break;
            }
        }
        pthread_mutex_unlock(&mDeviceLock);

        // create an new device if not exist
        if (!pDevice)
        {
            pDevice = new BleDevice(bleAddr);
            if (!pDevice) return false;
        }

        // add scanned info
        pDevice->AddRssiInfo(devRssi);
        pDevice->AddAdvertUUID(pAdvert->GetAdvertUUID());

        // add device parameters
        index = 0;
        while (pAdvert->GetParameter(index++, advPara))
        {
            if ((msgLen > advPara.offs) &&
                (msgLen >= (advPara.offs + advPara.size)))
            {
                BleScanData advScan;
                memset(&advScan, 0, sizeof(advScan));

                memcpy(advScan.name, advPara.name, BLE_MAX_NAME_LEN);
                advScan.type = advPara.type;
                advScan.size = advPara.size;
                memcpy(advScan.data, &msgBuf[advPara.offs], advPara.size);
                pDevice->AddScanData(advScan);
            }
        }

        // add the device to list
        pthread_mutex_lock(&mDeviceLock);
        mDeviceList.push_back(pDevice);
        pthread_mutex_unlock(&mDeviceLock);
        pthread_mutex_unlock(&mAdvertLock);
        return true;
    }

    // not recognized
    pthread_mutex_unlock(&mAdvertLock);
    return false;
}

bool BleCentral::ReportAllDevices(BleCommand &bleCmd)
{
    DEVICE_ITER devIter;
    BleDevice  *pDevice;

    // making NET acknowledge message
    cJSON *pOutRoot = cJSON_CreateObject();
    cJSON *pOutDevs = cJSON_CreateArray();
    if (!pOutRoot || !pOutDevs)
        return false;

    // appending all devices info
    pthread_mutex_lock(&mDeviceLock);
    for (devIter = mDeviceList.begin();
         devIter!= mDeviceList.end();
         devIter++)
    {
        pDevice = *devIter;
        cJSON *pObject = cJSON_CreateObject();
        cJSON *pRssiArray = cJSON_CreateArray();
        cJSON *pTimeArray = cJSON_CreateArray();
        cJSON *pUUIDArray = cJSON_CreateArray();
        cJSON *pParaArray = cJSON_CreateArray();
        if (!pObject || !pRssiArray || !pTimeArray || 
            !pUUIDArray || !pParaArray)
            continue;

        BleAddrInfo bleAddr;
        pDevice->GetAddress(bleAddr);

        //BLE_DEBUG("BleCentral::ReportAllDevices(): reporting device[%02X:%02X:%02X:%02X:%02X:%02X]...",
        //          bleAddr.addr[0], bleAddr.addr[1], bleAddr.addr[2],
        //          bleAddr.addr[3], bleAddr.addr[4], bleAddr.addr[5]);

        char buffer[64] = {0};
        MsgHex2ASCII(buffer, bleAddr.addr, BLE_MAX_ADDR_LEN);
        cJSON_AddStringToObject(pObject, "ble_addr",  buffer);
        cJSON_AddNumberToObject(pObject, "addr_type", bleAddr.type);

        pDevice->PrintRssiToArray(pRssiArray, pTimeArray);
        pDevice->PrintUUIDToArray(pUUIDArray);
        pDevice->PrintScanToArray(pParaArray);

        cJSON_AddItemToObject(pObject, "scan_rssi", pRssiArray);
        cJSON_AddItemToObject(pObject, "scan_time", pTimeArray);
        cJSON_AddItemToObject(pObject, "scan_uuid", pUUIDArray);
        cJSON_AddItemToObject(pObject, "scan_para", pParaArray);
        cJSON_AddItemToArray(pOutDevs, pObject);
    }
    pthread_mutex_unlock(&mDeviceLock);

    // set sequence no
    cJSON_AddNumberToObject(pOutRoot, "seq_no", bleCmd.net_seq_no);
    cJSON_AddItemToObject(pOutRoot, "devices", pOutDevs);

    // sending NET message
    char *pOutBuff = cJSON_PrintUnformatted(pOutRoot);
    int   nOutSize = CentralNetWrite(bleCmd, pOutBuff, strlen(pOutBuff));

    // do clear work
    if (pOutBuff) free(pOutBuff);
    if (pOutRoot) cJSON_Delete(pOutRoot);
    return (nOutSize > 0);
}

bool BleCentral::PostAllDevices(BleCommand &bleCmd)
{
    DEVICE_ITER devIter;
    BleDevice  *pDevice;

    // making NET acknowledge message
    cJSON *pOutRoot = cJSON_CreateObject();
    cJSON *pOutDevs = cJSON_CreateArray();
    if (!pOutRoot || !pOutDevs)
        return false;

    // add CB info
    cJSON *pCBInfo = cJSON_CreateObject();
    cJSON_AddStringToObject(pCBInfo, "cbid", GetDevSerialNo());
    cJSON_AddNumberToObject(pCBInfo, "time", time(0));
    cJSON_AddNumberToObject(pOutRoot, "seq_no", bleCmd.net_seq_no);
    cJSON_AddItemToObject(pOutRoot, "cloudbeacon", pCBInfo);

    // add devices
    pthread_mutex_lock(&mDeviceLock);
    for (devIter = mDeviceList.begin();
         devIter!= mDeviceList.end();
         devIter++)
    {
        pDevice = *devIter;
        if (!pDevice) continue;

        cJSON *pObject = cJSON_CreateObject();
        if (!pObject) continue;

        BleAddrInfo bleAddr;
        pDevice->GetAddress(bleAddr);

        char buffer[64] = {0};
        MsgHex2ASCII(buffer, bleAddr.addr, BLE_MAX_ADDR_LEN);
        cJSON_AddStringToObject(pObject, "ble_addr",  buffer);
        cJSON_AddNumberToObject(pObject, "addr_type", bleAddr.type);

        BleRssiInfo bleRssi;
        pDevice->GetLastRssi(bleRssi);

        cJSON_AddNumberToObject(pObject, "scan_rssi", (int)bleRssi.rssi);
        cJSON_AddNumberToObject(pObject, "scan_time", bleRssi.time);

        pDevice->PrintScanToObject(pObject);
        cJSON_AddItemToArray(pOutDevs, pObject);
    }
    pthread_mutex_unlock(&mDeviceLock);
    cJSON_AddItemToObject(pOutRoot, "beacons", pOutDevs);

    // sending NET message
    char *pOutBuff = cJSON_PrintUnformatted(pOutRoot);
    int   nOutSize = CentralNetWrite(bleCmd, pOutBuff, strlen(pOutBuff));

    // do clear work
    if (pOutBuff) free(pOutBuff);
    if (pOutRoot) cJSON_Delete(pOutRoot);
    return (nOutSize > 0);
}

void BleCentral::DeleteAllDevices()
{
    pthread_mutex_lock(&mDeviceLock);
    while (!mDeviceList.empty())
    {
        delete mDeviceList.front();
        mDeviceList.pop_front();
    }
    pthread_mutex_unlock(&mDeviceLock);
}

bool BleCentral::HandleCharactReport(BleCommand &bleCmd, const char *msgBuf, int msgLen)
{
    uint16_t     bleUUID;
    BleAddrInfo  bleAddr;

    // parsing characteristic data
    bleCmd.ble_seq_no = big_uint32_decode((uint8_t *)msgBuf);
    msgBuf += sizeof(uint32_t);

    // only handling request BLE message
    if (bleCmd.ble_seq_no != mCmdRequest.ble_seq_no)
        return false;

    // parsing device address
    memcpy(bleAddr.addr, msgBuf, BLE_MAX_ADDR_LEN);
    msgBuf += BLE_MAX_ADDR_LEN;
    bleAddr.type = *msgBuf++;

    // parsing characteristic uuid
    bleUUID = big_uint16_decode((uint8_t *)msgBuf);
    msgBuf += sizeof(uint16_t);

    // checking query characteristic
    if (mCharRWIndex >= mCharRWArray.size() ||
        mCharRWArray[mCharRWIndex].uuid != bleUUID)
        return false;

    // save characteristic data
    mCharRWArray[mCharRWIndex].stat = ACK_RESULT_SUCCEED;
    mCharRWArray[mCharRWIndex].size = *msgBuf++;
    memcpy(mCharRWArray[mCharRWIndex].data,
           msgBuf,
           mCharRWArray[mCharRWIndex].size);

    BLE_DEBUG("BleCentral::HandleCharactReport(): character uuid=0x%04X, type=%d, offs=%d, size=%d",
              mCharRWArray[mCharRWIndex].uuid,
              mCharRWArray[mCharRWIndex].type,
              mCharRWArray[mCharRWIndex].offs,
              mCharRWArray[mCharRWIndex].size);

    // query next characteristic
    mCharRWIndex++;
    mHandleTime = time(0);
    if (mCharRWIndex < mCharRWArray.size())
    {
        BLE_DEBUG("BleCentral::HandleCharactReport(): query next characteristic[0x%04X]...",
                  mCharRWArray[mCharRWIndex].uuid);

        // query the next characteristic
        return BleQueryChar(mCmdRequest,
                            mConnectAddr,
                            mCharRWArray[mCharRWIndex]);
    }
    else
    {
        BLE_DEBUG("BleCentral::HandleCharactReport(): query completed, disconnecting...");

        // query characteristic finished
        return BleDisconnect(mCmdRequest,
                             mConnectAddr);
    }
}

bool BleCentral::ReportCharacters(BleCommand &bleCmd)
{
    // making NET acknowledge message
    cJSON *pOutRoot = cJSON_CreateObject();
    cJSON *pCharLst = cJSON_CreateArray();
    cJSON_AddNumberToObject(pOutRoot, "ack_no", bleCmd.net_seq_no);
    cJSON_AddNumberToObject(pOutRoot, "ret_no", bleCmd.ret_value);

    // appending BLE address
    char buffer[64] = {0};
    MsgHex2ASCII(buffer, mConnectAddr.addr, BLE_MAX_ADDR_LEN);
    cJSON_AddStringToObject(pOutRoot, "ble_addr",  buffer);
    cJSON_AddNumberToObject(pOutRoot, "addr_type", mConnectAddr.type);

    // appending characteristic list
    cJSON_AddItemToObject(pOutRoot, "char_list", pCharLst);
    for (size_t i = 0; i < mCharRWArray.size(); i++)
    {
        cJSON *pCharact = cJSON_CreateObject();
        cJSON_AddNumberToObject(pCharact, "stat", mCharRWArray[i].stat);
        cJSON_AddNumberToObject(pCharact, "uuid", mCharRWArray[i].uuid);
        cJSON_AddItemToArray(pCharLst, pCharact);

        if ((bleCmd.net_msg_id == NET_BEACON_QUERY_ACK) &&
            (mCharRWArray[i].stat == ACK_RESULT_SUCCEED))
        {
            cJSON_AddNumberToObject(pCharact, "type", mCharRWArray[i].type);
            cJSON_AddNumberToObject(pCharact, "size", mCharRWArray[i].size);

            if (mCharRWArray[i].type != BLE_TYPE_STRING)
            {
                MsgHex2ASCII(buffer, mCharRWArray[i].data, mCharRWArray[i].size);
                cJSON_AddStringToObject(pCharact, "data", buffer);
            }
            else
            {
                strcpy(buffer, string_check((char *)mCharRWArray[i].data));
                cJSON_AddStringToObject(pCharact, "data", buffer);
            }
        }
    }

    // sending NET message
    char *pOutBuff = cJSON_PrintUnformatted(pOutRoot);
    int   nOutSize = CentralNetWrite(bleCmd, pOutBuff, strlen(pOutBuff));

    // do clear work
    if (pOutBuff) free(pOutBuff);
    if (pOutRoot) cJSON_Delete(pOutRoot);
    return (nOutSize > 0);
}

bool BleCentral::HandleCentralReport(BleCommand &bleCmd, const char *msgBuf, int msgLen)
{
    // get sequence number
    bleCmd.ble_seq_no = big_uint32_decode((uint8_t *)msgBuf);
    msgBuf += sizeof(uint32_t);

    // update central MAC address
    memcpy(mCentralAddr.addr, msgBuf, BLE_MAX_ADDR_LEN);
    msgBuf += BLE_MAX_ADDR_LEN;
    mCentralAddr.type = 1;

    // get central info
    BleChipInfo bleChip;
    memset(&bleChip, 0, sizeof(bleChip));

    memcpy(bleChip.uuid, msgBuf, BLE_MAX_UUID_LEN);
    msgBuf += BLE_MAX_UUID_LEN;

    bleChip.major = big_uint16_decode((uint8_t *)msgBuf); msgBuf += sizeof(uint16_t);
    bleChip.minor = big_uint16_decode((uint8_t *)msgBuf); msgBuf += sizeof(uint16_t);
    bleChip.ms_power = *msgBuf++;
    bleChip.tx_power = *msgBuf++;
    mCentralType = big_uint16_decode((uint8_t *)msgBuf); msgBuf += sizeof(uint16_t);
    mFirmwareVer = big_uint16_decode((uint8_t *)msgBuf); msgBuf += sizeof(uint16_t);
    bleChip.mode = big_uint16_decode((uint8_t *)msgBuf); msgBuf += sizeof(uint16_t);

    int power_level = *msgBuf++;
    int temperature = *msgBuf++;

    bleChip.adv_interval = big_uint16_decode((uint8_t *)msgBuf); msgBuf += sizeof(uint16_t);
    bleChip.bat_interval = big_uint16_decode((uint8_t *)msgBuf); msgBuf += sizeof(uint16_t);
    bleChip.tmp_interval = big_uint16_decode((uint8_t *)msgBuf); msgBuf += sizeof(uint16_t);
    bleChip.asw_interval = big_uint16_decode((uint8_t *)msgBuf); msgBuf += sizeof(uint16_t);

    memcpy(bleChip.eurl, msgBuf, BLE_MAX_EURL_LEN);
    msgBuf += BLE_MAX_EURL_LEN;
    bleChip.eurl[BLE_MAX_EURL_LEN] = *msgBuf++;

    memcpy(bleChip.name, msgBuf, BLE_MAX_NAME_LEN);
    msgBuf += BLE_MAX_NAME_LEN;

    // only handling request BLE message
    if (mCmdRequest.ble_seq_no != bleCmd.ble_seq_no)
        return false;

    // making NET acknowledge message
    cJSON *pOutRoot = cJSON_CreateObject();
    cJSON_AddNumberToObject(pOutRoot, "ack_no", mCmdRequest.net_seq_no);
    cJSON_AddNumberToObject(pOutRoot, "ret_no", ACK_RESULT_SUCCEED);

    char buffer[64] = {0};
    MsgHex2ASCII(buffer, mCentralAddr.addr, BLE_MAX_ADDR_LEN);
    cJSON_AddStringToObject(pOutRoot, "ble_addr", buffer);

    MsgHex2ASCII(buffer, bleChip.uuid, BLE_MAX_UUID_LEN);
    cJSON_AddStringToObject(pOutRoot, "ble_uuid", buffer);

    cJSON_AddNumberToObject(pOutRoot, "major", bleChip.major);
    cJSON_AddNumberToObject(pOutRoot, "minor", bleChip.minor);
    cJSON_AddNumberToObject(pOutRoot, "ms_power", bleChip.ms_power);
    cJSON_AddNumberToObject(pOutRoot, "tx_power", bleChip.tx_power);

    cJSON_AddNumberToObject(pOutRoot, "fw_type",    mCentralType);
    cJSON_AddNumberToObject(pOutRoot, "fw_version", mFirmwareVer);
    cJSON_AddNumberToObject(pOutRoot, "dev_mode",   bleChip.mode);

    cJSON_AddNumberToObject(pOutRoot, "power_level", power_level);
    cJSON_AddNumberToObject(pOutRoot, "temperature", temperature);

    cJSON_AddNumberToObject(pOutRoot, "adv_interval",        bleChip.adv_interval);
    cJSON_AddNumberToObject(pOutRoot, "temp_check_interval", bleChip.tmp_interval);
    cJSON_AddNumberToObject(pOutRoot, "batt_check_interval", bleChip.bat_interval);
    cJSON_AddNumberToObject(pOutRoot, "adv_switch_interval", bleChip.asw_interval);

    int length = bleChip.eurl[BLE_MAX_EURL_LEN];
    Hex2EddystoneURL(buffer, bleChip.eurl, length);
    cJSON_AddStringToObject(pOutRoot, "eddystone_url", buffer);

    strcpy(buffer, string_check((char *)bleChip.name));
    cJSON_AddStringToObject(pOutRoot, "ble_name", buffer);

    // sending NET message
    char *pOutBuff = cJSON_PrintUnformatted(pOutRoot);
    int   nOutSize = CentralNetWrite(mCmdRequest, pOutBuff, strlen(pOutBuff));

    // do clear work
    if (pOutBuff) free(pOutBuff);
    if (pOutRoot) cJSON_Delete(pOutRoot);

    // handling finish
    mIsHandling = false;
    return (nOutSize > 0);
}

static uint8_t gUserDataBuf[USR_MAX_BUFFER_LEN] = {0};
bool BleCentral::HandleUsrDataReport(BleCommand &bleCmd, const char *msgBuf, int msgLen)
{
    uint8_t ackLen = 0;
    uint8_t ackBuf[BLE_MAX_DATA_LEN];

    // checking the length
    msgLen = (uint8_t)*msgBuf++;
    if (msgLen < 4) return false;

    // get user message header
    uint8_t msgSeq = (uint8_t)*msgBuf++;
    uint8_t msgOpt = (uint8_t)*msgBuf++;
    uint8_t usrMsg = (uint8_t)*msgBuf++;
    uint8_t msgNum = (uint8_t)*msgBuf++;

    uint16_t length = msgOpt & USR_MSG_LENGTH_MASK;
    uint16_t offset = msgNum * USR_MAX_PACKET_LEN;

    if ((length < 4) || (length > BLE_MAX_DATA_LEN))
        return false;
    if (offset > (USR_MAX_BUFFER_LEN - USR_MAX_PACKET_LEN))
        return false;

    // saving the message data
    length -= 4;
    memset(&gUserDataBuf[offset],      0, length);
    memcpy(&gUserDataBuf[offset], msgBuf, length);

    // acknowledge
    if (!(msgOpt & USR_MSG_OPTION_END_PACKAGE))
    {
        ackBuf[ackLen++] = msgSeq;
        ackBuf[ackLen++] = USR_MSG_OPTION_END_PACKAGE;
        ackBuf[ackLen++] = ACK_RESULT_SUCCEED;
        ackBuf[ackLen++] = msgNum;
        ackBuf[1]       |= ackLen;
        BleSetupUserData(bleCmd, ackBuf, ackLen);
        return true;
    }

    // setup WIFI STA/AP info
    // EN(1)+CH(1)+SEC(1)+ENCRYPT(1)+ssid('\0')+key('\0')
    if ((usrMsg == USR_MSG_SETUP_STA_INFO) || 
        (usrMsg == USR_MSG_SETUP_AP_INFO))
    {
        WiFiInfo wi;
        memset(&wi, 0, sizeof(WiFiInfo));

        offset = 0;
        wi.enable     = gUserDataBuf[offset++];
        wi.channel    = gUserDataBuf[offset++];
        wi.secMode    = gUserDataBuf[offset++];
        wi.encrypType = gUserDataBuf[offset++];

        strncpy(wi.ssid, (char *)&gUserDataBuf[offset], 128);
        offset += strlen(wi.ssid) + 1;
        strncpy(wi.key, (char *)&gUserDataBuf[offset], 128);
        offset += strlen(wi.key) + 1;

        ackBuf[ackLen++] = msgSeq;
        ackBuf[ackLen++] = USR_MSG_OPTION_END_PACKAGE;

        if (usrMsg == USR_MSG_SETUP_STA_INFO)
        {
            ackBuf[ackLen++] = Set_StaInfo(&wi) 
                             ? ACK_RESULT_FAILED
                             : ACK_RESULT_SUCCEED;
        }
        else
        {
            ackBuf[ackLen++] = Set_APInfo(&wi) 
                             ? ACK_RESULT_FAILED
                             : ACK_RESULT_SUCCEED;
        }

        ackBuf[ackLen++] = msgNum;
        ackBuf[1]       |= ackLen;
        BleSetupUserData(bleCmd, ackBuf, ackLen);
        return true;
    }

    // get WIFI STA/AP info
    if ((usrMsg == USR_MSG_QUERY_STA_INFO) ||
        (usrMsg == USR_MSG_QUERY_AP_INFO))
    {
        WiFiInfo wi;
        memset(&wi, 0, sizeof(WiFiInfo));

        if (usrMsg == USR_MSG_QUERY_STA_INFO)
            Get_StaInfo(&wi);
        else
            Get_APInfo(&wi);

        length = 0;
        gUserDataBuf[length++] = wi.enable ? 1 : 0;
        gUserDataBuf[length++] = (uint8_t)wi.channel;
        gUserDataBuf[length++] = (uint8_t)wi.secMode;
        gUserDataBuf[length++] = (uint8_t)wi.encrypType;

        strcpy((char *)&gUserDataBuf[length], wi.ssid);
        length += strlen(wi.ssid) + 1;
        strcpy((char *)&gUserDataBuf[length], wi.key);
        length += strlen(wi.key) + 1;

        offset = 0;
        while (offset < length)
        {
            ackLen = 0;
            ackBuf[ackLen++] = msgSeq;

            if (length < (offset + USR_MAX_PACKET_LEN))
            {
                msgLen = length - offset;
                ackBuf[ackLen++] = USR_MSG_OPTION_END_PACKAGE;
            }
            else
            {
                msgLen = USR_MAX_PACKET_LEN;
                ackBuf[ackLen++] = 0;
            }

            ackBuf[ackLen++] = ACK_RESULT_SUCCEED;
            ackBuf[ackLen++] = msgNum;

            memcpy(&ackBuf[ackLen],
                   &gUserDataBuf[offset],
                   msgLen);
            ackLen += msgLen;
            offset += msgLen;

            ackBuf[1] |= ackLen;
            BleSetupUserData(bleCmd, ackBuf, ackLen);
            SLEEP(10);
        }

        return true;
    }

    // invalid message
    return false;
}

int BleCentral::UpdateFirmware(const char *pUpdatePath)
{
#define DFU_HEADER_WAIT_TIMEOUT         5000
#define DFU_PACKET_WAIT_TIMEOUT         1000
#define DFU_HEADER_RETRY_COUNT          3
#define DFU_PACKET_RETRY_COUNT          5

    FILE        *fp = NULL;
    uint8_t     dfuBuf[DFU_MAX_BUFFER_LEN];
    uint8_t     msgBuf[COM_MAX_BUF_SIZE];
    uint8_t     dfuLen = 0;
    int         totLen = 0;
    int         pktNum = 0;
    int         totNum = 0;
    time_t      timeout= 0;
    int         retries= 0;
    BleCommand  bleCmd;

    // stop central at first
    BLE_DEBUG("BleCentral::UpdateFirmware(): stopping central...");
    Stop();
    SLEEP(500);

    bleCmd.ble_seq_no = mCmdCounter++;
    bleCmd.ret_value  = ACK_RESULT_SUCCEED;

    // open firmware file
    BLE_DEBUG("BleCentral::UpdateFirmware(): loading updated firmware file...");
    fp = fopen(pUpdatePath, "r");
    if (!fp)
    {
        BLE_DEBUG("BleCentral::UpdateFirmware(): open file %s failed!", pUpdatePath);
        bleCmd.ret_value = ACK_RESULT_FAILED;
        goto BLE_RESTART;
    }

    // get total file length, and calculate the total packet number
    fseek(fp, 0, SEEK_END);
    totLen = ftell(fp);
    rewind(fp);

    pktNum = 0;
    totNum = totLen / DFU_MAX_PACKET_LEN;
    if (totLen % DFU_MAX_PACKET_LEN)
        totNum += 1;

    BLE_DEBUG("BleCentral::UpdateFirmware(): start updating(length=%d, packets=%d)...",
              totLen, totNum);

    // making DFU header
    dfuLen = 0;
    memset(dfuBuf, 0, DFU_MAX_BUFFER_LEN);
    dfuLen+= big_uint16_encode((uint16_t)pktNum, &dfuBuf[dfuLen]);      // packet number
    dfuLen+= big_uint16_encode(0,                &dfuBuf[dfuLen]);      // firmware type
    dfuLen+= big_uint16_encode(0,                &dfuBuf[dfuLen]);      // firmware version
    dfuLen+= big_uint32_encode((uint16_t)totLen, &dfuBuf[dfuLen]);      // file size

    // writing DFU header
    retries = 1;
    while (1)
    {
        // send command, waiting acknowledge
        timeout = 0;
        BleUpdateCentral(bleCmd, dfuBuf, DFU_MAX_BUFFER_LEN);
        while (timeout < DFU_HEADER_WAIT_TIMEOUT)
        {
            // make sure got update acknowledge
            if ((CentralComRead(bleCmd, (char *)msgBuf, COM_MAX_BUF_SIZE) <= 0) ||
                (BLE_GENERAL_ACK    != big_uint16_decode(&msgBuf[0]))           ||
                (BLE_CENTRAL_UPDATE != big_uint16_decode(&msgBuf[10])))
            {
                timeout += 20;
                SLEEP(20);
                continue;
            }

            // invalid state, BLE is connected as peripheral
            bleCmd.ret_value  = big_uint16_decode(&msgBuf[12]);
            if (bleCmd.ret_value == ACK_RESULT_INVALID_STATE)
            {
                BLE_DEBUG("BleCentral::UpdateFirmware(): BLE connected, updating failed!");
                fclose(fp);
                goto BLE_RESTART;
            }

            // OK, the DFU header write success
            if (bleCmd.ret_value == ACK_RESULT_SUCCEED)
            {
                timeout = 0;
                retries = 1;
                break;
            }
        }

        // got DFU header acknowledge
        if (timeout < DFU_HEADER_WAIT_TIMEOUT)
            break;

        // waiting DFU header acknowledge timeout, retry it
        BLE_DEBUG("BleCentral::UpdateFirmware(): waiting DFU header acknowledge timeout!");
        if (++retries > DFU_HEADER_RETRY_COUNT)
        {
            BLE_DEBUG("BleCentral::UpdateFirmware(): writing DFU header[%d/%d] failed!",
                      pktNum, totNum);
            fclose(fp);
            bleCmd.ret_value = ACK_RESULT_FAILED;
            goto BLE_RESTART;
        }
    }

    // making the first DFU packet
    pktNum  = 1;
    dfuLen  = 0;
    memset(dfuBuf, 0, DFU_MAX_BUFFER_LEN);
    dfuLen+= big_uint16_encode((uint16_t)pktNum, &dfuBuf[dfuLen]);      // packet number
    dfuLen+= big_uint16_encode((uint16_t)totNum, &dfuBuf[dfuLen]);      // total packets
    dfuLen+= fread(&dfuBuf[dfuLen], 1, DFU_MAX_PACKET_LEN, fp);         // packet data

    // write firmware data
    retries = 1;
    while (pktNum <= totNum)
    {
        // send command, waiting acknowledge
        timeout = 0;
        BleUpdateCentral(bleCmd, dfuBuf, DFU_MAX_BUFFER_LEN);
        while (timeout < DFU_PACKET_WAIT_TIMEOUT)
        {
            // make sure got update acknowledge
            if ((CentralComRead(bleCmd, (char *)msgBuf, COM_MAX_BUF_SIZE) <= 0) ||
                (BLE_GENERAL_ACK    != big_uint16_decode(&msgBuf[0]))           ||
                (BLE_CENTRAL_UPDATE != big_uint16_decode(&msgBuf[10])))
            {
                timeout += 20;
                SLEEP(20);
                continue;
            }

            // invalid state, BLE is connected as peripheral
            bleCmd.ret_value  = big_uint16_decode(&msgBuf[12]);
            if (bleCmd.ret_value == ACK_RESULT_INVALID_STATE)
            {
                BLE_DEBUG("BleCentral::UpdateFirmware(): BLE connected, updating failed!");
                fclose(fp);
                goto BLE_RESTART;
            }

            // OK, the DFU packet write success, prepare next one
            if (bleCmd.ret_value == ACK_RESULT_SUCCEED)
            {
                pktNum++;
                dfuLen = 0;
                memset(dfuBuf, 0, DFU_MAX_BUFFER_LEN);
                dfuLen+= big_uint16_encode((uint16_t)pktNum, &dfuBuf[dfuLen]);  // packet number
                dfuLen+= big_uint16_encode((uint16_t)totNum, &dfuBuf[dfuLen]);  // total packets
                dfuLen+= fread(&dfuBuf[dfuLen], 1, DFU_MAX_PACKET_LEN, fp);     // packet data

                retries = 1;
                break;
            }
        }

        // waiting DFU packet timeout, retry it
        if (timeout >= DFU_PACKET_WAIT_TIMEOUT)
        {
            BLE_DEBUG("BleCentral::UpdateFirmware(): waiting DFU packet[%d/%d] acknowledge timeout!",
                      pktNum, totNum);
            if (++retries > DFU_PACKET_RETRY_COUNT)
            {
                BLE_DEBUG("BleCentral::UpdateFirmware(): writing DFU packet[%d/%d] failed!",
                          pktNum, totNum);
                fclose(fp);
                bleCmd.ret_value = ACK_RESULT_FAILED;
                goto BLE_RESTART;
            }
        }
    }

    // sending RESET command to BLE central
    fclose(fp);
    while (1)
    {
        BLE_DEBUG("BleCentral::UpdateFirmware(): start rebooting...");
        BleResetCentral(bleCmd);
        SLEEP(2000);
    }

    // ------------------------------------------
    // power will be TURN OFF by BLE central!!
    // ------------------------------------------

BLE_RESTART:

    // restart central
    BLE_DEBUG("BleCentral::UpdateFirmware(): restarting BLE central...");
    if (!Start())
    {
        BLE_DEBUG("BleCentral::UpdateFirmware(): restart BLE central failed!");
        return ACK_RESULT_FAILED;
    }

    // reload scanning configure file
    BLE_DEBUG("BleCentral::UpdateFirmware(): reloading scanning configure file...");
    if (!ReloadConfig(mConfigPath.c_str()))
    {
        BLE_DEBUG("BleCentral::UpdateFirmware(): scanning configure file load failed!");
        return ACK_RESULT_FAILED;
    }

    // resume scanning
    return ResumeScanning();
}
