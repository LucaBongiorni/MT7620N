#include <assert.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <arpa/inet.h>


#include "SerialCom.h"
#include "serialPct.h"
#include "defCom.h"
#include "cbProjMain.h"

#if 0
const int DEF_SERBUFF_LEN = 1024;
static const char HEAD_FLAG = 0x02;
const int HEAD_FLAG_LEN = sizeof(HEAD_FLAG);
static const char TAIL_FLAG = 0x03;
const int TAIL_FLAG_LEN = sizeof(TAIL_FLAG);
#endif



void
printHex(const unsigned char* buf, int len);


typedef struct
{	
	int  sysSpeed;
	unsigned long  userSpeed;
}speedPara;

speedPara speed_atr[] = 
{
	{B300,  300},
	{B600,  600},
	{B1200, 1200},
	{B2400, 2400},
	{B4800, 4800},
	{B9600, 9600},
	{B19200,  19200},
	{B38400,  38400},
	{B57600,  57600},
	{B115200, 115200},
	{B230400, 230400}	
};


CComTransmit::CComTransmit()
{
	m_fd = -1;
	m_fp = NULL;
	m_bOpenFlag = false;
}


/*****************************************************************************
串口类析构函数
******************************************************************************/
CComTransmit::~CComTransmit()
{
	if (m_bOpenFlag)
	{
		CloseCom();
	}
}


/*****************************************************************************
打印要操作的串口的配置参数
******************************************************************************/
void CComTransmit::TraceComPar(TerminalInfo *com_dev)
{
#if 1
	printf("Open Com\n");
	printf("dev_path:%s\n", com_dev->dev_path);
	printf("baudrate:%ld\n", com_dev->baudrate);
	printf("databit:%d\n", com_dev->databit);
	printf("parity:%d\n", com_dev->parity);
	printf("flowctrl:%d\n", com_dev->flowctrl);
	printf("stopbit:%d\n", com_dev->stopbit);
#endif
}


