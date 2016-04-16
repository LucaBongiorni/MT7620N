#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "serialPct.h"

#define LOCK_SERBUFF       pthread_mutex_lock(&m_mutex)
#define UNLOCK_SERBUFF     pthread_mutex_unlock(&m_mutex)

#ifndef _DEBUG
#define write_log(fmt,args...)
#else
#define write_log(fmt,args...) printf("[%s][%d]"fmt"\n",__FILE__,__LINE__,##args)
#endif

SerBuff::SerBuff():
	m_start(0), m_end(0), m_bufflen(0), m_size(DEF_SERBUFF_LEN)
{
	m_buff = (char*)malloc(DEF_SERBUFF_LEN);
	memset(m_buff, 0, DEF_SERBUFF_LEN);
	pthread_mutex_init(&m_mutex, NULL);
	pthread_spin_init(&m_spinLock, 0);
}

SerBuff::SerBuff(int size):
	m_start(0), m_end(0), m_bufflen(0), m_size(size)
{
	m_buff = (char*)malloc(size);
	memset(m_buff, 0, size);
	pthread_mutex_init(&m_mutex, NULL);
	pthread_spin_init(&m_spinLock, 0);
}

SerBuff::~SerBuff()
{
	if (m_buff)
	{
		free(m_buff), m_buff = NULL;
	}
	m_bufflen = m_start = m_end = 0;
	pthread_mutex_destroy(&m_mutex);
	pthread_spin_destroy(&m_spinLock);
}


int 
SerBuff::addBuffToSerBuff(const char* buff, int buffLen)
{
	int temp = 0;
	if (m_start <= m_end)
	{
		if (m_bufflen == m_size)
		{
			if (false == judgeHasTailFlag()) 
			{
				clrBuff();
			}
			return -1; // 已经满了
		}
		
		temp = m_size - m_end;
		if (temp + m_start >= buffLen)
		{
			if (temp >= buffLen)
			{
				memcpy(&m_buff[m_end], buff, buffLen);
				m_end += buffLen;
				if (m_end >= m_size) m_end = 0;
			}
			else
			{
				memcpy(&m_buff[m_end], buff, temp);
				memcpy(m_buff, buff+temp, buffLen-temp);
				m_end = buffLen - temp;
			}
		}
		else
		{
			if (false == judgeHasTailFlag()) 
			{ 
				clrBuff();
			}
			return -1; // 添加不进去，buff太长
		}
	}
	else 
	{
		temp = m_start - m_end;
		if (temp >= buffLen)
		{
			memcpy(&m_buff[m_end], buff, buffLen);
			m_end += buffLen;
		}
		else
		{
			if (false == judgeHasTailFlag()) 
			{
				clrBuff();
			}
			return -1; // 添加不进去，buff太长
		}
	}
	
	m_bufflen += buffLen;
	return buffLen;
}



