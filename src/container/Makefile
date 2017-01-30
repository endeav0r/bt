OBJS=buf.o byte_buf.o graph.o list.o memmap.o tags.o tree.o uint64.o varstore.o

CFLAGS=-Wall -O2 -g -Werror
INCLUDE=-I../

all : $(OBJS)

%.o : %.c
	$(CC) -c -o $@ $< $(INCLUDE) $(CFLAGS)

clean :
	rm -f *.o
