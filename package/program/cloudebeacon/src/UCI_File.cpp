#ifdef USE_OPENWRT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uci.h>


#include "UCI_File.h"




//static const char *delimiter = " ";
UCI::UCI(const char* dir)
{
	m_ctx = uci_alloc_context();
	if (!m_ctx)
	{
		fprintf(stderr, "Out of memory.\n");
		return ;
	}
	uci_set_confdir(m_ctx, dir);
	uci_set_savedir(m_ctx, UCI_SAVE_DIR);
	m_pkg  = NULL;
	m_ptr  = NULL;
	m_File = NULL;
}

UCI::UCI()
{
	m_ctx = uci_alloc_context();
	if (!m_ctx)
	{
		fprintf(stderr, "Out of memory.\n");
		return ;
	}
	uci_set_savedir(m_ctx, UCI_SAVE_DIR);
	m_pkg  = NULL;
	m_ptr  = NULL;
	m_File = NULL;
}

UCI::UCI(const char* pFile, bool isFile)
{
	m_ctx = uci_alloc_context();
	if (!m_ctx)
	{
		fprintf(stderr, "Out of memory.\n");
		return ;
	}
	uci_set_savedir(m_ctx, UCI_SAVE_DIR);
	m_pkg  = NULL;
	m_ptr  = NULL;
	m_File = NULL;

	UCI_LoadFile(pFile);
}


UCI::~UCI()
{
	UCI_UnloadFile();
	if (!m_ctx)
	{
		uci_free_context(m_ctx);
		m_ctx = NULL;
	}
	if (!m_pkg)
		free(m_pkg), m_pkg = NULL;
	if (m_File) 
		free(m_File), m_File = NULL;
}

void 
UCI::UCI_SetConfDir(const char* dir)
{
	uci_set_confdir(m_ctx, dir);
}



int 
UCI::UCI_LoadFile(const char* File)
{
	m_File = strdup(File);
	return uci_load(m_ctx, File, &m_pkg);
}

int 
UCI::UCI_UnloadFile()
{
	return uci_unload(m_ctx, m_pkg);
}



char* 
UCI::UCI_GetOptionValue(const char* section, const char* option, bool &isList)
{
	struct uci_element *e = NULL;
	struct uci_option *o = NULL;
	struct uci_section *s = NULL;
	char temp[128] = {0};
	int i = 0;
	const char* p = section+1;

	if (NULL == section || NULL == option)
		return NULL;

	if (section[0] == '@')
	{
		while (*p != '[' && *p != '\0')temp[i++] = *p++;
		temp[i] = 0;
		++p, i=0;
		
		//printf("temp = %s, *p=%c\n", temp, *p);
		uci_foreach_element(&m_pkg->sections, e)
		{
			s = uci_to_section(e);
			if (s->anonymous != true) continue;
			if (strcmp(s->type, temp) == 0)
			{
				if (*p - '0' != i++) continue;
				
				o = uci_lookup_option(m_ctx, s, option);
				if (NULL == o) return NULL;

				if (UCI_TYPE_STRING == o->type)
					return (o->v.string);
				else if (UCI_TYPE_LIST == o->type)
					return (char*)&(o->v.list);
				else
					return NULL;
			}
		}
	}
	else
	{
		// 扫描一个文件中的区块
		s = uci_lookup_section(m_ctx, m_pkg, section);
		if (NULL == s) return NULL;

		// 扫描一个区块中的条目
		o = uci_lookup_option(m_ctx, s, option);
		if (NULL == o) return NULL;

		// 根据条目类型做处理
		if (UCI_TYPE_STRING == o->type)
			return (o->v.string);
		else if (UCI_TYPE_LIST == o->type)
			return (char*)&(o->v.list);
		else
			return NULL;
	}
	return NULL;
}


