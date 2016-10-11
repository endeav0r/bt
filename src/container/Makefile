OBJS=buf.o byte_buf.o list.o memmap.o tree.o varstore.o

CFLAGS=-Wall -O2 -g
INCLUDE=-I../

all : $(OBJS)

%.o : %.c
	$(CC) -c -o $@ $< $(INCLUDE) $(CFLAGS)

clean :
	rm -f *.o