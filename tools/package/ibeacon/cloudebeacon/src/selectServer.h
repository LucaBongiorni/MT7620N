#ifndef __SELECT__SOCKET_H__
#define __SELECT__SOCKET_H__

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <syslog.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>


#ifndef SUCCESS
#define SUCCESS 0
#endif 
#ifndef FAILED
#define FAILED -1
#endif

#define SEL_FD_HANDLER_ENABLED	0
#define SEL_FD_HANDLER_DISABLED	1

#define INBUF_SIZE 1024


typedef void (*selFdHandler_t)(void*, int fd, void *data);
typedef void (*t_SignalHandler)(void);


typedef struct fd_control_s
{
    int             inUse;
    void            *data;
    selFdHandler_t  handleRead;
    selFdHandler_t  handleWrite;
    selFdHandler_t  handleExcept;
} fd_control_t;




// ����ÿ�������������Ӷ���Ϣ
typedef struct controller_info 
{
    int tcpFD;
    struct sockaddr remote;	

	// �����ַ���
    unsigned char inBuf[INBUF_SIZE+1];
    int  inBufLen;	

	// �����ַ���
    char *outBuf;		// ������ַ�
    int  outBufSize;	// outbuf ��������С
    int  outBufPos;		// outbuf ��ǰλ��
    int  outBufLen;  	// �� outBufPos λ������ʣ�µĳ���

    void *monitorPortId;

    struct controller_info *next;
}CtrlInfo_t;




typedef struct selector_s
{
	// ��������������
    fd_control_t fds[FD_SETSIZE];
    
    // ����������
    fd_set readSet;
    fd_set writeSet;
    fd_set exceptSet;

    int maxfd; // ����������
}Selector_t;






int  Close(int iSockFD);
void ClearFdHandlers(Selector_t *sel, int fd);
void CtrlOutput(Selector_t *sel, CtrlInfo_t *cntlr, const char *data, int dataLen);
void SetFdReadHandler(Selector_t *sel, int fd, int state);
void SetFdWriteHandler(Selector_t *sel, int fd, int state);
void SetFdExceptHandler(Selector_t *sel, int fd, int state);







#endif /*__SELECT__SOCKET_H__*/

