#ifndef __PROCOTOL_H__
#define __PROCOTOL_H__

#include "netSockTcp.h"


#ifndef u_int8
#define u_int8 unsigned char
#endif 
#ifndef u_int16
#define u_int16 unsigned short
#endif 
#ifndef u_int32
#define u_int32 unsigned int 
#endif 



// ��ͷ�Ͱ�β��־
const int AddLength = 4;
static const char HEAD[2]  = {0x02, 0x00};
#define HEAD_LEN  (sizeof(HEAD))
static const char TAIL     = 0x03;
#define TAIL_LEN  (sizeof(TAIL))

// ��ȥ��Ϣ�壬Ĭ�ϵ�Э��ͷ���ȣ��ɳ����ɶ�
#define DEF_PRO_HEAD_LEN     32 

#define MAC_SERVER 1
#define BEACON_SERVER 2

class Procotol
{
public:
	Procotol(){};
	~Procotol(){};

public:
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
	
	// ��������: �Ӱ�ͷ�Ͱ�β
	// ��������: content ������ַ����Ӱ�ͷ����ַ�Ҳͨ���ò������
	//           contentLen ������ַ�����
	//           typeID ��������
	//           PkgTotal ���͵��ܰ���
	//           PkgSeq ���͵ڼ�����
	// �� �� ֵ: �ɹ����ؼӰ�ͷ����ַ����ȣ�ʧ�ܷ���-1
	static int 
	AddPkgHeadAndTail(char* content, u_int16 contentLen, u_int16 typeID, char PkgTotal, char PkgSeq);

	// ��������: �Ӱ�ͷ�Ͱ�β
	// ��������: content ������ַ����Ӱ�ͷ����ַ�Ҳͨ���ò������
	//           contentLen ������ַ�����
	//           typeID ��������
	//           PkgTotal ���͵��ܰ���
	//           PkgSeq ���͵ڼ�����
	// �� �� ֵ: �ɹ����ؼӰ�ͷ����ַ����ȣ�ʧ�ܷ���-1
	static int 
	NetAddPkgHeadAndTail(char* content, u_int16 contentLen, u_int16 typeID, char PkgTotal, char PkgSeq);


	// ��������: ȥ��ͷ�Ͱ�β
	// ��������: content ������ַ���ȥ��ͷ����ַ�Ҳͨ���ò������
	//           contentLen ������ַ�����
	//           typeID ������������ݰ�����
	//           PkgTotal ����������ܹ����ٸ���
	//           PkgSeq ����������ڼ�����
	// �� �� ֵ: �ɹ�����ȥ����ͷ�Ͱ�β���ַ����ȣ�ʧ�ܷ���-1
	static int 
	DelPkgHeadAndTail(char* content, int contentLen, u_int16* typeID, u_int8* PkgTotal, u_int8* PkgSeq);

	// ��������: ����ͷ�Ƿ���ȷ����������Ϣ�峤��
	// ��������: content�������������ͷ����
	//           length�������������Ϣ�峤��
	// �� �� ֵ: �ɹ�����0�� ʧ�ܷ���-1��
	static int 
	CheckPkgHead(const char* content, u_int16* length);

	// ��������: ����ͷ�Ƿ���ȷ����������Ϣ�峤��
	// ��������: content�������������ͷ����
	//           length�������������Ϣ�峤��
	// �� �� ֵ: �ɹ�����0�� ʧ�ܷ���-1��
	static int 
	NetCheckPkgHead(const char* content, u_int16* length);
	
	// ��������: ��ʮ�����ƴ�ӡ�ַ�
	// ��������: buf �����ӡ���ַ�
	//           len ��ӡ�ĳ���
	static void 
	printHex(const char* buf, int len);

	// ��������: ��װ�㲥����Ϣ��
	// �� �� ֵ: �ɹ�������װ�����Ϣ�壬ʧ�ܷ��� NULL; 
	static char* 
	MakeBcastMes();