int 
SerBuff::getFrmFromSerBuff(char* outBuff, int *outLen)
{
	const char* pStart = NULL;
	const char* const pEnd = m_buff + m_end;
	const char* pTail = NULL;
	const char* pHead = NULL;
	const char *const pSize = m_buff + m_size;
	int temp = 0;
	//int startVal = 0;

#if 0
	printf("m_start=%d, m_end=%d, m_bufflen=%d\n", 
		m_start, m_end, m_bufflen);
	if (m_start >= m_size || m_end >= m_size) exit(1);

	if (m_bufflen > m_size || m_bufflen < 0)
	{
		printf("m_bufflen=%d\n", m_bufflen);
		exit(1);
	}
#endif

	if (m_end > m_start)
	{
		pStart = &m_buff[m_start];
		pTail = SerStrchr(pStart, pEnd, TAIL_FLAG);
		if (pTail)
		{
			pHead = OsiteStrchr(pTail, pStart, HEAD_FLAG);
			if (pHead)
			{
				// 提取 pHead 到 pTail 这一段
				*outLen = temp = pTail-pHead+1;
				memcpy(outBuff, pHead, temp);
				m_bufflen = m_bufflen - (pTail-pStart+1);
				m_start = pTail - m_buff + 1;
				return temp;
			}
			else
			{
				// 找到尾，找不到头，干掉这一段
				m_bufflen = m_bufflen - (pTail-pStart+1);
				m_start = pTail - m_buff + 1;
				getFrmFromSerBuff(outBuff, outLen);
				//return 0;
			}
		}
		else
		{
			// 找不到尾，没有完整包，直接返回
			return -1;  
		}
	}
	else 
	{
		if (m_bufflen == 0)
		{
			//printf("m_bufflen=%d\n", m_bufflen);
			return 0;
		}

		pStart = &m_buff[m_start];
		pTail = SerStrchr(pStart, pSize, TAIL_FLAG);
		if (pTail)
		{
			// pTail 在 pStart 到 m_buff+m_size 这段距离中
			pHead = OsiteStrchr(pTail, pStart, HEAD_FLAG);
			if (pHead)
			{
				//printf("222222pHead=%d\n", pHead-m_buff);
				*outLen = temp = pTail - pHead + 1;
				memcpy(outBuff, pHead, temp);

				m_start = pTail - m_buff + 1;
				m_bufflen = m_bufflen - (pTail-pStart+1);
				//printf("[%d]----bufflen=%d, m_start=%d, m_end=%d\n", __LINE__, m_bufflen, m_start, m_end);
				if (m_start == m_size) m_start = 0;
				return temp;
			}
			else
			{
				// 找到尾，没找到头，砍掉这一段，重新找尾，
				m_start = pTail - m_buff + 1;
				m_bufflen = m_bufflen - (pTail-pStart+1);
				//printf("[%d]----bufflen=%d, m_start=%d, m_end=%d\n", __LINE__, m_bufflen, m_start, m_end);
				if (m_start == m_size) m_start = 0;
				getFrmFromSerBuff(outBuff, outLen);
				//return 0;
			}
		}
		else
		{
			pTail = SerStrchr(m_buff, pEnd, TAIL_FLAG);
			if (pTail)
			{
				// pTail 在m_buff 到 m_pEnd 这段距离中
				pHead = OsiteStrchr(pTail, m_buff, HEAD_FLAG);
				if (pHead)
				{
					// 拷贝pHead 到 pTail 这段距离
					*outLen = temp = pTail - pHead + 1;
					memcpy(outBuff, pHead, temp);
					m_start = pTail - m_buff + 1;
					m_bufflen = m_bufflen - ((pSize-pStart) + (pTail-m_buff+1));
					//printf("[%d]----bufflen=%d, m_start=%d, m_end=%d\n", __LINE__, m_bufflen, m_start, m_end);
					return temp;
				}
				else
				{
					pHead = OsiteStrchr(pSize, pStart, HEAD_FLAG);
					if (pHead)
					{
						// pHead 在 pStart 到 mbuff+m_size这段距离中
						temp = pSize - pHead;
						memcpy(outBuff, pHead, temp);
						memcpy(outBuff+temp, m_buff, pTail-m_buff+1);
						temp = temp + (pTail-m_buff+1);
						
						m_start = pTail - m_buff + 1;
						m_bufflen = m_bufflen - ((pSize-pStart) + (pTail-m_buff+1));
						//printf("[%d]----bufflen=%d, m_start=%d, m_end=%d\n", __LINE__, m_bufflen, m_start, m_end);
						*outLen = temp;
						return temp;
					}
					else
					{
						// 找到尾，没找到头，砍掉这一段，退出
						m_start = pTail - m_buff + 1;
						m_bufflen = m_bufflen - ((pSize-pStart) + (pTail-m_buff+1));
						getFrmFromSerBuff(outBuff, outLen);
						//return 0;
					}
				}
			}
			else
			{
				// 找不到尾，没有完整包，直接返回
				//printf("[%s:%d]can't find tail flag.\n", __FILE__, __LINE__);
				return -1;
			}
		}
	}
	return -1;
}

