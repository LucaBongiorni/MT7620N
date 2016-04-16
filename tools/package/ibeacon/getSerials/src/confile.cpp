/*****************************************************************************
这是一个应用在 linux 环境下的专于 key/value 读写 .ini文件的配置文件
一个文件当中含有多个section,每个section当中含有多个key
key和 section都是一种结构。
例如：
   {
     [UserSettings]
     Name=Joe User            //一个key 
     Date of Birth=12/25/01   //一个key
								等等
     ;
     ; Settings unique to this server
     ;
   } 整体为一个section

	[ServerSettings]
	Port=1200
	IP_Address=127.0.0.1
	MachineName=ADMIN
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <cassert>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include "defCom.h"
#include "confile.h"
#include "StrOperate.h"
#include "includes.h"




#ifdef WIN32
  #define snprintf  _snprintf
  #define vsnprintf _vsnprintf
#endif

namespace SOIni
{

// 声明函数
void* CreateInstance(void *pArgHandle, void *pArg);
void  DestroyInstance(void *pArg);
bool  Load(void *pArg,t_Str szFileName);
t_Str GetString(void *pArg,t_Str szKey, t_Str szSection);
int   GetKeyInt(void *pArg,t_Str szKey, t_Str szSection);
t_Str GetValue(void *pArg,t_Str szKey, t_Str szSection);
int   GetInt(void *pArg,t_Str szKey, t_Str szSection);
t_Str GetKeyComment(void *pArg,t_Str szKey, t_Str szSection);
bool  SetValue(void *pArg, t_Str szKey, t_Str szValue, t_Str szComment, t_Str szSection);
bool  Save(void *pArg);
bool  CreateSection(void *pArg,t_Str szSection, t_Str szComment);
void  Clear(void *pArg);
void  SetFileName(void *pArg,t_Str szFileName);
int   SectionCount(void *pArg);
int   KeyCount(void *pArg);
bool  SetKeyComment(void *pArg,t_Str szKey, t_Str szComment, t_Str szSection);
bool  SetSectionComment(void *pArg,t_Str szSection, t_Str szComment);
bool  SetFloat(void *pArg,t_Str szKey, float fValue, t_Str szComment, t_Str szSection);
bool  SetInt(void *pArg,t_Str szKey, int nValue, t_Str szComment, t_Str szSection);
bool  SetBool(void *pArg,t_Str szKey, bool bValue, t_Str szComment, t_Str szSection);
float GetFloat(void *pArg,t_Str szKey, t_Str szSection);
bool  GetBool(void *pArg,t_Str szKey, t_Str szSection);
bool  DeleteSection(void *pArg,t_Str szSection);
bool  DeleteKey(void *pArg,t_Str szKey, t_Str szFromSection);
bool  CreateKey(void *pArg,t_Str szKey, t_Str szValue, t_Str szComment, t_Str szSection);
t_Str GetKey(void *pArg,t_Str szKey, t_Str szSection);



//定义
void* 
CreateInstance(void *pArgHandle, void *pArg)
{
	CIniOp *pIni = new CIniOp();
	return pIni;
}

void 
DestroyInstance(void *pArg)
{
	CIniOp *pIni = (CIniOp *)pArg;
	if(pIni)
	{
		delete pIni;
		pIni = NULL;		
	}
}

/*************************************************
函数名称:         Load
函数功能:         加载文件
调用的函数:       GetSection(), SetValue(),CreateSection(),Report(),GetNextWord()
输入参数:         szFileName
返回值:           (bool)true 表示成功，false表示失败
*****************************************************/
bool 
Load(void *pArg,t_Str szFileName)
{
	CIniOp *pIni = (CIniOp *)pArg;
    fstream File(szFileName.c_str(), std::ios::in|std::ios::out);//?nocreat

    if ( File.is_open() )
    {
        bool bDone = false;//
        bool bAutoKey = (pIni->m_Flags & AUTOCREATE_KEYS) == AUTOCREATE_KEYS;
        bool bAutoSec = (pIni->m_Flags & AUTOCREATE_SECTIONS) == AUTOCREATE_SECTIONS;

        t_Str szLine;
        t_Str szComment;//注释
        char buffer[MAX_BUFFER_LEN];
        t_Section * pSection = pIni->GetSection("");


        pIni->m_Flags |= AUTOCREATE_KEYS;
        pIni->m_Flags |= AUTOCREATE_SECTIONS;

		bool bFirstLine = true;	// 是否第一行
		pIni->m_sFileStart.clear();

        //循环读文件
        while ( !bDone )
        {
            memset(buffer, 0, MAX_BUFFER_LEN);
            (void)File.getline(buffer, MAX_BUFFER_LEN);

			// 保留UTF-8等文件的起始头
			if (bFirstLine && (memcmp(buffer, "\xEF\xBB\xBF", 3) == 0))
			{
				pIni->m_sFileStart = "\xEF\xBB\xBF";
				szLine = buffer + 3;
			}
			else
			{
				szLine = buffer;
			}
            pIni->Trim(szLine);
			bFirstLine = false;

            bDone = ( File.eof() || File.bad() || File.fail() );//检测文件的状态

            if ( szLine.find_first_of(CommentIndicators) == 0 )
            {
                szComment += "\n";
                szComment += szLine;
            }
            else if ( szLine.find_first_of('[') == 0 ) // 新 section
	    	{
                (void)szLine.erase( 0, 1 );
                (void)szLine.erase( szLine.find_last_of(']'), 1 );

                (void)CreateSection(pIni,szLine, szComment);
                pSection = pIni->GetSection(szLine);
                szComment = t_Str("");
            }
            else if ( szLine.size() > 0 ) // 增加一个 key/value
            {
                t_Str szKey = pIni->GetNextWord(szLine);
                t_Str szValue = szLine;

                if ( szKey.size() > 0 )
                {
					if (pSection != NULL)
					{
						(void)SetValue(pIni,szKey, szValue, szComment, pSection->szName);
					}
                    szComment = t_Str("");
                }
            }
        }

        // 重新记录 bAutoKey,bAutoSec的状态
        if ( !bAutoKey )
            pIni->m_Flags &= ~AUTOCREATE_KEYS;

        if ( !bAutoSec )
            pIni->m_Flags &= ~AUTOCREATE_SECTIONS;
    }
    else
    {
    	char tmp[128] = {0};
		memcpy(tmp, "[CIniOp::Load] Unable to open file. Does it exist?", 
			sizeof("[CIniOp::Load] Unable to open file. Does it exist?"));
        pIni->Report(E_INFO, tmp);
        return false;
    }

    File.close();
    pIni->m_szFileName = szFileName;
    return true;
}



