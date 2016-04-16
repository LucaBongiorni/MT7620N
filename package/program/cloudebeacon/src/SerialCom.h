#ifndef __SERIAL_COM__H__
#define __SERIAL_COM__H__

#include <stdio.h>
#include <stdlib.h>

#ifndef u_int8
#define u_int8 unsigned char
#endif
#ifndef u_int16
#define u_int16 unsigned short
#endif
#ifndef u_int32
#define u_int32 unsigned int
#endif

#if 0
#ifndef write_log
#define write_log(fmt,args...) printf("[%s][%d]"fmt"\n",__FUNCTION__,__LINE__,##args)
#endif
#ifndef write_error_log
#define write_error_log(fmt,args...)  printf("[%s][%d]Error:"fmt"\n",__FUNCTION__,__LINE__,##args)
#endif
#ifndef write_normal_log
#define write_normal_log(fmt,args...) printf("[%s][%d]Log:"fmt"\n",__FUNCTION__,__LINE__,##args)
#endif
#endif

#define SERIALS_PKG_LEN    1024
static const char SERIALS_HEAD[1] = {0x02};
#define SERIALS_HEAD_LEN   (sizeof(SERIALS_HEAD))
const int SERIALS_NULLPKG_LEN  = 7;



#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


typedef struct{
	int second;
	int usecond;
}timeOut_set;

typedef struct {
	unsigned long 	baudrate;	    /*波特率*/
	unsigned char 	databit;		/*数据位，取值5、6、7、8*/
	unsigned char 	parity;		    /*奇偶校验位，取值'n':无校验'o':奇校验'e'偶校验*/
	unsigned char 	stopbit;		/*停止位，取值1,2*/
	char			dev_path[32];   /*串口设备路径*/
	int 			flowctrl;		/*流控制，用于通知可写可读，select时需要设置这个*/
	//timeOut_set	read;		    /*读超时设置*/
	//timeOut_set	write;		    /*写超时设置*/
}TerminalInfo;


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#ifdef USE_OPENWRT
#define DEF_DEV_PATH    "/dev/ttyS0"
#else
#define DEF_DEV_PATH    "/dev/ttyUSB0"
#endif

class CComTransmit
{
public:
	// 函数功能: 构造函数
	CComTransmit();
	// 函数功能: 析构函数
	~CComTransmit();
	// 函数功能: 
	// 函数参数:
	// 返 回 值:
	int OpenCom(TerminalInfo *com_dev);
	// 函数功能: 
	// 返 回 值:
	int CloseCom();
	// 函数功能: 根据类型清空输入输出缓冲区
	// 函数参数: 0 清空输入队列
	//           1 清空输出队列
	//           2 同时清空输入和输出队列
	// 返 回 值: 成功返回0，失败返回 -1
	int IOFlush(int mode);
	// 函数功能: 按行读取串口数据
	// 函数参数:
	// 
	// 
	// 
	// 返 回 值:
	int FgetsCom(char *ReadBuffer, int size, int *retSize, int timeOut);
	// 函数功能: 读取串口数据
	// 函数参数: ReadBuffer，输入缓冲区，size，输入缓冲区的大小
	//           retSize，输出读取到的字符长度
	//           teme_out，设置超时，单位毫秒
	// 返 回 值: 成功返回读取到的字符长度，失败返回-1
	int ReadCom(char *ReadBuffer, int buffLen, int *retSize, int timeOut);
	// 函数功能: 向串口写入数据
	// 函数参数: WriteBuffer，写入数据地址，size，写入数据长度
	// 
	// 
	// 返 回 值:
	int WriteCom(char *WriteBuffer, int size, int timeOut);

	/////////////////////////////////////////////////////////////////////////////////////////
	// 函数功能: 串口协议加包头和包尾(加了两层包头包尾)
	// 函数参数: buff，消息体；buffLen，消息体长度；
	//           typeID，消息体类型
	//           switfNum, 流水号
	//           outBuff, 输出参数，输出加包头以及转换后的长度
	// 返 回 值: 返回加上包头后outBuff的长度，小于0为错误
	static int
	SerialsAddPkgHeadAndTail(char* buff, int buffLen, u_int16 typeID, int switfNum, char* outBuff);

