#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <shadow.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdarg.h>


#if 0
#include "base64RSA.h"
#endif

#include "cJSON.h"
#include "defCom.h"
#include "md5.h"
#include "UCI_File.h"


#ifndef u_int8
#define u_int8 unsigned char
#endif
#ifndef u_int16
#define u_int16 unsigned short
#endif
#ifndef u_int32
#define u_int32 unsigned int
#endif


// 包头和包尾标志
const int AddLength = 4;
static const char HEAD[2] = {0x02, 0x00};
#define HEAD_LEN  (sizeof(HEAD))
static const char TAIL = 0x03;
#define TAIL_LEN  (sizeof(TAIL))


// 除去消息体，默认的协议头长度，可长不可短
#define DEF_PRO_HEAD_LEN   32
#define MAC_SERVER         1
#define BEACON_SERVER      2


#define SERIALS_PKG_LEN    1024
static const char SERIALS_HEAD[1] = {0x02};
#define SERIALS_HEAD_LEN   (sizeof(SERIALS_HEAD))
const int SERIALS_NULLPKG_LEN  = 7;




void
pCheckPSInfo(int sockFD, u_int16 typeID, char* RecvBuff, int buffLen)
{
	char *Output = NULL, *out = NULL;
	const int OutputLen = 4096;
	int nRet;
	char* ptemp = NULL;
	char* pVal = NULL;
	char* pLine = NULL;
	cJSON* root = NULL, *info = NULL;
	char USER[32] = {0};
	char STAT[16] = {0};
	char COMMAND[256] = {0};
	int PID, VSZ;
	
	root = cJSON_CreateArray();
	Output = (char*)malloc(OutputLen);
	if (!Output)
	{
		goto sendPkg;
	}
	nRet = GetShellCmdOutput("/bin/ps", Output, OutputLen);
	if (nRet == -1)
	{
		goto sendPkg;
	}
	
	pLine = strstr(Output, "PID");
	if (pLine)
		while(*pLine++ != '\n');     // 跑到下一行
	else
		goto sendPkg;

	for (;;)
	{
		pVal = pLine;
		while(*pLine != '\n' && *pLine)++pLine; *pLine++ = 0;
		if (pLine - Output >= nRet) break;

		while(isspace(*pVal))++pVal; ptemp = pVal;
		while(!isspace(*ptemp))++ptemp; *ptemp++ = 0;
		PID = atoi(pVal);

		pVal = ptemp;
		while(isspace(*pVal))++pVal; ptemp = pVal;
		while(!isspace(*ptemp))++ptemp; *ptemp++ = 0;
		strncpy(USER, pVal, sizeof(USER));

		pVal = ptemp;
		while(isspace(*pVal))++pVal; ptemp = pVal;
		while(!isspace(*ptemp))++ptemp; *ptemp++ = 0;
		VSZ = atoi(pVal);

		pVal = ptemp;
		while(isspace(*pVal))++pVal; ptemp = pVal;
		while(!isspace(*ptemp))++ptemp; *ptemp++ = 0;
		strncpy(STAT, pVal, sizeof(STAT));

		pVal = ptemp;
		while(isspace(*pVal))++pVal; ptemp = pVal; 
		strncpy(COMMAND, pVal, sizeof(COMMAND));

		//printf("PID=%d, USER=%s, VSZ=%d, STAT=%s, COMMAND=%s\n", PID, USER, VSZ, STAT, COMMAND);
		
		//nRet = sscanf(pVal, " %d %s %d %s %s", &PID, USER, &VSZ, STAT, COMMAND);
		//if (5 != nRet) break;

		info = cJSON_CreateObject();
		cJSON_AddNumberToObject(info, "PID", PID);
		cJSON_AddStringToObject(info, "USER", USER);
		cJSON_AddNumberToObject(info, "VSZ", VSZ);
		cJSON_AddStringToObject(info, "STAT", STAT);
		cJSON_AddStringToObject(info, "COMMAND", COMMAND);
		cJSON_AddItemToArray(root, info);
	}
	
sendPkg:
	if (Output) free(Output), Output = NULL;
	out = cJSON_Print(root);
	printf("%s\n", out);
	cJSON_Delete(root), root = NULL;
	if (out) free(out), out = NULL;
}



int main()
{
	pCheckPSInfo(0, 0, NULL, 0);
	return 0;
}






