LIB			= bsl_var
SUBLIB_DIRS	= interface utils implement 
SUBLIBS		= bsl_var_utils bsl_var_implement 


VAR_ROOT= .
include Makefile.env

all: lib clean

lib: sublibs 
	@echo "[make] building lib $(LIB) ...";
	@for SUBLIB in $(SUBLIBS); do\
		ar xo $(OUTPUT_LIB_PATH)/lib$$SUBLIB.a;	\
	done
	@mkdir -p $(OUTPUT_LIB_PATH)
	@ar cr $(OUTPUT_LIB_PATH)/lib$(LIB).a *.o

sublibs:
	@for SUBLIB in $(SUBLIB_DIRS); do\
		echo "[make] building lib $$SUBLIB ...";\
		make -C $$SUBLIB;\
	done
		
clean:
	@echo "[make] cleaning ..."
	@rm -f *.o
	@for SUBLIB in $(SUBLIB_DIRS); do\
		echo "[make] cleaning lib $$SUBLIB ...";\
		make clean -C $$SUBLIB;\
	done
