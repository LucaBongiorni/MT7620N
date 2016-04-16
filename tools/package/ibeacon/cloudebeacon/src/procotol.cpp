#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>



#include "procotol.h"
#include "defCom.h"
#include "cJSON.h"
#include "cbProjMain.h"
#include "netSockTcp.h"
#include "base64RSA.h"


void 
Procotol::printHex(const char* buf, int len)
{
    int i;
    if (NULL == buf)
    {
        return;
    }
    for (i=0; i<len; ++i)
    {
        printf("%02x ", buf[i]);
        if ( ((i+1)%32 == 0) && i )
        {
            printf("\n");
        }
    }
    if (i)
    {
        printf("\n");
    }
    return ;
}


// 函数功能: unsigned int十六进制转字符
void 
Procotol::Uint32HexTostr(u_int32 hex, char* str)
{
	u_int8 temp = 0;
	int i;

	for (i=0; i<8; ++i)
	{
		temp = (hex >> (7-i)*4) & 0x0000000f;
		if (temp < 10)
			str[i] = temp + '0';
		else 
			str[i] = temp + 'a' - 10;
	}
	return ;
}

// 函数功能: 字符转unsigned int十六进制，转8个字节
int 
Procotol::StrToUint32Hex(u_int32* hex, const char* str)
{
	u_int8 temp;
	int i;

	*hex = 0;
	for (i=0; i<8; ++i)
	{
		if ('0' <= str[i] && '9' >= str[i])
		{
			temp = str[i] - '0';
		}
		else if ('a' <= str[i] && 'f' >= str[i])
		{
			temp = str[i] - 'a' + 10;
		}
		else if ('A' <= str[i] && 'F' >= str[i])
		{
			temp = str[i] - 'A' + 10;
		}
		else
		{
			return -1;
		}
		*hex = *hex | (temp << (7-i)*4);
	}
	return 0;
}

// 十六进制转字符，转两个字节
void 
Procotol::Uint16HexToStr(u_int16 hex, char *str)
{
	u_int8 temp;
	int i;

	for (i=0; i<4; ++i)
	{
		temp = (hex >> (3-i)*4) & 0x000f;
		if (temp < 10)
			str[i] = temp + '0';
		else 
			str[i] = temp + 'a' - 10;
	}
	return ;
}

// 字符转十六进制，转四个字节
int 
Procotol::StrToUint16Hex(u_int16* hex, const char* str)
{
	u_int8 temp;
	int i;

	*hex = 0;
	for (i=0; i<4; ++i)
	{
		if ('0' <= str[i] && '9' >= str[i])
		{
			temp = str[i] - '0';
		}
		else if ('a' <= str[i] && 'f' >= str[i])
		{
			temp = str[i] - 'a' + 10;
		}
		else if ('A' <= str[i] && 'F' >= str[i])
		{
			temp = str[i] - 'A' + 10;
		}
		else
		{
			return -1;
		}
		*hex = *hex | (temp << (3-i)*4);
	}
	return 0;
}


// 十六进制转字符，转两个字节
void 
Procotol::Uint8HexToStr(u_int8 hex, char *str)
{
	u_int8 temp;
	int i;

	for (i=0; i<2; ++i)
	{
		temp = (hex >> (1-i)*4) & 0x0f;
		if (temp < 10)
			str[i] = temp + '0';
		else 
			str[i] = temp + 'a' - 10;
	}
	return ;
}

// 字符转十六进制，转四个字节
int 
Procotol::StrToUint8Hex(u_int8* hex, const char* str)
{
	u_int8 temp;
	int i;

	*hex = 0;
	for (i=0; i<2; ++i)
	{
		if ('0' <= str[i] && '9' >= str[i])
		{
			temp = str[i] - '0';
		}
		else if ('a' <= str[i] && 'f' >= str[i])
		{
			temp = str[i] - 'a' + 10;
		}
		else if ('A' <= str[i] && 'F' >= str[i])
		{
			temp = str[i] - 'A' + 10;
		}
		else
		{
			return -1;
		}
		*hex = *hex | (temp << (1-i)*4);
	}
	return 0;
}


