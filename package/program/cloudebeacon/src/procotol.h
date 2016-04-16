#ifndef __PROCOTOL_H__
#define __PROCOTOL_H__

//#include "netSockTcp.h"

#ifndef u_int8
#define u_int8 unsigned char
#endif
#ifndef u_int16
#define u_int16 unsigned short
#endif
#ifndef u_int32
#define u_int32 unsigned int
#endif


// 包头和包尾标志
const int AddLength = 4;
static const char HEAD[2] = {0x02, 0x00};
#define HEAD_LEN  (sizeof(HEAD))
static const char TAIL = 0x03;
#define TAIL_LEN  (sizeof(TAIL))


// 除去消息体，默认的协议头长度，可长不可短
#define DEF_PRO_HEAD_LEN            32
#define EVERY_PKG_LEN               1380

//#define MAC_SERVER         1
//#define BEACON_SERVER      2


class Procotol
{
public:
    Procotol(){};
    ~Procotol(){};
public:
    // 函数功能: unsigned int十六进制转字符
    // 函数参数: hex, 输入参数，unsigned int 类型的数字
    //           str, 输出参数，转换成字符，8个字节
    static void
    Uint32HexTostr(u_int32 hex, char* str);
    // 函数功能: 字符转unsigned int十六进制，转8个字节
    // 函数参数: hex, 输出参数，unsigned int 类型的数字
    //           str, 输入参数，16进制数字字符
    // 返 回 值: 成功返回0，失败返回 -1
    static int
    StrToUint32Hex(u_int32* hex, const char* str);

    // 函数功能: unsigned short十六进制转字符
    // 函数参数: hex unsigned short 类型的数字
    //           str 输出参数，转换成字符，4个字节
    static void
    Uint16HexToStr(u_int16 hex, char *str);

    // 函数功能: 字符转unsigned short十六进制，转四个字节
    // 函数参数: hex 输出参数，转换成的整数
    //           str 输入的字符
    // 返 回 值: 成功返回0，失败返回 -1
    static int
    StrToUint16Hex(u_int16* hex, const char* str);

    // 函数功能: unsigned char十六进制转字符，转1个字节
    // 函数参数: hex unsigned char 类型的数字
    //           str 输出参数，转换成字符，2个字节
    static void
    Uint8HexToStr(u_int8 hex, char *str);

    // 函数功能: 字符转十六进制，转2个字节
    // 函数参数: hex 输出参数，转换成的整数
    //           str 输入的字符
    // 返 回 值: 成功返回0，失败返回 -1
    static int
    StrToUint8Hex(u_int8* hex, const char* str);

    // 函数功能: 加包头和包尾
    // 函数参数: content 输入的字符，加包头后的字符也通过该参数输出
    //           contentLen 输入的字符长度
    //           typeID 数据类型
    //           PkgTotal 发送的总包数
    //           PkgSeq 发送第几个包
    // 返 回 值: 成功返回加包头后的字符长度，失败返回-1
    static int
    AddPkgHeadAndTail(char* content, u_int16 contentLen, u_int16 typeID, char PkgTotal, char PkgSeq);
    // 函数功能: 加包头和包尾
    // 函数参数: content 输入的字符，加包头后的字符也通过该参数输出
    //           contentLen 输入的字符长度
    //           typeID 数据类型
    //           PkgTotal 发送的总包数
    //           PkgSeq 发送第几个包
    // 返 回 值: 成功返回加包头后的字符长度，失败返回-1
    static int
    NetAddPkgHeadAndTail(char* content, u_int16 contentLen, u_int16 typeID, char PkgTotal, char PkgSeq);
    // 函数功能: 去包头和包尾
    // 函数参数: content 输入的字符，去包头后的字符也通过该参数输出
    //           contentLen 输入的字符长度
    //           typeID 输出参数，数据包类型
    //           PkgTotal 输出参数，总共多少个包
    //           PkgSeq 输出参数，第几个包
    // 返 回 值: 成功返回去掉包头和包尾的字符长度，失败返回-1
    static int
    DelPkgHeadAndTail(char* content, int contentLen, u_int16* typeID, u_int8* PkgTotal, u_int8* PkgSeq);
    // 函数功能: 检查包头是否正确，并返回消息体长度
    // 函数参数: content，输入参数，包头数据
    //           length，输出参数，消息体长度
    // 返 回 值: 成功返回0， 失败返回-1；
    static int
    CheckPkgHead(const char* content, u_int16* length);
    // 函数功能: 检查包头是否正确，并返回消息体长度
    // 函数参数: content，输入参数，包头数据
    //           length，输出参数，消息体长度
    // 返 回 值: 成功返回0， 失败返回-1；
    static int
    NetCheckPkgHead(const char* content, u_int16* length);

