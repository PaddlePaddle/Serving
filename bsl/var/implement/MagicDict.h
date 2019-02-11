/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: MagicDict.h,v 1.2 2010/04/28 12:45:33 scmpf Exp $ 
 * 
 **************************************************************************/



/**
 * @file MagicDict.h
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2008/09/24 01:36:18
 * @version $Revision: 1.2 $ 
 * @brief 
 *  
 **/
#ifndef  __BSL_VAR_MAGIC_DICT_H__
#define  __BSL_VAR_MAGIC_DICT_H__

#include <map>
#if __GNUC__ >= 4 && __GNUC_MINOR__ >= 4 
#include <tr1/unordered_map>
#elif __GNUC__ >= 4 && __GNUC_MINOR__ >= 3 
#include <unordered_map>
#elif __GNUC__ >= 3
#include <ext/hash_map>
#else
#include <hash_map>
#endif
#include "bsl/pool/bsl_pool.h"
#include "bsl/var/IVar.h"
#include "bsl/var/MagicRef.h"
#include "bsl/var/Null.h"
#include "bsl/var/utils.h"

#if __GNUC__ < 3
namespace __gnu_cxx{
    /**
    * @brief 使g++2.96也统一为__gnu_cxx命名空间
    *  
    *  
    */
    using std::hash_map;
    using std::hash;
#if __GNUC__ >= 4 && __GNUC_MINOR__ >= 4 
    using std::tr1;
#endif
}
#endif

namespace bsl{
namespace var{
    //forward declarations & typedefs
    template<typename implement_t>
        class MagicBasicDict;

    template<template<class>class Alloc>
        class __MagicStdMapAdapter;

    template<template<class>class Alloc>
        class __MagicGnuHashMapAdapter;

    /**
    * @brief 封装了std::map，并使用bsl::pool_allocator的字典
    *  
    */
    typedef MagicBasicDict<__MagicStdMapAdapter<bsl::pool_allocator> >    MagicStdMapDict;

    /**
    * @brief 封装了__gnu_cxx::hash_map，并使用bsl::pool_allocator的字典
    *  
    *  
    */
    typedef MagicBasicDict<__MagicGnuHashMapAdapter<bsl::pool_allocator> >MagicGnuHashDict;

    /**
    * @brief 最推荐使用的字典
    *  
    *  目前最推荐使用的是封装了std::map，并使用bsl::pool_allocator的StdMapDict
    */
    typedef MagicStdMapDict                                          MagicDict;


    template<typename implement_t>
        class MagicBasicDict: public IVar{
        public:
            typedef IVar::string_type                       string_type;
            typedef IVar::field_type                        field_type;
            typedef typename implement_t::allocator_type    allocator_type;
            typedef DictIterator                            dict_iterator;
            typedef DictConstIterator                       dict_const_iterator;
            typedef MagicRef                                reference_type;

        public:
            MagicBasicDict(bsl::ResourcePool& rp)
                :_dict(), _alloc(), _rp(rp) {}

            explicit MagicBasicDict( bsl::ResourcePool& rp, size_t init_capacity, const allocator_type& alloc_ = allocator_type() )
                :_dict(init_capacity, alloc_), _alloc(alloc_), _rp(rp) {}

            explicit MagicBasicDict( bsl::ResourcePool& rp, const allocator_type& alloc_ )
                :_dict(alloc_), _alloc(alloc_), _rp(rp) {}

            MagicBasicDict( const MagicBasicDict& other )
                :IVar(other), _dict(other._dict), _alloc(other._alloc), _rp(other._rp) {}

            MagicBasicDict& operator = ( const MagicBasicDict& other ){
                try{
                    _dict = other._dict;
                    _alloc= other._alloc;
                }catch(bsl::Exception& e){
                    e<<"{"<<__PRETTY_FUNCTION__<<"("<<other.dump(0)<<")}";
                    throw;
                }catch(std::exception& e){
                    throw StdException(e)<<BSL_EARG<<"{"<<__PRETTY_FUNCTION__<<"("<<other.dump(0)<<")}";
                }catch(...){
                    throw UnknownException()<<BSL_EARG<<"{"<<__PRETTY_FUNCTION__<<"("<<other.dump(0)<<")}";
                }
                return *this;
            }

            virtual ~MagicBasicDict() {}