int 
CComTransmit::OpenCom(TerminalInfo *com_dev)
{
	//TraceComPar(com_dev);
	if (m_bOpenFlag)
	{
		return -1;
	}
	
	//m_fd = open(com_dev->dev_path, O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY);
	m_fd = open(com_dev->dev_path, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (m_fd < 0)
	{
		Debug(I_ERROR, "Open Serials Com failed. devPath=%s", com_dev->dev_path);
		return -1;
	}

#if 0
	m_fp = fdopen(m_fd, "r+b");
	if (m_fp == NULL)
	{
		close(m_fd);
		m_fd = -1;
		return -1;
	}
#endif
	
	if (SetComSpeed(m_fd, com_dev->baudrate)  < 0)
	{
		goto failed;
	}

	if (SetComParity(m_fd, com_dev->databit, com_dev->stopbit, com_dev->parity, com_dev->flowctrl) < 0 )
	{
		goto failed;
	}
	
	memcpy(&m_ComDev, com_dev, sizeof(TerminalInfo));
	SetComRawMode(m_fd);
	m_bOpenFlag = true;

	return 0;
	
failed:
	close(m_fd);
	m_fd = -1;
	//m_fp = NULL;
	return -1;
}


int 
CComTransmit::CloseCom()
{
#if 0
	if(m_bOpenFlag)
		CloseCom( );
	return 0;
#else
	if (!m_bOpenFlag)
		return -1;
	
	int ret = close(m_fd);
	if (ret < 0)
	{
		printf("close Com failure!\n");		
	}
	m_fd = -1;
	m_fp = NULL;
	m_bOpenFlag = false;

	return 0;
#endif
}


int  CComTransmit::SetComSpeed(int fd, unsigned int speed)
{
	int comspeed = 0;
	//int status = 0;
	struct termios Opt;

	for (unsigned int i = 0; i < sizeof(speed_atr)/sizeof(speedPara); i++)
	{
		if(speed == speed_atr[i].userSpeed)
		{
			comspeed = speed_atr[i].sysSpeed;
			break;
		}
	}

	tcgetattr(fd, &Opt);
	tcflush(fd, TCIOFLUSH);
	if (cfsetispeed(&Opt, comspeed)<0)
	{
		return -1;
	}
	if (cfsetospeed(&Opt, comspeed)<0)
	{
		return -1;
	}
	
	if (tcsetattr(fd, TCSANOW, &Opt) <0)
	{
		perror("tcsetattr fd1");
		return -1;
	}
	tcflush(fd,TCIOFLUSH);

	return 0;
 }


int 
CComTransmit::SetComParity(int fd, int databits, int stopbits, int parity, int flowctrl)
{
	struct termios options; 

	if (tcgetattr(fd, &options) != 0)
	{
		printf("[%s][%d]tcgetattr failed. %m\n", __FILE__, __LINE__);
		return -1;
	}

	/*设置数据位数*/
	options.c_cflag &= ~CSIZE;
  	switch (databits)
	{ 
  		case '5':
			options.c_cflag |= CS5;
			break;
		case '6':
			options.c_cflag |= CS6;
			break;
  		case '7':
  			options.c_cflag |= CS7;
  			break;
  		case '8':
			options.c_cflag |= CS8;
			break;
		default:
			printf("[%s][%d]-------databits------\n", __FILE__, __LINE__);
			return -1;
	}
	
	// 设置奇偶位
  	switch (parity)
	{
  		case 'n':
		case 'N':
		case 0:
			options.c_cflag &= ~PARENB;    /* Clear parity enable */
			options.c_iflag &= ~INPCK;     /* Enable parity checking */
			break;
		case 'o':
		case 'O':
		case 1:
			options.c_cflag |= (PARODD | PARENB);  /* 设置为奇效验*/ 
			options.c_iflag |= INPCK;              /* Disnable parity checking */
			break;
		case 'e':
		case 'E':
		case 2:
			options.c_cflag |= PARENB;      /* Enable parity */
			options.c_cflag &= ~PARODD;     /* 转换为偶效验*/  
			options.c_iflag |= INPCK;       /* Disnable parity checking */
			break;
		case 'S':
		case 's':  /*as no parity*/
			options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;
			break;
		default:
			printf("[%s][%d]-------parity------\n", __FILE__, __LINE__);
			return -1;
	}
	
	/* 设置停止位*/   
  	switch (stopbits)
  	{
  		case '1':
  			options.c_cflag &= ~CSTOPB;
			break;
		case '2':
			options.c_cflag |= CSTOPB;
			break;
		default:
			printf("[%s][%d]-------stopbits------\n", __FILE__, __LINE__);
			return -1;
	}
	
	/* 设置流控制*/
	switch(flowctrl)	
	{
		case 0:
			// 串口数据的传输，设置为无类型模式
			options.c_cflag &= ~CRTSCTS;   /*no flow control*/
			break;
		case 1:
			options.c_cflag |= CRTSCTS;    /*hardware flow control*/
			break;
		case 2:
			options.c_cflag |= IXON|IXOFF|IXANY; /*software flow control*/
			break;
		default:
			options.c_cflag &= ~CRTSCTS;   /*no flow control*/
			break;
	}
	
  	/* Set input parity option */
  	if (parity != 'n')
  		options.c_iflag |= INPCK;   // 允许输入奇偶校验
	
	options.c_cc[VTIME] = 10;    // 1 seconds
	options.c_cc[VMIN]  = 0;

	// 原始模式
	options.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);  /*Input*/
	options.c_oflag  &= ~OPOST;   /*Output*/

  	tcflush(fd, TCIFLUSH);        /* Update the options and do it NOW */
  	if (tcsetattr(fd, TCSANOW, &options) != 0)
	{
		printf("[%s][%d]-------set serials attr failed.------\n", __FILE__, __LINE__);
		return -1;
	}
  	return 0;
}


int CComTransmit::IOFlush(int mode)
{
	if (m_fd > 0)
	{
		switch(mode)
	    {
	    case 0:
	        return tcflush(m_fd, TCIFLUSH);
	        break;
	    case 1:
	        return tcflush(m_fd, TCOFLUSH);
	        break;
	    default:
	        return tcflush(m_fd, TCIOFLUSH);
	        break;
	    }
	}
	return 0;
}

int CComTransmit::SetComRawMode(int fd)
{
	struct termios options; 

	// 获取串口状态信息
	tcgetattr(m_fd, &options);

	cfmakeraw(&options);
	if (tcsetattr(fd, TCSANOW, &options) <0)
	{
		return -1;
	}
	tcflush(fd, TCIOFLUSH);	
	return 0;
}


