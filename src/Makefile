OBJS=btlog.o hooks.o object.o

CFLAGS=-Wall -O2 -g
LIB=arch/*.o \
    arch/source/*.o \
	arch/target/*.o \
	bt/*.o \
	container/*.o \
	platform/*.o \
	plugins/*.o \
	-ldl
INCLUDE=-I./

all : $(OBJS)
	make -C arch
	make -C bt
	make -C container
	make -C loader
	make -C platform
	make -C plugins
	make -C test
	$(CC) -o jit_example jit_example.c $(INCLUDE) $(OBJS) $(LIB) $(CFLAGS)
	$(CC) -o jit_hsvm jit_hsvm.c $(INCLUDE) $(OBJS) $(LIB) $(CFLAGS)

%.o : %.c
	$(CC) -fPIC -c -o $@ $< $(INCLUDE) $(CFLAGS)

clean :
	make -C arch clean
	make -C bt clean
	make -C container clean
	make -C loader clean
	make -C platform clean
	make -C plugins clean
	make -C test clean
	rm -f jit_example
	rm -f jit_hsvm
	rm -f *.o
	rm -rf *dSYM
