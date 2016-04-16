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

// ��������: �����緢������
// ��������: output�������������ȡ�����ַ�
//			 outputLen�����������output��������С
//			 typeID��������������ݰ�����
//			 phoneHandle������������ֻ�ͨѶ���
//			 NetFlag����������������־
//			 timeOut����ʱ�ȴ�����λ����
// �� �� ֵ: ���� -1 Ϊ�����ɹ����ش��ڶ�ȡ�������ݳ��� 
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

	// ʹ���߳���
	if (1 == NetFlag)
	{
		// ���͵������������ֱ�ӷ���
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
		// ������û�����ݣ���ʱ�ȴ�
		if (0 != pthreadcom.m_SerialsAndNet.waitForEvent(timeOut))
		{
			write_log("Recv timeout.");
			pthread_mutex_unlock(&readLock);
			return -1;
		}
		else
		{
			// ��ʱ�����ȷ����������
			nRet = GetNodeInfoFromNet2SerList(&net2SerHead, 
						output, outputLen, typeID, phoneHandle, NetFlag, MemSize);
			pthread_mutex_unlock(&readLock);
			return nRet;
		}
	}
	else
	{	
		// �����������ݣ�ֱ�ӷ��ء�
		pthread_mutex_unlock(&readLock);
		return nRet;	
	}
}

// ��������: �򴮿�д���ݣ������ݼӵ����ͻ�������
// ��������: buff�������������������
//           buffLen������������������ݳ���
//           typeID����������
//           phoneHandle���ֻ�ͨѶ���
//           netFlag�������־
// �� �� ֵ: �ɹ�����0�� ʧ�ܷ���-1��
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


// ��������: �������ݸ��ֻ�
// ��������: data�������������������
//           dataLen������������������ݳ���
//           typeID�������������������
// �� �� ֵ: �ɹ�����д������ݳ��ȣ�ʧ�ܷ���-1��
int NetWriteDataToPhone(char* data, int dataLen, u_int16 typeID)
{
	int phoneHandle;
	data[dataLen] = 0;
	// ������phoneHandle
	cJSON* root;
	root = cJSON_Parse(data);
	phoneHandle = cJSON_GetNumberItem(root, "phoneHandle");
	cJSON_Delete(root);
	
	// �Ӱ�ͷ
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

// ��������: �������ݸ��ֻ�������
// ��������: data�������������������
//           dataLen������������������ݳ���
//           typeID�������������������
// �� �� ֵ: �ɹ�����0��ʧ�ܷ���-1��
int 
NetWriteDataToPhoneServer(char* data, int dataLen, u_int16 typeID)
{
	int phoneHandle;
	data[dataLen] = 0;
	
	// ������phoneHandle
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

// ��������: �ֻ��������������ȡ����
// ��������: data������������������ݣ������ͷ�
//           dataLen������������������ݳ���
//           phoneHandle���ֻ�ͨѶ���
//           typeID�������������������
//           timeOut����ʱʱ�䣻
// �� �� ֵ: �ɹ�����0��ʧ�ܷ���-1��
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


// ��������: ��ͻ���������ķ���������beacon����
// ��������: buff��������������͵�json����
//           buffLen��������������͵�json���ݳ���
// �� �� ֵ: 0���ɹ���-1��û�в���ͻ���������-2�����ӷ�����ʧ��
int PostDataToCustomBeaconServer(const char* buff, int buffLen)
{
	IbeaconConfig *conf = GetConfigPosition();
	if (false == conf->getIsOpenBeaconSer())
	{
		return -1;
	}

	char* httpHead = (char*)malloc(buffLen+POST_HEAD_LEN);
	int postDataLen;
	// ��װ post ͷ
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

	// ��������
	CSocketTCP *sockConnect = new CSocketTCP(conf->getBeaconSerHost(), conf->getBeaconSerPort());
	int sockfd = sockConnect->Connect();
	if (-1 == sockfd)
	{
		delete sockConnect;
		free(httpHead);
		return -2;
	}
	
	// ��������
	Write(sockfd, httpHead, postDataLen, 3000);
	Close(sockfd);
	delete sockConnect;
	free(httpHead);
	return 0;
}


// ��������: ��ͻ���������ķ���������mac����
// ��������: buff��������������͵�json����
//           buffLen��������������͵�json���ݳ���
// �� �� ֵ: 0���ɹ���-1��û�в���ͻ���������-2�����ӷ�����ʧ��
int PostDataToCustomMacServer(const char* buff, int buffLen)
{
	IbeaconConfig *conf = GetConfigPosition();
	if (false == conf->getIsOpenMacSer())
	{
		return -1;
	}

	char* httpHead = (char*)malloc(buffLen+POST_HEAD_LEN);
	int postDataLen;
	// ��װ post ͷ
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

	// ��������
	CSocketTCP *sockConnect = new CSocketTCP(conf->getMacSerHost(), conf->getMacSerPort());
	int sockfd = sockConnect->Connect();
	if (-1 == sockfd)
	{
		delete sockConnect;
		free(httpHead);
		return -2;
	}
	
	// ��������
	Write(sockfd, httpHead, postDataLen, 3000);
	Close(sockfd);
	delete sockConnect;
	free(httpHead);
	return 0;
}



