#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>


#include "procotol.h"
#include "socketClient.h"
#include "defCom.h"
#include "pthreadServer.h"
#include "netSockTcp.h"
#include "cbProjMain.h"
#include "base64RSA.h"
#include "list.h"
#include "listStruct.h"
#include "cJSON.h"
#include "confile.h"
#include "pthreadCom.h"
#include "dealWithOpenwrt.h"
#include "beaconConf.h"

#ifdef SERIALSCOM
#include "SerialCom.h"
#include "md5.h"
#endif



static void *(*SktCliMalloc)(size_t size) = malloc;
static void (*SktCliFree)(void* ptr) = free;




// ����λɨ��ģ��
void 
dealWithRstScanDev(int sockFd, char* buff, int buffLen)
{
	// ��ȡ��ˮ��
	int switfNum = 0, nRet = 0, outLen = 0;
	char outBuff[128] = {0};
	char* out = NULL;
	cJSON* root = NULL;
	char* pOutBuff = outBuff;
	u_int16 typeID;
	short val = -1;

	buff[buffLen] = 0;
	root = cJSON_Parse(buff);
	if (!root)
	{
		Debug(I_ERROR, "JSON Parse Failed...");
		return;
	}
	switfNum = cJSON_GetNumberItem(root, "seq_no");
	cJSON_Delete(root), root = NULL;

	// ���͸�λɨ��ģ�����������
	//nRet = sendAndRecvComData(NULL, 0, COM_RST_SCAN_DEV, switfNum, outBuff, outLen, 3000);
	if (nRet < 0)
	{
		val = -1;
		goto dealWithFailed;
	}

	// ��ȡ��������
	pOutBuff += 4;
	typeID = ntohs(*(u_int16*)pOutBuff);
	if (typeID != COM_BEG_SCAN_DEV)
	{
		val = -1;
		goto dealWithFailed;
	}
	pOutBuff += 2;
	val = ntohs(*(short*)pOutBuff);
	switfNum = ntohl(*(int*)outBuff);
	
dealWithFailed:
	root = cJSON_CreateObject();
	cJSON_AddNumberToObject(root, "ack_no", switfNum);
	cJSON_AddNumberToObject(root, "ret_no", val);
	out = cJSON_Print(root);
	outLen = strlen(out);
	memcpy(outBuff, out, outLen);
	CJSONFree(out), out = NULL;
	
	// Ӧ��λɨ��ģ��
	nRet = Procotol::AddPkgHeadAndTail(outBuff, outLen, MES_RST_BLU_DEV_ACK, 1, 1);
	Send(sockFd, outBuff, nRet, 3000);
	return;
}


void dealWithBegScanDev(int sockFd, char* buff, int buffLen)
{
	cJSON* root = NULL;
	int switfNum = 0, nRet = 0, outLen = 0;
	char outBuff[128] = {0};
	char* out = NULL;
	char* pOutBuff = outBuff;
	u_int16 typeID;
	short val = -1;
	
	buff[buffLen] = 0;
	root = cJSON_Parse(buff);
	if (!root)
	{
		Debug(I_ERROR, "JSON Parse Failed...");
		return;
	}
	switfNum = cJSON_GetNumberItem(root, "seq_no");
	cJSON_Delete(root), root = NULL;

	// ���Ϳ�ʼɨ��ģ�����������
	//nRet = sendAndRecvComData(NULL, 0, COM_BEG_SCAN_DEV, switfNum, outBuff, outLen, 3000);
	if (nRet < 0)
	{
		val = -1;
		goto dealWithFailed;
	}

	// ��ȡ������Ϣ��
	pOutBuff += 4;
	typeID = ntohs(*(u_int16*)pOutBuff);
	if (typeID != COM_BEG_SCAN_DEV)
	{
		val = -1;
		goto dealWithFailed;
	}
	pOutBuff += 2;
	val = ntohs(*(short*)pOutBuff);
	switfNum = ntohl(*(int*)outBuff);
	
dealWithFailed:
	root = cJSON_CreateObject();
	cJSON_AddNumberToObject(root, "ack_no", switfNum);
	cJSON_AddNumberToObject(root, "ret_no", val);
	out = cJSON_Print(root);
	outLen = strlen(out);
	memcpy(outBuff, out, outLen);
	CJSONFree(out), out = NULL;

	// Ӧ��ʼɨ��ģ��
	nRet = Procotol::AddPkgHeadAndTail(outBuff, outLen, MES_BEG_SCAN_DEV_ACK, 1, 1);
	Send(sockFd, outBuff, nRet, 3000);
	return;
}

void dealWithStpScanDev(int sockFd, char* buff, int buffLen)
{
	cJSON* root = NULL;
	int switfNum = 0, nRet = 0, outLen = 0;
	char outBuff[128] = {0};
	char* out = NULL;
	char* pOutBuff = outBuff;
	u_int16 typeID;
	short val = -1;
	
	buff[buffLen] = 0;
	root = cJSON_Parse(buff);
	if (!root)
	{
		Debug(I_ERROR, "JSON Parse Failed...");
		return;
	}
	switfNum = cJSON_GetNumberItem(root, "seq_no");
	cJSON_Delete(root), root = NULL;

	// ����ֹͣɨ��ģ�����������
	//nRet = sendAndRecvComData(NULL, 0, COM_STP_SCAN_DEV, switfNum, outBuff, outLen, 3000);
	if (nRet < 0)
	{
		val = -1;
		goto dealWithFailed;
	}

	// ��ȡ������Ϣ��
	pOutBuff += 4;
	typeID = ntohs(*(u_int16*)pOutBuff);
	if (typeID != COM_STP_SCAN_DEV)
	{
		val = -1;
		goto dealWithFailed;
	}
	pOutBuff += 2;
	val = ntohs(*(short*)pOutBuff);
	switfNum = ntohl(*(int*)outBuff);
	
dealWithFailed:
	root = cJSON_CreateObject();
	cJSON_AddNumberToObject(root, "ack_no", switfNum);
	cJSON_AddNumberToObject(root, "ret_no", val);
	out = cJSON_Print(root);
	outLen = strlen(out);
	memcpy(outBuff, out, outLen);
	CJSONFree(out), out = NULL;

	// Ӧ��ֹͣɨ��ģ�����������
	nRet = Procotol::AddPkgHeadAndTail(outBuff, outLen, MES_STP_SCAN_DEV_ACK, 1, 1);
	Send(sockFd, outBuff, nRet, 3000);
	return;
}

