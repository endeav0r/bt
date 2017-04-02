OBJS=plugins.o

PLUGINS=tainttrace.so

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
INCLUDE=-I../

all : $(OBJS) $(PLUGINS)

%.o : %.c
	$(CC) -fPIC -c -o $@ $< $(INCLUDE) $(CFLAGS)

%.so : %.c
	$(CC) -fPIC -shared -o $@ $< $(INCLUDE) $(CFLAGS) $(LIB)

clean :
	rm -f *.o
	rm -rf *.so
	rm -rf *.dSYM
