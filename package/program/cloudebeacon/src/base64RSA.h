#ifndef _ENCRYPT_RSA_H_
#define _ENCRYPT_RSA_H_

/******************************************************************************/
#include <string>
#include <openssl/rsa.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/bn.h>
#include <assert.h>
/******************************************************************************/

#if 0
// 公钥和私钥的生成方法(linux 命令):
// 安装 openssl 库
1、生成私钥:
openssl genrsa -out private.key 1024         #没有对私钥进行加密
openssl genrsa -des3 -out private.key 1024   #会对私钥进行des3加密，会提示输入密码
根据私钥生成公钥:
openssl rsa -in private.key -pubout > public.key
转化成PKCS#8编码编码，供Java解密时，进行调用
openssl pkcs8 -topk8 -in private.key -out pkcs8_private.key -nocrypt
去除私钥的密码
openssl rsa -in private.key -out private.key
编译的时候加上 -lssl -lcrypto
#endif


#define BYTE    unsigned char
#define WORD    unsigned short
#define DWORD   unsigned long
#undef  write_log
#define write_log(fmt,args...) printf("[%s][%d]"fmt"\n", __FILE__,__LINE__,##args)

#ifndef u_int8
#define u_int8 unsigned char
#endif
#ifndef u_int16
#define u_int16 unsigned short
#endif
#ifndef u_int32
#define u_int32 unsigned int
#endif
#ifndef u_int64
#define u_int64 unsigned long long
#endif

#undef  u32tLen
#define u32tLen     (sizeof(u_int32))
#undef  u16tLen
#define u16tLen     (sizeof(u_int16))
#undef  u8tLen
#define u8tLen      (sizeof(u_int8))

#ifndef ASSERT
#define ASSERT      assert
#endif

int base64_decode(const char *src, unsigned char* dest, int max_size);
int base64_encode(const char *src, unsigned char* dest, int max_size);

// Base64操作类
class CBase64Operate
{
protected:
    // Base64 解码表
    static const char table64[];
    // 未知函数
    void decodeQuantum(u_int8 *dest, const char *src) const;
public:
    // 构造函数
    CBase64Operate();
    virtual ~CBase64Operate();
    // 加密
    // 输入: szData 内容
    //       dwLen  长度
    // 输出: sOData 输出缓存
    virtual bool Encrypt(const char szData[], DWORD dwLen, std::string &sOData);
    // 加密
    // 输入: szData 内容
    //       dwLen  长度，如果为0，使用strlen
    //       dwOLen 输出缓存长度，无用
    // 输出: szOData 输出缓存
    //       dwOLen 实际长度
    virtual bool Encrypt(const char szData[], DWORD dwLen, char szOData[], DWORD &dwOLen);
    // 解密
    // 输入: szData 内容
    //       dwLen  长度
    // 输出: sOData 输出缓存
    virtual bool Decrypt(const char szData[], DWORD dwLen, std::string &sOData);
};


// EVP rsa 加密方式
class EVPRSA
{
public:
    // 函数功能: 构造函数
    // 函数参数: pwd，字符串，任意字符串都可以，pwdLen，字符串长度
    // 返 回 值: 成功返回0，失败-1
    EVPRSA(const u_int8* pwd, int pwdLen);
    // 函数功能: 构造函数，无密码模式
    EVPRSA();
    // 函数功能: 析构函数
    ~EVPRSA();
public:
    // 函数功能: 生成密匙文件
    // 函数参数: pubKeyFile，公钥文件路径；priKeyFile，私钥文件路径
    //           length，密匙长度
    // 返 回 值: 成功返回0，失败-1
    int generateKeyFile(const char *pubKeyFile, const char *priKeyFile, int length);
    // 函数功能: 打开公钥
    // 函数参数: keyfile，公钥文件路径
    // 返 回 值: 成功返回0，失败-1
    int openPublicKey(const char *keyfile);
    // 函数功能: 打开私钥
    // 函数参数: keyfile，私钥文件路径
    // 返 回 值: 成功返回0，失败-1
    int openPrivateKey(const char *keyfile);
    // 函数功能: 使用公钥加密，这种封装格式只适用公钥加密，私钥解密
    // 输入参数: origData，待加密数据；origDataLen，待加密数据长度；
    // 输出参数: encData，加密后的数据；encDataLen，加密后的数据长度
    // 返 回 值: 成功返回0，失败-1
    int rsaKeyEncrypt(const u_int8 *origData, size_t origDataLen,
                      u_int8 *encData, size_t &encDataLen);
    // 函数功能: 使用密钥解密，这种封装格式只适用公钥加密，私钥解密，这里key必须是私钥
    // 输入参数: encData，加密后的数据，encDataLen，加密后的数据长度；
    // 输出参数: origData，解密后的数据，origDataLen，解密后的数据长度；
    // 返 回 值: 成功返回0，失败-1
    int rsaKeyDecrypt(const u_int8 *encData, size_t encDataLen,
                      u_int8 *origData, size_t &origDataLen);
private:
    u_int8* m_passwd;     // 私钥加密密码
    int m_passwdLen;
    EVP_PKEY* m_priEVPKey;
    EVP_PKEY* m_pubEVPKey;
};



