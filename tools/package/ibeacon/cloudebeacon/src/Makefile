
CFLAG = -Wall 
CFLAG += -D__VERDATA__=\"`date +%Y-%m-%d`\" -D__VERTIME__=__TIME__

CFLAG += -DBLUETOOTH


CFLAG += -DWEBSERVER

CFLAG += -DLOCALSOCKET

CFLAG += -DUPLOADMACINFO

#¹ã²¥·þÎñÆ÷
CFLAG += -DBROADCAST


DEBUG = -D_DEBUG
export STAGING_DIR

#LIBS += -L $(CROSS_PATH)/../target-mipsel_24kec+dsp_uClibc-0.9.33.2/usr/lib/
LDLIBS += -lm -lpthread -lssl -lcrypto -lubox -luci



OBJS =  StrOperate.o confile.o cbProjMain.o listStruct.o procotol.o netSockTcp.o cJSON.o broadcast.o 
OBJS += base64RSA.o pthreadServer.o socketClient.o MemPool.o pthreadCom.o UCI_File.o
PROGRAM = cloudebeacon


$(PROGRAM):$(OBJS)
	$(CXX) -o $@ $^ $(LDLIBS)

%.o:%.cpp
	$(CXX) -c -o $@ $< $(INCS) $(CFLAG) $(DEBUG)
%.o:%.c
	$(CXX) -c -o $@ $< $(INCS) $(CFLAG) $(DEBUG)

clean:
	rm -f $(OBJS) ${PROGRAM}

install:
	cp -fr $(PROGRAM) ../bin/

