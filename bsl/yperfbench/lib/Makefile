CC=g++
WORKROOT=../../

INCLUDES=-I../include 
CFLAGS = -fsigned-char -Wall -W -pipe -Wno-unused-parameter -g 
LDFLAGS= -lpthread -lm
OBJS=yperfbench.o xutils.o
TARGET=
LIB=libyperfbench.a
all : $(TARGET) $(LIB)

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


