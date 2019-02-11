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

PROVIDE_HEADS	= $(wildcard *.h) 

PROVIDE_OBJECTS	= 

DEPEND_HEADS	= $(PROVIDE_HEADS)  $(wildcard $(OUTPUT_HEAD_PATH)/*.h )

DEPEND_OBJECTS	= 

DEPEND_LIBS		= 

#####  other  #####

TAG_ROOT		= $(VAR_ROOT)

#######################################     RULES     ####################################### 
include $(VAR_ROOT)/Makefile.rules

####################################### SPECIAL RULES ####################################### 
check_cast_generated.h: check_cast.py
	./$^ > $@