#if 0
void dealWithSetDevParam(int sockFd, char* buff, int buffLen)
{
	cJSON* root = NULL;
	int nRet = 0, outLen = 0;
	char temp[32] = {0};
	char addr[32] = {0};
	char outBuff[128] = {0};
	char sendBuff[128] = {0};
	char* out = NULL;
	char* pOutBuff = outBuff;
	char* pSendBuff = sendBuff;
	u_int16 typeID;
	short val = -1;
	u_int16 tmp16;
	int switfNum, uuids, addrType;
	char* bleAddr = NULL, *pBleAddr = NULL;
	char* values = NULL;

	// ����json
	buff[buffLen] = 0;
	root = cJSON_Parse(buff);
	switfNum = cJSON_GetNumberItem(root, "seq_no");
	bleAddr  = cJSON_GetStringItem(root, "ble_addr");
	// ƴ�ӷ�����Ϣ
	pBleAddr = bleAddr;
	nRet = 0;
	while(*pBleAddr != '\0')
	{
		if (*pBleAddr != ':') 
		{
			*pOutBuff++ = *pBleAddr;
		}
		addr[nRet++] = *pBleAddr;
		++pBleAddr;
	}
	addr[nRet] = 0;
	CComTransmit::ASCIIToHex((u_int8*)outBuff, (u_int8*)temp, 6);
	// �豸��ַ
	memcpy(pSendBuff, temp, 6);
	pSendBuff += 6;
	// ��ַ����
	addrType = cJSON_GetNumberItem(root, "addr_type");
	temp[0] = addrType & 0x000000ff;
	*pSendBuff = temp[0];
	pSendBuff++;
	// ����uuid
	uuids = cJSON_GetNumberItem(root, "uuids");
	tmp16 = uuids & 0x0000ffff;
	memcpy(pSendBuff, &tmp16, 2);
	pSendBuff += 2;
	// ��������
	nRet = cJSON_GetNumberItem(root, "lengths");
	temp[0] = nRet & 0x000000ff;
	*pSendBuff = temp[0];
	pSendBuff++;
	// ƫ����
	nRet = cJSON_GetNumberItem(root, "offsets");
	// ��������
	values = cJSON_GetStringItem(root, "values");
	nRet = strlen(values);
	memcpy(pSendBuff, values, nRet);
	pSendBuff += nRet;
	cJSON_Delete(root), root = NULL;

	// ����ֹͣɨ��ģ�����������
	//nRet = sendAndRecvComData(sendBuff, pSendBuff-sendBuff, 
	//			COM_SET_SERVER_PARAM, switfNum, outBuff, outLen, 3000);
	if (nRet < 0)
	{
		val = -1;
		goto dealWithFailed;
	}

	// ��ȡ������Ϣ��
	pOutBuff += 4;
	typeID = ntohs(*(u_int16*)pOutBuff);
	if (typeID != COM_SET_SERVER_PARAM)
	{
		val = -1;
		goto dealWithFailed;
	}
	pOutBuff += 2;
	val = ntohs(*(short*)pOutBuff);
	switfNum = ntohl(*(int*)outBuff);
	
dealWithFailed:
	root = cJSON_CreateObject();
	cJSON_AddNumberToObject(root, "ack_no", switfNum);
	cJSON_AddNumberToObject(root, "ret_no", val);
	cJSON_AddStringToObject(root, "ble_addr", addr);
	cJSON_AddNumberToObject(root, "addr_type", addrType);
	cJSON_AddNumberToObject(root, "uuids", uuids);
	cJSON_AddNumberToObject(root, "stats", val);
	out = cJSON_Print(root);
	outLen = strlen(out);
	memcpy(outBuff, out, outLen);
	CJSONFree(out), out = NULL;

	// Ӧ��ֹͣɨ��ģ�����������
	nRet = Procotol::AddPkgHeadAndTail(outBuff, outLen, MES_SET_DEV_PARAM_ACK, 1, 1);
	Send(sockFd, outBuff, nRet, 3000);
	return;
}


void dealWithQurDevParam(int sockFd, char* buff, int buffLen)
{
	cJSON* root = NULL;
	int nRet = 0, outLen = 0;
	char temp[32] = {0};
	char addr[32] = {0};
	char outBuff[128]  = {0};
	char sendBuff[128] = {0};
	char* out = NULL;
	char* pOutBuff  = outBuff;
	char* pSendBuff = sendBuff;
	u_int16 typeID;
	short val = -1;
	u_int16 tmp16;
	int switfNum, uuids, addrType;
	char* bleAddr = NULL, *pBleAddr = NULL;
	//char* values = NULL;

	// ����json
	buff[buffLen] = 0;
	root = cJSON_Parse(buff);
	switfNum = cJSON_GetNumberItem(root, "seq_no");
	bleAddr = cJSON_GetStringItem(root, "ble_addr");
	// ƴ�ӷ�����Ϣ
	pBleAddr = bleAddr;
	nRet = 0;
	while(*pBleAddr != '\0')
	{
		if (*pBleAddr != ':') 
		{
			*pOutBuff++ = *pBleAddr;
		}
		addr[nRet++] = *pBleAddr;
		++pBleAddr;
	}
	addr[nRet] = 0;
	CComTransmit::ASCIIToHex((u_int8*)outBuff, (u_int8*)temp, 6);
	// �豸��ַ
	memcpy(pSendBuff, temp, 6);
	pSendBuff += 6;
	// ��ַ����
	addrType = cJSON_GetNumberItem(root, "addr_type");
	temp[0] = addrType & 0x000000ff;
	*pSendBuff = temp[0];
	pSendBuff++;
	// ����uuid
	uuids = cJSON_GetNumberItem(root, "uuids");
	tmp16 = uuids & 0x0000ffff;
	memcpy(pSendBuff, &tmp16, 2);
	pSendBuff += 2;
	cJSON_Delete(root), root = NULL;

	// ����ֹͣɨ��ģ�����������
	//nRet = sendAndRecvComData(sendBuff, pSendBuff-sendBuff, 
	//			COM_FIND_SERVER_PARAM , switfNum, outBuff, outLen, 3000);
	if (nRet < 0)
	{
		val = -1;
		goto dealWithFailed;
	}

	// ��ȡ������Ϣ��
	pOutBuff += 4;
	typeID = ntohs(*(u_int16*)pOutBuff);
	if (typeID != COM_SET_SERVER_PARAM)
	{
		val = -1;
		goto dealWithFailed;
	}
	pOutBuff += 2;
	val = ntohs(*(short*)pOutBuff);
	switfNum = ntohl(*(int*)outBuff);
	
dealWithFailed:
	root = cJSON_CreateObject();
	cJSON_AddNumberToObject(root, "ack_no", switfNum);
	cJSON_AddNumberToObject(root, "ret_no", val);
	cJSON_AddStringToObject(root, "ble_addr", addr);
	cJSON_AddNumberToObject(root, "addr_type", addrType);
	cJSON_AddNumberToObject(root, "uuids", uuids);
	cJSON_AddNumberToObject(root, "stats", val);
	//cJSON_AddNumberToObject(root, "lengths", n);
	//cJSON_AddStringToObject(root, "values", s);
	out = cJSON_Print(root);
	outLen = strlen(out);
	memcpy(outBuff, out, outLen);
	CJSONFree(out), out = NULL;

	// Ӧ��ֹͣɨ��ģ�����������
	nRet = Procotol::AddPkgHeadAndTail(outBuff, outLen, MES_SET_DEV_PARAM_ACK, 1, 1);
	Send(sockFd, outBuff, nRet, 3000);
	return;
}

