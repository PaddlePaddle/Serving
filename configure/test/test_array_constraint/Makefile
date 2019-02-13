#COMAKE2 edit-mode: -*- Makefile -*-
####################64Bit Mode####################
ifeq ($(shell uname -m),x86_64)
CC=gcc
CXX=g++
CXXFLAGS=-g \
  -pipe \
  -W \
  -Wall \
  -fPIC
CFLAGS=-g \
  -pipe \
  -W \
  -Wall \
  -fPIC
CPPFLAGS=-D_GNU_SOURCE \
  -D__STDC_LIMIT_MACROS \
  -DVERSION=\"1.9.8.7\"
INCPATH=-I. \
  -I../../ \
  -I./include \
  -I./output \
  -I./output/include
DEP_INCPATH=-I../../../../com/btest/gtest \
  -I../../../../com/btest/gtest/include \
  -I../../../../com/btest/gtest/output \
  -I../../../../com/btest/gtest/output/include \
  -I../../../../lib2-64/bsl \
  -I../../../../lib2-64/bsl/include \
  -I../../../../lib2-64/bsl/output \
  -I../../../../lib2-64/bsl/output/include \
  -I../../../../lib2-64/ullib \
  -I../../../../lib2-64/ullib/include \
  -I../../../../lib2-64/ullib/output \
  -I../../../../lib2-64/ullib/output/include \
  -I../../../../public/spreg \
  -I../../../../public/spreg/include \
  -I../../../../public/spreg/output \
  -I../../../../public/spreg/output/include \
  -I../../../../quality/autotest/reportlib/cpp \
  -I../../../../quality/autotest/reportlib/cpp/include \
  -I../../../../quality/autotest/reportlib/cpp/output \
  -I../../../../quality/autotest/reportlib/cpp/output/include \
  -I../../../../third-64/pcre \
  -I../../../../third-64/pcre/include \
  -I../../../../third-64/pcre/output \
  -I../../../../third-64/pcre/output/include

#============ CCP vars ============
CCHECK=@ccheck.py
CCHECK_FLAGS=
PCLINT=@pclint
PCLINT_FLAGS=
CCP=@ccp.py
CCP_FLAGS=


#COMAKE UUID
COMAKE_MD5=96636deacbb58c864ba532f366b07a55  COMAKE


.PHONY:all
all:comake2_makefile_check test_arr 
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mall[0m']"
	@echo "make all done"

.PHONY:comake2_makefile_check
comake2_makefile_check:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mcomake2_makefile_check[0m']"
	#in case of error, update 'Makefile' by 'comake2'
	@echo "$(COMAKE_MD5)">comake2.md5
	@md5sum -c --status comake2.md5
	@rm -f comake2.md5

.PHONY:ccpclean
ccpclean:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mccpclean[0m']"
	@echo "make ccpclean done"

.PHONY:clean
clean:ccpclean
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mclean[0m']"
	rm -rf test_arr
	rm -rf ./output/bin/test_arr
	rm -rf test_arr_test_arr.o

.PHONY:dist
dist:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mdist[0m']"
	tar czvf output.tar.gz output
	@echo "make dist done"

.PHONY:distclean
distclean:clean
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mdistclean[0m']"
	rm -f output.tar.gz
	@echo "make distclean done"

.PHONY:love
love:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mlove[0m']"
	@echo "make love done"

test_arr:test_arr_test_arr.o \
  ../../libconfig.a
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mtest_arr[0m']"
	$(CXX) test_arr_test_arr.o -Xlinker "-(" ../../libconfig.a ../../../../com/btest/gtest/output/lib/libgtest.a \
  ../../../../com/btest/gtest/output/lib/libgtest_main.a \
  ../../../../lib2-64/bsl/lib/libbsl.a \
  ../../../../lib2-64/bsl/lib/libbsl_ResourcePool.a \
  ../../../../lib2-64/bsl/lib/libbsl_archive.a \
  ../../../../lib2-64/bsl/lib/libbsl_buffer.a \
  ../../../../lib2-64/bsl/lib/libbsl_check_cast.a \
  ../../../../lib2-64/bsl/lib/libbsl_exception.a \
  ../../../../lib2-64/bsl/lib/libbsl_pool.a \
  ../../../../lib2-64/bsl/lib/libbsl_utils.a \
  ../../../../lib2-64/bsl/lib/libbsl_var.a \
  ../../../../lib2-64/bsl/lib/libbsl_var_implement.a \
  ../../../../lib2-64/bsl/lib/libbsl_var_utils.a \
  ../../../../lib2-64/ullib/lib/libullib.a \
  ../../../../public/spreg/libspreg.a \
  ../../../../quality/autotest/reportlib/cpp/libautotest.a \
  ../../../../third-64/pcre/lib/libpcre.a \
  ../../../../third-64/pcre/lib/libpcrecpp.a \
  ../../../../third-64/pcre/lib/libpcreposix.a -lpthread \
  -lcrypto \
  -lrt -Xlinker "-)" -o test_arr
	mkdir -p ./output/bin
	cp -f --link test_arr ./output/bin

test_arr_test_arr.o:test_arr.cpp
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mtest_arr_test_arr.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o test_arr_test_arr.o test_arr.cpp

endif #ifeq ($(shell uname -m),x86_64)


