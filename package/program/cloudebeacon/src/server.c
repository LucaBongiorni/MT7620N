#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define PORT 6023

int main()
{
    setvbuf(stdout, NULL, _IONBF, 0);
    fflush(stdout);
    int sock = -1;
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        printf("socket error\n");
        return -1;
    }
    const int opt = 1;
    int nb = 0;
    nb = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *)&opt, sizeof(opt));
    if(nb == -1)
    {
        printf("set socket error...\n");
        return -1;
    }
    struct sockaddr_in addrto;
    bzero(&addrto, sizeof(struct sockaddr_in));
    addrto.sin_family = AF_INET;
    addrto.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    addrto.sin_port = htons(PORT);
    int nlen = sizeof(addrto);
    /*
        if (bind(sock,(struct sockaddr *)&(addrto), sizeof(struct sockaddr_in)) == -1)
        {
            printf("bind error...\n");
            return -1;
        }
    */
    bzero(&addrto, sizeof(struct sockaddr_in));
    addrto.sin_family = AF_INET;
    addrto.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    addrto.sin_port = htons(PORT);
    struct sockaddr_in from;
    bzero(&from, sizeof(struct sockaddr_in));
    from.sin_family = AF_INET;
    from.sin_addr.s_addr = htonl(INADDR_ANY);
    from.sin_port = htons(PORT);
    char recvbuff[1024] = {0};
    int len = sizeof(struct sockaddr_in);
    while(1)
    {
        sleep(1);
        char smsg[] = {"abcdef"};
        int ret = sendto(sock, smsg, strlen(smsg), 0, (struct sockaddr*)&addrto, nlen);
        if(ret<0)
        {
            printf("send error....\n");
        }
        else
        {
            printf("ok \n");
        }
        // ½ÓÊÕ
        memset(recvbuff, 0, 1024);
        ret = recvfrom(sock, recvbuff, 1024, 0, (struct sockaddr*)&from, (socklen_t*)&len);
        if (ret < 0)
        {
        }
        else
        {
            printf("recvbuff:%s\n", recvbuff);
        }
    }
    return 0;
}


