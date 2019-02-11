######################################################
# MAKEFILE TEMPLATE                                  #
# AUTHOR:		chenxm (chenxiaoming@baidu.com)      #
# LAST UPDATE:	Fri Jan  9 12:33:18 CST 2009		 #
######################################################

####################################### CONFIGURATION ####################################### 
 
#####  basic configuration  #####
VAR_ROOT		= ../var
include $(VAR_ROOT)/Makefile.env

#####  build & test configuration  #####

PROVIDE_ENTRANCES = ResourcePool.h ShallowCopyString.h

PROVIDE_HEADS	= $(wildcard *.h) 

PROVIDE_OBJECTS	= ResourcePool.o ShallowCopyString.o

PROVIDE_LIB		= bsl_ResourcePool

DEPEND_HEADS	= $(PROVIDE_HEADS)  

DEPEND_OBJECTS	= 

DEPEND_LIBS		= 

#####  other  #####

TAG_ROOT		= $(VAR_ROOT)

#####  OVERWRITE  #####

OUTPUT_HEAD_PATH= $(BSL_ROOT)/output/include/bsl/ResourcePool

#######################################     RULES     ####################################### 
include $(VAR_ROOT)/Makefile.rules

####################################### SPECIAL RULES ####################################### 
check_cast_generated.h: check_cast.py
	./$^ > $@


