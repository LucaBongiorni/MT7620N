//=============================================================================
// FILE  : ble_device.cpp
// DESC  :
// AUTHOR: PINBO
// CREATE: 2015-06-02
// MODIFY:
//=============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ble_defines.h"
#include "ble_device.h"


//=============================================================================
//=============================================================================
BleDevice::BleDevice()
{
    memset(&mMacAddr, 0, sizeof(BleAddrInfo));
    mRssiVec.clear();
    mUUIDMap.clear();
    mScanMap.clear();
    mCharMap.clear();
    mScanMap.clear();
}

BleDevice::BleDevice(BleAddrInfo &addr)
{
    memcpy(&mMacAddr, &addr, sizeof(BleAddrInfo));
    mRssiVec.clear();
    mUUIDMap.clear();
    mScanMap.clear();
    mCharMap.clear();
    mScanMap.clear();
}

BleDevice::~BleDevice()
{
    ClearAllRssis();
    ClearAllUUIDs();
    ClearAllChars();
    ClearAllScans();
}

void BleDevice::SetAddress(BleAddrInfo &addr)
{
    memcpy(&mMacAddr, &addr, sizeof(BleAddrInfo));
}

void BleDevice::GetAddress(BleAddrInfo &addr)
{
    memcpy(&addr, &mMacAddr, sizeof(BleAddrInfo));
}

bool BleDevice::CmpAddress(BleAddrInfo &addr)
{
    return (memcmp(&mMacAddr, &addr, sizeof(BleAddrInfo)) == 0);
}

bool BleDevice::AddRssiInfo(BleRssiInfo &info)
{
    int n = mRssiVec.size();
    if (n > 0 && (mRssiVec[n - 1].time == info.time))
        mRssiVec[n - 1].rssi = info.rssi;
    else
        mRssiVec.push_back(info);
    return true;
}

int BleDevice::GetRssiCount()
{
    return mRssiVec.size();
}

void BleDevice::ClearAllRssis()
{
    mRssiVec.clear();
}

bool BleDevice::GetLastRssi(BleRssiInfo &info)
{
    if (mRssiVec.empty())
        return false;

    info = mRssiVec.back();
    return true;
}

bool BleDevice::AddAdvertUUID(uint16_t uuid)
{
    UUID_ITER iter = mUUIDMap.find(uuid);
    if (iter == mUUIDMap.end())
        mUUIDMap.insert(UUID_PAIR(uuid, uuid));
    return true;
}

int BleDevice::GetUUIDCount()
{
    return mUUIDMap.size();
}

void BleDevice::ClearAllUUIDs()
{
    mUUIDMap.clear();
}

bool BleDevice::AddCharInfo(BleCharInfo &info)
{
    CHAR_ITER iter = mCharMap.find(info.uuid);
    if (iter == mCharMap.end())
        mCharMap.insert(CHAR_PAIR(info.uuid, info));
    else
        iter->second = info;
    return true;
}

int BleDevice::GetCharCount()
{
    return mCharMap.size();
}

void BleDevice::ClearAllChars()
{
    mCharMap.clear();
}

bool BleDevice::AddScanData(BleScanData &info)
{
    SCAN_ITER iter = mScanMap.find((const char *)info.name);
    if (iter == mScanMap.end())
        mScanMap.insert(SCAN_PAIR((const char *)info.name, info));
    else
        iter->second = info;
    return true;
}

int BleDevice::GetScanCount()
{
    return mScanMap.size();
}

void BleDevice::ClearAllScans()
{
    mScanMap.clear();
}

bool BleDevice::PrintRssiToArray(cJSON *pRssiArray, cJSON *pTimeArray)
{
    RSSI_ITER iter = mRssiVec.begin();
    for (; iter != mRssiVec.end(); iter++)
    {
        cJSON_AddItemToArray(pRssiArray, cJSON_CreateNumber((*iter).rssi));
        cJSON_AddItemToArray(pTimeArray, cJSON_CreateNumber((*iter).time));
    }
    return true;
}

bool BleDevice::PrintUUIDToArray(cJSON *pUUIDArray)
{
    UUID_ITER iter = mUUIDMap.begin();
    for (; iter != mUUIDMap.end(); iter++)
    {
        cJSON_AddItemToArray(pUUIDArray, cJSON_CreateNumber(iter->second));
    }
    return true;
}

bool BleDevice::PrintCharToArray(cJSON *pCharArray)
{
    return true;
}

bool BleDevice::PrintScanToArray(cJSON *pScanArray)
{
    char buffer[64];
    SCAN_ITER iter = mScanMap.begin();
    for (; iter != mScanMap.end(); iter++)
    {
        cJSON *pParameter = cJSON_CreateObject();
        if (!pParameter) return false;
        cJSON_AddStringToObject(pParameter, "para_name", (const char *)iter->second.name);
        cJSON_AddNumberToObject(pParameter, "para_type", iter->second.type);
        cJSON_AddNumberToObject(pParameter, "para_size", iter->second.size);

        if (iter->second.type != BLE_TYPE_STRING)
        {
            MsgHex2ASCII(buffer, iter->second.data, iter->second.size);
            cJSON_AddStringToObject(pParameter, "para_data", buffer);
        }
        else
        {
            strcpy(buffer, string_check((char *)iter->second.data));
            cJSON_AddStringToObject(pParameter, "para_data", buffer);
        }

        cJSON_AddItemToArray(pScanArray, pParameter);
    }

    return true;
}

bool BleDevice::PrintScanToObject(cJSON *pObject)
{
    char buffer[64];
    SCAN_ITER iter = mScanMap.begin();
    for (; iter != mScanMap.end(); iter++)
    {
        if (iter->second.type != BLE_TYPE_STRING)
        {
            MsgHex2ASCII(buffer, iter->second.data, iter->second.size);
            cJSON_AddStringToObject(pObject, (const char *)iter->second.name, buffer);
        }
        else
        {
            strcpy(buffer, string_check((char *)iter->second.data));
            cJSON_AddStringToObject(pObject, (const char *)iter->second.name, buffer);
        }
    }

    return true;
}