int CComTransmit::FgetsCom(char *ReadBuffer, int size, int *retSize, int timeOut)
{	
	struct timeval rdtimeout;
	fd_set 	read_fds;
	//int retval;

	rdtimeout.tv_sec  = timeOut / 1000;
	rdtimeout.tv_usec = (timeOut % 1000) * 1000;
	FD_ZERO(&read_fds);
	FD_SET(m_fd, &read_fds);
	
	if (select(m_fd+1,  &read_fds, NULL, NULL, &rdtimeout) < 0)
	{
		*retSize = 0;
		return -1;
	}
	
	if (!FD_ISSET(m_fd, &read_fds))
	{
		*retSize = 0;
		return -1;
	}
	

	//按行读串口数据
	fgets(ReadBuffer, size, m_fp);
	*retSize = strlen(ReadBuffer);
	return 0;
}



#if 0
int 
CComTransmit::ReadOneFrame(char *ReadBuffer, int buffLen, int *retSize)
{
	int i, nRet, readLen, temp = 0;
	char* pReadBuff = ReadBuffer;
	char* pTail = NULL, *pHead = NULL;

	usleep(100*100);
	for (;;)
	{
		readLen = 0;
		nRet = ReadCom(pReadBuff, buffLen, &readLen, 100);
		printf("readcom: nRet=%d, readLen=%d\n", nRet, readLen);
		if (nRet < 0)
		{
			*retSize = 0;
			return -1;
		}
		printf("---------find tail------\n");
		// 查找包尾，从后往前查
		for (i=readLen-1; i>=0; --i)
		{
			if (pReadBuff[i] == TAIL_FLAG)
			{
				pTail = &pReadBuff[i];
				goto FINDPKGHEAD;
			}	
		}
		pReadBuff += readLen;
		buffLen -= readLen;
	}

FINDPKGHEAD:
	// 查找包头
	for (pReadBuff=ReadBuffer; pReadBuff < pTail; ++pReadBuff)
	{
		if (*pReadBuff == HEAD_FLAG)
		{
			pHead = pReadBuff;
			break;
		}
	}
	if (pReadBuff == pTail)
	{	
		temp = *retSize = 0;
		return -1;
	}
	
	temp = *retSize = pTail - pHead + 1;
	// 将包头到包尾这段距离复制到缓冲区中
	for (i=0; pHead <= pTail; ++pHead, ++i)
	{
		ReadBuffer[i] = *pHead;
	}
	
	printf("temp=%d\n", temp);
	return temp;
}
#endif



int 
CComTransmit::ReadCom(char *ReadBuffer, int buffLen, int *retSize, int timeOut)
{
	fd_set 	read_fds;
	int 	retval;	
	struct timeval rdtimeout;
	rdtimeout.tv_sec  = timeOut / 1000;
	rdtimeout.tv_usec = (timeOut % 1000) * 1000;
	FD_ZERO(&read_fds);
	FD_SET(m_fd, &read_fds);
	
	if (select(m_fd+1, &read_fds, NULL, NULL, &rdtimeout) < 0)
	{
		//printf("[%s:%d]select time out.\n", __FILE__, __LINE__);
		*retSize = 0;
		return -1;
	}
	else
	{
		if (!FD_ISSET(m_fd, &read_fds))
		{
			//printf("[%s:%d]Serail read time out[%d]\n",  __FILE__, __LINE__, timeOut);
			*retSize = 0;
			return -1;
		}
		else
		{
			retval = read(m_fd, ReadBuffer, buffLen);
			if (retval < 0)
			{
				Debug(I_ERROR, "[%s:%d]read buffLen=%d, %m",  __FILE__, __LINE__, retval);
				*retSize = 0;
				return -1;
			}
			
			*retSize = retval;
			return retval;
		}
	}
}


