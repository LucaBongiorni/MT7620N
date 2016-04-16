/*****************************************************************************
����һ��Ӧ���� linux �����µ�ר�� key/value ��д .ini�ļ��������ļ�
һ���ļ����к��ж��section,ÿ��section���к��ж��key
key�� section����һ�ֽṹ��
���磺
   {
     [UserSettings]
     Name=Joe User            //һ��key 
     Date of Birth=12/25/01   //һ��key
								�ȵ�
     ;
     ; Settings unique to this server
     ;
   } ����Ϊһ��section

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

// ��������
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



//����
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
��������:         Load
��������:         �����ļ�
���õĺ���:       GetSection(), SetValue(),CreateSection(),Report(),GetNextWord()
�������:         szFileName
����ֵ:           (bool)true ��ʾ�ɹ���false��ʾʧ��
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
        t_Str szComment;//ע��
        char buffer[MAX_BUFFER_LEN];
        t_Section * pSection = pIni->GetSection("");


        pIni->m_Flags |= AUTOCREATE_KEYS;
        pIni->m_Flags |= AUTOCREATE_SECTIONS;

		bool bFirstLine = true;	// �Ƿ��һ��
		pIni->m_sFileStart.clear();

        //ѭ�����ļ�
        while ( !bDone )
        {
            memset(buffer, 0, MAX_BUFFER_LEN);
            (void)File.getline(buffer, MAX_BUFFER_LEN);

			// ����UTF-8���ļ�����ʼͷ
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

            bDone = ( File.eof() || File.bad() || File.fail() );//����ļ���״̬

            if ( szLine.find_first_of(CommentIndicators) == 0 )
            {
                szComment += "\n";
                szComment += szLine;
            }
            else if ( szLine.find_first_of('[') == 0 ) // �� section
	    	{
                (void)szLine.erase( 0, 1 );
                (void)szLine.erase( szLine.find_last_of(']'), 1 );

                (void)CreateSection(pIni,szLine, szComment);
                pSection = pIni->GetSection(szLine);
                szComment = t_Str("");
            }
            else if ( szLine.size() > 0 ) // ����һ�� key/value
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

        // ���¼�¼ bAutoKey,bAutoSec��״̬
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
�������ƣ�      GetString
�������ܣ�      �ɸ�����section����keysֵt_Str   
���õĺ���:		GetValue
���������      t_Str szKey, t_Str szSection = t_Str("")
����ֵ��        (t_Str)����key��ֵ�����ַ�������ʽ
ʵ��˵��: 		����return GetValue(szKey, szSection);����
*****************************************************************************/
t_Str 
GetString(void *pArg,t_Str szKey, t_Str szSection)
{
	CIniOp *pIni = (CIniOp *)pArg;
    return GetValue(pIni,szKey, szSection);
}