bool 
SerBuff::judgeHasTailFlag()
{
	int i;
	if (m_start < m_end)
	{
		for (i=m_start; i<m_end; ++i)
			if (m_buff[i] == TAIL_FLAG) 
				return true;
	}
	else
	{
		if (m_bufflen == 0)
		{
			return false;
		}
		else
		{
			for (i=m_start; i<m_size; ++i)
				if (m_buff[i] == TAIL_FLAG) return true;
			for (i=0; i<m_end; ++i)
				if (m_buff[i] == TAIL_FLAG) return true;
		}
	}
	return false;
}

void 
SerBuff::clrBuff()
{
	m_bufflen = 0;
	m_start = 0;
	m_end = 0;
	memset(m_buff, 0, m_size);
}



const char* 
SerBuff::SerStrchr(const char* pstart, const char* pend, char chr)
{
	if (NULL == pstart || NULL == pend) return NULL;

	const char* ptemp = pstart;
	for (; ptemp < pend; ++ptemp)
	{
		if (*ptemp == chr) 
		{
			return ptemp;
		}
	}
	return NULL;
}


const char* 
SerBuff::OsiteStrchr(const char* pend, const char* pstart, char chr)
{
	if (NULL == pstart || NULL == pend) return NULL;

	const char* ptemp = pend-1;
	for (; ptemp >= pstart; --ptemp)
	{
		if (*ptemp == chr) 
		{
			return ptemp;
		}
	}
	return NULL;
}

void 
SerBuff::printBuff()
{
	int i;
	LOCK_SERBUFF;
	if (m_start < m_end)
	{
		for (i=m_start; i<m_end; ++i)
		{
			printf("%02x ", m_buff[i]);
			if (m_buff[i] == TAIL_FLAG) printf("\n");
		}
		printf("\n");
	}
	else
	{
		if (m_start == m_end && m_bufflen == m_size)
		{
			for (i=m_start; i<m_size; ++i)
			{
				printf("%02x ", m_buff[i]);
				if (m_buff[i] == TAIL_FLAG) printf("\n");
			}
			for (i=0; i<m_end; ++i)
			{
				printf("%02x ", m_buff[i]);
				if (m_buff[i] == TAIL_FLAG) printf("\n");
			}
			printf("\n");
		}
	}
	printf("m_start=%d, m_end=%d, m_bufflen=%d\n\n", m_start, m_end, m_bufflen);
	UNLOCK_SERBUFF;
}


void printBuff(char* buff, int outLen)
{
	printf("\n\n\n#################\n");
	printf("Get buff: outLen=%d\n", outLen);
	for (int i=0; i<outLen; ++i)
		printf("%02x ", buff[i]);
	printf("\n");
	printf("#################\n");
}

int 
SerBuff::ADD_BuffToSerBuff(const char* buff, int buffLen)
{
	if (NULL == buff || buffLen > m_size)
		return 0;
	
	int temp;
	LOCK_SERBUFF;
	temp = addBuffToSerBuff(buff, buffLen);
	UNLOCK_SERBUFF;
	return temp;
}

int 
SerBuff::GET_FrmFromSerBuff(char* outBuff, int *outLen)
{
	if (NULL == outBuff || NULL == outLen) 
	{
		printf("outbuff or outlen is null.\n");
		return 0;
	}
	
	int temp;
	LOCK_SERBUFF;
	temp = getFrmFromSerBuff(outBuff, outLen);
	UNLOCK_SERBUFF;
	return temp;
}




#if 0