const char*
UCI::UCI_GetOptionValue(const char* section, const char* option)
{
	struct uci_option *o = NULL;
	struct uci_section *s = NULL;
	struct uci_element *e;
	char temp[128] = {0};
	int i = 0;
	const char* p = section+1;

	if (NULL == section || NULL == option)
		return NULL;

	if (section[0] == '@')
	{
		while (*p != '[' && *p != '\0')temp[i++] = *p++;
		temp[i] = 0;
		++p, i=0;
		
		//printf("temp = %s, *p=%c\n", temp, *p);
		uci_foreach_element(&m_pkg->sections, e)
		{
			s = uci_to_section(e);
			if (s->anonymous != true) continue;
			if (strcmp(s->type, temp) == 0)
			{
				if (*p - '0' != i++) continue;
				
				o = uci_lookup_option(m_ctx, s, option);
				if (NULL == o) return NULL;

				if (UCI_TYPE_STRING == o->type)
					return (o->v.string);
				else 
					return NULL;
			}
		}
	}
	else
	{
		s = uci_lookup_section(m_ctx, m_pkg, section);
		if (NULL == s) return NULL;
		o = uci_lookup_option(m_ctx, s, option);
		if (NULL == o) return NULL;

		if (UCI_TYPE_STRING == o->type)
			return (o->v.string);
		else 
			return NULL;
	}
	return NULL;
}


const struct uci_list*
UCI::UCI_GetListValue(const char* section, const char* option)
{
	struct uci_option *o = NULL;
	struct uci_section *s = NULL;
	struct uci_element *e;
	char temp[128] = {0};
	int i = 0;
	const char* p = section+1;

	if (NULL == section || NULL == option)
		return NULL;

	if (section[0] == '@')
	{
		while (*p != '[' && *p != '\0')temp[i++] = *p++;
		temp[i] = 0;
		++p, i=0;
		
		uci_foreach_element(&m_pkg->sections, e)
		{
			s = uci_to_section(e);
			if (s->anonymous != true) 
				continue;
			if (strcmp(s->type, temp) == 0)
			{
				if (*p - '0' != i++) continue;
				
				o = uci_lookup_option(m_ctx, s, option);
				if (NULL == o) 
					return NULL;

				if (UCI_TYPE_LIST == o->type)
					return &(o->v.list);
				else 
					return NULL;
			}
		}
	}
	else
	{
		s = uci_lookup_section(m_ctx, m_pkg, section);
		if (NULL == s) return NULL;

		o = uci_lookup_option(m_ctx, s, option);
		if (NULL == o) return NULL;

		if (UCI_TYPE_LIST == o->type)
			return &(o->v.list);
		else 
			return NULL;
	}
	return NULL;
}


// 函数功能: 设置相关配置值
int
UCI::UCI_SetOptionValue(const char* section, const char* option, const char* value)
{
	struct uci_option *o  = NULL;
	struct uci_section *s = NULL;
	struct uci_ptr ptr;

	struct uci_element *e = NULL;
	char temp[128] = {0};
	int i = 0;
	const char* p = section+1;
	
	if (NULL == section || NULL == option || NULL == value)
		return -1;

	memset(&ptr, 0, sizeof(struct uci_ptr));
	ptr.p = m_pkg;
	ptr.package = m_pkg->e.name;
	ptr.section = section;
	ptr.option  = option;
	ptr.value   = value;
	ptr.target  = UCI_TYPE_OPTION;

	if (section[0] == '@')
	{
		while (*p != '[' && *p != '\0')temp[i++] = *p++;
		temp[i] = 0;
		++p, i=0;

		uci_foreach_element(&m_pkg->sections, e)
		{
			s = uci_to_section(e);
			if (s->anonymous != true) continue;
			if (strcmp(s->type, temp) == 0)
			{
				if (*p - '0' != i++) continue;
				ptr.s = s;
				break;
			}
		}
	}
	else
	{
		s = uci_lookup_section(m_ctx, m_pkg, section);
		if (NULL == s)  
			return -1;
		ptr.s = s;
	}

	o = uci_lookup_option(m_ctx, s, option);
	if (NULL == o)
		return uci_set(m_ctx, &ptr);   // 添加
	ptr.o = o;
		
	if (o->type == UCI_TYPE_STRING) 
		return uci_set(m_ctx, &ptr);   // 修改
	else
		return -1;
}