/*****************************************************************************
函数名称：      GetString
函数功能：      由给定的section返回keys值t_Str   
调用的函数:		GetValue
输入参数：      t_Str szKey, t_Str szSection = t_Str("")
返回值：        (t_Str)返回key的值，以字符串的形式
实现说明: 		调用return GetValue(szKey, szSection);即可
*****************************************************************************/
t_Str 
GetString(void *pArg,t_Str szKey, t_Str szSection)
{
	CIniOp *pIni = (CIniOp *)pArg;
    return GetValue(pIni,szKey, szSection);
}


/*****************************************************************************
函数名称：          GetKeyInt
函数功能：　　      从指定的section中返回指定的key 值
调用的函数:                 GetSection(),CompareNoCase() 
输入参数：          t_Str szKey, t_Str szSection
返回值：            (int  )-1表示没有找到，其他表示下标 
*****************************************************************************/
int 
GetKeyInt(void *pArg,t_Str szKey, t_Str szSection)
{
	CIniOp *pIni = (CIniOp *)pArg;
    t_Section* pSection;

    if ( (pSection = pIni->GetSection(szSection)) == NULL )
        return -1;
	
    vector<t_Key>::iterator k_pos;
    int n = 0;
    for (k_pos = pSection->Keys.begin(); k_pos != pSection->Keys.end(); k_pos++)
    {
        //遍历keys
		if ( pIni->CompareNoCase( (*k_pos).szKey, szKey ) == 0 )
			return n;
		n++;
    }
    return -1;
}


/****************************************************************************
函数名称：          GetValue
函数功能：          由给定的section返回keys值
调用的函数:			GetSection(),GetKeyInt()
输入参数：          t_Str szKey, t_Str szSection = t_Str("")
返回值：            (t_Str)返回key的值，以字符串的形式
*****************************************************************************/
t_Str 
GetValue(void *pArg,t_Str szKey, t_Str szSection) 
{
	CIniOp *pIni = (CIniOp *)pArg;
	t_Section *pSection= pIni->GetSection(szSection);
	int nRet;
	nRet = GetKeyInt(pIni,szKey,szSection);
	if (nRet < 0)
	{
		return t_Str("");
	}
	t_Key key;
	key = pSection->Keys[nRet];
    return  key.szValue;
}


/*****************************************************************************
函数名称：          GetInt
函数功能：          以int 的形式返回值int
调用的函数:			GetValue()
输入参数：          t_Str szKey, t_Str szSection = t_Str("")
返回值：            (int)返回key的值，int的形式
*****************************************************************************/
int 
GetInt(void *pArg,t_Str szKey, t_Str szSection)
{
	CIniOp *pIni = (CIniOp *)pArg;
    t_Str szValue = GetValue(pIni,szKey, szSection);

    if ( szValue.size() == 0 )
        return INT_MIN;

    return atoi( szValue.c_str() );
}


/*************************************************
函数名称:		    GetKeyComment
函数功能:		    取指定的key注释
调用的函数:			GetSection(),CompareNoCase()
输入参数:			(t_Str szKey, t_Str szComment, t_Str szSection)
返回值:				注释                   
*****************************************************/
t_Str 
GetKeyComment(void *pArg,t_Str szKey, t_Str szSection)
{
	
	CIniOp *pIni = (CIniOp *)pArg;    
    t_Section* pSection;

	t_Str szRes;

    if ( (pSection = pIni->GetSection(szSection)) == NULL )
        return szRes;

    vector<t_Key>::iterator k_pos;
	//遍历查找 szKey
    for (k_pos = pSection->Keys.begin(); k_pos != pSection->Keys.end(); k_pos++)
    {
        if ( pIni->CompareNoCase(k_pos->szKey, szKey ) == 0 )
        {
            szRes = k_pos->szComment;
            break;
        }
    }
    return szRes;
}


/*************************************************
函数名称:        	SetValue
函数功能:           给定一个key/value/section 之后，增加之
调用的函数:         GetKey(),GetSection(),CreateSectoin()
输入参数:           (t_Str szKey, t_Str szComment, t_Str szSection)
返回值:             (bool)true 表示成功，false表示失败
*****************************************************/
bool 
SetValue(void *pArg, t_Str szKey, t_Str szValue, t_Str szComment, t_Str szSection)
{

    CIniOp *pIni = (CIniOp *)pArg;
    t_Section* pSection = pIni->GetSection(szSection);

    if (pSection == NULL)
    {
        if ( !(pIni->m_Flags & AUTOCREATE_SECTIONS) || !CreateSection(pIni,szSection,""))
            return false;
        pSection = pIni->GetSection(szSection);
    }
	
    int nRet;
    nRet = GetKeyInt(pIni,szKey,szSection);
    t_Key key;

    // 如果Key不存在，增加一个新的
    if (( nRet==-1) && (pIni->m_Flags & AUTOCREATE_KEYS))
    {
        key.szKey = szKey;
        key.szValue = szValue;
        key.szComment = szComment;

        pIni->m_bDirty = true;
        (*pSection).Keys.push_back(key);
        return true;
    }

    if ( nRet!= -1 )//Key存在
    {
        pSection->Keys[nRet].szValue = szValue;
        pSection->Keys[nRet].szComment = szComment;
        pIni->m_bDirty = true;
        return true;
    }
    return false;
}