    // 函数功能: 以十六进制打印字符
    // 函数参数: buf 输入打印的字符
    //           len 打印的长度
    static void
    printHex(const char* buf, int len);

	// 函数功能: 字符转16进制
	// 函数参数: ascii, 字符，
	//           hex，输出参数，转换后的16进制
	//           len，转换长度
	// 返 回 值: 0，成功，-1，失败
	static int 
	ASCIIToHex(u_int8* ascii, u_int8* hex, int len);

	// 函数功能: 16进制转字符
	// 函数参数: phex,
	//           pascii,
	//           len,
	// 返 回 值: 
	static void 
	HexToASCII(u_int8* phex, u_int8* pascii, int len);

    // 函数功能: 组装广播包消息体
    // 返 回 值: 成功返回组装后的消息体，失败返回 NULL;
    // static char* MakeBcastMes();
    // 函数功能: 向服务器发送mac地址信息
    // 函数参数: macInfo: 输入参数: 从proc文件读取到的数据内容
    //           outLen: 输出参数: 返回数据长度；
    // 返 回 值: 返回组装后的数据，使用之后需要释放，失败返回NULL
    // static char* MakeUploadMacInfo(char* macInfo, int& outLen);
    // static int RecvOnePkg(int sockFD, char* recvBuff, int buffLen, int timeOut);
};


#if 0
void (*pUint32HexToStr)(u_int32, char*) = Procotol::Uint32HexTostr;
int  (*pStrToUint32Hex)(u_int32*, const char*) = Procotol::StrToUint32Hex;
void (*pUint16HexToStr)(u_int16, char *) = Procotol::Uint16HexToStr;
int  (*pStrToUint16Hex)(u_int16*, const char*) = Procotol::StrToUint16Hex;
void (*pUint8HexToStr)(u_int16, char *) = Procotol::Uint8HexToStr;
int  (*pStrToUint8Hex)(u_int16*, const char*) = Procotol::StrToUint8Hex;
int  (*pAddPkgHeadAndTail)(char*, u_int16, u_int16, char, char) = Procotol::AddPkgHeadAndTail;
int  (*pDelPkgHeadAndTail)(char*, int, u_int16*, u_int8*, u_int8*) = Procotol::DelPkgHeadAndTail;
int  (*pCheckPkgHead)(const char*, u_int16*) = Procotol::CheckPkgHead;
void (*pPrintHex)(const char*, int) = Procotol::printHex;
#endif


#define SuccessJSON       "{\"returnVal\": 0}"
#define FailedJSON        "{\"returnVal\": 1}"
#define BeenBinded        "{\"returnVal\": 2}"
#define UnBinded          "{\"returnVal\": 2}"


#define RETURN_JSON_0     "{\"returnVal\": 0}"
#define RETURN_JSON_1     "{\"returnVal\": 1}"
#define RETURN_JSON_2     "{\"returnVal\": 2}"
#define RETURN_JSON_3     "{\"returnVal\": 3}"
#define RETURN_JSON_4     "{\"returnVal\": 4}"
#define RETURN_JSON_5     "{\"returnVal\": 5}"
#define RETURN_JSON_6     "{\"returnVal\": 6}"
#define RETURN_JSON_7     "{\"returnVal\": 7}"

// 串口协议
#define COM_STARDARD_RETURN          0x0101      // 通用返回
#define COM_RST_SCAN_DEV             0x0102      // 复位扫描模块
#define COM_BEG_SCAN_DEV             0x0103      // 开始扫描设备
#define COM_STP_SCAN_DEV             0x0104      // 停止扫描设备
#define COM_REP_SCAN_INFO            0x0105      // 上报扫描结果
#define COM_BLD_DEV_CONNECT          0x0106      // 建立设备连接
#define COM_BRK_DEV_CONNECT          0x0107      // 断开连接
#define COM_SET_SERVER_PARAM         0x0108      // 设置服务参数
#define COM_FIND_SERVER_PARAM        0x0109      // 查询服务参数
#define COM_REP_SERVER_PARAM         0x010a      // 上报服务参数
#define COM_FIND_BLU_INFO            0x010b      // 查询模块信息
#define COM_UPL_BLU_INFO             0x010c      // 上报模块信息