int 
UCI::UCI_SetSectionValue(const char* section, const char* newSec)
{
	struct uci_section *s = NULL;
	struct uci_ptr ptr;

	struct uci_element *e = NULL;
	char temp[128] = {0};
	int i = 0;
	const char* p = section+1;
	
	if (NULL == section || NULL == newSec)
		return -1;
	
	memset(&ptr, 0, sizeof(struct uci_ptr));
	ptr.p = m_pkg;
	ptr.package = m_pkg->e.name;
	ptr.section = section;
	ptr.value = newSec;
	ptr.target  = UCI_TYPE_SECTION;

	if (section[0] == '@')
	{
		while (*p != '[' && *p != '\0')temp[i++] = *p++;
		temp[i] = 0;
		++p, i=0;

		uci_foreach_element(&m_pkg->sections, e)
		{
			s = uci_to_section(e);
			if (s->anonymous != true) continue;
			if (strcmp(s->type, temp) == 0)
			{
				if (*p - '0' != i++) continue;
				ptr.s = s;
				break;
			}
		}
	}
	else
	{
		s = uci_lookup_section(m_ctx, m_pkg, section);
		if (NULL == s)  return -1;
		ptr.s = s;
	}

	return uci_rename(m_ctx, &ptr);
}


int 
UCI::UCI_AddSection(const char* section, const char* name)
{
	if (NULL == section)
		return -1;
	
	struct uci_ptr ptr;
	memset(&ptr, 0, sizeof(struct uci_ptr));
	ptr.p = m_pkg;
	ptr.package = m_pkg->e.name;
	ptr.section = section;
	ptr.value   = name;
	ptr.target  = UCI_TYPE_SECTION; 

	return uci_set(m_ctx, &ptr);
}


int 
UCI::UCI_AddOption(const char* section, const char* option, const char* value)
{
	struct uci_section *s = NULL;
	struct uci_ptr ptr;

	struct uci_element *e = NULL;
	char temp[128] = {0};
	int i = 0;
	const char* p = section+1;

	if (NULL == section || NULL == option || NULL == value) 
		return -1;
	
	memset(&ptr, 0, sizeof(struct uci_ptr));
	ptr.p = m_pkg;
	ptr.package = m_pkg->e.name;
	ptr.section = section;
	ptr.option  = option;
	ptr.value   = value;
	ptr.target  = UCI_TYPE_OPTION; 

	if (section[0] == '@')
	{
		while (*p != '[' && *p != '\0')temp[i++] = *p++;
		temp[i] = 0;
		++p, i=0;

		uci_foreach_element(&m_pkg->sections, e)
		{
			s = uci_to_section(e);
			if (s->anonymous != true) continue;
			if (strcmp(s->type, temp) == 0)
			{
				if (*p - '0' != i++) continue;
				ptr.s = s;
				break;
			}
		}
	}
	else
	{
		s = uci_lookup_section(m_ctx, m_pkg, section);
		if (NULL == s)  return -1;
		ptr.s = s;
	}

	return uci_set(m_ctx, &ptr);
}


int 
UCI::UCI_DelSection(const char* section)
{
	struct uci_section *s = NULL;
	struct uci_ptr ptr;
	struct uci_element *e = NULL;
	char temp[128] = {0};
	int i = 0;
	const char* p = section+1;
	
	if (NULL == section)
		return -1;
	
	memset(&ptr, 0, sizeof(struct uci_ptr));
	ptr.p = m_pkg;
	ptr.package = m_pkg->e.name;
	ptr.section = section;
	ptr.target  = UCI_TYPE_SECTION; 

	if (section[0] == '@')
	{
		while (*p != '[' && *p != '\0')temp[i++] = *p++;
		temp[i] = 0;
		++p, i=0;

		uci_foreach_element(&m_pkg->sections, e)
		{
			s = uci_to_section(e);
			if (s->anonymous != true) continue;
			if (strcmp(s->type, temp) == 0)
			{
				if (*p - '0' != i++) continue;
				ptr.s = s;
				break;
			}
		}
	}
	else
	{
		s = uci_lookup_section(m_ctx, m_pkg, section);
		if (NULL == s)  return -1;
		ptr.s = s;
	}
	
	return uci_delete(m_ctx, &ptr);
}

