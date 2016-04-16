//�������һ�����ڴ��ڲ�����C++���룬���������л���Ϊlinux
#ifndef _SERIALCOM_H_  
#define _SERIALCOM_H_  

  
typedef struct _serialconfig_{  
long baud;/*������:��38400,115200��*/  
unsigned char databits;/*����λ0-8λ;1-7λ;2-6λ;3-5λ;*/  
unsigned char parity;/*��żУ��λ:0-����żУ��;1-����Ϊ��Ч��;2-ת��ΪżЧ��*/  
unsigned char stopbits;/*ֹͣλ:0--1λֹͣλ;1--2λֹͣλ*/  
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
    int ComFd;  /*����������*/  
    int stat;  
    long baud;/*������:��38400,115200��*/  
    unsigned char databits;/*����λ0-8λ;1-7λ;2-6λ;3-5λ;*/  
    unsigned char parity;/*��żУ��λ:0-����żУ��;1-����Ϊ��Ч��;2-ת��ΪżЧ��*/  
    unsigned char stopbits;/*ֹͣλ:0--1λֹͣλ;1--2λֹͣλ*/  
    int usec;  
    int port;   /*0,1,2;only 3 choice*/ 
};
#endif  
 