// 网络协议
#define MES_RST_BLU_DEV              0x0101      // 复位蓝牙模块
#define MES_RST_BLU_DEV_ACK          0x0102      // 复位蓝牙模块回复
#define MES_BEG_SCAN_DEV             0x0103      // 开始扫描设备
#define MES_BEG_SCAN_DEV_ACK         0x0104      // 回复开始扫描设备
#define MES_STP_SCAN_DEV             0x0105      // 停止扫描设备
#define MES_STP_SCAN_DEV_ACK         0x0106      // 回复停止扫描设备
#define MES_REP_SCAN_INFO            0x0107      // 上报扫描结果
#define MES_SET_DEV_PARAM            0x0108      // 设置设备参数
#define MES_SET_DEV_PARAM_ACK        0x0109      // 回复设置设备参数
#define MES_QUR_DEV_PARAM            0x010a      // 查询设备参数
#define MES_QUR_DEV_PARAM_ACK        0x010b      // 查询设备参数应答
#define MES_QUR_BLU_INFO             0x010c      // 查询蓝牙信息
#define MES_QUR_BLU_INFO_ACK         0x010d      // 上报蓝牙信息
#define MES_SET_BLU_INFO             0x010e      // 设置蓝牙信息
#define MES_SET_BLU_INFO_ACK         0x010f      // 设置蓝牙信息应答


#define MES_BCT_GET_CON_INFO         0x2fff      // 广播获取配置信息
#define MES_SND_HEART                0x1000      // 发送心跳包
#define MES_SND_HEART_ACK            0x1000      // 心跳包确认
#define MES_APY_PUB_KEY              0x1001      // 向服务器请求公钥
#define MES_APY_PUB_KEY_ACK          0x1002      // 服务器返回公钥
#define MES_SND_UID_TO_SER           0x1003      // 向服务器发送身份
#define MES_SND_UID_TO_SER_ACK       0x1004      // 服务器返回验证结果
#define MES_SER_GET_CBC_INFO         0x1005      // 服务器获取CloudBeacon设备的基本参数
#define MES_SER_GET_CBC_INFO_ACK     0x1006      // 向服务器返回CloudBeacon设备的基本参数
#define MES_UPL_PHO_MAC_INFO         0x1007      // 上传 WiFi 扫描到的 MAC 地址
#define MES_UPL_PHO_MAC_INFO_ACK     0x1008      // 服务器返回
#define MES_SET_SER_INFO             0x1009      // 设置客户Mac服务器基本信息
#define MES_SET_SER_INFO_ACK         0x100a      // 返回设置客户Mac服务器基本信息结果
#define MES_GET_SER_INFO             0x100b      // 获取客户Beacon服务器基本信息
#define MES_GET_SER_INFO_ACK         0x100c      // 返回获取客户Beacon服务器基本信息结果
#define MES_SET_UPL_INTERVAL         0x100d      // 配置tcp上传mac地址的间隔时间和上传beacon的间隔时间
#define MES_SET_UPL_INTERVAL_ACK     0x100e      // 返回配置tcp上传mac地址的间隔时间和上传beacon的间隔时间

#define MES_VFY_KEY_SER              0x100f      // 验证手机key 
#define MES_VFY_KEY_SER_ACK          0x1010      // 返回验证手机key结果
#define MES_APL_PHO_BIND_CBC         0x1011      // 手机账户绑定cloudbeacon设备
#define MES_APL_PHO_BIND_CBC_ACK     0x1012      // 返回手机账户绑定cloudbeacon设备的结果
#define MES_APL_PHO_UNBIND_CBC       0x1013      // 手机账户解绑cloudbeacon设备
#define MES_APL_PHO_UNBIND_CBC_ACK   0x1014      // 返回手机账户解绑cloudbeacon设备的结果
#define MES_SYN_CBC_CONF             0x1015      // 同步手机配置cloudbeacon的信息
#define MES_SYN_CBC_CONF_ACK         0x1016      // 返回同步结果
#define MES_UPL_BLU_CONF_FILE        0x1017      // 上传蓝牙配置文件
#define MES_UPL_BLU_CONF_FILE_ACK    0x1018      // 确认上传蓝牙配置文件
#define MES_UPD_BLU_CONF_FILE        0x1019      // 更新蓝牙配置文件
#define MES_UPD_BLU_CONF_FILE_ACK    0x101a      // 确认更新蓝牙配置文件
#define MES_UPDATE_BLU_BIN           0x101b      // 更新蓝牙固件
#define MES_UPDATE_BLU_BIN_ACK       0x101c      // 返回更新蓝牙配置文件信息
#define MES_UPDATE_CB_PRO            0x101d      // 更新cloudbeacon程序
#define MES_UPDATE_CB_PRO_ACK        0x101e      // 更新cloudbeacon程序返回值




