#include <string.h>

#include "defCom.h"
#include "StrOperate.h"

/******************************************************************************/
std::string CStrOperate::Trim(const char s[])
{
	ASSERT(s != NULL);
	
	std::string sTmp(s);

	std::string r;
	r = sTmp.erase(sTmp.find_last_not_of(' ') + 1);
	r = r.erase(r.find_last_not_of('\t') + 1);
	r = r.erase(0, r.find_first_not_of(' '));
	r = r.erase(0, r.find_first_not_of('\t'));

	// ȥ������
	r = r.erase(r.find_last_not_of('\n') + 1);
	r = r.erase(r.find_last_not_of('\r') + 1);

	return r;
}

/******************************************************************************/
int CStrOperate::Tokenize(std::vector<std::string> &o, const char s[], char c)
{
	ASSERT(s != NULL);
	
	// ��������ʱ�ִ�
	std::string sTmp(s);
	sTmp = Trim(sTmp.c_str());

	// ��ʼ�ָ�
	int nPos;
	while ((nPos = (int)sTmp.find(c)) != -1)
	{
		// ��Ӳ�ɾ��
		o.push_back(Trim(sTmp.substr(0, (unsigned int)nPos).c_str()));
		(void)sTmp.erase(0, (unsigned int)(nPos + 1));
		sTmp = Trim(sTmp.c_str());
	}

	// ���ʣ�µ�
	if (sTmp.size() > 0)
	{
		o.push_back(Trim(sTmp.c_str()));
	}

	return (int)o.size();
}

/******************************************************************************/
bool CStrOperate::ToBool(char c)
{
	if (('y' == c) || ('Y' == c))
	{
		return true;
	}

	return false;
}




/******************************************************************************/
time_t CTimeOperate::Str2Time(const char sTime[])
{
	ASSERT(sTime != NULL);

	tm t = {0};

	// �ȴ�������
	if (NULL != strchr(sTime, '-'))
	{
		sscanf(sTime, "%d-%d-%d", &t.tm_year, &t.tm_mon, &t.tm_mday);
		t.tm_year -= 1900;
		t.tm_mon -= 1;
	}

	// ����ʱ��
	if (NULL != strchr(sTime, ':'))
	{
		const char *pSpace = strchr(sTime, ' ');
		if (pSpace != NULL)
		{
			sscanf(pSpace + 1, "%d:%d:%d", &t.tm_hour, &t.tm_min, &t.tm_sec);
		}
		else
		{
			sscanf(sTime, "%d:%d:%d", &t.tm_hour, &t.tm_min, &t.tm_sec);
		}
	}

	if (-1 == mktime(&t))
	{
		// �����޷ָ�������
		if (8 == strlen(sTime))
		{
			sscanf(sTime, "%4d%2d%2d", &t.tm_year, &t.tm_mon, &t.tm_mday);
			t.tm_year -= 1900;
			t.tm_mon -= 1;
		}
		// �����޷ָ���ʱ��
		else if (6 == strlen(sTime))
		{
			sscanf(sTime, "%2d%2d%2d", &t.tm_hour, &t.tm_min, &t.tm_sec);
		}
		// �����޷ָ������ں�ʱ�����
		else if (15 == strlen(sTime))
		{
			sscanf(sTime, "%4d%2d%2d %2d%2d%2d",
				&t.tm_year, &t.tm_mon, &t.tm_mday,
				&t.tm_hour, &t.tm_min, &t.tm_sec);
			t.tm_year -= 1900;
			t.tm_mon -= 1;
		}
	}

	return mktime(&t);
}

/******************************************************************************/
void CTimeOperate::Time2Str(time_t t, E_FORMAT eFormat, char sTime[])
{
	ASSERT(sTime != NULL);

	tm tTime;
	(void)localtime_r(&t, &tTime);

	switch (eFormat)
	{
		case ALL:
			(void)strftime(sTime, MAX_DATE_LEN, "%Y-%m-%d %H:%M:%S", &tTime);
			break;

		case DATE:
			(void)strftime(sTime, MAX_DATE_LEN, "%Y-%m-%d", &tTime);
			break;

		case NDATE:
			(void)strftime(sTime, MAX_DATE_LEN, "%Y%m%d", &tTime);
			break;

		case TIME:
			(void)strftime(sTime, MAX_DATE_LEN, "%H:%M:%S", &tTime);
			break;

		case NTIME:
			(void)strftime(sTime, MAX_DATE_LEN, "%H%M%S", &tTime);
			break;

		case NDATETIME:
			(void)strftime(sTime, MAX_DATE_LEN, "%Y%m%d%H%M%S", &tTime);
			break;
	}
}





