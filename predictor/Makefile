#
# bcloud default makefile
# more details please refer to ./.bcloud.cache/makefile.*
#
.SILENT:

#
#makefile-self location, must be always on the top!!!
#
MAKEFILE_PATH_DEFAULT := $(word $(words $(MAKEFILE_LIST)), $(MAKEFILE_LIST))
MAKEFILE_DIR_DEFAULT := $(shell cd $(dir $(MAKEFILE_PATH_DEFAULT)) && pwd)

MAKEFILE_DEAULT := 'imported'

#
#Global Configs
#
WORK_ROOT := $(shell cd $(MAKEFILE_DIR_DEFAULT)/../../.. && pwd)
GCC_PATH := /opt/compiler/gcc-4.8.2/bin

#
#import global configs
#
ifneq ($(MAKEFILE_CONFIG), 'imported')
    include $(MAKEFILE_DIR_DEFAULT)/.bcloud.cache/makefile.config
endif

.PHONEY: all clean distclean cleanall help
all: main

clean:
	echo removing $(WORK_ROOT)/bc_out/baidu/paddle-serving/predictor/\*.o ...
	find $(WORK_ROOT)/bc_out/baidu/paddle-serving/predictor -name "*.o" | xargs rm -rf
	echo removing $(WORK_ROOT)/bc_out/baidu/paddle-serving/predictor/output/bin ...
	rm -rf $(WORK_ROOT)/bc_out/baidu/paddle-serving/predictor/output/bin
	echo removing $(WORK_ROOT)/bc_out/baidu/paddle-serving/predictor/output/lib ...
	rm -rf $(WORK_ROOT)/bc_out/baidu/paddle-serving/predictor/output/lib
	echo removing $(WORK_ROOT)/bc_out/baidu/paddle-serving/predictor/output/so ...
	rm -rf $(WORK_ROOT)/bc_out/baidu/paddle-serving/predictor/output/so
	echo removing $(WORK_ROOT)/bc_out/baidu/paddle-serving/predictor/output/test ...
	rm -rf $(WORK_ROOT)/bc_out/baidu/paddle-serving/predictor/output/test
	echo removing $(WORK_ROOT)/baidu/paddle-serving/predictor/output ...
	rm -rf $(WORK_ROOT)/baidu/paddle-serving/predictor/output

distclean:
	echo removing $(WORK_ROOT)/baidu/paddle-serving/predictor/Makefile ...
	rm -rf $(WORK_ROOT)/baidu/paddle-serving/predictor/Makefile
	echo removing $(WORK_ROOT)/baidu/paddle-serving/predictor/.bcloud.cache/makefile.baidu_paddle-serving_predictor ...
	rm -rf $(WORK_ROOT)/baidu/paddle-serving/predictor/.bcloud.cache/makefile.baidu_paddle-serving_predictor
	echo removing $(WORK_ROOT)/bc_out/baidu/paddle-serving/predictor ...
	rm -rf $(WORK_ROOT)/bc_out/baidu/paddle-serving/predictor
	echo removing $(WORK_ROOT)/baidu/paddle-serving/predictor/output ...
	rm -rf $(WORK_ROOT)/baidu/paddle-serving/predictor/output
	echo -e '\033[32m[NOTICE]\033[0m'
	echo -e \\tplease run \'bcloud local -M\' before next \'make\'!!!

cleanall:
	echo removing $(WORK_ROOT)/bc_out ...
	rm -rf $(WORK_ROOT)/bc_out
	echo removing $(WORK_ROOT)/baidu/paddle-serving/predictor/output ...
	rm -rf $(WORK_ROOT)/baidu/paddle-serving/predictor/output
	echo removing $(WORK_ROOT)/baidu/paddle-serving/predictor/Makefile ...
	rm -rf $(WORK_ROOT)/baidu/paddle-serving/predictor/Makefile
	echo removing $(WORK_ROOT)/baidu/paddle-serving/predictor/.bcloud.cache/makefile\* ...
	rm -rf $(WORK_ROOT)/baidu/paddle-serving/predictor/.bcloud.cache/makefile*
	echo -e '\033[32m[NOTICE]\033[0m'
	echo -e \\tplease run \'bcloud local or bcloud local -M ALL\' before next \'make\'!!!

app/ecom/elib/ecommon-lib: app/ecom/elib/ecommon-lib_deps

baidu/base/protobuf-json: baidu/base/protobuf-json_deps

baidu/base/common: baidu/base/common_deps

third-64/gtest: third-64/gtest_deps

baidu/base/bvar: baidu/base/bvar_deps

public/protobuf-json: public/protobuf-json_deps

baidu/base/cache: baidu/base/cache_deps

baidu/base/spreg: baidu/base/spreg_deps

baidu/bns/naming-lib: baidu/bns/naming-lib_deps

lib2-64/bsl: lib2-64/bsl_deps

third-64/gflags: third-64/gflags_deps

third-64/pcre: third-64/pcre_deps

lib2-64/cache: lib2-64/cache_deps

baidu/base/bthread: baidu/base/bthread_deps

third-64/leveldb: third-64/leveldb_deps

lib2-64/ullib: lib2-64/ullib_deps

public/common: public/common_deps

public/bvar: public/bvar_deps

baidu/base/bsl: baidu/base/bsl_deps

baidu/base/configure: baidu/base/configure_deps

public/spreg: public/spreg_deps

public/configure: public/configure_deps

