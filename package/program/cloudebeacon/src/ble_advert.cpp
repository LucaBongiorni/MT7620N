//=============================================================================
// FILE  : ble_advert.cpp
// DESC  :
// AUTHOR: PINBO
// CREATE: 2015-06-02
// MODIFY:
//=============================================================================

#include <stdint.h>
#include <stdbool.h>

#include "ble_defines.h"
#include "ble_advert.h"


//=============================================================================
//=============================================================================
BleAdvert::BleAdvert()
{
    mAdvType = BLE_ADV_TYPE_ADVERTIS;
    mAdvUUID = 0;
    mCondArray.clear();
    mParaArray.clear();
}

BleAdvert::~BleAdvert()
{

}

uint8_t BleAdvert::GetAdvertType()
{
    return mAdvType;
}

uint8_t BleAdvert::SetAdvertType(uint8_t type)
{
    uint8_t old = mAdvType;
    return (mAdvType = type), old;
}

uint16_t BleAdvert::GetAdvertUUID()
{
    return mAdvUUID;
}

uint16_t BleAdvert::SetAdvertUUID(uint16_t uuid)
{
    uint8_t old = mAdvUUID;
    return (mAdvUUID = uuid), old;
}

bool BleAdvert::AddCondition(BleCondInfo &cond)
{
    mCondArray.push_back(cond);
    return true;
}

bool BleAdvert::GetCondition(size_t index, BleCondInfo &cond)
{
    if (index >= mCondArray.size())
        return false;

    cond = mCondArray[index];
    return true;
}

size_t BleAdvert::GetConditionCount()
{
    return mCondArray.size();
}

void BleAdvert::ClearAllConditions()
{
    mCondArray.clear();
}

bool BleAdvert::AddParameter(BleParaInfo &para)
{
    mParaArray.push_back(para);
    return true;
}

bool BleAdvert::GetParameter(size_t index, BleParaInfo &para)
{
    if (index >= mParaArray.size())
        return false;

    para = mParaArray[index];
    return true;
}

size_t BleAdvert::GetParameterCount()
{
    return mParaArray.size();
}

void BleAdvert::ClearAllParameters()
{
    mParaArray.clear();
}