void dealWithQurBluInfo(int sockFd, char* buff, int buffLen)
{
	cJSON* root = NULL;
	int switfNum = 0, nRet = 0, outLen = 0;
	char outBuff[128] = {0};
	char* out = NULL;
	char* pOutBuff = outBuff;
	u_int16 typeID;
	short val = -1;
	
	buff[buffLen] = 0;
	root = cJSON_Parse(buff);
	switfNum = cJSON_GetNumberItem(root, "seq_no");
	cJSON_Delete(root), root = NULL;

	// ����ֹͣɨ��ģ�����������
	//nRet = sendAndRecvComData(NULL, 0, COM_STP_SCAN_DEV, switfNum, outBuff, outLen, 3000);
	if (nRet < 0)
	{
		val = -1;
		goto dealWithFailed;
	}

	// ��ȡ������Ϣ��
	pOutBuff += 4;
	typeID = ntohs(*(u_int16*)pOutBuff);
	if (typeID != COM_STP_SCAN_DEV)
	{
		val = -1;
		goto dealWithFailed;
	}
	pOutBuff += 2;
	val = ntohs(*(short*)pOutBuff);
	switfNum = ntohl(*(int*)outBuff);
	
dealWithFailed:
	root = cJSON_CreateObject();
	cJSON_AddNumberToObject(root, "ack_no", switfNum);
	cJSON_AddNumberToObject(root, "ret_no", val);
	out = cJSON_Print(root);
	outLen = strlen(out);
	memcpy(outBuff, out, outLen);
	CJSONFree(out), out = NULL;

	// Ӧ��ֹͣɨ��ģ�����������
	nRet = Procotol::AddPkgHeadAndTail(outBuff, outLen, MES_STP_SCAN_DEV_ACK, 1, 1);
	Send(sockFd, outBuff, nRet, 3000);
	return;
}

void dealWithSetBluInfo(int sockFd, char* buff, int buffLen)
{

}
#endif






/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////

CConWebSerProc::CConWebSerProc():
	m_conWebSerPthreadID(0), 
	m_isExitWebSerPthread(false), 
	m_tcp(NULL)
{
	m_reconnectWifi = 0;
	m_reconnectFlag = 0;
	pthread_rwlock_init(&m_exitLock, NULL);
	IbeaconConfig *conf = GetConfigPosition();
	write_normal_log("%s:%d", conf->getWebDomain(), conf->getWebPort());
	m_tcp = new CSocketTCP(conf->getWebDomain(), conf->getWebPort());
}


int
CConWebSerProc::ProveToServer(int sockFd, const char* uID)
{
	char buff[512] = {0};
	int  nRet;
	
	u_int16 length;
	u_int16 typeID;
	u_int8  PkgTotal, PkgSeq;

	unlink(DEF_PUBLIC_KEY_FILE);

	// ƴ������Կ��Ϣ
	nRet = Procotol::AddPkgHeadAndTail(buff, 0, MES_APY_PUB_KEY, 1, 1);
	if (nRet == -1)
	{
		Debug(I_DEBUG, "ProveToServer Failed.");
		return -1;
	}
	
	// ��������
	length = nRet;
	nRet = Send(sockFd, buff, nRet, 10000);
	if (nRet <= 0)
	{
		Debug(I_DEBUG, "ProveToServer Failed.");
		return nRet;
	}

	// ���ع�Կ
	nRet = Recv(sockFd, buff, HEAD_LEN+4, 10000);
	if (nRet <= 0)
	{
		Debug(I_DEBUG, "ProveToServer Failed. nRet=%d, %m", nRet);
		return nRet;
	}
	Procotol::CheckPkgHead(buff, &length);
	nRet = Recv(sockFd, buff, (int)length, 10000);
	if (nRet <= 0)
	{
		Debug(I_DEBUG, "ProveToServer Failed.nRet=%d, %m", nRet);
		return nRet;
	}
	
	// ������Կ
	nRet = Procotol::DelPkgHeadAndTail(buff, length-TAIL_LEN, &typeID, &PkgTotal, &PkgSeq);
	if (nRet == -1 || typeID != MES_APY_PUB_KEY_ACK)
	{
		Debug(I_DEBUG, "ProveToServer Failed. nRet=%d", nRet);
		return -1;
	}
	LoadMemToFile(buff, nRet, DEF_PUBLIC_KEY_FILE);
	
	// ʹ�ù�Կ���� uid
	EVPRSA *rsa = new EVPRSA();
	nRet = rsa->openPublicKey(DEF_PUBLIC_KEY_FILE);
	if (-1 == nRet)
	{
		Debug(I_DEBUG, "ProveToServer Failed.");
		delete rsa, rsa = NULL;	
		return -1;
	}

	// ʹ�ù�Կ����
	size_t encDataLen = sizeof(buff);
	nRet = rsa->rsaKeyEncrypt((unsigned char*)uID, 
		(sizeof("11:22:33:44:55:66:aa:bb:cc:dd:ee:ff")-1), (u_int8*)buff, encDataLen);
	if (-1 == nRet)
	{
		Debug(I_DEBUG, "ProveToServer Failed.");
		delete rsa, rsa = NULL;	
		return -1;
	}
	delete rsa, rsa = NULL;	

	// �Լ��ܺ�����ݽ���base64
	CBase64Operate base64;
	char data[512] = {0};
	unsigned long dataLen = sizeof(data);
	if ( false == base64.Encrypt(buff, encDataLen, data, dataLen) )
	{
		Debug(I_DEBUG, "ProveToServer Failed.");
		return -1;
	}
	
	// ���ͼ��ܺ��uid
	nRet = Procotol::AddPkgHeadAndTail(data, dataLen, MES_SND_UID_TO_SER, 1, 1);
	if (-1 == nRet)
	{
		Debug(I_DEBUG, "ProveToServer Failed.");
		return -1;
	}
	nRet = Send(sockFd, data, nRet, 10000);
	if (-1 == nRet)
	{
		Debug(I_DEBUG, "ProveToServer Failed.");
		return -1;
	}

	// ����
	nRet = Recv(sockFd, buff, HEAD_LEN+4, 10000);
	if (0 >= nRet)
	{
		Debug(I_DEBUG, "ProveToServer Failed. nRet=%d", nRet);
		return nRet;
	}
	Procotol::CheckPkgHead(buff, &length);
	nRet = Recv(sockFd, buff, length, 3000);
	if (0 >= nRet)
	{
		Debug(I_DEBUG, "ProveToServer Failed. nRet=%d", nRet);
		return nRet;
	}

	// ����
	nRet = Procotol::DelPkgHeadAndTail(buff, length-TAIL_LEN, &typeID, &PkgTotal, &PkgSeq);
	if (-1 == nRet || typeID != MES_SND_UID_TO_SER_ACK)
	{
		Debug(I_DEBUG, "ProveToServer Failed.");
		return -1;
	}
	return 1;
}

