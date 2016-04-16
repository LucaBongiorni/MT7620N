#ifndef __PROCOTOL_H__
#define __PROCOTOL_H__

typedef unsigned char    u_int8;
typedef unsigned short   u_int16;
typedef unsigned int     u_int32;


// 包头和包尾标志
static const char HEAD[2]  = {0x02, 0x00};
static const int  HEAD_LEN = sizeof(HEAD);
static const char TAIL     = 0x03;
static const int  TAIL_LEN = sizeof(TAIL);



#define BEACON_CMD_RESET  		0x0001
#define BEACON_CMD_UP			0x0002
#define BEACOM_CMD_HEART		0x1000



#define BEACOM_CMD_GET_UPDATE_INFO          0xff00
#define BEACOM_CMD_GET_UPDATE_INFO_ACK      0xff01


class Procotol
{
public:
	Procotol(){};
	~Procotol(){};

public:
	// 函数功能: unsigned int十六进制转字符
	// 函数参数: hex, 输入参数，unsigned int 类型的数字
	//           str, 输出参数，转换成字符，8个字节
	static void Uint32HexTostr(u_int32 hex, char* str);

	// 函数功能: 字符转unsigned int十六进制，转8个字节
	// 函数参数: hex, 输出参数，unsigned int 类型的数字
	//           str, 输入参数，16进制数字字符
	static int StrToUint32Hex(u_int32* hex, char* str);
	
	// 函数功能: unsigned short十六进制转字符
	// 函数参数: hex unsigned short 类型的数字
	//           str 输出参数，转换成字符，4个字节
	static void Uint16HexToStr(u_int16 hex, char *str);
	
	// 函数功能: 字符转unsigned short十六进制，转四个字节
	// 函数参数: hex 输出参数，转换成的整数
	//           str 输入的字符
	static int StrToUint16Hex(u_int16* hex, char* str);
	
	// 函数功能: unsigned char十六进制转字符，转1个字节
	// 函数参数: hex unsigned char 类型的数字
	//           str 输出参数，转换成字符，2个字节           
	static void Uint8HexToStr(u_int8 hex, char *str);
	
	// 函数功能: 字符转十六进制，转2个字节
	// 函数参数: hex 输出参数，转换成的整数
	//           str 输入的字符
	static int StrToUint8Hex(u_int8* hex, char* str);

	
	// 函数功能: 加包头和包尾
	// 函数参数: content 输入的字符，加包头后的字符也通过该参数输出
	//           contentLen 输入的字符长度
	//           typeID 数据类型
	//           PkgTotal 发送的总包数
	//           PkgSeq 发送第几个包
	// 返 回 值: 成功返回加包头后的字符长度，失败返回-1
	static int AddPkgHeadAndTail(char* content, u_int16 contentLen, u_int16 typeID, char PkgTotal, char PkgSeq);

	// 函数功能: 去包头和包尾
	// 函数参数: content 输入的字符，去包头后的字符也通过该参数输出
	//           contentLen 输入的字符长度
	//           typeID 输出参数，数据包类型
	//           PkgTotal 输出参数，总共多少个包
	//           PkgSeq 输出参数，第几个包
	// 返 回 值: 成功返回去掉包头和包尾的字符长度，失败返回-1
	static int DelPkgHeadAndTail(char* content, int contentLen, u_int16* typeID, u_int8* PkgTotal, u_int8* PkgSeq);

	// 函数功能: 以十六进制打印字符
	// 函数参数: buf 输入打印的字符
	//           len 打印的长度
	static void printHex(const char* buf, int len);
};





#if 0
/*校验和*/
unsigned char c = 0x00;
pBuf[12] = 0x00;
for(i=0; i<FrameLen; ++i)
{
    c += pBuf[i];
}
pBuf[12] = 0xAA - c;
#endif




#endif /*__PROCOTOL_H__*/