	// 函数功能: 串口协议去包头和包尾，以及提取typeid和流水号(去掉两层包头包尾)
	// 函数参数: buff，数据包内容，buffLen，数据包长度
	//           typeID，输出参数，数据包类型
	//           switfNum，输出参数，流水号
	//           outBuff，输出参数，去掉包头后的字符
	// 返 回 值: 去掉包头后的字符长度，小于0为错误
	static int
	SerialsDelPkgHeadAndTail(char* buff, int buffLen, u_int16 &typeID, int &switfNum, char* outBuff);

	// 函数功能: 串口协议加包头和包尾(加第一层包头包尾)
	// 函数参数: buff，消息体；buffLen，消息体长度；
	//           outBuff, 输出参数，输出加包头以及转换后的长度
	// 返 回 值: 返回加上包头后outBuff的长度，小于0为错误
	static int
	SerialsAddFirstHeadAndTail(char* buff, int buffLen, char* outBuff);
	
	// 函数功能: 去掉串口协议去包头和包尾，(去掉第一层包头包尾)
	// 函数参数: buff，数据包内容，buffLen，数据包长度
	//           outBuff，输出参数，去掉包头后的字符
	// 返 回 值: 去掉包头后的字符长度，小于0为错误
	static int
	SerialsDelFirstHeadAndTail(char* buff, int buffLen, char* outBuff);

	// 函数功能: 串口协议加包头和包尾(加第二层包头包尾)
	// 函数参数: buff，消息体；buffLen，消息体长度；
	//           typeID，消息体类型
	//           switfNum, 流水号
	//           outBuff, 输出参数，输出加包头以及转换后的长度
	// 返 回 值: 返回加上包头后outBuff的长度，小于0为错误
	static int
	SerialsAddSecondHeadAndTail(char* buff, int buffLen, u_int16 typeID, int switfNum, char* outBuff);

	// 函数功能: 串口协议去包头和包尾，以及提取typeid和流水号(去掉第二层包头包尾)
	// 函数参数: buff，数据包内容，buffLen，数据包长度
	//           typeID，输出参数，数据包类型
	//           switfNum，输出参数，流水号
	//           outBuff，输出参数，去掉包头后的字符
	// 返 回 值: 去掉包头后的字符长度，小于0为错误
	static int
	SerialsDelSecondHeadAndTail(char* buff, int buffLen, u_int16 &typeID, int &switfNum, char* outBuff);

	// 函数功能: 字符转16进制
	// 函数参数: ascii, 字符，
	//           hex，输出参数，转换后的16进制
	//           len，转换长度
	// 返 回 值: 0，成功，-1，失败
	static int 
	ASCIIToHex(u_int8* ascii, u_int8* hex, int len);

	// 函数功能: 16进制转字符
	// 函数参数: phex,
	//           pascii,
	//           len,
	// 返 回 值: 
	static void 
	HexToASCII(u_int8* phex, u_int8* pascii, int len);

	// 函数功能: unsigned int十六进制转字符
    // 函数参数: hex, 输入参数，unsigned int 类型的数字
    //           str, 输出参数，转换成字符，8个字节
    static void
    Uint32HexTostr(u_int32 hex, char* str);
    // 函数功能: 字符转unsigned int十六进制，转8个字节
    // 函数参数: hex, 输出参数，unsigned int 类型的数字
    //           str, 输入参数，16进制数字字符
    // 返 回 值: 成功返回0，失败返回 -1
    static int
    StrToUint32Hex(u_int32* hex, const char* str);

    // 函数功能: unsigned short十六进制转字符
    // 函数参数: hex unsigned short 类型的数字
    //           str 输出参数，转换成字符，4个字节
    static void
    Uint16HexToStr(u_int16 hex, char *str);

    // 函数功能: 字符转unsigned short十六进制，转四个字节
    // 函数参数: hex 输出参数，转换成的整数
    //           str 输入的字符
    // 返 回 值: 成功返回0，失败返回 -1
    static int
    StrToUint16Hex(u_int16* hex, const char* str);

    // 函数功能: unsigned char十六进制转字符，转1个字节
    // 函数参数: hex unsigned char 类型的数字
    //           str 输出参数，转换成字符，2个字节
    static void
    Uint8HexToStr(u_int8 hex, char *str);