void* CConWebSerProc::conWebSerPthread(void* argc)
{
	int heartCnt = 0;
	int sockFd = 0;
	int nRet, contentLen;
	char  buff[1500] = {0};
	char* pBuff = buff;
	char* pContent = NULL;
	u_int16 length = 0;
	CConWebSerProc* webProc = (CConWebSerProc*)argc;

	IbeaconConfig *conf = GetConfigPosition();
	(void)signal(SIGPIPE, SIG_IGN);

	u_int16 typeID; 
	u_int8 PkgTotal; 
	u_int8 PkgSeq;
	
	// �������������洢�������
	ListManage IDHead;
	InitTypeIDList(&IDHead);

CONNECT_SERVER:
	setPthreadcomServerFd(0);
	DelAllTypeIdNode(&IDHead);
	pthread_rwlock_rdlock(&webProc->m_exitLock);
	if (webProc->m_isExitWebSerPthread)
	{
		pthread_rwlock_unlock(&webProc->m_exitLock);
		return (void*)NULL;
	}
	pthread_rwlock_unlock(&webProc->m_exitLock);
	
	sockFd = webProc->m_tcp->Connect(3000, conf->getWebDomain());
	if (sockFd == -1)
	{
		// �Ƿ�����wifi�������������ζ������ϣ��ӳ�ʮ��ʱ�������ӡ�
		if (webProc->m_reconnectWifi++ == 10)
		{
			if (webProc->m_reconnectFlag++ <= 3)
			{
				webProc->m_reconnectWifi = 0;
				checkNetConnect((void*)conf->getWebDomain());
			}
		}
		if (webProc->m_reconnectWifi == 100)
			webProc->m_reconnectWifi = 0, webProc->m_reconnectFlag = 0;
		Debug(I_ERROR, "Connect Web Server Failed. sockFd=%d, domain=%s", sockFd, conf->getWebDomain());
		sleep(3);
		goto CONNECT_SERVER;
	}
	webProc->m_reconnectWifi = 0, webProc->m_reconnectFlag = 0;
	SetSocketStatus(sockFd);
	
	// ������֤
	nRet = ProveToServer(sockFd, conf->getSerials());
	if (nRet <= 0)
	{
		Close(sockFd);
		Debug(I_ERROR, "Prove To Server Failed.");
		sleep(3);
		goto CONNECT_SERVER;
	}
	
	setPthreadcomServerFd(sockFd);
	for (;;)
	{
		pthread_rwlock_rdlock(&webProc->m_exitLock);
		if (webProc->m_isExitWebSerPthread)
		{
			pthread_rwlock_unlock(&webProc->m_exitLock);
			break;
		}
		pthread_rwlock_unlock(&webProc->m_exitLock);

		memset(buff, 0, sizeof(buff));
		nRet = RecvOnePkg(sockFd, buff, sizeof(buff), 6000);
		if (nRet < 0)         // �������Ѿ��رջ������������һ��ʱ������
		{
			if (nRet==-2 || nRet==-3)continue;
			Close(sockFd);
			Debug(I_ERROR, "%m. Reconnect after 3 second");
			heartCnt = 0;
			sleep(3);
			goto CONNECT_SERVER;
		}
		else if (nRet == 0)   // ��ʱ
		{
			if (heartCnt++ == 20)
			{
				heartCnt = 0;
				// ��ʱ����������
				nRet = Procotol::AddPkgHeadAndTail(buff, 0, MES_SND_HEART, 1, 1);
				nRet = Send(sockFd, buff, nRet, 3000);
				if ( 0 >= nRet )
				{
					// ����������ʧ�ܣ���������
					Close(sockFd);
					Debug(I_ERROR, "send failed: %m. returnval=%d", nRet);
					sleep(3);
					goto CONNECT_SERVER;
				}
				Debug(I_DEBUG, "send heart package nRet=%d", nRet);
			}
			continue;
		}
		else
		{
			heartCnt = 0;
		}
		
		pBuff = NULL;
		contentLen = Procotol::DelPkgHeadAndTail(buff, nRet-TAIL_LEN, &typeID, &PkgTotal, &PkgSeq);
		if (contentLen < 0)
		{
			Debug(I_INFO, "Error package.nRet=%d", contentLen);
			Debug(I_INFO, "length=%d, PkgTotal=%d, PkgSeq=%d, typeID=%02x", length, PkgTotal, PkgSeq, typeID);
			continue;
		}
		
		Debug(I_INFO, "length=%d, PkgTotal=%d, PkgSeq=%d, typeID=%02x", contentLen, PkgTotal, PkgSeq, typeID);
		if (PkgTotal != PkgSeq)
		{
			// �����������ֵ���浽������
			AddNodeToIDList(typeID, &IDHead, buff, contentLen, PkgSeq);
			
			// �ж����а��Ƿ��Ѿ�ȫ��������
			//if (false == CheckIDNodeIsComplete(IDHead, typeID, PkgTotal)) // ������ʹ��tcp������ʹ�����
				continue;
		}
		else if ( 1 != PkgTotal && PkgTotal == PkgSeq )
		{
			// ��������һ����
			AddNodeToIDList(typeID, &IDHead, buff, contentLen, PkgSeq);

			// �ж����а��Ƿ��Ѿ�ȫ��������	
			//if (false == CheckIDNodeIsComplete(IDHead, typeID, PkgTotal)) // ������ʹ��tcp������ʹ�����
			//	continue;
		                                                                                                                     	
			// �� IDList ����ȡ�������ַ�����ɾ���� ID �ڵ�
			contentLen = PickUpBuffFromIDList(&IDHead, typeID, &pBuff);
			DelOneNodeByTypeID(&IDHead, typeID);
			if (0 >= contentLen)
			{
				// ��ȡʧ��
				Debug(I_DEBUG, "Pick up Buffer From IDList Failed.");
				if (pBuff) free(pBuff), pBuff = NULL;
				continue;
			}
		}

		if (pBuff)
			pContent = pBuff;
		else
		{
			pContent = buff;
			DelAllTypeIdNode(&IDHead);
		}
		
		switch(typeID)
		{
	#if 0	
		case MES_RST_BLU_DEV:            // ��λ�����豸
			write_normal_log("-------------Rst Blu Dev-------------");
			dealWithRstScanDev(sockFd, pContent, contentLen);
			break;
		case MES_BEG_SCAN_DEV:           // ��ʼɨ��ģ��
			write_normal_log("-------------Beg Scan Dev-------------");
			dealWithBegScanDev(sockFd, pContent, contentLen);
			break;
		case MES_STP_SCAN_DEV:           // ֹͣɨ��ģ��
			write_normal_log("-------------Stp Scan Dev-------------");
			dealWithStpScanDev(sockFd, pContent, contentLen);
			break;
		case MES_SET_DEV_PARAM:          // �����豸����
			write_normal_log("-------------Set Dev Param-------------");
			dealWithSetDevParam(sockFd, pContent, contentLen);   
			break;
		case MES_QUR_DEV_PARAM:          // ��ѯ�豸����
			write_normal_log("-------------Qur Dev Param-------------");
			dealWithQurDevParam(sockFd, pContent, contentLen);
			break;
		case MES_QUR_BLU_INFO:           // ��ѯ������Ϣ
			write_normal_log("-------------Qur Blu Info-------------");
			dealWithQurBluInfo(sockFd, pContent, contentLen);
			break;
		case MES_SET_BLU_INFO:           // ����������Ϣ
			write_normal_log("-------------Set Blu Info-------------");
			dealWithSetBluInfo(sockFd, pContent, contentLen);
			break;
	#endif
	#ifdef SERIALSCOM
		case MES_RST_BLU_DEV ... 0x0113:
			Debug(I_ERROR, "----Server send conmand to serials. typeID=%02x", typeID);
			// pContent[nRet] = 0;
			// �������ݸ�����
			WriteDataToSerials(pContent, contentLen, typeID, 0, true);
			break;		
		case MES_UPL_BLU_CONF_FILE:      // �������·�����ϴ����������ļ�  
			Debug(I_DEBUG, "----upload bluetooth conf file-------------");
			dealWithUplBluConfFile(sockFd, pContent, contentLen);
			break;
		case MES_UPD_BLU_CONF_FILE:      // �������·�����������������ļ�
			Debug(I_DEBUG, "----update bluetooth conf file------------");
			dealWithUpdBluConfFile(sockFd, pContent, contentLen);
			break;
	#endif
		case MES_SER_GET_CBC_INFO:    // ��������ȡCloudBeacon�豸�Ļ�������
			Debug(I_DEBUG, "----Get cloudebeacon info-------------");
			dealWithSerGetCBCinfo(sockFd, pContent);
			break;	
		case MES_SET_SER_INFO:       // ���ÿͻ�������������Ϣ
			Debug(I_DEBUG, "----Set costom service info-----------nRet=%d", contentLen);
			dealWithSetSerInfo(sockFd, pContent, contentLen);
			break;
		case MES_GET_SER_INFO:
			Debug(I_DEBUG, "----Get ser indo-----------");
			dealWithSerInfo(sockFd);
			break;	
		case MES_SET_UPL_INTERVAL:       // ����tcp�ϴ�mac��ַ�ļ��ʱ����ϴ�beacon�ļ��ʱ��
			Debug(I_DEBUG, "----Set upload interval and open------");
			dealWithSetUPLInterval(sockFd, pContent);
			break;
		case MES_VFY_KEY_SER_ACK:        // ������֤�ֻ�key���
		case MES_APL_PHO_BIND_CBC_ACK:   // �����ֻ��˻���cloudbeacon�豸�Ľ��
		case MES_APL_PHO_UNBIND_CBC_ACK: // �����ֻ��˻����cloudbeacon�豸�Ľ��
		case MES_SYN_CBC_CONF_ACK:
			NetWriteDataToPhoneServer(pContent, contentLen, typeID);
			break;
		case MES_APL_PHO_BIND_CBC:       // ͨ�����������а�
			Debug(I_DEBUG, "----bind cloudebeacon---------------");
			dealWithAplPhoBind(sockFd, pContent, contentLen);
			break;
		case MES_APL_PHO_UNBIND_CBC:     // ͨ�����������н��
			Debug(I_DEBUG, "----unbind cloudebeacon-------------");
			dealWithAplPhoUnbind(sockFd, pContent, contentLen);
			break;
		case MES_UPDATE_CB_PRO:
			UpdateCBProgram(sockFd, MES_UPDATE_CB_PRO_ACK, pContent, contentLen);
			break;
	
		case MES_SND_HEART:
			break;
		case MES_UPL_PHO_MAC_INFO_ACK:
			break;
		default:
			Debug(I_DEBUG, "Recv Unknown Package. typeID=%02x", typeID);
		}

		if (pBuff) free(pBuff), pBuff = NULL;
	}

	UninitTypeIDList(&IDHead);
	// �ر� socket
	Close(sockFd);
	Debug(I_ERROR, "Connect Web Server Pthread Exit...");
	return (void*)NULL;
}