/*************************************************
函数名称:		  Save
函数功能:		  把 Section 和  keys 保存到文件中
调用的函数:		  WriteLn(),Report()
返回值:			  (bool)true 表示成功，false表示失败  
*****************************************************/
bool 
Save(void *pArg)
{
	char tempInfo[256] = {0};
	CIniOp *pIni = (CIniOp *)pArg;
	if ( KeyCount(pIni) == 0 && SectionCount(pIni) == 0 )//统计key和section 的数目
	{
		// 如果没有，report E_INFO
		strcpy(tempInfo, "[CIniOp::Save] Nothing to save.");
		pIni->Report(E_INFO, tempInfo);
		return false; 
    }

	if ( pIni->m_szFileName.size() == 0 )//没有文件名 
	{
		strcpy(tempInfo, "[CIniOp::Save] No filename has been set.");
		pIni->Report(E_ERROR, tempInfo);
		return false;
    }

    fstream File(pIni->m_szFileName.c_str(), ios::out|ios::trunc);
	if ( File.is_open() )
    {
		// 先写文件头
		if (!pIni->m_sFileStart.empty())
		{
			(void)File.write(pIni->m_sFileStart.c_str(), pIni->m_sFileStart.length());
		}

		vector<t_Key>::iterator   	  k_pos;
		vector<t_Section*>::iterator  s_pos;
		// 外循环遍历 section
        for (s_pos = pIni->m_Sections.begin(); s_pos != pIni->m_Sections.end(); s_pos++)
        {
            bool bWroteComment = false;//是否写注释

            if ( (*s_pos)->szComment.size() > 0 )
            {
                bWroteComment = true;
				memset(tempInfo, 0, 256);
				strncpy(tempInfo, "\n%s", 256);
                (void)pIni->WriteLn(File, tempInfo, pIni->CommentStr((*s_pos)->szComment).c_str());
            }

            if ( (*s_pos)->szName.size() > 0 )
            {
            	memset(tempInfo, 0, 256);
				strncpy(tempInfo, "%s[%s]", 256);
                (void)pIni->WriteLn(File, tempInfo, 
                        bWroteComment ? "" : "\n", 
                        (*s_pos)->szName.c_str());
            }

            // 内循环遍历 keys
            for (k_pos = (*s_pos)->Keys.begin(); k_pos != (*s_pos)->Keys.end(); k_pos++)
            {             
                if (k_pos->szKey.size() > 0 && k_pos->szValue.size() > 0 )
                {
                	memset(tempInfo, 0, 256);
					strncpy(tempInfo, "%s%s%s%s%c%s", 256);
                    (void)pIni->WriteLn(File, tempInfo, 
                        k_pos->szComment.size() > 0 ? "\n" : "",
                        pIni->CommentStr(k_pos->szComment).c_str(),
                        k_pos->szComment.size() > 0 ? "\n" : "",
                        k_pos->szKey.c_str(),
                        EqualIndicators[0],
                        k_pos->szValue.c_str());
                }
				if (k_pos->szKey.size() > 0 && k_pos->szValue.size() == 0)
                {
               		memset(tempInfo, 0, 256);
					strncpy(tempInfo, "%s%s%s%s%c", 256);
					(void)pIni->WriteLn(File, tempInfo, 
                        k_pos->szComment.size() > 0 ? "\n" : "",
                        pIni->CommentStr(k_pos->szComment).c_str(),
                        k_pos->szComment.size() > 0 ? "\n" : "",
                        k_pos->szKey.c_str(),
                        EqualIndicators[0]);
            	} 
            }
        }
    }
    else
    {
    	memset(tempInfo, 0, 256);
		strncpy(tempInfo, "[CIniOp::Save] Unable to save file.", 256);
        pIni->Report(E_ERROR, tempInfo);
        return false;
    }

    pIni->m_bDirty = false;
    
    (void)File.flush();
    File.close();

    return true;
}

/*****************************************************************************
函数名称：       CreateSection
函数功能：       建立section,但是没有keys
调用的函数:      GetSection()
输入参数：       名字t_Str szSection,注释 t_Str szComment = t_Str("")
返回值：         (bool)true表示成功，false表示失败
*****************************************************************************/
bool 
CreateSection(void *pArg,t_Str szSection, t_Str szComment)
{
    CIniOp *pIni = (CIniOp *)pArg;
    t_Section * pSection = pIni->GetSection(szSection);

    if ( pSection )
    {
    	char tmp[256] = {0};
		strncpy(tmp, "[CIniOp::CreateSection] Section <%s> allready exists. Aborting.", 256);
        pIni->Report(E_INFO, tmp, szSection.c_str());
        return false;
    }

    pSection = new t_Section;
    //建立新的section
    pSection->szName = szSection;
    pSection->szComment = szComment;
    pIni->m_Sections.push_back(pSection);
    pIni->m_bDirty = true;

    return true;	//lint !e429
}


