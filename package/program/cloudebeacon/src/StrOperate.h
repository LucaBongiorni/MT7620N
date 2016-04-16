#ifndef _STR_OPERATE_H_
#define _STR_OPERATE_H_

/******************************************************************************/
#include <vector>
#include <string>
#include <sys/types.h>
#include "defCom.h"
/******************************************************************************/
#define MAX_DATE_LEN    25      // ʱ���ַ���


// �ִ���������
class CStrOperate
{
public:
    // ȥ��β�ո�
    // ����:    s       Ҫ������ִ�
    // ���:            ��
    // ����:    string  ������ֵ
    // ˵��:            ��
    static std::string Trim(const char s[]);

    // �ָ��ִ�
    // ����:    s   Ҫ�ָ���ִ�
    //          c   �ָ��ַ�
    // ���:    o   �ָ���ֵ
    // ����:    int ����
    // ˵��:        ��
    static int Tokenize(std::vector<std::string> &o, const char s[], char c);
    // ��y/nת��bool
    // ����:    c       y/n
    // ���:            ��
    // ����:    bool    ת��ֵ
    // ˵��:            ��
    static bool ToBool(char c);
};
/******************************************************************************/

// ʱ���������
class CTimeOperate
{
public:
    enum E_FORMAT   // ʱ���ַ���ʽ
    {
        ALL,        // ������ʽ 1999-01-01 01:01:01
        DATE,       // ��-���� 1999-01-01
        NDATE,      // ����-���� 19990101
        TIME,       // ��:��ʱ�� 01:01:01
        NTIME,      // ����:��ʱ�� 010101
        NDATETIME   // ����-:����ʱ�� 19990101010101
    };
    // ��ʱ���ַ�ת��Ϊtime_t
    // ����:    sTime   ʱ�������ַ�
    // ���:            ��
    // ����:            ת�����ֵ
    // ˵��:            ��
    static time_t Str2Time(const char sTime[]);
    // ��time_tת��Ϊʱ���ַ�
    // ����:    t       ʱ��
    //          eFormat ��ʽ
    // ���:    sTime   �����
    // ����:            ��
    // ˵��:            ��
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
    // Functionality: �ȴ��¼�
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
