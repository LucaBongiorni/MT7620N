#ifndef _STR_OPERATE_H_
#define _STR_OPERATE_H_

/******************************************************************************/
#include <vector>
#include <string>
#include <sys/types.h>
/******************************************************************************/

#define MAX_DATE_LEN	25		// 时间字符长


// 字串操作基类
class CStrOperate
{
public:
	// 去首尾空格
	// 输入:	s		要处理的字串
	// 输出:			无
	// 返回:	string	处理后的值
	// 说明:			无
	static std::string Trim(const char s[]);
	
	// 分割字串
	// 输入:	s	要分割的字串
	//			c	分割字符
	// 输出:	o	分割后的值
	// 返回:	int	个数
	// 说明:		无
	static int Tokenize(std::vector<std::string> &o, const char s[], char c);

	// 将y/n转成bool
	// 输入:	c		y/n
	// 输出:			无
	// 返回:	bool	转换值
	// 说明:			无
	static bool ToBool(char c);
};

/******************************************************************************/


// 时间操作基类
class CTimeOperate
{
public:
	enum E_FORMAT	// 时间字符格式
	{
		ALL,		// 完整格式 1999-01-01 01:01:01
		DATE,		// 带-的年 1999-01-01
		NDATE,		// 不带-的年 19990101
		TIME,		// 带:的时间 01:01:01
		NTIME,		// 不带:的时间 010101
		NDATETIME	// 不带-:的年时间 19990101010101
	};

	// 将时间字符转换为time_t
	// 输入:	sTime	时间描述字符
	// 输出:			无
	// 返回:			转换后的值
	// 说明:			无
	static time_t Str2Time(const char sTime[]);

	// 将time_t转换为时间字符
	// 输入:	t		时间
	//			eFormat	格式
	// 输出:	sTime	输出串
	// 返回:			无
	// 说明:			无
	static void Time2Str(time_t t, E_FORMAT eFormat, char sTime[]);
};



#endif