int 
CComTransmit::WriteCom(char *WriteBuffer, int size, int timeOut)
{
	fd_set 	serial_write; 
	struct timeval WTtimeout;
	
	FD_ZERO(&serial_write);
	FD_SET(m_fd, &serial_write);

	WTtimeout.tv_sec  = timeOut / 1000;
	WTtimeout.tv_usec = (timeOut % 1000) * 1000;
	
	if (select(m_fd+1, NULL, &serial_write, NULL, &WTtimeout) < 0)
	{
		return -1;
	}
	
	if (!FD_ISSET(m_fd, &serial_write))
	{
		return -1;
	}

	return write(m_fd, WriteBuffer, size);

#if 0
	/*HI3515串口驱动随机的会把换行符丢掉，导致发命令给MCU失败，所以加此补丁*/
	char test_value[2];
	test_value[0] = 0xd;
	test_value[1] = 0xa;
	write(m_fd, test_value ,2);
#endif	
}




int
CComTransmit::SerialsAddPkgHeadAndTail(char* buff, int buffLen, u_int16 typeID, int switfNum, char* outBuff)
{
	int temp, i;
	unsigned short leng;
	u_int8 sendData[32] = {0};
	unsigned char* pSendData = sendData;
	char* pOutBuff = outBuff;
	u_int8 checkNum = 0;

	if (buffLen > SERIALS_PKG_LEN || NULL == outBuff) return -1;

	// 计算校验和，消息头和消息体
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
	// 消息体
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
	HexToASCII(sendData, (u_int8*)pOutBuff, 6);
	pOutBuff += 12;
	// 消息体
	if (buff)
	{
		HexToASCII((u_int8*)buff, (u_int8*)pOutBuff, buffLen);
		pOutBuff += (buffLen * 2);
	}
	// 校验和 1 个字节
	Uint8HexToStr(checkNum, (char*)pOutBuff);
	pOutBuff += 2;
	// 包尾
	memcpy(pOutBuff, &TAIL_FLAG, TAIL_FLAG_LEN);
	pOutBuff += TAIL_FLAG_LEN;
	return (pOutBuff - outBuff);
}



int
CComTransmit::SerialsDelPkgHeadAndTail(char* buff, int buffLen, 
	u_int16 &typeID, int &switfNum, char* outBuff)
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
	if ( 0 != memcmp(pBuff+buffLen-TAIL_FLAG_LEN, &TAIL_FLAG, TAIL_FLAG_LEN) )
	{
		return -1;
	}
	pBuff += SERIALS_HEAD_LEN;

	// 长度、typeID、流水号
	ASCIIToHex((u_int8*)pBuff, temp, 8);
	leng     = ntohs(*(u_int16*)temp);
	typeID   = ntohs(*(u_int16*)(temp+2));
	switfNum = ntohl(*(u_int32*)(temp+4));
	pBuff += 16;

	//printf("leng=%d\n", leng);
	//printf("typeID=%x\n", typeID);
	//printf("switfNum=%d\n", switfNum);

	// 消息头的检验和
	for (i=2; i<8; ++i)
	{
		checkNum ^= temp[i];
	}

	// 消息体
	if (leng > SERIALS_NULLPKG_LEN)
	{
		nRet = leng - SERIALS_NULLPKG_LEN;
		// 转换消息体
		ASCIIToHex((u_int8*)pBuff, (u_int8*)outBuff, nRet);
		// 消息体的校验和
		for (i=0; i<nRet; ++i)
		{
			checkNum ^= outBuff[i];
		}
		pBuff += (nRet * 2);
	}

	memset(temp, 0, 32);
	//printf("pBuff[0]=%c, pBuff[1]=%c\n", pBuff[0], pBuff[1]);
	ASCIIToHex((u_int8*)pBuff, temp, 1);
	if (checkNum != *(u_int16*)temp)
	{
		printf("checkNum is error. checkNum=%x, temp=%x\n", checkNum, *(u_int16*)temp);
		return -1;
	}

	return nRet;
}



// 函数功能: unsigned int十六进制转字符
void 
CComTransmit::Uint32HexTostr(u_int32 hex, char* str)
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

// 函数功能: 字符转unsigned int十六进制，转8个字节
int 
CComTransmit::StrToUint32Hex(u_int32* hex, const char* str)
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
CComTransmit::Uint16HexToStr(u_int16 hex, char *str)
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

// 字符转十六进制，转四个字节
int 
CComTransmit::StrToUint16Hex(u_int16* hex, const char* str)
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
CComTransmit::Uint8HexToStr(u_int8 hex, char *str)
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

