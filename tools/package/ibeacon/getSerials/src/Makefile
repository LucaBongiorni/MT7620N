
CFLAG = -Wall 
CFLAG += -D__VERDATA__=\"`date +%Y-%m-%d`\" -D__VERTIME__=__TIME__
CFLAG += -DBLUETOOTH
CFLAG += -DWEBSERVER
CFLAG += -DLOCALSOCKET

DEBUG = -D_DEBUG


OBJS=beacon.o StrOperate.o confile.o common.o serial_port.o

LDLIBS += -lpthread


PROGRAM = getSerials

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

