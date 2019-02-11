#include <cxxtest/TestSuite.h>
#include <bsl/pool/bsl_cachedpool.h>

class bsl_test_pool : public CxxTest::TestSuite
{
public:
	void test_normal(void) {   
		bsl::cached_mempool o;
		o.free(0, 0);
	}  
	void test_normal1(void) {   
		bsl::mempool *p = new bsl::cached_mempool();
		p->free(0, 0);
                delete p;
	}
    void test_clear(){
        bsl::cached_mempool o;
        o.malloc(2048);
        o.malloc(2049);
        o.clear();
        o.malloc(2049);
    }
};