            /**
             * @brief 返回子元素个数（数组或字典）类型支持
             *
             * @return  size_t 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2010/03/17 17:35:15
            **/
            virtual size_t size() const {
                return _dict.size();
            }

            /**
             * @brief 清理函数
             *
             * @return  void 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2010/03/17 17:48:46
             **/
            virtual void clear(){
                return _dict.clear();
            }
            
            /**
             * @brief 克隆函数
             *
             * 该函数只克隆本身结点，不克隆子结点，对引用类型，克隆指向的结点
             *
             * @param [in] rp   : bsl::ResourcePool&
             * @return  MagicBasicDict& 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2010/03/17 17:58:07
            **/
            virtual MagicBasicDict& clone( bsl::ResourcePool& rp ) const {
                return rp.clone(*this);
            }

            /**
             * @brief 克隆函数
             * 
             * 由is_deep_copy参数控制是否深复制
             * 若为false, 只克隆本身结点，不克隆子结点
             * 若为true, 克隆本身结点，并且递归克隆子结点
             * 对引用类型，克隆指向的结点
             * 
             * @param [in] rp   : bsl::ResourcePool& 
             * @param [in] is_deep_copy : bool
             * @return  MagicBasicDict& 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2010/03/17 17:58:07 / modified by zhujianwei at 2011/03/22
            **/
            virtual MagicBasicDict& clone( bsl::ResourcePool& rp, bool is_deep_copy ) const {
                MagicBasicDict& res = rp.clone(*this);
                if(is_deep_copy){
                    IVar::dict_const_iterator iter = dict_begin();
                    IVar::dict_const_iterator end  = dict_end();
                    for(; iter != end ; ++iter){
                        res.set(iter->key(), (iter->value()).clone(rp, is_deep_copy));
                    }
                }
                return res;
            }

            /**
             * @brief 递归打印自身及下级结点信息
             *
             * @param [in] verbose_level   : size_t
             * @return  string_type 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2009/05/14 17:34:58
            **/
            /**
             * @brief 输出调试信息（仅用于调试）可通过verbose_level控制递归层数
             *
             * @return  string_type 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2010/03/17 19:44:20
            **/
            virtual string_type dump(size_t verbose_level = 0) const {
                string_type res;
                if ( verbose_level == 0 ){
                    res.appendf("[bsl::var::MagicBasicDict] this[%p] size[%zd]", this, _dict.size() ); 
                }else{
                    dump_to_string(*this, res, verbose_level, "", 0 );
                }
                return res;
            }

            /**
             * @brief 转化为字符串
             *
             * @return  string_type 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2010/03/17 19:44:20
            **/
            virtual string_type to_string() const {
                return dump(0);
            }

            virtual MagicBasicDict& operator = ( IVar& var ) {
                if ( typeid(var) == typeid(*this) ){
                    _dict = dynamic_cast<const MagicBasicDict&>(var)._dict;
                }else if ( var.is_dict() ){
                    _dict.clear();
                    IVar::dict_iterator iter_ = var.dict_begin();
                    IVar::dict_iterator end  = var.dict_end();
                    for( ; iter_ != end ; ++ iter_ ){
                        bsl::var::MagicRef mr(_rp);
                        mr = iter_->value();
                        _dict.insert(std::make_pair(iter_->key(), mr));
                    }
                }else{
                    throw bsl::InvalidOperationException()<<BSL_EARG<<"cast from "<<var.dump();
                }
                return *this;   
            }


            /**
             * @brief 获取类型字符串（所有var类型都支持，仅用于调试使用，勿用于类型判断）
             *
             * @return  string_type 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2010/03/17 19:44:20
            **/
            virtual string_type get_type() const {
                return "bsl::var::MagicBasicDict";
            }

            /**
             * @brief 获取类型掩码（所有var类型都支持）
             *
             * @return  IVar::mask_type 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2010/03/17 19:44:20
            **/
            virtual IVar::mask_type get_mask() const {
                return IVar::IS_DICT;
            }

            //testers
            virtual bool is_dict() const {
                return true;
            }

            //converters
            using IVar::operator =;

            //methods for dict
            virtual IVar& get( const field_type& name ) {
                iter_impl_t iter_ = _dict.find( name );
                if ( iter_ == _dict.end() ){
                    return bsl::var::Null::null;
                }
                return iter_->second;
            }

