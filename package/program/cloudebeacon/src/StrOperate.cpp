#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

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

	// 去除换行
	r = r.erase(r.find_last_not_of('\n') + 1);
	r = r.erase(r.find_last_not_of('\r') + 1);

	return r;
}

/******************************************************************************/
int 
CStrOperate::Tokenize(std::vector<std::string> &o, const char s[], char c)
{
	ASSERT(s != NULL);
	
	// 拷贝到临时字串
	std::string sTmp(s);
	sTmp = Trim(sTmp.c_str());

	// 开始分割
	int nPos;
	while ((nPos = (int)sTmp.find(c)) != -1)
	{
		// 添加并删除
		o.push_back(Trim(sTmp.substr(0, (unsigned int)nPos).c_str()));
		(void)sTmp.erase(0, (unsigned int)(nPos + 1));
		sTmp = Trim(sTmp.c_str());
	}

	// 添加剩下的
	if (sTmp.size() > 0)
	{
		o.push_back(Trim(sTmp.c_str()));
	}

	return (int)o.size();
}

/******************************************************************************/
bool 
CStrOperate::ToBool(char c)
{
	if (('y' == c) || ('Y' == c))
	{
		return true;
	}

	return false;
}




/******************************************************************************/
time_t 
CTimeOperate::Str2Time(const char sTime[])
{
	ASSERT(sTime != NULL);

	tm t = {0};

	// 先处理日期
	if (strchr(sTime, '-') != NULL)
	{
		sscanf(sTime, "%d-%d-%d", &t.tm_year, &t.tm_mon, &t.tm_mday);
		t.tm_year -= 1900;
		t.tm_mon -= 1;
	}

	// 处理时间
	if (strchr(sTime, ':') != NULL)
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
		// 处理无分隔的日期
		if (8 == strlen(sTime))
		{
			sscanf(sTime, "%4d%2d%2d", &t.tm_year, &t.tm_mon, &t.tm_mday);
			t.tm_year -= 1900;
			t.tm_mon -= 1;
		}
		// 处理无分隔的时间
		else if (6 == strlen(sTime))
		{
			sscanf(sTime, "%2d%2d%2d", &t.tm_hour, &t.tm_min, &t.tm_sec);
		}
		// 处理无分隔的日期和时间组合
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
void 
CTimeOperate::Time2Str(time_t t, E_FORMAT eFormat, char sTime[])
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






bool    CTimer::m_bUseMicroSecond = false;
u_int64 CTimer::s_ullCPUFrequency = CTimer::readCPUFrequency();


CTimer::CTimer():
m_ullSchedTime(), m_TickCond(), m_TickLock()
{
	pthread_mutex_init(&m_TickLock, NULL);
	pthread_cond_init(&m_TickCond, NULL);

	pthread_mutex_init(&m_EventLock, NULL);
	pthread_cond_init(&m_EventCond, NULL);
	//printf("2222222222222222\n");
}

CTimer::~CTimer()
{
	pthread_mutex_destroy(&m_TickLock);
	pthread_cond_destroy(&m_TickCond);
	pthread_mutex_destroy(&m_EventLock);
	pthread_cond_destroy(&m_EventCond);
}


void 
CTimer::rdtsc(u_int64 &x)
{
	if (m_bUseMicroSecond)
	{
		x = getTime();
		return;
	}

	x = getTime();
}

u_int64 
CTimer::readCPUFrequency()
{
   u_int64 frequency = 1;  // 1 tick per microsecond.

   // Fall back to microsecond if the resolution is not high enough.
   if (frequency < 10)
   {
      frequency = 1;
      m_bUseMicroSecond = true;
   }
   return frequency;
}

u_int64 
CTimer::getCPUFrequency()
{
   return s_ullCPUFrequency;
}

void 
CTimer::sleep(u_int64 interval)
{
   u_int64 t;
   rdtsc(t);

   // sleep next "interval" time
   sleepto(t + interval);
}

void 
CTimer::sleepto(u_int64 nexttime)
{
	// Use class member such that the method can be interrupted by others
	m_ullSchedTime = nexttime;

	u_int64 t;
	rdtsc(t);
	int temp;
	while (t < m_ullSchedTime)
	{
		struct timeval now;
		struct timespec timeout;
		gettimeofday(&now, 0);
		if (now.tv_usec < 990000)
		{
			timeout.tv_sec  = now.tv_sec;
			timeout.tv_nsec = (now.tv_usec + 10000) * 1000;
		}
		else
		{
			timeout.tv_sec  = now.tv_sec + 1;
			timeout.tv_nsec = (now.tv_usec + 10000 - 1000000) * 1000;
		}
		pthread_mutex_lock(&m_TickLock);
		temp = pthread_cond_timedwait(&m_TickCond, &m_TickLock, &timeout);
		pthread_mutex_unlock(&m_TickLock);
		printf("temp=%d\n", temp);
		rdtsc(t);
	}
	return ;
}

void 
CTimer::interrupt()
{
   // schedule the sleepto time to the current CCs, so that it will stop
   rdtsc(m_ullSchedTime);
   tick();
}

void 
CTimer::tick()
{
	pthread_cond_signal(&m_TickCond);
}

u_int64 
CTimer::getTime()
{
	struct timeval t;
	gettimeofday(&t, 0);
	return t.tv_sec * 1000000ULL + t.tv_usec;
}

void
CTimer::triggerEvent()
{
	pthread_cond_signal(&m_EventCond);
}

int 
CTimer::waitForEvent(int mTime)
{
	struct timespec timeout;
	struct timeval  now;
	gettimeofday(&now, 0);
	
	if (now.tv_usec < 990000)
	{
		timeout.tv_sec  = now.tv_sec + mTime / 1000;
		timeout.tv_nsec = (now.tv_usec + 10000) * 1000;
	}
	else
	{
		timeout.tv_sec  = now.tv_sec + 1 + mTime / 1000;
		timeout.tv_nsec = (now.tv_usec + 10000 - 1000000) * 1000;
	}
	int temp;
	pthread_mutex_lock(&m_EventLock);
	// 唤醒返回0，非唤醒返回一个数值
	temp = pthread_cond_timedwait(&m_EventCond, &m_EventLock, &timeout);
	pthread_mutex_unlock(&m_EventLock);
	return temp;
}

void CTimer::sleep()
{
	usleep(10);
	return ;
}

#if 0
CTimer *ti = new CTimer();
void *wait(void* arg)
{
	sleep(2);
	ti->triggerEvent();
	printf("----------------------------\n");
	return (void*)NULL;
}
// 测试定时器
int 
main()
{
	pthread_t id;

	pthread_create(&id, NULL, wait, NULL);
	//pthread_detach(id);
	ti->waitForEvent(1000);
	printf("1111111111111111111111\n");
	delete  ti;
	printf("######################\n");
	return 0;
}
#endif 



