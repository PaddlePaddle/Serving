/* -*- c++ -*-
   copy[write] by dirlt(dirtysalt1987@gmail.com) */
#ifndef _BSL_KV_BTREE_ARCHIVE_TRAITS_H_
#define _BSL_KV_BTREE_ARCHIVE_TRAITS_H_

//如果依赖头文件,那么直接在bsl_kv_btree.h里面填写.
//btree的archive接口...汗...
template<typename T>
class _btree_archive_traits {
    T *_ptr;

public:
    _btree_archive_traits(T *ptr):_ptr(ptr) {
    }
    _btree_archive_traits():_ptr(NULL) {
    }
    uint64_t size()const {
        return (uint64_t)sizeof(T);
    }
    int serialize(char *buf,const uint64_t buf_size,
                  uint64_t &data_pos)const {
        int err=BTREE_OK;
        if(_ptr && buf_size>data_pos &&
                (buf_size-data_pos)>=(uint64_t)size()) {
            T *tmp=(T*)(buf+data_pos);
            *tmp=*_ptr;
            data_pos+=(uint64_t)size();
        } else {
            err=BTREE_UNKNOWN_ERROR;
        }
        return err;
    }
    int deserialize(char *buf,const uint64_t buf_size,
                    uint64_t &data_pos) {
        int err=BTREE_OK;
        if(_ptr && buf_size > data_pos &&
                (buf_size - data_pos)>=(uint64_t)size()) {
            T *tmp=(T*)(buf+data_pos);
            *_ptr=*tmp;
            data_pos+=(uint64_t)size();
        } else {
            err=BTREE_UNKNOWN_ERROR;
        }
        return err;
    }
};

//为std::string定制的特化版本..
template<>
class _btree_archive_traits<std::string> {
    std::string *_ptr;

public:
    _btree_archive_traits(std::string *ptr):_ptr(ptr) {
    }
    _btree_archive_traits():_ptr(NULL) {
    }
    uint64_t size()const {
        if(!_ptr) {
            return UNDEF_ID;
        }
        size_t siz=_ptr->size();
        return (uint64_t)sizeof(siz)+
               (uint64_t)siz;
    }
    int serialize(char *buf,const uint64_t buf_size,
                  uint64_t &data_pos)const {
        if(!_ptr) {
            return BTREE_UNKNOWN_ERROR;
        }
        size_t siz=_ptr->size();
        //首先序列化长度部分..
        if(buf_size>data_pos &&
                (buf_size-data_pos)>=(uint64_t)sizeof(siz)) {
            size_t *tmp=(size_t*)(buf+data_pos);
            *tmp=siz;
            data_pos+=(uint64_t)sizeof(siz);
        } else {
            return BTREE_UNKNOWN_ERROR;
        }
        //然后序列化字符串部分..
        if(buf_size>data_pos &&
                (buf_size-data_pos)>=(uint64_t)siz) {
            memcpy(buf+data_pos,_ptr->c_str(),siz);
            data_pos+=siz;
        } else {
            return BTREE_UNKNOWN_ERROR;
        }
        return BTREE_OK;
    }
    int deserialize(char *buf,const uint64_t buf_size,
                    uint64_t &data_pos) {
        if(!_ptr) {
            return BTREE_UNKNOWN_ERROR;
        }
        //首先读入size部分.
        size_t siz;
        if(buf_size > data_pos &&
                (buf_size - data_pos)>=(uint64_t)sizeof(siz)) {
            size_t *tmp=(size_t*)(buf+data_pos);
            siz=*tmp;
            data_pos+=(uint64_t)sizeof(siz);
        } else {
            return BTREE_UNKNOWN_ERROR;
        }
        //然后读入字符串部分
        const char *str;
        if(buf_size > data_pos &&
                (buf_size - data_pos)>=(uint64_t)siz) {
            str=(const char*)(buf+data_pos);
            data_pos += (uint64_t)siz;
        } else {
            return BTREE_UNKNOWN_ERROR;
        }
        //然后进行初始化.
        //bsl construct没有使用两个参数初始化的...
        //所以是用原始的..
        ::new(static_cast<void*>(_ptr))std::string(str,siz);
        return BTREE_OK;
    }
};

//bsl::string和std::string应该是走同样的逻辑的..
template<>
class _btree_archive_traits<bsl::string> {
    bsl::string *_ptr;

public:
    _btree_archive_traits(bsl::string *ptr):_ptr(ptr) {
    }
    _btree_archive_traits():_ptr(NULL) {
    }
    uint64_t size()const {
        if(!_ptr) {
            return UNDEF_ID;
        }
        size_t siz=_ptr->size();
        return (uint64_t)sizeof(siz)+
               (uint64_t)siz;
    }
    int serialize(char *buf,const uint64_t buf_size,
                  uint64_t &data_pos)const {
        if(!_ptr) {
            return BTREE_UNKNOWN_ERROR;
        }
        size_t siz=_ptr->size();
        //首先序列化长度部分..
        if(buf_size>data_pos &&
                (buf_size-data_pos)>=(uint64_t)sizeof(siz)) {
            size_t *tmp=(size_t*)(buf+data_pos);
            *tmp=siz;
            data_pos+=(uint64_t)sizeof(siz);
        } else {
            return BTREE_UNKNOWN_ERROR;
        }
        //然后序列化字符串部分..
        if(buf_size>data_pos &&
                (buf_size-data_pos)>=(uint64_t)siz) {
            memcpy(buf+data_pos,_ptr->c_str(),siz);
            data_pos+=siz;
        } else {
            return BTREE_UNKNOWN_ERROR;
        }
        return BTREE_OK;
    }
    int deserialize(char *buf,const uint64_t buf_size,
                    uint64_t &data_pos) {
        if(!_ptr) {
            return BTREE_UNKNOWN_ERROR;
        }
        //首先读入size部分.
        size_t siz;
        if(buf_size > data_pos &&
                (buf_size - data_pos)>=(uint64_t)sizeof(siz)) {
            size_t *tmp=(size_t*)(buf+data_pos);
            siz=*tmp;
            data_pos+=(uint64_t)sizeof(siz);
        } else {
            return BTREE_UNKNOWN_ERROR;
        }
        //然后读入字符串部分
        const char *str;
        if(buf_size > data_pos &&
                (buf_size - data_pos)>=(uint64_t)siz) {
            str=(const char*)(buf+data_pos);
            data_pos += (uint64_t)siz;
        } else {
            return BTREE_UNKNOWN_ERROR;
        }
        //然后进行初始化.
        //bsl construct没有使用两个参数初始化的...
        //所以是用原始的..
        ::new(static_cast<void*>(_ptr))bsl::string(str,siz);
        return BTREE_OK;
    }
};

#endif