            virtual IVar& get( const field_type& name, IVar& default_value ) {
                iter_impl_t iter_ = _dict.find( name );
                if ( iter_ == _dict.end() ){
                    return default_value;
                }
                return iter_->second;
            }

            virtual const IVar& get( const field_type& name ) const {
                const_iter_impl_t iter_ = _dict.find( name );
                if ( iter_ == _dict.end() ){
                    return bsl::var::Null::null;
                }
                return iter_->second;
            }

            virtual const IVar& get( const field_type& name, const IVar& default_value ) const {
                const_iter_impl_t iter_ = _dict.find( name );
                if ( iter_ == _dict.end() ){
                    return default_value;
                }
                return iter_->second;
            }

            virtual void set( const field_type& name, IVar& value ){
                bsl::var::MagicRef mr(_rp);
                mr = value;
                typedef std::pair<typename implement_t::iterator, bool> pair_t;
                pair_t res = _dict.insert( std::pair<field_type, bsl::var::MagicRef>( name,  mr ) );
                if ( !res.second ){
                    //already exist
                    res.first->second = mr;
                }
            }

            virtual bool del( const field_type& name ) {
                iter_impl_t iter_ = _dict.find( name );
                if ( iter_ == _dict.end() ){
                    return false;
                }
                _dict.erase( iter_ );
                return true;
            }

            virtual dict_const_iterator dict_begin() const {
                return dict_const_iterator( _s_create_const_iterator( _dict.begin(), &_alloc ), _s_clone_const_iterator, _s_destroy_const_iterator, &_alloc );
            }

            virtual dict_iterator dict_begin() {
                return dict_iterator( _s_create_iterator( _dict.begin(), &_alloc ), _s_clone_iterator, _s_destroy_iterator, &_alloc );
            }

            virtual dict_const_iterator dict_end() const {
                return dict_const_iterator( _s_create_const_iterator( _dict.end(), &_alloc ), _s_clone_const_iterator, _s_destroy_const_iterator, &_alloc );
            }

            virtual dict_iterator dict_end() {
                return dict_iterator( _s_create_iterator( _dict.end(), &_alloc ), _s_clone_iterator, _s_destroy_iterator, &_alloc );
            }

            virtual const IVar& operator []( const field_type& name ) const {
                const_iter_impl_t iter_ = _dict.find(name);
                if ( iter_ == _dict.end() ){
                    throw bsl::KeyNotFoundException()<<BSL_EARG<<"key["<<name<<"]";
                }
                return iter_->second;
            }

            virtual IVar& operator []( const field_type& name ){
                iter_impl_t iter_ = _dict.find(name);
                if ( iter_ == _dict.end() ){
                    iter_ = _dict.insert(std::make_pair(name, bsl::var::MagicRef(_rp))).first;
                }
                return iter_->second;
            }

            //methods for array
#if __GNUC__ > 2
            using IVar::operator [];
            using IVar::get;
            using IVar::set;
            using IVar::del;
#else
            //avoid using bug of g++ 2.96
            virtual IVar& get( size_t idx ) {
                throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] index["<<idx<<"]";
            }

            virtual IVar& get( size_t idx, IVar& default_value ) {
                throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] index["<<idx<<"] default["<<default_value.dump(0)<<"]";
            }

            virtual const IVar& get( size_t idx ) const {
                throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] index["<<idx<<"]";
            }

            virtual const IVar& get( size_t idx, const IVar& default_value ) const {
                throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] index["<<idx<<"] default["<<default_value.dump(0)<<"]";
            }

            virtual void set( size_t idx, IVar& value_ ){
                throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] index["<<idx<<"] value["<<value_.dump(0)<<"]";
            }

            virtual bool del( size_t idx ){
                throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] index["<<idx<<"]";
            }

            virtual const IVar& operator []( int idx ) const {
                throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] index["<<idx<<"]";
            }

            virtual IVar& operator []( int idx ){
                throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] index["<<idx<<"]";
            }

