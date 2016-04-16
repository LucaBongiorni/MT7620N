#ifndef _STR_OPERATE_H_
#define _STR_OPERATE_H_

/******************************************************************************/
#include <vector>
#include <string>
#include <sys/types.h>
/******************************************************************************/

#define MAX_DATE_LEN	25		// ʱ���ַ���


// �ִ���������
class CStrOperate
{
public:
	// ȥ��β�ո�
	// ����:	s		Ҫ������ִ�
	// ���:			��
	// ����:	string	������ֵ
	// ˵��:			��
	static std::string Trim(const char s[]);
	
	// �ָ��ִ�
	// ����:	s	Ҫ�ָ���ִ�
	//			c	�ָ��ַ�
	// ���:	o	�ָ���ֵ
	// ����:	int	����
	// ˵��:		��
	static int Tokenize(std::vector<std::string> &o, const char s[], char c);

	// ��y/nת��bool
	// ����:	c		y/n
	// ���:			��
	// ����:	bool	ת��ֵ
	// ˵��:			��
	static bool ToBool(char c);
};

/******************************************************************************/


// ʱ���������
class CTimeOperate
{
public:
	enum E_FORMAT	// ʱ���ַ���ʽ
	{
		ALL,		// ������ʽ 1999-01-01 01:01:01
		DATE,		// ��-���� 1999-01-01
		NDATE,		// ����-���� 19990101
		TIME,		// ��:��ʱ�� 01:01:01
		NTIME,		// ����:��ʱ�� 010101
		NDATETIME	// ����-:����ʱ�� 19990101010101
	};

	// ��ʱ���ַ�ת��Ϊtime_t
	// ����:	sTime	ʱ�������ַ�
	// ���:			��
	// ����:			ת�����ֵ
	// ˵��:			��
	static time_t Str2Time(const char sTime[]);

	// ��time_tת��Ϊʱ���ַ�
	// ����:	t		ʱ��
	//			eFormat	��ʽ
	// ���:	sTime	�����
	// ����:			��
	// ˵��:			��
	static void Time2Str(time_t t, E_FORMAT eFormat, char sTime[]);
};



#endif
