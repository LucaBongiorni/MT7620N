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
// 单一长度线程
class MemPool
{
public:
    MemPool();
    MemPool(int sliceSize, int sliceTolNum);
    ~MemPool();
	
public:
    void *Alloc(int size);
    void Free(void* pstr);

	// 函数功能: 验证内存池结构体是否正确
	int test();
	
private:
    pthread_mutex_t m_mutex;
    int m_SliceAllocNum;   // 分配了多少片
    int m_SliceSize;       // 每片多大
    int m_SliceTolNum;     // 总共多少片
    int m_Allocflag;       // 分配到第几个的标志，用于提高分配速度
    void* m_pStart;
    void* m_pEnd;
    char *m_pSlice;

private:
	void *MemPoolAlloc(int size);
    void MemPoolFree(void* pstr);

	// 函数功能: 验证内存池结构体是否正确
	int testMemPool();
};



#endif /*__MEM_POOL_H__*/

