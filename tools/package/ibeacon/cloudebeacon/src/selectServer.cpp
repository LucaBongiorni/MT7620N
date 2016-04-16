#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>




#include "selectServer.h"
#include "defCom.h"



static t_SignalHandler userSighupHandler = NULL;
static t_SignalHandler userSigintHandler = NULL;

CtrlInfo_t *controllers = NULL;
Selector_t *sel = NULL;

static int max_controller_ports = 4;	// ͬһʱ�䣬�����������������������
static int num_controller_ports = 0;	// ��ǰ��������


static void init_fd(fd_control_t *fd);



// ��������: 
// ��������: sig
//           handler 
void
SetSignalHandler(int sig, t_SignalHandler handler)
{
    if (sig == SIGHUP)
		userSighupHandler = handler;
    else if (sig == SIGINT)
		userSigintHandler = handler;
}

static void
shutdown_controller(CtrlInfo_t *cntlr)
{
    CtrlInfo_t *prev;
    CtrlInfo_t *curr;

    ClearFdHandlers(sel, cntlr->tcpFD);
    Close(cntlr->tcpFD);
    if (cntlr->outBuf != NULL) 
	{
		free(cntlr->outBuf);
    }
    cntlr->outBuf = NULL;

    /* Remove it from the linked list. */
    prev = NULL;
    curr = controllers;
    while (curr != NULL)
	{
		if (cntlr == curr)
		{
		    if (prev == NULL) 
			{
				controllers = controllers->next;
		    }
			else 
			{
				prev->next = curr->next;
		    }
		    num_controller_ports--;
		    break;
		}

		prev = curr;
		curr = curr->next;
    }

    free(cntlr);
}

// ��������: 
// ��������: 
// �� �� ֵ: 
int dealWithReadBuff()
{


}



// ��������: ������
// ��������: fd
//           data 
static void
handleTcpFdRead(void* selector, int fd, void *data)
{
    CtrlInfo_t *cntlr = (CtrlInfo_t *)data;
	Selector_t *sel = (Selector_t *)selector;
    int read_count;
    int start, end;
    int i, j;

    if (cntlr->inBufLen == INBUF_SIZE) 
	{
        const char *err = "Input line too long\n\r";
		CtrlOutput(sel, cntlr, err, strlen(err));
		cntlr->inBufLen = 0;
		return ;
    }

	// ��ȡ����
    read_count = read(fd, &(cntlr->inBuf[cntlr->inBufLen]), INBUF_SIZE - cntlr->inBufLen);
    if (read_count < 0) 
	{
		if (errno == EINTR || errno == EAGAIN)
		{
		    // �жϺͷ�������������������
		    return ;
		}

		write_error_log("read error for controller port: %m");
		shutdown_controller(cntlr);
		return ;
    } 
	else if (read_count == 0) 
	{
		/* The other end closed the port, shut it down. */
		write_error_log("remote IP has been closed.");
		shutdown_controller(cntlr);
		return ;
    }

	// �����ȡ��������
    start = cntlr->inBufLen;
	end   = cntlr->inBufLen + read_count;
	
    read_count = dealWithReadBuff();
	// δ����


	
	start += read_count;

	// ��¼���������
    cntlr->inBufLen += read_count;
    // ɾ����ȡ��������
    for (j=0, i=start; i<end; ++i, ++j)
    {
		cntlr->inBuf[j] = cntlr->inBuf[i];
	}
	cntlr->inBufLen = end - start;
	return ;
}


/* 
   The TCP port has room to write some data.  This is only activated
   if a write fails to complete, it is deactivated as soon as writing
   is available again. 
*/
static void
handleTcpFdWrite(void* selector, int fd, void *data)
{
    CtrlInfo_t *cntlr = (CtrlInfo_t *)data;
	Selector_t *sel   = (Selector_t *)selector;
	
    int write_count;

    write_count = write(cntlr->tcpFD, &(cntlr->outBuf[cntlr->outBufPos]), cntlr->outBufLen);
    if (write_count == -1)
	{
		if (errno == EAGAIN)
		{
			/* This again was due to O_NONBLOCK, just ignore it. */
		} 
		else if (errno == EPIPE) 
		{
		    goto out_fail;
		} 
		else 
		{
		    write_error_log("The tcp write for controller had error: %m");
		    goto out_fail;
		}
    } 
	else 
	{
		cntlr->outBufLen -= write_count;
		if (cntlr->outBufLen != 0) 
		{
		    /* We didn't write all the data, continue writing. */
		    cntlr->outBufPos += write_count;
		} 
		else
		{
		    /* We are done writing, turn the reader back on. */
		    free(cntlr->outBuf);
		    cntlr->outBuf = NULL;
		    SetFdReadHandler(sel, cntlr->tcpFD, SEL_FD_HANDLER_ENABLED);
		    SetFdWriteHandler(sel, cntlr->tcpFD, SEL_FD_HANDLER_DISABLED);
		}
    }
	
out:
    return;

out_fail:
    shutdown_controller(cntlr);
}


