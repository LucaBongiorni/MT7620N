#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <errno.h>
#include <iostream>

#ifdef WIN32
#include <Windows.h>
#pragma comment(lib, "pthreadVC2.lib")
#pragma comment(lib,"ws2_32.lib")
#else
#include <pthread.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if_arp.h>
#endif

#include "defCom.h"
#include "confile.h"
#include "StrOperate.h"
#include "netSockTcp.h"
#include "procotol.h"
#include "listStruct.h"
#include "cbProjMain.h"
#include "socketClient.h"
#include "pthreadCom.h"
#include "beaconConf.h"
#include "cJSON.h"
#include "rbtree.h"
#include "thread_pool.h"


#ifdef UPDATEBLUEBIN
#include "upBToothBin.h"
#endif

#ifdef UPLOADMACINFO
#include "gatherMac.h"
#endif

#ifdef LOCALSOCKET  
#include "pthreadServer.h"
#endif

#ifdef SERIALSCOM
#include "ble_central.h"
#include "SerialCom.h"
#endif 

#ifdef BROADCAST
#include "broadcast.h"
#endif

using namespace std;

cloudBeaconMain *g_main = NULL;


CTask::CTask()
{
	m_tree = (TaskTree*)malloc(TaskTreeLen);
	pthread_mutex_init(&m_tree->mutex, NULL);
	m_tree->treeRoot = RB_ROOT;
	m_tree->taskNum  = 0;
	m_tree->freeNode = freeNode;
}




CTask::~CTask()
{
	if (m_tree)
	{
		destroyAllNode();
		free(m_tree), m_tree = NULL;
	}
}

TaskNode* CTask::getMinTime(void)
{
	TaskNode* node = NULL;
	struct rb_node* ptemp = NULL;
	ptemp = rb_first(&m_tree->treeRoot);
	node  = rb_entry(ptemp, TaskNode, treeNode);
	return node;
}

TaskNode* CTask::getMaxTime(void)
{
	TaskNode* node = NULL;
	struct rb_node* ptemp = NULL;
	ptemp = rb_last(&m_tree->treeRoot);
	node  = rb_entry(ptemp, TaskNode, treeNode);
	return node;
}

void CTask::freeNode(TaskNode* node)
{
	if (node->arg) free(node->arg);
	if (node) free(node);
}

TaskNode* CTask::createNode(int id, int interTime, 
	void (*pHandle)(void* arg), void* parg)
{
	TaskNode* _new = (TaskNode*)malloc(TaskNodeLen);
	if (_new)
	{
		memset(_new, 0, TaskNodeLen);
		if (parg)
			_new->arg     = strdup((const char*)parg);
		else
			_new->arg     = NULL;
		
		//_new->interTime   = interTime;
		_new->pHandle     = pHandle;
		_new->executeTime = getCurTime() + interTime;
		_new->id          = id;
	}
	return _new;
}


void CTask::destroyAllNode(void)
{
	TaskNode* pNode = NULL;
	struct rb_node* p = NULL;//rb_first(&m_tree->treeRoot);
	//for (; p; p=rb_next(p))
	while(p=rb_first(&m_tree->treeRoot))
	{
		pNode = rb_entry(p, TaskNode, treeNode);
		rb_erase(p, &m_tree->treeRoot);
		m_tree->freeNode(pNode);
		pNode = NULL;
	}
}



int CTask::delOneTask(int id)
{
	TaskNode* pNode = getTask(id);
	if (NULL == pNode) 
		return -1;

	rb_erase(&pNode->treeNode, &m_tree->treeRoot);
	m_tree->freeNode(pNode);
	return 0;
}

TaskNode* CTask::getTask(int id)
{
	struct rb_node* p = rb_first(&m_tree->treeRoot);
	for (; p; p=rb_next(p))
	{
		TaskNode* node = rb_entry(p, TaskNode, treeNode);
		if (node->id == id) 
			return node;
	}
	return NULL;
}

