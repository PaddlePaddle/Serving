######################################################
# MAKEFILE TEMPLATE                                  #
# AUTHOR:		chenxm (chenxiaoming@baidu.com)      #
# LAST UPDATE:	2008年 08月 18日 星期一 14:49:27 CST #
######################################################

####################################### CONFIGURATION ####################################### 

#####  basic configuration  #####
TARGET		= bsl_buffer
BSL_ROOT	= ..
include Makefile.env

CC			= g++
OUTPUT_HEAD_PATH= $(BSL_ROOT)/output/include/bsl
OUTPUT_LIB_PATH	= $(BSL_ROOT)/output/lib

#####  build & test configuration  #####

LOCAL_HEADS	= $(wildcard *.h) $(wildcard *.hpp)

HEADS		= $(LOCAL_HEADS) 

OBJECTS		= AutoBuffer.o BinBuffer.o

INCLUDES	= 
LIBS	    = -L../output/lib/ -lbsl

#####  other  #####

TAG_ROOT	= $(BSL_ROOT)

#######################################     RULES     ####################################### 

#comment the following line for debugging
.SILENT: all test output clean tag doc debug depend

.PHONY: all test output clean tag doc debug depend

#comment the following line to use default known suffixes (DANGEROUS!!!)
.SUFFIXES:

all: output clean
	cp *.h ../

%: %.o $(OBJECTS)
	@echo "[make] building $@ ..."
	$(CC) -o $@ $^ $(BSL_LDFLAGS) $(LIBS) 

%.o: %.cpp $(HEADS)
	@echo "[make] building $@ ..."
	$(CC) -g -o $@ -c $< $(DEBUG) $(BSL_CXXFLAGS) $(INCLUDES) 

depend:
	@for DEPENDENCY in $(DEPENDENCYS); do echo "[make]     making lib $$DEPENDENCY ..."; make -C $$DEPENDENCY; done

test: $(TEST_TARGETS)
	@echo "[make] testing ..."
	@for TEST_TARGET in $(TEST_TARGETS); do echo "[make]     testing $$TEST_TARGET ..."; ./$$TEST_TARGET; done

test_$(TARGET): test_$(TARGET).o $(OBJECTS)
	@echo "[make] building $@ ..."
	$(CC) -o $@ $^ $(BSL_LDFLAGS) $(LIBS) 

output: lib$(TARGET).a $(LOCAL_HEADS)
	@echo "[make] copying $< to $(OUTPUT_LIB_PATH) ..."
	mkdir -p $(OUTPUT_LIB_PATH)
	cp $< $(OUTPUT_LIB_PATH) 
	@echo "[make] copying $(LOCAL_HEADS) to $(OUTPUT_HEAD_PATH) ..."
	mkdir -p $(OUTPUT_HEAD_PATH)
	cp $(LOCAL_HEADS) $(OUTPUT_HEAD_PATH) 

lib$(TARGET).a: $(OBJECTS)
	@echo "[make] building $@ ..."
	ar cr $@ $^

clean:
	@echo "[make] cleaning ..."
	rm *.o *.a $(TEST_TARGETS) -f
	
doc:
	@echo "[make] generating documents ..."
	doxygen 

tag:
	@echo "[make] generating tags ..."
	ctags --c++-kinds=+p --fields=+iaS --extra=+q -R $(TAG_ROOT);
	
debug:
	@echo "[make] printing variables ..."
	@echo 'basic configuration'
	@echo '$$(TARGET)       = $(TARGET)'
	@echo ''
	@echo 'path configuration'
	@echo '$$(WORK_ROOT)     = $(WORK_ROOT)'
	@echo '$$(LIB2_PATH)     = $(LIB2_PATH)'
	@echo '$$(BSL_ROOT)     = $(BSL_ROOT)'
	@echo '$$(OUTPUT_HEAD_PATH) = $(OUTPUT_HEAD_PATH)'
	@echo '$$(OUTPUT_LIB_PATH)  = $(OUTPUT_LIB_PATH)'
	@echo ''
	@echo 'build & test configuration'
	@echo '$$(INCLUDES)     = $(INCLUDES)'
	@echo '$$(LOCAL_HEADS)  = $(LOCAL_HEADS)'
	@echo '$$(OBJECTS)      = $(OBJECTS)'
	@echo '$$(LIBS)         = $(LIBS)'
	@echo '$$(TEST_TARGETS) = $(TEST_TARGETS)'
	@echo ''
	@echo 'other'
	@echo '$$(TAG_ROOT)		= $(TAG_ROOT)'
