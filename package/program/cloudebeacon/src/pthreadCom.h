#ifndef __PTHREAD_COM_H__
#define __PTHREAD_COM_H__

#if 0
�߳�ͨѶ�࣬���ڸ����߳�֮���ͨѶ
#endif

#include <pthread.h>

#include "defCom.h"
#include "listStruct.h"
#include "StrOperate.h"

// �豸��Ϊ������
#define PhoneFlagHash   32


class ServerM : public CTimer
{
public:
    bool     m_user;
    int      m_sockFd;
    pthread_mutex_t m_Mutex;
    // �����߳��ڲ����ݵ�ͬ��
    char*    m_buff;
    short    m_buffLen;
    short    m_buffSize;
    u_int16  m_typeID;

public:
    ServerM();
    ~ServerM();
};


//////////////////////////////////////////////////////
class PthreadCom
{
public:
    ServerM* m_phone[PhoneFlagHash];
    int m_NetServerFd;           // ���������
    pthread_mutex_t m_lock;
	pthread_mutex_t m_sendLock;  // �������������Ҫ�Ǽ��������ѹ����
    CTimer m_SerialsAndNet;
	
public:
    PthreadCom();
    ~PthreadCom();
	
public:
    // ��������: ��ȡ�ֻ���־��
    // �� �� ֵ: �ɹ����ر�־�ţ�ʧ�ܷ���-1
    int OpenPhoneHandle();
    // ��������: �����ֻ���־��
    // ��������: ��
    void ClosePhoneHandle(int flag);
    void setNetServerFd(int sockfd);
    int  getNetServerFd();
};



//////////////////////////////////////////////////////////////////////////////////////
// ��������: ��ȡ�ֻ����
// �� �� ֵ: �ɹ����ؾ����ʧ�ܷ���-1
int OpenPhoneHandle();
// ��������: �ر��ֻ����
// ��������: ��
void ClosePhoneHandle(int handle);
void setPthreadcomServerFd(int sockfd);
// ��������: ���ڴ������ȡ����
// ��������: output�������������ȡ�����ַ�
//           outputLen�����������output��������С
//           typeID��������������ݰ�����
//           phoneHandle������������ֻ�ͨѶ���
//           NetFlag����������������־
//           MemSize��������������ж�������û��ȡ
//           timeOut����ʱ�ȴ�����λ����
// �� �� ֵ: ���� -1 Ϊ�����ɹ����ش��ڶ�ȡ�������ݳ���
int
ReadDataFromNet(char* output,
                int outputLen,
                u_int16* typeID,
                u_int32* phoneHandle,
                char* NetFlag,
                int *MemSize,
                int timeOut);
// ��������: �����緢������
// ��������: output�������������ȡ�����ַ�
//           outputLen�����������output��������С
//           typeID��������������ݰ�����
//           phoneHandle������������ֻ�ͨѶ���
//           NetFlag����������������־
//           timeOut����ʱ�ȴ�����λ����
// �� �� ֵ: ���� -1 Ϊ�����ɹ����ش��ڶ�ȡ�������ݳ���
int
WriteDataToNet(const char* data,
               int dataLen,
               u_int16 typeID,
               char NetFlag,
               u_int32 phoneHandle);
// ��������: �򴮿�д���ݣ������ݼӵ����ͻ�������
// ��������: buff�������������������
//           buffLen������������������ݳ���
//           typeID����������
//           phoneHandle���ֻ�ͨѶ���
//           netFlag�������־
// �� �� ֵ: �ɹ�����0�� ʧ�ܷ���-1��
int
WriteDataToSerials(const char* buff,
                   int buffLen,
                   u_int16 typeID,
                   int phoneHandle,
                   bool netFlag);
// ��������: �������ݸ��ֻ�
// ��������: data�������������������
//           dataLen������������������ݳ���
//           typeID�������������������
// �� �� ֵ: �ɹ�����д������ݳ��ȣ�ʧ�ܷ���-1��
int
NetWriteDataToPhone(char* data, int dataLen, u_int16 typeID);
// ��������: �������ݸ��ֻ�������
// ��������: data�������������������
//           dataLen������������������ݳ���
//           typeID�������������������
// �� �� ֵ: �ɹ�����0��ʧ�ܷ���-1��
int
NetWriteDataToPhoneServer(
    char* data,
    int dataLen,
    u_int16 typeID);
// ��������: �ֻ��������������ȡ����
// ��������: data������������������ݣ������ͷ�
//           dataLen������������������ݳ���
//           phoneHandle���ֻ�ͨѶ���
//           typeID�������������������
//           timeOut����ʱʱ�䣻
// �� �� ֵ: �ɹ�����0��ʧ�ܷ���-1��
int
PhoneServerReadDataFromNet(char** data,
                           int* dataLen,
                           int phoneHandle,
                           u_int16 *typeID,
                           int timeOut);
// ��������: ��ͻ���������ķ���������beacon����
// ��������: buff��������������͵�json����
//           buffLen��������������͵�json���ݳ���
// �� �� ֵ: 0���ɹ���-1��û�в���ͻ���������-2�����ӷ�����ʧ��
int
PostDataToCustomBeaconServer(const char* buff, int buffLen);

ListManage* GetNet2SerHead();


// ��������: ͨ��tcp��������
// ��������: sockFD, tcp�ļ�������
//           buff�����͵��ַ����壬buffLen���ַ������С
//           typeID����Ϣ����
// �� �� ֵ: ���͵ĳ���
int sendDataByTCP(int sockFD, const char* buff, int buffLen, u_int16 typeID);


// ��������: ����һ�����ݰ�
// ��������: sockFD, tcp�ļ�������
//           recvBuff�����յ��ַ����壬buffLen���ַ������С
//           timeOut����ʱ����λ����
// �� �� ֵ: -1������رգ�-2����ͷ��������0�����ݰ�����
int RecvOnePkg(int sockFD, char *recvBuff, int buffLen, int timeOut);


#endif /*__PTHREAD_COM_H__*/