#endif

        private:
            typedef typename implement_t::iterator          iter_impl_t;
            typedef typename implement_t::const_iterator    const_iter_impl_t;
            class DictIteratorImpl;
            class DictConstIteratorImpl;

            class DictIteratorImpl: public IDictIteratorImpl{
                friend class DictConstIteratorImpl;
            public:

                DictIteratorImpl( const iter_impl_t& iter_ )
                    :_iter(iter_){}

                virtual ~DictIteratorImpl() {
                    //pass
                }

                virtual const string_type& key() const {
                    return _iter->first;
                }

                virtual IVar& value() const {
                    return _iter->second;
                }

                virtual void iterate(){
                    ++ _iter;
                }

                virtual void assign( const IDictIteratorImpl& other ) {
                    const DictIteratorImpl *p = dynamic_cast<const DictIteratorImpl *>(&other);
                    if ( !p ){
                        throw bsl::BadCastException()<<BSL_EARG<<"from:"<<typeid(other).name()<<" to:DictIteratorImpl";
                    }
                    _iter   = p->_iter;
                }

                virtual bool equal_to( const IDictIteratorImpl& other ) const;

                virtual bool equal_to( const IDictConstIteratorImpl& other ) const;

            private:
                iter_impl_t _iter;
            };


            class DictConstIteratorImpl: public IDictConstIteratorImpl{
                friend class DictIteratorImpl;
            public:

                DictConstIteratorImpl( const const_iter_impl_t& iter_ )
                    :_iter(iter_){}

                virtual ~DictConstIteratorImpl() {
                    //pass
                }

                virtual const string_type& key() const {
                    return _iter->first;
                }

                virtual const IVar& value() const {
                    return _iter->second;
                }

                virtual void iterate(){
                    ++ _iter;
                }

                virtual void assign( const IDictIteratorImpl& other ) {
                    const DictIteratorImpl *p = dynamic_cast<const DictIteratorImpl *>(&other);
                    if ( !p ){
                        throw bsl::BadCastException()<<BSL_EARG<<"from:"<<typeid(other).name()<<" to:DictIteratorImpl";
                    }
                    _iter   = p->_iter;
                }

                virtual void assign( const IDictConstIteratorImpl& other ) {
                    const DictConstIteratorImpl *p = dynamic_cast<const DictConstIteratorImpl *>(&other);
                    if ( !p ){
                        throw bsl::BadCastException()<<BSL_EARG<<"from:"<<typeid(other).name()<<" to:DictIteratorImpl";
                    }
                    _iter   = p->_iter;
                }

                virtual bool equal_to( const IDictConstIteratorImpl& other ) const;

                virtual bool equal_to( const IDictIteratorImpl& other ) const;

            private:
                const_iter_impl_t _iter;
            };

            static IDictIteratorImpl * _s_create_iterator( const iter_impl_t& iter_, const void *p_alloc ){
                typedef typename allocator_type::template rebind<DictIteratorImpl>::other impl_alloc_t;
                IDictIteratorImpl *p = impl_alloc_t(*static_cast<const allocator_type*>(p_alloc)).allocate(1);   //throw
                new(p) DictIteratorImpl( iter_ );  //nothrow
                return p;
            }

            static IDictConstIteratorImpl * _s_create_const_iterator( const const_iter_impl_t& iter_, const void *p_alloc ){
                typedef typename allocator_type::template rebind<DictConstIteratorImpl>::other impl_alloc_t;
                IDictConstIteratorImpl *p = impl_alloc_t(*static_cast<const allocator_type*>(p_alloc)).allocate(1);   //throw
                new(p) DictConstIteratorImpl( iter_ );  //nothrow
                return p;
            }

            static IDictIteratorImpl * _s_clone_iterator( const IDictIteratorImpl *p_other, const void *p_alloc ){
                typedef typename allocator_type::template rebind<DictIteratorImpl>::other impl_alloc_t;
                const DictIteratorImpl *psrc = dynamic_cast<const DictIteratorImpl*>(p_other);
                if ( !psrc ){
                    throw bsl::BadCastException()<<BSL_EARG<<"from:"<<typeid(*p_other).name()<<" to:DictIteratorImpl";
                }
                IDictIteratorImpl *p = impl_alloc_t(*static_cast<const allocator_type*>(p_alloc)).allocate(1);   //throw
                new(p) DictIteratorImpl(*psrc);  
                return p;
            }

            static IDictConstIteratorImpl * _s_clone_const_iterator( const IDictConstIteratorImpl *p_other, const void *p_alloc ){
                typedef typename allocator_type::template rebind<DictConstIteratorImpl>::other impl_alloc_t;
                const DictConstIteratorImpl *psrc = dynamic_cast<const DictConstIteratorImpl*>(p_other);
                if ( !psrc ){
                    throw bsl::BadCastException()<<BSL_EARG<<"from:"<<typeid(*p_other).name()<<" to:DictIteratorImpl";
                }
                IDictConstIteratorImpl *p = impl_alloc_t(*static_cast<const allocator_type*>(p_alloc)).allocate(1);   //throw
                new(p) DictConstIteratorImpl(*psrc);  
                return p;
            }

            static void _s_destroy_iterator( IDictIteratorImpl * p, const void *p_alloc){
                typedef typename allocator_type::template rebind<DictIteratorImpl>::other impl_alloc_t;
                DictIteratorImpl *_p = dynamic_cast<DictIteratorImpl *>(p); 
                if ( _p ){
                    _p->~DictIteratorImpl();
                    impl_alloc_t(*static_cast<const allocator_type*>(p_alloc)).deallocate( _p, 1 );
                }
            }

            static void _s_destroy_const_iterator( IDictConstIteratorImpl * p, const void *p_alloc ){
                typedef typename allocator_type::template rebind<DictConstIteratorImpl>::other impl_alloc_t;
                DictConstIteratorImpl *_p = dynamic_cast<DictConstIteratorImpl *>(p); 
                if ( _p ){
                    _p->~DictConstIteratorImpl();
                    impl_alloc_t(*static_cast<const allocator_type*>(p_alloc)).deallocate( _p, 1 );
                }
            }

            implement_t     _dict;
            allocator_type  _alloc;   //TODO：为了简单直观，没有做EBO优化，以后可以加上
            bsl::ResourcePool& _rp;
        };

    template<typename implement_t>
        inline bool MagicBasicDict<implement_t>::DictIteratorImpl::equal_to( const IDictIteratorImpl& other ) const {
            const DictIteratorImpl *p = dynamic_cast<const DictIteratorImpl *>(&other);
            return p != NULL  && _iter == p->_iter;
        }

    template<typename implement_t>
        inline bool MagicBasicDict<implement_t>::DictIteratorImpl::equal_to( const IDictConstIteratorImpl& other ) const {
            const DictConstIteratorImpl *p = dynamic_cast<const DictConstIteratorImpl *>(&other);
            return p != NULL  && p->_iter == _iter;
        }

    template<typename implement_t>
        inline bool MagicBasicDict<implement_t>::DictConstIteratorImpl::equal_to( const IDictIteratorImpl& other ) const {
            const DictIteratorImpl *p = dynamic_cast<const DictIteratorImpl *>(&other);
            return p != NULL  && _iter == p->_iter; 
        }

    template<typename implement_t>
        inline bool MagicBasicDict<implement_t>::DictConstIteratorImpl::equal_to( const IDictConstIteratorImpl& other ) const {
            const DictConstIteratorImpl *p = dynamic_cast<const DictConstIteratorImpl *>(&other);
            return p != NULL  && _iter == p->_iter;
        }

    /**
    * @brief 修整一下std::map的接口，提供一个接受allocator的构造函数
    *  
    *  典型的adapter模式
    *  该adapter还能使其运行时名字没有这么吓人:-P
    *  
    */
    template<template<class>class Alloc>
        class __MagicStdMapAdapter: public std::map<IVar::string_type, bsl::var::MagicRef, std::less<IVar::string_type>, Alloc<std::pair<IVar::string_type, bsl::var::MagicRef> > >{
        public:
            typedef std::map<IVar::string_type, bsl::var::MagicRef, std::less<IVar::string_type>, Alloc<std::pair<IVar::string_type, bsl::var::MagicRef> > > base_type;
            typedef typename base_type::allocator_type  allocator_type;
            typedef typename base_type::iterator        iterator;
            typedef typename base_type::const_iterator  const_iterator;

            /**
             * @brief 默认构造函数
             *
             * @see 
             * @author chenxm
             * @date 2009/04/08 11:22:53
            **/
            __MagicStdMapAdapter()
                :base_type(){}

            /**
             * @brief 使用allocator初始化的构造函数
             *
             * @param [in] alloc_   : const allocator_type&
             * @return  explicit 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2009/04/08 11:23:05
            **/
            explicit __MagicStdMapAdapter( const allocator_type& alloc_ )
                :base_type(std::less<IVar::string_type>(), alloc_ ){}

            /**
             * @brief 使用init_capacity与allocator初始化的构造函数
             *
             * 对于std::map，init_capacity没有意义，直接忽略
             * @param [in] init_capacity   : size_t
             * @param [in] alloc_   : const allocator_type&
             * @return  explicit 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2009/04/08 12:02:53
            **/
            explicit __MagicStdMapAdapter( size_t /*init_capacity*/, const allocator_type& alloc_ )
                :base_type(std::less<IVar::string_type>(), alloc_ ){}

            //inherit everything else
        };

    /**
    * @brief 修整一下__gnu_cxx::hash_map的接口，提供一个接受allocator的构造函数
    *  
    *  典型的adapter模式
    *  该adapter还能使其运行时名字没有这么吓人:-P
    *  
    */
    template<template<class>class Alloc>
