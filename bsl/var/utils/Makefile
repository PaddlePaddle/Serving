######################################################
# MAKEFILE TEMPLATE                                  #
# AUTHOR:		chenxm (chenxiaoming@baidu.com)      #
# LAST UPDATE:	Fri Jan  9 12:33:18 CST 2009		 #
######################################################

####################################### CONFIGURATION ####################################### 
 
#####  basic configuration  #####
VAR_ROOT		= ..
include $(VAR_ROOT)/Makefile.env

#####  build & test configuration  #####

PROVIDE_HEADS	= $(wildcard *.h) $(patsubst %.py, %.h, $(wildcard *.py))

PROVIDE_OBJECTS	= utils.o

PROVIDE_LIB		= bsl_var_utils

DEPEND_HEADS	= $(PROVIDE_HEADS)  $(wildcard $(OUTPUT_HEAD_PATH)/*.h )

#####  other  #####

TAG_ROOT		= $(VAR_ROOT)

#######################################     RULES     ####################################### 
include $(VAR_ROOT)/Makefile.rules

####################################### SPECIAL RULES ####################################### 
assign.h: assign.py
	python $^ > $@