/*
{
	"name":"Return CloudBeacon Basic Param",
	"values":{
		"wan_mac_adr":"78:01:6C:06:A6:29", #cloudbeacon��mac��ַ
		"version": int, #cloudbeaconӦ�ó���汾��
	}
}
*/
char* CConWebSerProc::MakeSerGetCBCInfoAck()
{
	char tmp[512] = {0};
	IbeaconConfig *conf = GetConfigPosition();
	cJSON *root,*info;
	root = cJSON_CreateObject();	
	cJSON_AddItemToObject(root, "name", cJSON_CreateString("Return CloudBeacon Basic Param"));
	cJSON_AddItemToObject(root, "values", info = cJSON_CreateObject());

	cJSON_AddStringToObject(info, "wan_mac_adr", GetLanMac());
	cJSON_AddNumberToObject(info, "version", conf->getVersion());
	snprintf(tmp, sizeof(tmp), "%s %s", __VERDATA__, __VERTIME__);
	cJSON_AddStringToObject(info, "makeTime", tmp);

	char* out = cJSON_PrintUnformatted(root);
	cJSON_Delete(root);	
	return out;
}


void 
CConWebSerProc::dealWithSerGetCBCinfo(int sockfd, char* buff)
{
	int outLen;
	char* ptemp = MakeSerGetCBCInfoAck();
	if (! ptemp)
	{
		Debug(I_DEBUG, "Make cloude beacon info failed.");
		return;
	}
	
	outLen = strlen(ptemp);
	memcpy(buff, ptemp, outLen);
	CJSONFree(ptemp), ptemp = NULL;
	
	outLen = Procotol::AddPkgHeadAndTail(buff, outLen, MES_SER_GET_CBC_INFO_ACK, 1, 1);
	if (-1 == outLen)
	{
		Debug(I_DEBUG, "Add package haad and tail failed.");
		return;
	}

	// ��������
	Send(sockfd, buff, outLen, 3000);
}


