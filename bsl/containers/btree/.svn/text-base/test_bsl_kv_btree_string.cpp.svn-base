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
typedef string Key_t;
typedef string Data_t;
typedef bsl::bsl_kv_btree_build<Key_t,Data_t> kv_btree_build_t;
typedef bsl::bsl_kv_btree_search<Key_t,Data_t> kv_btree_search_t;
string strfy(unsigned int i){
    char buf[1024];
    snprintf(buf,sizeof(buf),"string.%04u",i);
    return buf;
}

int test_insert(kv_btree_build_t &bt){
    for(int i=0; i<10; i++) {
        Key_t k=strfy(i);
        Data_t v=strfy(i);
        //插入成功..
        assert(bt.set(k,v)==bsl::BTREE_INSERT_SUC);
    }
    for(int i=0; i<10; i++) {
        Key_t k=strfy(i);
        Data_t v=strfy(i);
        //数据存在..
        assert(bt.set(k,v)==bsl::BTREE_EXIST);
    }
    for(int i=0; i<10; i++) {
        Key_t k=strfy(i);
        Data_t v=strfy(i);
        //数据被覆盖..
        assert(bt.set(k,v,!0)==bsl::BTREE_OVERWRITE);
    }
    for(int i=10; i<20; i++) {
        Key_t k=strfy(i);
        Data_t v=strfy(i);
        assert(bt.set(k,v,!0)==bsl::BTREE_INSERT_SUC);
    }
    return 0;
}

int test_batch_mutate(kv_btree_build_t &bt){
    bt.make_batch_mutate();
    bt.end_batch_mutate();
    return 0;
}

int test_get(kv_btree_build_t &bt){
    for(int i=0; i<20; i++) {
        Key_t k=strfy(i);
        Data_t v;
        assert(bt.get(k,&v)==bsl::BTREE_EXIST);
        assert(bt.get(k)==bsl::BTREE_EXIST);
        assert(v==k);
    }
    for(int i=20; i<30; i++) {
        Key_t k=strfy(i);
        Data_t v;
        assert(bt.get(k,&v)==bsl::BTREE_NOEXIST);
    }
    return 0;
}

int test_get_const(const kv_btree_build_t &bt){
    for(int i=0; i<20; i++) {
        Key_t k=strfy(i);
        Data_t v;
        assert(bt.get(k,&v)==bsl::BTREE_EXIST);
        assert(bt.get(k)==bsl::BTREE_EXIST);
        assert(v==k);
    }
    for(int i=20; i<30; i++) {
        Key_t k=strfy(i);
        Data_t v;
        assert(bt.get(k,&v)==bsl::BTREE_NOEXIST);
    }
    return 0;
}

int test_erase(kv_btree_build_t &bt){
    for(int i=10; i<20; i++) {
        Key_t k=strfy(i);
        assert(bt.erase(k)==bsl::BTREE_EXIST);
    }
    for(int i=20; i<30; i++) {
        Key_t k=strfy(i);
        assert(bt.erase(k)==bsl::BTREE_NOEXIST);
    }
    return 0;
}

int test_clear(kv_btree_build_t &bt){
    bt.clear();
    return 0;
}

int test_store_checkpoint(kv_btree_build_t &bt){
    assert(bt.make_checkpoint()==bsl::BTREE_OK);
    int fd=-1;
    fd=open("bt.img",O_WRONLY | O_CREAT,0666);
    assert(fd!=-1);
    assert(bt.store(fd,0)==bsl::BTREE_OK);
    assert(bt.end_checkpoint()==bsl::BTREE_OK);
    close(fd);
    return 0;
}

int test_load_checkpoint(kv_btree_build_t &bt){
    int fd=-1;
    fd=open("bt.img",O_RDONLY,0666);
    assert(fd!=-1);
    assert(bt.load(fd,0)==bsl::BTREE_OK);
    close(fd);
    return 0;
}

int test_size(const kv_btree_build_t &bt,uint64_t v){
    assert(bt.size()==v);
    return 0;
}

int test_iterator(kv_btree_build_t &bt){
    unsigned long long v=0;
    for(kv_btree_build_t::iterator it=bt.begin();
        it!=bt.end(); ++it) {
        assert((*it)==strfy(v));
        v++;
    }
    v=0;
    for(kv_btree_build_t::iterator it=bt.begin();
        it!=bt.end(); it++) {
        assert((*it)==strfy(v));
        v++;
    }
    return 0;
}

int test_const_iterator(const kv_btree_build_t &bt){
    unsigned long long v=0;
    for(kv_btree_build_t::const_iterator it=bt.begin();
        it!=bt.end(); ++it) {
        assert((*it)==strfy(v));
        v++;
    }
    v=0;
    for(kv_btree_build_t::const_iterator it=bt.begin();
        it!=bt.end(); it++) {
        assert((*it)==strfy(v));
        v++;
    }
    return 0;
}

int test_reverse_iterator(kv_btree_build_t &bt){
    unsigned long long v=19;
    for(kv_btree_build_t::reverse_iterator it=bt.rbegin();
        it!=bt.rend(); ++it) {
        assert((*it)==strfy(v));
        v--;
    }
    v=19;
    for(kv_btree_build_t::reverse_iterator it=bt.rbegin();
        it!=bt.rend(); it++) {
        assert((*it)==strfy(v));
        v--;
    }
    return 0;
}

