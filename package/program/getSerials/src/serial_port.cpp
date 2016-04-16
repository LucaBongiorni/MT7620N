#include "includes.h"


using namespace std; 


  
Serialcom::Serialcom()  
{  
    long baud = 19200;  
    unsigned char databits = '8';  
    unsigned char parity = 'N';  
    unsigned char stopbits = '1';  
    usec = 1000;  
    port = 2;  
    stat = 0;  
    ComFd = -1;
#ifdef SERIAL_PORT_OWN_DEBUG
    std::cout<<baud<<"\t"<<databits<<"\t"<<parity<<"\t"<<stopbits<<std::endl;  
#endif
}  
  

  
Serialcom::~Serialcom()  
{  
  
}  
void Serialcom::SetSerialcom(serialconfig_t serialconf,int delay,int choice)  
{  
    long baud = serialconf.baud;  
    unsigned char databits = serialconf.databits;  
    unsigned char parity = serialconf.parity;  
    unsigned char stopbits = serialconf.stopbits;  
    usec = delay;  
    port = choice;  
    stat = 0;  
    ComFd = -1;  
}  

int Serialcom::SerialcomOpen()  
{  
    char dev[][13] = {"/dev/tty0", "/dev/tty1", "/dev/ttyS0"};//xtm 此处对应port，选择你要打开的串口  
    if((port < 0) || (port > 2) ){ 
#ifdef SERIAL_PORT_OWN_DEBUG
        printf("the port is out range");
#endif
        return -1;  
    }  
    /*open port*/  
    ComFd = open(dev[port], O_RDWR | O_NOCTTY | O_NDELAY);  
    if(ComFd < 0){  
        ComFd = open(dev[port], O_RDWR | O_NOCTTY | O_NDELAY);  
        if(ComFd < 0){   
#ifdef SERIAL_PORT_OWN_DEBUG
            perror("open serial port"); 
#endif
            return -1;  
        }  
    }  
   
    if(fcntl(ComFd, F_SETFL,0) < 0){ 
#ifdef SERIAL_PORT_OWN_DEBUG   		
        perror("fcntl F_SETFL");  
#endif
     ;
    }
	

    if(isatty(ComFd) == 0){  
#ifdef SERIAL_PORT_OWN_DEBUG  		
        perror("isatty is not a terminal device"); 
#endif	
       ;

    }  

    stat = 1;  
      
      
    struct termios new_cfg, old_cfg;  
    int speed_arry[]= {B2400, B4800, B9600, B19200, B38400, B115200};  
    int speed[]={2400,4800,9600,19200,38400,115200};  
    int i = 0;   
      
    /*save and test the serial port*/
	
    if(tcgetattr(ComFd, &old_cfg) < 0){  
#ifdef SERIAL_PORT_OWN_DEBUG		
        perror("tcgetattr");
#endif	

        return -1;  
    } 

  
    new_cfg = old_cfg;  
    cfmakeraw(&new_cfg);      
    new_cfg.c_cflag &= ~ CSIZE;      
      
      
    for(i = sizeof(speed_arry) / sizeof(speed_arry[0]); i > 0; i--)  
    {  
        if(baud == speed[i]){  
            cfsetispeed(&new_cfg,speed_arry[i]);  
            cfsetospeed(&new_cfg,speed_arry[i]);  
        }  
    }  
  
  
    switch(databits)     
    {  
        case '7':  
                new_cfg.c_cflag |= CS7;  
                break;  
                  
        default:  
        case '8':  
                new_cfg.c_cflag |= CS8;  
                break;  
    }  
      
    switch(parity)  
    {  
        default:  
        case 'N':  
        case 'n':  
        {  
            new_cfg.c_cflag &= ~PARENB;      
            new_cfg.c_iflag &= ~INPCK;       
        }  
        break;  
  
        case 'o':  
        case 'O':  
        {  
            new_cfg.c_cflag |= (PARODD | PARENB);   
            new_cfg.c_iflag |= INPCK;       
        }  
        break;  
  
        case 'e':  
        case 'E':  
        {  
            new_cfg.c_cflag |= PARENB;  
            new_cfg.c_cflag &= ~PARODD;      
            new_cfg.c_iflag |= INPCK;  
        }  
        break;  
  
        case 's':  
        case 'S':  
        {  
            new_cfg.c_cflag &= ~PARENB;  
            new_cfg.c_cflag &= ~CSTOPB;  
        }  
        break;  
    }  
  
    switch(stopbits)  
    {  
        default:  
        case '1':  
        {  
            new_cfg.c_cflag &= ~CSTOPB;  
        }  
        break;  
  
        case '2':  
        {  
            new_cfg.c_cflag |= CSTOPB;  
        }  
        break;  
    }  
  
    /*set wait time*/  
    new_cfg.c_cc[VTIME] = 0;  
    new_cfg.c_cc[VMIN]  = 1;  
  
    tcflush(ComFd, TCIFLUSH);    
    if((tcsetattr(ComFd, TCSANOW, &new_cfg)) < 0)  
    { 
#ifdef SERIAL_PORT_OWN_DEBUG    
        perror("tcsetattr"); 
#endif
        return -1;  
    }  
#ifdef SERIAL_PORT_OWN_DEBUG	
    printf("Open and Set serial ok,%d\n",ComFd);  
#endif
    return ComFd;     
}  
  
int Serialcom::SerialcomClose()  
{  
    close(ComFd);  
    stat = 0;  
    return stat;  
}     
      
/*> 0 有文件描述符可读， =0 超时 < 0 无描述符可读*/  
int Serialcom::CheckSerialcomData()  
{  
    fd_set rset;  
    struct timeval  tv;  
    int ret;  
  
    FD_ZERO(&rset);  
    FD_SET(ComFd, &rset);  
  
   // tv.tv_sec= usec;  
    //tv.tv_usec = 0;  
    //modify by xiaomeidi
     tv.tv_sec= 0;  
     tv.tv_usec =usec;  
  
    ret = select(ComFd+1, &rset, NULL, NULL, &tv);  
#ifdef SERIAL_PORT_OWN_DEBUG
    if (ret>0)  
        printf("Check have data=%d\n",ret);  
#endif	
    return ret;  
}  
  
int Serialcom::SerialcomRead(char* pbuffer,int dataLen)  
{  
    int ret;  
    ret = read(ComFd,pbuffer,dataLen);  
      
    return ret;  
}     
  
int Serialcom::SerialcomWrite(char* pbuffer,int dataLen)  
{  
    int ret;  
    ret = write(ComFd,pbuffer,dataLen);  
#ifdef SERIAL_PORT_OWN_DEBUG	
    if(ret > 0)    
        printf("SerialcomWrite pbuffer = %s\n",pbuffer); 
#endif	
    return ret;   
}  

 
 

