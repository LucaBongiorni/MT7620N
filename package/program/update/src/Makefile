
#CROSS_PATH=/home/openwrt/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/
#CROSS_COMPILE=$(CROSS_PATH)bin/mipsel-openwrt-linux-
#CXX=$(CROSS_COMPILE)g++

export STAGING_DIR

CFLAG = -Wall 
CFLAG += -D__VERDATA__=\"`date +%Y-%m-%d`\" -D__VERTIME__=__TIME__
CFLAG += -DBLUETOOTH
CFLAG += -DWEBSERVER
CFLAG += -DLOCALSOCKET

DEBUG = -D_DEBUG


OBJS=StrOperate.o confile.o updateMain.o md5.o netSockTcp.o cJSON.o updateOperate.o

INCS = -I $(CROSS_PATH)/include/
LIBS = -L $(CROSS_PATH)/lib
LIBS += -lpthread


PROGRAM=update

$(PROGRAM):$(OBJS)
	$(CXX) -o $@ $^ $(LIBS)

%.o:%.cpp
	$(CXX) -c -o $@ $< $(INCS) $(CFLAG) $(DEBUG)
%.o:%.c
	$(CXX) -c -o $@ $< $(INCS) $(CFLAG) $(DEBUG)

clean:
	rm -f $(OBJS) ${PROGRAM}

install:
	cp -fr $(PROGRAM) ../bin/

