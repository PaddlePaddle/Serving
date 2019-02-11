#include "bsl_kv_btree.h"

#ifndef BSL_BTREE_WORKS
#error "BSL btree doesn't work"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>

using namespace std;
//对于key需要指定各种compare的方法...
struct Key_t {
    unsigned long long key;
    Key_t():key(0) {
    }
    bool operator==(const Key_t &k)const {
        return key==k.key;
    }
    bool operator!=(const Key_t &k)const {
        return key!=k.key;
    }
    bool operator<(const Key_t &k)const {
        return key<k.key;
    }
    bool operator>(const Key_t &k)const {
        return key>k.key;
    }
};

struct Data_t {
    unsigned long long data;
    Data_t():data(0) {
    }
};

typedef bsl::bsl_kv_btree_build<Key_t,Data_t> kv_btree_build_t;
typedef bsl::bsl_kv_btree_search<Key_t,Data_t> kv_btree_search_t;

//顺序插入这些数据..
static const int DATA_NR=1000000;
void * thread_set(void *arg){
    kv_btree_build_t &bt=*(kv_btree_build_t*)arg;
    for(int i=0;i<DATA_NR;i++){
        Key_t k;
        Data_t v;
        k.key=i;
        v.data=i+2;
        //插入成功..
        //因为是直接覆盖写的...
        bt.set(k,v);
    }
    return NULL;
}
void *thread_get(void *arg){
    kv_btree_build_t &bt=*(kv_btree_build_t*)arg;
    for(int i=0; i<DATA_NR; i++) {
        Key_t k;
        Data_t v;
        k.key=i;
        if(bt.get(k,&v)==bsl::BTREE_EXIST){
            //这里面data=key+2.
            assert(v.data==(k.key+2));
        }
    }
    return NULL;
}
void *thread_build_iterator(void *arg){
    kv_btree_build_t &bt=*(kv_btree_build_t*)arg;
    for(kv_btree_build_t::iterator it=bt.begin();
        it!=bt.end();++it){
        Key_t k;
        Data_t v;
        v=*it;
        k=it.ref_key();
        assert(v.data==(k.key+2));
    }
    return NULL;
}
void *thread_search_iterator(void *arg){
    kv_btree_build_t &bt=*(kv_btree_build_t*)arg;
    kv_btree_search_t sbt;
    assert(bt.snapshot(sbt)==0);    
    //验证计数...
    size_t cnt=0;
    for(kv_btree_search_t::iterator it=sbt.begin();
        it!=sbt.end();++it){
        Key_t k;
        Data_t v;
        v=*it;
        k=it.ref_key();
        assert(v.data==(k.key+2));
        cnt++;
    }
    assert(cnt==sbt.size());
    return NULL;
}

static const int WR_THREAD_NR=1;
static const int RD_THREAD_NR=10;
int main(){
    kv_btree_build_t bt;
    pthread_t tid_set[WR_THREAD_NR];
    for(int i=0;i<WR_THREAD_NR;i++){
        pthread_create(tid_set+i,NULL,thread_set,&bt);
    }
    pthread_t tid_get[RD_THREAD_NR];
    pthread_t tid_build_iter[RD_THREAD_NR];
    pthread_t tid_search_iter[RD_THREAD_NR];
    for(int i=0;i<RD_THREAD_NR;i++){
        pthread_create(tid_get+i,NULL,thread_get,&bt);
        pthread_create(tid_build_iter+i,NULL,thread_build_iterator,&bt);
        pthread_create(tid_search_iter+i,NULL,thread_search_iterator,&bt);
    }
    for(int i=0;i<WR_THREAD_NR;i++){
        pthread_join(tid_set[i],NULL);
    }
    for(int i=0;i<RD_THREAD_NR;i++){
        pthread_join(tid_get[i],NULL);
        pthread_join(tid_build_iter[i],NULL);
        pthread_join(tid_search_iter[i],NULL);
    }
    return 0;
}
