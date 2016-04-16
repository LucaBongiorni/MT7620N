#include <iostream>
#include <memory>
#include <string>
#include <stdlib.h>

#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/bn.h>

#include "base64RSA.h"


int decode_base64_char(unsigned int code)
{
	if (code >= 'A' && code <= 'Z')
		return (int)(code - 'A');
	if (code >= 'a' && code <= 'z')
		return (int)((code - 'a') + 26);
	if (code >= '0' && code <= '9')
		return (int)((code - '0') + 52);
	if (code == '+')
		return 62;
	if (code == '/')
		return 63;
	return 64;
}

int base64_decode(const char *src, unsigned char* dest, int max_size)
{
	int size = 0;
	int nfrom = 0;
	unsigned char ch_high_bits = 0;
	if(!src)
	{
		return -1;
	}
	while (*src)
	{
		if (size >= max_size)
			break;

		unsigned char ch = *src++;
		if (ch == '\r' || ch == '\n')
			continue;
		ch = (unsigned char) decode_base64_char(ch);
		if (ch >= 64)
			break;

		switch ( (nfrom++) % 4 )
		{
		case 0:
			ch_high_bits = ch << 2;
			break;

		case 1:
			dest[size++] = ch_high_bits | (ch >> 4);
			ch_high_bits = ch << 4;
			break;

		case 2:
			dest[size++] = ch_high_bits | (ch >> 2);
			ch_high_bits = ch << 6;
			break;

		default:
			dest[size++] = ch_high_bits | ch;
			break;
		}
	}

	return (size);
}






int base64_encode(const char *src, unsigned char* dest, int max_size)
{
	int size = 0;
	int nfrom;
	unsigned char ch_high4_bits = 0;
	int src_len = (int)strlen( src );		
	static const char* base64_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	for (nfrom=0; nfrom<src_len; nfrom++)
	{
		if (size >= max_size)
			break;

		unsigned char ch = src[nfrom];
		switch (nfrom % 3)
		{
		case 0:
			dest[size++] = base64_table[ch >> 2];
			ch_high4_bits = (ch << 4) & 0x30;
			break;

		case 1:
			dest[size++]= base64_table[ch_high4_bits | (ch >> 4)];
			ch_high4_bits = (ch << 2) & 0x3c;
			break;

		default:
			dest[size++]= base64_table[ch_high4_bits | (ch >> 6)];
			if (size < max_size)
			{
				dest[size++]= base64_table[ch & 0x3f];
			}
		}
		
	}

	if (nfrom % 3 != 0 && size < max_size)	
	{
		dest[size++]= base64_table[ch_high4_bits];
		int npad = (4 - (nfrom % 3)) - 1;
		if (size+npad <= max_size)
		{
			memset(dest+size, '=', (unsigned long)npad);
			size += npad;
		}
	}
	dest[size++]=0;
	return (size);
}


#if 0
#include <stdio.h>
int main()
{
	char buff[] = "aa8964e200f0462483683a72055a87e8";
	char mac[] = "66517E801746";
	int i, j, k;

	j = strlen(buff);
	char temp[128] = {0};
	for (i=0,k=0; i<j; ++i, ++k)
	{
		k = k % 12;
		temp[i] = buff[i] + mac[k];
		printf("%c ", mac[k]);
	}
	printf("\n");

	for (k=0; k<i; ++k)
	{
		printf("%02x ", temp[k]);
	}
	printf("\n");
	
	unsigned char dest[128] = {0};
	base64_encode(temp, dest, 128);
	printf("%s\n", dest);
	return 0;
}
#endif








/******************************************************************************/

