OBJS=hsvm.o x86.o

CFLAGS=-Wall -O2 -g
INCLUDE=-I../../

all : $(OBJS)

%.o : %.c
	$(CC) -c $< -o $@ $(INCLUDE) $(CFLAGS)

clean :
	rm -f *.o
