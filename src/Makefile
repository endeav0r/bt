OBJS=object.o

CFLAGS=-Wall -O2 -g

all : $(OBJS)
	make -C arch
	make -C bt
	make -C container
	make -C loader
	make -C test

%.o : %.c
	$(CC) -c -o $@ $< $(INCLUDE) $(CFLAGS)

clean :
	make -C arch clean
	make -C bt clean
	make -C container clean
	make -C loader clean
	make -C test clean