const char CBase64Operate::table64[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/******************************************************************************/
CBase64Operate::CBase64Operate()
{
}

/******************************************************************************/
CBase64Operate::~CBase64Operate()
{
}

/******************************************************************************/
bool CBase64Operate::Encrypt(const char szData[], DWORD dwLen,
								std::string &sOData)
{
	ASSERT(szData != NULL);

	unsigned char ibuf[3];
	unsigned char obuf[4];
	int i;
	int inputparts;
	char *output;

	char *indata = (char *)szData;

	if (0 == dwLen)
	{
	    dwLen = strlen(indata);
	}

	DWORD dwELen = (dwLen / 3 + 1) * 4;
	std::auto_ptr<char> autoBuf(new char[dwELen + 1]);

	output = autoBuf.get();

	while (dwLen > 0)
	{
		for (i = inputparts = 0; i < 3; i++)
		{
			if (dwLen > 0)
			{
				inputparts++;
				ibuf[i] = (unsigned char)*indata;
				indata++;
				dwLen--;
			}
			else
			{
				ibuf[i] = 0;
			}
		}

		obuf[0] = (unsigned char)((ibuf[0] & 0xFC) >> 2);
		obuf[1] = (unsigned char)(((ibuf[0] & 0x03) << 4) | \
			((ibuf[1] & 0xF0) >> 4));
		obuf[2] = (unsigned char)(((ibuf[1] & 0x0F) << 2) | \
			((ibuf[2] & 0xC0) >> 6));
		obuf[3] = (unsigned char)(ibuf[2] & 0x3F);

		switch(inputparts)
		{
		case 1: /* only one byte read */
			(void)snprintf(output, 5, "%c%c==",
				table64[obuf[0]],
				table64[obuf[1]]);
			break;

		case 2: /* two bytes read */
			(void)snprintf(output, 5, "%c%c%c=",
				table64[obuf[0]],
				table64[obuf[1]],
				table64[obuf[2]]);
			break;

		default:
			(void)snprintf(output, 5, "%c%c%c%c",
				table64[obuf[0]],
				table64[obuf[1]],
				table64[obuf[2]],
				table64[obuf[3]] );
			break;
		}

		output += 4;
	}

	*output = 0;

	sOData = autoBuf.get();

	return true;
}

/******************************************************************************/
bool CBase64Operate::Encrypt(const char szData[], DWORD dwLen, char szOData[],
							DWORD &dwOLen)
{
	ASSERT(szData != NULL);
	ASSERT(szOData != NULL);

	std::string sOData;
	if (!Encrypt(szData, dwLen, sOData))
	{
		return false;
	}

	if (dwOLen > sOData.length() + 1)
	{
		dwOLen = sOData.length() + 1;
	}

	// 生成结果
	memcpy(szOData, sOData.c_str(), dwOLen);
	dwOLen--;	// 去掉结尾的\0

	return true;
}

/******************************************************************************/
bool CBase64Operate::Decrypt(const char szData[], DWORD dwLen,
								std::string &sOData)
{
	dwLen = dwLen;
	
	ASSERT(szData != NULL);

	int length = 0;
	int equalsTerm = 0;
	int i;
	int numQuantums;
	unsigned char lastQuantum[3];
	size_t rawlen=0;
	unsigned char *newstr;

	while((szData[length] != '=') && szData[length])
		length++;

	/* A maximum of two = padding characters is allowed */
	if(szData[length] == '=')
	{
		equalsTerm++;
		if(szData[length+equalsTerm] == '=')
			equalsTerm++;
	}
	numQuantums = (length + equalsTerm) / 4;

	/* Don't allocate a buffer if the decoded length is 0 */
	if (numQuantums <= 0)
		return false;

	rawlen = (DWORD)((numQuantums * 3) - equalsTerm);

	/* The buffer must be large enough to make room for the last quantum
	(which may be partially thrown out) and the zero terminator. */
	std::auto_ptr<char> autoBuf(new char[rawlen+4]);
	newstr = (BYTE *)autoBuf.get();

	/* Decode all but the last quantum (which may not decode to a
	multiple of 3 bytes) */
	for(i = 0; i < numQuantums - 1; i++)
	{
		decodeQuantum((unsigned char *)newstr, szData);
		newstr += 3; szData += 4;
	}

	/* This final decode may actually read slightly past the end of the buffer
	if the input string is missing pad bytes.  This will almost always be
	harmless. */
	decodeQuantum(lastQuantum, szData);
	for(i = 0; i < 3 - equalsTerm; i++)
		newstr[i] = lastQuantum[i];

	newstr[i] = 0; /* zero terminate */

	(void)sOData.assign(autoBuf.get(),
		(DWORD)(newstr + i - (BYTE *)autoBuf.get()));

	return true;
}

/******************************************************************************/
void CBase64Operate::decodeQuantum(unsigned char *dest,
	const char *src) const
{
	unsigned int x = 0;
	int i;
	for(i = 0; i < 4; i++)
	{
		if(src[i] >= 'A' && src[i] <= 'Z')
		{
			x = (x << 6) + (unsigned char)((src[i] - 'A') + 0);
		}
		else if(src[i] >= 'a' && src[i] <= 'z')
		{
			x = (x << 6) + (unsigned char)((src[i] - 'a') + 26);
		}
		else if(src[i] >= '0' && src[i] <= '9')
		{
			x = (x << 6) + (unsigned char)((src[i] - '0') + 52);
		}
		else if(src[i] == '+')
		{
			x = (x << 6) + 62;
		}
		else if(src[i] == '/')
		{
			x = (x << 6) + 63;
		}
		else if(src[i] == '=')
		{
			x = (x << 6);
		}
	}

	dest[2] = (unsigned char)(x & 255);
	x >>= 8;
	dest[1] = (unsigned char)(x & 255);
	x >>= 8;
	dest[0] = (unsigned char)(x & 255);
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
EVPRSA::EVPRSA(const unsigned char* pwd, int pwdLen)
{
	if (pwd)
	{
		m_passwd = new unsigned char[pwdLen+1];
		memcpy(m_passwd, pwd, pwdLen);
		m_passwd[pwdLen] = 0;
		m_passwdLen = pwdLen;
	}
	else
	{
		m_passwd = NULL;
		m_passwdLen = 0;
	}

	m_priEVPKey = NULL;
	m_pubEVPKey = NULL;
}
EVPRSA::EVPRSA()
{
	m_passwd = NULL;
	m_passwdLen = 0;
	
	m_priEVPKey = NULL;
	m_pubEVPKey = NULL;
}
EVPRSA::~EVPRSA()
{
	if (m_passwd)
	{
		delete [] m_passwd;
		m_passwd = NULL;
	}
	if (NULL == m_priEVPKey)
	{
		EVP_PKEY_free(m_priEVPKey);
		m_priEVPKey = NULL;
	}
	if (NULL == m_pubEVPKey)
	{
		EVP_PKEY_free(m_pubEVPKey);
		m_pubEVPKey = NULL;
	}
}



int EVPRSA::openPublicKey(const char *keyfile)  
{
    RSA *rsa = NULL;  
  
    OpenSSL_add_all_algorithms();  
    BIO *bp = BIO_new(BIO_s_file());;  
    BIO_read_filename(bp, keyfile);  
    if (NULL == bp)  
    {  
        write_log("open_public_key bio file new error!");  
        return -1;  
    }  
  
    //rsa = PEM_read_bio_RSAPublicKey(bp, NULL, NULL, NULL); 
	rsa = PEM_read_bio_RSA_PUBKEY(bp, NULL, NULL, NULL);
	//rsa = PEM_read_RSA_PUBKEY(bp, NULL, NULL, NULL);
    if (rsa == NULL)  
    {  
        write_log("open_public_key failed to PEM_read_bio_RSAPublicKey!");  
        BIO_free(bp);  
        RSA_free(rsa);   
        return -1;  
    }  

    write_log("open_public_key success to PEM_read_bio_RSAPublicKey!");  
    m_pubEVPKey = EVP_PKEY_new();  
    if(NULL == m_pubEVPKey)  
    {  
        write_log("open_public_key EVP_PKEY_new failed.");  
        RSA_free(rsa); 
		BIO_free(bp); 
        return -1;  
    }  
  
    EVP_PKEY_assign_RSA(m_pubEVPKey, rsa); 
	BIO_free(bp); 
    return 0;  
}  

  
int EVPRSA::generateKeyFile(const char *pubKeyFile, const char *priKeyFile, int length)
{
	static const char rnd_seed[] = "make the random number.";
    RSA *rsa = NULL;  
    RAND_seed(rnd_seed, sizeof(rnd_seed));  
    rsa = RSA_generate_key(length, RSA_F4, NULL, NULL);  
    if(rsa == NULL)  
    {  
        write_log("RSA_generate_key error!");  
        return -1;  
    }  
  
    // 开始生成公钥文件  
    BIO *bp = BIO_new(BIO_s_file());  
    if (NULL == bp)  
    {  
        write_log("generate_key bio file new error!\n"); 
		RSA_free(rsa); 
        return -1;  
    }  
  
    if (BIO_write_filename(bp, (void *)pubKeyFile) <= 0)  
    {  
        write_log("BIO_write_filename error!\n");
		RSA_free(rsa); 
		BIO_free_all(bp);
        return -1;  
    }  

#if 0  
    if (PEM_write_bio_RSAPublicKey(bp, rsa) != 1)  
    {  
        write_log("PEM_write_bio_RSAPublicKey error!\n");
		RSA_free(rsa); 
		BIO_free_all(bp);
        return -1;  
    }  
#endif 	
	if (PEM_write_bio_RSA_PUBKEY(bp, rsa) != 1)  
    {  
        write_log("PEM_write_bio_RSAPublicKey error!\n");
		RSA_free(rsa); 
		BIO_free_all(bp);
        return -1;  
    } 
      
    // 公钥文件生成成功，释放资源   
    BIO_free_all(bp);  
  
    // 生成私钥文件  
    bp = BIO_new_file(priKeyFile, "w+");  
	if (NULL == bp)  
    {  
        write_log("generate_key bio file new error2!\n"); 
		RSA_free(rsa); 
        return -1;  
    } 
	
	if (m_passwd)
	{
	    if (PEM_write_bio_RSAPrivateKey(bp, rsa, EVP_des_ede3_ofb(), m_passwd, m_passwdLen, NULL, NULL) != 1)  
	    {  
	        write_log("PEM_write_bio_RSAPublicKey error!\n");
			RSA_free(rsa); 
			BIO_free_all(bp); 
	        return -1;  
	    }  
	}
	else
	{
		if (PEM_write_bio_RSAPrivateKey(bp, rsa, NULL, NULL, 0, NULL, NULL) != 1)  
	    {  
	        write_log("PEM_write_bio_RSAPublicKey error!\n");
			RSA_free(rsa); 
			BIO_free_all(bp); 
	        return -1;  
	    }  
	}
  
    // 释放资源  
    BIO_free_all(bp);  
    RSA_free(rsa);   
    return 0;  
}



// 打开私钥文件，返回EVP_PKEY结构的指针  
int EVPRSA::openPrivateKey(const char *keyfile)  
{   
    RSA *rsa = RSA_new();  
    OpenSSL_add_all_algorithms();  
    BIO *bp = NULL;  
    bp = BIO_new_file(keyfile, "rb");   
    if (NULL == bp)  
    {  
        write_log("open_private_key bio file new error!\n"); 
		RSA_free(rsa);
        return -1;  
    }  
  
    rsa = PEM_read_bio_RSAPrivateKey(bp, &rsa, NULL, (void *)m_passwd);  
    if (rsa == NULL)  
    {  
        write_log("open_private_key failed to PEM_read_bio_RSAPrivateKey!\n");  
        BIO_free(bp);  
        RSA_free(rsa);  
        return -1;  
    }  
    
    m_priEVPKey = EVP_PKEY_new();  
    if (NULL == m_priEVPKey)  
    {  
        write_log("open_private_key EVP_PKEY_new failed\n");  
        RSA_free(rsa);
		BIO_free(bp); 
        return -1;  
    }  
    EVP_PKEY_assign_RSA(m_priEVPKey, rsa);  

	BIO_free(bp);
    return 0;  
}  




int EVPRSA::rsaKeyEncrypt(const unsigned char *origData, size_t origDataLen,   
                    unsigned char *encData, size_t &encDataLen)  
{  
    EVP_PKEY_CTX *ctx = NULL;  
    OpenSSL_add_all_ciphers();  
  
    ctx = EVP_PKEY_CTX_new(m_pubEVPKey, NULL);  
    if (NULL == ctx)  
    {  
        write_log("ras_pubkey_encrypt failed to open ctx.");  
        EVP_PKEY_free(m_pubEVPKey);
		m_pubEVPKey = NULL;
        return -1;  
    }  
  
    if (EVP_PKEY_encrypt_init(ctx) <= 0)  
    {  
        write_log("ras_pubkey_encrypt failed to EVP_PKEY_encrypt_init.");  
        EVP_PKEY_free(m_pubEVPKey);  
		m_pubEVPKey = NULL;
        return -1;  
    }  

  	size_t temp;
	int nRet; 
	nRet = EVP_PKEY_encrypt(ctx, encData, &temp, origData, origDataLen);
    if ( nRet <= 0)  
    {  
        write_log("ras_pubkey_encrypt failed to EVP_PKEY_encrypt. nRet=%d", nRet);  
        EVP_PKEY_CTX_free(ctx);  
        EVP_PKEY_free(m_pubEVPKey); 
		m_pubEVPKey = NULL;
        return -1;  
    }  
	encDataLen = temp;
  
    EVP_PKEY_CTX_free(ctx);  
    EVP_PKEY_free(m_pubEVPKey); 
	m_pubEVPKey = NULL;
    return 0;  
}  



int EVPRSA::rsaKeyDecrypt(const unsigned char *encData, size_t encDataLen,   
                    unsigned char *origData, size_t &origDataLen)  
{  
    EVP_PKEY_CTX *ctx = NULL;  
    OpenSSL_add_all_ciphers(); 
	int nRet;
	size_t temp;
  
    ctx = EVP_PKEY_CTX_new(m_priEVPKey, NULL);  
    if (NULL == ctx)  
    {  
		write_log("ras_prikey_decryptfailed to open ctx.\n");  
		EVP_PKEY_free(m_priEVPKey);  
		return -1;  
    }  

  	nRet = EVP_PKEY_decrypt_init(ctx);
    if (nRet <= 0)  
    {  
        write_log("ras_prikey_decryptfailed to EVP_PKEY_decrypt_init.\n");  
        EVP_PKEY_free(m_priEVPKey);  
        return -1;  
    }  
  	
	nRet = EVP_PKEY_decrypt(ctx, origData, &temp, encData, encDataLen);
    if ( nRet <= 0)  
    {  
        write_log("ras_prikey_decrypt failed to EVP_PKEY_decrypt.");  
        EVP_PKEY_CTX_free(ctx);  
        EVP_PKEY_free(m_priEVPKey);  
        return -1;  
    } 
	origDataLen = temp;
  
    EVP_PKEY_CTX_free(ctx);  
    EVP_PKEY_free(m_priEVPKey);  
	m_priEVPKey = NULL;
    return 0;  
}  



//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
RSAOperate::RSAOperate()  
{  
    m_pubKey = NULL;  
    m_priKey = NULL;  
  
    m_pubExpd = NULL;  
    m_priExpd = NULL;  
    m_module = NULL;  
	m_pubExpdLen = 0;  
    m_priExpdLen = 0;  
    m_moduleLen = 0;  
}  
  
RSAOperate::~RSAOperate()  
{  
    closeKey();  
    freeRes();  
}  
  
// 生成密钥函数  
int RSAOperate::generateKeyStr(const char* filePath)  
{  
    RSA *r = NULL;    
    int bits = RSA_KEY_LENGTH;   
    unsigned long e = RSA_F4;  
  
    r = RSA_generate_key(bits, e, NULL, NULL);    
      
    // 用作显示  
    RSA_print_fp(stdout, r, 11);  
    FILE *fp = fopen(filePath, "w");  
    if(NULL == fp)  
    {
    	write_log("open file:%s failed.", filePath);
        return -1;  
    }  
      
    RSA_print_fp(fp, r, 0);  
    fclose(fp);  
    return 0;  
}  


// 初始化参数  
int RSAOperate::setParams(const unsigned char *pubExpd, int pubExpdLen,   
                       const unsigned char *priExpd, int priExpdLen,  
                       const unsigned char *module,   int moduleLen)  
{  
    if (pubExpd)  
    {  
		m_pubExpdLen = pubExpdLen;  
        m_pubExpd = new unsigned char[pubExpdLen];  
        if(!m_pubExpd)  
        {  
            freeRes();  
            return -1;  
        } 
        memcpy(m_pubExpd, pubExpd, m_pubExpdLen);  
    }  
  
    if (priExpd)  
    {  
        m_priExpdLen = priExpdLen;  
        m_priExpd = new unsigned char[priExpdLen];  
        if(!m_priExpd)  
        {  
            freeRes();  
            return -1;  
        }  
        memcpy(m_priExpd, priExpd, priExpdLen);  
    }  
  
    if (module)  
    {  
        m_moduleLen = moduleLen;  
        m_module = new unsigned char[moduleLen];  
        if(!m_module)  
        {  
            freeRes();  
            return -1;  
        }  
        memcpy(m_module, module, moduleLen);  
    }  
  
    return 0;  
}  
 
int RSAOperate::openPriKeyAndPubKey()  
{  
	//构建RSA数据结构  
    m_priKey = RSA_new();  
    m_priKey->e = BN_bin2bn(m_pubExpd, m_pubExpdLen, m_priKey->e);  
    m_priKey->d = BN_bin2bn(m_priExpd, m_priExpdLen, m_priKey->d);  
    m_priKey->n = BN_bin2bn(m_module, m_moduleLen, m_priKey->n);  
  
    //RSA_print_fp(stdout, m_priKey, 0);  
  
    return 0;  
}  
  
// 打开私钥  
int RSAOperate::openPriKey()  
{  
    //构建RSA数据结构  
    m_priKey = RSA_new();   
    m_priKey->d = BN_bin2bn(m_priExpd, m_priExpdLen, m_priKey->d);  
    m_priKey->n = BN_bin2bn(m_module, m_moduleLen, m_priKey->n);  
  
    return 0;  
} 

// 打开公钥  
int RSAOperate::openPubKey() 
{  
    //构建RSA数据结构  
    m_pubKey = RSA_new();  
    m_pubKey->e = BN_bin2bn(m_pubExpd, m_pubExpdLen, m_pubKey->e);  
    m_pubKey->n = BN_bin2bn(m_module, m_moduleLen, m_pubKey->n);  
  
    //RSA_print_fp(stdout, m_pubKey, 0);  
    return 0;  
}  


// 私钥加密函数  
int RSAOperate::priKeyEncrypt(const unsigned char *in, int inLen,  
                   unsigned char **out, int &outLen)  
{  
    outLen = RSA_size(m_priKey);  
    *out = (unsigned char *)malloc(outLen);  
    if (NULL == *out)  
    {   
		write_log("prikey_encrypt:malloc error!");  
        return -1;  
    }  
    memset((void *)*out, 0, outLen);  
  
    printf("prikey_encrypt:Begin RSA_private_encrypt ...\n");  
    int ret =  RSA_private_encrypt(inLen, in, *out, m_priKey, RSA_PKCS1_PADDING); 
  
    return ret;  
}


// 公钥解密函数，返回解密后的数据长度  
int RSAOperate::pubKeyDecrypt(const unsigned char *in, int inLen,  
                           unsigned char **out, int &outLen)  
{  
    outLen =  RSA_size(m_pubKey);  
    *out =  (unsigned char *)malloc(outLen);  
    if(NULL == *out)  
    {  
		write_log("pubkey_decrypt:malloc error!");  
        return -1;  
    }  
    memset((void *)*out, 0, outLen);  
  
    printf("pubkey_decrypt:Begin RSA_public_decrypt ...\n");  
    int ret = RSA_public_decrypt(inLen, in, *out, m_pubKey, RSA_PKCS1_PADDING);  
    return ret;  
}  


// 公钥加密函数  
int RSAOperate::pubKeyEncrypt(const unsigned char *in, int inLen,  
                           unsigned char **out, int &outLen)  
{  
    outLen = RSA_size(m_pubKey);  
    *out = (unsigned char *)malloc(outLen);  
    if(NULL == *out)  
	{  
		write_log("Malloc error!");  
        return -1;  
    }  
	memset((void *)*out, 0, outLen);  
  
    printf("pubkey_encrypt:Begin RSA_public_encrypt ...\n");  
    int ret =  RSA_public_encrypt(inLen, in, *out, m_pubKey, RSA_PKCS1_PADDING/*RSA_NO_PADDING*/);  
    return ret;  
}  
  
// 私钥解密函数，返回解密后的长度  
int RSAOperate::priKeyDecrypt(const unsigned char *in, int inLen,  
                           unsigned char **out, int &outLen)  
{  
    outLen = RSA_size(m_priKey);  
    *out = (unsigned char *)malloc(outLen);  
    if (NULL == *out)  
    {  
        write_log("Malloc error!");  
        return -1;  
    }  
    memset((void *)*out, 0, outLen);  
  
    printf("prikey_decrypt:Begin RSA_private_decrypt ...\n");  
    int ret = RSA_private_decrypt(inLen, in, *out, m_priKey, RSA_PKCS1_PADDING);  
  
    return ret;  
}  
  
// 释放分配的内存资源  
void RSAOperate::freeRes()  
{  
    if (m_pubExpd)  
    {  
        delete [] m_pubExpd;  
        m_pubExpd = NULL;  
    }  
  
    if (m_priExpd)  
    {  
        delete [] m_priExpd;  
        m_priExpd = NULL;  
    }  
	if (m_module)  
    {  
        delete [] m_module;  
        m_module = NULL;  
    }  
}  
  
// 释放公钥和私钥结构资源  
int RSAOperate::closeKey()  
{  
	if (m_pubKey)  
	{  
		RSA_free(m_pubKey);  
		m_pubKey = NULL;  
	}  

	if (m_priKey)  
	{  
		RSA_free(m_priKey);  
		m_priKey = NULL;  
	}  
	return 0;  
}  



#if 0
// 测试 rsaOperate 类
int main()
{
	const char origBuff[1024] = {"Hello World."};
	int buffLen = strlen(origBuff);
	u_int8 *out = NULL;
	int outLen;
	int nRet;
	u_int8 *buff = NULL;
	int Len;
	
	RSAOperate rsa;
	// rsa.generateKeyStr("./mod.conf");
	rsa.setParams();
	rsa.openPriKeyAndPubKey();
	rsa.openPubKey();

	// 私钥加密，公钥解密
	nRet = rsa.priKeyEncrypt((u_int8*)origBuff, buffLen, &out, outLen);
	if (-1 == nRet)
	{
		write_log("11111111111111111111111111");
		return -1;
	}
	nRet = rsa.pubKeyDecrypt(out, outLen, &buff, Len);
	if (-1 == nRet)
	{
		write_log("2222222222222222222222222");
		return -1;
	}
	printf("buff=%s, nRet=%d, Len=%d\n", buff, nRet, Len);
	if (out) free(out), out = NULL;
	if (buff) free(buff), buff = NULL;

	
	
	// 公钥加密，私钥解密
	nRet = rsa.pubKeyEncrypt((u_int8*)origBuff, buffLen, &out, outLen);
	if (-1 == nRet)
	{
		write_log("333333333333333333333333");
		return -1;
	} 
	nRet = rsa.priKeyDecrypt(out, outLen, &buff, Len);
	if (-1 == nRet)
	{
		write_log("44444444444444444444444");
		return -1;
	} 
	printf("buff=%s, nRet=%d, Len=%d\n", buff, nRet, Len);
	if (out) free(out), out = NULL;
	if (buff) free(buff), buff = NULL;
	
	while(1)sleep(10);
	return 0;
}
#endif 



#if 0
// 测试 EVP RSA 模式的加密方式
int 
LoadMemToFile(const unsigned char* buff, const int buffLen, const char* FilePath)
{
    FILE *fd = fopen(FilePath, "w");
    if (NULL == fd)
    {
        printf("Open %s Failed.", FilePath);
        return -1;
    }
    int i = 0;
    i = fwrite(buff, buffLen, 1, fd);
    if (i != 1)
    {
        printf("Error: fwrite to file failed.\n");
        fclose(fd);
        return -1;
    }
    fclose(fd);
    return 0;
}
int 
ReadFileNLenToMem(char* buff, const int buffLen, const char* FilePath)
{
    FILE *fd = fopen(FilePath, "r");
    if (NULL == fd)
    {
        printf("Open %s Failed.", FilePath);
        return -1;
    }
    int i = 0;
    i = fread(buff, 1, buffLen, fd);
    
    fclose(fd);
    return i;
}
static inline void 
print_hex(const unsigned char* buf, int len)
{
    int i;
    if (NULL == buf)
    {
        return;
    }
    for (i=0; i<len; ++i)
    {
        printf("%02x ", buf[i]);
        if ( ((i+1)%32 == 0) && i )
        {
            printf("\n");
        }
    }
    if (i)
    {
        printf("\n");
    }
    return ;
}


int main()
{	
	//EVPRSA *rsa = new EVPRSA((u_int8*)"123456", 6);
	EVPRSA *rsa = new EVPRSA();
	int nRet;
	printf("RSA_F4=%ld\n", RSA_F4);
	
#if 0
	// 生成密匙文件
	nRet = rsa->generateKeyFile("./public.key", "./private.key", 1024);
	if (-1 == nRet)
	{
		write_log("11111111111111111111111");
		return 0;
	} 
#endif

#if 0
	CBase64Operate base64;
	char buff[256] = {0};
	memset(buff, 'a', 128);
	memset(buff+128, 'b', 128);
	char output[512] = {0};
	DWORD outputLen = 0;
	base64.Encrypt(buff, 256, output, outputLen);
	printf("output=%s\n", output);

	std::string Data;
	base64.Decrypt(output, outputLen, Data);
	printf("buff=%s\n", Data.c_str());
	return 0;
#endif 

#if 0
	CBase64Operate base64;
	std::string Data;
	char buff[1024] = {0};
	nRet = ReadFileNLenToMem(buff, 256, "./my2.dat");
	printf("buff=%s\n", buff);
	base64.Decrypt(buff, nRet, Data);
	//printf("Data.c_str()=%s\n", Data.c_str());
	//LoadMemToFile((unsigned char*)Data.c_str(), Data.size(), "./public.key");
#endif 


#if 1
	// 打开私钥文件
	nRet = rsa->openPrivateKey("./private.key");
	if (-1 == nRet)
	{
		write_log("3333333333333333333333333");
		return 0;
	}
#endif

	// 打开公钥文件
	nRet = rsa->openPublicKey("./public.key");
	if (-1 == nRet)
	{
		write_log("22222222222222222222222");
		return 0;
	}

#if 0
	// 使用公钥加密
	unsigned char origData[128] = {0};
	memset(origData, 'a', 117);
	size_t origDataLen = 117;
	unsigned char encData[1024] = {0};
	size_t encDataLen;
	nRet = rsa->rsaKeyEncrypt(origData, origDataLen, encData, encDataLen);
	if (-1 == nRet)
	{
		write_log("4444444444444444444444444");
		return 0;
	}
	printf("encDataLen=%d\n", encDataLen);
	CBase64Operate base64;
	unsigned char base64Out[1024] = {0};
	DWORD outLen = 1024;
	base64.Encrypt((char*)encData, encDataLen, (char*)base64Out, outLen);
	print_hex(encData, encDataLen);
	printf("outLen=%ld\n", outLen);
	LoadMemToFile(base64Out, outLen, "./abc.dat");
#endif 

#if 1
	// 读取文件内容
	char buff[1024] = {0};
	nRet = ReadFileNLenToMem(buff, 512, "./my2.dat");
	CBase64Operate base64;
	std::string Data;
	base64.Decrypt(buff, nRet, Data);
	
	unsigned char encData[1024] = {0};
	size_t encDataLen = Data.size();
	printf("size=%d\n", encDataLen);
	memcpy(encData, Data.c_str(), Data.size());
#endif 

#if 1
	// 使用私钥解密
	unsigned char origBuff[1024] = {0};
	size_t len;
	nRet = rsa->rsaKeyDecrypt(encData, encDataLen, origBuff, len);
	if (-1 == nRet)
	{
		write_log("555555555555555555555555");
		return 0;
	}
	printf("origBuff=%s\n", origBuff);
#endif
	delete rsa;	
	
	return 0;
}
#endif 


/*
EVPRSA rsa((u_int8*)"RSA", 3);
// 生成公钥和私钥
int generateKeyFile(const char *pubKeyFile, const char *priKeyFile, int length)
{
	return rsa.generateKeyFile(pubKeyFile, priKeyFile, length);
}

// 函数功能: 打开公钥
// 函数参数: keyfile，公钥文件路径
// 返 回 值: 成功返回0，失败-1
int openPublicKey(const char *keyfile)
{
	return rsa.openPublicKey(keyfile);
}

// 函数功能: 打开私钥
// 函数参数: keyfile，私钥文件路径
// 返 回 值: 成功返回0，失败-1
int openPrivateKey(const char *keyfile)
{
	return rsa.openPrivateKey(keyfile);
}

// 函数功能: 使用公钥加密，这种封装格式只适用公钥加密，私钥解密
// 输入参数: origData，待加密数据；origDataLen，待加密数据长度；
// 输出参数: encData，加密后的数据；encDataLen，加密后的数据长度
// 返 回 值: 成功返回0，失败-1
int rsaKeyEncrypt(const u_int8 *origData, size_t origDataLen,	
				u_int8 *encData, size_t &encDataLen)
{
	return rsa.rsaKeyEncrypt(origData, origDataLen, encData, encDataLen);
}

// 函数功能: 使用密钥解密，这种封装格式只适用公钥加密，私钥解密，这里key必须是私钥 
// 输入参数: encData，加密后的数据，encDataLen，加密后的数据长度；
// 输出参数: origData，解密后的数据，origDataLen，解密后的数据长度；		  
// 返 回 值: 成功返回0，失败-1
int rsaKeyDecrypt(const u_int8 *encData, size_t encDataLen,   
				u_int8 *origData, size_t &origDataLen)
{
	return rsa.rsaKeyDecrypt(encData, encDataLen, origData, origDataLen);
}
*/


