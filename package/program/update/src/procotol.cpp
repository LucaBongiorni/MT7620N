#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>



#include "procotol.h"
#include "defCom.h"

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
Procotol::StrToUint32Hex(u_int32* hex, char* str)
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
Procotol::StrToUint16Hex(u_int16* hex, char* str)
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
Procotol::StrToUint8Hex(u_int8* hex, char* str)
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
	char* pCheck = temp + sizeof(int) + HEAD_LEN;  			// 计算校验和的起始位
	u_int16 length;

	if (NULL == content || contentLen > 1400 || content < 0)
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
	memcpy(ptemp, content, contentLen); // 填充数据内容
	ptemp += contentLen;

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
		write_error_log("checksum error. checksum=%d, 0xAA=%d, nRet=%d", checkNum, 0xaa, nRet);
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
	return length;
}








#if 0
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

