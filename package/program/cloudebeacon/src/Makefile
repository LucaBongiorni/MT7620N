###########################################################
# 功能开关，YES，打开，NO，关闭

# 打印信息
_DEBUG         := NO

# 开启串口蓝牙通讯功能
SERIALSCOM     := YES

#连接服务器的子端
WEBSERVER      := YES

#本地服务器
LOCALSOCKET    := YES

#上传mac地址
UPLOADMACINFO  := YES

#广播服务器
BROADCAST      := YES

#使用交叉工具
OPENWRT        := YES

#平台AR71xx/MT7620
#BOARD          := MT7620


#更新蓝牙固件
updateBlueBin  := YES

#兼顾老版本
OLD_VERSION    := YES
############################################################

CFLAG := -Wall
ifeq (NO, ${OPENWRT})
CFLAG += -fno-stack-protector -z execstack -mpreferred-stack-boundary=2
endif
CFLAG += -D__VERDATA__=\"`date +%Y-%m-%d`\" -D__VERTIME__=__TIME__

ifeq (YES, ${SERIALSCOM})
	CFLAG += -DSERIALSCOM
endif

ifeq (YES, $(WEBSERVER))
	CFLAG += -DWEBSERVER
endif

ifeq (YES, $(LOCALSOCKET))
	CFLAG += -DLOCALSOCKET
endif

ifeq (YES, $(UPLOADMACINFO))
	CFLAG += -DUPLOADMACINFO
endif

ifeq (YES, $(OLD_VERSION))
	CFLAG += -DOLD_VERSION
endif 

ifeq (YES, $(BROADCAST))
	CFLAG += -DBROADCAST
endif


ifeq (YES, ${updateBlueBin})
	CFLAG += -DUPDATEBLUEBIN
endif


export STAGING_DIR



ifeq (YES, ${OPENWRT})

ifeq (MT7620, $(BOARD))
	CROSS_PATH=/home/openwrt/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/
	CROSS_COMPILE=$(CROSS_PATH)/bin/mipsel-openwrt-linux-
	INCS = -I$(CROSS_PATH)/../target-mipsel_24kec+dsp_uClibc-0.9.33.2/usr/include/
	LIBS = -L$(CROSS_PATH)/../target-mipsel_24kec+dsp_uClibc-0.9.33.2/usr/lib/
	#INCS += -I$(CROSS_PATH)/../target-mipsel_r2_uClibc-0.9.33.2/usr/include/
	#LIBS += -L$(CROSS_PATH)/../target-mipsel_r2_uClibc-0.9.33.2/usr/lib/
	INCS += -I $(CROSS_PATH)/include/
	LIBS += -L $(CROSS_PATH)/lib/
	CXX=$(CROSS_COMPILE)g++
else ifeq (AR71xx, $(BOARD))	
	CROSS_PATH=/root/ar71xx-toolchain/OpenWrt-Toolchain-ar71xx-for-mips_r2-gcc-4.6-linaro_uClibc-0.9.33.2/toolchain-mips_r2_gcc-4.6-linaro_uClibc-0.9.33.2/
	CROSS_COMPILE=$(CROSS_PATH)/bin/mips-openwrt-linux-
	INCS = -I$(CROSS_PATH)/../target-mipsel_24kec+dsp_uClibc-0.9.33.2/usr/include/
	LIBS = -L$(CROSS_PATH)/../target-mipsel_24kec+dsp_uClibc-0.9.33.2/usr/lib/
	INCS += -I$(CROSS_PATH)/../target-mipsel_r2_uClibc-0.9.33.2/usr/include/
	LIBS += -L$(CROSS_PATH)/../target-mipsel_r2_uClibc-0.9.33.2/usr/lib/
	INCS += -I $(CROSS_PATH)/include/
	LIBS += -L $(CROSS_PATH)/lib/
	CXX=$(CROSS_COMPILE)g++
endif

	CFLAG += -DUSE_OPENWRT
	LDLIBS += -ldl -lm -lpthread -lssl -lcrypto -lcrypt -lubox -luci
else
	CXX=g++
	INCS := -I/include/ -I/usr/include/ -I/usr/local/include/
	LIBS := -L/lib/ -L/usr/lib/ -L/usr/local/lib
	LDLIBS := -lm -lpthread -lssl -lcrypto -lcrypt
endif


OBJS =  StrOperate.o confile.o cbProjMain.o listStruct.o procotol.o netSockTcp.o 
OBJS += base64RSA.o socketClient.o MemPool.o pthreadCom.o beaconConf.o rbtree.o
OBJS += cJSON.o thread_pool.o dealWithOpenwrt.o defCom.o UCI_File.o netPing.o


ifeq (YES, ${SERIALSCOM})
OBJS += SerialCom.o serialPct.o
OBJS += ble_advert.o ble_central.o ble_central_test.o ble_common.o ble_device.o
OBJS += tinyxml.o tinyxmlerror.o tinyxmlparser.o
endif

ifeq (YES, ${LOCALSOCKET})
OBJS += pthreadServer.o
endif

ifeq (YES, ${BROADCAST})
OBJS += broadcast.o
endif

ifeq (YES, ${updateBlueBin})
ifeq (YES, ${SERIALSCOM})
OBJS += upBToothBin.o md5.o
endif
endif

ifeq (YES, ${UPLOADMACINFO})
OBJS += gatherMac.o
endif



PROGRAM = cloudbeacon
$(PROGRAM):$(OBJS)
	$(CXX) -o $@ $^ $(LIBS) $(LDLIBS)

%.o:%.cpp
	$(CXX) -c -o $@ $< $(INCS) $(CFLAG) $(DEBUG)
%.o:%.c
	$(CXX) -c -o $@ $< $(INCS) $(CFLAG) $(DEBUG)

clean:
	rm -f $(OBJS) ${PROGRAM}

install:
	mkdir -p /usr/ibeacon/conf/
	mkdir -p /usr/ibeacon/bin/
	mkdir -p /usr/ibeacon/tool/
	cp -rf $(PROGRAM) /usr/ibeacon/bin/
	cp -rf *.conf /usr/ibeacon/conf
	cp -rf ScanFormats.xml /usr/ibeacon/conf

