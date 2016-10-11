OBJS=testobj.o

CFLAGS=-Wall -O2 -g
INCLUDE=-I../
LIB=../container/*.o ../object.o *.o

all : $(OBJS)
	$(CC) -o test_object test_object.c $(INCLUDE) $(LIB) $(CFLAGS)
	$(CC) -o test_buf test_buf.c $(INCLUDE) $(LIB) $(CFLAGS)
	$(CC) -o test_byte_buf test_byte_buf.c $(INCLUDE) $(LIB) $(CFLAGS)
	$(CC) -o test_list test_list.c $(INCLUDE) $(LIB) $(CFLAGS)

%.o : %.c
	$(CC) -c -o $@ $< $(INCLUDE) $(CFLAGS)

clean :
	rm -f *.o
	rm -f test_object
	rm -f test_buf
	rm -f test_byte_buf
	rm -f test_list
	rm -rf *.dSYM