// 加包头和包尾
int 
Procotol::AddPkgHeadAndTail(char* content, u_int16 contentLen, u_int16 typeID, 
	char PkgTotal, char PkgSeq)
{
	char temp[1428] = {0};
	//char checkSum[1428] = {0};
	char* ptemp  = NULL;
	char* pCheck = temp + sizeof(int) + HEAD_LEN;  	// 计算校验和的起始位
	u_int16 length;

	if (NULL == content || contentLen > EVERY_PKG_LEN)
	{
		printf("[%s][%d]Error: Parameter is Error.\n", __FILE__,__LINE__);
		return -1;
	}

	ptemp = temp;
	memcpy(ptemp, &HEAD, HEAD_LEN);		// 填充包头
	ptemp += HEAD_LEN;

	length = contentLen + 14;			// 填充第一层协议的长度
	Uint16HexToStr(length, ptemp);
	ptemp += 4;

	Uint16HexToStr(typeID, ptemp);   	// 数据类型
	ptemp += 4; 
	
	length = contentLen;				// 填充第二层协议的长度
	Uint16HexToStr(length, ptemp);
	ptemp += 4;

	Uint8HexToStr(PkgTotal, ptemp);	  	// 总共多少个包
	ptemp += 2;
	Uint8HexToStr(PkgSeq, ptemp);	  	// 第几个包
	ptemp += 2;

	if (contentLen > 0)                // 填充数据内容
	{
		memcpy(ptemp, content, contentLen); 
		ptemp += contentLen;
	}

	// 计算校验和
	u_int8 i = 0;
	for (; pCheck<ptemp; ++pCheck)
	{
		i ^= *pCheck;
	}
	//printf("checkNum=%d, 0xaa=%d\n", CHECK-i, 0xaa);
	Uint8HexToStr(i, ptemp);			// 填充校验和
	ptemp += 2;

	// 填充包尾
	memcpy(ptemp, &TAIL, TAIL_LEN);
	ptemp += TAIL_LEN;
	
	// 通过参数传出
	memcpy(content, temp, ptemp-temp);
	return ptemp-temp;
}

int 
Procotol::NetAddPkgHeadAndTail(char* content, u_int16 contentLen, u_int16 typeID, 
	char PkgTotal, char PkgSeq)
{
	char temp[1428] = {0};
	//char checkSum[1428] = {0};
	char* ptemp  = NULL;
	char* pCheck = temp + sizeof(int) + HEAD_LEN;  	// 计算校验和的起始位
	u_int16 length;

	if (NULL == content || contentLen > EVERY_PKG_LEN)
	{
		printf("[%s][%d]Error: Parameter is Error.\n", __FILE__,__LINE__);
		return -1;
	}

	ptemp = temp + AddLength;
	memcpy(ptemp, &HEAD, HEAD_LEN);		// 填充包头
	ptemp += HEAD_LEN;

	length = contentLen + 14;			// 填充第一层协议的长度
	Uint16HexToStr(length, ptemp);
	ptemp += 4;

	Uint16HexToStr(typeID, ptemp);   	// 数据类型
	ptemp += 4; 
	
	length = contentLen;				// 填充第二层协议的长度
	Uint16HexToStr(length, ptemp);
	ptemp += 4;

	Uint8HexToStr(PkgTotal, ptemp);	  	// 总共多少个包
	ptemp += 2;
	Uint8HexToStr(PkgSeq, ptemp);	  	// 第几个包
	ptemp += 2;

	if (contentLen > 0)                // 填充数据内容
	{
		memcpy(ptemp, content, contentLen); 
		ptemp += contentLen;
	}

	// 计算校验和
	u_int8 i = 0;
	for (; pCheck<ptemp; ++pCheck)
	{
		i ^= *pCheck;
	}
	//printf("checkNum=%d, 0xaa=%d\n", CHECK-i, 0xaa);
	Uint8HexToStr(i, ptemp);			// 填充校验和
	ptemp += 2;

	// 填充包尾
	memcpy(ptemp, &TAIL, TAIL_LEN);
	ptemp += TAIL_LEN;
	
	// 通过参数传出
	*(int*)temp  = htonl(ptemp-temp-AddLength);
	memcpy(content, temp, ptemp-temp);
	printf("%d, [%d,%d,%d,%d]\n", ptemp-temp-AddLength, *temp, *(temp+1), *(temp+2), *(temp+3));
	return ptemp-temp;
}


