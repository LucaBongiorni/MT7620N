#ifndef __UCI_H__
#define __UCI_H__
#include <stdio.h>
#include <uci.h>


#define UCI_SAVE_DIR    "/tmp"

class UCI
{
public:
	UCI();
	UCI(const char* dir);
	~UCI();

public:
	// ��������: ����ɨ��·��
	void 
	UCI_SetConfDir(const char* dir);
	// uci commit
	void 
	UCI_Commit();


	// ��������: uci ���������ļ�
	// ��������: File���ļ�·��
	// �� �� ֵ: ����0�ɹ�
	int 
	UCI_LoadFile(const char* File);
	// ��������: uci ж�������ļ�
	int 
	UCI_UnloadFile();


	// ��������: ��ȡ�������ֵ����option������δ֪��������optionҲ������list
	// ��������: section: ��������
	//           option:  ��Ŀ����
	//           isList������������ж��Ƿ�Ϊlist����
	// �� �� ֵ: isList == true�����������ַ��
	//           isList == false������option��ֵ��ʧ�ܷ���NULL
	char* 
	UCI_GetOptionValue(const char* section, const char* option, bool &isList);
	// ��������: ��ȡ�������ֵ��option����
	// ��������: section: ��������
	//           option:  ��Ŀ����
	// �� �� ֵ: ����option��ֵ��ʧ�ܷ���NULL
	const char*
	UCI_GetOptionValue(const char* section, const char* option);
	// ��������: ��ȡ�������ֵ��list����
	// ��������: section: ��������
	//           option:  ��Ŀ����
	// �� �� ֵ: ���������ַ��ʧ�ܷ���NULL
	const struct uci_list*
	UCI_GetListValue(const char* section, const char* option);
	

	// ��������: �޸�option����ֵ
	// ��������: section: ��������
	//           option:  ��Ŀ����
	//           value:   ��Ŀֵ
	// �� �� ֵ: �ɹ�0��ʧ�� -1��
	int
	UCI_SetOptionValue(const char* section, const char* option, const char* value);
	// ��������: �޸�section����ֵ
	// ��������: section: ��������
	//           value:   ��Ŀֵ
	// �� �� ֵ: �ɹ�0��ʧ�� -1��
	int 
	UCI_SetSectionValue(const char* section, const char* newSec);
	


	// ��������: �����������ã���֧������
	// ��������: section: ����
	//           name:  ��������
	// �� �� ֵ: �ɹ�0����0ʧ��
	int 
	UCI_AddSection(const char* section, const char* name);
	// ��������: ����������Ŀ
	// ��������: section: ����
	//           option:  ��Ŀ����
	//           name: ��Ŀֵ
	// �� �� ֵ: �ɹ�0����0ʧ��
	int 
	UCI_AddOption(const char* section, const char* option, const char* value);


	// ��������: ɾ����������
	// ��������: section: ����
	//           name:  ��������
	// �� �� ֵ: �ɹ�0����0ʧ��
	int 
	UCI_DelSection(const char* section);
	// ��������: ɾ��������Ŀ
	// ��������: section: ����
	//           option:  ��Ŀ����
	// �� �� ֵ: �ɹ�0����0ʧ��
	int 
	UCI_DelOption(const char* section, const char* option);


	// ��������: �޸�list����ֵ
	// ��������: section: ��������
	//           list:    ��Ŀ����
	//           value:   ��Ŀֵ
	//           newVal:  �µ���Ŀֵ
	// �� �� ֵ: �ɹ�0��ʧ�� -1��
	int
	UCI_SetListValue(const char* section, const char* list, const char* value, const char* newVal);
	// ��������: ���list����ֵ
	// ��������: section: ��������
	//           list:    ��Ŀ����
	//           value:   ��Ŀֵ
	// �� �� ֵ: �ɹ�0��ʧ�� -1��
	int 
	UCI_AddList(const char* section, const char* list, const char* value);
	// ��������: ɾ��list����ֵ
	// ��������: section: ��������
	//           list:    ��Ŀ����
	//           value:   ��Ŀֵ
	// �� �� ֵ: �ɹ�0��ʧ�� -1��
	int 
	UCI_DelList(const char* section, const char* list, const char* value);
	
private:
	struct uci_context *m_ctx;
	struct uci_ptr     *m_ptr;   
	struct uci_package *m_pkg;
	char* m_File;

private:
	char* 
	uci_get_value(struct uci_option *o);
};


void UCI_PrintList(struct uci_list* List);

#endif /*__UCI_H__*/