int CTask::addOneTask(int id, 
	void (*pTimeOut)(void* arg), void *arg)
{
	int timeOut = getInterTimeByID(id);
	if (-1 == timeOut) 
	{
		Debug(I_ERROR, "timeOut=-1");
		return -1;
	}
		
	TaskNode* pNode = createNode(id, timeOut, pTimeOut, arg);
	if (NULL == pNode) return -1;

	struct rb_node **_new = &(m_tree->treeRoot.rb_node);
	struct rb_node *parent = NULL;

	unsigned long long key = pNode->executeTime;
	while (*_new) 
	{
		parent = *_new;
		if (key < rb_entry(parent, TaskNode, treeNode)->executeTime)
			_new = &parent->rb_left;
		else
			_new = &parent->rb_right;
	}

	rb_link_node(&pNode->treeNode, parent, _new);
	rb_insert_color(&pNode->treeNode, &m_tree->treeRoot);
	return 0;
}

int CTask::addOneTask(TaskNode* tasker)
{
	if (! tasker) return -1;
	TaskNode* pNode = (TaskNode*)malloc(TaskNodeLen);
	if (! pNode) return -1;

	pNode->id = tasker->id;
	pNode->executeTime     = getCurTime() + getInterTimeByID(tasker->id);
	//pNode->interTime     = tasker->interTime;
	//pNode->pCheckInterTime = tasker->pCheckInterTime;
	//pNode->pGetInterTime = tasker->pGetInterTime;
	pNode->pHandle         = tasker->pHandle;
	if (! tasker->arg)
		pNode->arg         = strdup((const char*)tasker->arg);
	else 
		pNode->arg         = NULL;

	struct rb_node **_new  = &(m_tree->treeRoot.rb_node);
	struct rb_node *parent = NULL;

	unsigned long long key = pNode->executeTime;
	while (*_new) 
	{
		parent = *_new;
		if (key < rb_entry(parent, TaskNode, treeNode)->executeTime)
			_new = &parent->rb_left;
		else
			_new = &parent->rb_right;
	}

	rb_link_node(&pNode->treeNode, parent, _new);
	rb_insert_color(&pNode->treeNode, &m_tree->treeRoot);
	return 0;
}

int CTask::changeOneTask(TaskNode* pNode, 
	unsigned long long curTime, int timeOut)
{
	rb_erase(&pNode->treeNode, &m_tree->treeRoot);
	pNode->executeTime = curTime + timeOut;

	struct rb_node **_new = &(m_tree->treeRoot.rb_node);
	struct rb_node *parent = NULL;

	unsigned long long key = pNode->executeTime;
	while (*_new) 
	{
		parent = *_new;
		if (key < rb_entry(parent, TaskNode, treeNode)->executeTime)
			_new = &parent->rb_left;
		else
			_new = &parent->rb_right;
	}

	rb_link_node(&pNode->treeNode, parent, _new);
	rb_insert_color(&pNode->treeNode, &m_tree->treeRoot);
	return 0;
}


