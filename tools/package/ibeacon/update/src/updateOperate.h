#ifndef __UPDATE_OPERATE_H__
#define __UPDATE_OPERATE_H__

#include <stdio.h>


class UPDATE
{
public:
	char*      m_confFilePath;		// �����ļ�
	char*      m_webDomain;		    // ����������
	u_int16    m_WebPort;
	int        m_version;
	char*      m_UpdateFilePath;
	char*      m_serials;
	char*      m_URL;
	int        m_HardwareType;

public:
	UPDATE();
	~UPDATE();

	// ��������: 
	// ��������: 
	//           
	// �� �� ֵ: 
	int 
	HexStringtoInt(u_int8 *str, int *datalen, char* md5);

	// ��������: ��ȡ�ű�ִ������ַ�
	// ��������: cmd��ִ�еĽű�����
	//           output������Ľ��
	// �� �� ֵ: �ɹ����ض�ȡ�����ַ�����ʧ�ܷ���-1
	int 
	GetShellCmdOutput(const char* cmd, char* Output, int OutputLen);

	// ��������: �������������Ͷ�ȡ�����ļ�
	// ��������: argc��argv��main ��������
	void 
	parseComLineAndConfFile(int argc, char **argv);

	// ��������: ƴ�ӷ�����Ϣ
	// ��������: httpHead���洢ƴ�Ӻ��post����
	//           headLen�����������ƴ�Ӻ��post���ݳ���
	// �� �� ֵ: �ɹ�����0��ʧ�ܷ���-1��
	int
	sprintSendInfo(char* httpHead, int *headLen);

	// ��������: ִ��ϵͳ����
	// ��������: kind������ģʽ��
	//           filePath������·��
	//           md5��md5У��
	//           ver���汾��
	// �� �� ֵ: �ɹ�����0��ʧ�ܷ���-1
	int 
	updateSystem(char kind, const char* filePathA, const char* filePathB,const char* md5, int ver);
};



static inline void 
usage(const char* argv)
{
    printf("Usage: %s [options]\n", argv);
    printf("\n");
    printf("  -c [filename] Use this config file.\n");
    printf("  -h            Print usage.\n");
    printf("  -v            Print version information.\n");
	printf("  -P            Web sever port.\n");
	printf("  -H            Web server domain.\n");
	//printf("  -s            Client Serials.\n");
    printf("\n");
}

#endif /*__UPDATE_OPERATE_H__*/
