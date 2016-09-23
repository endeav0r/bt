OBJS=buf.o list.o mmap.o tree.o

CFLAGS=-Wall -O2 -g

all : $(OBJS)

%.o : %.c
	$(CC) -c -o $@ $< $(INCLUDE) $(CFLAGS)

clean :
	rm -f *.o