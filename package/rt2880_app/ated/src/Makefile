


CFLAG = -Wall 
CFLAG += -D__VERDATA__=\"`date +%Y-%m-%d`\" -D__VERTIME__=__TIME__



OBJS=ated.o

PROGRAM=ated

$(PROGRAM):$(OBJS)
	$(CC) -o $@ $^ $(LDLIBS)


%.o:%.c
	$(CC) -c -o $@ $< $(INCS) $(CFLAG) $(DEBUG)

clean:
	rm -f $(OBJS) ${PROGRAM}

install:
	cp -fr $(PROGRAM) ../bin/