/*************************************************
函数名称:		    Clear
函数功能:			清除所有的 section
调用的函数:			empty(),clear()
*****************************************************/
void Clear(void *pArg)
{
	CIniOp *pIni = (CIniOp *)pArg;
	typedef vector<t_Section*>  value_type;    
	pIni->m_bDirty = false;
	pIni->m_szFileName = t_Str("");
	for(value_type::iterator pointer=pIni->m_Sections.begin();
		pointer!=pIni->m_Sections.end(); pointer++)
	{
		delete *pointer;
	}
	pIni->m_Sections.clear();
}


/*************************************************
函数名称:		    SetFileName
函数功能:		    设置文件名
调用的函数:			size()
输入参数:			szFileName        
*****************************************************/
void 
SetFileName(void *pArg,t_Str szFileName)
{
	CIniOp *pIni = (CIniOp *)pArg;
    if (pIni->m_szFileName.size() != 0 && pIni->CompareNoCase(szFileName, pIni->m_szFileName) != 0)//字符串比较
    {
        pIni->m_bDirty = true;//是否修改
    }
    pIni->m_szFileName = szFileName;
}


/*****************************************************************************
函数名称：       SectionCount
函数功能：       返回 Section　的数目
返回值：         (int )返回 Section　的数目
*****************************************************************************/
int 
SectionCount(void *pArg)
{
	CIniOp *pIni = (CIniOp *)pArg;
    return pIni->m_Sections.size();
}

/*****************************************************************************
函数名称：          GetCount
函数功能：          返回所有section 当中的key 数目
返回值：   　　　　　(int )返回所有section 当中的key 数目
*****************************************************************************/
int 
KeyCount(void *pArg)
{
	CIniOp *pIni = (CIniOp *)pArg;
	int nCounter = 0;

	vector<t_Section*>::iterator pointer;
	for (pointer=pIni->m_Sections.begin(); pointer!=pIni->m_Sections.end(); pointer++)
	{
		nCounter += (*pointer)->Keys.size();
	}
	return nCounter;
}


/*************************************************
函数名称:		SetKeyComment
函数功能:		为指定的key 加注释
调用的函数:		GetSection(),CompareNoCase()
输入参数:		(t_Str szKey, t_Str szComment, t_Str szSection)
返回值:			(bool)true 表示成功，false表示失败
*****************************************************/
bool 
SetKeyComment(void *pArg,t_Str szKey, t_Str szComment, t_Str szSection)
{
	CIniOp *pIni = (CIniOp *)pArg;    
	t_Section* pSection;

    if ( (pSection = pIni->GetSection(szSection)) == NULL )
        return false;
    
    vector<t_Key>::iterator k_pos;
	//遍历查找 szKey
    for (k_pos = pSection->Keys.begin(); k_pos != pSection->Keys.end(); k_pos++)
    {
        if ( pIni->CompareNoCase(k_pos->szKey, szKey ) == 0 )
        {
            k_pos->szComment = szComment;
            pIni->m_bDirty = true;
            return true;
        }
    }
    return false;
}

/*************************************************
函数名称:		    SetSectionComment
函数功能:		    为指定的section 加注释
调用的函数:			CompareNoCase()
输入参数:			(t_Str szKey, t_Str szComment, t_Str szSection)
返回值:				(bool)true 表示成功，false表示失败
*****************************************************/
bool 
SetSectionComment(void *pArg,t_Str szSection, t_Str szComment)
{
	CIniOp *pIni = (CIniOp *)pArg;
	//循环遍历查找 section
    typedef vector<t_Section *> value_type;
    for (value_type::iterator s_pos = pIni->m_Sections.begin(); s_pos != pIni->m_Sections.end(); s_pos++)  
    {
        if ( pIni->CompareNoCase( (*s_pos)->szName, szSection ) == 0 ) 
        {
            (*s_pos)->szComment = szComment;
            pIni->m_bDirty = true;
            return true;
        }
    }

    return false;
}

/*****************************************************************************
函数名称：    SetFloat
函数功能：    在一个section 当中建立给定的key（float）,或者当“AUTOCREATE_KEYS”被激活时建立
调用的函数:	  SetValue()
输入参数：    t_Str szKey,float fValue,t_Str szComment = t_Str("")， t_Str szSection = t_Str("")
返回值：      (bool)true表示成功，false表示失败
*****************************************************************************/
bool 
SetFloat(void *pArg,t_Str szKey, float fValue, t_Str szComment, t_Str szSection)
{
	CIniOp *pIni = (CIniOp *)pArg;
    char szStr[64];

    sprintf(szStr, "%f", fValue);

    return SetValue(pIni,szKey, szStr, szComment, szSection);
}

/*****************************************************************************
函数名称：    SetInt
函数功能：    在一个section 当中建立给定的key（int）,或者当“AUTOCREATE_KEYS”被激活时建立
调用的函数:   SetValue
输入参数：    t_Str szKey,int nValue,t_Str szComment = t_Str("")， t_Str szSection = t_Str("")
返回值：      (bool)true表示成功，false表示失败
*****************************************************************************/
bool 
SetInt(void *pArg,t_Str szKey, int nValue, t_Str szComment, t_Str szSection)
{
	CIniOp *pIni = (CIniOp *)pArg;
    char szStr[64];
    sprintf(szStr, "%d", nValue);//?

    return SetValue(pIni,szKey, szStr, szComment, szSection);

}

/*****************************************************************************
函数名称：     SetBool
函数功能：     在一个section 当中建立给定的key（bool）,或者当“AUTOCREATE_KEYS”被激活时建立
调用的函数:	   SetValue
输入参数：     t_Str szKey,bool bValue,t_Str szComment = t_Str("")， t_Str szSection = t_Str("")
返回值：       (bool)true表示成功，false表示失败
*****************************************************************************/
bool 
SetBool(void *pArg,t_Str szKey, bool bValue, t_Str szComment, t_Str szSection)
{
	CIniOp *pIni = (CIniOp *)pArg;
    t_Str szValue = bValue ?  "True" : "False";

    return SetValue(pIni,szKey, szValue, szComment, szSection);
}



