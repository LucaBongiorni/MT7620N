#ifndef __UP_BTOOTH__H__
#define __UP_BTOOTH__H__

void updateBluetoothBinTask(void* arg);

int ReadOnePkg(int iSockFD, char * cData, int iLen, int iTimeOut);
int postDataToBlueSer(char** out, int &outLen);


#endif /*__UPDATE__H__*/