/* Handle an exception from the TCP port. */
static void
handleTcpFdExcept(void* selector, int fd, void *data)
{
    CtrlInfo_t *cntlr = (CtrlInfo_t *)data;
	Selector_t *sel = (Selector_t *)selector;

    write_error_log("Select exception for controller port");
    shutdown_controller(cntlr);
}


// ��������: 
// ��������: sel   
//           fd 
//           data 
//           read_handler, write_handler,except_handler �ֱ�Ϊ��д���������
// �� �� ֵ: 
void
SetFdHandlers(Selector_t *sel, int fd, void *data,
		    selFdHandler_t read_handler, selFdHandler_t write_handler, selFdHandler_t except_handler)
{
    sel->fds[fd].inUse        = 1;
    sel->fds[fd].data         = data;
    sel->fds[fd].handleRead   = read_handler;
    sel->fds[fd].handleWrite  = write_handler;
    sel->fds[fd].handleExcept = except_handler;

    /* Move maxfd up if necessary. */
    if (fd > sel->maxfd) 
	{
		sel->maxfd = fd;
    }
}

// ��������: 
// ��������: sel  
//           fd 
// �� �� ֵ: 
void
ClearFdHandlers(Selector_t *sel, int fd)
{
    init_fd(&(sel->fds[fd]));
    FD_CLR(fd, &sel->readSet);
    FD_CLR(fd, &sel->writeSet);
    FD_CLR(fd, &sel->exceptSet);

    /* Move maxfd down if necessary. */
    if (fd == sel->maxfd) 
	{
		while ((sel->maxfd >= 0) && (! sel->fds[sel->maxfd].inUse)) 
		{
		    sel->maxfd--;
		}
    }
}




void
SetFdReadHandler(Selector_t *sel, int fd, int state)
{
	if (NULL == sel || fd <= 0)
		return ;
	
    if (state == SEL_FD_HANDLER_ENABLED) 
	{
		FD_SET(fd, &sel->readSet);
    } 
	else if (state == SEL_FD_HANDLER_DISABLED)
	{
		FD_CLR(fd, &sel->readSet);
    }
	else 
	{
		return ;
	}
}

void 
SetFdWriteHandler(Selector_t *sel, int fd, int state)
{
	if (NULL == sel || fd <= 0) return ;
    if (state == SEL_FD_HANDLER_ENABLED) 
	{
		FD_SET(fd, &sel->writeSet);
    }
	else if (state == SEL_FD_HANDLER_DISABLED) 
	{
		FD_CLR(fd, &sel->writeSet);
    }
	else
	{
		return ;
	}
}

/* Set whether the file descriptor will be monitored for exceptions
   on the file descriptor. */
void 
SetFdExceptHandler(Selector_t *sel, int fd, int state)
{
	if (NULL == sel || fd <= 0) return ;
    if (state == SEL_FD_HANDLER_ENABLED) 
	{
		FD_SET(fd, &sel->exceptSet);
    } 
	else if (state == SEL_FD_HANDLER_DISABLED) 
	{
		FD_CLR(fd, &sel->exceptSet);
    }
	else
	{
		return ;
	}
}

static void
init_fd(fd_control_t *fd)
{
    fd->inUse        = 0;
    fd->data         = NULL;
    fd->handleRead   = NULL;
    fd->handleWrite  = NULL;
    fd->handleExcept = NULL;
}


