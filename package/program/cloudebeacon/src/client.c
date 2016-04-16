#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 6023

int main()
{
    setvbuf(stdout, NULL, _IONBF, 0);
    fflush(stdout);
    // 绑定地址
    struct sockaddr_in addrto;
    bzero(&addrto, sizeof(struct sockaddr_in));
    addrto.sin_family = AF_INET;
    addrto.sin_addr.s_addr = htonl(INADDR_ANY);
    addrto.sin_port = htons(PORT);
    // 广播地址
    struct sockaddr_in from;
    bzero(&from, sizeof(struct sockaddr_in));
    from.sin_family = AF_INET;
    from.sin_addr.s_addr = htonl(INADDR_ANY);
    from.sin_port = htons(PORT);
    int sock = -1;
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        printf("socket error\n");
        return -1;
    }
    const int opt = 1;
    //设置该套接字为广播类型，
    int nb = 0;
    nb = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *)&opt, sizeof(opt));
    if(nb == -1)
    {
        printf("set socket error...\n");
        return -1;
    }
    if (bind(sock,(struct sockaddr *)&(addrto), sizeof(struct sockaddr_in)) == -1)
    {
        printf("bind error...\n");
        return -1;
    }
    int len = sizeof(struct sockaddr_in);
    char smsg[100] = {0};
    bzero(&addrto, sizeof(struct sockaddr_in));
    addrto.sin_family = AF_INET;
    addrto.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    addrto.sin_port = htons(PORT);
    int nlen = sizeof(addrto);
    while(1)
    {
        int ret = recvfrom(sock, smsg, 100, 0, (struct sockaddr*)&from,(socklen_t*)&len);
        if (ret<=0)
        {
            printf("read error....\n");
        }
        else
        {
            printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
            printf("ret=%d, recv=%s\n", ret, smsg);
            printf("from.sin_addr.s_addr=%s, from.sin_port=%d\n",
                   inet_ntoa(from.sin_addr), ntohs(from.sin_port));
            printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
        }
        memset(smsg, 'a', 20);
        ret = sendto(sock, smsg, 20, 0, (struct sockaddr*)&addrto, nlen);
        if (ret<=0)
        {
        }
        else
        {
            printf("send succuss.\n");
        }
        sleep(1);
    }
    return 0;
}


