OBJS=btins.o jit.o

CFLAGS=-Wall -O2 -g

all : $(OBJS)

%.o : %.c
	$(CC) -c -o $@ $< $(INCLUDE) $(CFLAGS)

clean :
	rm -f *.o