void CTask::checkAddTask(void)
{
	struct rb_node* p = rb_first(&m_tree->treeRoot);
	for (; p; p=rb_next(p))
	{
		TaskNode* node = rb_entry(p, TaskNode, treeNode);
		Debug(I_ERROR, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
		Debug(I_ERROR, "node->id=%d", node->id);
		Debug(I_ERROR, "node->executeTime=%lld", node->executeTime);
		//printf("node->interTime=%d\n", node->interTime);
		node->pHandle(NULL);
	}
	Debug(I_ERROR, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
}

unsigned long long CTask::getCurTime(void)
{
	struct timeval now;
	memset(&now, 0, sizeof(now));
	gettimeofday(&now, 0);

	return (((unsigned long long)now.tv_sec) * 1000 + (now.tv_usec / 1000));
}



int CTask::getInterTimeByID(int id)
{
	IbeaconConfig* conf = GetConfigPosition();
	switch(id)
	{
	case GATH_MAC_TASK_ID:
		return (5*1000);
	case UPLD_MAC_TASK_ID:
		return (1000 * conf->getMacInterval());
	case UPDT_BTH_TASK_ID:
		return (60*1000);
	case CHEK_LOG_FILE_ID:
		return (60*60*1000);
	case CHEK_NET_CONN_ID:
		return (30*1000);
	case PRINT_COM_CNT1_ID:
		return (1*1000);
	case PRINT_COM_CNT10_ID:
		return (10*1000);
	default:
		return -1;
	}
}


void 
cloudBeaconMain::usage(char* arg)
{
    printf("Usage: %s [options]\n", arg);
    printf("\n");
    printf("  -c [filename] Use this config file.\n");
    printf("  -h            Print usage.\n");
    printf("  -v            Print version information.\n");
    printf("  -p            Local server listen port.\n");
    printf("  -b            Local broadcast listen port.\n");
    printf("  -n            Save Node Number.\n");
    printf("  -H            Web server domain.\n");
    printf("  -P            Web sever port.\n");
    printf("\n");
}

void 
cloudBeaconMain::parseComLineAndConfFile(int argc, char **argv)
{
    int c, nRet;
	bool webDomainFlag    = false;
	bool confFilePathFlag = false;
	bool net2serNumFlag   = false;
	char temp[256] = {0};
	//char webDomain[DOMAIN_LEN] = {0};
	//char url[URL_LEN] = {0};
	//char WiFiIfname[16] = {0};
	//char LanIfname[16]  = {0};
	//char WanIfname[16]  = {0};
	char phoneKey[PHONE_KEY_LEN+1] = {0};
	char UpdateFilePath[64] = {0};
	//char* pMac = NULL;
	//u_int32 tempIP;

    while (-1 != (c = getopt(argc, argv, "c:hvP:p:H:n:")))
    {
		switch(c) 
		{
			case 'h':
				usage(argv[0]);
				exit(1);
				break;
				
			case 'c':
				if (optarg) 
				{
					m_config->setConfFilePath(optarg);
					confFilePathFlag = true;
				}
				else
				{
					usage(argv[0]);
					exit(1);
				}
				break;
				
			case 'v':
				printf("%s version: %s %s\n", argv[0], __VERDATA__, __VERTIME__);
				exit(1);
				break;

			case 'P':
				if (optarg)
				{
					m_config->setWebPort(atoi(optarg));
				}
				break;

			case 'p':
				if (optarg)
				{
					m_config->setLocalPort(atoi(optarg));
				}
				break;

			case 'b':
				if (optarg)
				{
					m_config->setBcastPort(atoi(optarg));
				}
				break;

			case 'H':
				if (optarg)
				{
					m_config->setWebDomain(optarg);
					webDomainFlag = true;
				}
				break;
			case 'n':
				if (optarg)
				{
					setNet2SerNodeNum(atoi(optarg));
					net2serNumFlag = true;
				}
				break;
				
			default:
				usage(argv[0]);
				exit(1);
				break;
		}
    }

	if (false == confFilePathFlag)
	{
		m_config->setConfFilePath(DEF_CONFIG_FILE_PATH);
	}

	// 加载配置文件
	if (!CConfFile::Init())
	{
		Debug(I_ERROR, "class CConfFile init failed.");
		exit(1);
	}
	CConfFile* confFile = new CConfFile();
	if (!confFile->LoadFile(m_config->getConfFilePath()))
	{
		Debug(I_ERROR, "Open %s failure.", m_config->getConfFilePath());
		delete confFile;
		exit(1);
	}
	
	if (false == webDomainFlag)
	{
		confFile->GetString("WebServerInfo", "Host", temp);
		m_config->setWebDomain(temp);
	}
	if (m_config->getWebPort() == 0)
	{
		m_config->setWebPort(confFile->GetInt("WebServerInfo", "Port"));
	}
	if (m_config->getLocalPort() == 0)
	{
		m_config->setLocalPort(confFile->GetInt("LocalServerInfo", "Port"));
	}
	if (m_config->getBcastPort() == 0)
	{
		m_config->setBcastPort(confFile->GetInt("LocalServerInfo", "BcastPort"));
	}
	m_config->setListenNum(confFile->GetInt("LocalServerInfo", "ListenNum"));
	if (false == net2serNumFlag)
	{
		setNet2SerNodeNum(confFile->GetInt("LocalServerInfo", "net2serNodeNum"));
	}

	
	// tcp 上传mac地址信息和beacon信息功能是否开启
	if (0 == confFile->GetInt("LocalServerInfo", "OpenUploadMacInfo"))
	{
		m_config->setTCPMacSerOpenVal(false);
	}
	else
	{
		m_config->setTCPMacSerOpenVal(true);
	}
	if (0 == confFile->GetInt("LocalServerInfo", "OpenUploadBeaconInfo"))
	{
		m_config->setTCPBeaconSerOpenVal(false);
	}
	else
	{
		m_config->setTCPBeaconSerOpenVal(true);
	}
	
	// tcp上传间隔时间
	m_config->setMacInterval(confFile->GetInt("LocalServerInfo", "MacInterval"));
	m_config->setBeaconInterval(confFile->GetInt("LocalServerInfo", "BeaconInterval"));
	//config->setMacSerInterval(config->getMacInterval());
	//config->setBeaconSerInterval(config->getBeaconInterval());

	// 配置文件路径
	confFile->GetString("LocalServerInfo", "UpdateFilePath", UpdateFilePath);

	// 开启扫描设备功能
	if (confFile->GetInt("LocalServerInfo", "OpenStartScanDev") != 0)
	{
		m_config->setOpenStartScanDev();
	}
	else
	{
		m_config->setCloseStartScanDev();
	}

	// 客户服务器信息
	if (confFile->GetInt("CustomServerInfo", "OpenMacSer") != 0)
	{
		// 开启
		m_config->setIsOpenMacSer(true);
		confFile->GetString("CustomServerInfo", "MacSerHost", temp);
		m_config->setMacSerHost(temp);
		confFile->GetString("CustomServerInfo", "MacSerUrl", temp);
		m_config->setMacSerUrl(temp);
		m_config->setMacSerPort(confFile->GetInt("CustomServerInfo", "MacSerPort"));
		//config->setMacSerInterval(confFile->GetInt("CustomServerInfo", "MacSerInterval"));
	}
	else
	{
		// 没有开启
		m_config->setIsOpenMacSer(false);
	}

	if (confFile->GetInt("CustomServerInfo", "OpenBeaconSer") != 0)
	{
		// 开启
		m_config->setIsOpenBeaconSer(true);
		confFile->GetString("CustomServerInfo", "BeaconSerHost", temp);
		m_config->setBeaconSerHost(temp);
		confFile->GetString("CustomServerInfo", "BeaconSerUrl", temp);
		m_config->setBeaconSerUrl(temp);
		m_config->setBeaconSerPort(confFile->GetInt("CustomServerInfo", "BeaconSerPort"));
		//config->setBeaconSerInterval(confFile->GetInt("CustomServerInfo", "BeaconSerInterval"));
	}
	else
	{
		// 没有开启
		m_config->setIsOpenBeaconSer(false);
	}

	// 获取串口名字
	confFile->GetString("Application", "DefaultCom", temp);
	m_config->setComName(temp);

	// 获取日志打印级别
	confFile->GetString("Application", "debuglevel", temp);
	m_config->debuglevel = atoi(temp);
		
	// 提取更新蓝牙配置文件信息
	confFile->GetString("updateBlueBin", "host", temp);
	m_config->setUpdateBlueBinHost(temp);
	confFile->GetString("updateBlueBin", "url", temp);
	m_config->setUpdateBlueBinUrl(temp);
	m_config->setUpdateBlueBinPort(confFile->GetInt("updateBlueBin", "port"));
	delete confFile, confFile = NULL;

	// 提取版本号
	confFile = new CConfFile();
	if (!confFile->LoadFile(UpdateFilePath))
	{
		Debug(I_ERROR, "Open conf file %s failed.", UpdateFilePath);
		delete confFile, confFile = NULL;
		exit(1);
	}
	m_config->setVersion(confFile->GetInt("UpdateInfo", "UpdateVersion"));
	delete confFile, confFile = NULL;

	// 加载phoneKey
	if (0 == access(DEF_PHONE_CONNECT_KEY, F_OK))
	{
		ReadFileNLenToMem(phoneKey, PHONE_KEY_LEN, DEF_PHONE_CONNECT_KEY);
		phoneKey[PHONE_KEY_LEN] = 0;
		m_config->setPhoneKey(phoneKey);
	}
	
/*	
	// 加载serilas
	if (0 == access(DEF_UID_FILE_PATH, F_OK))
	{
		pMac = LoadFileToMem(DEF_UID_FILE_PATH, &nRet);
		config.setSerials(pMac);
		free(pMac), pMac = NULL;
	}
	else
	{
		getBlueMacAddr(temp);
		strcat(temp, ":");
		strcat(temp, config.getWanMAC());
		//nRet = snprintf(temp, 256, "%s:%s", temp, config.getWanMAC());
		config.setSerials(temp);
		// 写到配置文件
		LoadMemToFile(temp, strlen(temp), DEF_UID_FILE_PATH);
	}
*/

	Debug(I_INFO, "WebServerHost=%s, Port=%d", 
		m_config->getWebDomain(), m_config->getWebPort());
	Debug(I_INFO, "LocalPort=%d, BcastPort=%d, ListenNum=%d", 
		m_config->getLocalPort(), m_config->getBcastPort(), m_config->getListenNum());
	Debug(I_INFO, "upIbeaconInfo=%d, openBlueDev=%d, upMacInfo=%d", 
		m_config->getTCPBeaconSerOpenVal() ? 1:0,
		m_config->getIsStartScanDev() ? 1:0, 
		m_config->getTCPMacSerOpenVal() ? 1:0);
	return ;
}


IbeaconConfig* GetConfigPosition()
{
	return g_main->m_config;
}

#ifdef 	SERIALSCOM
int cloudBeaconMain::setSerials()
{
	char temp[64] = {0};
	int nRet; 
	// 设置序列号
	if ( -1 == getBlueMacAddr(temp) )
	{
		if (0 == access(DEF_UID_FILE_PATH, F_OK))
		{
			char* buff = NULL;
			buff = LoadFileToMem(DEF_UID_FILE_PATH, &nRet);
			if (!buff) return -1;
			if (0 == strcmp(buff, "00:00:00:00:00:00"))
			{
				free(buff), buff = NULL;
				return -1;
			}
			strncpy(temp, buff, sizeof(temp));
			strcat(temp, ":");
			strcat(temp, GetLanMac());
			m_config->setSerials(temp);
			if (buff) free(buff), buff = NULL;
			return 0;
		}
		else
		{
			resetBlueCentral();
			return -1;
		}
	}
	strcat(temp, ":");
	strcat(temp, GetLanMac());
	m_config->setSerials(temp);
	// printf("serials=%s\n", temp);
	// 写到配置文件
	LoadMemToFile(temp, strlen(temp), DEF_UID_FILE_PATH);
	return 0;
}

BleCentral *getCentralPosition()
{
	return g_main->m_central;
}

int cloudBeaconMain::startBlueCentral()
{
	bool tmp = m_central->ReloadConfig(DEF_BLU_CONF_FILE);
	if (!tmp)
	{
		Debug(I_ERROR, "central load configuration failed!");
		UninitSerialsCom();
		return 0;
	}
	
	//central->SetReportEnable(true);
	//central->SetReportInterval(10);
	//central->BleCentralReset();
	//sleep(5);

	if (!m_central->Start())
	{
		printf("central start failed!\n");
		m_central->Stop();
		UninitSerialsCom();
		return 0;
	}
	return 0;
}

void cloudBeaconMain::endBlueCentral()
{
	m_central->Stop();
	UninitSerialsCom();
}

// 重新加载蓝牙配置文件
int cloudBeaconMain::reloadBlueConfFile()
{
	if (!m_central->ReloadConfig(DEF_BLU_CONF_FILE))
	{
		Debug(I_DEBUG, "central load configuration failed!\n");
		//central->BLEScanningStop();
		return -1;
	}
	return 0;
}

int reloadBlueConfFile()
{
	return g_main->reloadBlueConfFile();
}


int cloudBeaconMain::resetBlueCentral()
{
	BleCommand bleCmd;
	memset(&bleCmd, 0, sizeof(bleCmd));
	return (m_central->BleResetCentral(bleCmd) ?  0 : -1);
}


// 获取蓝牙mac地址
int cloudBeaconMain::getBlueMacAddr(char* mac)
{
	int i;
	struct BleAddrInfo addr;
	memset(&addr, 0, sizeof(addr));

	m_central->GetBLEAddress(addr);
	
	for (i=0; i<6; ++i)
		if (addr.addr[i] != 0) break;
	if (6 == i) return -1;

	snprintf(mac, 32, "%02x:%02x:%02x:%02x:%02x:%02x", 
		addr.addr[0], addr.addr[1], addr.addr[2], 
		addr.addr[3], addr.addr[4], addr.addr[5]);
	
	printf("@@@@@@@@@@@@@@@@@@@@@@\n");
	printf("blue mac=%s\n", mac);
	printf("@@@@@@@@@@@@@@@@@@@@@@\n");
	return 0;
}
#endif


int cloudBeaconMain::broadcastPthreadStart()
{
	if (NULL == m_phoneBcast)
	{
		m_phoneBcast = new CBcast;
		if (NULL == m_phoneBcast) return -1;
		m_phoneBcast->bcastPthreadRun();
	}
	return 0;
}

void cloudBeaconMain::broadcastPthreadExit()
{
	if (m_phoneBcast)
	{
		m_phoneBcast->bcastPthreadExit();
		delete m_phoneBcast, m_phoneBcast = NULL;
	}
}



int cloudBeaconMain::conWebSerProcStart()
{
	if (NULL == m_conWebSer)
	{
		m_conWebSer = new CConWebSerProc;
		if (NULL == m_conWebSer) 
			return -1;
		return m_conWebSer->conWebSerProcRun();
	}
	return 0;
}

void cloudBeaconMain::conWebSerProcExit()
{
	if (m_conWebSer)
	{
		m_conWebSer->conWebSerProcExit();
		delete m_conWebSer, m_conWebSer = NULL;
	}
}


void doExitProgram(int signo)
{
	if (SIGINT == signo)
	{
		//UninitNet2SerList(GetNet2SerHead());
	#ifdef 	SERIALSCOM
		g_main->endBlueCentral();
	#endif
	
	#ifdef BROADCAST
		g_main->broadcastPthreadExit();
	#endif

	#ifdef WEBSERVER
		g_main->conWebSerProcExit();
	#endif

		phoneSerPthreadExit();

		pthread_rwlock_wrlock(&g_main->m_exitLock);
		g_main->m_isExitMain = true;
		pthread_rwlock_unlock(&g_main->m_exitLock);
		
		//UninitNet2SerList(GetNet2SerHead());
		//if (g_main) delete g_main, g_main = NULL;
		Debug(I_INFO, "exit after 3 seconds...\n");
		sleep(3);
	} 
}

void 
cloudBeaconMain::run(int argc, char** argv)
{
	int nRet = -1;
	int timeOut = 0;
	unsigned long long cur;
	//pthread_t localSocketID;

	// 设置信号
	signal(SIGINT, doExitProgram);
    // signal(SIGTERM, doExitProgram);
	parseComLineAndConfFile(argc, argv);
	(void)signal(SIGPIPE, SIG_IGN);

	ListManage* net2SerHead = GetNet2SerHead();
	InitNet2SerList(net2SerHead);

#ifdef SERIALSCOM  // 初始化串口
	if (-1 == InitSerialsCom(m_config->getComName()))
	{
		Debug(I_ERROR, "Init serials failed.");
		UninitNet2SerList(net2SerHead);
		return;
	}
	m_central = new BleCentral;
	
	startBlueCentral();
	// 设置序列号
	if ( -1 == setSerials())
	{
		Debug(I_ERROR, "set serials failed.");
		UninitNet2SerList(net2SerHead);
		if (m_central) delete m_central, m_central = NULL;
		return;
	}	
#else
	if (0 == access(DEF_UID_FILE_PATH, F_OK))
	{
		char *pMac = NULL;
		pMac = LoadFileToMem(DEF_UID_FILE_PATH, &nRet);
		if (pMac)
		{
			m_config->setSerials(pMac);
			free(pMac), pMac = NULL;
		}
	}
	else
	{
		char serials[64] = {0};
		strncpy(serials, GetLanMac(), 64);
		strcat(serials, ":");
		strcat(serials, GetLanMac());
		m_config->setSerials(serials);
	}
#endif

#ifdef BROADCAST    // 开启一个广播服务器线程
	broadcastPthreadStart();
#endif

#ifdef WEBSERVER
	conWebSerProcStart();
#endif

#ifdef LOCALSOCKET
	phoneSerPthreadStart();
#endif

#ifdef SERIALSCOM
	m_central->ResumeScanning();
#endif
	
#if defined USE_OPENWRT && defined UPLOADMACINFO
	m_task->addOneTask(GATH_MAC_TASK_ID, gatherMacTask, NULL);
	m_task->addOneTask(UPLD_MAC_TASK_ID, uploadMacTask, NULL);
#endif
#if defined UPDATEBLUEBIN && defined SERIALSCOM
	m_task->addOneTask(UPDT_BTH_TASK_ID, updateBluetoothBinTask, NULL);
#endif

#if 0
	m_task->addOneTask(PRINT_COM_CNT1_ID, printReadComCnt1, NULL);
	m_task->addOneTask(PRINT_COM_CNT10_ID, printReadComCnt2, NULL);
#endif

	// 添加检测日志文件任务
	m_task->addOneTask(CHEK_LOG_FILE_ID, checkLogFileSize, NULL);
	m_task->checkAddTask();
	
	TaskNode* pTemp = NULL;

	//sleep(10);
	//m_central->UpdateFirmware(DEF_BLU_BIN_FILE);
	
	// 主函数轮询任务
	for (;;)
	{
		pthread_rwlock_rdlock(&g_main->m_exitLock);
		if (m_isExitMain)
		{
			pthread_rwlock_unlock(&g_main->m_exitLock);
			break;
		}
		pthread_rwlock_unlock(&g_main->m_exitLock);
		cur = CTask::getCurTime();
		pTemp = m_task->getMinTime();
		if (pTemp && cur >= pTemp->executeTime)
		{
			pTemp->pHandle(pTemp->arg);
			// 重新获取时间
			timeOut = m_task->getInterTimeByID(pTemp->id);
			// 改变执行时间
			m_task->changeOneTask(pTemp, cur, timeOut);
		}
		usleep(200);   // 休眠0.2毫秒，加上代码运行时间，保证误差不大于1毫秒
	}

	UninitNet2SerList(net2SerHead);
	//doExitProgram(SIGINT);
	return ;
}


int main(int argc, char** argv)
{
	// 将进程设置为分离状态
	detachPid();
	FILE* logFd = fopen(LOG_FILE_PATH, "a+");
	fprintf(logFd, "\n\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\ncloudbeacon program started.\n");
	fclose(logFd);

	if (! g_main)
	{
		g_main = new cloudBeaconMain;
	}
	
	cJSON_Hooks hooks;
	hooks.free_fn   = CJSONFree;
	hooks.malloc_fn = CJSONMalloc;
	cJSON_InitHooks(&hooks);

	// 初始化线程池
	pool_init(10);	
	if (g_main) g_main->run(argc, argv);
	pool_destroy();
	if (g_main) delete g_main, g_main = NULL;

	return 0;
}

