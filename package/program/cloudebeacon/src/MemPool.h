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
// ��һ�����߳�
class MemPool
{
public:
    MemPool();
    MemPool(int sliceSize, int sliceTolNum);
    ~MemPool();
	
public:
    void *Alloc(int size);
    void Free(void* pstr);

	// ��������: ��֤�ڴ�ؽṹ���Ƿ���ȷ
	int test();
	
private:
    pthread_mutex_t m_mutex;
    int m_SliceAllocNum;   // �����˶���Ƭ
    int m_SliceSize;       // ÿƬ���
    int m_SliceTolNum;     // �ܹ�����Ƭ
    int m_Allocflag;       // ���䵽�ڼ����ı�־��������߷����ٶ�
    void* m_pStart;
    void* m_pEnd;
    char *m_pSlice;

private:
	void *MemPoolAlloc(int size);
    void MemPoolFree(void* pstr);

	// ��������: ��֤�ڴ�ؽṹ���Ƿ���ȷ
	int testMemPool();
};



#endif /*__MEM_POOL_H__*/