// ��������: 
// ��������: fd 
//           data 
static void
AcceptHandlerPortRead(void* selector, int fd, void *data)
{
	Selector_t* sel = (Selector_t*)selector;
	CtrlInfo_t* cntlr;
	socklen_t len;
	const char* err = NULL;
	int optval;

	cntlr = (CtrlInfo_t*)malloc(sizeof(*cntlr));
	if (cntlr == NULL) 
	{
		err = "Could not allocate controller port\n\r";
		goto errout2;
	}
	
	len = sizeof(cntlr->remote);
	cntlr->tcpFD = accept(fd, (struct sockaddr*)&(cntlr->remote), &len);
	if (cntlr->tcpFD == -1)
	{
		write_error_log("Could not accept on controller port: %m");
		goto errout;
	}
	
	// ���÷�����ģʽ
	if (fcntl(cntlr->tcpFD, F_SETFL, O_NONBLOCK) == -1)
	{
		Close(cntlr->tcpFD);
		write_error_log("Could not fcntl the tcp port: %m");
		goto errout;
	}
	
	// ���������������
	optval = 1;
	if (setsockopt(cntlr->tcpFD, SOL_SOCKET, SO_KEEPALIVE, (void *)&optval,
			   sizeof(optval)) == -1) 
	{
		Close(cntlr->tcpFD);
		write_error_log("Could not enable SO_KEEPALIVE on the tcp port: %m");
		goto errout;
	}
	
	cntlr->inBufLen      = 0;
	cntlr->outBuf        = NULL;
	cntlr->monitorPortId = NULL;
	
	SetFdHandlers(sel, cntlr->tcpFD, cntlr, handleTcpFdRead, handleTcpFdWrite, handleTcpFdExcept);
	
	// ʹ֮�ܶ�������д
	SetFdReadHandler(sel, cntlr->tcpFD, SEL_FD_HANDLER_DISABLED);
	SetFdWriteHandler(sel, cntlr->tcpFD, SEL_FD_HANDLER_ENABLED);
	printf("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n");

	// �齨����
	cntlr->next = controllers;
	controllers = cntlr;
	num_controller_ports++;
	return ;
	
errout:
	free(cntlr);
	return ;

errout2:
	return ;
}



// ��������: ԭ close �����ķ�װ
// �������: iSockFD : Ҫ�رյ��׽���������
// �������: ��
// �� �� ֵ: 0: �رճɹ���-1: �ر�ʧ��
int 
Close(int iSockFD)
{
	if (iSockFD > 0)
    {
    	//shutdown(iSockFD, SHUT_RDWR);
        if (-1 == close(iSockFD))
        {
			write_log("close socket[%d] err:%s\n", iSockFD, strerror(errno));      
        }
        iSockFD = -1;
        return SUCCESS;
    }
    else
    {
        return FAILED;
    }
}

// ��������: ����socket�����������˿ں�ip����״̬
// �������:
// �� �� ֵ: �ɹ� 0��ʧ�� -1
int 
SetSocketStatus(int SockFd)
{
    //ȡ���ļ���������״̬
    int nFlags = fcntl(SockFd, F_GETFL, 0);
    if (-1 == nFlags)
    {
        write_error_log("SetSocketStatus error");
        return FAILED;
    }
	
    //�����ļ�������Ϊ������
    if ( -1 == fcntl(SockFd, F_SETFL, nFlags | O_NONBLOCK) )
    {
        write_error_log("SetSocketStatus fcntl error");
        return FAILED;
    }
	
    int bReUseAddr = 1;
    //�����׽��ֵ�����ʹ���ܹ��ڼ����������ʱ������ٴ�ʹ���׽��ֵĶ˿ں�IP
    int error = setsockopt(SockFd, SOL_SOCKET, SO_REUSEADDR, (const char*)&bReUseAddr, sizeof(bReUseAddr));
    if(error != 0)
    {
        return FAILED;
    }
	
    //SO_LINGER, ���Źر�socket���ᱣ֤������������ȫ���������
    //��ѡ���UDP��SOCKET��Ч
    //struct linger zeroLinger = {1, 2};
    //setsockopt(SockFd, SOL_SOCKET, SO_LINGER, (const char *)&zeroLinger, sizeof(linger));
    return SUCCESS;
}



