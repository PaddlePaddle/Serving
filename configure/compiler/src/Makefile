#* -*- makefile -*- 
#   copy[write] by dirlt(zhang_yan@baidu.com)
#   date time:Sun Nov 16 13:55:54 CST 2008
#   file name:Makefile 
CC=g++

WORKROOT=../../../..
ifeq ($(MAC),64)
	LIBPATH=$(WORKROOT)/lib2-64
else
	LIBPATH=$(WORKROOT)/lib2
endif
ifeq ($(MAC),ARM32)
	LIBPATH=$(WORKROOT)/lib2-arm32
endif	

BSL=$(LIBPATH)/bsl

OBJS=idl_gram.o idl_lex.o idl.o 
CPPFLAGS=-fPIC -fsigned-char -Wall -W -pipe -Wno-unused-parameter -g -UDEBUG 
LDFLAGS=-lpthread -lm
INCLUDES=-I../ -I$(BSL)/include
LDPATH=-L../ -L$(BSL)/lib -lbsl_exception
LIB=libidlcompiler.a #create the lib 
TARGET= #create the exe file 


#32-biit and 64-bit platforms
ifeq ($(MAC),64)
	FLEX=./flex.64
	BISON=./bison.64
	CLRSRC=idl_lex.c idl_gram.c idl_lex.h idl_gram.h
else
	FLEX=./flex.32
	BISON=./bison.32
	CLRSRC=
endif
ifeq ($MAC),ARM32)
	FLEX=./flex.arm32
	BISON=./bison.arm32
	CLRSRC=idl_lex.c idl_gram.c idl_lex.h idl_gram.h
endif
all:$(LIB)
$(TARGET):$(OBJS)
	$(CC) $(CPPFLAGS) -o $@ $(OBJS) $(LDPATH) $(LDFLAGS)
$(LIB):$(OBJS)
	mkdir -p ../output/lib
	mkdir -p ../output/include
	ar cr $(LIB) $(OBJS)
	rm -f ../output/include/idl_conf_if.h
	rm -f ../output/lib/libidlcompiler.a
	cp idl_conf_if.h ../output/include
	cp libidlcompiler.a ../output/lib
%.o:%.cpp %.h
	$(CC) $(CPPFLAGS) $(INCLUDES)  -o $@ -c $<
%.o:%.c %.h
	$(CC) $(CPPFLAGS) $(INCLUDES)  -o $@ -c $<
clean:
	rm -rf $(OBJS) $(TARGET) $(LIB) $(CLRSRC)
	rm -rf *~
	rm -rf ../output
ifeq ($(MAC),64)
idl_gram.c idl_gram.h:idl.gram idl_lex.h
	$(BISON) idl.gram
idl_lex.c idl_lex.h:idl.lex
	$(FLEX) idl.lex
endif
