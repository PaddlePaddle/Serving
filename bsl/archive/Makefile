CC=/usr/bin/g++

ifeq ($(MAC),64)
	LIBPATH=$(WORKROOT)lib2-64
else
	LIBPATH=$(WORKROOT)lib2
endif

BSL=../
OUTINC=$(BSL)/output/include/bsl/archive
OUTLIB=$(BSL)/output/lib



INCLUDES=-I$(BSL)/../
CFLAGS = -fsigned-char -Wall -W -pipe -Wno-unused-parameter -g -fPIC
LDFLAGS=  -lpthread -lm
OBJS=bsl_filestream.o 
TARGET=
LIB=libbsl_archive.a
all : $(TARGET) $(LIB) output

%.o : %.cpp
	$(CC) $(CFLAGS) -c $< -o $@ $(INCLUDES)

$(LIB) : $(OBJS)
	rm -f $@
	ar cr $@ $(OBJS)

output : $(LIB)
	rm -rf $(OUTINC)
	mkdir -p $(OUTINC)
	mkdir -p $(OUTLIB)
	cp *.h $(OUTINC)
	cp $(LIB) $(OUTLIB)


tags : 
	ctags -R *

clean:
	rm -f $(OBJS) $(TARGET) $(LIB) test.o
	rm -f -r $(OUTINC);
	rm -f $(OUTLIB)/$(LIB)