int 
Procotol::NetCheckPkgHead(const char* content, u_int16* length)
{
	if (NULL == content) return -1;
	const char* pcontent = content + AddLength;

	if ( 0 != memcmp(pcontent, HEAD, HEAD_LEN-AddLength) )
	{
		write_error_log("Error package.%x %x", pcontent[0], pcontent[1]);
		return -1;
	}
	u_int16 temp = 0;
	if ( -1 == StrToUint16Hex(&temp, pcontent+HEAD_LEN-AddLength) )
	{
		// 出错处理
		write_error_log("Error package.");
		return -1;
	}
	*length = temp + TAIL_LEN;
	return 0;
}





// 去包头和包尾，这里的content已经剥除了第一层
int 
Procotol::DelPkgHeadAndTail(char* content, int contentLen, 
			u_int16 *typeID, u_int8* PkgTotal, u_int8* PkgSeq)
{	
	char* pContent = content;
	u_int16 length = 0;
	u_int8 checkNum = 0;
	u_int8 nRet = 0;
	
	// 检测校验和
	for (int i=0; i<contentLen-2; ++i)
	{
		checkNum ^= content[i];
	}
	pContent = content + contentLen - 2;
	StrToUint8Hex(&nRet, pContent);
	if (nRet != checkNum)
	{
		write_error_log("checksum error. checksum=%d, 0xAA=%d, nRet=%d[%c][%c]", 
			checkNum, 0xaa, nRet, pContent[0], pContent[1]);
		return -1;
	}

	// typeID
	pContent = content;
	StrToUint16Hex(typeID, pContent);
	pContent += 4;

	// 长度
	StrToUint16Hex(&length, pContent);
	pContent += 4;
	//printf("length=%d\n", length);

	// 总包数
	StrToUint8Hex(PkgTotal, pContent);
	pContent += 2;

	// 分包数
	StrToUint8Hex(PkgSeq, pContent);
	pContent += 2;

	for (int i=0; i<length; ++i)
	{
		content[i] = pContent[i];
	}
	content[length] = 0;
	return length;
}


int Procotol::CheckPkgHead(const char* content, u_int16* length)
{
	if (NULL == content) return -1;

	if ( 0 != memcmp(content, HEAD, HEAD_LEN) )
	{
		write_error_log("Error package.%x %x", content[0], content[1]);
		return -1;
	}
	u_int16 temp = 0;
	if ( -1 == StrToUint16Hex(&temp, content+HEAD_LEN) )
	{
		// 出错处理
		write_error_log("Error package.");
		return -1;
	}
	*length = temp + TAIL_LEN;
	return 0;
}

char* Procotol::MakeBcastMes()
{
	IbeaconConfig *conf = GetConfigPosition();
	cJSON *root,*info;
	root = cJSON_CreateObject();	
	cJSON_AddItemToObject(root, "name", cJSON_CreateString("Get Device Info"));
	cJSON_AddItemToObject(root, "Info", info = cJSON_CreateObject());
	cJSON_AddStringToObject(info, "WanIP", conf->getWanIP());
	cJSON_AddStringToObject(info, "WanMac", conf->getWanMAC());
	cJSON_AddStringToObject(info, "LanIP", conf->getLanIP());
	cJSON_AddStringToObject(info, "LanMac", conf->getLanMAC());
	cJSON_AddNumberToObject(info, "LocalPort", conf->getLocalPort());

	char* out = cJSON_Print(root);	
	cJSON_Delete(root);	

	return out;
}


/*
{
	"name":"Return CloudBeacon Basic Param",
	"values":{
		"wan_mac_adr":"78:01:6C:06:A6:29", #cloudbeacon的mac地址
		"version": int       #cloudbeacon应用程序版本号
	}
}
*/
char* 
Procotol::MakeSerGetCBCInfoAck()
{
	IbeaconConfig *conf = GetConfigPosition();
	cJSON *root,*info;
	root = cJSON_CreateObject();	
	cJSON_AddItemToObject(root, "name", cJSON_CreateString("Return CloudBeacon Basic Param"));
	cJSON_AddItemToObject(root, "values", info = cJSON_CreateObject());
	cJSON_AddStringToObject(info, "wan_mac_adr", conf->getWanMAC());
	cJSON_AddNumberToObject(info, "version", conf->getVersion());

	char* out = cJSON_Print(root);	
	cJSON_Delete(root);	
	return out;
}