void
CConWebSerProc::dealWithSetSerInfo(int sockfd, char* buff, int buffLen)
{
	IbeaconConfig* conf = GetConfigPosition();
	char temp[16] = {0};
	u_int16 portVal;
	time_t timeVal;
	cJSON *root = NULL;
	char *host  = NULL, *url = NULL; 
	char sendbuff[128] = {0};
	int sendbuffLen;
	int returnVal  = 0;
	int isOpen = 0;
	int tcpMacOpen, tcpBeaconOpen, tcpMacInter, tcpBeaconInter;

	CConfFile* confFile = new CConfFile();
	conf->lockConfFile();
	if (!confFile->LoadFile(conf->getConfFilePath()))
	{
		conf->UnlockConfFile();
		delete confFile, confFile = NULL;
		returnVal = 4;
		Debug(I_ERROR, "load config file failed.");
		goto SetFailed;
	}
	conf->UnlockConfFile();

	buff[buffLen] = 0;
	//printf("cJSON=%s\n", buff);
	root = cJSON_Parse(buff);
	if (NULL == root)
	{
		returnVal = 2;	
		delete confFile, confFile = NULL;
		goto SetFailed;
	}
	isOpen = cJSON_GetNumberItem(root, "POST_MacSerOpen");
	if (isOpen == 1)
	{
		portVal = cJSON_GetNumberItem(root, "POST_MacSerPort");
		timeVal = cJSON_GetNumberItem(root, "POST_MacSerInterval");
		host = cJSON_GetStringItem(root, "POST_MacSerHost");
		url  = cJSON_GetStringItem(root, "POST_MacSerUrl");
		if ( !host || !url || portVal < 1 || timeVal < 1 )
		{
			Debug(I_DEBUG, "Some Value is Error.");
			delete confFile, confFile = NULL;
			cJSON_Delete(root), root = NULL;
			returnVal = 11;
			goto setBeacon;
		}
	
		// �޸������ļ�
		confFile->SetValue("CustomServerInfo", "MacSerHost", host, "#�ͻ�Mac��������ַ");
		confFile->SetValue("CustomServerInfo", "MacSerUrl", url, "#�ͻ�Mac������url");
		snprintf(temp, 8, "%d", portVal);
		confFile->SetValue("CustomServerInfo", "MacSerPort", temp, "#�ͻ�Mac�������˿�");
		snprintf(temp, 16, "%ld", timeVal);
		//confFile->SetValue("CustomServerInfo", "MacSerInterval", temp, "#�ͻ�Mac�������ϴ����ʱ��(��λ:s)");
		confFile->SetValue("CustomServerInfo", "MacInterval", temp, "#Mac�������ϴ����ʱ��(��λ:s)");
		temp[0] = '1', temp[1] = 0;
		confFile->SetValue("CustomServerInfo", "OpenMacSer", temp, "#����Mac������");
		
		// �޸�������Ϣ
		conf->setIsOpenMacSer(true);
		conf->setMacSerHost(host);
		conf->setMacSerUrl(url);
		conf->setMacSerPort(portVal);
		conf->setMacInterval(timeVal);
	}
	else
	{
		temp[0] = '0', temp[1] = 0;
		confFile->SetValue("CustomServerInfo", "OpenMacSer", temp, "#�ر�Mac������");
		conf->setIsOpenMacSer(false);
		write_normal_log("close POST Mac Server.");
	}	

setBeacon:
	isOpen = cJSON_GetNumberItem(root, "POST_BeaconSerOpen");
	if (isOpen == 1)  // ��
	{
		portVal = cJSON_GetNumberItem(root, "POST_BeaconSerPort");
		timeVal = cJSON_GetNumberItem(root, "POST_BeaconSerInterval");
		host = cJSON_GetStringItem(root, "POST_BeaconSerHost");
		url  = cJSON_GetStringItem(root, "POST_BeaconSerUrl");
		if ( !host || !url || portVal < 1 || timeVal < 1 )
		{
			Debug(I_DEBUG, "Some Value is Error.");
			delete confFile, confFile = NULL;
			cJSON_Delete(root), root = NULL;
			returnVal == 11 ? returnVal = 13 : returnVal= 12;
			goto SetTcp;
		}

		confFile->SetValue("CustomServerInfo", "BeaconSerHost", host, "#�ͻ�Beacon��������ַ");
		confFile->SetValue("CustomServerInfo", "BeaconSerUrl", url, "#�ͻ�Beacon������url");
		snprintf(temp, 8, "%d", portVal);
		confFile->SetValue("CustomServerInfo", "BeaconSerPort", temp, "#�ͻ�Beacon�������˿�");
		snprintf(temp, 16, "%ld", timeVal);
		//confFile->SetValue("CustomServerInfo", "BeaconInterval", temp, "#�ͻ�Beacon�������ϴ����ʱ��(��λ:s)");
		confFile->SetValue("CustomServerInfo", "BeaconInterval", temp, "#Beacon�������ϴ����ʱ��(��λ:s)");
		temp[0] = '1', temp[1] = 0;
		confFile->SetValue("CustomServerInfo", "OpenBeaconSer", temp, "#����Beacon������");
		
		// �޸�������Ϣ
		conf->setIsOpenBeaconSer(true);
		conf->setBeaconSerHost(host);
		conf->setBeaconSerUrl(url);
		conf->setBeaconSerPort(portVal);
		conf->setBeaconInterval(timeVal);
	}
	else
	{
		temp[0] = '0', temp[1] = 0;
		confFile->SetValue("CustomServerInfo", "OpenBeaconSer", temp, "#�ر�Beacon������");
		conf->setIsOpenBeaconSer(false);
		Debug(I_INFO, "close POST Beacon Server.");
	}

SetTcp:
	tcpMacOpen = cJSON_GetNumberItem(root, "TCP_MacSerOpen");
	tcpBeaconOpen = cJSON_GetNumberItem(root, "TCP_BeaconSerOpen");
	tcpMacInter = cJSON_GetNumberItem(root, "TCP_MacSerInterval");
	tcpBeaconInter = cJSON_GetNumberItem(root, "TCP_BeaconSerInterval");
	//printf("tcpMacOpen=%d\n", tcpMacOpen);
	//printf("tcpBeaconOpen=%d\n", tcpBeaconOpen);
	//printf("tcpMacSerInter=%d\n", tcpMacInter);
	//printf("tcpBeaconSerInter=%d\n", tcpBeaconInter);
	if (tcpMacOpen != 1)
	{
		temp[0] = '0', temp[1] = 0;
		confFile->SetValue("LocalServerInfo", "OpenUploadMacInfo", temp, "#�ر�tcp�ϴ�mac��ַ����");
		conf->setTCPMacSerOpenVal(false);
	}
	else
	{
		temp[0] = '1', temp[1] = 0;
		confFile->SetValue("LocalServerInfo", "OpenUploadMacInfo", temp, "#����tcp�ϴ�mac��ַ����");
		conf->setTCPMacSerOpenVal(true);
	}
	if (tcpBeaconOpen != 1)
	{
		temp[0] = '0', temp[1] = 0;
		confFile->SetValue("LocalServerInfo", "OpenUploadBeaconInfo", temp, "#�ر�tcp�ϴ�beaconɨ����Ϣ����");
		conf->setTCPBeaconSerOpenVal(false); 
	}
	else
	{
		temp[0] = '1', temp[1] = 0;
		confFile->SetValue("LocalServerInfo", "OpenUploadBeaconInfo", temp, "#����tcp�ϴ�beaconɨ����Ϣ����");
		conf->setTCPBeaconSerOpenVal(true); 
	}
	if (tcpMacInter == -1 || tcpBeaconInter == -1)
	{
		returnVal = 14;
	}
	else
	{
		snprintf(temp, sizeof(temp), "%d", tcpMacInter);
		confFile->SetValue("LocalServerInfo", "MacInterval", temp, "#�ϴ�Mac��ַ���ʱ��");
		snprintf(temp, sizeof(temp), "%d", tcpBeaconInter);
		confFile->SetValue("LocalServerInfo", "BeaconInterval", temp, "#�ϴ�beaconɨ����Ϣ���ʱ��");

		conf->setMacInterval(tcpMacInter);
		conf->setBeaconInterval(tcpBeaconInter);
	}

	conf->lockConfFile();
	confFile->Save();
	conf->UnlockConfFile();
	
	delete confFile, confFile = NULL;
	cJSON_Delete(root), root = NULL;

SetFailed:
	sendbuffLen = snprintf(sendbuff, 128, "{\"returnVal\": %d}", returnVal);
	sendDataByTCP(sockfd, sendbuff, sendbuffLen, MES_SET_SER_INFO_ACK);
	return ;
}