baidu/base/ullib: baidu/base/ullib_deps

baidu/base/baidu-rpc: baidu/base/baidu-rpc_deps

third-64/libevent: third-64/libevent_deps

baidu/third-party/opencv: baidu/third-party/opencv_deps

baidu/base/dict: baidu/base/dict_deps

op/oped/noah/webfoot/naming-lib: op/oped/noah/webfoot/naming-lib_deps

baidu/elib/ecommon-lib: baidu/elib/ecommon-lib_deps

public/bthread: public/bthread_deps

public/noah/giano-lib/release/baas-lib-c: public/noah/giano-lib/release/baas-lib-c_deps

app/ecom/im/mempool: app/ecom/im/mempool_deps

baidu/base/mcpack2pb: baidu/base/mcpack2pb_deps

third-64/boost: third-64/boost_deps

public/baidu-rpc: public/baidu-rpc_deps

public/mcpack2pb: public/mcpack2pb_deps

baidu/base/iobuf: baidu/base/iobuf_deps

public/iobuf: public/iobuf_deps

baidu/im-common/mempool: baidu/im-common/mempool_deps

lib2-64/dict: lib2-64/dict_deps

help:
	echo -e 'all available targets of make($(MAKE_VERSION)):\n'
	echo '    all              : will compile all targets and run release.bcloud'
	echo '    no-release.bcloud: will compile all targets and not run release.bcloud'
	echo '    no-ut            : will compile all targets without ut and run release.bcloud'
	echo '    test             : will compile all targets, run ut and run release.bcloud'
	echo '    clean            : will only cleanup Intermediate files(such as .o, .so, .a, ut and bin) of main module:'
	echo '                       $(WORK_ROOT)/bc_out/baidu/paddle-serving/predictor/output/bin'
	echo '                       $(WORK_ROOT)/bc_out/baidu/paddle-serving/predictor/output/lib'
	echo '                       $(WORK_ROOT)/bc_out/baidu/paddle-serving/predictor/output/so'
	echo '                       $(WORK_ROOT)/bc_out/baidu/paddle-serving/predictor/\*.o'
	echo '                       $(WORK_ROOT)/bc_out/baidu/paddle-serving/predictor/output/test'
	echo '                       $(WORK_ROOT)/baidu/paddle-serving/predictor/output'
	echo '    distclean        : will cleanup makefile, intermediate files(such as .o, .so, .a, ut and bin) and pb.cc/pb.h idl.h/idl.cpp of main module:'
	echo '                       $(WORK_ROOT)/baidu/paddle-serving/predictor/Makefile'
	echo '                       $(WORK_ROOT)/baidu/paddle-serving/predictor/.bcloud.cache/makefile.baidu_paddle-serving_predictor'
	echo '                       $(WORK_ROOT)/bc_out/baidu/paddle-serving/predictor'
	echo '                       $(WORK_ROOT)/baidu/paddle-serving/predictor/output'
	echo '    cleanall         : will cleanup makefiles, intermediate files(such as .o, .so, .a, ut and bin) and pb.cc/pb.h idl.h/idl.cpp of all modules:'
	echo '                       $(WORK_ROOT)/bc_out'
	echo '                       $(WORK_ROOT)/baidu/paddle-serving/predictor/output'
	echo '                       $(WORK_ROOT)/baidu/paddle-serving/predictor/Makefile'
	echo '                       $(WORK_ROOT)/baidu/paddle-serving/predictor/.bcloud.cache/makefile*'
	echo '    help             : list all available make targets'
	echo -e '\ntargets for the compile of dependency module:'
	echo '    app/ecom/elib/ecommon-lib'
	echo '    app/ecom/im/mempool'
	echo '    baidu/base/baidu-rpc'
	echo '    baidu/base/bsl'
	echo '    baidu/base/bthread'
	echo '    baidu/base/bvar'
	echo '    baidu/base/cache'
	echo '    baidu/base/common'
	echo '    baidu/base/configure'
	echo '    baidu/base/dict'
	echo '    baidu/base/iobuf'
	echo '    baidu/base/mcpack2pb'
	echo '    baidu/base/protobuf-json'
	echo '    baidu/base/spreg'
	echo '    baidu/base/ullib'
	echo '    baidu/bns/naming-lib'
	echo '    baidu/elib/ecommon-lib'
	echo '    baidu/im-common/mempool'
	echo '    baidu/third-party/opencv'
	echo '    lib2-64/bsl'
	echo '    lib2-64/cache'
	echo '    lib2-64/dict'
	echo '    lib2-64/ullib'
	echo '    op/oped/noah/webfoot/naming-lib'
	echo '    public/baidu-rpc'
	echo '    public/bthread'
	echo '    public/bvar'
	echo '    public/common'
	echo '    public/configure'
	echo '    public/iobuf'
	echo '    public/mcpack2pb'
	echo '    public/noah/giano-lib/release/baas-lib-c'
	echo '    public/protobuf-json'
	echo '    public/spreg'
	echo '    third-64/boost'
	echo '    third-64/gflags'
	echo '    third-64/gtest'
	echo '    third-64/leveldb'
	echo '    third-64/libevent'
	echo '    third-64/pcre'



#
#import dependency modules
#
ifneq ($(MAKEFILE_BAIDU_PADDLE-SERVING_PREDICTOR), 'imported')
    include $(MAKEFILE_DIR_DEFAULT)/.bcloud.cache/makefile.baidu_paddle-serving_predictor
endif