int
Procotol::RecvOnePkg(int sockFD, char *recvBuff, int buffLen, int timeOut)
{
	u_int16 length;
	int nRet;
	// 接收包头
	if ( 0 >= Read(sockFD, recvBuff, 4+HEAD_LEN, timeOut) )
	{
		write_error_log("Read error.");
		return -1;
	}
	
	// 协议判断
	if ( -1 == CheckPkgHead(recvBuff, &length) )
	{
		write_error_log("Package error.");
		return -2;
	}
	
	// 读取数据
	if ( 0 >= (nRet = Read(sockFD, recvBuff, length, timeOut)) ) 
	{
		write_error_log("Read error.");
		return -1;
	}
	return nRet;
}


#if 0
char*
Procotol::MakeUploadMacInfo(char* macInfo, int& outLen)
{
#define DEF_MAC_LEN   (sizeof("00:00:00:00:00:00\n")-1)
	// macInfo 从proc文件系统读取到的信息
	int i = 0, j = 0, k = 0, l=0;
	char* pMac = macInfo;

	// 粗略估计大概有多少个Mac地址，多估计
	j = strlen(macInfo);
	i =  j / DEF_MAC_LEN;
	(j % DEF_MAC_LEN) ? ++i : 0;

	char* mac[i];
	char* temp[i];
	j = 0;
	mac[j++] = pMac;
	++k;
	while(*pMac)
	{
		if (*pMac == '\n' || *pMac == '\r')
		{
			*pMac++ = 0;
			while((! isxdigit(*pMac)) && *pMac)++pMac;
			if (!*pMac) break;
			mac[j++] = pMac;
			++k;
		}
		else 
			++pMac;
	}

	// 数据是否有效
	int loop;
	for (j=0; j<k; ++j)
	{
		pMac = mac[j];
		for (loop=0; loop<DEF_MAC_LEN-1; ++loop)
		{
			if (((!isxdigit(pMac[loop])) && pMac[loop] != ':') || 0 == pMac[loop])
			{
				break;
			}
		}
		if (loop == DEF_MAC_LEN-1)
		{
			temp[l++] = mac[j];
		}
	}
	cJSON *root;
	char* out;

	root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "mac_info", cJSON_CreateStringArray((const char**)temp, l));
	out = cJSON_Print(root);	
	cJSON_Delete(root);
	printf("%s\n", out);
	
	int jsonLen = strlen(out);
	char* sendBuff = new char[jsonLen+DEF_PRO_HEAD_LEN];
	memcpy(sendBuff, out, jsonLen);
	free(out);
	
	outLen = AddPkgHeadAndTail(sendBuff, jsonLen, MES_UPL_PHO_MAC_INFO, 1, 1);
	if (-1 == outLen)
	{
		write_error_log("Add Package Head And Tail Failed.");
		delete [] sendBuff;
		return NULL;
	}
	return sendBuff;
}
#endif 







#ifdef TestProcotol
int main()
{
	u_int16 temp = 0x0001;
	int nRet;
	char str[4] = {0};

	Procotol::Uint16HexToStr(temp, str);
	Procotol::printHex(str, 4);
	memcpy(str, "0f0102a2", 4);
	Procotol::StrToUint16Hex(&temp, str);
	printf("0f01=%x\n", temp);
	
	temp = 0;
	//Procotol::StrToUint16Hex(&temp, str);
	printf("%0x\n-----------------------------\n\n", temp);

	char buff[1024] = {0};
	char buff1[1024] = {0};
	memset(buff, 'a', 4);
	nRet = Procotol::AddPkgHeadAndTail(buff, 4, 1, 1, 1);
	Procotol::printHex(buff, nRet);
	printf("-----------------------------------------\n");

	
	u_int16 typeID;
	u_int16 leng;
	u_int8 PkgTotal, PkgSeq;
	int i;
	Procotol::StrToUint16Hex(&leng, buff+2);
	printf("leng=%d\n", leng);
	memcpy(buff1, buff+6, leng);
	i = Procotol::DelPkgHeadAndTail(buff1, leng, &typeID, &PkgTotal, &PkgSeq);
	if ( i > 0)
		Procotol::printHex(buff1, i);
	return 0;
}
#endif