void 
CtrlOutput(Selector_t *sel, CtrlInfo_t *cntlr, const char *data, int dataLen)
{
    if (cntlr->outBuf != NULL) 
	{
		int  newSize = cntlr->outBufLen + dataLen;

		if (newSize <= cntlr->outBufSize) 
		{
		    int i;

		    if (cntlr->outBufPos > 0) 
			{
				for (i=0; i<cntlr->outBufLen; i++) 
				{
				    cntlr->outBuf[i] = cntlr->outBuf[cntlr->outBufPos + i];
				}
		    }
		    memcpy(&(cntlr->outBuf[cntlr->outBufLen]), data, dataLen);
		} 
		else 
		{
		    // ����һ�������ڴ�
		    char *newbuf;

		    // ������ڴ��СΪ 1024 ��������
		    newSize = ((newSize / 1024) * 1024) + 1024;
		    newbuf = (char*)malloc(newSize);
		    if (newbuf == NULL) 
			{
				write_error_log("Malloc Memory Failed.");
				return;
		    }

		    cntlr->outBufSize = newSize;

		    // ���������ݿ������µ��ڴ���
		    memcpy(newbuf, &(cntlr->outBuf[cntlr->outBufPos]), cntlr->outBufLen);
		    memcpy(newbuf+cntlr->outBufLen, data, dataLen);
		    free(cntlr->outBuf);
		    cntlr->outBuf = newbuf;
		}
		cntlr->outBufPos = 0;
		cntlr->outBufLen += dataLen;
    } 
	else 
	{
		char *newbuf;
		int  newSize = ((dataLen / 1024) * 1024) + 1024;

		newbuf = (char*)malloc(newSize);
		if (newbuf == NULL) 
		{
		    write_error_log("Malloc Memory Failed.");
		    return;
		}
		
		cntlr->outBufSize = newSize;
		memcpy(newbuf, data, dataLen);
		cntlr->outBuf       = newbuf;
		cntlr->outBufPos   	= 0;
		cntlr->outBufLen 	= dataLen;

		// ����Ϊ����ģʽ
		SetFdReadHandler(sel,  cntlr->tcpFD, SEL_FD_HANDLER_DISABLED);
		SetFdWriteHandler(sel, cntlr->tcpFD, SEL_FD_HANDLER_ENABLED);
    }
}


int
scan_tcp_port(const char *ip, const char* port, int domain,
	      struct sockaddr_storage *addr, socklen_t *addr_len)
{
    char *strtok_data;
    struct addrinfo hints, *ai;

    memset(addr, 0, sizeof(*addr));    
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags  = AI_PASSIVE;
    hints.ai_family = domain;
    if (getaddrinfo(ip, port, &hints, &ai))
		return -1;

    memcpy(addr, ai->ai_addr, ai->ai_addrlen);
    *addr_len = ai->ai_addrlen;
    freeaddrinfo(ai);
    return 0;
}


// ��������: ʵ���� Selector_t ��һ������
// ��������: newSelector
// �� �� ֵ: 
int
AllocSelector(Selector_t **newSelector)
{
    Selector_t *sel = NULL;
    int        i;

    sel = (Selector_t *)malloc(sizeof(Selector_t));
    if (!sel)
		return ENOMEM;

    FD_ZERO(&sel->readSet);
    FD_ZERO(&sel->writeSet);
    FD_ZERO(&sel->exceptSet);

    for (i=0; i<FD_SETSIZE; ++i) 	// FD_SETSIZE = 1024
	{
		init_fd(&(sel->fds[i]));
    }
	//printf("FD_SETSIZE=%d\n", FD_SETSIZE);

    sel->maxfd = 0;
    //sel->timer_top  = NULL;
    //sel->timer_last = NULL;

    *newSelector = sel;
    return 0;
}


