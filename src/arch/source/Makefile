OBJS=hsvm.o mips.o

CFLAGS=-Wall -O2 -g
INCLUDE=-I../../ -I/usr/local/include

all : $(OBJS)

%.o : %.c
	$(CC) -fPIC -c $< -o $@ $(INCLUDE) $(CFLAGS)

clean :
	rm -f *.o
