#ifndef __UCI_H__
#define __UCI_H__
#include <stdio.h>
#include <uci.h>


#define UCI_SAVE_DIR    "/tmp"

class UCI
{
public:
	UCI();
	UCI(const char* dir);
	~UCI();

public:
	// 函数功能: 设置扫描路径
	void 
	UCI_SetConfDir(const char* dir);
	// uci commit
	void 
	UCI_Commit();


	// 函数功能: uci 加载配置文件
	// 函数参数: File，文件路径
	// 返 回 值: 返回0成功
	int 
	UCI_LoadFile(const char* File);
	// 函数功能: uci 卸载配置文件
	int 
	UCI_UnloadFile();


	// 函数功能: 读取相关配置值，对option的类型未知，可能是option也可能是list
	// 函数参数: section: 区块名字
	//           option:  条目名字
	//           isList，输出参数，判断是否为list类型
	// 返 回 值: isList == true，返回链表地址，
	//           isList == false，返回option的值，失败返回NULL
	char* 
	UCI_GetOptionValue(const char* section, const char* option, bool &isList);
	// 函数功能: 读取相关配置值，option类型
	// 函数参数: section: 区块名字
	//           option:  条目名字
	// 返 回 值: 返回option的值，失败返回NULL
	const char*
	UCI_GetOptionValue(const char* section, const char* option);
	// 函数功能: 读取相关配置值，list类型
	// 函数参数: section: 区块名字
	//           option:  条目名字
	// 返 回 值: 返回链表地址，失败返回NULL
	const struct uci_list*
	UCI_GetListValue(const char* section, const char* option);
	

	// 函数功能: 修改option配置值
	// 函数参数: section: 区块名字
	//           option:  条目名字
	//           value:   条目值
	// 返 回 值: 成功0，失败 -1；
	int
	UCI_SetOptionValue(const char* section, const char* option, const char* value);
	// 函数功能: 修改section配置值
	// 函数参数: section: 区块名字
	//           value:   条目值
	// 返 回 值: 成功0，失败 -1；
	int 
	UCI_SetSectionValue(const char* section, const char* newSec);
	


	// 函数功能: 增加区块配置，不支持匿名
	// 函数参数: section: 区块
	//           name:  区块名字
	// 返 回 值: 成功0，非0失败
	int 
	UCI_AddSection(const char* section, const char* name);
	// 函数功能: 增加区块条目
	// 函数参数: section: 区块
	//           option:  条目名字
	//           name: 条目值
	// 返 回 值: 成功0，非0失败
	int 
	UCI_AddOption(const char* section, const char* option, const char* value);


	// 函数功能: 删除区块配置
	// 函数参数: section: 区块
	//           name:  区块名字
	// 返 回 值: 成功0，非0失败
	int 
	UCI_DelSection(const char* section);
	// 函数功能: 删除区块条目
	// 函数参数: section: 区块
	//           option:  条目名字
	// 返 回 值: 成功0，非0失败
	int 
	UCI_DelOption(const char* section, const char* option);


	// 函数功能: 修改list配置值
	// 函数参数: section: 区块名字
	//           list:    条目名字
	//           value:   条目值
	//           newVal:  新的条目值
	// 返 回 值: 成功0，失败 -1；
	int
	UCI_SetListValue(const char* section, const char* list, const char* value, const char* newVal);
	// 函数功能: 添加list配置值
	// 函数参数: section: 区块名字
	//           list:    条目名字
	//           value:   条目值
	// 返 回 值: 成功0，失败 -1；
	int 
	UCI_AddList(const char* section, const char* list, const char* value);
	// 函数功能: 删除list配置值
	// 函数参数: section: 区块名字
	//           list:    条目名字
	//           value:   条目值
	// 返 回 值: 成功0，失败 -1；
	int 
	UCI_DelList(const char* section, const char* list, const char* value);
	
private:
	struct uci_context *m_ctx;
	struct uci_ptr     *m_ptr;   
	struct uci_package *m_pkg;
	char* m_File;

private:
	char* 
	uci_get_value(struct uci_option *o);
};


void UCI_PrintList(struct uci_list* List);

#endif /*__UCI_H__*/