    // 函数功能: 字符转十六进制，转2个字节
    // 函数参数: hex 输出参数，转换成的整数
    //           str 输入的字符
    // 返 回 值: 成功返回0，失败返回 -1
    static int
    StrToUint8Hex(u_int8* hex, const char* str);
	

private:
	// 函数功能: 打印串口基本信息
	// 函数参数:
	// 返 回 值:
	void TraceComPar(TerminalInfo *com_dev);
	// 函数功能: 设置串口波特率
	// 函数参数: fd，打开的串口描述符
	//           speed，波特率
	// 返 回 值: 成功返回0，失败返回 -1；
	int  SetComSpeed(int fd, unsigned int speed);
	// 函数功能: 设置串口数据位，奇偶位，停止位，校验，控制模式
	// 函数参数: fd，打开的串口描述符，
	//           databits，数据位
	//           stopbits，停止位
	//           parity，奇偶位
	//           flowctrl，控制模式
	// 返 回 值: 成功返回0，失败返回-1
	int  SetComParity(int fd, int databits, int stopbits, int parity, int flowctrl);
	// 函数功能: 设置串口为原始模式
	// 函数参数:
	// 返 回 值:
	int  SetComRawMode(int fd);

private:
	int 	m_fd;               // 文件描述符
	FILE   *m_fp;	            // 
	bool	m_bOpenFlag;        // 打开标志
	TerminalInfo  m_ComDev;     // 设备信息
};

#if 0
	// 无效验 8 位
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;

	// 奇效验(Odd) 7 位
	option.c_cflag |= ~PARENB;
	option.c_cflag &= ~PARODD;
	option.c_cflag &= ~CSTOPB;
	option.c_cflag &= ~CSIZE;
	option.c_cflag |= ~CS7

	// 偶效验 7 位
	option.c_cflag &= ~PARENB;
	option.c_cflag |= ~PARODD;
	option.c_cflag &= ~CSTOPB;
	option.c_cflag &= ~CSIZE;
	option.c_cflag |= ~CS7

	// Space 效验
	option.c_cflag &= ~PARENB;
	option.c_cflag &= ~CSTOPB;
	option.c_cflag &= &~CSIZE;
	option.c_cflag |= ~CS8
#endif

// 函数功能: 向串口发送数据
// 函数参数: sendBuff，发送的数据内容；
//           sendLen，发送的数据内容长度
//           timeOut，超时，单位毫秒
// 返 回 值: 成功返回发送的长度，失败返回0；
int WriteDataToSerialsCom(char *sendBuff, int sendLen, int timeOut, char netFlag, int phoneFlag);


// 函数功能: 读取一条串口指令
// 函数参数: ReadBuffer，读入缓冲区，buffLen，缓冲区大小
//           retSize，输出参数，读取到的长度，失败为0
// 返 回 值: 成功返回读取到的长度，失败返回-1；
int ReadOneFrame(char *ReadBuffer, int buffLen, int *retSize);



int 
ReadOne(char *ReadBuffer, int buffLen, int *retSize);








/*********************************************************************************/
// 函数功能: 初始化串口
int InitSerialsCom(const char* serName);


// 函数功能: 最后销毁串口
void UninitSerialsCom();


// 函数功能: 向串口发送数据
// 函数参数: sendBuff，发送的数据内容；(消息头+消息体，不包含第一层)
//           sendLen，发送的数据内容长度
//           timeOut，超时，单位毫秒
// 返 回 值: 成功返回发送的长度，失败返回0；
int SendDataToSerialsCom(char *sendBuff, int sendLen, int timeOut);


// 函数功能: 读取一条串口指令(消息头+消息体，不包含第一层)
// 函数参数: ReadBuffer，读入缓冲区，buffLen，缓冲区大小
//           retSize，输出参数，读取到的长度，失败为0
// 返 回 值: 成功返回读取到的长度，失败返回-1；
int RecvOneFrame(char *ReadBuffer, int buffLen, int *retSize);


// 函数功能: 清空缓冲区数据
// 函数参数: 
void ClearSerialBuff();
/*********************************************************************************/

/*
void printReadComCnt1(void* arg);
void printReadComCnt2(void* arg);
*/


#endif /*__SERIAL_COM__H__*/