/*****************************************************************************
函数名称：          GetFloat
函数功能：          以float的形式返回值float 
调用的函数:			GetValue
输入参数：          t_Str szKey, t_Str szSection = t_Str("")
返回值：            (float)返回key的值，float的形式
*****************************************************************************/
float 
GetFloat(void *pArg,t_Str szKey, t_Str szSection)
{
	CIniOp *pIni = (CIniOp *)pArg;
    t_Str szValue = GetValue(pIni,szKey, szSection);

    if ( szValue.size() == 0 )
        return -1.0;

    return (float)atof( szValue.c_str() );
}

/*****************************************************************************
函数名称：    GetBool    
函数功能：    以bool的形式返回值bool   
调用的函数:	  CetValue()
输入参数：    t_Str szKey, t_Str szSection = t_Str("")
返回值：      (bool)返回key的值，bool的形式
*****************************************************************************/
bool 
GetBool(void *pArg,t_Str szKey, t_Str szSection)
{
	CIniOp *pIni = (CIniOp *)pArg;
    bool bValue = false;
    t_Str szValue = GetValue(pIni,szKey, szSection);

    if ( szValue.find("1") == 0 
        || pIni->CompareNoCase(szValue, "true") 
        || pIni->CompareNoCase(szValue, "yes") )
    {
        bValue = true;
    }

    return bValue;
}

/*****************************************************************************
函数名称：  DeleteSection
函数功能：  删除指定的 section.
输入参数：  t_Str szSection名字
返回值：    (bool)true表示成功，false表示失败
*****************************************************************************/
bool 
DeleteSection(void *pArg, t_Str szSection)
{
	CIniOp *pIni = (CIniOp *)pArg;
	vector<t_Section *>::iterator pointer;

	//遍历查找指定的 section
    for(pointer=pIni->m_Sections.begin();pointer!=pIni->m_Sections.end();pointer++)
    {
 
        if (pIni->CompareNoCase( (*pointer)->szName, szSection ) == 0 ) 
        {
	     delete *pointer;			
            (void)pIni->m_Sections.erase(pointer);
            return true;
        }
    }
    return false;
}


/*****************************************************************************
函数名称：          DeleteKey
函数功能：          从指定的section当中删除指定的key
调用的函数:			GetSection(),CompareNoCase()
输入参数：          t_Str szKey, t_Str szFromSection 
返回值：            (bool)true表示成功，false表示失败
*****************************************************************************/
bool 
DeleteKey(void *pArg,t_Str szKey, t_Str szFromSection)
{

	CIniOp *pIni = (CIniOp *)pArg;
    t_Section* pSection;

	if ( (pSection = pIni->GetSection(szFromSection)) == NULL )
		return false;

 	vector<t_Key>::iterator pointer;
    for(pointer=pSection->Keys.begin();pointer!=pSection->Keys.end();pointer++)
    {
        //遍历查找指定的 key
        if (pIni->CompareNoCase( pointer->szKey, szKey ) == 0 )
        {
            (void)pSection->Keys.erase(pointer);
            //delete pointer;
            return true;
        }
    } 
    return false;
}


/*****************************************************************************
函数名称：    CreateKey
函数功能：    在要求的section当中建立一个新的key
调用的函数:   SetValue()
输入参数：    t_Str szKey,int nValue,t_Str szComment = t_Str("")， t_Str szSection = t_Str("")
返回值：      (bool)true表示成功，false表示失败
*****************************************************************************/
bool 
CreateKey(void *pArg,t_Str szKey, t_Str szValue, t_Str szComment, t_Str szSection)
{
	CIniOp *pIni = (CIniOp *)pArg;
    if(szSection.size()==0)
        return false;
    bool bAutoKey = (pIni->m_Flags & AUTOCREATE_KEYS) == AUTOCREATE_KEYS;
    bool bReturn  = false;

    pIni->m_Flags |= AUTOCREATE_KEYS;

    bReturn = SetValue(pIni,szKey, szValue, szComment, szSection);

    if ( !bAutoKey )
        pIni->m_Flags &= ~AUTOCREATE_KEYS;

    return bReturn;
}


/*****************************************************************************
函数名称:      GetKey　　　　　
函数功能:      从指定的szSection中，把指定名(szKey)的key以整个字符串的形式返回
调用的函数:    GetKeyInt(),GetSection
输入参数:      名字t_Str szKey, 名字t_Str szSection
返回值:        (t_Str )返回指定key的名字，以字符串的形式
*****************************************************************************/
t_Str 
GetKey(void *pArg,t_Str szKey, t_Str szSection)
{
	 CIniOp *pIni = (CIniOp *)pArg;
	 t_Section* pSection;

	if ( (pSection = pIni->GetSection(szSection)) == NULL )
		return t_Str("");

	int nPos;
	nPos = GetKeyInt(pIni,szKey, szSection);
	if (nPos < 0)
	{
		return t_Str("");
	}
	return pSection->Keys[nPos].szKey;
}


}


/*****************************************************************************
函数名称:		    CIniOp
函数功能:			构造函数，确定m_Flages的标志
调用的函数:			Clear()
其它:	                   
*****************************************************************************/
CIniOp::CIniOp()
{
    m_bDirty = false;
    m_szFileName = t_Str("");
    for(value_type::iterator pointer=m_Sections.begin();pointer!=m_Sections.end();pointer++)
    {
		delete * pointer;
		//m_Sections.erase(pointer);
    }
    m_Sections.clear();


    m_Flags = (AUTOCREATE_SECTIONS | AUTOCREATE_KEYS);
}

