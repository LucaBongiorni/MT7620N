#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "pthreadCom.h"
#include "listStruct.h"
#include "procotol.h"
#include "cJSON.h"
#include "cbProjMain.h"


PthreadCom pthreadcom;
ListManage net2SerHead;

ListManage* GetNet2SerHead()
{
	return &net2SerHead;
}


ServerM::ServerM()
{
	m_buff     = NULL;
	m_buffLen  = 0;
	m_buffSize = 0;
	m_user     = false;
	m_sockFd   = -1;
	pthread_mutex_init(&m_Mutex, NULL);
}
ServerM::~ServerM()
{
	if (m_buff) free(m_buff), m_buff = NULL;
	pthread_mutex_destroy(&m_Mutex);
	m_user     = false;
	m_sockFd   = -1;
	m_buffLen  = 0;
	m_buffSize = 0;
}



PthreadCom::PthreadCom()
{
	for (int i=0; i<PhoneFlagHash; ++i)
	{
		m_phone[i] = new ServerM;		
	}
	pthread_mutex_init(&m_lock, 0);
}

PthreadCom::~PthreadCom()
{
	for (int i=0; i<PhoneFlagHash; ++i)
	{
		if (m_phone[i])
		{
			if (m_phone[i]->m_buff)
			{
				delete [] m_phone[i]->m_buff;
				m_phone[i]->m_buff = NULL;
			}
			delete m_phone[i];
			m_phone[i] = NULL;
		}	
	}
	pthread_mutex_destroy(&m_lock);
}

int 
PthreadCom::OpenPhoneHandle()
{
	pthread_mutex_lock(&m_lock);
	for (int i=0; i<PhoneFlagHash; ++i)
	{
		if (false == m_phone[i]->m_user)
		{
			m_phone[i]->m_user = true;
			pthread_mutex_unlock(&m_lock);
			return i;
		}
	}
	pthread_mutex_unlock(&m_lock);
	return -1;
}
void 
PthreadCom::ClosePhoneHandle(int flag)
{
	pthread_mutex_lock(&m_lock);
	if (flag >= 0 && flag < PhoneFlagHash)
	{
		m_phone[flag]->m_user = false;
		if (m_phone[flag]->m_buff != NULL)
		{
			free(m_phone[flag]->m_buff);
			m_phone[flag]->m_buff     = NULL;
			m_phone[flag]->m_buffLen  = 0;
			m_phone[flag]->m_buffSize = 0;
			m_phone[flag]->m_typeID   = 0;
		}
	}
	pthread_mutex_unlock(&m_lock);
	return ;
}


void 
PthreadCom::setNetServerFd(int sockfd)
{
	m_NetServerFd = sockfd;
}

int  
PthreadCom::getNetServerFd()
{
	return m_NetServerFd;
}

void setPthreadcomServerFd(int sockfd)
{
	pthreadcom.m_NetServerFd = sockfd;
}

int OpenPhoneHandle()
{
	return pthreadcom.OpenPhoneHandle();
}

void ClosePhoneHandle(int handle)
{
	pthreadcom.ClosePhoneHandle(handle);
}

