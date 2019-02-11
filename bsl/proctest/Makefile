BSL=../
BENCH=$(BSL)/yperfbench/bin/
PYTHON=python
ifeq ($(MAC),64)
	MAKECONF=make.conf
	RSSH=work@tc-testing-com00.tc:xwtest/
else
	MAKECONF=make32.conf
	RSSH=ziyuan@zjm-ziyuan-test01.zjm:xwtest/
	PYTHON=python2.5
endif

all :
	chmod +x $(BENCH)/bench
	make -C $(BSL)/yperfbench
	$(PYTHON)  $(BENCH)/bench -dconf/ -c$(MAKECONF) -C -V

test :
	$(PYTHON)  $(BENCH)/bench -dconf/ -c$(MAKECONF) -V

output :
	$(PYTHON)  $(BENCH)/bench -dconf/ -c$(MAKECONF) -fresult -V

remote :
	$(PYTHON)  $(BENCH)/bench -dconf/ -c$(MAKECONF) -fresult -V -r$(RSSH)

clean :
	rm -f *.o bsl_unsafe_hash .bench*