/*****************************************************************************
�������ƣ�          GetKeyInt
�������ܣ�����      ��ָ����section�з���ָ����key ֵ
���õĺ���:                 GetSection(),CompareNoCase() 
���������          t_Str szKey, t_Str szSection
����ֵ��            (int  )-1��ʾû���ҵ���������ʾ�±� 
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
        //����keys
		if ( pIni->CompareNoCase( (*k_pos).szKey, szKey ) == 0 )
			return n;
		n++;
    }
    return -1;
}


/****************************************************************************
�������ƣ�          GetValue
�������ܣ�          �ɸ�����section����keysֵ
���õĺ���:			GetSection(),GetKeyInt()
���������          t_Str szKey, t_Str szSection = t_Str("")
����ֵ��            (t_Str)����key��ֵ�����ַ�������ʽ
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
�������ƣ�          GetInt
�������ܣ�          ��int ����ʽ����ֵint
���õĺ���:			GetValue()
���������          t_Str szKey, t_Str szSection = t_Str("")
����ֵ��            (int)����key��ֵ��int����ʽ
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
��������:		    GetKeyComment
��������:		    ȡָ����keyע��
���õĺ���:			GetSection(),CompareNoCase()
�������:			(t_Str szKey, t_Str szComment, t_Str szSection)
����ֵ:				ע��                   
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
	//�������� szKey
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
��������:        	SetValue
��������:           ����һ��key/value/section ֮������֮
���õĺ���:         GetKey(),GetSection(),CreateSectoin()
�������:           (t_Str szKey, t_Str szComment, t_Str szSection)
����ֵ:             (bool)true ��ʾ�ɹ���false��ʾʧ��
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

    // ���Key�����ڣ�����һ���µ�
    if (( nRet==-1) && (pIni->m_Flags & AUTOCREATE_KEYS))
    {
        key.szKey = szKey;
        key.szValue = szValue;
        key.szComment = szComment;

        pIni->m_bDirty = true;
        (*pSection).Keys.push_back(key);
        return true;
    }

    if ( nRet!= -1 )//Key����
    {
        pSection->Keys[nRet].szValue = szValue;
        pSection->Keys[nRet].szComment = szComment;
        pIni->m_bDirty = true;
        return true;
    }
    return false;
}


/*************************************************
��������:		  Save
��������:		  �� Section ��  keys ���浽�ļ���
���õĺ���:		  WriteLn(),Report()
����ֵ:			  (bool)true ��ʾ�ɹ���false��ʾʧ��  
*****************************************************/
bool 
Save(void *pArg)
{
	char tempInfo[256] = {0};
	CIniOp *pIni = (CIniOp *)pArg;
	if ( KeyCount(pIni) == 0 && SectionCount(pIni) == 0 )//ͳ��key��section ����Ŀ
	{
		// ���û�У�report E_INFO
		strcpy(tempInfo, "[CIniOp::Save] Nothing to save.");
		pIni->Report(E_INFO, tempInfo);
		return false; 
    }

	if ( pIni->m_szFileName.size() == 0 )//û���ļ��� 
	{
		strcpy(tempInfo, "[CIniOp::Save] No filename has been set.");
		pIni->Report(E_ERROR, tempInfo);
		return false;
    }

    fstream File(pIni->m_szFileName.c_str(), ios::out|ios::trunc);
	if ( File.is_open() )
    {
		// ��д�ļ�ͷ
		if (!pIni->m_sFileStart.empty())
		{
			(void)File.write(pIni->m_sFileStart.c_str(), pIni->m_sFileStart.length());
		}

		vector<t_Key>::iterator   	  k_pos;
		vector<t_Section*>::iterator  s_pos;
		// ��ѭ������ section
        for (s_pos = pIni->m_Sections.begin(); s_pos != pIni->m_Sections.end(); s_pos++)
        {
            bool bWroteComment = false;//�Ƿ�дע��

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

            // ��ѭ������ keys
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
�������ƣ�       CreateSection
�������ܣ�       ����section,����û��keys
���õĺ���:      GetSection()
���������       ����t_Str szSection,ע�� t_Str szComment = t_Str("")
����ֵ��         (bool)true��ʾ�ɹ���false��ʾʧ��
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
    //�����µ�section
    pSection->szName = szSection;
    pSection->szComment = szComment;
    pIni->m_Sections.push_back(pSection);
    pIni->m_bDirty = true;

    return true;	//lint !e429
}


/*************************************************
��������:		    Clear
��������:			������е� section
���õĺ���:			empty(),clear()
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
��������:		    SetFileName
��������:		    �����ļ���
���õĺ���:			size()
�������:			szFileName        
*****************************************************/
void 
SetFileName(void *pArg,t_Str szFileName)
{
	CIniOp *pIni = (CIniOp *)pArg;
    if (pIni->m_szFileName.size() != 0 && pIni->CompareNoCase(szFileName, pIni->m_szFileName) != 0)//�ַ����Ƚ�
    {
        pIni->m_bDirty = true;//�Ƿ��޸�
    }
    pIni->m_szFileName = szFileName;
}


/*****************************************************************************
�������ƣ�       SectionCount
�������ܣ�       ���� Section������Ŀ
����ֵ��         (int )���� Section������Ŀ
*****************************************************************************/
int 
SectionCount(void *pArg)
{
	CIniOp *pIni = (CIniOp *)pArg;
    return pIni->m_Sections.size();
}

/*****************************************************************************
�������ƣ�          GetCount
�������ܣ�          ��������section ���е�key ��Ŀ
����ֵ��   ����������(int )��������section ���е�key ��Ŀ
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
��������:		SetKeyComment
��������:		Ϊָ����key ��ע��
���õĺ���:		GetSection(),CompareNoCase()
�������:		(t_Str szKey, t_Str szComment, t_Str szSection)
����ֵ:			(bool)true ��ʾ�ɹ���false��ʾʧ��
*****************************************************/
bool 
SetKeyComment(void *pArg,t_Str szKey, t_Str szComment, t_Str szSection)
{
	CIniOp *pIni = (CIniOp *)pArg;    
	t_Section* pSection;

    if ( (pSection = pIni->GetSection(szSection)) == NULL )
        return false;
    
    vector<t_Key>::iterator k_pos;
	//�������� szKey
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
��������:		    SetSectionComment
��������:		    Ϊָ����section ��ע��
���õĺ���:			CompareNoCase()
�������:			(t_Str szKey, t_Str szComment, t_Str szSection)
����ֵ:				(bool)true ��ʾ�ɹ���false��ʾʧ��
*****************************************************/
bool 
SetSectionComment(void *pArg,t_Str szSection, t_Str szComment)
{
	CIniOp *pIni = (CIniOp *)pArg;
	//ѭ���������� section
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
�������ƣ�    SetFloat
�������ܣ�    ��һ��section ���н���������key��float��,���ߵ���AUTOCREATE_KEYS��������ʱ����
���õĺ���:	  SetValue()
���������    t_Str szKey,float fValue,t_Str szComment = t_Str("")�� t_Str szSection = t_Str("")
����ֵ��      (bool)true��ʾ�ɹ���false��ʾʧ��
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
�������ƣ�    SetInt
�������ܣ�    ��һ��section ���н���������key��int��,���ߵ���AUTOCREATE_KEYS��������ʱ����
���õĺ���:   SetValue
���������    t_Str szKey,int nValue,t_Str szComment = t_Str("")�� t_Str szSection = t_Str("")
����ֵ��      (bool)true��ʾ�ɹ���false��ʾʧ��
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
�������ƣ�     SetBool
�������ܣ�     ��һ��section ���н���������key��bool��,���ߵ���AUTOCREATE_KEYS��������ʱ����
���õĺ���:	   SetValue
���������     t_Str szKey,bool bValue,t_Str szComment = t_Str("")�� t_Str szSection = t_Str("")
����ֵ��       (bool)true��ʾ�ɹ���false��ʾʧ��
*****************************************************************************/
bool 
SetBool(void *pArg,t_Str szKey, bool bValue, t_Str szComment, t_Str szSection)
{
	CIniOp *pIni = (CIniOp *)pArg;
    t_Str szValue = bValue ?  "True" : "False";

    return SetValue(pIni,szKey, szValue, szComment, szSection);
}



/*****************************************************************************
�������ƣ�          GetFloat
�������ܣ�          ��float����ʽ����ֵfloat 
���õĺ���:			GetValue
���������          t_Str szKey, t_Str szSection = t_Str("")
����ֵ��            (float)����key��ֵ��float����ʽ
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
�������ƣ�    GetBool    
�������ܣ�    ��bool����ʽ����ֵbool   
���õĺ���:	  CetValue()
���������    t_Str szKey, t_Str szSection = t_Str("")
����ֵ��      (bool)����key��ֵ��bool����ʽ
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
�������ƣ�  DeleteSection
�������ܣ�  ɾ��ָ���� section.
���������  t_Str szSection����
����ֵ��    (bool)true��ʾ�ɹ���false��ʾʧ��
*****************************************************************************/
bool 
DeleteSection(void *pArg, t_Str szSection)
{
	CIniOp *pIni = (CIniOp *)pArg;
	vector<t_Section *>::iterator pointer;

	//��������ָ���� section
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
�������ƣ�          DeleteKey
�������ܣ�          ��ָ����section����ɾ��ָ����key
���õĺ���:			GetSection(),CompareNoCase()
���������          t_Str szKey, t_Str szFromSection 
����ֵ��            (bool)true��ʾ�ɹ���false��ʾʧ��
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
        //��������ָ���� key
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
�������ƣ�    CreateKey
�������ܣ�    ��Ҫ���section���н���һ���µ�key
���õĺ���:   SetValue()
���������    t_Str szKey,int nValue,t_Str szComment = t_Str("")�� t_Str szSection = t_Str("")
����ֵ��      (bool)true��ʾ�ɹ���false��ʾʧ��
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
��������:      GetKey����������
��������:      ��ָ����szSection�У���ָ����(szKey)��key�������ַ�������ʽ����
���õĺ���:    GetKeyInt(),GetSection
�������:      ����t_Str szKey, ����t_Str szSection
����ֵ:        (t_Str )����ָ��key�����֣����ַ�������ʽ
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
��������:		    CIniOp
��������:			���캯����ȷ��m_Flages�ı�־
���õĺ���:			Clear()
����:	                   
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
��������:		    ~CIniOp
��������:			�����������ͷ�section    
*****************************************************************************/
CIniOp::~CIniOp()
{
	try
	{
		value_type::iterator pointer;

		//�ͷ�ÿһ��section
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
�������ƣ�     CommentStr(t_Str szComment);
�������ܣ�     ����һ���ַ�������Ӧ�ġ�ע�͡���
���õĺ���:    Trim()
���������     t_Str szComment,ע��
����ֵ��       (t_Str )
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
�������ƣ�          GetNextWord       
�������ܣ�          �õ�һ��key
���õĺ���:			Trim()
���������          _Str& CommandLine          
����ֵ��            (t_str )���ؾ�������������У��õ�Key ������
ʵ��˵����
    ͨ�� ��int nPos = CommandLine.find_first_of(EqualIndicators);��
    �������ַ������еĳ���λ��nPos
    ����һ���ַ���t_Str sWord = t_Str("");
    �ж�
    if ( nPos���� -1 )
    {   �ҵ���
      ȡ�ַ���sWord = CommandLine.substr(0, nPos);
      ȥ��nPosǰ����ַ�CommandLine.erase(0, nPos+1);
     }
    else
     {   û���ҵ�
     sWord = CommandLine;
     ����CommandLine = t_Str("");
     }

     ȥ��sWord���ߵĿո�Trim(sWord);
     ����return sWord
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
�������ƣ�          CompareNoCase      
�������ܣ�       �� �����ַ����ıȽϣ������ִ�Сд
���õĺ���:			stricmp()
�����ú����嵥:		��
�����ʵı�:			��
���޸ĵı�:			��
���������          Ҫ�Ƚϵ������ַ���t_Str str1, t_Str str2
����ֵ��            (int )����ȷ���0������ȷ���-1      
ʵ��˵����
    ֱ�ӵ��ÿ⺯������
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
�������ƣ�          Trim
�������ܣ�     ���� ���ַ������ߵĿո����
���������  ���������ַ���t_Str szStr
ʵ��˵��:
   �ҵ���ߵ���ʹλ��
   // trim ���
   nPos = szStr.find_first_not_of(szTrimChars);

   if ( nPos����0�ҵ� )
   ȥ�����szStr.erase(0, nPos);
   �ҵ��ұߵ���ʹλ��
   // trim �ұ�
   nPos = szStr.find_last_not_of(szTrimChars);
   rPos = szStr.find_last_of(szTrimChars);
   if ( rPos ���� nPos �� rPos > -1)
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

    // trim ���
    nPos = szStr.find_first_not_of(szTrimChars);

    if ( nPos > 0 )
        (void)szStr.erase(0, nPos);

    // trim �ұ�
    nPos = szStr.find_last_not_of(szTrimChars);
    rPos = szStr.find_last_of(szTrimChars);

    if ( rPos > nPos && rPos > -1)
    {
        (void)szStr.erase(rPos, szStr.size()-rPos);
    }
}

/*****************************************************************************
�������ƣ�          WriteLn
�������ܣ�          д�ļ�,���ҷ����ֽ���Ŀ
���������          �ļ������� fstream& stream,ָ���ļ�����ָ�� char* fmt
����ֵ��            ( int )����д���ļ����ֽ���Ŀ
ʵ��˵����
    ���建����char buf[MAX_BUFFER_LEN];
    ���巵�ص��ֽڳ���int nLength;
    �ַ���t_Str szMsg;
    memset(buf, 0, MAX_BUFFER_LEN);
    va_list args;
    va_start (args, fmt);
    ���뵽����nLength = vsnprintf(buf, MAX_BUFFER_LEN, fmt, args);//?
    va_end (args);
   if ( buf[nLength] != '\n' && buf[nLength] != '\r' )
   buf[nLength++] = '\n';
   ���뵽�ļ�stream.write(buf, nLength);
   �����ֽ���Ŀ    return nLength;   
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
�������ƣ� Report         
�������ܣ� ���ش�����Ϣ
��������� ��Ϣ�ĸ������e_DebugLevel DebugLevel, ָ���ļ�����ָ�� char* fmt
����ֵ��   ( void )
*****************************************************************************/
void 
CIniOp::Report(e_DebugLevel DebugLevel/*��Ϣ�ĸ������*/, char *fmt, ...)
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
    
	//���������Ϣ
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



// ��ʼ����̬��Ա����
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
	// ��Ҫ���ڶ�̬��ʱ���򿪶�̬��ľ����Ŀǰδʹ��
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

	// ȡֵ
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

// ��ʼ�������ļ�
if (!CConfFile::Init())
{
	fprintf(stderr, "Init ini failure\n");
	return -1;
}
	
// �������ļ�
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

confFile->SetValue("Mark_cb_Info", "cb", adr_ble_chg, "#CloudBeaconΨһ��ʶ");
confFile->Save();




free(p_marc_net);
p_marc_net=NULL;

delete confFile;

	return 0;
}

#endif



