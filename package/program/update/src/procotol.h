#ifndef __PROCOTOL_H__
#define __PROCOTOL_H__

typedef unsigned char    u_int8;
typedef unsigned short   u_int16;
typedef unsigned int     u_int32;


// ��ͷ�Ͱ�β��־
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
	// ��������: unsigned intʮ������ת�ַ�
	// ��������: hex, ���������unsigned int ���͵�����
	//           str, ���������ת�����ַ���8���ֽ�
	static void Uint32HexTostr(u_int32 hex, char* str);

	// ��������: �ַ�תunsigned intʮ�����ƣ�ת8���ֽ�
	// ��������: hex, ���������unsigned int ���͵�����
	//           str, ���������16���������ַ�
	static int StrToUint32Hex(u_int32* hex, char* str);
	
	// ��������: unsigned shortʮ������ת�ַ�
	// ��������: hex unsigned short ���͵�����
	//           str ���������ת�����ַ���4���ֽ�
	static void Uint16HexToStr(u_int16 hex, char *str);
	
	// ��������: �ַ�תunsigned shortʮ�����ƣ�ת�ĸ��ֽ�
	// ��������: hex ���������ת���ɵ�����
	//           str ������ַ�
	static int StrToUint16Hex(u_int16* hex, char* str);
	
	// ��������: unsigned charʮ������ת�ַ���ת1���ֽ�
	// ��������: hex unsigned char ���͵�����
	//           str ���������ת�����ַ���2���ֽ�           
	static void Uint8HexToStr(u_int8 hex, char *str);
	
	// ��������: �ַ�תʮ�����ƣ�ת2���ֽ�
	// ��������: hex ���������ת���ɵ�����
	//           str ������ַ�
	static int StrToUint8Hex(u_int8* hex, char* str);

	
	// ��������: �Ӱ�ͷ�Ͱ�β
	// ��������: content ������ַ����Ӱ�ͷ����ַ�Ҳͨ���ò������
	//           contentLen ������ַ�����
	//           typeID ��������
	//           PkgTotal ���͵��ܰ���
	//           PkgSeq ���͵ڼ�����
	// �� �� ֵ: �ɹ����ؼӰ�ͷ����ַ����ȣ�ʧ�ܷ���-1
	static int AddPkgHeadAndTail(char* content, u_int16 contentLen, u_int16 typeID, char PkgTotal, char PkgSeq);

	// ��������: ȥ��ͷ�Ͱ�β
	// ��������: content ������ַ���ȥ��ͷ����ַ�Ҳͨ���ò������
	//           contentLen ������ַ�����
	//           typeID ������������ݰ�����
	//           PkgTotal ����������ܹ����ٸ���
	//           PkgSeq ����������ڼ�����
	// �� �� ֵ: �ɹ�����ȥ����ͷ�Ͱ�β���ַ����ȣ�ʧ�ܷ���-1
	static int DelPkgHeadAndTail(char* content, int contentLen, u_int16* typeID, u_int8* PkgTotal, u_int8* PkgSeq);

	// ��������: ��ʮ�����ƴ�ӡ�ַ�
	// ��������: buf �����ӡ���ַ�
	//           len ��ӡ�ĳ���
	static void printHex(const char* buf, int len);
};





#if 0
/*У���*/
unsigned char c = 0x00;
pBuf[12] = 0x00;
for(i=0; i<FrameLen; ++i)
{
    c += pBuf[i];
}
pBuf[12] = 0xAA - c;
#endif




#endif /*__PROCOTOL_H__*/