#if __GNUC__ >= 4 && __GNUC_MINOR__ >= 4 
        class __MagicGnuHashMapAdapter: public std::tr1::unordered_map<
                                        IVar::string_type, 
                                        bsl::var::MagicRef, 
                                        ::__gnu_cxx::hash<IVar::string_type>, 
                                        std::equal_to<IVar::string_type>, 
                                        Alloc<bsl::var::MagicRef> > {
#elif __GNUC__ >= 4 && __GNUC_MINOR__ >= 3  
         class __MagicGnuHashMapAdapter: public std::unordered_map<
                                         IVar::string_type, 
                                         bsl::var::MagicRef, 
                                         __gnu_cxx::hash<IVar::string_type>, 
                                         std::equal_to<IVar::string_type>, 
                                         Alloc<bsl::var::MagicRef> > {
#else
        class __MagicGnuHashMapAdapter: public __gnu_cxx::hash_map<
                                        IVar::string_type, 
                                        bsl::var::MagicRef, 
                                        ::__gnu_cxx::hash<IVar::string_type>, 
                                        std::equal_to<IVar::string_type>, 
                                        Alloc<bsl::var::MagicRef> > {
#endif
        public:
#if __GNUC__ >= 4 && __GNUC_MINOR__ >= 4 
             typedef std::tr1::unordered_map<
                 IVar::string_type,  
                 bsl::var::MagicRef, 
                 ::__gnu_cxx::hash<IVar::string_type>,
                 std::equal_to<IVar::string_type>, 
                 Alloc<bsl::var::MagicRef> > base_type;
#elif __GNUC__ >= 4 && __GNUC_MINOR__ >= 3
            typedef std::unordered_map<
                IVar::string_type, 
                bsl::var::MagicRef, 
                __gnu_cxx::hash<IVar::string_type>, 
                std::equal_to<IVar::string_type>, 
                Alloc<bsl::var::MagicRef> > base_type;
#else
            typedef __gnu_cxx::hash_map<
                IVar::string_type, 
                bsl::var::MagicRef, 
                ::__gnu_cxx::hash<IVar::string_type>, 
                std::equal_to<IVar::string_type>, 
                Alloc<bsl::var::MagicRef> > base_type;
#endif
            typedef typename base_type::allocator_type  allocator_type;
            typedef typename base_type::iterator        iterator;
            typedef typename base_type::const_iterator  const_iterator;

            /**
             * @brief 默认构造函数
             *
             * @see 
             * @author chenxm
             * @date 2009/04/08 11:22:15
            **/
            __MagicGnuHashMapAdapter()
                :base_type(){}

            /**
             * @brief 使用allocator初始化的构造函数
             *
             * 100是__gnu_cxx::hash_map使用的默认容量值
             * @param [in] alloc_   : const allocator_type&
             * @return  explicit 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2009/04/08 11:22:27
            **/
            explicit __MagicGnuHashMapAdapter( const allocator_type& alloc_ )
                :base_type(100, typename base_type::hasher(), typename base_type::key_equal(), alloc_ ){}

            /**
             * @brief 使用init_capacity与allocator初始化的构造函数
             *
             * @param [in] init_capacity   : size_t
             * @param [in] alloc_   : const allocator_type&
             * @return  explicit 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2009/04/08 11:58:52
            **/
            explicit __MagicGnuHashMapAdapter( size_t init_capacity, const allocator_type& alloc_ )
                :base_type(init_capacity, typename base_type::hasher(), typename base_type::key_equal(), alloc_ ){}

            //inherit everything else
        };
}}   //namespace bsl::var

#endif  //__BSL_VAR_DICT_H__

/* vim: set ts=4 sw=4 sts=4 tw=100 */
