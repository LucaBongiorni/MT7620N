
CFLAG = -Wall 
CFLAG += -D__VERDATA__=\"`date +%Y-%m-%d`\" -D__VERTIME__=__TIME__


DEBUG = -D_DEBUG


OBJS := ctrlFirewall.o
OBJS += tool.o

LDLIBS:=


PROGRAM=ctrlFirewall

$(PROGRAM):$(OBJS)
	$(CC) -o $@ $^ $(LDLIBS)

%.o:%.cpp
	$(CC) -c -o $@ $< $(INCS) $(CFLAG) $(DEBUG)
%.o:%.c
	$(CC) -c -o $@ $< $(INCS) $(CFLAG) $(DEBUG)

clean:
	rm -f $(OBJS) ${PROGRAM}

install:
	cp -fr $(PROGRAM) ../bin/

