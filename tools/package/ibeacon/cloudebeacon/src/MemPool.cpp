#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>


#include "MemPool.h"


MemPool::MemPool()
{
	m_SliceAllocNum = 0;
	m_SliceSize = DEF_SLICE_SIZE;
	m_SliceTolNum = DEF_SLICE_NUM;

	int size = (m_SliceSize+1) * m_SliceTolNum;
	m_pSlice = (char*)malloc(size);
	if (NULL == m_pSlice)
	{
		write_error_log("Malloc Memory Failed.");
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
	if (NULL == m_pSlice)
	{
		write_error_log("Malloc Memory Failed.");
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
	if (m_SliceAllocNum >= m_SliceTolNum || size > m_SliceSize)
	{
		write_log("-------------------malloc------------");
		return malloc(size);
	}
	int i;
	int temp;

	pthread_mutex_lock(&m_mutex);
	for (i=m_Allocflag; i<m_SliceTolNum; ++i)
	{
		temp = (m_SliceSize+1)*i;
		if (m_pSlice[temp] == 0)
		{
			m_pSlice[temp] = 1;
			++m_Allocflag, ++m_SliceAllocNum;
			m_Allocflag = m_Allocflag % m_SliceTolNum;
			pthread_mutex_unlock(&m_mutex);
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
			pthread_mutex_unlock(&m_mutex);
			return (void*)(m_pSlice + (temp + 1));
		}
	}
	pthread_mutex_unlock(&m_mutex);
	return NULL;
}


void 
MemPool::Free(void* pstr)
{
	if (pstr < m_pStart || pstr > m_pEnd)
	{
		write_log("--------------free--------------");
		free(pstr);
	}
	else
	{
	/*
		if ((pstr - m_pStart) % (m_SliceSize+1) != 0)
		{
			write_error_log("free error.");
			exit(2);
		}
	*/
		pthread_mutex_lock(&m_mutex);
		*(char*)((char*)pstr - 1) = 0;
		--m_SliceAllocNum;
		pthread_mutex_unlock(&m_mutex);
	}
	return ;
}