// 字符转十六进制，转四个字节
int 
CComTransmit::StrToUint8Hex(u_int8* hex, const char* str)
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


void 
CComTransmit::HexToASCII(u_int8* phex, u_int8* pascii, int len)
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
            tmp = tmp + 'A' - 10;
        }
        *(pascii++) = tmp;
        tmp= (*(phex++))&0x0F;
        if (tmp < 10)
        {
            tmp += '0';
        }
        else
        {
            tmp = tmp + 'A' - 10;
        }
        *(pascii++)=tmp;
    }
}

int 
CComTransmit::ASCIIToHex(u_int8* ascii, u_int8* hex, int len)
{
    u_int8 tmp, tmp1;
    int i = 0;
    for(i=0; i<len; i++)
    {
        tmp = *(ascii++);
        if (tmp >= '0' && tmp <= '9')
        {
            tmp -= '0';
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
            tmp1 -= '0';
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
		
        tmp = (tmp<<4)&0xF0;
        tmp1 &= 0x0F;
        *(hex++) = tmp|tmp1;
    }
	return 0;
}


int
CComTransmit::SerialsAddFirstHeadAndTail(char* buff, int buffLen, char* outBuff)
{
	if (NULL == buff || NULL == outBuff)
	{
		return -1;
	}
	char* pOutBuff = outBuff;
	// 加包头
	memcpy(pOutBuff, SERIALS_HEAD, SERIALS_HEAD_LEN);
	pOutBuff += SERIALS_HEAD_LEN;
	// 长度
	Uint16HexToStr(buffLen, (char*)pOutBuff);
	pOutBuff += 4;
	// 消息体
	HexToASCII((u_int8*)buff, (u_int8*)pOutBuff, buffLen);
	pOutBuff += (buffLen * 2);
	// 包尾
	memcpy(pOutBuff, &TAIL_FLAG, TAIL_FLAG_LEN);
	pOutBuff += TAIL_FLAG_LEN;
	return (pOutBuff - outBuff);	
}


int
CComTransmit::SerialsDelFirstHeadAndTail(char* buff, int buffLen, char* outBuff)
{
	if (NULL == buff || NULL == outBuff)
	{
		return -1;
	}
	
	char* pBuff = buff;
	u_int16 leng = 0;
	//int nRet = 0;
	
	// 检查包头、包尾
	if ( 0 != memcmp(pBuff, &HEAD_FLAG, HEAD_FLAG_LEN) )
	{
		//printf("33333333333333333, *pBuff=%02x\n", *pBuff);
		return -1;
	}
	if ( 0 != memcmp(pBuff+buffLen-TAIL_FLAG_LEN, &TAIL_FLAG, TAIL_FLAG_LEN) )
	{
		return -1;
	}
	pBuff += HEAD_FLAG_LEN;

	// 长度
	StrToUint16Hex(&leng, pBuff);
	pBuff += 4;
	//printf("leng=%d\n", leng);

	// 提取消息体
	ASCIIToHex((u_int8*)pBuff, (u_int8*)outBuff, leng);
	return leng;
}


int
CComTransmit::SerialsAddSecondHeadAndTail(char* buff, int buffLen, u_int16 typeID, int switfNum, char* outBuff)
{
	int temp, i;
	unsigned short leng;
	u_int8 sendData[32] = {0};
	unsigned char* pSendData = sendData;
	char* pOutBuff = outBuff;
	u_int8 checkNum = 0;

	if (buffLen > SERIALS_PKG_LEN || NULL == outBuff) return -1;

	// 计算校验和，消息头和消息体
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
	// 消息体
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

	// 消息头 6 个字节
	HexToASCII(sendData, (u_int8*)pOutBuff, 6);
	pOutBuff += 12;
	// 消息体
	if (buff)
	{
		HexToASCII((u_int8*)buff, (u_int8*)pOutBuff, buffLen);
		pOutBuff += (buffLen * 2);
	}
	// 校验和 1 个字节
	Uint8HexToStr(checkNum, (char*)pOutBuff);
	pOutBuff += 2;
	
	return (pOutBuff - outBuff);
}

int
CComTransmit::SerialsDelSecondHeadAndTail(char* buff, int buffLen, u_int16 &typeID, int &switfNum, char* outBuff)
{
	if (NULL == buff || NULL == outBuff )
		return -1;

	char* pBuff = buff;
	u_int8 temp[32] = {0};
	u_int8 checkNum = 0;
	int nRet = 0, i = 0;

	// typeID、流水号
	ASCIIToHex((u_int8*)pBuff, temp, 6);
	typeID = ntohs(*(u_int16*)(temp));
	switfNum = ntohl(*(u_int32*)(temp+2));
	pBuff += 12;

	//printf("typeID=%x\n", typeID);
	//printf("switfNum=%d\n", switfNum);

	// 消息头的检验和
	for (i=2; i<8; ++i)
	{
		checkNum ^= temp[i];
	}

	// 消息体
	if (buffLen > SERIALS_NULLPKG_LEN*2)
	{
		nRet = buffLen - SERIALS_NULLPKG_LEN*2;
		// 转换消息体
		ASCIIToHex((u_int8*)pBuff, (u_int8*)outBuff, nRet);
		// 消息体的校验和
		for (i=0; i<nRet; ++i)
		{
			checkNum ^= outBuff[i];
		}
		pBuff += (nRet * 2);
	}

	memset(temp, 0, 32);
	//printf("pBuff[0]=%c, pBuff[1]=%c\n", pBuff[0], pBuff[1]);
	ASCIIToHex((u_int8*)pBuff, temp, 1);
	if (checkNum != *(u_int16*)temp)
	{
		printf("checkNum is error. checkNum=%x, temp=%x\n", checkNum, *(u_int16*)temp);
		return -1;
	}

	return nRet;
}











////////////////////////////////////////////////////////////////////////
SerBuff* comBuff = NULL;
CComTransmit* com = NULL;

void ClearSerialBuff()
{
	comBuff->clrBuff();
}

int InitSerialsCom(const char* serName)
{
	TerminalInfo terminal;
	terminal.baudrate = 115200;
	terminal.databit  = '8';
	terminal.parity   = 'N';
	terminal.stopbit  = '1';
	terminal.flowctrl = 0;
	strncpy(terminal.dev_path, serName, 32);

	int nRet = 0;
	com = new CComTransmit();
	if (NULL == com)
	{
		printf("[%s:%d]Create CComTransmit Failed.\n", __FUNCTION__, __LINE__);
		return -1;
	}
	comBuff = new SerBuff(4096);
	if (NULL == comBuff)
	{
		printf("[%s:%d]Create SerBuff Failed.\n", __FUNCTION__, __LINE__);
		delete com, com = NULL;
		return -1;
	}
	
	nRet = com->OpenCom(&terminal);
	if (nRet < 0)
	{
		printf("[%s:%d]Open serials failed. nRet = %d\n", __FUNCTION__, __LINE__, nRet);
		delete com, com = NULL;
		delete comBuff, comBuff = NULL;
		return -1;
	}
	return 0;
}

void UninitSerialsCom()
{
	if (com)
	{
		com->CloseCom();
		delete com, com = NULL;
	}
	if (comBuff)
	{
		delete comBuff, comBuff = NULL;
	}
}

static int 
SendDataToCom(char *sendBuff, int sendLen, int timeOut)
{
	int nRet = 0;
	if (NULL == com || NULL == sendBuff) 
	{
		printf("[%s:%d]Doesn't open serials\n", __FILE__, __LINE__);
		return -1;
	}

	// 加第一层包头
	char outBuff[256] = {0};
	nRet = CComTransmit::SerialsAddFirstHeadAndTail(sendBuff, sendLen, outBuff);
	if (nRet < 0)
	{
		printf("[%s:%d]Add First package Head And Tail Failed.\n", __FILE__, __LINE__);
		return -1;
	}
	// 发送
	nRet = com->WriteCom(outBuff, nRet, timeOut);
	if (nRet < 0)
	{
		Debug(I_ERROR, "write to serials failed. nRet=%d", nRet);
		return -1;
	}
	return nRet;
}


int 
SendDataToSerialsCom(char *sendBuff, int sendLen, int timeOut)
{
	// 互斥，避免同时向串口写数据，其实write函数也是互斥的
	int temp = 0;
	static pthread_mutex_t comMutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_lock(&comMutex);
	temp = SendDataToCom(sendBuff, sendLen, timeOut);
	pthread_mutex_unlock(&comMutex);
	// 写日记
	//Debug(I_ERROR, "send data to serials, dataLen=%d", temp);
	return temp;
}



//static FILE* pfile = NULL;
//static int inClt = 0;
static int 
RecvOne(char *ReadBuffer, int buffLen, int *retSize)
{
	int nRet = 0;
	int outLen = 0;
	char outBuff[512] = {0};

	// 休眠一小段时间，等待数据的到来
	//usleep(1000*10);

	// 从串口读数据，并加入到缓冲区中，如果缓冲区数据已经满了，丢掉这条数据
	memset(ReadBuffer, 0, buffLen);
	nRet = com->ReadCom(ReadBuffer, buffLen, &outLen, 100);
	if (nRet > 0)
	{
		comBuff->ADD_BuffToSerBuff(ReadBuffer, outLen);
	}
	

	//printf("@@@@@@@@@@@@\n");
	//printf("read from serials, readLen=%d\n", nRet);
	//printf("@@@@@@@@@@@@\n");

	// 从缓冲区中提取一帧数据
	memset(outBuff, 0, sizeof(outBuff));
	outLen = 0;
	nRet = comBuff->GET_FrmFromSerBuff(outBuff, &outLen);
	if (nRet <= 0)
	{
		*retSize = 0;
		return -1;
	}

	// 去除一帧数据的第一层包头和包尾
	memset(ReadBuffer, 0, buffLen);
	nRet = CComTransmit::SerialsDelFirstHeadAndTail(outBuff, outLen, ReadBuffer);
	if (nRet < 0)
	{
		*retSize = 0;
		return -1;
	}
	*retSize = nRet;
	return nRet;
}


//static unsigned int recvPkg1 = 0;
//static unsigned int recvPkg2 = 0;
static pthread_mutex_t RecvMutex = PTHREAD_MUTEX_INITIALIZER;
//static pthread_mutex_t pkgLock = PTHREAD_MUTEX_INITIALIZER;
int RecvOneFrame(char *ReadBuffer, int buffLen, int *retSize)
{
	// 互斥，避免同时从串口读取数据
	int temp = 0;
	pthread_mutex_lock(&RecvMutex);
	temp = RecvOne(ReadBuffer, buffLen, retSize);
	pthread_mutex_unlock(&RecvMutex);
	//if (temp > 0) 
	//{
	//	pthread_mutex_lock(&pkgLock);
	//	++recvPkg1, ++recvPkg2;
	//	pthread_mutex_unlock(&pkgLock);
	//}
	return temp;
}



int WriteDataToSerialsCom(char *sendBuff, int sendLen, int timeOut)
{
	int nRet = 0;
	if (NULL == com || NULL == sendBuff) 
	{
		printf("[%s:%d]Doesn't open serials\n", __FILE__, __LINE__);
		return -1;
	}

	// 发送
	nRet = com->WriteCom(sendBuff, sendLen, timeOut);
	if (nRet < 0)
	{
		printf("[%s:%d]write to serials failed. nRet=%d\n", __FUNCTION__, __LINE__, nRet);
		return -1;
	}
	return nRet;
}


int 
ReadOne(char *ReadBuffer, int buffLen, int *retSize)
{
	int nRet = 0;
	int outLen = 0;

	// 休眠一小段时间，等待数据的到来
	usleep(1000*10);

	// 从串口读数据，并加入到缓冲区中，如果缓冲区数据已经满了，丢掉这条数据
	memset(ReadBuffer, 0, buffLen);
	nRet = com->ReadCom(ReadBuffer, buffLen, &outLen, 10);
	if (nRet > 0)
	{
		comBuff->ADD_BuffToSerBuff(ReadBuffer, outLen);
	}

	// 从缓冲区中提取一帧数据
	memset(ReadBuffer, 0, buffLen);
	outLen = 0;
	nRet = comBuff->GET_FrmFromSerBuff(ReadBuffer, &outLen);
	if (nRet < 0)
	{
		*retSize = 0;
		return -1;
	}
	*retSize = outLen;
	return outLen;
}


int ReadOneFrame(char *ReadBuffer, int buffLen, int *retSize)
{
	// 互斥，避免同时从串口读取数据
	int temp = 0;
	static pthread_mutex_t ReadMutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_lock(&ReadMutex);
	temp = ReadOne(ReadBuffer, buffLen, retSize);
	pthread_mutex_unlock(&ReadMutex);
	return temp;
}

/*
void printReadComCnt1(void* arg)
{
	int temp;
	pthread_mutex_lock(&pkgLock);
	temp = recvPkg1;
	recvPkg1 = 0;
	pthread_mutex_unlock(&pkgLock);
	printf("scanned count in 1 second: %d\n", temp);
	return;
}

void printReadComCnt2(void* arg)
{
	int temp;
	pthread_mutex_lock(&pkgLock);
	temp = recvPkg2;
	recvPkg2 = 0;
	pthread_mutex_unlock(&pkgLock);
	printf("scanned count in 10 second: %d\n", temp);
	return;
}
*/




void
printHex(const unsigned char* buf, int len)
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



#if 0
static unsigned int switfNum = 1;
//SerBuff *serialsBuff;
int main()
{
	int nRet, sendLen, recvLen, i;
	u_int16 typeID;
	int num;
	char sendBuff[1024] = {0};
	char outBuff[1024] = {0};

	nRet = InitSerialsCom();
	if (nRet < 0)
	{
		return -1;
	}

	// 发送开始扫描 0x0103 
	sendLen = CComTransmit::SerialsAddPkgHeadAndTail(NULL, 0, 0x0103, switfNum, sendBuff);
	printHex((u_int8*)sendBuff, sendLen);
	nRet = SendDataToSerialsCom(sendBuff, sendLen, 3000);
	if (nRet < 0)
	{
		printf("send Data To SerialsCom Failed.");
		return -1;
	}

	while(1)
	{
		memset(sendBuff, 0, 1024);
		recvLen = 0;
		
		nRet = RecvOneFrame(sendBuff, 1024, &recvLen);
		printf("nRet=%d\n", nRet);
		if (nRet > 0)
		{
			printf("@@@@@@@@@@@@@@@@@@@@@, nRet=%d, recvLen=%d\n", nRet, recvLen);
			printHex((u_int8*)sendBuff, recvLen);
			printf("@@@@@@@@@@@@@@@@@@@@@\n");
			memset(outBuff, 0, 1024);

		#if 0	
			nRet = CComTransmit::SerialsDelPkgHeadAndTail(sendBuff, recvLen, typeID, num, outBuff);
			if (nRet > 0)
			{
				printHex((u_int8*)outBuff, nRet);
			}
			else
			{
				printf("@@@@@@@@@@@@@@@@@@@@@@@@\n");
				printf("--------------BUG...\n");
				printf("@@@@@@@@@@@@@@@@@@@@@@@@\n");
				exit(1);
			}
		#endif
		}
		usleep(100*3000);
	}

	UninitSerialsCom();
	return 0;
}

#endif



#if 0
// 测试串口协议，加包头，去包头
int main()
{
	int sendLen, nRet;
	u_int16 typeID;
	int num;
	char sendBuff[1024] = {0};
	char outBuff[1024] = {0};
	memset(outBuff, 'a', 5);

	sendLen = CComTransmit::SerialsAddPkgHeadAndTail(outBuff, 5, 0x0102, 1, sendBuff);
	printHex((u_int8*)sendBuff, sendLen);

	
	nRet = CComTransmit::SerialsDelPkgHeadAndTail(sendBuff, sendLen, typeID, num, outBuff);
	printHex((u_int8*)outBuff, nRet);

	return 0;
}
#endif

#if 0
// 测试串口协议，加包头，去包头
int main()
{
	int sendLen, nRet;
	char sendBuff[1024] = {0};
	char outBuff[1024] = {0};
	memset(outBuff, 'a', 5);

	sendLen = CComTransmit::SerialsAddFirstHeadAndTail(outBuff, 5, sendBuff);
	printHex((u_int8*)sendBuff, sendLen);

	
	nRet = CComTransmit::SerialsDelFirstHeadAndTail(sendBuff, sendLen, outBuff);
	printHex((u_int8*)outBuff, nRet);

	return 0;
}
#endif



