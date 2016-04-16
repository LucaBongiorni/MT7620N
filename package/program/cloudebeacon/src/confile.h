#ifndef __CON__FILE__H__
#define __CON__FILE__H__

#include <vector>
#include <fstream>
#include <string.h>

using namespace std;

typedef enum __DebugLevel__
{
	E_DEBUG = 1,      // 调试信息
	E_INFO,           // 调试信息
	E_WARN,           // 警告信息
	E_ERROR,          // 错误信息，写日记
	E_FATAL,          // 失败信息，写日志
	E_CRITICAL,
}e_DebugLevel;



//定义全局的结构，类型
///////////////////////////////////////////////////////////////////////////
// AUTOCREATE_SECTIONS
//建立动态 section 的标志.
#define AUTOCREATE_SECTIONS     (1L<<1)
// AUOTCREATE_KEYS
// 建立动态 key 的标志
#define AUTOCREATE_KEYS         (1L<<2)
// MAX_BUFFER_LEN
//一次读写缓冲区的最大字节数
#define MAX_BUFFER_LEN           1024*3


typedef std::string t_Str;
// CommentIndicators
// “说明”的指示标志
const t_Str CommentIndicators = t_Str(";#");
// EqualIndicators
// “Key 相等”的标志
const t_Str EqualIndicators   = t_Str("=");    //t_Str("=:");
// WhiteSpace
// 通过 Trim() 函数清楚字符串两边的空格
const t_Str WhiteSpace = t_Str(" \t\n\r");


// st_key 一个key 的结构
class t_Key
{
public:
    t_Str        szKey;          //key 的名字
    t_Str        szValue;        //key 的值
    t_Str        szComment;      //key 的说明
public:
    t_Key()                    //构造初始化
    {
        szKey     = t_Str("");
        szValue   = t_Str("");
        szComment = t_Str("");
    }
    ~t_Key()                            //析构清楚一个 key
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
    t_Key & operator=(const t_Key & another)   //key 的赋值“＝”
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
    t_Key(const t_Key &another)                //复制初始化的构造函数
    {
        szKey = another.szKey;
        szValue = another.szValue;
        szComment = another.szComment;
    }
};
// st_section
// 每个 section 结构当中有多个key
class t_Section
{
public:
    t_Str        szName;         //section 的名字
    t_Str        szComment;      //section 的说明
    vector<t_Key>   Keys;        //由key 组成的容器（即：一个section 当中有多个key )
public:
    t_Section()           //构造函数
    {
        szName    = t_Str("");
        szComment = t_Str("");
    }

    ~t_Section()                           //析构函数释放一个section 及内部的所有的key
    {
    }
    t_Section & operator= (const t_Section &another)//section　的赋值操作
    {
        if ( this != & another )
        {
            szName = another.szName;
            szComment = another.szComment;
            Keys = another.Keys;
        }
        return *this;
    }
    t_Section(const t_Section &another)        //复制初始化的构造函数
    {
        szName = another.szName;
        szComment = another.szComment;
        Keys = another.Keys;
    }
} ;
typedef std::vector<t_Section> SectionList;
typedef SectionList::iterator SectionItor;
// CIniOp　文件结构
class CIniOp
{
public:
    //写文件,并且返回字节数目
    int WriteLn(fstream& stream, char* fmt, ...);
    //向stdout返回信息
    void Report(e_DebugLevel DebugLevel, char *fmt, ...);
    //得到一个key
    t_Str GetNextWord(t_Str& CommandLine);
    //两个字符串的比较，相等返回0
    int CompareNoCase(t_Str str1, t_Str str2);
    //把字符串两边的空格清楚
    void  Trim(t_Str& szStr);
public:
    CIniOp();
    //析构函数
    ~CIniOp();
    t_Section*  GetSection(t_Str szSection);
    t_Str CommentStr(t_Str szComment);
public:
    long        m_Flags;        // 设置标志
    typedef vector<t_Section*>  value_type;
    value_type  m_Sections;     //  sections的容器
    t_Str       m_szFileName;   // 写入的文件名
    bool        m_bDirty;       // 跟宗数据是否改变
    t_Str       m_sFileStart;   // 文件起始段，例如UTF-8文件前面加有\xEF\xBB\xBF
};
class CFileOperate
{
public:
    // 获取文件大小
    static long GetFileSize(const char szFile[]);
    // 构造函数
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
    // 初始化函数
    static bool Init();

    CConfFile();
    virtual ~CConfFile();
    // 加载文件函数
    bool LoadFile(const char szFile[]);
    // 获取字符
    void GetString(const char szApp[], const char szKey[], char szStr[]);
    //
    void GetTrimString(const char szApp[], const char szKey[], char szStr[]);
    // 获取整型数据
    int GetInt(const char szApp[], const char szKey[]);
    // 获取注释信息
    void GetKeyComment(const char szApp[], const char szKey[], char szComment[]);
    // 设置配置内容
    // szApp 配置块信息
    // szKey 配置左值
    // szVal 配置右值
    // szComment 注释内容
    bool SetValue(const char szApp[], const char szKey[],
                  const char szVal[], const char szComment[]);
    // 保存
    bool Save();
protected:
    void *m_pIniObj;
};
#endif /*__CON__FILE__H__*/
