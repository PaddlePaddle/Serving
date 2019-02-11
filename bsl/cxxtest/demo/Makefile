CXX	= ../

INCLUDE	= 	-I ./\
		-I$(CXX) \

LIB	=	

CFLAG	=	-fsigned-char -Wall -g -D_REENTRANT -D_FILTER -DNDEBUG -fPIC $(INCLUDE)
CPPFLAG	=	-fsigned-char -Wall -g -D_REENTRANT -D_FILTER -fPIC $(INCLUDE)

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
		rm -f  $(CPPFILE) $(EXE) 
test	: $(EXE) 
		@failed=0;
		@for exe in $(EXE) ; do echo; echo -n doing $$exe ">>>>>>>>";echo;echo;\
			./$$exe ; \
			echo done ; \
			echo ;\
		done;