/*****************************************************************************
函数名称:		    ~CIniOp
函数功能:			析构函数，释放section    
*****************************************************************************/
CIniOp::~CIniOp()
{
	try
	{
		value_type::iterator pointer;

		//释放每一个section
		for(pointer=m_Sections.begin();pointer!=m_Sections.end();pointer++)
		{
			delete  *pointer;
			//m_Sections.erase(pointer);
		}

		m_Sections.clear();
	}
	catch (...)
	{
	}
}

/*****************************************************************************
函数名称：     CommentStr(t_Str szComment);
函数功能：     解析一个字符串到相应的“注释”中
调用的函数:    Trim()
输入参数：     t_Str szComment,注释
返回值：       (t_Str )
*****************************************************************************/
t_Str 
CIniOp::CommentStr(t_Str szComment)
{
    t_Str szNewStr = t_Str("");

    Trim(szComment);

        if ( szComment.size() == 0 )
          return szComment;

    if ( szComment.find_first_of(CommentIndicators) != 0 )
    {
        szNewStr = CommentIndicators[0];
        szNewStr += " ";
    }

    szNewStr += szComment;

    return szNewStr;
}

t_Section* 
CIniOp::GetSection(t_Str szSection)
{

    vector<t_Section*>::iterator  s_pos;
    for (s_pos = m_Sections.begin(); s_pos != m_Sections.end(); s_pos++)

    {
        t_Section * pSection = (t_Section *) (*s_pos);

        if ( CompareNoCase( pSection->szName, szSection ) == 0 )
            return pSection;
    }

    return NULL;
}

/*****************************************************************************
函数名称：          GetNextWord       
函数功能：          得到一个key
调用的函数:			Trim()
输入参数：          _Str& CommandLine          
返回值：            (t_str )返回经过处理的命令行，得到Key 的名字
实现说明：
    通过 “int nPos = CommandLine.find_first_of(EqualIndicators);”
    查找在字符串当中的出现位置nPos
    定义一个字符串t_Str sWord = t_Str("");
    判断
    if ( nPos大于 -1 )
    {   找到了
      取字符串sWord = CommandLine.substr(0, nPos);
      去掉nPos前面的字符CommandLine.erase(0, nPos+1);
     }
    else
     {   没有找到
     sWord = CommandLine;
     赋空CommandLine = t_Str("");
     }

     去掉sWord两边的空格Trim(sWord);
     返回return sWord
*****************************************************************************/
t_Str 
CIniOp::GetNextWord(t_Str& CommandLine)
{
    int nPos = CommandLine.find_first_of(EqualIndicators);
    t_Str sWord = t_Str("");

    if ( nPos > -1 )
    {
        sWord = CommandLine.substr(0, nPos);
        (void)CommandLine.erase(0, nPos+1);
    }
    else
    {
        sWord = CommandLine;
        CommandLine = t_Str("");
    }


    Trim(sWord);
    return sWord;
}


/*****************************************************************************
函数名称：          CompareNoCase      
函数功能：       　 两个字符串的比较，不区分大小写
调用的函数:			stricmp()
被调用函数清单:		无
被访问的表:			无
被修改的表:			无
输入参数：          要比较的两个字符串t_Str str1, t_Str str2
返回值：            (int )，相等返回0，不相等返回-1      
实现说明：
    直接调用库函数即可
   #ifdef WIN32//?
 return stricmp(str1.c_str(), str2.c_str());    
   #else
   return strcasecmp(str1.c_str(), str2.c_str());
   #endif
*****************************************************************************/
int 
CIniOp::CompareNoCase(t_Str str1, t_Str str2)
{
#ifdef WIN32//?
	return stricmp(str1.c_str(), str2.c_str());    
#else
    return strcasecmp(str1.c_str(), str2.c_str());
#endif
}


/*****************************************************************************
函数名称：          Trim
函数功能：     　　 把字符串两边的空格清楚
输入参数：  　　　　字符串t_Str szStr
实现说明:
   找到左边的起使位置
   // trim 左边
   nPos = szStr.find_first_not_of(szTrimChars);

   if ( nPos大于0找到 )
   去掉左边szStr.erase(0, nPos);
   找到右边的起使位置
   // trim 右边
   nPos = szStr.find_last_not_of(szTrimChars);
   rPos = szStr.find_last_of(szTrimChars);
   if ( rPos 大于 nPos 且 rPos > -1)
   {
    szStr.erase(rPos, szStr.size()-rPos);
   }
*****************************************************************************/
void 
CIniOp::Trim(t_Str& szStr)
{
    t_Str szTrimChars = WhiteSpace;
    
    szTrimChars += EqualIndicators;
    int nPos, rPos;

    // trim 左边
    nPos = szStr.find_first_not_of(szTrimChars);

    if ( nPos > 0 )
        (void)szStr.erase(0, nPos);

    // trim 右边
    nPos = szStr.find_last_not_of(szTrimChars);
    rPos = szStr.find_last_of(szTrimChars);

    if ( rPos > nPos && rPos > -1)
    {
        (void)szStr.erase(rPos, szStr.size()-rPos);
    }
}

