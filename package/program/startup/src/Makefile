
CFLAG = -Wall 

OBJS=hello.o

PROGRAM = hello
$(PROGRAM):$(OBJS)
	$(CXX) -o $@ $^ $(LDLIBS)

%.o:%.c
	$(CC) -c -o $@ $< $(INCS) $(CFLAG) $(DEBUG)

