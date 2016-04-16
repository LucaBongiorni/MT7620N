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
	unsigned long 	baudrate;	    /*������*/
	unsigned char 	databit;		/*����λ��ȡֵ5��6��7��8*/
	unsigned char 	parity;		    /*��żУ��λ��ȡֵ'n':��У��'o':��У��'e'żУ��*/
	unsigned char 	stopbit;		/*ֹͣλ��ȡֵ1,2*/
	char			dev_path[32];   /*�����豸·��*/
	int 			flowctrl;		/*�����ƣ�����֪ͨ��д�ɶ���selectʱ��Ҫ�������*/
	//timeOut_set	read;		    /*����ʱ����*/
	//timeOut_set	write;		    /*д��ʱ����*/
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
	// ��������: ���캯��
	CComTransmit();
	// ��������: ��������
	~CComTransmit();
	// ��������: 
	// ��������:
	// �� �� ֵ:
	int OpenCom(TerminalInfo *com_dev);
	// ��������: 
	// �� �� ֵ:
	int CloseCom();
	// ��������: ������������������������
	// ��������: 0 ����������
	//           1 ����������
	//           2 ͬʱ���������������
	// �� �� ֵ: �ɹ�����0��ʧ�ܷ��� -1
	int IOFlush(int mode);
	// ��������: ���ж�ȡ��������
	// ��������:
	// 
	// 
	// 
	// �� �� ֵ:
	int FgetsCom(char *ReadBuffer, int size, int *retSize, int timeOut);
	// ��������: ��ȡ��������
	// ��������: ReadBuffer�����뻺������size�����뻺�����Ĵ�С
	//           retSize�������ȡ�����ַ�����
	//           teme_out�����ó�ʱ����λ����
	// �� �� ֵ: �ɹ����ض�ȡ�����ַ����ȣ�ʧ�ܷ���-1
	int ReadCom(char *ReadBuffer, int buffLen, int *retSize, int timeOut);
	// ��������: �򴮿�д������
	// ��������: WriteBuffer��д�����ݵ�ַ��size��д�����ݳ���
	// 
	// 
	// �� �� ֵ:
	int WriteCom(char *WriteBuffer, int size, int timeOut);

	/////////////////////////////////////////////////////////////////////////////////////////
	// ��������: ����Э��Ӱ�ͷ�Ͱ�β(���������ͷ��β)
	// ��������: buff����Ϣ�壻buffLen����Ϣ�峤�ȣ�
	//           typeID����Ϣ������
	//           switfNum, ��ˮ��
	//           outBuff, �������������Ӱ�ͷ�Լ�ת����ĳ���
	// �� �� ֵ: ���ؼ��ϰ�ͷ��outBuff�ĳ��ȣ�С��0Ϊ����
	static int
	SerialsAddPkgHeadAndTail(char* buff, int buffLen, u_int16 typeID, int switfNum, char* outBuff);

	// ��������: ����Э��ȥ��ͷ�Ͱ�β���Լ���ȡtypeid����ˮ��(ȥ�������ͷ��β)
	// ��������: buff�����ݰ����ݣ�buffLen�����ݰ�����
	//           typeID��������������ݰ�����
	//           switfNum�������������ˮ��
	//           outBuff�����������ȥ����ͷ����ַ�
	// �� �� ֵ: ȥ����ͷ����ַ����ȣ�С��0Ϊ����
	static int
	SerialsDelPkgHeadAndTail(char* buff, int buffLen, u_int16 &typeID, int &switfNum, char* outBuff);

	// ��������: ����Э��Ӱ�ͷ�Ͱ�β(�ӵ�һ���ͷ��β)
	// ��������: buff����Ϣ�壻buffLen����Ϣ�峤�ȣ�
	//           outBuff, �������������Ӱ�ͷ�Լ�ת����ĳ���
	// �� �� ֵ: ���ؼ��ϰ�ͷ��outBuff�ĳ��ȣ�С��0Ϊ����
	static int
	SerialsAddFirstHeadAndTail(char* buff, int buffLen, char* outBuff);
	
	// ��������: ȥ������Э��ȥ��ͷ�Ͱ�β��(ȥ����һ���ͷ��β)
	// ��������: buff�����ݰ����ݣ�buffLen�����ݰ�����
	//           outBuff�����������ȥ����ͷ����ַ�
	// �� �� ֵ: ȥ����ͷ����ַ����ȣ�С��0Ϊ����
	static int
	SerialsDelFirstHeadAndTail(char* buff, int buffLen, char* outBuff);

	// ��������: ����Э��Ӱ�ͷ�Ͱ�β(�ӵڶ����ͷ��β)
	// ��������: buff����Ϣ�壻buffLen����Ϣ�峤�ȣ�
	//           typeID����Ϣ������
	//           switfNum, ��ˮ��
	//           outBuff, �������������Ӱ�ͷ�Լ�ת����ĳ���
	// �� �� ֵ: ���ؼ��ϰ�ͷ��outBuff�ĳ��ȣ�С��0Ϊ����
	static int
	SerialsAddSecondHeadAndTail(char* buff, int buffLen, u_int16 typeID, int switfNum, char* outBuff);

	// ��������: ����Э��ȥ��ͷ�Ͱ�β���Լ���ȡtypeid����ˮ��(ȥ���ڶ����ͷ��β)
	// ��������: buff�����ݰ����ݣ�buffLen�����ݰ�����
	//           typeID��������������ݰ�����
	//           switfNum�������������ˮ��
	//           outBuff�����������ȥ����ͷ����ַ�
	// �� �� ֵ: ȥ����ͷ����ַ����ȣ�С��0Ϊ����
	static int
	SerialsDelSecondHeadAndTail(char* buff, int buffLen, u_int16 &typeID, int &switfNum, char* outBuff);

	// ��������: �ַ�ת16����
	// ��������: ascii, �ַ���
	//           hex�����������ת�����16����
	//           len��ת������
	// �� �� ֵ: 0���ɹ���-1��ʧ��
	static int 
	ASCIIToHex(u_int8* ascii, u_int8* hex, int len);

	// ��������: 16����ת�ַ�
	// ��������: phex,
	//           pascii,
	//           len,
	// �� �� ֵ: 
	static void 
	HexToASCII(u_int8* phex, u_int8* pascii, int len);

	// ��������: unsigned intʮ������ת�ַ�
    // ��������: hex, ���������unsigned int ���͵�����
    //           str, ���������ת�����ַ���8���ֽ�
    static void
    Uint32HexTostr(u_int32 hex, char* str);
    // ��������: �ַ�תunsigned intʮ�����ƣ�ת8���ֽ�
    // ��������: hex, ���������unsigned int ���͵�����
    //           str, ���������16���������ַ�
    // �� �� ֵ: �ɹ�����0��ʧ�ܷ��� -1
    static int
    StrToUint32Hex(u_int32* hex, const char* str);

    // ��������: unsigned shortʮ������ת�ַ�
    // ��������: hex unsigned short ���͵�����
    //           str ���������ת�����ַ���4���ֽ�
    static void
    Uint16HexToStr(u_int16 hex, char *str);

    // ��������: �ַ�תunsigned shortʮ�����ƣ�ת�ĸ��ֽ�
    // ��������: hex ���������ת���ɵ�����
    //           str ������ַ�
    // �� �� ֵ: �ɹ�����0��ʧ�ܷ��� -1
    static int
    StrToUint16Hex(u_int16* hex, const char* str);

    // ��������: unsigned charʮ������ת�ַ���ת1���ֽ�
    // ��������: hex unsigned char ���͵�����
    //           str ���������ת�����ַ���2���ֽ�
    static void
    Uint8HexToStr(u_int8 hex, char *str);

    // ��������: �ַ�תʮ�����ƣ�ת2���ֽ�
    // ��������: hex ���������ת���ɵ�����
    //           str ������ַ�
    // �� �� ֵ: �ɹ�����0��ʧ�ܷ��� -1
    static int
    StrToUint8Hex(u_int8* hex, const char* str);
	