int main()
{
	char buff[] = "1122334455667788";
	char buff1[] = "aabbccddeefgh";
	char buff2[] = "AABCDEFGHIJKLMNOPQR";
	int buff2Len = strlen(buff2);
	int buff1Len = strlen(buff1);
	int buffLen = strlen(buff);
	buff1[0] = buff2[0] = buff[0] = HEAD_FLAG;
	buff[buffLen-1] = TAIL_FLAG;
	int temp = 0;
	printf("buffLen=%d, buff1Len=%d, buff2Len=%d\n", buffLen, buff1Len, buff2Len);
	
	SerBuff * serbuff = new SerBuff(64);
	for (int i=0; i<1; ++i)
	{
		temp = serbuff->addBuffToSerBuff(buff1, buff1Len);
		if (temp < 0)
			printf("temp =%d, i=%d\n", temp, i);
		
		serbuff->printBuff();
		temp = serbuff->addBuffToSerBuff(buff, buffLen);
		if (temp < 0)
			printf("temp =%d, i=%d\n", temp, i);
		
		serbuff->printBuff();
		temp = serbuff->addBuffToSerBuff(buff2, buff2Len);
		if (temp < 0)
			printf("temp =%d, i=%d\n", temp, i);
		
		serbuff->printBuff();
	}

	temp = serbuff->addBuffToSerBuff(buff, buffLen);
	serbuff->printBuff();

	char outBuff[64] = {0};
	int outLen;
	temp = serbuff->getFrmFromSerBuff(outBuff, &outLen);
	printBuff(outBuff, outLen);
	printf("temp=%d\n", temp);

	temp = serbuff->addBuffToSerBuff(buff1, buff1Len);
	serbuff->printBuff();

	temp = serbuff->getFrmFromSerBuff(outBuff, &outLen);
	printBuff(outBuff, outLen);
	temp = serbuff->getFrmFromSerBuff(outBuff, &outLen);
	printBuff(outBuff, outLen);
	serbuff->printBuff();
	return 0;
	
	serbuff->getFrmFromSerBuff(outBuff, &outLen);
	printBuff(outBuff, outLen);

	for (int i=0; i<3; ++i)
	{
		temp = serbuff->addBuffToSerBuff(buff1, 8);
		if (temp < 0)
		{
			printf("temp =%d, i=%d\n", temp, i);
			break;
		}
		temp = serbuff->addBuffToSerBuff(buff2, buff2Len);
		if (temp < 0)
		{
			printf("temp =%d, i=%d\n", temp, i);
			break;
		}
		serbuff->printBuff();
	}
	
	printf("\n\n\n");
	temp = serbuff->getFrmFromSerBuff(outBuff, &outLen);
	if (temp > 0)
		printBuff(outBuff, outLen);
	temp = serbuff->getFrmFromSerBuff(outBuff, &outLen);
	if (temp > 0)
		printBuff(outBuff, outLen);
	temp = serbuff->getFrmFromSerBuff(outBuff, &outLen);
	if (temp > 0)
		printBuff(outBuff, outLen);
	
	delete serbuff, serbuff = NULL;
	return 0;
}

#endif





#if 0
SerBuff serbuff(32);
#define TEST_CNT  1000000
volatile int sum = 0;


pthread_rwlock_t mutex = PTHREAD_RWLOCK_INITIALIZER;
char buff1[] = "112345678900";
const int buff1Len = (sizeof(buff1)-1);


void *addbuff1(void* arg)
{	
	buff1[0] = HEAD_FLAG; 
	buff1[buff1Len-1] = TAIL_FLAG;
	
	int Val;
	int cnt = 0;
	while(1)
	{
		Val = serbuff.ADD_BuffToSerBuff(buff1, buff1Len);
		if (Val > 0)
		{
			++cnt;
			if (cnt == TEST_CNT)
			{
				pthread_rwlock_wrlock(&mutex);
				printf("add count=%d\n", cnt);
				serbuff.printBuff();
				sum = 1;
				pthread_rwlock_unlock(&mutex);

				printf("---addbuff1 pthread exit-----\n");
				return (void*)NULL;
			}
			//if (cnt % 10 == 0) 
				printf("add cnt=%d\n", cnt);
		}
		//else
		//	printf("addbuff1: temp=%d\n", Val);
		usleep(4);
	}
}

void *addbuff2(void* arg)
{
	char buff2[] = "AABCDEFGHIJKLMNOPQ";
	const int buff2Len = (sizeof(buff2)-1);
	buff2[0] = HEAD_FLAG; 
	
	int temp;
	while(1)
	{
		pthread_rwlock_rdlock(&mutex);
		if (sum == 1)
		{
			pthread_rwlock_unlock(&mutex);
			printf("---addbuff2 pthread exit-----\n");
			return (void*)NULL;
		}
		pthread_rwlock_unlock(&mutex);
		
		temp = serbuff.ADD_BuffToSerBuff(buff2, buff2Len);
		if (temp > 0)
			//printAddBuff(buff2, buff2Len);
		usleep(3);
	}
}

