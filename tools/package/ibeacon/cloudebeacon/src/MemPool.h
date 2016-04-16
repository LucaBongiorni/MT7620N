#ifndef __MEM_POOL_H__
#define __MEM_POOL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "defCom.h"


#define DEF_SLICE_SIZE      1024
#define DEF_SLICE_NUM       256


///////////////////////////////////////////////////////////////////////////////////
// 链表专门使用的内存池
class MemPool
{
public:
	MemPool();
	MemPool(int sliceSize, int sliceTolNum);
	~MemPool();

public:
	void *Alloc(int size);
	void Free(void* pstr);

private:
	pthread_mutex_t m_mutex;
	int m_SliceAllocNum;   // 分配了多少片
	int m_SliceSize;       // 每片多大
	int m_SliceTolNum;     // 总共多少片

	int m_Allocflag;       // 分配到第几个的标志，用于提高分配速度
	void* m_pStart;
	void* m_pEnd;
	
	char *m_pSlice;
};






#endif /*__MEM_POOL_H__*/

