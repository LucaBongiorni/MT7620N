#ifndef __CON__FILE__H__
#define __CON__FILE__H__

#include <vector>
#include <fstream>
#include <string.h>

using namespace std;

typedef enum __DebugLevel__
{
	E_DEBUG = 1,      // ������Ϣ
	E_INFO,           // ������Ϣ
	E_WARN,           // ������Ϣ
	E_ERROR,          // ������Ϣ��д�ռ�
	E_FATAL,          // ʧ����Ϣ��д��־
	E_CRITICAL,
}e_DebugLevel;



//����ȫ�ֵĽṹ������
///////////////////////////////////////////////////////////////////////////
// AUTOCREATE_SECTIONS
//������̬ section �ı�־.
#define AUTOCREATE_SECTIONS     (1L<<1)
// AUOTCREATE_KEYS
// ������̬ key �ı�־
#define AUTOCREATE_KEYS         (1L<<2)
// MAX_BUFFER_LEN
//һ�ζ�д������������ֽ���
#define MAX_BUFFER_LEN           1024*3


typedef std::string t_Str;
// CommentIndicators
// ��˵������ָʾ��־
const t_Str CommentIndicators = t_Str(";#");
// EqualIndicators
// ��Key ��ȡ��ı�־
const t_Str EqualIndicators   = t_Str("=");    //t_Str("=:");
// WhiteSpace
// ͨ�� Trim() ��������ַ������ߵĿո�
const t_Str WhiteSpace = t_Str(" \t\n\r");


// st_key һ��key �Ľṹ
class t_Key
{
public:
    t_Str        szKey;          //key ������
    t_Str        szValue;        //key ��ֵ
    t_Str        szComment;      //key ��˵��
public:
    t_Key()                    //�����ʼ��
    {
        szKey     = t_Str("");
        szValue   = t_Str("");
        szComment = t_Str("");
    }
    ~t_Key()                            //�������һ�� key
    {
        try
        {
            (void)szKey.erase();
            (void)szValue.erase();
            (void)szComment.erase();
        }
        catch (...)
        {
        }
    }
    t_Key & operator=(const t_Key & another)   //key �ĸ�ֵ������
    {
        if (this == &another)
        {
            return *this;
        }
        szKey = another.szKey;
        szValue = another.szValue;
        szComment = another.szComment;
        return *this;
    }
    t_Key(const t_Key &another)                //���Ƴ�ʼ���Ĺ��캯��
    {
        szKey = another.szKey;
        szValue = another.szValue;
        szComment = another.szComment;
    }
};
// st_section
// ÿ�� section �ṹ�����ж��key
class t_Section
{
public:
    t_Str        szName;         //section ������
    t_Str        szComment;      //section ��˵��
    vector<t_Key>   Keys;        //��key ��ɵ�����������һ��section �����ж��key )
public:
    t_Section()           //���캯��
    {
        szName    = t_Str("");
        szComment = t_Str("");
    }

    ~t_Section()                           //���������ͷ�һ��section ���ڲ������е�key
    {
    }
    t_Section & operator= (const t_Section &another)//section���ĸ�ֵ����
    {
        if ( this != & another )
        {
            szName = another.szName;
            szComment = another.szComment;
            Keys = another.Keys;
        }
        return *this;
    }
    t_Section(const t_Section &another)        //���Ƴ�ʼ���Ĺ��캯��
    {
        szName = another.szName;
        szComment = another.szComment;
        Keys = another.Keys;
    }
} ;
typedef std::vector<t_Section> SectionList;
typedef SectionList::iterator SectionItor;
// CIniOp���ļ��ṹ
class CIniOp
{
public:
    //д�ļ�,���ҷ����ֽ���Ŀ
    int WriteLn(fstream& stream, char* fmt, ...);
    //��stdout������Ϣ
    void Report(e_DebugLevel DebugLevel, char *fmt, ...);
    //�õ�һ��key
    t_Str GetNextWord(t_Str& CommandLine);
    //�����ַ����ıȽϣ���ȷ���0
    int CompareNoCase(t_Str str1, t_Str str2);
    //���ַ������ߵĿո����
    void  Trim(t_Str& szStr);
public:
    CIniOp();
    //��������
    ~CIniOp();
    t_Section*  GetSection(t_Str szSection);
    t_Str CommentStr(t_Str szComment);
public:
    long        m_Flags;        // ���ñ�־
    typedef vector<t_Section*>  value_type;
    value_type  m_Sections;     //  sections������
    t_Str       m_szFileName;   // д����ļ���
    bool        m_bDirty;       // ���������Ƿ�ı�
    t_Str       m_sFileStart;   // �ļ���ʼ�Σ�����UTF-8�ļ�ǰ�����\xEF\xBB\xBF
};
class CFileOperate
{
public:
    // ��ȡ�ļ���С
    static long GetFileSize(const char szFile[]);
    // ���캯��
    CFileOperate();
    virtual ~CFileOperate();
};
/******************************************************************************/
class CConfFile : public CFileOperate
{
protected:
    static void*                m_hHandle;
    static INI_CREATEINSTANCE   m_pCreateFun;
    static INI_DESTROYINSTANCE  m_pDestroyFun;
    static INI_GETSTRING        m_pGetStrFun;
    static INI_GETINT           m_pGetIntFun;
    static INI_LOAD             m_pLoadFun;
    static INI_SETVALUE         m_pSetValueFun;
    static INI_SAVE             m_pSaveFun;
    static INI_GETKEYCOMMENT    m_pGetKeyCommentFun;
public:
    // ��ʼ������
    static bool Init();

    CConfFile();
    virtual ~CConfFile();
    // �����ļ�����
    bool LoadFile(const char szFile[]);
    // ��ȡ�ַ�
    void GetString(const char szApp[], const char szKey[], char szStr[]);
    //
    void GetTrimString(const char szApp[], const char szKey[], char szStr[]);
    // ��ȡ��������
    int GetInt(const char szApp[], const char szKey[]);
    // ��ȡע����Ϣ
    void GetKeyComment(const char szApp[], const char szKey[], char szComment[]);
    // ������������
    // szApp ���ÿ���Ϣ
    // szKey ������ֵ
    // szVal ������ֵ
    // szComment ע������
    bool SetValue(const char szApp[], const char szKey[],
                  const char szVal[], const char szComment[]);
    // ����
    bool Save();
protected:
    void *m_pIniObj;
};
#endif /*__CON__FILE__H__*/
