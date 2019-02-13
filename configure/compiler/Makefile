#* -*- makefile -*- 
#   copy[write] by dirlt(zhang_yan@baidu.com)
#   date time:Mon Nov 17 22:44:02 CST 2008
#   file name:Makefile 
all: 
	make -C ./src
clean:
	make -C ./src clean
	make -C ./demo clean
	rm -rf output
test:
	make -C ./testcase
mydemo:
	make -C ./demo
output:
	mkdir ./output
	cp -r ./include ./output
	cp -r ./lib ./output
dist:
	tar czvf ../idlcompiler.tar.gz ../idlcompiler

