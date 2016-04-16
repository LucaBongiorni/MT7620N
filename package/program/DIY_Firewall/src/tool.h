#ifndef _TOOL_H
#define _TOOL_H

#ifndef __LINUX_KERNEL__
#define printk  printf
#include <stdbool.h>
#endif

#define IFNAMESIZ  18

#define PRINT_BUG

/* ��ӡip ��ַ */
#define NIPQUAD(addr) ((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3]

#ifdef  PRINT_BUG
#define write_log(fmt,args...) printk("%s %d %s "fmt "\n",__FILE__,__LINE__,__FUNCTION__,##args)
#else
#define write_log
#endif 

#undef  write_error_log
#define write_error_log(fmt,args...) printk("%s %d %s "fmt "\n",__FILE__,__LINE__,__FUNCTION__,##args)
#undef  write_normal_log
#define write_normal_log(fmt,args...) printk("%s %d %s "fmt "\n",__FILE__,__LINE__,__FUNCTION__,##args)

#define print_ip(ip) printk("%d.%d.%d.%d\n", NIPQUAD(ip))



/*�ж����� ip �Ƿ�Ϊͬ������*/
#define IS_SAME_NET(ipS, ipD, mask) (((ipS)&(mask))==((ipD)&(mask)))
/*�� 16 ���Ƶ���ʽ��ӡ�ַ�����ÿ�д�ӡ16��*/
void print_hex(const unsigned char* buf, int len);
/* ��ӡ�ַ���ǰ N λ*/
void print_nstr(const char* buf, int len);



static inline char* inet_ntoa_(unsigned int ip, char *buf)
{
    unsigned char *tmp = (unsigned char *)&ip;
	sprintf(buf, "%d.%d.%d.%d", 
		         tmp[0] & 0xff, 
		         tmp[1] & 0xff,
                 tmp[2] & 0xff, 
                 tmp[3] & 0xff);
	return buf;
}

/* ���ַ���IP ת��������IP */
// in_aton(const char *str)  #include <linux/inet.h>


// ��������: ��һ���ַ����в�����һ���ַ���
// ��������: buf,
//           find,
//           len,
// �� �� ֵ: 
char* nstrstr(const char* buf, const char* find, int len);


// ��������: ��׼����ת��Ϊdns����
// ��������: domain�������������׼����
//           dnsDomain�����������dns����
// �� �� ֵ: �ɹ�����0��ʧ�ܷ���-1��
int stdDomain2DnsDomain(const char* domain, char* dnsDomain);

// ��������: dns����ת��Ϊ��׼����
// ��������: dnsDomain�����������dns���� 
//           domain�������������׼����
// �� �� ֵ: �ɹ�����0��ʧ�ܷ���-1��
int DnsDomain2stdDomain(const char* dnsDomain, char* domain);



#ifndef __LINUX_KERNEL__
// ��������: ִ��shell����������
// ��������: cmd_line��������
//           quiet���Ƿ��ӡ������Ϣ��1��ӡ��0����ӡ
// �� �� ֵ: �������� 0
int execute(char *cmd_line, int quiet);

// ��������: ִ��iptables����
// ��������: format,... �ɱ������������printf�Ĳ���
// �� �� ֵ: �ɹ�����
int iptablesCommand(const char *format, ...);


unsigned int getIPByIfname(const char *ifname);

char* getMacByIfName(const char *ifname);

int getIPByDomain(const char* pDomain, unsigned int *lAddr);



#else
// ��������: "ͨ���豸���ֻ�ȡ�����豸�Ļ�����Ϣ
// ��������: pIfname: �����������������
//           lanIP: �������������ip
//           mask:  �����������������
// �� �� ֵ: �ɹ�����true��ʧ�ܷ���false
bool getDevInfoByIfname(const char* p_ifname, unsigned int *lanip, unsigned int *mask);

/* ������ip ת�����ַ���*/
const char* inet_ntoa(unsigned int ip);

#endif





#define MARK_PKG_DNS_REQUEST          0x00010000
#define MARK_PKG_HTTP_GET_REQUEST     0x00020000




#endif /*_TOOL_H */