// 公钥指数
const u_int8 PUBLIC_EXPONENT_HEX[] =  {0x01, 0x00, 0x01};
// 模数
const u_int8 MODULES_HEX[] = { \
                               0xd8, 0xe3, 0x79, 0x65, 0x93, 0x45, 0x1b, 0x90, 0x76, 0xc1, 0x79, 0xb7, 0xa8, 0x50, \
                               0x85, 0x87, 0x3d, 0xd8, 0xd3, 0x66, 0x47, 0x6e, 0xa0, 0x3f, 0x9e, 0xe7, 0x84, 0x55, \
                               0x4a, 0xff, 0xd2, 0xb9, 0x6e, 0x40, 0x75, 0x5d, 0xe6, 0x55, 0x92, 0xb5, 0x52, 0xd7, \
                               0x13, 0x7d, 0x23, 0xa0, 0xdd, 0xc0, 0xbe, 0xb4, 0x06, 0xe2, 0xc4, 0x2b, 0x67, 0xcb, \
                               0x39, 0x8e, 0x3d, 0xd6, 0xe1, 0xb6, 0x56, 0xcd, 0xf2, 0xa7, 0x4b, 0xb0, 0xb6, 0xf0, \
                               0x23, 0xee, 0xb4, 0xc5, 0xb4, 0x73, 0xa5, 0xec, 0x51, 0x41, 0xa3, 0x48, 0x5b, 0x89, \
                               0xab, 0x2f, 0x92, 0xa8, 0xd0, 0x97, 0xaf, 0xd7, 0x3e, 0x2e, 0x48, 0xe8, 0xc8, 0x62, \
                               0xd9, 0x1f, 0x4d, 0x5c, 0xdf, 0x03, 0xff, 0x0d, 0x5e, 0xfa, 0xc6, 0xf4, 0xcb, 0x3a, \
                               0x1f, 0x90, 0x84, 0x28, 0x97, 0x5c, 0x50, 0xcf, 0xc0, 0xe8, 0x36, 0xa2, 0x2d, 0x1c, \
                               0xcb, 0x15
                             };
// 私钥指数
const u_int8 PRIVATE_EXPONENT_HEX[] = {\
                                       0x98, 0x46, 0x95, 0x8c, 0x10, 0x6a, 0xfb, 0xe0, 0x60, 0xd0, 0x94, 0x21, 0xb3, 0x25, \
                                       0xdd, 0xaa, 0x47, 0x6c, 0xfd, 0x77, 0x98, 0xfd, 0x7d, 0xbb, 0x4d, 0x58, 0xe5, 0x1b, \
                                       0x71, 0x4d, 0xcd, 0xe8, 0x6d, 0x15, 0x0a, 0x92, 0x43, 0xce, 0x9e, 0xf3, 0x79, 0xd7, \
                                       0x11, 0x92, 0xd1, 0xb9, 0xf7, 0x17, 0x85, 0x8d, 0x26, 0x2c, 0x7e, 0x68, 0xaf, 0x36, \
                                       0x0b, 0x11, 0xe8, 0x2e, 0xff, 0x48, 0x23, 0x06, 0xa8, 0xcf, 0x79, 0x51, 0xa3, 0x3a, \
                                       0xce, 0x76, 0xfb, 0xc9, 0x09, 0x15, 0x5e, 0xf1, 0xdc, 0xc0, 0xa1, 0x2c, 0x0c, 0x6b, \
                                       0x39, 0x07, 0x63, 0x1a, 0x1b, 0x2b, 0xaa, 0xcf, 0xbc, 0xe4, 0x54, 0x21, 0xf4, 0x86, \
                                       0x7a, 0x2a, 0x5f, 0x06, 0xaa, 0xbd, 0x74, 0x1e, 0x94, 0xb4, 0xda, 0x34, 0x48, 0x36, \
                                       0xf8, 0x51, 0xfd, 0x3f, 0x1b, 0xea, 0x42, 0x25, 0xfb, 0x2b, 0xbd, 0xc2, 0xb3, 0x1d, \
                                       0x9b, 0x01
                                      };
#define RSA_KEY_LENGTH 1024
static const char rnd_seed[] = "string to make the random number generator initialized";


// 使用rsa直接加密，这种模式可以使用公钥加密，私钥解密，也可以使用私钥加密，公钥解密；
class RSAOperate
{
public:
    RSAOperate();
    ~RSAOperate();

