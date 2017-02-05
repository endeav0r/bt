OBJS=testobj.o

CFLAGS=-Wall -O2 -g
INCLUDE=-I../
LIB=../arch/target/*.o ../bt/*.o ../container/*.o ../*.o *.o

all : $(OBJS)
	$(CC) -o test_amd64 test_amd64.c $(INCLUDE) $(LIB) $(CFLAGS)
	$(CC) -o test_buf test_buf.c $(INCLUDE) $(LIB) $(CFLAGS)
	$(CC) -o test_byte_buf test_byte_buf.c $(INCLUDE) $(LIB) $(CFLAGS)
	$(CC) -o test_list test_list.c $(INCLUDE) $(LIB) $(CFLAGS)
	$(CC) -o test_object test_object.c $(INCLUDE) $(LIB) $(CFLAGS)
	$(CC) -o test_tree test_tree.c $(INCLUDE) $(LIB) $(CFLAGS)
	$(CC) -o test_varstore test_varstore.c $(INCLUDE) $(LIB) $(CFLAGS)
	./test_amd64
	./test_buf
	./test_byte_buf
	./test_list
	./test_object
	./test_tree
	./test_varstore

%.o : %.c
	$(CC) -c -o $@ $< $(INCLUDE) $(CFLAGS)

clean :
	rm -f *.o
	rm -r test_amd64
	rm -f test_buf
	rm -f test_byte_buf
	rm -f test_list
	rm -f test_object
	rm -f test_tree
	rm -f test_varstore
	rm -rf *.dSYM
