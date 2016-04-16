#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>


#include "MemPool.h"
#include "cbProjMain.h"


#define write_error_log(fmt,args...)  printf("[%s][%d]Error:"fmt"\n",__FILE__,__LINE__,##args)


MemPool::MemPool()
{
	m_SliceAllocNum = 0;
	m_SliceSize   = DEF_SLICE_SIZE;
	m_SliceTolNum = DEF_SLICE_NUM;

	int size = (m_SliceSize+1) * m_SliceTolNum;
	m_pSlice = (char*)malloc(size);
	if (NULL == m_pSlice)
	{
		Debug(I_ERROR, "Malloc Memory Failed.");
		return ;
	}
	
	m_pStart = m_pSlice;
	m_pEnd   = m_pSlice + size;
	m_Allocflag = 0;

	for (int i=0; i<m_SliceTolNum; ++i)
	{
		m_pSlice[(m_SliceSize+1)*i] = 0;
	}
	pthread_mutex_init(&m_mutex, 0);
}


MemPool::MemPool(int sliceSize, int sliceTolNum)
{
	m_SliceAllocNum = 0;
	m_SliceSize = sliceSize;
	m_SliceTolNum = sliceTolNum;

	int size = (m_SliceSize+1) * m_SliceTolNum;
	m_pSlice = (char*)malloc(size);
	if (! m_pSlice)
	{
		Debug(I_ERROR, "Malloc Memory Failed.");
		return ;
	}
	
	m_pStart = m_pSlice;
	m_pEnd   = m_pSlice + size;
	m_Allocflag = 0;

	for (int i=0; i<m_SliceTolNum; ++i)
	{
		m_pSlice[(m_SliceSize+1)*i] = 0;
	}
	pthread_mutex_init(&m_mutex, 0);
}


MemPool::~MemPool()
{	
	if (m_pSlice)
		free(m_pSlice), m_pSlice = NULL;
	m_pStart = NULL;
	m_pEnd   = NULL;
	pthread_mutex_destroy(&m_mutex);
}

void*
MemPool::Alloc(int size)
{
	void* temp = NULL;
	pthread_mutex_lock(&m_mutex);
	temp = MemPoolAlloc(size);
	pthread_mutex_unlock(&m_mutex);
	return temp;
}


void 
MemPool::Free(void* pstr)
{
	pthread_mutex_lock(&m_mutex);
	MemPoolFree(pstr);
	pthread_mutex_unlock(&m_mutex);
}

void* 
MemPool::MemPoolAlloc(int size)
{
	if (m_SliceAllocNum >= m_SliceTolNum || 
		size > m_SliceSize)
	{
		return malloc(size);
	}
	int i;
	int temp;

	for (i=m_Allocflag; i<m_SliceTolNum; ++i)
	{
		temp = (m_SliceSize+1)*i;
		if (m_pSlice[temp] == 0)
		{
			m_pSlice[temp] = 1;
			++m_Allocflag, ++m_SliceAllocNum;
			m_Allocflag = m_Allocflag % m_SliceTolNum;
			return (void*)(m_pSlice + (temp + 1));
		}
	}
	for (i=0; i<m_Allocflag; ++i)
	{
		temp = (m_SliceSize+1)*i;
		if (m_pSlice[temp] == 0)
		{
			m_pSlice[temp] = 1;
			++m_Allocflag, ++m_SliceAllocNum;
			m_Allocflag = m_Allocflag % m_SliceTolNum;
			return (void*)(m_pSlice + (temp + 1));
		}
	}
	return NULL;
}

void 
MemPool::MemPoolFree(void* pstr)
{
	if (pstr < m_pStart || pstr > m_pEnd)
	{
		Debug(I_INFO, "--------------free--------------");
		free(pstr);
	}
	else
	{
	/*
		if ((pstr - m_pStart) % (m_SliceSize+1) != 0)
		{
			Debug(I_ERROR, "free error.");
			exit(2);
		}
	*/
		*(char*)((char*)pstr - 1) = 0;
		--m_SliceAllocNum;
	}
	return ;
}

int MemPool::testMemPool()
{
	int allocNum = 0, freeNum = 0;
	int i, temp;
	
	for (i=0; i<m_SliceTolNum; ++i)
	{
		temp = (m_SliceSize+1)*i;
		if (m_pSlice[temp] == 1)
		{
			++allocNum;
		}
		else if (m_pSlice[temp] == 0)
		{
			++freeNum;
		}
		else
			return -1;
	}

	printf("freeNum=%d, allocNum=%d, m_SliceAllocNum=%d\n", 
		freeNum, allocNum, m_SliceAllocNum);

	if (m_SliceAllocNum != allocNum)
		return -1;
	return 0;
}

int MemPool::test()
{
	int temp;
	pthread_mutex_lock(&m_mutex);
	temp = testMemPool();
	pthread_mutex_unlock(&m_mutex);
	return temp;
}


#if 0
MemPool* mem = new MemPool(64, 1024);
#define TEST_COUNT  100000
volatile int exitNum = 0;

void* testFun1(void* arg)
{
	char* pStr[128];
	int i, cnt = 0;
	for (i=0; i<128; ++i)
		pStr[i] = NULL;

	while (1)
	{
		printf("testcnt=%d\n", cnt);
		for (i=0; i<128; ++i)
		{
			pStr[i] = (char*)mem->Alloc(64);
			// 对内存赋值，如果算法出错，检测将能检测到
			memset(pStr[i], 'a', 64);
			usleep(10);
		}
		usleep(10);
		for (i=0; i<128; ++i)
		{
			mem->Free(pStr[i]), pStr[i] = NULL;
		}
		cnt++;

		if (cnt == TEST_COUNT)
		{
			exitNum++;
			return (void*)NULL;
		}
		usleep(5000);
	}
}

void* testFun2(void* arg)
{
	char* pStr[256];
	int i, cnt = 0;
	for (i=0; i<256; ++i)
		pStr[i] = NULL;

	while (1)
	{
		for (i=0; i<256; ++i)
		{
			pStr[i] = (char*)mem->Alloc(64);
			memset(pStr[i], 'a', 64);
			usleep(50);
		}
		usleep(50);
		for (i=0; i<256; ++i)
		{
			mem->Free(pStr[i]), pStr[i] = NULL;
		}
		cnt++;

		if (cnt == TEST_COUNT)
		{	
			exitNum++;
			return (void*)NULL;
		}
		usleep(2000);
	}
}

void* testFun3(void* arg)
{
	char* pStr[64];
	int i, cnt = 0;
	for (i=0; i<64; ++i)
		pStr[i] = NULL;

	while (1)
	{
		for (i=0; i<64; ++i)
		{
			pStr[i] = (char*)mem->Alloc(64);
			memset(pStr[i], 'a', 64);
			usleep(10);
		}
		usleep(100);
		for (i=0; i<64; ++i)
		{
			mem->Free(pStr[i]), pStr[i] = NULL;
		}
		cnt++;

		if (cnt == TEST_COUNT)
		{
			exitNum++;
			return (void*)NULL;
		}
		usleep(5000);
	}
}


int main()
{
	pthread_t testID1, testID2, testID3;
	pthread_create(&testID1, 0, testFun1, NULL);
	pthread_detach(testID1);
	
	pthread_create(&testID2, 0, testFun2, NULL);
	pthread_detach(testID2);

	pthread_create(&testID3, 0, testFun3, NULL);
	pthread_detach(testID3);

	while(1)
	{
		usleep(1000 * 400);
		mem->test();
		if (exitNum == 3)
			return 0;
	}

	delete mem, mem = NULL;
	return 0;
}
#endif