    // 函数功能: 生成私钥指数和模数
    // 函数参数:
    // 返 回 值: 成功返回-1，失败返回0；
    int generateKeyStr(const char* filePath);

    // 函数功能: 初始化参数
    // 函数参数: pubExpd，公钥指数；pubExpdLen，公钥指数长度；
    //           priExpd，私钥指数；priExpdLen，私钥指数长度；
    //           module，模数；moduleLen，模数长度；
    // 返 回 值: 成功返回-1，失败返回0；
    int setParams(const u_int8 *pubExpd = PUBLIC_EXPONENT_HEX, int pubExpdLen = 3,
                  const u_int8 *priExpd = PRIVATE_EXPONENT_HEX,int priExpdLen = 128,
                  const u_int8 *module = MODULES_HEX, int moduleLen = 128);

    // 函数功能:  在一个key中同时打开公钥和私钥，该key既可用作公钥函数，也可用作私钥函数
    // 返 回 值:
    int openPriKeyAndPubKey();
    // 函数功能:
    // 返 回 值:
    int openPriKey();
    // 函数功能:
    // 返 回 值:
    int openPubKey();

    // 函数功能: 使用私钥进行加密
    // 输入参数: in，待加密字符；inLen，待加密字符长度；
    // 输出参数: out，加密后的字符；outLen，加密后字符的缓冲区大小；
    // 返 回 值: 失败返回-1，成功返回加密后的字符长度；
    int priKeyEncrypt(const u_int8 *in, int inLen, u_int8 **out, int &outLen);
    // 函数功能: 使用公钥进行解密
    // 输入参数: in，加密后的字符；inLen，加密后字符的长度；
    // 输出参数: out，解密后的字符；outLen，解密后的字符缓冲区大小；
    // 返 回 值: 失败返回-1，成功返回解密后的字符长度；
    int pubKeyDecrypt(const u_int8 *in, int inLen, u_int8 **out, int &outLen);
    // 函数功能: 使用公钥进行加密
    // 输入参数: in，待加密字符；inLen，待加密字符长度；
    // 输出参数: out，加密后的字符；outLen，加密后字符的缓冲区大小；
    // 返 回 值: 失败返回-1，成功返回加密后的字符长度；
    int pubKeyEncrypt(const u_int8 *in, int inLen, u_int8 **out, int &outLen);
    // 函数功能: 使用私钥进行解密
    // 输入参数: in，加密后的字符；inLen，加密后字符的长度；
    // 输出参数: out，解密后的字符；outLen，解密后的字符缓冲区大小；
    // 返 回 值: 失败返回-1，成功返回解密后的字符长度；
    int priKeyDecrypt(const u_int8 *in, int inLen, u_int8 **out, int &outLen);
    // 函数功能:
    // 函数参数:
    // 返 回 值:
    int closeKey();

protected:
    // 函数功能: 打开key
    // 函数参数:
    // 返 回 值:
    void freeRes();

private:
    RSA *m_pubKey;
    RSA *m_priKey;

    u_int8 *m_pubExpd;
    u_int8 *m_priExpd;
    u_int8 *m_module;

    int m_pubExpdLen;
    int m_priExpdLen;
    int m_moduleLen;
};


#if 0
// 函数功能: 生成密匙文件
// 函数参数: pubKeyFile，公钥文件路径；priKeyFile，私钥文件路径
//           length，密匙长度
// 返 回 值: 成功返回0，失败-1
int generateKeyFile(const char *pubKeyFile, const char *priKeyFile, int length);
// 函数功能: 打开公钥
// 函数参数: keyfile，公钥文件路径
// 返 回 值: 成功返回0，失败-1
int openPublicKey(const char *keyfile);
// 函数功能: 打开私钥
// 函数参数: keyfile，私钥文件路径
// 返 回 值: 成功返回0，失败-1
int openPrivateKey(const char *keyfile);
// 函数功能: 使用公钥加密，这种封装格式只适用公钥加密，私钥解密
// 输入参数: origData，待加密数据；origDataLen，待加密数据长度；
// 输出参数: encData，加密后的数据；encDataLen，加密后的数据长度
// 返 回 值: 成功返回0，失败-1
int rsaKeyEncrypt(const u_int8 *origData, size_t origDataLen,
                  u_int8 *encData, size_t &encDataLen);
// 函数功能: 使用密钥解密，这种封装格式只适用公钥加密，私钥解密，这里key必须是私钥
// 输入参数: encData，加密后的数据，encDataLen，加密后的数据长度；
// 输出参数: origData，解密后的数据，origDataLen，解密后的数据长度；
// 返 回 值: 成功返回0，失败-1
int rsaKeyDecrypt(const u_int8 *encData, size_t encDataLen,
                  u_int8 *origData, size_t &origDataLen);
#endif

#endif

