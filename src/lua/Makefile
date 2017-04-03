INCLUDE=-I../
CFLAGS=-Wall -O2 -g
LIB=../*.o \
    ../arch/*.o \
    ../arch/source/*.o \
	../arch/target/*.o \
	../bt/*.o \
	../container/*.o \
	../platform/*.o \
	../plugins/*.o \
	-lcapstone

OSX_FLAGS=-bundle -undefined dynamic_lookup
LINUX_FLAGS=-shared

all : binarytoolkit.c
	$(CC) $(OSX_FLAGS) -o binarytoolkit.so binarytoolkit.c $(INCLUDE) $(CFLAGS) $(LIB)

clean :
	rm -f binarytoolkit.so
	rm -rf *.o
	rm -rf *.dSYM