// 函数功能: 向网络发送数据
// 函数参数: output，输出参数，读取到的字符
//			 outputLen，输入参数，output缓冲区大小
//			 typeID，输出参数，数据包类型
//			 phoneHandle，输入参数，手机通讯句柄
//			 NetFlag，输出参数，网络标志
//			 timeOut，超时等待，单位毫秒
// 返 回 值: 返回 -1 为出错，成功返回大于读取到的数据长度 
int 
WriteDataToNet(const char* data, int dataLen, 
			u_int16	typeID, char NetFlag, u_int32 phoneHandle)
{
	if (NULL == data || 0 >= pthreadcom.m_NetServerFd)
	{
		return -2;
	}
	write_normal_log("-----------write Data to net-------, pthreadcom.m_NetServerFd=%d", 
		pthreadcom.m_NetServerFd);
	
	char PkgTotal = dataLen / EVERY_PKG_LEN + 1;
	(dataLen % EVERY_PKG_LEN == 0) ? PkgTotal : (++PkgTotal);
	char PkgSeq;
	char temp[EVERY_PKG_LEN + DEF_PRO_HEAD_LEN] = {0};
	int nRet, nSend = 0, SendLen = 0, length;
	//IbeaconConfig *conf = GetConfigPosition();

	// 使用线程锁
	if (1 == NetFlag)
	{
		// 发送到网络服务器，直接发送
		for (nSend=0, PkgSeq=1; PkgSeq<=PkgTotal; ++PkgSeq)
		{
			if (PkgSeq < PkgTotal)
				SendLen = EVERY_PKG_LEN;
			else 
				SendLen = dataLen % EVERY_PKG_LEN;

			memcpy(temp, data+nSend, SendLen);
			length = Procotol::AddPkgHeadAndTail(temp, SendLen, typeID, PkgTotal, PkgSeq);
			nRet = Send(pthreadcom.m_NetServerFd, temp, length, 3000);
			if (nRet <= 0)
			{
				write_error_log("Write Error.nRet=%d", nRet);
				return nSend;
			}
			write_normal_log("Send succuss. pkgTotal=%d, pkgSeq=%d", PkgTotal, PkgSeq);
			nSend += SendLen;
		}
	}
	else 
	{
		if (NULL == pthreadcom.m_phone[phoneHandle])
		{
			return -1;
		}

		for (nSend=0, PkgSeq=1; PkgSeq<=PkgTotal; ++PkgSeq)
		{
			if (PkgSeq == PkgTotal)
				SendLen = dataLen % EVERY_PKG_LEN;
			else
				SendLen = EVERY_PKG_LEN;
			
			memcpy(temp, data+nSend, SendLen);
			nRet = Procotol::AddPkgHeadAndTail(temp, SendLen, typeID, PkgTotal, PkgSeq);
			nRet = Write(pthreadcom.m_phone[phoneHandle]->m_sockFd, data, nRet, 3000);
			if (nRet <= 0)
			{
				write_error_log("Write Error.nRet=%d", nRet);
				return nSend;
			}
			write_normal_log("Send succuss. pkgTotal=%d, pkgSeq=%d", PkgTotal, PkgSeq);
			nSend += SendLen;
		}
	}

	return nSend;
}

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
ReadDataFromNet(char* output, int outputLen, u_int16* typeID, 
		u_int32* phoneHandle, char* NetFlag, int *MemSize, int timeOut)
{
	static pthread_mutex_t readLock = PTHREAD_MUTEX_INITIALIZER;
	int nRet;

	pthread_mutex_lock(&readLock);
	nRet = GetNodeInfoFromNet2SerList(&net2SerHead, 
				output, outputLen, typeID, phoneHandle, NetFlag, MemSize);
	if (nRet == -1)
	{
		// 缓冲区没有数据，超时等待
		if (0 != pthreadcom.m_SerialsAndNet.waitForEvent(timeOut))
		{
			write_log("Recv timeout.");
			pthread_mutex_unlock(&readLock);
			return -1;
		}
		else
		{
			// 这时候可以确定有数据了
			nRet = GetNodeInfoFromNet2SerList(&net2SerHead, 
						output, outputLen, typeID, phoneHandle, NetFlag, MemSize);
			pthread_mutex_unlock(&readLock);
			return nRet;
		}
	}
	else
	{	
		// 缓冲区有数据，直接返回。
		pthread_mutex_unlock(&readLock);
		return nRet;	
	}
}

// 函数功能: 向串口写数据，将数据加到发送缓冲区中
// 函数参数: buff，输入参数，发送数据
//           buffLen，输入参数，发送数据长度
//           typeID，数据类型
//           phoneHandle，手机通讯句柄
//           netFlag，网络标志
// 返 回 值: 成功返回0， 失败返回-1；
int 
WriteDataToSerials(const char* buff, int buffLen, u_int16 typeID, int phoneHandle, bool netFlag)
{
	int nRet;
	nRet = AddNodeToNet2SerList(&net2SerHead, buff, buffLen, typeID, phoneHandle, netFlag);
	if (0 == nRet)
	{
		pthreadcom.m_SerialsAndNet.triggerEvent();
	}
	return nRet;
}


