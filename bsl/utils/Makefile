CC=g++
BSL=../
INCLUDES=-ftemplate-depth-128
CFLAGS =  -fsigned-char -fPIC -O2
LDFLAGS= 
OBJS=bsl_memcpy.o
TARGET=
LIB=libbsl_utils.a

OUTINC=$(BSL)/output/include/bsl/utils
OUTLIB=$(BSL)/output/lib

all : $(TARGET) $(LIB) output

%.o : %.cpp
	$(CC) $(CFLAGS) -c $< -o $@ $(INCLUDES)

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(INCLUDES)

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
	rm -f $(OBJS) $(TARGET) $(LIB)
	rm -f -r $(OUTINC);
	rm -f $(OUTLIB)/$(LIB)


