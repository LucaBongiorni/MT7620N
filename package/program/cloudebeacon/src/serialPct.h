#ifndef __SERIALS_PCT__H__
#define __SERIALS_PCT__H__

#include <pthread.h>

#if 0
ά��һ��buff�ṹ�壬����������ʱ��������������ȷ��񣬶������ݼӵ�buff���棬
��ȡ���ݵ�ʱ�򣬻����Э��ͷβ��־����ȡ���ݡ�
���ַ�ʽ���ܿ�����ԭ����Ҫ��Э����Ʋ����ƣ�
�����ݽṹ�ڶ������±��뽫�������޸�Ϊ��������
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
	// ��������: ���buff
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
	// ��������: ��pstartλ�ÿ�ʼ��pendλ�ã�����chr�ַ�
	// ��������: pstart, pend, �ַ�ָ��
	//           chr�����ҵ��ַ�
	// �� �� ֵ: �ɹ�����chr��λ�ã�ʧ�ܷ���NULL
	static const char* 
	SerStrchr(const char* pstart, const char* pend, char chr);
	// ��������: ��pendλ�ÿ�ʼ����pstartλ�ã�����chr�ַ�
	// ��������: pstart, pend, �ַ�ָ��
	//           chr�����ҵ��ַ�
	// �� �� ֵ: �ɹ�����chr��λ�ã�ʧ�ܷ���NULL
	static const char* 
	OsiteStrchr(const char* pend, const char* pstart, char chr);
	// ��������: �ж�buff������û�н�β��־
	//           
	bool judgeHasTailFlag();

	// ��������: ��������������
	int addBuffToSerBuff(const char* buff, int buffLen);
	// ��������: �ӻ�����ȡ����
	// ��������: outBuff�������������ȡ��������
	//           outLen�������������ȡ�������ݳ���
	// �� �� ֵ: �ɹ����ض�ȡ���ĳ��ȣ�ʧ�ܷ���0
	int getFrmFromSerBuff(char* outBuff, int *outLen);
};




#endif /*__SERIALS_PCT__H__*/