int 
UCI::UCI_DelOption(const char* section, const char* option)
{
	struct uci_section *s = NULL;
	struct uci_ptr ptr;
	struct uci_element *e = NULL;
	char temp[128] = {0};
	int i = 0;
	const char* p = section+1;
	
	if (NULL == section || NULL == option) 
		return -1;

	memset(&ptr, 0, sizeof(struct uci_ptr));
	ptr.p = m_pkg;
	ptr.package = m_pkg->e.name;
	ptr.section = section;
	ptr.option  = option;
	ptr.target  = UCI_TYPE_OPTION; 

	if (section[0] == '@')
	{
		while (*p != '[' && *p != '\0')temp[i++] = *p++;
		temp[i] = 0;
		++p, i=0;

		uci_foreach_element(&m_pkg->sections, e)
		{
			s = uci_to_section(e);
			if (s->anonymous != true) continue;
			if (strcmp(s->type, temp) == 0)
			{
				if (*p - '0' != i++) continue;
				ptr.s = s;
				break;
			}
		}
	}
	else
	{
		s = uci_lookup_section(m_ctx, m_pkg, section);
		if (NULL == s)  return -1;
		ptr.s = s;
	}
	
	return uci_delete(m_ctx, &ptr);
}


int
UCI::UCI_SetListValue(const char* section, const char* list, const char* value, const char* newVal)
{
	int nRet;
	nRet = UCI_DelList(section, list, value);
	if ( 0 != nRet) return nRet;

	nRet = UCI_AddList(section, list, newVal);
	if ( 0 != nRet) return nRet;
	return 0;
}

int 
UCI::UCI_AddList(const char* section, const char* list, const char* value)
{
	struct uci_option *o = NULL;
	struct uci_section *s = NULL;
	struct uci_ptr ptr;
	struct uci_element *e = NULL;
	char temp[128] = {0};
	int i = 0;
	const char* p = section+1;
	
	if (NULL == section || NULL == list || NULL == value)
		return -1;
	
	memset(&ptr, 0, sizeof(struct uci_ptr));
	ptr.p = m_pkg;
	ptr.package = m_pkg->e.name;
	ptr.target  = UCI_TYPE_OPTION;
	ptr.option  = list;
	ptr.value   = value;

	if (section[0] == '@')
	{
		while (*p != '[' && *p != '\0')temp[i++] = *p++;
		temp[i] = 0;
		++p, i=0;

		uci_foreach_element(&m_pkg->sections, e)
		{
			s = uci_to_section(e);
			if (s->anonymous != true) continue;
			if (strcmp(s->type, temp) == 0)
			{
				if (*p - '0' != i++) continue;
				ptr.s = s;
				break;
			}
		}
	}
	else
	{
		s = uci_lookup_section(m_ctx, m_pkg, section);
		if (NULL == s)  return -1;
		ptr.s = s;
	}
	
	o = uci_lookup_option(m_ctx, s, list);
	if (NULL == o) return -1;
	ptr.o = o;
	ptr.o->type = UCI_TYPE_LIST;
	
	return uci_add_list(m_ctx, &ptr);		
}

int 
UCI::UCI_DelList(const char* section, const char* list, const char* value)
{
	struct uci_option *o = NULL;
	struct uci_section *s = NULL;
	struct uci_ptr ptr;
	struct uci_element *e = NULL;
	char temp[128] = {0};
	int i = 0;
	const char* p = section+1;
	
	if (NULL == section || NULL == list || NULL == value)
		return -1;

	memset(&ptr, 0, sizeof(struct uci_ptr));
	ptr.p = m_pkg;
	ptr.package = m_pkg->e.name;
	ptr.target  = UCI_TYPE_OPTION;
	ptr.option  = list;
	ptr.value   = value;

	if (section[0] == '@')
	{
		while (*p != '[' && *p != '\0')temp[i++] = *p++;
		temp[i] = 0;
		++p, i=0;

		uci_foreach_element(&m_pkg->sections, e)
		{
			s = uci_to_section(e);
			if (s->anonymous != true) continue;
			if (strcmp(s->type, temp) == 0)
			{
				if (*p - '0' != i++) continue;
				ptr.s = s;
				break;
			}
		}
	}
	else
	{
		s = uci_lookup_section(m_ctx, m_pkg, section);
		if (NULL == s)  return -1;
		ptr.s = s;
	}

	o = uci_lookup_option(m_ctx, s, list);
	if (NULL == o) return -1;
	ptr.o = o;
	ptr.o->type = UCI_TYPE_LIST;

	return uci_del_list(m_ctx, &ptr);
}



