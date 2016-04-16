#ifndef _STR_OPERATE_H_
#define _STR_OPERATE_H_

/******************************************************************************/
#include <vector>
#include <string>
#include <sys/types.h>
#include "defCom.h"
/******************************************************************************/
#define MAX_DATE_LEN    25      // 时间字符长


// 字串操作基类
class CStrOperate
{
public:
    // 去首尾空格
    // 输入:    s       要处理的字串
    // 输出:            无
    // 返回:    string  处理后的值
    // 说明:            无
    static std::string Trim(const char s[]);

    // 分割字串
    // 输入:    s   要分割的字串
    //          c   分割字符
    // 输出:    o   分割后的值
    // 返回:    int 个数
    // 说明:        无
    static int Tokenize(std::vector<std::string> &o, const char s[], char c);
    // 将y/n转成bool
    // 输入:    c       y/n
    // 输出:            无
    // 返回:    bool    转换值
    // 说明:            无
    static bool ToBool(char c);
};
/******************************************************************************/

// 时间操作基类
class CTimeOperate
{
public:
    enum E_FORMAT   // 时间字符格式
    {
        ALL,        // 完整格式 1999-01-01 01:01:01
        DATE,       // 带-的年 1999-01-01
        NDATE,      // 不带-的年 19990101
        TIME,       // 带:的时间 01:01:01
        NTIME,      // 不带:的时间 010101
        NDATETIME   // 不带-:的年时间 19990101010101
    };
    // 将时间字符转换为time_t
    // 输入:    sTime   时间描述字符
    // 输出:            无
    // 返回:            转换后的值
    // 说明:            无
    static time_t Str2Time(const char sTime[]);
    // 将time_t转换为时间字符
    // 输入:    t       时间
    //          eFormat 格式
    // 输出:    sTime   输出串
    // 返回:            无
    // 说明:            无
    static void Time2Str(time_t t, E_FORMAT eFormat, char sTime[]);
    

};


class CTimer
{
public:
    CTimer();
    ~CTimer();
public:
    // Functionality:
    //    Sleep for "interval" CCs.
    // Parameters:
    //    0) [in] interval: CCs to sleep.
    // Returned value:
    //    None.
    void sleep(u_int64 interval);
    // Functionality:
    //    Seelp until CC "nexttime".
    // Parameters:
    //    0) [in] nexttime: next time the caller is waken up.
    // Returned value:
    //    None.
    void sleepto(u_int64 nexttime);
    // Functionality:
    //    Stop the sleep() or sleepto() methods.
    // Parameters:
    //    None.
    // Returned value:
    //    None.
    void interrupt();
    // Functionality:
    //    trigger the clock for a tick, for better granuality in no_busy_waiting timer.
    // Parameters:
    //    None.
    // Returned value:
    //    None.
    void tick();
public:
    // Functionality:
    //    Read the CPU clock cycle into x.
    // Parameters:
    //    0) [out] x: to record cpu clock cycles.
    // Returned value:
    //    None.
    static void rdtsc(u_int64 &x);
    // Functionality:
    //    return the CPU frequency.
    // Parameters:
    //    None.
    // Returned value:
    //    CPU frequency.
    static u_int64 getCPUFrequency();
    // Functionality:
    //    check the current time, 64bit, in microseconds.
    // Parameters:
    //    None.
    // Returned value:
    //    current time in microseconds.
    static u_int64 getTime();
    // Functionality:
    //    trigger an event such as new connection, close, new data, etc. for "select" call.
    // Parameters:
    //    None.
    // Returned value:
    //    None.
    void triggerEvent();
    // Functionality: 等待事件
    // Parameters:
    //    None.
    // Returned value:
    //    None.
    int waitForEvent(int mTime);
    // Functionality:
    //    sleep for a short interval. exact sleep time does not matter
    // Parameters:
    //    None.
    // Returned value:
    //    None.
    static void sleep();
	
private:
    u_int64 getTimeInMicroSec();
	
private:
    u_int64 m_ullSchedTime;             // next schedulled time
    pthread_cond_t m_TickCond;
    pthread_mutex_t m_TickLock;
    pthread_cond_t m_EventCond;
    pthread_mutex_t m_EventLock;
	
private:
    static u_int64 s_ullCPUFrequency;    // CPU frequency : clock cycles per microsecond
    static u_int64 readCPUFrequency();
    static bool m_bUseMicroSecond;       // No higher resolution timer available, use gettimeofday().
};


#endif
