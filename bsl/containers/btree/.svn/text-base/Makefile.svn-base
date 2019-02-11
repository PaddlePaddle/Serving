VERSION_ID=bsl_btree[by YangZhenKun][JustForX8664]
CXX=g++

INCPATH=-I. -I../../../  #-I$(HOME)/utils/valgrind-3.5.0/memcheck -I$(HOME)/utils/valgrind-3.5.0/include 
LDPATH=-L.
LDFLAGS=-lpthread 
CXXFLAGS =-fsigned-char -Wall -W -g -O2 -pipe -fPIC -finline-functions \
	-D__STDC_LIMIT_MACROS -D_GNU_SOURCE -D_XOPEN_SOURE=500 \
	-D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -D__VERSION_ID__="\"$(VERSION_ID)\""

all:output
%.o:%.cpp
	$(CXX) -o $@ -c $^ $(CXXFLAGS) $(INCPATH)
TARGETS=test_bsl_kv_btree test_bsl_kv_btree_string test_mt_bsl_kv_btree test_xmemcpy_correctness
app:$(TARGETS)
%:%.cpp
	$(CXX) -o $@ $^ $(CXXFLAGS) $(INCPATH) $(LDPATH) $(LDFLAGS)
BSL=../../
OUTINC=$(BSL)/output/include/bsl/containers/btree
output:$(LIBNAME)
	rm -rf $(OUTINC)
	mkdir -p $(OUTINC)
	cp -r asm-i386 $(OUTINC)
	cp -r asm-x86_64 $(OUTINC)
	cp *h $(OUTINC)
	cp gen_xmemcpy_h.py $(OUTINC)
CLEANFILES=$(OBJS) $(TARGETS)
.PHONY: clean
clean:
	rm -rf $(CLEANFILES) $(APPTARGETS) *~
dist:clean
	cd ../;tar czvf btree.tar.gz btree;cd btree