private:
	// ��������: ��ӡ���ڻ�����Ϣ
	// ��������:
	// �� �� ֵ:
	void TraceComPar(TerminalInfo *com_dev);
	// ��������: ���ô��ڲ�����
	// ��������: fd���򿪵Ĵ���������
	//           speed��������
	// �� �� ֵ: �ɹ�����0��ʧ�ܷ��� -1��
	int  SetComSpeed(int fd, unsigned int speed);
	// ��������: ���ô�������λ����żλ��ֹͣλ��У�飬����ģʽ
	// ��������: fd���򿪵Ĵ�����������
	//           databits������λ
	//           stopbits��ֹͣλ
	//           parity����żλ
	//           flowctrl������ģʽ
	// �� �� ֵ: �ɹ�����0��ʧ�ܷ���-1
	int  SetComParity(int fd, int databits, int stopbits, int parity, int flowctrl);
	// ��������: ���ô���Ϊԭʼģʽ
	// ��������:
	// �� �� ֵ:
	int  SetComRawMode(int fd);

private:
	int 	m_fd;               // �ļ�������
	FILE   *m_fp;	            // 
	bool	m_bOpenFlag;        // �򿪱�־
	TerminalInfo  m_ComDev;     // �豸��Ϣ
};

#if 0
	// ��Ч�� 8 λ
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;

	// ��Ч��(Odd) 7 λ
	option.c_cflag |= ~PARENB;
	option.c_cflag &= ~PARODD;
	option.c_cflag &= ~CSTOPB;
	option.c_cflag &= ~CSIZE;
	option.c_cflag |= ~CS7

	// żЧ�� 7 λ
	option.c_cflag &= ~PARENB;
	option.c_cflag |= ~PARODD;
	option.c_cflag &= ~CSTOPB;
	option.c_cflag &= ~CSIZE;
	option.c_cflag |= ~CS7

	// Space Ч��
	option.c_cflag &= ~PARENB;
	option.c_cflag &= ~CSTOPB;
	option.c_cflag &= &~CSIZE;
	option.c_cflag |= ~CS8