void
CConWebSerProc::dealWithSerInfo(int sockfd)
{
	IbeaconConfig* conf = GetConfigPosition();

	int SerOpen, SerPort, SerInterval, sendbuffLen;
	const char* SerHost, *SerUrl;
	cJSON *root;
	char* out = NULL;
	
	root = cJSON_CreateObject();
	SerOpen = (conf->getIsOpenMacSer() ? 1 : 0);
	SerHost = conf->getMacSerHost();
	SerUrl  = conf->getMacSerUrl();
	SerPort = conf->getMacSerPort();
	SerInterval = conf->getMacInterval();
	cJSON_AddNumberToObject(root, "POST_MacSerOpen", SerOpen);
	cJSON_AddStringToObject(root, "POST_MacSerHost", SerHost);
	cJSON_AddStringToObject(root, "POST_MacSerUrl", SerUrl);
	cJSON_AddNumberToObject(root, "POST_MacSerPort", SerPort);
	cJSON_AddNumberToObject(root, "POST_MacSerInterval", SerInterval);
	cJSON_AddNumberToObject(root, "POST_MacInterval", SerInterval);

	SerOpen = (conf->getIsOpenBeaconSer() ? 1 : 0);
	SerHost = conf->getBeaconSerHost();
	SerUrl  = conf->getBeaconSerUrl();
	SerPort = conf->getBeaconSerPort();
	SerInterval = conf->getBeaconInterval();
	cJSON_AddNumberToObject(root, "POST_BeaconSerOpen", SerOpen);
	cJSON_AddStringToObject(root, "POST_BeaconSerHost", SerHost);
	cJSON_AddStringToObject(root, "POST_BeaconSerUrl",  SerUrl);
	cJSON_AddNumberToObject(root, "POST_BeaconSerPort", SerPort);
	cJSON_AddNumberToObject(root, "POST_BeaconSerInterval", SerInterval);
	cJSON_AddNumberToObject(root, "POST_BeaconInterval", SerInterval);

	SerHost = conf->getWebDomain();
	SerPort = conf->getWebPort();
	SerOpen = (conf->getTCPMacSerOpenVal() ? 1 : 0);
	SerInterval = conf->getMacInterval();
	cJSON_AddNumberToObject(root, "TCP_MacSerOpen", SerOpen);
	cJSON_AddStringToObject(root, "TCP_MacSerHost", SerHost);
	cJSON_AddNumberToObject(root, "TCP_MacSerPort", SerPort);
	cJSON_AddNumberToObject(root, "TCP_MacSerInterval", SerInterval);

	SerOpen = (conf->getTCPBeaconSerOpenVal() ? 1: 0);
	SerInterval = conf->getBeaconInterval();
	cJSON_AddNumberToObject(root, "TCP_BeaconSerOpen", SerOpen);
	cJSON_AddStringToObject(root, "TCP_BeaconSerHost", SerHost);
	cJSON_AddNumberToObject(root, "TCP_BeaconSerPort", SerPort);
	cJSON_AddNumberToObject(root, "TCP_BeaconSerInterval", SerInterval);
	
	out = cJSON_PrintUnformatted(root);	
	cJSON_Delete(root);	
	sendDataByTCP(sockfd, out, strlen(out), MES_GET_SER_INFO_ACK);
	CJSONFree(out), out = NULL;
	return ;	
}


void 
CConWebSerProc::UploadDataToNet(int sockFd, const char* data, int dataLen, u_int16 typeID)
{
	int PkgSeq, PkgTotal, sendLen, nSend, len;
	char temp[EVERY_PKG_LEN+DEF_PRO_HEAD_LEN] = {0};
	
	PkgTotal = dataLen / EVERY_PKG_LEN + 1;
	if (dataLen % EVERY_PKG_LEN == 0) --PkgTotal;
	for (sendLen=0, PkgSeq=1; PkgSeq <= PkgTotal; ++PkgSeq)
	{
		if (PkgSeq == PkgTotal)
			nSend = dataLen % EVERY_PKG_LEN;
		else
			nSend = EVERY_PKG_LEN;

		memcpy(temp, data+sendLen, nSend);
		len = Procotol::AddPkgHeadAndTail(temp, nSend, typeID, PkgTotal, PkgSeq);
		sendLen += nSend;

		Send(sockFd, temp, len, 3000);
	}
}



void
CConWebSerProc::dealWithSetUPLInterval(int sockfd, char* buff)
{
	IbeaconConfig* conf = GetConfigPosition();
	cJSON *root;
	time_t beaconTime, macTime;
	int isUploadMacInfo, isUploadBeaconInfo;
	char temp[16] = {0};
	int val = 1;
	char sendbuff[128] = {0};
	int sendbuffLen;

	root = cJSON_Parse(buff);
	if (!root)
	{
		Debug(I_ERROR, "JSON Parse Failed...");
		return;
	}

	CConfFile* confFile = new CConfFile();
	conf->lockConfFile();
	if (!confFile->LoadFile(conf->getConfFilePath()))
	{
		Debug(I_ERROR, "Open config file %s failed.", conf->getConfFilePath());
		delete confFile, confFile = NULL;
		conf->UnlockConfFile();
		val = 0;
		goto SetFailed;
	}
	conf->UnlockConfFile();
	
	isUploadMacInfo = cJSON_GetNumberItem(root, "UploadMacInfo");
	if (0 == isUploadMacInfo)
	{
		temp[0] = '0', temp[1] = 0;
		confFile->SetValue("LocalServerInfo", "OpenUploadMacInfo", temp, "#�ر��ϴ�mac��ַ����");
		conf->setTCPMacSerOpenVal(false);
	}
	else
	{
		temp[0] = '1', temp[1] = 0;
		confFile->SetValue("LocalServerInfo", "OpenUploadMacInfo", temp, "#�����ϴ�mac��ַ����");
		conf->setTCPMacSerOpenVal(true);
	}
	
	isUploadBeaconInfo = cJSON_GetNumberItem(root, "UploadBeaconInfo");
	if (0 == isUploadBeaconInfo)
	{
		temp[0] = '0', temp[1] = 0;
		confFile->SetValue("LocalServerInfo", "OpenUploadBeaconInfo", temp, "#�ر��ϴ�beacon����");
		conf->setTCPBeaconSerOpenVal(false);
	}
	else
	{
		temp[0] = '1', temp[1] = 0;
		confFile->SetValue("LocalServerInfo", "OpenUploadBeaconInfo", temp, "#�����ϴ�beacon����");
		conf->setTCPBeaconSerOpenVal(true);
	}
		
	macTime = cJSON_GetNumberItem(root, "MacInterval");
	beaconTime = cJSON_GetNumberItem(root, "BeaconInterval");
	if (macTime < 1 || beaconTime < 1)
	{
		delete confFile, confFile = NULL;
		cJSON_Delete(root);
		val = 0;
		goto SetFailed;
	}
	// �޸������ļ�
	snprintf(temp, 16, "%ld", macTime);
	confFile->SetValue("LocalServerInfo", "MacInterval", temp, "#�ϴ�mac��ַ���ʱ��");
	snprintf(temp, 16, "%ld", beaconTime);
	confFile->SetValue("LocalServerInfo", "BeaconInterval", temp, "#�ϴ�beacon��Ϣ���ʱ��");
	
	// �޸�������Ϣ
	conf->setMacInterval(macTime);
	conf->setBeaconInterval(beaconTime);
	if (0 == isUploadMacInfo)
		conf->setTCPMacSerOpenVal(false);
	else
		conf->setTCPMacSerOpenVal(true);
	if (0 == isUploadBeaconInfo)
		conf->setTCPBeaconSerOpenVal(false);
	else
		conf->setTCPBeaconSerOpenVal(true);

	conf->lockConfFile();
	confFile->Save();
	conf->UnlockConfFile();
	
	delete confFile, confFile = NULL;
	cJSON_Delete(root);

SetFailed:
	sendbuffLen = snprintf(sendbuff, 1024, "{\"returnVal\": %d}", val);
	sendDataByTCP(sockfd, sendbuff, sendbuffLen, MES_SET_UPL_INTERVAL_ACK);
	return ;
}


