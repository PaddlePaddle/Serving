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

PROVIDE_ENTRANCES= check_cast.h

PROVIDE_HEADS	= $(filter-out check_cast_generated.h, $(wildcard *.h) ) check_cast_generated.h 

PROVIDE_OBJECTS	= check_cast_cstring.o

PROVIDE_LIB		= bsl_check_cast

DEPEND_HEADS	= $(PROVIDE_HEADS)  

DEPEND_OBJECTS	= 

DEPEND_LIBS		= 

#####  other  #####

TAG_ROOT		= $(VAR_ROOT)

#####  OVERWRITE  #####

OUTPUT_HEAD_PATH= $(BSL_ROOT)/output/include/bsl/check_cast

#######################################     RULES     ####################################### 
include $(VAR_ROOT)/Makefile.rules

####################################### SPECIAL RULES ####################################### 
check_cast_generated.h: check_cast.py
	./$^ > $@


