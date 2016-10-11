OBJS=

CFLAGS=-Wall -O2 -g
INCLUDE=-I../

all : $(OBJS)
	make -C source
	make -C target

%.o : %.c
	$(CC) -c -o $@ $< $(INCLUDE) $(CFLAGS)

clean :
	make -C source clean
	make -C target clean