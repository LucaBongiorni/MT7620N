#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>



#include "procotol.h"
//#include "defCom.h"
//#include "cJSON.h"
//#include "cbProjMain.h"
//#include "netSockTcp.h"
//#include "base64RSA.h"
//#include "broadcast.h"


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


// ��������: unsigned intʮ������ת�ַ�
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
			str[i] = temp + 'A' - 10;
	}
	return ;
}

// ��������: �ַ�תunsigned intʮ�����ƣ�ת8���ֽ�
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

// ʮ������ת�ַ���ת�����ֽ�
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
			str[i] = temp + 'A' - 10;
	}
	return ;
}

// �ַ�תʮ�����ƣ�ת�ĸ��ֽ�
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


// ʮ������ת�ַ���ת�����ֽ�
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
			str[i] = temp + 'A' - 10;
	}
	return ;
}

// �ַ�תʮ�����ƣ�ת�ĸ��ֽ�
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


// �Ӱ�ͷ�Ͱ�β
int 
Procotol::AddPkgHeadAndTail(char* content, u_int16 contentLen, u_int16 typeID, 
	char PkgTotal, char PkgSeq)
{
	char temp[1428] = {0};
	//char checkSum[1428] = {0};
	char* ptemp  = NULL;
	char* pCheck = temp + sizeof(int) + HEAD_LEN;  	// ����У��͵���ʼλ
	u_int16 length;

	if (NULL == content || contentLen > EVERY_PKG_LEN)
	{
		printf("[%s:%d]Error: Parameter is Error.\n", __FILE__,__LINE__);
		return -1;
	}

	ptemp = temp;
	memcpy(ptemp, &HEAD, HEAD_LEN);		// ����ͷ
	ptemp += HEAD_LEN;

	length = contentLen + 14;			// ����һ��Э��ĳ���
	Uint16HexToStr(length, ptemp);
	ptemp += 4;

	Uint16HexToStr(typeID, ptemp);   	// ��������
	ptemp += 4; 
	
	length = contentLen;				// ���ڶ���Э��ĳ���
	Uint16HexToStr(length, ptemp);
	ptemp += 4;

	Uint8HexToStr(PkgTotal, ptemp);	  	// �ܹ����ٸ���
	ptemp += 2;
	Uint8HexToStr(PkgSeq, ptemp);	  	// �ڼ�����
	ptemp += 2;

	if (contentLen > 0)                // �����������
	{
		memcpy(ptemp, content, contentLen); 
		ptemp += contentLen;
	}

	// ����У���
	u_int8 i = 0;
	for (; pCheck<ptemp; ++pCheck)
	{
		i ^= *pCheck;
	}
	//printf("checkNum=%d, 0xaa=%d\n", CHECK-i, 0xaa);
	Uint8HexToStr(i, ptemp);			// ���У���
	ptemp += 2;

	// ����β
	memcpy(ptemp, &TAIL, TAIL_LEN);
	ptemp += TAIL_LEN;
	
	// ͨ����������
	memcpy(content, temp, ptemp-temp);
	return ptemp-temp;
}

int 
Procotol::NetAddPkgHeadAndTail(char* content, u_int16 contentLen, u_int16 typeID, 
	char PkgTotal, char PkgSeq)
{
	char temp[1500] = {0};
	//char checkSum[1428] = {0};
	char* ptemp  = NULL;
	char* pCheck = temp + sizeof(int) + HEAD_LEN;  	// ����У��͵���ʼλ
	u_int16 length;

	if (NULL == content || contentLen > EVERY_PKG_LEN)
	{
		printf("[%s:%d]Error: Parameter is Error.\n", __FILE__,__LINE__);
		return -1;
	}

	ptemp = temp + AddLength;
	memcpy(ptemp, &HEAD, HEAD_LEN);		// ����ͷ
	ptemp += HEAD_LEN;

	length = contentLen + 14;			// ����һ��Э��ĳ���
	Uint16HexToStr(length, ptemp);
	ptemp += 4;

	Uint16HexToStr(typeID, ptemp);   	// ��������
	ptemp += 4; 
	
	length = contentLen;				// ���ڶ���Э��ĳ���
	Uint16HexToStr(length, ptemp);
	ptemp += 4;

	Uint8HexToStr(PkgTotal, ptemp);	  	// �ܹ����ٸ���
	ptemp += 2;
	Uint8HexToStr(PkgSeq, ptemp);	  	// �ڼ�����
	ptemp += 2;

	if (contentLen > 0)                // �����������
	{
		memcpy(ptemp, content, contentLen); 
		ptemp += contentLen;
	}

	// ����У���
	u_int8 i = 0;
	for (; pCheck<ptemp; ++pCheck)
	{
		i ^= *pCheck;
	}
	//printf("checkNum=%d, 0xaa=%d\n", CHECK-i, 0xaa);
	Uint8HexToStr(i, ptemp);			// ���У���
	ptemp += 2;

	// ����β
	memcpy(ptemp, &TAIL, TAIL_LEN);
	ptemp += TAIL_LEN;
	
	// ͨ����������
	*(int*)temp  = htonl(ptemp-temp-AddLength);
	memcpy(content, temp, ptemp-temp);
	//printf("%d, [%d,%d,%d,%d]\n", ptemp-temp-AddLength, *temp, *(temp+1), *(temp+2), *(temp+3));
	return ptemp-temp;
}