#if 0
//
int main()
{
    cJSON *root,*fmt, *human, *vroot;
    char *out;  /* declare a few. */
    root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "name", cJSON_CreateString("Jack (\"Bee\") Nimble"));
    cJSON_AddItemToObject(root, "format", fmt = cJSON_CreateObject());
    cJSON_AddFalseToObject (fmt, "interlace");
    cJSON_AddNumberToObject(fmt, "frame rate", 24);
    //cJSON_AddItemReferenceToObject
    cJSON_AddItemToObject(root, "human", human = cJSON_CreateObject());
    cJSON_AddNullToObject(human, "interlace");
    cJSON_AddNumberToObject(human, "age", 24);
    /* Print to text, Delete the cJSON, print it, release the string. */
    out = cJSON_Print(root);
    printf("%s\n",out);
    CJSONFree(out);
    vroot = cJSON_CreateObject();
    cJSON_AddItemReferenceToObject(vroot, "name", human);
    out = cJSON_Print(vroot);
    printf("%s\n", out);
    cJSON_Delete(vroot);
    cJSON_Delete(root);
    CJSONFree(out);
    return 0;
}

#if 0
// 测试添加json
int main()
{
    char text0[] = "{\n\t\"type\":       \"rect\", \n\t\"width\":      1920, \n\t\"height\":     1080\n}";
    cJSON *root = cJSON_Parse(text0);
    cJSON_AddStringToObject(root, "mac", "112233445566");
    char* out = cJSON_Print(root);
    cJSON_Delete(root);
    printf("out=%s\n", out);
    CJSONFree(out);
    return 0;
}
#endif
#if 0
CBase64Operate base64;
// mac 地址和key规则匹配
int main()
{
    char *mac = "112233445566";
    int i = 0, j;
    unsigned char key[17] = "1234567890abcdef";
    for (j=0, i=0; i<16; ++i, ++j)
    {
        key[i] = key[i] + mac[j];
        j = j % 12;
    }
    for (i=0; i<16; ++i)
    {
        printf("%0x", key[i]);
    }
    printf("\n");
    char output[1024] = {0};
    unsigned long len;
    // base64
    base64.Encrypt((char*)key, 16, output, len);
    output[len] = 0;
    printf("output=%s\n", output);
    std::string data;
    base64.Decrypt(output, len, data);
    memcpy(key, data.c_str(), 16);
    for (j=0, i=0; i<16; ++i, ++j)
    {
        printf("%0x", key[i]);
        key[i] = key[i] - mac[j];
        j = j % 12;
    }
    printf("\n");
    printf("%s\n", key);
    return 0;
}
#endif

char*
LoadFileToMem(const char* FilePath, int* Size)
{
    struct stat sb;
    if ( -1 == stat(FilePath, &sb) )
    {
        printf("stat file failed.\n");
        return NULL;
    }
	char* buff = NULL;
    int fileSize = sb.st_size+1;
    buff = (char*)malloc(fileSize);
    if (NULL == buff)
    {
        printf("Malloc memory failed.\n");
        return NULL;
    }
    memset(buff, 0, fileSize);
    FILE *fd = fopen(FilePath, "r");
    if (NULL == fd)
    {
        printf("Open %s Failed.\n", FilePath);
        if (buff) free(buff), buff = NULL;
        return NULL;
    }
    int nRead = 0;
    int j = fileSize / 512;
    nRead = fread(buff, 512, j+1, fd);
    if (nRead != j)
    {
        printf("[%s:%d]Error: fread to file failed. nRead=%d, j=%d\n", __FILE__, __LINE__, nRead, j);
        if (buff) free(buff), buff = NULL;
        return NULL;
    }
    fclose(fd);
    *(buff + fileSize - 1) = 0;
	*Size = fileSize;
    return buff;
}

#if 0
int main()
{
    int nRet;
    char* buff = NULL;
    buff = LoadFileToMem("./abc.dat", &nRet);
    if (buff)
    {
        printf("buff:%s\n", buff);
		free(buff);
    }
    return 0;
}
#endif

#if 0
const char*
inetNtoA(unsigned int ip)
{
    static char buf[sizeof "123.456.789.123"] = {0};
    unsigned char *tmp = (unsigned char *)&ip;
    sprintf(buf, "%d.%d.%d.%d", tmp[0] & 0xff, tmp[1] & 0xff,
            tmp[2] & 0xff, tmp[3] & 0xff);
    return buf;
}
int main()
{
    printf("%s\n", inetNtoA(0));
    return 0;
}
#endif