int test_const_reverse_iterator(const kv_btree_build_t &bt){
    unsigned long long v=19;
    for(kv_btree_build_t::const_reverse_iterator it=bt.rbegin();
        it!=bt.rend(); ++it) {
        assert((*it)==strfy(v));
        v--;
    }
    v=19;
    for(kv_btree_build_t::const_reverse_iterator it=bt.rbegin();
        it!=bt.rend(); it++) {
        assert((*it)==strfy(v));
        v--;
    }
    return 0;
}

int test_snapshot(kv_btree_build_t &bt,
                  kv_btree_search_t &sbt){
    assert(bt.snapshot(sbt)==bsl::BTREE_OK);
    return 0;
}

int test_snapshot_get(kv_btree_search_t &bt){
    for(int i=0; i<20; i++) {
        Key_t k=strfy(i);
        Data_t v;
        assert(bt.get(k,&v)==bsl::BTREE_EXIST);
        assert(bt.get(k)==bsl::BTREE_EXIST);
        assert(v==k);
    }
    for(int i=20; i<30; i++) {
        Key_t k=strfy(i);
        Data_t v;
        assert(bt.get(k,&v)==bsl::BTREE_NOEXIST);
    }
    return 0;
}

int test_snapshot_get_const(const kv_btree_search_t &bt){
    for(int i=0; i<20; i++) {
        Key_t k=strfy(i);
        Data_t v;
        assert(bt.get(k,&v)==bsl::BTREE_EXIST);
        assert(bt.get(k)==bsl::BTREE_EXIST);
        assert(v==k);
    }
    for(int i=20; i<30; i++) {
        Key_t k=strfy(i);
        Data_t v;
        assert(bt.get(k,&v)==bsl::BTREE_NOEXIST);
    }
    return 0;
}

int test_store_snapshot(kv_btree_search_t &sbt){
    int fd=-1;
    fd=open("bt.img",O_WRONLY | O_CREAT,0666);
    assert(fd!=-1);
    assert(sbt.store(fd,0)==bsl::BTREE_OK);
    close(fd);
    return 0;
}

int test_snapshot_iterator(kv_btree_search_t &bt){
    unsigned long long v=0;
    for(kv_btree_search_t::iterator it=bt.begin();
        it!=bt.end(); ++it) {
        assert((*it)==strfy(v));
        v++;
    }
    v=0;
    for(kv_btree_search_t::iterator it=bt.begin();
        it!=bt.end(); it++) {
        assert((*it)==strfy(v));
        v++;
    }
    return 0;
}

int test_snapshot_const_iterator(const kv_btree_search_t &bt){
    unsigned long long v=0;
    for(kv_btree_search_t::const_iterator it=bt.begin();
        it!=bt.end(); ++it) {
        assert((*it)==strfy(v));
        v++;
    }
    v=0;
    for(kv_btree_search_t::const_iterator it=bt.begin();
        it!=bt.end(); it++) {
        assert((*it)==strfy(v));
        v++;
    }
    return 0;
}

int test_snapshot_reverse_iterator(kv_btree_search_t &bt){
    unsigned long long v=19;
    for(kv_btree_search_t::reverse_iterator it=bt.rbegin();
        it!=bt.rend(); ++it) {
        assert((*it)==strfy(v));
        v--;
    }
    v=19;
    for(kv_btree_search_t::reverse_iterator it=bt.rbegin();
        it!=bt.rend(); it++) {
        assert((*it)==strfy(v));
        v--;
    }
    return 0;
}

int test_snapshot_const_reverse_iterator(const kv_btree_search_t &bt){
    unsigned long long v=19;
    for(kv_btree_search_t::const_reverse_iterator it=bt.rbegin();
        it!=bt.rend(); ++it) {
        assert((*it)==strfy(v));
        v--;
    }
    v=19;
    for(kv_btree_search_t::const_reverse_iterator it=bt.rbegin();
        it!=bt.rend(); it++) {
        assert((*it)==strfy(v));
        v--;
    }
    return 0;
}

int test_print_iterator(kv_btree_build_t &bt){
    for(kv_btree_build_t::iterator it=bt.begin(); it!=bt.end(); ++it) {
        cout<<*it<<endl;
    }
    return 0;
}

int test_print_snapshot_iterator(kv_btree_search_t &bt){
    for(kv_btree_search_t::iterator it=bt.begin(); it!=bt.end(); ++it) {
        cout<<*it<<endl;
    }
    return 0;
}

int main(){
    kv_btree_build_t bt;
    kv_btree_search_t sbt;
    test_insert(bt);
    test_batch_mutate(bt);
    test_size(bt,20);

    test_get(bt);
    test_get_const(bt);
    test_iterator(bt);
    test_const_iterator(bt);
    test_reverse_iterator(bt);
    test_const_reverse_iterator(bt);
    test_snapshot(bt,sbt);

    test_store_checkpoint(bt);
    test_erase(bt);
    test_size(bt,10);
    test_clear(bt);
    test_size(bt,0);
    test_load_checkpoint(bt);
    test_size(bt,20);

    test_iterator(bt);
    test_const_iterator(bt);
    test_reverse_iterator(bt);
    test_const_reverse_iterator(bt);
    //test_print_iterator(bt);
    test_clear(bt);
    test_size(bt,0);

    //测试snapshot..
    test_snapshot_get(sbt);
    test_snapshot_get_const(sbt);
    test_store_snapshot(sbt);
    test_snapshot_iterator(sbt);
    test_snapshot_const_iterator(sbt);
    test_snapshot_reverse_iterator(sbt);
    test_snapshot_const_reverse_iterator(sbt);

    //载入snapshot的内容是否可读..
    test_load_checkpoint(bt);
    test_size(bt,20);
    test_clear(bt);
    system("rm -rf bt.img");
    return 0;
}
