WORKROOT=../../../
CXX	= $(WORKROOT)/libsrc/bsl/cxxtest

MARCS=
ifeq ($(MAC),64)
	LIBPATH=$(WORKROOT)lib2-64
else
	LIBPATH=$(WORKROOT)lib2
endif

PUBLIC=$(WORKROOT)/public
ULLIB=$(LIBPATH)/ullib/
NSHEAD=$(PUBLIC)/nshead/
DICT=$(LIBPATH)/dict/
BSL=../output/

XDATA=*.data phash.dat.1 *.dat data 1 failtest
INCLUDE= -I./ -I$(CXX) -I$(ULLIB)/include -I$(NSHEAD) -I$(DICT)/include/ -I../../
#XFLAG = -fsigned-char -Wall -W -pipe -Wno-unused-parameter -g -DBSL_DEBUG_MOD
#XXFLAG = -fsigned-char -Wall -W -pipe -Wno-unused-parameter -g -fprofile-arcs -ftest-coverage
XFLAG = -fsigned-char -Wall -W  -g -D_XOPEN_SOURE=500 -D_GNU_SOURCE -ftemplate-depth-128 -g -fPIC -fsigned-char -Wall -Winline -pipe 
		#-Wreturn-type  \
		#-Wtrigraphs -Wformat -Wparentheses -Wpointer-arith -finline-functions \
		#-Wshadow 
LIB=  -L$(ULLIB)/lib -L$(NSHEAD) -L$(DICT)/lib -L$(BSL)/lib -lbsl -luldict -lcrypto -lcrypt -lullib -lpthread -lm

CFLAG	=	$(XFLAG) -D_REENTRANT -D_FILTER -DNDEBUG $(INCLUDE) 
CPPFLAG	=	$(XFLAG) -D_REENTRANT -D_FILTER $(INCLUDE) -rdynamic

CXXTEST	= 	$(CXX)/cxxtestgen.pl
OUTPUT	= 	--error-printer --have-std --have-eh 

CC		= gcc
CPP		= g++
PERL	= perl
PY		= python

ALLTESTS=$(wildcard *.h)
CPPFILE=$(ALLTESTS:.h=.cpp)
EXE=$(basename $(ALLTESTS))

#$(CPPFILE) : $(ALLTESTS)
.h.cpp :
	$(CXXTEST) $(OUTPUT) -o $*.cpp $*.h 

	
all	   : $(EXE)
	
$(EXE) : $(CPPFILE) 
	$(CPP) $(CPPFLAG) -o $@   $@.cpp $(LIB) 

#=========================================================================
.PHONY	: clean  all test

clean	:
		rm -f  $(CPPFILE) $(EXE) $(XDATA)
test	: $(EXE) 
		@failed=0;
		@for exe in $(EXE) ; do echo; echo -n doing $$exe ">>>>>>>>";echo;echo;\
			./$$exe ; \
			echo done ; \
			echo ;\
		done;

%.h: %.py
	./$^ > $@

bsl_test_string: bsl_test_string.h bsl_test_string.hpp $(BSL)/include/bsl/containers/string/bsl_string.h

bsl_test_string_pool: bsl_test_string_pool.h $(BSL)/include/bsl/containers/string/bsl_string.h $(BSL)/include/bsl/pool/bsl_debugpool.h

bsl_test_shared_buffer: bsl_test_shared_buffer.h $(BSL)/include/bsl/exception/bsl_shared_buffer.h

bsl_test_auto_buffer: bsl_test_auto_buffer.h $(BSL)/include/bsl/AutoBuffer.h

bsl_test_exception: bsl_test_exception.h $(BSL)/include/bsl/exception/bsl_exception.h

bsl_test_bin_buffer: bsl_test_bin_buffer.h $(BSL)/include/bsl/BinBuffer.h