#if 0
void alarmfn(int sig)
{
    printf("-------------------------------%d\n", sig);
    return;
}
int main()
{
	signal(SIGALRM, alarmfn);
    alarm(2);//

	sleep(3);
	printf("sleep 1s.\n");
	alarm(1);
	while(1)sleep(10);
	return 0;
}
#endif

typedef unsigned char u8;
typedef unsigned short u16;
#define RST_CLOUD_BCN_CMD    0x0102

u8 com_Rx_chk_xor(u8*p_msg, int msg_len)
{
    int i;
    unsigned char val = 0;
    for (i=0; i<msg_len; i++)
    {
        val ^= p_msg[i];
    }
    return val;
}
void HexToASCII(u8*phex, u8*pascii, int len)
{
    unsigned char tmp;
    int i=0;
    for(i=0; i<len; i++)
    {
        tmp=((*phex)&0xF0)>>4;
        if(tmp<10)
        {
            tmp+=0x30;
        }
        else
        {
            tmp+=0x37;
        }
        *(pascii++)=tmp;
        tmp=(*(phex++))&0x0F;
        if(tmp<10)
        {
            tmp+=0x30;
        }
        else
        {
            tmp+=0x37;
        }
        *(pascii++)=tmp;
    }
}
int rstcloudbeaconCMD(void)
{
    unsigned int  len;
    u8 tmp_u8=0;
    u8 send_buf[120];
    u8 send_data[60];
    //01 02  00 00 00 01
    unsigned char* pSendData = send_data;
	unsigned char* pCheckNum = pSendData + 2;

	// 填充长度
	unsigned short leng = htons(14);
	memcpy(pSendData, &leng, 2);
	pSendData += 2;

	// 填充id
	leng = htons(RST_CLOUD_BCN_CMD);
	memcpy(pSendData, &leng, 2);
	pSendData += 2;

	// 填充流水号
	len = htonl(1);
	memcpy(pSendData, &len, 4);
	pSendData += 4;

	// 填充消息体

	// 填充校验和
	*pSendData = 0;
	for (int i=0; i<6; ++i)
	{
		*pSendData ^= *(pCheckNum + i);
	}
	++pSendData;

	// 转换成字符
    HexToASCII(send_data, &send_buf[1], pSendData-send_data);

	for (int i=0; i<9; ++i)
	{
		printf("%x ", send_data[i]);
	}
	printf("\n");
	for (int i=1; i<19; ++i)
	{
		printf("%c ", send_buf[i]);
	}
	printf("\n");


	
    send_buf[0]=0x02;
    //send_buf[1]=0x00;
    send_buf[19]=0x03;//send_buf[0]的0+长度*2+1
    len=20;
    //memcpy(send_buf_dn, send_buf,len);

    return 0;
}

void 
Uint16HexToStr(u_int16 hex, char *str)
{
	u_int8 temp;
	int i;

	for (i=0; i<4; ++i)
	{
		temp = (hex >> (3-i)*4) & 0x000f;
		if (temp < 10)
			str[i] = temp + '0';
		else 
			str[i] = temp + 'A' - 10;
	}
	return ;
}

void 
Uint8HexToStr(u_int8 hex, char *str)
{
	u_int8 temp;
	int i;

	for (i=0; i<2; ++i)
	{
		temp = (hex >> (1-i)*4) & 0x0f;
		if (temp < 10)
			str[i] = temp + '0';
		else 
			str[i] = temp + 'A' - 10;
	}
	return ;
}

void 
Uint32HexTostr(u_int32 hex, char* str)
{
	u_int8 temp = 0;
	int i;

	for (i=0; i<8; ++i)
	{
		temp = (hex >> (7-i)*4) & 0x0000000f;
		if (temp < 10)
			str[i] = temp + '0';
		else 
			str[i] = temp + 'A' - 10;
	}
	return ;
}