#endif

// ��������: �򴮿ڷ�������
// ��������: sendBuff�����͵��������ݣ�
//           sendLen�����͵��������ݳ���
//           timeOut����ʱ����λ����
// �� �� ֵ: �ɹ����ط��͵ĳ��ȣ�ʧ�ܷ���0��
int WriteDataToSerialsCom(char *sendBuff, int sendLen, int timeOut, char netFlag, int phoneFlag);


// ��������: ��ȡһ������ָ��
// ��������: ReadBuffer�����뻺������buffLen����������С
//           retSize�������������ȡ���ĳ��ȣ�ʧ��Ϊ0
// �� �� ֵ: �ɹ����ض�ȡ���ĳ��ȣ�ʧ�ܷ���-1��
int ReadOneFrame(char *ReadBuffer, int buffLen, int *retSize);



int 
ReadOne(char *ReadBuffer, int buffLen, int *retSize);








/*********************************************************************************/
// ��������: ��ʼ������
int InitSerialsCom(const char* serName);


// ��������: ������ٴ���
void UninitSerialsCom();


// ��������: �򴮿ڷ�������
// ��������: sendBuff�����͵��������ݣ�(��Ϣͷ+��Ϣ�壬��������һ��)
//           sendLen�����͵��������ݳ���
//           timeOut����ʱ����λ����
// �� �� ֵ: �ɹ����ط��͵ĳ��ȣ�ʧ�ܷ���0��
int SendDataToSerialsCom(char *sendBuff, int sendLen, int timeOut);


// ��������: ��ȡһ������ָ��(��Ϣͷ+��Ϣ�壬��������һ��)
// ��������: ReadBuffer�����뻺������buffLen����������С
//           retSize�������������ȡ���ĳ��ȣ�ʧ��Ϊ0
// �� �� ֵ: �ɹ����ض�ȡ���ĳ��ȣ�ʧ�ܷ���-1��
int RecvOneFrame(char *ReadBuffer, int buffLen, int *retSize);


// ��������: ��ջ���������
// ��������: 
void ClearSerialBuff();
/*********************************************************************************/

/*
void printReadComCnt1(void* arg);
void printReadComCnt2(void* arg);
*/


#endif /*__SERIAL_COM__H__*/

