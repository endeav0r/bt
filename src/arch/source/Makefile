OBJS=hsvm.o

CFLAGS=-Wall -O2 -g

all : $(OBJS)
	make -C source
	make -C target

%.o : %.c
	$(CC) -c -o $@ $< $(INCLUDE) $(CFLAGS)

clean :
	make -f *.o