// 函数功能: 发送数据给手机
// 函数参数: data，输入参数，发送数据
//           dataLen，输入参数，发送数据长度
//           typeID，输入参数，数据类型
// 返 回 值: 成功返回写入的数据长度，失败返回-1；
int NetWriteDataToPhone(char* data, int dataLen, u_int16 typeID)
{
	int phoneHandle;
	data[dataLen] = 0;
	// 解析出phoneHandle
	cJSON* root;
	root = cJSON_Parse(data);
	phoneHandle = cJSON_GetNumberItem(root, "phoneHandle");
	cJSON_Delete(root);
	
	// 加包头
	char PkgTotal = dataLen / EVERY_PKG_LEN;
	(dataLen % EVERY_PKG_LEN == 0) ? PkgTotal : (++PkgTotal);
	char PkgSeq;
	int  nRet, nSend = 0;
	u_int16 length;
	char temp[EVERY_PKG_LEN + DEF_PRO_HEAD_LEN];
	for (PkgSeq=1; PkgSeq<=PkgTotal; ++PkgSeq)
	{
		if (PkgSeq == PkgTotal)
			length = dataLen % EVERY_PKG_LEN;
		else
			length = EVERY_PKG_LEN;

		memcpy(temp, data+nSend, length);
		nRet = Procotol::AddPkgHeadAndTail(temp, length, typeID, PkgTotal, PkgSeq);
		if ( 0 >= Write(pthreadcom.m_phone[phoneHandle]->m_sockFd, temp, nRet, 3000))
			return nSend;
		nSend += length;
	}
	return nSend;
}

// 函数功能: 发送数据给手机服务器
// 函数参数: data，输入参数，发送数据
//           dataLen，输入参数，发送数据长度
//           typeID，输入参数，数据类型
// 返 回 值: 成功返回0，失败返回-1；
int 
NetWriteDataToPhoneServer(char* data, int dataLen, u_int16 typeID)
{
	int phoneHandle;
	data[dataLen] = 0;
	
	// 解析出phoneHandle
	cJSON* root;
	root = cJSON_Parse(data);
	phoneHandle = cJSON_GetNumberItem(root, "phoneHandle");
	cJSON_Delete(root);

	if (pthreadcom.m_phone[phoneHandle]->m_buffSize < dataLen)
	{
		if (NULL == pthreadcom.m_phone[phoneHandle]->m_buff)
		{
			pthreadcom.m_phone[phoneHandle]->m_buff = (char*)malloc(dataLen);
			memcpy(pthreadcom.m_phone[phoneHandle]->m_buff, data, dataLen);
			pthreadcom.m_phone[phoneHandle]->m_buffSize = dataLen;
			pthreadcom.m_phone[phoneHandle]->m_buffLen  = dataLen;
		}
		else
		{
			pthreadcom.m_phone[phoneHandle]->m_buff = 
				(char*)realloc(pthreadcom.m_phone[phoneHandle]->m_buff, dataLen);
			memcpy(pthreadcom.m_phone[phoneHandle]->m_buff, data, dataLen);
			pthreadcom.m_phone[phoneHandle]->m_buffSize = dataLen;
			pthreadcom.m_phone[phoneHandle]->m_buffLen  = dataLen;
		}
	}
	else
	{
		memcpy(pthreadcom.m_phone[phoneHandle]->m_buff, data, dataLen);
		pthreadcom.m_phone[phoneHandle]->m_buffLen = dataLen;
	}
	pthreadcom.m_phone[phoneHandle]->triggerEvent();
	return 0;
}

