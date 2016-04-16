//=============================================================================
// FILE  : ble_advert.h
// DESC  :
// AUTHOR: PINBO
// CREATE: 2015-06-02
// MODIFY:
//=============================================================================

#ifndef __BLE_ADVERT_H__
#define __BLE_ADVERT_H__

#include <stdint.h>
#include <stdbool.h>
#include <vector>
#include <string>
#include "ble_defines.h"


//=============================================================================
//=============================================================================

// Advertising data types
#define BLE_ADV_TYPE_ADVERTIS           0
#define BLE_ADV_TYPE_SCAN_RSP           1

//=============================================================================
//=============================================================================
struct BleCondInfo
{
    uint8_t     type;
    uint8_t     offs;
    uint8_t     size;
    uint8_t     data[BLE_MAX_DATA_LEN+1];
};

struct BleParaInfo
{
    uint8_t     name[BLE_MAX_NAME_LEN+1];
    uint8_t     type;
    uint8_t     offs;
    uint8_t     size;
};

//=============================================================================
//=============================================================================
class BleAdvert
{
protected:
    typedef std::vector<BleCondInfo>            COND_ARRAY;
    typedef std::vector<BleCondInfo>::iterator  COND_ITER;

    typedef std::vector<BleParaInfo>            PARA_ARRAY;
    typedef std::vector<BleParaInfo>::iterator  PARA_ITER;

    COND_ARRAY      mCondArray;
    PARA_ARRAY      mParaArray;

    uint8_t         mAdvType;
    uint16_t        mAdvUUID;

public:
    BleAdvert();
    virtual ~BleAdvert();

    uint8_t GetAdvertType();
    uint8_t SetAdvertType(uint8_t type);

    uint16_t GetAdvertUUID();
    uint16_t SetAdvertUUID(uint16_t uuid);

    bool   AddCondition(BleCondInfo &cond);
    bool   GetCondition(size_t index, BleCondInfo &cond);
    size_t GetConditionCount();
    void   ClearAllConditions();

    bool   AddParameter(BleParaInfo &para);
    bool   GetParameter(size_t index, BleParaInfo &para);
    size_t GetParameterCount();
    void   ClearAllParameters();
};

//=============================================================================
#endif /* __BLE_ADVERT_H__ */