void *addbuff3(void* arg)
{
	char buff3[] = "aabbccddeefffsfdsfs";
	const int buff3Len = (sizeof(buff3)-1);
	//buff3[0] = HEAD_FLAG; 
	
	int temp;
	int addLen = 1;
	while(1)
	{
		pthread_rwlock_rdlock(&mutex);
		if (sum == 1)
		{
			pthread_rwlock_unlock(&mutex);
			printf("---addbuff3 pthread exit-----\n");
			return (void*)NULL;
		}
		pthread_rwlock_unlock(&mutex);

		temp = serbuff.ADD_BuffToSerBuff(buff3, addLen);
		if (temp > 0)
			//printAddBuff(buff3, buff3Len);

		addLen++;
		if (addLen == buff3Len) addLen = 1;
		usleep(2);
	}
}


void printGetBuff(char* buff, int outLen)
{
	printf("\n\n\n@@@@@@@@@@@@@@@@@@@@@\n");
	printf("Get buff: outLen=%d\n", outLen);
	for (int i=0; i<outLen; ++i)
		printf("%02x ", buff[i]);
	printf("\n");
	printf("@@@@@@@@@@@@@@@@@@@@@\n");
}

void *getbuff4(void* arg)
{
	char outBuff[64] = {0};
	int outLen = 0, tmp;
	
	int count1 = 0;
	int getCnt = 0;
	int erroCnt = 0;
	while (1)
	{		
		outLen = 0;
		memset(outBuff, 0, 64);
		tmp = serbuff.GET_FrmFromSerBuff(outBuff, &outLen);
		getCnt++;
		if (tmp > 0)
		{
			if (0 == memcmp(outBuff, buff1, buff1Len))
			{
				//printBuff(outBuff, outLen);
				++count1;
				if (count1 == TEST_CNT)
				{
					serbuff.printBuff();
					printf("---getbuff4 pthread exit-----\n");
					printf("count=%d, tmp=%d, getCnt=%d, erroCnt=%d\n", count1, tmp, getCnt, erroCnt);
					return (void*)NULL;
				}
			}
			else
			{
				//printGetBuff(outBuff, outLen);
				//serbuff.printBuff();
				++erroCnt;
			}
		}	
		printf("[%d]------temp=%d, count1=%d\n", __LINE__, tmp, count1);
		usleep(5);
	}
}



int main()
{
	// 开两个线程，一个线程不断加正确的协议字符，一个线程加错误字符，一个线程不断读，
	pthread_t addId1, addId2, addId3, getId4;
	
	pthread_create(&addId1, 0, addbuff1, NULL);
	pthread_detach(addId1);
	pthread_create(&getId4, 0, getbuff4, NULL);
	pthread_detach(getId4);
	pthread_create(&addId2, 0, addbuff2, NULL);
	pthread_detach(addId2);
	pthread_create(&addId3, 0, addbuff3, NULL);
	pthread_detach(addId3);
	
	while(1)sleep(10);
	return 0;
}
#endif 


#if 0
const char* 
SerStrchr(const char* pstart, const char* pend, char chr)
{
	if (NULL == pstart || NULL == pend) return NULL;
	printf("*pstart=%c, *pend=%c\n", *pstart, *pend);

	const char* ptemp = pstart;
	while(ptemp < pend)
	{
		if (*ptemp == chr) return ptemp;
		++ptemp;
	}
	return NULL;
}


const char* 
OsiteStrchr(const char* pend, const char* pstart, char chr)
{
	if (NULL == pstart || NULL == pend) return NULL;

	const char* ptemp = pend-1;
	while (ptemp >= pstart)
	{
		if (*ptemp == chr) return ptemp;
		--ptemp;
	}
	return NULL;
}

int main()
{
	char *buff = (char*)malloc(6);
	memcpy(buff, "abcdefg", 7);

	const char* temp = SerStrchr(buff, buff+6, 'g');
	if (temp)
		printf("%c\n", *temp);
	
	return 0;
}

#endif