// 函数功能: 手机服务器从网络读取数据
// 函数参数: data，输出参数，接收数据，不用释放
//           dataLen，输出参数，接收数据长度
//           phoneHandle，手机通讯句柄
//           typeID，输出参数，数据类型
//           timeOut，超时时间；
// 返 回 值: 成功返回0，失败返回-1；
int PhoneServerReadDataFromNet(char** data, int* dataLen, int phoneHandle, u_int16 *typeID, int timeOut)
{
	if (0 != pthreadcom.m_phone[phoneHandle]->waitForEvent(timeOut))
	{
		return -1;
	}
	else
	{
		*data = pthreadcom.m_phone[phoneHandle]->m_buff;
		*dataLen = pthreadcom.m_phone[phoneHandle]->m_buffLen;
		*typeID = pthreadcom.m_phone[phoneHandle]->m_typeID;
		return 0;
	}
}


// 函数功能: 向客户自主架设的服务器发送beacon数据
// 函数参数: buff，输入参数，发送的json数据
//           buffLen，输入参数，发送的json数据长度
// 返 回 值: 0，成功；-1，没有部署客户服务器；-2，连接服务器失败
int PostDataToCustomBeaconServer(const char* buff, int buffLen)
{
	IbeaconConfig *conf = GetConfigPosition();
	if (false == conf->getIsOpenBeaconSer())
	{
		return -1;
	}

	char* httpHead = (char*)malloc(buffLen+POST_HEAD_LEN);
	int postDataLen;
	// 组装 post 头
	postDataLen = snprintf(httpHead, buffLen+POST_HEAD_LEN, 
					"POST %s HTTP/1.1\r\n"
					"HOST: %s:%d\r\n"
					"Content-Type: application/json;charset=utf-8\r\n"
					"Content-Length: %d\r\n\r\n"
					"%s",
					conf->getBeaconSerUrl(), 
					conf->getBeaconSerHost(), 
					conf->getBeaconSerPort(), 
					buffLen, buff);

	// 创建连接
	CSocketTCP *sockConnect = new CSocketTCP(conf->getBeaconSerHost(), conf->getBeaconSerPort());
	int sockfd = sockConnect->Connect();
	if (-1 == sockfd)
	{
		delete sockConnect;
		free(httpHead);
		return -2;
	}
	
	// 发送数据
	Write(sockfd, httpHead, postDataLen, 3000);
	Close(sockfd);
	delete sockConnect;
	free(httpHead);
	return 0;
}


// 函数功能: 向客户自主架设的服务器发送mac数据
// 函数参数: buff，输入参数，发送的json数据
//           buffLen，输入参数，发送的json数据长度
// 返 回 值: 0，成功；-1，没有部署客户服务器；-2，连接服务器失败
int PostDataToCustomMacServer(const char* buff, int buffLen)
{
	IbeaconConfig *conf = GetConfigPosition();
	if (false == conf->getIsOpenMacSer())
	{
		return -1;
	}

	char* httpHead = (char*)malloc(buffLen+POST_HEAD_LEN);
	int postDataLen;
	// 组装 post 头
	postDataLen = snprintf(httpHead, buffLen+POST_HEAD_LEN, 
					"POST %s HTTP/1.1\r\n"
					"HOST: %s:%d\r\n"
					"Content-Type: application/json;charset=utf-8\r\n"
					"Content-Length: %d\r\n\r\n"
					"%s",
					conf->getMacSerUrl(), 
					conf->getMacSerHost(), 
					conf->getMacSerPort(), 
					buffLen, buff);
	//write_log("%s", httpHead);

	// 创建连接
	CSocketTCP *sockConnect = new CSocketTCP(conf->getMacSerHost(), conf->getMacSerPort());
	int sockfd = sockConnect->Connect();
	if (-1 == sockfd)
	{
		delete sockConnect;
		free(httpHead);
		return -2;
	}
	
	// 发送数据
	Write(sockfd, httpHead, postDataLen, 3000);
	Close(sockfd);
	delete sockConnect;
	free(httpHead);
	return 0;
}