// ��������: TCP �׽��ּ���
// �������: listenNum ��������
// �� �� ֵ: -1 ʧ�ܣ��ɹ������׽���
int 
InitTcpListen(Selector_t *sel, u_int16 uPort, int listenNum)
{
    int SockFd = socket(AF_INET, SOCK_STREAM, 0); 
    if (SockFd == -1)
    {
		write_error_log("InitTcpListen failed, %s", strerror(errno));
        return -1;
    }
	
    //����Ϊ������ģʽ
    //�����׽��ֵ�����ʹ���ܹ��ڼ����������ʱ������ٴ�ʹ���׽��ֵĶ˿ں�IP
    if (SetSocketStatus(SockFd) != SUCCESS)
    {
        write_error_log("set socket status errror!");
        return -1;
    }
	
    struct sockaddr_in Addr = {0};
    Addr.sin_family      = AF_INET;
    Addr.sin_port        = htons(uPort);
    Addr.sin_addr.s_addr = INADDR_ANY;
    int iRtn = bind(SockFd, (struct sockaddr*)&Addr, sizeof(struct sockaddr_in));
    if (-1 == iRtn)
    {
        Close(SockFd);
        write_error_log("InitTcpListen failed, %s", strerror(errno));
        return -1;
    }
	
    iRtn = listen(SockFd, listenNum);
    if (-1 == iRtn)
    {
        Close(SockFd);
        write_error_log("InitTcpListen failed, %s", strerror(errno));
        return -1;
    }

	// ��ӵ�selector�У�ʹ֮�ܶ�
	SetFdHandlers(sel, SockFd, NULL, AcceptHandlerPortRead, NULL, NULL);
    SetFdReadHandler(sel, SockFd, SEL_FD_HANDLER_ENABLED); 
    return SockFd;
}


void
SelectLoop(Selector_t *sel)
{
	struct timeval timeout;
	timeout.tv_sec  = 0;
	timeout.tv_usec = 1000*50;
	
	for (;;)
	{
	    fd_set tmp_read_set;
	    fd_set tmp_write_set;
	    fd_set tmp_except_set;
	    int i, err;
		
	    memcpy(&tmp_read_set, &sel->readSet, sizeof(tmp_read_set));
	    memcpy(&tmp_write_set, &sel->writeSet, sizeof(tmp_write_set));
	    memcpy(&tmp_except_set, &sel->exceptSet, sizeof(tmp_except_set));

	    err = select(sel->maxfd+1, &tmp_read_set, &tmp_write_set, &tmp_except_set, &timeout);
	    if (err == 0)
		{
			// ��ʱ��continue
	    } 
		else if (err < 0) 
		{
			// ����
			if (errno == EINTR) 
			{
			    /* EINTR is ok, just restart the operation. */
			    timeout.tv_sec  = 1;
			    timeout.tv_usec = 0;
			} 
			else 
			{
			    /* An error is bad, we need to abort. */
			    write_error_log("select_loop() - select: %m");
			    exit(1);
			}
	    } 
		else 
		{
			for (i=0; i<=sel->maxfd; i++) 
			{
			    if (FD_ISSET(i, &tmp_read_set)) 
				{
					if (sel->fds[i].handleRead == NULL) 
					{
					    // û�����øþ�����Ƴ���
					    printf("------------del Read---------Fd=%d\n", i);
					    SetFdReadHandler(sel, i, SEL_FD_HANDLER_DISABLED);
					}
					else 
					{
					    sel->fds[i].handleRead(sel, i, sel->fds[i].data);
					}
			    }
			    if (FD_ISSET(i, &tmp_write_set)) 
				{
					if (sel->fds[i].handleWrite == NULL) 
					{
					    // û�����øþ�����Ƴ���
					    printf("------------del write---------Fd=%d\n", i);
					    SetFdWriteHandler(sel, i, SEL_FD_HANDLER_DISABLED);
					} 
					else 
					{
					    sel->fds[i].handleWrite(sel, i, sel->fds[i].data);
					}
			    }
			    if (FD_ISSET(i, &tmp_except_set)) 
				{
					if (sel->fds[i].handleExcept == NULL)
					{
					    // û�����øþ�����Ƴ���
					    printf("------------del Except---------Fd=%d\n", i);
					    SetFdExceptHandler(sel, i, SEL_FD_HANDLER_DISABLED);
					}
					else 
					{
					    sel->fds[i].handleExcept(sel, i, sel->fds[i].data);
					}
			    }
			}
	    }
	}
}




int 
main()
{
	int nRet;
	int sockFd;

	// ��ʼ�� Selector
	AllocSelector(&sel);
	printf("111111111111111111111\n");
	sockFd = InitTcpListen(sel, 8888, 10);
	if (sockFd == -1)
	{
		write_error_log("Init TCP Listen failed.");
		free(sel);
	}
	printf("222222222222222222222\n");
	SelectLoop(sel);

	if (sel)free(sel);
	return 0;
}


