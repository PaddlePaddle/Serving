#include <cassert>
#include "bsl_kv_btree_xmemcpy.h"
int main(){
    const int SZ=4096;
    unsigned char src[SZ];
    for(int i=0;i<SZ;i++){
        src[i]=0xDE;
    }        
    for(int i=0;i<SZ;i++){
        unsigned char *dst=new unsigned char[i];
        ZY::xmemcpy(dst,src,i);
        for(int j=0;j<i;j++){
            assert(dst[j]==0xDE);            
        }        
        delete [] dst;
    }
    return 0;
}
