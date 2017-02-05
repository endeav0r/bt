OBJS=hsvm.o

CFLAGS=-Wall -O2 -g
INCLUDE=-I../

all : $(OBJS)
	make -C source
	make -C target

%.o : %.c %.h
	$(CC) -fPIC -c -o $@ $< $(INCLUDE) $(CFLAGS)

%.o : %.c
	$(CC) -fPIC -c -o $@ $< $(INCLUDE) $(CFLAGS)

clean :
	make -C source clean
	make -C target clean
	rm -f *.o
