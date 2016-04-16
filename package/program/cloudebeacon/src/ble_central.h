//=============================================================================
// FILE  : ble_central.h
// DESC  :
// AUTHOR: PINBO
// CREATE: 2015-06-02
// MODIFY:
//=============================================================================

#ifndef __BLE_CENTRAL_H__
#define __BLE_CENTRAL_H__

#include <stdint.h>
#include <pthread.h>
#include <list>
#include <vector>
#include <string>

#include "ble_defines.h"
#include "ble_advert.h"
#include "ble_device.h"

//=============================================================================
//=============================================================================
struct BleChipInfo
{
    uint8_t     uuid[BLE_MAX_UUID_LEN+1];
    uint8_t     skey[BLE_MAX_SKEY_LEN+1];
    uint16_t    major;
    uint16_t    minor;
    uint16_t    mode;
    int8_t      ms_power;
    int8_t      tx_power;
    uint16_t    adv_interval;
    uint16_t    bat_interval;
    uint16_t    tmp_interval;
    uint16_t    asw_interval;
    uint8_t     eurl[BLE_MAX_EURL_LEN+1];
    uint8_t     name[BLE_MAX_NAME_LEN+1];
};

//=============================================================================
//=============================================================================
class BleCentral
{
protected:
    typedef std::list<BleAdvert*>                       ADVERT_LIST;
    typedef std::list<BleAdvert*>::iterator             ADVERT_ITER;

    typedef std::list<BleDevice*>                       DEVICE_LIST;
    typedef std::list<BleDevice*>::iterator             DEVICE_ITER;

    typedef std::vector<BleCharInfo>                    CHAR_LIST;
    typedef std::vector<BleCharInfo>::iterator          CHAR_ITER;

    BleAddrInfo     mCentralAddr;
    uint16_t        mCentralType;
    uint16_t        mFirmwareVer;

    std::string     mConfigPath;
    std::string     mConfigVer;
    ADVERT_LIST     mAdvertList;
    pthread_mutex_t mAdvertLock;

    pthread_mutex_t mDeviceLock;
    DEVICE_LIST     mDeviceList;

    BleCommand      mCmdRequest;
    uint32_t        mCmdCounter;
    bool            mIsHandling;
    time_t          mHandleTime;

    bool            mIsScanning;
    time_t          mReportTime;

    BleAddrInfo     mConnectAddr;
    uint16_t        mConnectUUID;
    uint8_t         mConnectSkey[BLE_MAX_SKEY_LEN];
    size_t          mCharRWIndex;
    CHAR_LIST       mCharRWArray;

    pthread_t       mNetRdThread;
    pthread_t       mComRdThread;
    bool            mNetRdRunning;
    bool            mComRdRunning;

public:
    BleCentral();
    virtual ~BleCentral();

public:
    bool GetBLEAddress(BleAddrInfo &bleAddr);
    uint16_t GetCentralType();
    uint16_t GetFirmwareVer();

    bool ReloadConfig(const char *pConfigPath);
    void UnloadConfig();

    bool Start();
    void Stop();

    bool ResumeScanning();
    int  UpdateFirmware(const char *pUpdatePath);

public:
    bool BleResetCentral(BleCommand &bleCmd);
    bool BleScanningStart(BleCommand &bleCmd);
    bool BleScanningStop(BleCommand &bleCmd);
    bool BleConnect(BleCommand &bleCmd, BleAddrInfo &bleAddr);
    bool BleDisconnect(BleCommand &bleCmd, BleAddrInfo &bleAddr);
    bool BleQueryChar(BleCommand &bleCmd, BleAddrInfo &bleAddr, BleCharInfo &bleChar);
    bool BleSetupChar(BleCommand &bleCmd, BleAddrInfo &bleAddr, BleCharInfo &bleChar);
    bool BleQueryCentral(BleCommand &bleCmd);
    bool BleSetupCentral(BleCommand &bleCmd, BleChipInfo &bleChip);
    bool BleUpdateCentral(BleCommand &bleCmd, uint8_t *data, uint8_t len);
    bool BleSetupUserData(BleCommand &bleCmd, uint8_t *data, uint8_t len);

protected:
    static void* CentralNetReadTask(void *arg);
    bool HandleQueryScanCfg(BleCommand &bleCmd, const char *msgBuf, int msgLen);
    bool HandleScanningStart(BleCommand &bleCmd, const char *msgBuf, int msgLen);
    bool HandleScanningStop(BleCommand &bleCmd, const char *msgBuf, int msgLen);
    bool HandleQueryBeacon(BleCommand &bleCmd, const char *msgBuf, int msgLen);
    bool HandleSetupBeacon(BleCommand &bleCmd, const char *msgBuf, int msgLen);
    bool HandleQueryCentral(BleCommand &bleCmd, const char *msgBuf, int msgLen);
    bool HandleSetupCentral(BleCommand &bleCmd, const char *msgBuf, int msgLen);

    bool GeneralReporting(BleCommand &bleCmd);
    bool ReportCharacters(BleCommand &bleCmd);
    bool ReportAllDevices(BleCommand &bleCmd);
    bool PostAllDevices(BleCommand &bleCmd);
    void DeleteAllDevices();

protected:
    static void* CentralComReadTask(void *arg);
    bool HandleGeneralAck(BleCommand &bleCmd, const char *msgBuf, int msgLen);
    bool HandleScannedReport(BleCommand &bleCmd, const char *msgBuf, int msgLen);
    bool HandleCharactReport(BleCommand &bleCmd, const char *msgBuf, int msgLen);
    bool HandleCentralReport(BleCommand &bleCmd, const char *msgBuf, int msgLen);
    bool HandleUsrDataReport(BleCommand &bleCmd, const char *msgBuf, int msgLen);

};

//=============================================================================
#endif /* __BLE_CENTRAL_H__ */
