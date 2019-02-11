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

DEPEND_HEADS	= $(PROVIDE_HEADS) $(wildcard ../../output/include/bsl/*.h) $(wildcard ../../output/include/bsl/var/*.h)

PROVIDE_OBJECTS	=

DEPEND_OBJECTS	=

PROVIDE_LIBS	=

DEPEND_LIBS		=

PROVIDE_BINS	= test_bsl_wrappers test_check_cast_gen test_check_cast test_ResourcePool test_ShallowCopystring test_var_utils test_var_Bool test_var_Null test_var_Ref test_var_Int32 test_var_Int64 test_var_Double test_var_ShallowString test_var_String test_var_String2 test_var_ShallowRaw test_var_Array test_var_Array2 test_var_Dict test_var_Function test_var_Method test_var_MagicRef test_var_Number

#####  OVERWRITE  #####

OUTPUT_HEAD_PATH 	=
OUTPUT_OBJECT_PATH 	=
OUTPUT_LIB_PATH		=
OUTPUT_BIN_PATH	    = 

#####  other  #####

TAG_ROOT		= $(VAR_ROOT)

TEST_TARGETS	= $(PROVIDE_BINS)

#######################################     RULES     ####################################### 
include $(VAR_ROOT)/Makefile.rules

####################################### SPECIAL RULES ####################################### 

test_check_cast_gen.cpp: test_check_cast_gen.py
	./$^ > $@

test: output
	@for TEST_TARGET in $(TEST_TARGETS); do\
		echo "[make] testing $$TEST_TARGET ...";\
		./$$TEST_TARGET;\
	done

valgrind_test: output
	@for TEST_TARGET in $(TEST_TARGETS); do\
		echo "[make] testing $$TEST_TARGET ...";\
		valgrind --leak-check=full --show-reachable=yes ./$$TEST_TARGET;\
	done



