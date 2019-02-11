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
  -I/home/luowei/workroot/libsrc/bsl/output/include
DEP_INCPATH=-I../com/btest/gtest \
  -I../com/btest/gtest/include \
  -I../com/btest/gtest/output \
  -I../com/btest/gtest/output/include \
  -I../lib2-64/ullib \
  -I../lib2-64/ullib/include \
  -I../lib2-64/ullib/output \
  -I../lib2-64/ullib/output/include \
  -I../quality/autotest/reportlib/cpp \
  -I../quality/autotest/reportlib/cpp/include \
  -I../quality/autotest/reportlib/cpp/output \
  -I../quality/autotest/reportlib/cpp/output/include

#============ CCP vars ============
CCHECK=@ccheck.py
CCHECK_FLAGS=
PCLINT=@pclint
PCLINT_FLAGS=
CCP=@ccp.py
CCP_FLAGS=


#COMAKE UUID
COMAKE_MD5=22995f423ceeb5f605432ae4280e1ed3  COMAKE


.PHONY:all
all:comake2_makefile_check ex_test_bsl_hash_multimap 
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
	rm -rf ex_test_bsl_hash_multimap
	rm -rf ./output/bin/ex_test_bsl_hash_multimap
	rm -rf ex_test_bsl_hash_multimap_test_bsl_hash_multimap.o

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

ex_test_bsl_hash_multimap:ex_test_bsl_hash_multimap_test_bsl_hash_multimap.o \
  /home/luowei/workroot/libsrc/bsl/output/lib/libbsl.a
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mex_test_bsl_hash_multimap[0m']"
	$(CXX) ex_test_bsl_hash_multimap_test_bsl_hash_multimap.o -Xlinker "-(" /home/luowei/workroot/libsrc/bsl/output/lib/libbsl.a ../com/btest/gtest/output/lib/libgtest.a \
  ../com/btest/gtest/output/lib/libgtest_main.a \
  ../lib2-64/ullib/lib/libullib.a \
  ../quality/autotest/reportlib/cpp/libautotest.a -lpthread \
  -lrt -Xlinker "-)" -o ex_test_bsl_hash_multimap
	mkdir -p ./output/bin
	cp -f --link ex_test_bsl_hash_multimap ./output/bin

ex_test_bsl_hash_multimap_test_bsl_hash_multimap.o:test_bsl_hash_multimap.cpp
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mex_test_bsl_hash_multimap_test_bsl_hash_multimap.o[0m']"
	$(CXX) -c $(INCPATH) $(DEP_INCPATH) $(CPPFLAGS) $(CXXFLAGS)  -o ex_test_bsl_hash_multimap_test_bsl_hash_multimap.o test_bsl_hash_multimap.cpp

endif #ifeq ($(shell uname -m),x86_64)