int
SerialsAddPkgHeadAndTail(char* buff, int buffLen, u_int16 typeID, int switfNum, u_int8* outBuff)
{
	unsigned int temp, i;
	unsigned short leng;
	u_int8 sendData[32] = {0};
	unsigned char* pSendData = sendData;
	unsigned char* pCheckNum = pSendData + 2;
	u_int8* pOutBuff = outBuff;
	u_int8 checkNum = 0;

	if (buffLen > SERIALS_PKG_LEN || NULL == outBuff) return -1;

	// 计算校验和，typeid和流水号
	leng = htons(typeID);
	memcpy(pSendData, &leng, 2);
	pSendData += 2;
	// 流水号
	temp = htonl(switfNum);
	memcpy(pSendData, &temp, 4);
	pSendData += 4;
	temp = pSendData - sendData;
	for (i=0; i<temp; ++i)
	{
		checkNum ^= sendData[i];
	}
	if (buff)
	{
		for (i=0; i<buffLen; ++i)
		{
			checkNum ^= buff[i];
		}	
	}
	else
	{
		buffLen = 0;
	}

	// 加包头
	memcpy(pOutBuff, SERIALS_HEAD, SERIALS_HEAD_LEN);
	pOutBuff += SERIALS_HEAD_LEN;
	// 长度
	Uint16HexToStr(SERIALS_NULLPKG_LEN + buffLen, (char*)pOutBuff);
	pOutBuff += 4;
	// 消息头 6 个字节
	HexToASCII(sendData, pOutBuff, 6);
	pOutBuff += 12;
	// 消息体
	if (buff)
	{
		memcpy(pOutBuff, buff, buffLen);
		pOutBuff += buffLen;
	}
	// 校验和 1 个字节
	Uint8HexToStr(checkNum, (char*)pOutBuff);
	pOutBuff += 2;
	// 包尾
	memcpy(pOutBuff, &TAIL, TAIL_LEN);
	pOutBuff += TAIL_LEN;
	return (pOutBuff - outBuff);
}

#if 0
void ASCIIToHex(u8*ascii, u8*hex, int len)
{
    u8 tmp,tmp1;
    int i=0;
    for (i=0; i<len; i++)
    {
        tmp = *(ascii++);
        if (tmp <= '9')
        {
            tmp -= '0';
        }
        else
        {
            tmp = tmp - 'A' + 10;
        }
		
        tmp1=*(ascii++);
        if (tmp1 <= '9')
        {
            tmp1 -= '0';
        }
        else
        {
            tmp1 = tmp - 'A' + 10;
        }
        tmp = (tmp<<4) & 0xF0;
        tmp1 &= 0x0F;
        *(hex++) = tmp|tmp1;
    }
}
#endif

int 
StrToUint8Hex(u_int8* hex, const char* str)
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



int
SerialsDelPkgHeadAndTail(char* buff, int buffLen, 
	u_int16 &typeID, int &switfNum, u_int8* outBuff)
{
	if (NULL == buff || NULL == outBuff )
		return -1;

	char* pBuff = buff;
	u_int8 temp[32] = {0};
	u_int16 leng = 0;
	u_int8 checkNum = 0;
	int nRet = 0, i = 0;
	
	// 检查包头、包尾
	if ( 0 != memcmp(pBuff, SERIALS_HEAD, SERIALS_HEAD_LEN) )
	{
		return -1;
	}
	if ( 0 != memcmp(pBuff+buffLen-TAIL_LEN, &TAIL, TAIL_LEN) )
	{
		return -1;
	}
	pBuff += SERIALS_HEAD_LEN;

	// 长度、typeID、流水号
	//ASCIIToHex((u_int8*)pBuff, temp, 8);
	leng = ntohs(*(u_int16*)temp);
	typeID = ntohs(*(u_int16*)(temp+2));
	switfNum = ntohl(*(u_int32*)(temp+4));
	pBuff += 16;

	// 消息头的检验和
	for (i=2; i<8; ++i)
	{
		checkNum ^= temp[i];
	}

	// 消息体
	if (leng > SERIALS_NULLPKG_LEN)
	{
		nRet = leng-SERIALS_NULLPKG_LEN;
		memcpy(outBuff, pBuff, nRet);
		// 消息体的校验和
		for (i=0; i<nRet; ++i)
		{
			checkNum ^= outBuff[i];
		}
		pBuff += nRet;
	}

	memset(temp, 0, 32);
	//printf("pBuff[0]=%c, pBuff[1]=%c\n", pBuff[0], pBuff[1]);
	StrToUint8Hex(&temp[0], pBuff);
	//ASCIIToHex((u_int8*)pBuff, temp, 1);
	if (checkNum != temp[0])
	{
		printf("checkNum is error. checkNum=%x, temp=%x\n", checkNum, temp[0]);
		return -1;
	}

	return nRet;
}



