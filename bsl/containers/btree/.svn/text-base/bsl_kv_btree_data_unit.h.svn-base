/* -*- c++ -*-
   copy[write] by dirlt(dirtysalt1987@gmail.com) */
#ifndef _BSL_KV_BTREE_DATA_UNIT_H_
#define _BSL_KV_BTREE_DATA_UNIT_H_

//如何依赖头文件,那么直接在bsl_kv_btree.h里面填写.
template<typename K,
         typename V,
         typename KARCHIVE=_btree_archive_traits<K>,
         typename VARCHIVE=_btree_archive_traits<V>,
         typename LT=std::less<K>,
         typename EQ=std::equal_to<K> >
//这里很难使用Archive这个类
//因为BSL里面的Archive根本就没有序列化到内存的接口..
//所以自己写Archive类.._btree_archive_traits.
//kv btree内部应该包含的内容...
//应该包含V的所有接口..
class _bsl_kv_btree_du {
public:
    typedef _bsl_kv_btree_du<K,V,KARCHIVE,VARCHIVE,LT,EQ> Self;
    //这样就能够得到最终的数据内容了...
    typedef K key_type;
    typedef V value_type;

public:
    //同时持有k,v.
    K _key;
    V _value;
    uint64_t _id;
    LT _lt;
    EQ _eq;
    _btree_archive_traits<uint64_t> _idarchive;
    KARCHIVE _karchive;
    VARCHIVE _varchive;

public:
    //有默认的初始化构造函数和copy构造函数...
    //并且用来初始化archive结构...
    _bsl_kv_btree_du():
        _key(),_value(),
        _idarchive(&_id),_karchive(&_key),
        _varchive(&_value) {
        init();
    }
    _bsl_kv_btree_du(const K&k,const V&v):
        _key(k),_value(v),
        _idarchive(&_id),_karchive(&_key),
        _varchive(&_value) {
        init();
    }
    _bsl_kv_btree_du(const _bsl_kv_btree_du &du):
        _key(du._key),_value(du._value),
        _idarchive(&_id),_karchive(&_key),
        _varchive(&_value) {
        init();
    }
    const Self & operator = (const Self & v) {
        //重复使用的话,那么释放之前的内存.....
        //其实这样是可以使用变长内存的...
        if(this!=&v) {
            bsl_destruct(this);
            bsl_construct(this,v);
        }
        return *this;
    }
    void init() {
        _id=UNDEF_ID;
    }
    //下面这些方法仅仅对于定长有效,但是对于变长的话,
//     //是需要重新实现的..
//     //这个可以固定住..
//     uint64_t get_store_size()const{
//         return (uint64_t)sizeof(Self);
//     }
//     //必须使用archive实现...
//     //但是archive没有实现copy到内存的接口...:(..
//     int store(char *buf,const uint64_t buf_size,
//               uint64_t &data_pos)const {
//                   int err=BTREE_OK;
//         if(buf_size > data_pos &&
//            (buf_size - data_pos)>= (uint64_t)sizeof(V)) {
//             //V *tmp=(V*)(buf+data_pos);
//             //*tmp=_value;
//             //data_pos+=(uint64_t)sizeof(V);
//             Self *tmp=(Self*)(buf+data_pos);
//             *tmp=*this;
//             data_pos+=(uint64_t)sizeof(Self);
//         } else {
//             err=BTREE_UNKNOWN_ERROR;
//         }
//         return err;
//     }
//     int load(char *buf,const uint64_t buf_size,
//              uint64_t &data_pos) {
//         int err=BTREE_OK;
//         if(buf_size > data_pos &&
//            (buf_size - data_pos)>= (uint64_t)sizeof(V)) {
//             //V *tmp=(V*)(buf+data_pos);
//             //_value=*tmp;
//             //data_pos+=(uint64_t)sizeof(V);
//             Self *tmp=(Self*)(buf+data_pos);
//             *this=*tmp;
//             data_pos+=(uint64_t)sizeof(Self);
//         } else {
//             err=BTREE_UNKNOWN_ERROR;
//         }
//         return err;
//     }
    //关于变长内存的话,那么store/load接口就需要花一些心思..
    uint64_t get_store_size()const {
        return (uint64_t)_idarchive.size()+
               (uint64_t)_karchive.size()+
               (uint64_t)_varchive.size();
    }
    int store(char *buf,const uint64_t buf_size,
              uint64_t &data_pos)const {
        int err=BTREE_OK;
        err=_karchive.serialize(buf,buf_size,data_pos);
        if(err!=BTREE_OK) {
            return err;
        }
        err=_varchive.serialize(buf,buf_size,data_pos);
        if(err!=BTREE_OK) {
            return err;
        }
        err=_idarchive.serialize(buf,buf_size,data_pos);
        if(err!=BTREE_OK) {
            return err;
        }
        return err;
    }
    int load(char *buf,const uint64_t buf_size,
             uint64_t &data_pos) {
        int err=BTREE_OK;
        err=_karchive.deserialize(buf,buf_size,data_pos);
        if(err!=BTREE_OK) {
            return err;
        }
        err=_varchive.deserialize(buf,buf_size,data_pos);
        if(err!=BTREE_OK) {
            return err;
        }
        err=_idarchive.deserialize(buf,buf_size,data_pos);
        if(err!=BTREE_OK) {
            return err;
        }
        return err;
    }
    void set_id(uint64_t id) {
        _id=id;
    }
    uint64_t get_id()const {
        return _id;
    }
    //使用Compare对象实现比较接口..
    bool operator==(const K &k)const {
        return _eq(_key,k);
    }
    bool operator==(const Self &self)const {
        return _eq(_key,self._key);
    }

    bool operator<(const K&k)const {
        return _lt(_key,k);
    }
    bool operator<(const Self &self)const {
        return _lt(_key,self._key);
    }
};

#endif
