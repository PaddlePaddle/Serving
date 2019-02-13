#* -*- makefile -*- 
#   copy[write] by dirlt(zhang_yan@baidu.com)
#   date time:Mon Nov 17 22:52:18 CST 2008
#   file name:Makefile 
CC=g++
OBJS=main.o 
CPPFLAGS=-fsigned-char -Wall -W -pipe -Wno-unused-parameter -g
LDFLAGS=-lpthread -lm -lidlcompiler
INCLUDES=-I../output/include
LDPATH=-L../output/lib
LIB= #create the lib 
TARGET=main #create the exe file 

#32-biit and 64-bit platforms
ifeq ($(MAC),64)
else
endif

all:$(TARGET)
$(TARGET):clean $(OBJS)
	$(CC) $(CPPFLAGS) -o $@ $(OBJS) $(LDPATH) $(LDFLAGS)
$(LIB):$(OBJS)
	ar cr $(LIB) $(OBJS)
%.o:%.cpp
	$(CC) $(INCLUDES) $(CPPFLAGS)  -o $@ -c $<
clean:
	rm -rf $(OBJS) $(TARGET) $(LIBS)
	rm -rf *~
test:
	valgrind ./main `ls ../testcase/*.def`
