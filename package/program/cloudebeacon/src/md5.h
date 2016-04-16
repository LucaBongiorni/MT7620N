#ifndef _h_MD5
#define _h_MD5

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


#ifndef UINT4
#define UINT4 unsigned long
#endif

#ifndef POINTER
#define POINTER unsigned char *
#endif

/* MD5 context. */
typedef struct 
{
	UINT4 state[4];             /* state (ABCD) */
	UINT4 count[2];        		/* number of bits, modulo 2^64 (lsb first) */
	unsigned char buffer[64];   /* input buffer */
} MD5_CONTEXT;

void MD5Init (MD5_CONTEXT *);
void MD5Update (MD5_CONTEXT *, unsigned char *, unsigned int);
void MD5Final (unsigned char [16], MD5_CONTEXT *);

// 计算文件的 MD5 值
void MDFile (char *filename, char buff[16]);

// 函数功能: 计算输入字符串的md5值
// 函数参数: inBuff, 输入字符串，inBuffLen，输入字符串的长度
//           outBuff, 输出的md5值，outBuffSize，输出的缓冲区大小，必须大于16；
// 返 回 值: 成功返回0，失败返回-1
int MD5Buff(unsigned char *inBuff, int inBuffLen, unsigned char* outBuff, int outBuffSize);



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */



#endif /* _h_MD5 */