/*****************************************************************************
函数名称：          WriteLn
函数功能：          写文件,并且返回字节数目
输入参数：          文件的类型 fstream& stream,指向文件名的指针 char* fmt
返回值：            ( int )返回写入文件的字节数目
实现说明：
    定义缓冲区char buf[MAX_BUFFER_LEN];
    定义返回的字节长度int nLength;
    字符串t_Str szMsg;
    memset(buf, 0, MAX_BUFFER_LEN);
    va_list args;
    va_start (args, fmt);
    输入到缓冲nLength = vsnprintf(buf, MAX_BUFFER_LEN, fmt, args);//?
    va_end (args);
   if ( buf[nLength] != '\n' && buf[nLength] != '\r' )
   buf[nLength++] = '\n';
   输入到文件stream.write(buf, nLength);
   返回字节数目    return nLength;   
*****************************************************************************/
int 
CIniOp::WriteLn(fstream& stream, char* fmt, ...)
{
    char buf[MAX_BUFFER_LEN];
    int nLength;
    t_Str szMsg;

    memset(buf, 0, MAX_BUFFER_LEN);
    va_list args;

    va_start (args, fmt);
    nLength = vsnprintf(buf, MAX_BUFFER_LEN, fmt, args);//?
    va_end (args);

    if ( buf[nLength] != '\n' && buf[nLength] != '\r' )
        buf[nLength++] = '\n';

    (void)stream.write(buf, nLength);

    return nLength;
}

/*****************************************************************************
函数名称： Report         
函数功能： 返回处理信息
输入参数： 信息的各种情况e_DebugLevel DebugLevel, 指向文件名的指针 char* fmt
返回值：   ( void )
*****************************************************************************/
void 
CIniOp::Report(e_DebugLevel DebugLevel/*信息的各种情况*/, char *fmt, ...)
{
    char buf[MAX_BUFFER_LEN];
    int nLength;
    t_Str szMsg;

    va_list args;

    memset(buf, 0, MAX_BUFFER_LEN);

    va_start (args, fmt);
	nLength = vsnprintf(buf, MAX_BUFFER_LEN, fmt, args);
    va_end (args);

    if ( buf[nLength] != '\n' && buf[nLength] != '\r' )
        buf[nLength++] = '\n';
    
	//各种情况信息
    switch ( DebugLevel )
    {
        case E_DEBUG:
            szMsg = "<debug> ";
            break;
        case E_INFO:
            szMsg = "<info> ";
            break;
        case E_WARN:
            szMsg = "<warn> ";
            break;
        case E_ERROR:
            szMsg = "<error> ";
            break;
        case E_FATAL:
            szMsg = "<fatal> ";
            break;
        case E_CRITICAL:
            szMsg = "<critical> ";
            break;
    }

    szMsg += buf;
}


/******************************************************************************/
CFileOperate::CFileOperate()
{
}

/******************************************************************************/
CFileOperate::~CFileOperate()
{
}

/******************************************************************************/
long 
CFileOperate::GetFileSize(const char szFile[])
{
	ASSERT(szFile != NULL);

	int nHd = open(szFile, O_RDONLY);
	if (-1 == nHd)
	{
		return -1;
	}

	long lRes = -1;
	try
	{
		struct stat file_info = {0};
		if (fstat(nHd, &file_info) == -1)
		{
			throw(lRes);
		}
		lRes = file_info.st_size;
	}
	catch (...)
	{
	}
	close(nHd);
	return lRes;
}



// 初始化静态成员变量
void*					CConfFile::m_hHandle			= NULL;
INI_CREATEINSTANCE		CConfFile::m_pCreateFun			= NULL;
INI_DESTROYINSTANCE		CConfFile::m_pDestroyFun		= NULL;
INI_LOAD				CConfFile::m_pLoadFun			= NULL;
INI_GETSTRING			CConfFile::m_pGetStrFun			= NULL;
INI_GETINT				CConfFile::m_pGetIntFun			= NULL;
INI_SETVALUE			CConfFile::m_pSetValueFun		= NULL;
INI_SAVE				CConfFile::m_pSaveFun			= NULL;
INI_GETKEYCOMMENT		CConfFile::m_pGetKeyCommentFun	= NULL;

bool 
CConfFile::Init()
{
	if (m_hHandle != NULL)
	{
		return true;
	}
	// 主要用于动态库时，打开动态库的句柄，目前未使用
	m_hHandle = (void *)1;

	try
	{
		m_pCreateFun   = SOIni::CreateInstance;
		m_pDestroyFun  = SOIni::DestroyInstance;
		m_pLoadFun     = SOIni::Load;
		m_pGetStrFun   = SOIni::GetString;
		m_pGetIntFun   = SOIni::GetInt;
		m_pSetValueFun = SOIni::SetValue;
		m_pSaveFun     = SOIni::Save;
		m_pGetKeyCommentFun = SOIni::GetKeyComment;
	}
	catch (...)
	{
		if (m_hHandle != NULL)
		{
			m_hHandle = NULL;
		}

		return false;
	}
	return true;
}

CConfFile::CConfFile()
{
	ASSERT(m_hHandle != NULL);

	m_pIniObj = m_pCreateFun(NULL, NULL);
}

CConfFile::~CConfFile()
{
	if (m_pIniObj != NULL)
	{
		m_pDestroyFun(m_pIniObj);
		m_pIniObj = NULL;
	}
}

bool 
CConfFile::LoadFile(const char szFile[])
{
	ASSERT(szFile != NULL);

	if ((NULL == m_hHandle) || (NULL == m_pIniObj))
	{
		return false;
	}

	return m_pLoadFun(m_pIniObj, szFile);
}

void 
CConfFile::GetString(const char szApp[], const char szKey[], char szStr[])
{
	ASSERT(szApp != NULL);
	ASSERT(szKey != NULL);
	ASSERT(szStr != NULL);

	if ((NULL == m_hHandle) || (NULL == m_pIniObj))
	{
		return;
	}

	std::string s = m_pGetStrFun(m_pIniObj, szKey, szApp);
	strcpy(szStr, s.c_str());
	return;
}


