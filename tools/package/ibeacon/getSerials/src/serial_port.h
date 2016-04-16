//下面给出一个关于串口操作的C++代码，本代码运行环境为linux
#ifndef _SERIALCOM_H_  
#define _SERIALCOM_H_  

  
typedef struct _serialconfig_{  
long baud;/*波特率:如38400,115200等*/  
unsigned char databits;/*数据位0-8位;1-7位;2-6位;3-5位;*/  
unsigned char parity;/*奇偶校验位:0-无奇偶校验;1-设置为奇效验;2-转换为偶效验*/  
unsigned char stopbits;/*停止位:0--1位停止位;1--2位停止位*/  
}serialconfig_t;  
  
class Serialcom  
{  
public:  
    Serialcom();  
    ~Serialcom();  
	void SetSerialcom(serialconfig_t serialconf,int delay,int choice);  
    int SerialcomOpen();  
    int SerialcomClose();  
    int CheckSerialcomData();  
    int SerialcomRead(char* pbuffer,int dataLen);  
    int SerialcomWrite(char* pbuffer,int dataLen);  
      
protected:  
    int ComFd;  /*串口描述符*/  
    int stat;  
    long baud;/*波特率:如38400,115200等*/  
    unsigned char databits;/*数据位0-8位;1-7位;2-6位;3-5位;*/  
    unsigned char parity;/*奇偶校验位:0-无奇偶校验;1-设置为奇效验;2-转换为偶效验*/  
    unsigned char stopbits;/*停止位:0--1位停止位;1--2位停止位*/  
    int usec;  
    int port;   /*0,1,2;only 3 choice*/ 
};
#endif  
 