#if 0
int main()
{
	rstcloudbeaconCMD();

	u_int8 outBuff[1024] = {0};
	int temp = SerialsAddPkgHeadAndTail(NULL, 0, RST_CLOUD_BCN_CMD, 1, outBuff);

	printf("--------------------------------------temp=%d\n", temp);
	for (int i=0; i<temp; ++i)
	{
		printf("%c ", outBuff[i]);
	}
	printf("\n");
	printf("-------------------------------------\n");


	u_int16 typeID;
	int switfNum;
	u_int8 str[1024] = {0};
	temp = SerialsDelPkgHeadAndTail((char*)outBuff, temp, typeID, switfNum, str);
	printf("typeID=%x\n", typeID);
	printf("switfNum=%x\n", switfNum);

	printf("--------------------------------------temp=%d\n", temp);
	for (int i=0; i<temp; ++i)
	{
		printf("%c ", str[i]);
	}
	printf("\n");
	printf("-------------------------------------\n");
	
	return 0;
}
#endif

int 
HexStringtoInt(char *str, int *datalen, char* md5)
{
	int i=0, j=0;
	unsigned char tmpstr[128] = {0};

	if (NULL == str || NULL == datalen)
	{
		return -1;
	}
	if (0 != *datalen % 2)
	{
		return -1;
	}

	for(i = 0; i < *datalen; i += 2)
	{
		if (str[i] >= '0' && str[i] <= '9')
		{
			str[i] = (str[i] - '0' ) * 16;
		}
		else if (str[i] <= 'f' && str[i] >= 'a')
		{
			str[i] = (str[i] - 'a' + 10) * 16;
		}
		else if (str[i] <= 'F' && str[i] >= 'A')
		{
			str[i]  = (str[i] - 'A' + 10) * 16;
		}

		if (str[i + 1] >= '0' && str[i + 1] <= '9')
		{
			str[i] += str[i + 1] - '0' ;
		}
		else if (str[i + 1] <= 'f' && str[i + 1] >= 'a')
		{
			str[i] += str[i + 1] - 'a' + 10;
		}
		else if (str[i + 1] <= 'F' && str[i + 1] >= 'A')
		{
			str[i]  += str[i + 1] - 'A' + 10;
		}
		tmpstr[j++] = str[i];
	}
	
	tmpstr[j] = '\0';
	*datalen =j;
	memcpy(md5, tmpstr, *datalen);
	md5[*datalen] = '\0';
	return 0;
}


int ASCIIToHex(char* ascii, char* hex, int len)
{
    u_int8 tmp, tmp1;
    int i = 0;
    for(i=0; i<len; i++)
    {
        tmp = (unsigned char)*(ascii++);
        if (tmp >= '0' && tmp <= '9')
        {
            tmp = tmp - '0';
        }
		else if (tmp >= 'a' && tmp <= 'f')
		{
			tmp = tmp - 'a' + 10;
		}
        else if (tmp >= 'A' && tmp <= 'F')
        {
            tmp = tmp - 'A' + 10;
        }
		else
		{
			return -1;
		}
		
        tmp1 = (unsigned char)*(ascii++);
        if (tmp1 >= '0' && tmp1 <= '9')
        {
            tmp1 = tmp1 - '0';
        }
		else if (tmp1 >= 'a' && tmp1 <= 'f')
		{
			tmp1 = tmp1 - 'a' + 10;
		}
        else if (tmp1 >= 'A' && tmp1 <= 'F')
        {
            tmp1 = tmp1 - 'A' + 10;
        }
		else
		{
			return -1;
		}

		
        tmp = (tmp<<4) & 0xF0;
        tmp1 &= 0x0F;
		printf("tmp=%02x, tmp1=%02x\n", (unsigned char)tmp, (unsigned char)tmp1);
        *(hex++) = tmp | tmp1;
    }
	return 0;
}

int main()
{
	char str[] = "1234567890abcdef";
	int nRet = sizeof(str)/sizeof(str[0]) - 1;
	char tmp[32] = {0};

	printf("nRet = %d\n", nRet);
	HexStringtoInt(str, &nRet, tmp);
	int i;
	for (i=0; i<8; ++i)
	{
		printf("%02x ", (unsigned char)tmp[i]);
	}
	printf("\n");

	memset(tmp, 0, sizeof(tmp));
	memcpy(str, "1234567890abcdef", 16);
	ASCIIToHex(str, tmp, 8);
	for (i=0; i<8; ++i)
	{
		printf("%02x ", (unsigned char)tmp[i]);
	}
	printf("\n");
	return 0;
}
#endif