void 
CConfFile::GetTrimString(const char szApp[], const char szKey[],
							char szStr[])
{
	ASSERT(szApp != NULL);
	ASSERT(szKey != NULL);
	ASSERT(szStr != NULL);

	if ((NULL == m_hHandle) || (NULL == m_pIniObj))
	{
		return;
	}

	// 取值
	GetString(szApp, szKey, szStr);

	// Trim
	std::string s = CStrOperate::Trim(szStr);
	strcpy(szStr, s.c_str());
}


int 
CConfFile::GetInt(const char szApp[], const char szKey[])
{
	ASSERT(szApp != NULL);
	ASSERT(szKey != NULL);

	if ((NULL == m_hHandle) || (NULL == m_pIniObj))
	{
		return -1;
	}

	return m_pGetIntFun(m_pIniObj, szKey, szApp);
}


void 
CConfFile::GetKeyComment(const char szApp[], const char szKey[],
								char szComment[])
{
	ASSERT(szApp != NULL);
	ASSERT(szKey != NULL);
	ASSERT(szComment != NULL);

	if ((NULL == m_hHandle) || (NULL == m_pIniObj))
	{
		return;
	}

	std::string sRes = m_pGetKeyCommentFun(m_pIniObj, szKey, szApp);
	strcpy(szComment, sRes.c_str());
	return;
}


bool 
CConfFile::SetValue(const char szApp[], const char szKey[],
						const char szVal[], const char szComment[])
{
	ASSERT(szApp != NULL);
	ASSERT(szKey != NULL);
	ASSERT(szVal != NULL);

	if ((NULL == m_hHandle) || (NULL == m_pIniObj))
	{
		return false;
	}

	std::string sApp = szApp;
	std::string sKey = szKey;
	std::string sVal = szVal;
	std::string sComment;
	if (szComment != NULL)
	{
		sComment = szComment;
	}

	return m_pSetValueFun(m_pIniObj, sKey, sVal, sComment, sApp);
}

/******************************************************************************/
bool 
CConfFile::Save()
{
	if ((NULL == m_hHandle) || (NULL == m_pIniObj))
	{
		return false;
	}

	return m_pSaveFun(m_pIniObj);
}
char* GetMacByIfName(const char *ifname)
{
	int r;
	int sockd;
	struct ifreq ifr;
	char *hwaddr, mac[32];

	if (ifname == NULL || strlen(ifname) > (IFNAMESIZ-1))
	{
		printf("[%s][%d]Param Error.", __FILE__, __LINE__);
		return NULL;
	}
	
	if (-1 == (sockd = socket(PF_INET, SOCK_DGRAM, 0))) 
	{
		printf("[%s][%d]Create Socket Failed.", __FILE__, __LINE__);
		return NULL;
	}
	
	strcpy(ifr.ifr_name, ifname);
	r = ioctl(sockd, SIOCGIFHWADDR, &ifr);
	if (r == -1) 
	{
		printf("[%s][%d]Ioctl Get Socket Ifaddr Failed.", __FILE__, __LINE__);
		close(sockd);
		return NULL;
	}
	hwaddr = ifr.ifr_hwaddr.sa_data;
	close(sockd);
	
	snprintf(mac, sizeof(mac), "%02X:%02X:%02X:%02X:%02X:%02X", 
			hwaddr[0] & 0xFF,
			hwaddr[1] & 0xFF,
			hwaddr[2] & 0xFF,
			hwaddr[3] & 0xFF,
			hwaddr[4] & 0xFF,
			hwaddr[5] & 0xFF
		);
	return strdup(mac);
}


#if 1
//CConfFile *g_pconfFile;	
int main()
{
 u8 adr_ble_cb[LEN_ADR_BLE];
 u8 len_adr_ble=0;
 char  adr_ble_chg[LEN_ADR_BLE*3*2];
 char  adr_ble_tmp[3];
 char* p_marc_net=NULL;


 init_cloud_beacon();
 string filename= string(MARK_DEV_CFG_FILE_PATH);
 fstream fn;
 fn.open(filename.c_str(),ofstream::out);
 if (fn) 
    fprintf(stderr, "create file Success\n");
 else
 	fprintf(stderr, "create file Failure\n");
 fn.close();

// 初始化配置文件
if (!CConfFile::Init())
{
	fprintf(stderr, "Init ini failure\n");
	return -1;
}
	
// 打开配置文件
CConfFile *confFile = new CConfFile();
//g_pconfFile = confFile;
if (!confFile->LoadFile(MARK_DEV_CFG_FILE_PATH))
{
	fprintf(stderr, "Open ini failure\n");
	delete confFile;
	return -1;
}


get_adr_ble_cb( (u8 *)&adr_ble_cb,&len_adr_ble);

for(int j=0;j<LEN_ADR_BLE;j++)
{
 
 sprintf(adr_ble_tmp, "%02x", adr_ble_cb[j]);
if(0==j)
   sprintf(adr_ble_chg, "%s", adr_ble_tmp);
else
   sprintf(adr_ble_chg, "%s:%s", adr_ble_chg,adr_ble_tmp);


}


p_marc_net= GetMacByIfName(IFCON_NAME);

sprintf(adr_ble_chg, "%s:%s", adr_ble_chg,p_marc_net);
printf("marc_net =%s\n",adr_ble_chg);

confFile->SetValue("Mark_cb_Info", "cb", adr_ble_chg, "#CloudBeacon唯一标识");
confFile->Save();




free(p_marc_net);
p_marc_net=NULL;

delete confFile;

	return 0;
}

#endif