int 
Procotol::NetCheckPkgHead(const char* content, u_int16* length)
{
	if (NULL == content) return -1;
	const char* pcontent = content + AddLength;

	if ( 0 != memcmp(pcontent, HEAD, HEAD_LEN-AddLength) )
	{
		printf("[%s:%d]Error package.%x %x\n", __FILE__, __LINE__, pcontent[0], pcontent[1]);
		return -1;
	}
	u_int16 temp = 0;
	if ( -1 == StrToUint16Hex(&temp, pcontent+HEAD_LEN-AddLength) )
	{
		// ������
		printf("[%s:%d]Error package.\n", __FILE__, __LINE__);
		return -1;
	}
	*length = temp + TAIL_LEN;
	return 0;
}





// ȥ��ͷ�Ͱ�β�������content�Ѿ������˵�һ��
int 
Procotol::DelPkgHeadAndTail(char* content, int contentLen, 
			u_int16 *typeID, u_int8* PkgTotal, u_int8* PkgSeq)
{	
	char* pContent = content;
	u_int16 length = 0;
	u_int8 checkNum = 0;
	u_int8 nRet = 0;
	
	// ���У���
	for (int i=0; i<contentLen-2; ++i)
	{
		checkNum ^= content[i];
	}
	pContent = content + contentLen - 2;
	StrToUint8Hex(&nRet, pContent);
	if (nRet != checkNum)
	{
		printf("[%s:%d]checksum error. checksum=%d, 0xAA=%d, nRet=%d[%x][%x]\n", 
			__FILE__, __LINE__, checkNum, 0xaa, nRet, pContent[0], pContent[1]);

		// typeID
		pContent = content;
		StrToUint16Hex(typeID, pContent);
		pContent += 4;

		// ����
		StrToUint16Hex(&length, pContent);
		pContent += 4;
		printf("length=%d\n", length);

		// �ܰ���
		StrToUint8Hex(PkgTotal, pContent);
		pContent += 2;

		// �ְ���
		StrToUint8Hex(PkgSeq, pContent);
		pContent += 2;

		for (int i=0; i<length; ++i)
		{
			content[i] = pContent[i];
		}
		content[length] = 0;
		
		return -1;
	}

	// typeID
	pContent = content;
	StrToUint16Hex(typeID, pContent);
	pContent += 4;

	// ����
	StrToUint16Hex(&length, pContent);
	pContent += 4;
	//printf("length=%d\n", length);

	// �ܰ���
	StrToUint8Hex(PkgTotal, pContent);
	pContent += 2;

	// �ְ���
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
	int returnVal = 0;
	if (NULL == content) return -1;

	if ( 0 != memcmp(content, HEAD, HEAD_LEN) )
	{
		printf("[%s:%d]Error package.%x %x\n", __FUNCTION__, __LINE__, content[0], content[1]);
		returnVal = -1;
	}
	u_int16 temp = 0;
	if ( -1 == StrToUint16Hex(&temp, content+HEAD_LEN) )
	{
		// ������
		printf("[%s:%d]Error package.\n", __FUNCTION__, __LINE__);
		returnVal = -1;
	}
	*length = temp + TAIL_LEN;
	return returnVal;
}



void 
Procotol::HexToASCII(u_int8* phex, u_int8* pascii, int len)
{
    u_int8 tmp;
    int i=0;
    for(i=0; i<len; i++)
    {
        tmp = ((*phex) & 0xF0) >> 4;
        if (tmp < 10)
        {
            tmp += '0';
        }
        else
        {
            tmp += ('A' - 10);
        }
        *(pascii++) = tmp;
        tmp= (*(phex++))&0x0F;
        if (tmp < 10)
        {
            tmp += '0';
        }
        else
        {
            tmp += ('A' - 10);
        }
        *(pascii++) = tmp;
    }
}

int 
Procotol::ASCIIToHex(u_int8* ascii, u_int8* hex, int len)
{
    u_int8 tmp, tmp1;
    int i = 0;
    for(i=0; i<len; i++)
    {
        tmp = *(ascii++);
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
        tmp1 = *(ascii++);
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
        *(hex++) = tmp | tmp1;
    }
	return 0;
}






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