#define MES_PHO_GET_CBC_INFO         0x2000      // 获取配置信息
#define MES_PHO_GET_CBC_INFO_ACK     0x2001      // 返回获取配置信息结果
#define MES_PHO_SET_CBC_INFO         0x2002      // 设置信息
#define MES_PHO_SET_CBC_INFO_ACK     0x2003      // 返回设置信息结果
#define MES_PHO_SND_KEY              0x2004      // 手机发送key
#define MES_PHO_SND_KEY_ACK          0x2005      // 返回
#define MES_PHO_BIND_CBC             0x2006      // 手机绑定设备
#define MES_PHO_BIND_CBC_ACK         0x2007      // 返回绑定设备结果
#define MES_PHO_UNBIND_CBC           0x2008      // 解绑设备
#define MES_PHO_UNBIND_CBC_ACK       0x2009      // 绑定设备
#define MES_GET_CB_IFNET_STA         0x200a      // 获取cb的网卡信息，类似ifconfig
#define MES_GET_CB_IFNET_STA_ACK     0x200b      // 返回获取cb的网卡信息；
#define MES_SET_CB_CON_BY_RTC        0x200c      // 设置cb使用网线上网
#define MES_SET_CB_CON_BY_RTC_ACK    0x200d      // 返回设置cb使用网线上网
#define MES_CHE_WIFI_AP              0x200e      // 搜寻wifi热点
#define MES_CHE_WIFI_AP_ACK          0x200f      // 返回搜寻wifi热点结果
#define MES_SET_WIFI_CONF            0x2010      // 配置wifi基本信息
#define MES_SET_WIFI_CONF_ACK        0x2011      // 返回配置wifi基本信息结果
#define MES_SET_ROOT_PASSWD          0x2012      // 设置密码
#define MES_SET_ROOT_PASSWD_ACK      0x2013      // 返回设置密码结果
#define MES_CHK_SD_INFO              0x2014      // 查看U盘挂载情况
#define MES_CHK_SD_INFO_ACK          0x2015      // 返回查看结果
#define MES_CHK_MEM_FLASH_SYS        0x2016      // 查看内存、flash、系统信息
#define MES_CHK_MEM_FLASH_SYS_ACK    0x2017      // 返回查看内存、flash、系统信息
#define MES_CHK_ARP_INFO             0x2018      // 查看 arp 列表信息
#define MES_CHK_ARP_INFO_ACK         0x2019      // 返回查看到的相关信息
#define MES_CHK_DHCP_INFO            0x201a      // 查看dhcp列表信息
#define MES_CHK_DHCP_INFO_ACK        0x201b      // 返回查看dhcp列表信息
#define MES_SET_3G_NET               0x201c      // 设置3g上网
#define MES_SET_3G_NET_ACK           0x201d      // 返回设置3g上网
#define MES_CHK_PS_INFO              0x201e      // 查看正在运行的程序
#define MES_CHK_PS_INFO_ACK          0x201f      // 返回查看正在运行的程序的结果
#define MES_GET_CB_LOG               0x2020      // 获取cloudbeacon运行日记
#define MES_GET_CB_LOG_ACK           0x2021      // 返回获取cloudbeacon运行日记的结构
#define MES_CHK_CB_NET_STA           0x2022      // 诊断cloudbeacon的网络状态
#define MES_CHK_CB_NET_STA_ACK       0x2023      // 返回诊断cloudbeacon的网络状态


#define MES_UNBIND_CB                0x20fc
#define MES_UNBIND_CB_ACK            0x20fd
#define MES_GET_CB_ALL_INFO          0x20fe
#define MES_GET_CB_ALL_INFO_ACK      0x20ff



#endif /*__PROCOTOL_H__*/


