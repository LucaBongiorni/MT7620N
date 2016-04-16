#ifndef __SERIALS_PCT__H__
#define __SERIALS_PCT__H__

#include <pthread.h>

#if 0
维护一个buff结构体，当有数据来时，无需检查数据正确与否，都将数据加到buff里面，
在取数据的时候，会根据协议头尾标志进行取数据。
这种方式性能开销大，原因主要是协议设计不完善；
该数据结构在多核情况下必须将互斥锁修改为自旋锁；
#endif

/*
typedef struct _SerialsPctBuff
{
	char* buff;
	unsigned short size;
	unsigned short start;
	unsigned short end;

	struct _SerialsPctBuff* (*createSerBuff)(int size);
	void (*destroySerBuff)(struct _SerialsPctBuff* buff);
	int (*addBuffToSerBuff)(const char* buff, int buffLen);
	int (*getFrmFromSerBuff)(char* outBuff, int* outLen);
}SerBuff;
*/

const int DEF_SERBUFF_LEN   = 4096;
static const char HEAD_FLAG = 0x02;
const int HEAD_FLAG_LEN = sizeof(HEAD_FLAG);
static const char TAIL_FLAG = 0x03;
const int TAIL_FLAG_LEN = sizeof(TAIL_FLAG);


class SerBuff
{
public:
	SerBuff(int size);
	SerBuff();
	~SerBuff();

public:
	int ADD_BuffToSerBuff(const char* buff, int buffLen);
	int GET_FrmFromSerBuff(char* outBuff, int *outLen);
	void printBuff();
	// 函数功能: 清空buff
	void clrBuff();


private:
	unsigned short m_start;
	unsigned short m_end;
	short m_bufflen;

	const unsigned short m_size;
	char* m_buff;

	pthread_mutex_t m_mutex;
	pthread_spinlock_t m_spinLock;

private:
	// 函数功能: 从pstart位置开始到pend位置，查找chr字符
	// 函数参数: pstart, pend, 字符指针
	//           chr，查找的字符
	// 返 回 值: 成功返回chr的位置，失败返回NULL
	static const char* 
	SerStrchr(const char* pstart, const char* pend, char chr);
	// 函数功能: 从pend位置开始往后倒pstart位置，查找chr字符
	// 函数参数: pstart, pend, 字符指针
	//           chr，查找的字符
	// 返 回 值: 成功返回chr的位置，失败返回NULL
	static const char* 
	OsiteStrchr(const char* pend, const char* pstart, char chr);
	// 函数功能: 判断buff里面有没有结尾标志
	//           
	bool judgeHasTailFlag();

	// 函数功能: 往缓冲区加数据
	int addBuffToSerBuff(const char* buff, int buffLen);
	// 函数功能: 从缓冲区取数据
	// 函数参数: outBuff，输出参数，读取到的数据
	//           outLen，输出参数，读取到的数据长度
	// 返 回 值: 成功返回读取到的长度，失败返回0
	int getFrmFromSerBuff(char* outBuff, int *outLen);
};




#endif /*__SERIALS_PCT__H__*/

