//=============================================================================
// FILE  : ble_device.h
// DESC  :
// AUTHOR: PINBO
// CREATE: 2015-06-02
// MODIFY:
//=============================================================================

#ifndef __BLE_DEVICE_H__
#define __BLE_DEVICE_H__


#include <stdint.h>
#include <stdbool.h>
#include <string>
#include <vector>
#include <map>

#include "ble_defines.h"
#include "cJSON.h"


//=============================================================================
//=============================================================================
// Parameter data types
#define BLE_TYPE_INVALID                0
#define BLE_TYPE_UNSIGNED_NUMBER        1
#define BLE_TYPE_SIGNED_NUMBER          2
#define BLE_TYPE_FLOAT_NUMBER           3
#define BLE_TYPE_BCD_ARRAY              4
#define BLE_TYPE_BYTE_ARRAY             5
#define BLE_TYPE_STRING                 6

//=============================================================================
//=============================================================================
struct BleAddrInfo
{
    uint8_t     type;
    uint8_t     addr[BLE_MAX_ADDR_LEN];
};

struct BleRssiInfo
{
    int8_t      rssi;
    time_t      time;
};

struct BleCharInfo
{
    int16_t     stat;
    uint16_t    uuid;
    uint8_t     type;
    uint8_t     offs;
    uint8_t     size;
    uint8_t     data[BLE_MAX_DATA_LEN+1];
};

struct BleScanData
{
    uint8_t     type;
    uint8_t     size;
    uint8_t     data[BLE_MAX_DATA_LEN+1];
    uint8_t     name[BLE_MAX_NAME_LEN+1];
};


//=============================================================================
//=============================================================================
class BleDevice
{
protected:
    typedef std::vector<BleRssiInfo>                        RSSI_LIST;
    typedef std::vector<BleRssiInfo>::iterator              RSSI_ITER;

    typedef std::pair<uint16_t, uint16_t>                   UUID_PAIR;
    typedef std::map<uint16_t,  uint16_t>                   UUID_LIST;
    typedef std::map<uint16_t,  uint16_t>::iterator         UUID_ITER;

    typedef std::pair<uint16_t, BleCharInfo>                CHAR_PAIR;
    typedef std::map<uint16_t,  BleCharInfo>                CHAR_LIST;
    typedef std::map<uint16_t,  BleCharInfo>::iterator      CHAR_ITER;

    typedef std::pair<std::string, BleScanData>             SCAN_PAIR;
    typedef std::map<std::string,  BleScanData>             SCAN_LIST;
    typedef std::map<std::string,  BleScanData>::iterator   SCAN_ITER;

    BleAddrInfo     mMacAddr;
    RSSI_LIST       mRssiVec;
    UUID_LIST       mUUIDMap;
    CHAR_LIST       mCharMap;
    SCAN_LIST       mScanMap;

public:
    BleDevice();
    BleDevice(BleAddrInfo &addr);
    virtual ~BleDevice();

public:
    void SetAddress(BleAddrInfo &addr);
    void GetAddress(BleAddrInfo &addr);
    bool CmpAddress(BleAddrInfo &addr);

    bool AddRssiInfo(BleRssiInfo &info);
    int  GetRssiCount();
    void ClearAllRssis();
    bool GetLastRssi(BleRssiInfo &info);

    bool AddAdvertUUID(uint16_t uuid);
    int  GetUUIDCount();
    void ClearAllUUIDs();

    bool AddCharInfo(BleCharInfo &info);
    int  GetCharCount();
    void ClearAllChars();

    bool AddScanData(BleScanData &info);
    int  GetScanCount();
    void ClearAllScans();

    bool PrintRssiToArray(cJSON *pRssiArray, cJSON *pTimeArray);
    bool PrintUUIDToArray(cJSON *pUUIDArray);
    bool PrintCharToArray(cJSON *pCharArray);
    bool PrintScanToArray(cJSON *pScanArray);
    bool PrintScanToObject(cJSON *pObject);
};

//=============================================================================
#endif /* __BLE_DEVICE_H__ */