// �������·�����: ���ֻ��˺�
void
CConWebSerProc::dealWithAplPhoBind(int sockFd, char* buff, int buffLen)
{
	IbeaconConfig *conf = GetConfigPosition();
	char* phoneKey = NULL;
	cJSON* root = NULL;
	char temp[256] = {0};
	int nRet;
	int returnVal = 0;

	buff[buffLen] = 0;
	root = cJSON_Parse(buff);
	if (!root)
	{
		Debug(I_ERROR, "JSON Parse Failed...");
		return;
	}
	phoneKey = cJSON_GetStringItem(root, "phoneKey");
	if (phoneKey)
	{
		unlink(DEF_PHONE_CONNECT_KEY);
		if (-1 == LoadMemToFile(phoneKey, strlen(phoneKey), DEF_PHONE_CONNECT_KEY) )
		{
			Debug(I_DEBUG, "Load Memory To File Failed.");
			returnVal = 1;
			goto dealWithFailed;
		}
	}
	else
	{
		Debug(I_DEBUG, "Get PhoneKey is NULL.");
		returnVal = 1;
		goto dealWithFailed;
	}

	// ͬ���߳�����
	conf->setPhoneKey(phoneKey);

dealWithFailed:
	// ͨ����������󶨳ɹ�
	nRet = snprintf(temp, sizeof(temp), "{\"returnVal\": %d}", returnVal);
	sendDataByTCP(sockFd, temp, nRet, MES_APL_PHO_BIND_CBC_ACK);
	cJSON_Delete(root), root = NULL;
}


// �������·�����: ����ֻ��˺�
void
CConWebSerProc::dealWithAplPhoUnbind(int sockFd, char* buff, int buffLen)
{
	IbeaconConfig *conf = GetConfigPosition();
	cJSON* root    = NULL;
	char* phoneKey = NULL;
	char* fileKey  = NULL;
	char temp[256] = {0};
	int nRet;
	int returnVal = 0;

	buff[buffLen] = 0;
	root = cJSON_Parse(buff);
	if (! root)
	{
		Debug(I_ERROR, "cJSON_Parse failed.buff=%s", buff);
		//printf("%s\n", buff);
		return ;
	}
	phoneKey = cJSON_GetStringItem(root, "phoneKey");
	unlink(DEF_PHONE_CONNECT_KEY);
	conf->setPhoneKey(NULL);
	cJSON_Delete(root), root = NULL;
	
	nRet = snprintf(temp, sizeof(temp), "{\"returnVal\": %d}", returnVal);
	sendDataByTCP(sockFd, temp, nRet, MES_APL_PHO_UNBIND_CBC_ACK);
}


// �������·�����ϴ����������ļ�
void
CConWebSerProc::dealWithUplBluConfFile(int sockFd, char* buff, int buffLen)
{
	// ��ȡ�����ļ���
	cJSON* root = NULL;
	char* file  = NULL;
	char temp[EVERY_PKG_LEN+DEF_PRO_HEAD_LEN] = {0};
	char* content = NULL;
	int nRet;
	
	buff[buffLen] = 0;
	root = cJSON_Parse(buff);
	if (!root)
	{
		Debug(I_ERROR, "JSON Parse Failed...");
		return;
	}
	file = cJSON_GetStringItem(root, "FileName");
	if (file)
	{
		//snprintf(temp, EVERY_PKG_LEN+DEF_PRO_HEAD_LEN, DEF_BLU_CONF_DIR"%s", file);
		strncpy(temp, DEF_BLU_CONF_FILE, EVERY_PKG_LEN+DEF_PRO_HEAD_LEN);
		if (0 == access(temp, F_OK))
		{
			content = LoadFileToMem(temp, &nRet);
			if (! content)
			{
				Debug(I_DEBUG, "Load File To Memory Failed.");
				cJSON_AddNullToObject(root, "Content");
			}
			else
			{
				cJSON_AddStringToObject(root, "Content", content);
			}
		}
		else
		{
			cJSON_AddNullToObject(root, "Content");
		}
	}
	else
	{
		cJSON_AddNullToObject(root, "Content");
	}

	free(content), content = NULL;
	content = cJSON_Print(root);
	cJSON_Delete(root), root = NULL;

	sendDataByTCP(sockFd, content, strlen(content), MES_UPL_BLU_CONF_FILE_ACK);
	CJSONFree(content), content = NULL;
	return ;
}

#ifdef SERIALSCOM
// �������·�����������������ļ�
void
CConWebSerProc::dealWithUpdBluConfFile(int sockFd, char* buff, int buffLen)
{
	cJSON* root = NULL;
	char *file = NULL, *content = NULL;
	char temp[256] = {0};
	int nRet = 0;
	int returnVal = 0;

	buff[buffLen] = 0;
	root = cJSON_Parse(buff);
	if (root == NULL) 
	{
		returnVal = 2;
		goto setFailed;
	}
	
	file    = cJSON_GetStringItem(root, "FileName");     // ȡ��
	content = cJSON_GetStringItem(root, "Content");
	if (content)
	{
		LoadMemToFile(content, strlen(content), DEF_BLU_CONF_FILE);
		// ���¼��������ļ�
		if ( -1 == reloadBlueConfFile())
			returnVal = 3;
		else 
			returnVal = 0;
	}
	else
	{
		returnVal = 2;
	}

setFailed:
	nRet = snprintf(temp, sizeof(temp), "{\"returnVal\": %d}", returnVal);
	sendDataByTCP(sockFd, temp, nRet, MES_UPD_BLU_CONF_FILE_ACK);
	cJSON_Delete(root), root = NULL;
}
#endif









