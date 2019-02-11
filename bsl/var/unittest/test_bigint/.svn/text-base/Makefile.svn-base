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
  -DVERSION=\"1.1.24.1\"
INCPATH=-I. \
  -I../../implement \
  -I../../../output/include/bsl/var/ \
  -I../../../output/include/
DEP_INCPATH=-I../../../../../com/btest/gtest \
  -I../../../../../com/btest/gtest/include \
  -I../../../../../com/btest/gtest/output \
  -I../../../../../com/btest/gtest/output/include \
  -I../../../../../quality/autotest/reportlib/cpp \
  -I../../../../../quality/autotest/reportlib/cpp/include \
  -I../../../../../quality/autotest/reportlib/cpp/output \
  -I../../../../../quality/autotest/reportlib/cpp/output/include

#============ CCP vars ============
CCHECK=@ccheck.py
CCHECK_FLAGS=
PCLINT=@pclint
PCLINT_FLAGS=
CCP=@ccp.py
CCP_FLAGS=


#COMAKE UUID
COMAKE_MD5=afd264fce12fed2521d2647be9662906  COMAKE


.PHONY:all
all:comake2_makefile_check test_BigInt 
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
	rm -rf test_BigInt
	rm -rf ./output/bin/test_BigInt
	rm -rf test_BigInt_test_BigInt.o

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

test_BigInt:test_BigInt_test_BigInt.o \
  ../../../output/lib/libbsl.a
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mtest_BigInt[0m']"
	$(CXX) test_BigInt_test_BigInt.o -Xlinker "-(" ../../../output/lib/libbsl.a ../../../../../com/btest/gtest/output/lib/libgtest.a \
  ../../../../../com/btest/gtest/output/lib/libgtest_main.a \
  ../../../../../quality/autotest/reportlib/cpp/libautotest.a -lpthread \
  -lcrypto \
  -lrt -Xlinker "-)" -o test_BigInt
	mkdir -p ./output/bin
	cp -f --link test_BigInt ./output/bin

test_BigInt_test_BigInt.o:test_BigInt.cpp
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mtest_BigInt_test_BigInt.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o test_BigInt_test_BigInt.o test_BigInt.cpp

endif #ifeq ($(shell uname -m),x86_64)


