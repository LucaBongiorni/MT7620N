#ifndef __PTHREAD_COM_H__
#define __PTHREAD_COM_H__

#if 0
线程通讯类，用于各个线程之间的通讯
#endif

#include <pthread.h>

#include "defCom.h"
#include "listStruct.h"
#include "StrOperate.h"

// 设备作为服务器
#define PhoneFlagHash   32


class ServerM : public CTimer
{
public:
    bool     m_user;
    int      m_sockFd;
    pthread_mutex_t m_Mutex;
    // 用于线程内部数据的同步
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
    int m_NetServerFd;           // 网络服务器
    pthread_mutex_t m_lock;
	pthread_mutex_t m_sendLock;  // 发送锁，这个主要是减轻服务器压力吧
    CTimer m_SerialsAndNet;
	
public:
    PthreadCom();
    ~PthreadCom();
	
public:
    // 函数功能: 获取手机标志号
    // 返 回 值: 成功返回标志号，失败返回-1
    int OpenPhoneHandle();
    // 函数功能: 返回手机标志号
    // 函数参数: 无
    void ClosePhoneHandle(int flag);
    void setNetServerFd(int sockfd);
    int  getNetServerFd();
};



//////////////////////////////////////////////////////////////////////////////////////
// 函数功能: 获取手机句柄
// 返 回 值: 成功返回句柄，失败返回-1
int OpenPhoneHandle();
// 函数功能: 关闭手机句柄
// 函数参数: 无
void ClosePhoneHandle(int handle);
void setPthreadcomServerFd(int sockfd);
// 函数功能: 串口从网络读取数据
// 函数参数: output，输出参数，读取到的字符
//           outputLen，输入参数，output缓冲区大小
//           typeID，输出参数，数据包类型
//           phoneHandle，输出参数，手机通讯句柄
//           NetFlag，输出参数，网络标志
//           MemSize，输出参数，还有多大的数据没读取
//           timeOut，超时等待，单位毫秒
// 返 回 值: 返回 -1 为出错，成功返回大于读取到的数据长度
int
ReadDataFromNet(char* output,
                int outputLen,
                u_int16* typeID,
                u_int32* phoneHandle,
                char* NetFlag,
                int *MemSize,
                int timeOut);
// 函数功能: 向网络发送数据
// 函数参数: output，输出参数，读取到的字符
//           outputLen，输入参数，output缓冲区大小
//           typeID，输出参数，数据包类型
//           phoneHandle，输入参数，手机通讯句柄
//           NetFlag，输出参数，网络标志
//           timeOut，超时等待，单位毫秒
// 返 回 值: 返回 -1 为出错，成功返回大于读取到的数据长度
int
WriteDataToNet(const char* data,
               int dataLen,
               u_int16 typeID,
               char NetFlag,
               u_int32 phoneHandle);
// 函数功能: 向串口写数据，将数据加到发送缓冲区中
// 函数参数: buff，输入参数，发送数据
//           buffLen，输入参数，发送数据长度
//           typeID，数据类型
//           phoneHandle，手机通讯句柄
//           netFlag，网络标志
// 返 回 值: 成功返回0， 失败返回-1；
int
WriteDataToSerials(const char* buff,
                   int buffLen,
                   u_int16 typeID,
                   int phoneHandle,
                   bool netFlag);
// 函数功能: 发送数据给手机
// 函数参数: data，输入参数，发送数据
//           dataLen，输入参数，发送数据长度
//           typeID，输入参数，数据类型
// 返 回 值: 成功返回写入的数据长度，失败返回-1；
int
NetWriteDataToPhone(char* data, int dataLen, u_int16 typeID);
// 函数功能: 发送数据给手机服务器
// 函数参数: data，输入参数，发送数据
//           dataLen，输入参数，发送数据长度
//           typeID，输入参数，数据类型
// 返 回 值: 成功返回0，失败返回-1；
int
NetWriteDataToPhoneServer(
    char* data,
    int dataLen,
    u_int16 typeID);
// 函数功能: 手机服务器从网络读取数据
// 函数参数: data，输出参数，接收数据，不用释放
//           dataLen，输出参数，接收数据长度
//           phoneHandle，手机通讯句柄
//           typeID，输出参数，数据类型
//           timeOut，超时时间；
// 返 回 值: 成功返回0，失败返回-1；
int
PhoneServerReadDataFromNet(char** data,
                           int* dataLen,
                           int phoneHandle,
                           u_int16 *typeID,
                           int timeOut);
// 函数功能: 向客户自主架设的服务器发送beacon数据
// 函数参数: buff，输入参数，发送的json数据
//           buffLen，输入参数，发送的json数据长度
// 返 回 值: 0，成功；-1，没有部署客户服务器；-2，连接服务器失败
int
PostDataToCustomBeaconServer(const char* buff, int buffLen);

ListManage* GetNet2SerHead();


// 函数功能: 通过tcp发送数据
// 函数参数: sockFD, tcp文件描述符
//           buff，发送的字符缓冲，buffLen，字符缓冲大小
//           typeID，消息类型
// 返 回 值: 发送的长度
int sendDataByTCP(int sockFD, const char* buff, int buffLen, u_int16 typeID);


// 函数功能: 接收一个数据包
// 函数参数: sockFD, tcp文件描述符
//           recvBuff，接收的字符缓冲，buffLen，字符缓冲大小
//           timeOut，超时，单位毫秒
// 返 回 值: -1，网络关闭，-2，包头出错，大于0，数据包长度
int RecvOnePkg(int sockFD, char *recvBuff, int buffLen, int timeOut);


#endif /*__PTHREAD_COM_H__*/


