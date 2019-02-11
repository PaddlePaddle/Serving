CC=g++

INCOUT=../output/include/bsl/pool/
LIBOUT=../output/lib/

INCLUDES= 
CFLAGS = -fsigned-char -Wall -W -pipe -Wno-unused-parameter -g -fPIC
LDFLAGS= 
OBJS=bsl_cachedpool.o bsl_xmempool.o bsl_xcompool.o bsl_cachedpoolappend.o
TARGET=
LIB= libbsl_pool.a
all : $(TARGET) $(LIB)
	mkdir -p $(INCOUT)
	mkdir -p $(LIBOUT)
	cp *.h $(INCOUT)
	cp $(LIB) $(LIBOUT)

%.o : %.cpp
	$(CC) $(CFLAGS) -c $< -o $@ $(INCLUDES)

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(INCLUDES)

$(LIB) : $(OBJS)
	rm -f $@
	ar cr $@ $(OBJS)

tags : 
	ctags -R *

clean:
	rm -f $(OBJS) $(TARGET) $(LIB)
	rm -rf $(INCOUT)
	rm -rf $(LIBOUT)/$(LIB)

bsl_cachedpool.o: bsl_cachedpool.cpp bsl_cachedpool.h bsl_pool.h
bsl_xcompool.o: bsl_xcompool.cpp bsl_xcompool.h bsl_xmempool.h bsl_pool.h
bsl_xmempool.o: bsl_xmempool.cpp bsl_xmempool.h bsl_pool.h
bsl_cachedpoolappend.o : bsl_cachedpoolappend.cpp bsl_cachedpoolappend.h bsl_pool.h
