OBJS=hsvm.o

CFLAGS=-Wall -O2 -g
INCLUDE=-I../../

all : $(OBJS)

%.o : %.c
	$(CC) -fPIC -c $< -o $@ $(INCLUDE) $(CFLAGS)

clean :
	rm -f *.o