void 
UCI::UCI_Commit()
{
	uci_commit(m_ctx, &m_pkg, false);
}

int
UCI::UCI_Save()
{
	return uci_save(m_ctx, m_pkg);
}


void 
UCI_PrintList(struct uci_list* List)
{
	struct uci_element *e;
	uci_foreach_element(List, e) 
	{
		printf("%s\n", e->name);
	}
	return;
}



/*
int main()
{
	UCI *uciFile = new UCI("/root/");
	int nRet = 0;
	nRet = uciFile->UCI_LoadFile("./network");
	if (0 != nRet)
	{
		delete uciFile, uciFile = NULL;
		printf("nRet=%d\n", nRet);
		return 0;
	}

	// 测试匿名模式
	const char* Switch = uciFile->UCI_GetOptionValue("@switch[0]", "name");
	printf("Switch.name=%s\n", Switch);
	uciFile->UCI_SetOptionValue("@switch[0]", "name", "Switch0");
	
	bool isList = false;
	char* ip = uciFile->UCI_GetOptionValue("lan", "ipaddr", isList);
	if (isList == false)
		printf("ip=%s\n", ip);
	else
		UCI_PrintList((struct uci_list*)ip);

	// 测试修改option值，option type bridge->option type dhcp
	nRet = uciFile->UCI_SetOptionValue("lan", "type", "dhcp");
	if (nRet != 0)
	{
		printf("[%s][%d]-------Error------\n", __FILE__, __LINE__);
	}
	// 测试修改section值，config interface lan->config interface lan4
	nRet = uciFile->UCI_SetSectionValue("lan", "lan4");
	if (nRet != 0)
	{
		printf("[%s][%d]-------Error-------\n", __FILE__, __LINE__);
	}

	// 测试增加section, config peaple hxw
	uciFile->UCI_AddSection("hxw", "peaple");
	
	// 测试增加option, option age 27
	uciFile->UCI_AddOption("hxw", "age", "27");
	uciFile->UCI_AddOption("wan", "ipaddr", "192.168.8.1");
	uciFile->UCI_AddOption("wan", "macaddr", "11:22:33:44:55:66");

	// 测试删除section，config interface 'wan6'
	uciFile->UCI_DelSection("wan6");
	// 测试删除option，option
	uciFile->UCI_DelOption("wan", "macaddr");

	// 测试删除匿名section，config switch_vlan
	uciFile->UCI_DelSection("@switch_vlan[0]");
	// 测试删除匿名section的option，option reset '1' 
	uciFile->UCI_DelOption("@switch[0]", "reset");
	// 测试增加匿名section的option，
	uciFile->UCI_AddOption("@switch[0]", "macaddr", "11:22:33:44:55:66");

	
	// 测试获取队列
	char* ifname = uciFile->UCI_GetOptionValue("wan", "ifname", isList);
	if (ifname)
		UCI_PrintList((struct uci_list*)ifname);
	else
		printf("ifname=%s\n", ifname);

	// 测试增加队列
	uciFile->UCI_AddList("wan", "ifname", "eth0.2");
	uciFile->UCI_AddList("wan", "ifname", "eth0.3");
	uciFile->UCI_AddList("wan", "ifname", "eth0.4");
	// 测试删除队列
	uciFile->UCI_DelList("wan", "ifname", "apcli1");
	// 测试修改队列值
	uciFile->UCI_SetListValue("wan", "ifname", "apcli2", "eth0.5");

	
	// 测试获取匿名section队列
	ifname = uciFile->UCI_GetOptionValue("@switch[0]", "ifname", isList);
	if (ifname)
		UCI_PrintList((struct uci_list*)ifname);
	else
		printf("ifname=%s\n", ifname);

	uciFile->UCI_AddList("@switch[0]", "ifname", "eth0.2");
	uciFile->UCI_AddList("@switch[0]", "ifname", "eth0.3");

	uciFile->UCI_DelList("@switch[0]", "ifname", "eth1");


	uciFile->UCI_Commit();
	delete uciFile, uciFile = NULL;
	printf("----------------delete-----------\n");
	return 0;
}
*/
#endif