	// ��������: ��װ�����������CloudBeacon�豸�Ļ�������
	// �� �� ֵ: ������װ������ݣ�ʹ��֮����Ҫ�ͷţ�ʧ�ܷ���NULL
	static char* 
	MakeSerGetCBCInfoAck();

	// ��������: �����������mac��ַ��Ϣ
	// ��������: macInfo: �������: ��proc�ļ���ȡ������������
	//           outLen: �������: �������ݳ��ȣ�
	// �� �� ֵ: ������װ������ݣ�ʹ��֮����Ҫ�ͷţ�ʧ�ܷ���NULL
	static char*
	MakeUploadMacInfo(char* macInfo, int& outLen);

	static int
	RecvOnePkg(int sockFD, char* recvBuff, int buffLen, int timeOut);
};

#if 0
void (*pUint32HexToStr)(u_int32, char*) = Procotol::Uint32HexTostr;
int  (*pStrToUint32Hex)(u_int32*, const char*) = Procotol::StrToUint32Hex; 
void (*pUint16HexToStr)(u_int16, char *) = Procotol::Uint16HexToStr;
int  (*pStrToUint16Hex)(u_int16*, const char*) = Procotol::StrToUint16Hex; 
void (*pUint8HexToStr)(u_int16, char *) = Procotol::Uint8HexToStr;
int  (*pStrToUint8Hex)(u_int16*, const char*) = Procotol::StrToUint8Hex; 
int  (*pAddPkgHeadAndTail)(char*, u_int16, u_int16, char, char) = Procotol::AddPkgHeadAndTail;
int  (*pDelPkgHeadAndTail)(char*, int, u_int16*, u_int8*, u_int8*) = Procotol::DelPkgHeadAndTail;
int  (*pCheckPkgHead)(const char*, u_int16*) = Procotol::CheckPkgHead;
void (*pPrintHex)(const char*, int) = Procotol::printHex;
#endif

#define SuccessJSON     "{\"returnVal\": 0}"
#define FailedJSON      "{\"returnVal\": 1}"
#define BeenBinded      "{\"returnVal\": 2}"
#define UnBinded    	"{\"returnVal\": 2}"

#define RETURN_JSON_0     "{\"returnVal\": 0}"
#define RETURN_JSON_1     "{\"returnVal\": 1}"
#define RETURN_JSON_2     "{\"returnVal\": 2}"
#define RETURN_JSON_3     "{\"returnVal\": 3}"
#define RETURN_JSON_4     "{\"returnVal\": 4}"
#define RETURN_JSON_5     "{\"returnVal\": 5}"
#define RETURN_JSON_6     "{\"returnVal\": 6}"
#define RETURN_JSON_7     "{\"returnVal\": 7}"




#define MES_RST_SCAN_DEV             0x0102      // ��λɨ��ģ��
#define MES_BEG_SCAN_DEV             0x0103      // ��ʼɨ���豸
#define MES_STP_SCAN_DEV	         0x0104      // ֹͣɨ���豸
#define MES_REP_SCAN_INFO            0x0105      // �ϱ�ɨ����
#define MES_BLD_DEV_CONNECT          0x0106      // �����豸����
#define MES_BRK_DEV_CONNECT          0x0107      // �Ͽ�����
#define MES_SET_SERVER_PARAM         0x0108      // ���÷������
#define MES_FIND_SERVER_PARAM        0x0109      // ��ѯ�������
#define MES_REP_SERVER_PARAM         0x010a      // �ϱ��������
#define MES_RST_CBC_BTH              0x010b      // ��λcloudbeacon�ϵ�Bluetooth



#define MES_BCT_GET_CON_INFO         0x2fff		 // �㲥��ȡ������Ϣ

#define MES_SND_HEART                0x1000      // ����������
#define MES_SND_HEART_ACK            0x1000      // ������ȷ��

#define MES_APY_PUB_KEY			     0x1001      // �����������Կ
#define MES_APY_PUB_KEY_ACK          0x1002      // ���������ع�Կ
#define MES_SND_UID_TO_SER           0x1003      // ��������������
#define MES_SND_UID_TO_SER_ACK       0x1004      // ������������֤���

#define MES_SER_GET_CBC_INFO         0x1005      // ��������ȡCloudBeacon�豸�Ļ�������
#define MES_SER_GET_CBC_INFO_ACK     0x1006      // �����������CloudBeacon�豸�Ļ�������

#define MES_UPL_PHO_MAC_INFO         0x1007      // �ϴ� WiFi ɨ�赽�� MAC ��ַ
#define MES_UPL_PHO_MAC_INFO_ACK     0x1008      // ����������

#define MES_SET_MAC_SER_INFO         0x1009      // ���ÿͻ�Mac������������Ϣ
#define MES_SET_MAC_SER_INFO_ACK     0x100a      // �������ÿͻ�Mac������������Ϣ���

#define MES_SET_BCB_SER_INFO         0x100b      // ���ÿͻ�Beacon������������Ϣ
#define MES_SET_BCB_SER_INFO_ACK     0x100c      // �������ÿͻ�Beacon������������Ϣ���

#define MES_SET_UPL_INTERVAL         0x100d      // ����tcp�ϴ�mac��ַ�ļ��ʱ����ϴ�beacon�ļ��ʱ��
#define MES_SET_UPL_INTERVAL_ACK     0x100e      // ��������tcp�ϴ�mac��ַ�ļ��ʱ����ϴ�beacon�ļ��ʱ��

#define MES_VFY_KEY_SER              0x100f      // ��֤�ֻ�key 
#define MES_VFY_KEY_SER_ACK          0x1010      // ������֤�ֻ�key���

#define MES_APL_PHO_BIND_CBC         0x1011      // �ֻ��˻���cloudbeacon�豸
#define MES_APL_PHO_BIND_CBC_ACK     0x1012      // �����ֻ��˻���cloudbeacon�豸�Ľ��

#define MES_APL_PHO_UNBIND_CBC       0x1013      // �ֻ��˻����cloudbeacon�豸
#define MES_APL_PHO_UNBIND_CBC_ACK   0x1014      // �����ֻ��˻����cloudbeacon�豸�Ľ��

#define MES_SYN_CBC_CONF             0x1015      // ͬ���ֻ�����cloudbeacon����Ϣ
#define MES_SYN_CBC_CONF_ACK         0x1016      // ����ͬ�����

#define MES_UPL_BLU_CONF_FILE        0x1017      // �ϴ����������ļ�
#define MES_UPL_BLU_CONF_FILE_ACK    0x1018      // ȷ���ϴ����������ļ�

#define MES_UPD_BLU_CONF_FILE        0x1019      // �������������ļ�
#define MES_UPD_BLU_CONF_FILE_ACK    0x101a      // ȷ�ϸ������������ļ�





#define MES_PHO_GET_CBC_INFO         0x2000      // ��ȡ������Ϣ
#define MES_PHO_GET_CBC_INFO_ACK     0x2001      // ���ػ�ȡ������Ϣ���

#define MES_PHO_SET_CBC_INFO         0x2002     // ������Ϣ
#define MES_PHO_SET_CBC_INFO_ACK     0x2003     // ����������Ϣ���

#define MES_PHO_SND_KEY              0x2004    // �ֻ�����key
#define MES_PHO_SND_KEY_ACK          0x2005    // ����

#define MES_PHO_BIND_CBC             0x2006    // �ֻ����豸
#define MES_PHO_BIND_CBC_ACK         0x2007    // ���ذ��豸���

#define MES_PHO_UNBIND_CBC           0x2008    // ����豸
#define MES_PHO_UNBIND_CBC_ACK       0x2009    // ���豸

#define MES_SET_WIFI_CONF            0x200a    // ����wifi������Ϣ
#define MES_SET_WIFI_CONF_ACK        0x200b    // ��������wifi������Ϣ���
 






#endif /*__PROCOTOL_H__*/

