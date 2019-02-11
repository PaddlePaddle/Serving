/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: Dict.h,v 1.4 2010/04/28 12:45:33 scmpf Exp $ 
 * 
 **************************************************************************/



/**
 * @file Dict.h
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2008/09/24 01:36:18
 * @version $Revision: 1.4 $ 
 * @brief 
 *  
 **/
#ifndef  __BSL_VAR_DICT_H__
#define  __BSL_VAR_DICT_H__

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
#include "bsl/var/Ref.h"
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
        class BasicDict;

    template<template<class>class Alloc>
        class __StdMapAdapter;

    template<template<class>class Alloc>
        class __GnuHashMapAdapter;

    /**
    * @brief 封装了std::map，并使用bsl::pool_allocator的字典
    *  
    */
    typedef BasicDict<__StdMapAdapter<bsl::pool_allocator> >    StdMapDict;

    /**
    * @brief 封装了__gnu_cxx::hash_map，并使用bsl::pool_allocator的字典
    *  
    *  
    */
    typedef BasicDict<__GnuHashMapAdapter<bsl::pool_allocator> >GnuHashDict;

    /**
    * @brief 最推荐使用的字典
    *  
    *  目前最推荐使用的是封装了std::map，并使用bsl::pool_allocator的StdMapDict
    */
    typedef StdMapDict                                          Dict;


    /**
     * @brief 字典类型
     *          
     **/

    template<typename implement_t>
        class BasicDict: public IVar{
        public:
            /**
             * @brief 字符串类型
             */
            typedef IVar::string_type                       string_type;
            /**
             * @brief 字段类型
             */
            typedef IVar::field_type                        field_type;
            /**
             * @brief allocator类型
             */
            typedef typename implement_t::allocator_type    allocator_type;
            /**
             * @brief 字典迭代器
             */
            typedef DictIterator                            dict_iterator;
            /**
             * @brief 只读字典迭代器
             */
            typedef DictConstIterator                       dict_const_iterator;
            /**
             * @brief 类型的引用 
             */
            typedef Ref                                     reference_type;

        public:
            BasicDict()
                :_dict(), _alloc() {}

            /**
             * @brief 构造函数
             *          
             * @param  [in] init_capacity   : size_t
             * @param  [in] alloc_          : allocator_type& 
             * @return
             * @retval
             * @see
             * @author liaoshangbin
             * @data 2010/08/12 19:50:51
             **/
            BasicDict( 
                    size_t init_capacity, 
                    const allocator_type& alloc_ = allocator_type() 
                    )
                :_dict(init_capacity, alloc_), _alloc(alloc_) {}

            /**
             * @brief 构造函数
             *          
             * @param  [in] alloc_      : const allocator_type&
             * @return
             * @retval
             * @see
             * @author liaoshangbin
             * @data 2010/08/12 19:52:00
             **/
            explicit BasicDict( const allocator_type& alloc_ )
                :_dict(alloc_), _alloc(alloc_) {}
            
            /**
             * @brief 复制构造函数
             *          
             * @param  [in] other   : const BasicDict&
             * @return
             * @retval
             * @see
             * @author liaoshangbin
             * @data 2010/08/12 19:52:33
             **/
            BasicDict( const BasicDict& other )
                :IVar(other), _dict(other._dict), _alloc(other._alloc) {}
            
            /**
             * @brief 复制赋值运算符
             *          
             * @param  [in] other   : const BasicDict&
             * @return  BasicDict&
             * @retval
             * @see
             * @author liaoshangbin
             * @data 2010/08/12 19:53:29
             **/
            BasicDict& operator = ( const BasicDict& other ){
                try{
                    _dict = other._dict;
                    _alloc = other._alloc;
                }catch(bsl::Exception& e){
                    e<<"{"<<__PRETTY_FUNCTION__<<"("<<other.dump(0)<<")}";
                    throw;
                }catch(std::exception& e){
                    throw StdException(e)
                        <<BSL_EARG<<"{"<<__PRETTY_FUNCTION__<<"("<<other.dump(0)<<")}";
                }catch(...){
                    throw UnknownException()
                        <<BSL_EARG<<"{"<<__PRETTY_FUNCTION__<<"("<<other.dump(0)<<")}";
                }
                return *this;
            }

            virtual ~BasicDict() {}

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
             **@param [in] rp   : bsl::ResourcePool&
             * @return  Dict&
             * @retval
             * @see 
             * @author chenxm
             * @date 2010/03/17 17:58:07
            **/
            virtual BasicDict& clone( bsl::ResourcePool& rp) const {
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
             * @return  BasicDict& 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2010/03/17 17:58:07 / modified by zhujianwei at 2011/03/22
            **/
            virtual BasicDict& clone( bsl::ResourcePool& rp, bool is_deep_copy ) const {
                BasicDict& res = rp.clone(*this);
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
                    res.appendf("[bsl::var::BasicDict] this[%p] size[%zd]", this, _dict.size() ); 
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

            /**
             * @brief 将IVar类型转化为Dict类型
             *          
             * @param  [in] var     : IVar&
             * @return BasicDict&
             * @retval
             * @see
             * @author liaoshangbin
             * @data 2010/08/12 19:54:43
             **/
            virtual BasicDict& operator = ( IVar& var ) {
                if ( typeid(var) == typeid(*this) ){
                    _dict = dynamic_cast<const BasicDict&>(var)._dict;
                }else if ( var.is_dict() ){
                    _dict.clear();
                    IVar::dict_iterator iter_ = var.dict_begin();
                    IVar::dict_iterator end  = var.dict_end();
                    for( ; iter_ != end ; ++iter_ ){
                        _dict[iter_->key()] = iter_->value();
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
                return "bsl::var::BasicDict";
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

            /**
             * @brief 是否为字典类型
             *
             **/
            //testers
            virtual bool is_dict() const {
                return true;
            }

            //converters
            using IVar::operator =;

            /**
             * @brief 返回字段名绑定 
             *        字典类型支持，is_dict()返回true
             *          
             * @param
             * @return
             * @retval
             * @see
             * @author liaoshangbin
             * @data 2010/08/12 19:56:53
             **/
            //methods for dict
            virtual IVar& get( const field_type& name ) {
                iter_impl_t iter_ = _dict.find( name );
                if ( iter_ == _dict.end() ){
                    return bsl::var::Null::null;
                }
                return iter_->second;
            }
            /**
             * @brief 返回字段名绑定，若字段名不存在，返回默认值
             *
             **/
            virtual IVar& get( const field_type& name, IVar& default_value ) {
                iter_impl_t iter_ = _dict.find( name );
                if ( iter_ == _dict.end() ){
                    return default_value;
                }
                return iter_->second;
            }
            /**
             * @brief 返回字段名绑定
             *
             **/
            virtual const IVar& get( const field_type& name ) const {
                const_iter_impl_t iter_ = _dict.find( name );
                if ( iter_ == _dict.end() ){
                    return bsl::var::Null::null;
                }
                return iter_->second;
            }

            /**
             * @brief 返回字段名绑定，若字段名不存在，返回默认值
             *
             **/
            virtual const IVar& get( const field_type& name, const IVar& default_value ) const {
                const_iter_impl_t iter_ = _dict.find( name );
                if ( iter_ == _dict.end() ){
                    return default_value;
                }
                return iter_->second;
            }
            /**
             * @brief 设置字段名绑定
             *
             **/
            virtual void set( const field_type& name, IVar& value ){
                _dict[name] = value;
            }

            /**
             * @brief 删除字段名绑定
             *
             **/
            virtual bool del( const field_type& name ) {
                iter_impl_t iter_ = _dict.find( name );
                if ( iter_ == _dict.end() ){
                    return false;
                }
                _dict.erase( iter_ );
                return true;
            }

            /**
             * @brief 返回只读起始字典迭代器
             *
             **/
            virtual dict_const_iterator dict_begin() const {
                return dict_const_iterator( 
                        _s_create_const_iterator( _dict.begin(), &_alloc ), 
                        _s_clone_const_iterator, 
                        _s_destroy_const_iterator, 
                        &_alloc 
                        );
            }
            /**
             * @brief 返回起始字典迭代器
             *
             **/
            virtual dict_iterator dict_begin() {
                return dict_iterator( 
                        _s_create_iterator( _dict.begin(), &_alloc ), 
                        _s_clone_iterator, 
                        _s_destroy_iterator, 
                        &_alloc 
                        );
            }
            /**
             * @brief 返回只读末尾字典迭代器
             *
             **/
            virtual dict_const_iterator dict_end() const {
                return dict_const_iterator( 
                        _s_create_const_iterator( _dict.end(), &_alloc ), 
                        _s_clone_const_iterator, 
                        _s_destroy_const_iterator, 
                        &_alloc 
                        );
            }
            /**
             * @brief 返回末尾字典迭代器
             *
             **/
            virtual dict_iterator dict_end() {
                return dict_iterator( 
                        _s_create_iterator( _dict.end(), &_alloc ), 
                        _s_clone_iterator, 
                        _s_destroy_iterator, 
                        &_alloc 
                        );
            }
            /**
             * @brief 返回/设置下标绑定
             *
             */
            virtual const IVar& operator []( const field_type& name ) const {
                const_iter_impl_t iter_ = _dict.find(name);
                if ( iter_ == _dict.end() ){
                    throw bsl::KeyNotFoundException()<<BSL_EARG<<"key["<<name<<"]";
                }
                return iter_->second;
            }

            /**
             * @brief 返回/设置下标绑定
             *
             **/
            virtual IVar& operator []( const field_type& name ){
                return _dict[name];
            }

            //methods for array
#if __GNUC__ > 2
            using IVar::operator [];
            using IVar::get;
            using IVar::set;
            using IVar::del;
#else
            //avoid using bug of g++ 2.96

            /**
             * @brief 获取下标idx处的IVar对象的引用对象，数组接口
             *
             */
            virtual IVar& get( size_t idx ) {
                throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] index["<<idx<<"]";
            }
            /**
             * @brief 获取下标idx处的IVar对象的引用对象，数组接口
             *
             */
            virtual IVar& get( size_t idx, IVar& default_value ) {
                throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] index["<<idx<<"] default["<<default_value.dump(0)<<"]";
            }
            /**
             * @brief 获取下标idx处的IVar对象的引用对象，数组接口
             *
             */
            virtual const IVar& get( size_t idx ) const {
                throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] index["<<idx<<"]";
            }
            /**
             * @brief 获取下标idx处的IVar对象的引用对象，数组接口
             *
             */
            virtual const IVar& get( size_t idx, const IVar& default_value ) const {
                throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] index["<<idx<<"] default["<<default_value.dump(0)<<"]";
            }
            /**
             * @brief 设置下标idx上的绑定
             *
             */
            virtual void set( size_t idx, IVar& value_ ){
                throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] index["<<idx<<"] value["<<value_.dump(0)<<"]";
            }
            /**
             * @brief 删除下标idx上的绑定
             *
             */
            virtual bool del( size_t idx ){
                throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] index["<<idx<<"]";
            }
            /**
             * @brief 返回/设置下标绑定
             *
             */
            virtual const IVar& operator []( int idx ) const {
                throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] index["<<idx<<"]";
            }
            /**
             * @brief 返回/设置下标绑定
             *
             */
            virtual IVar& operator []( int idx ){
                throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] index["<<idx<<"]";
            }

#endif

        private:
            /**
             * @brief 字典类型的迭代器
             *
             */
            typedef typename implement_t::iterator          iter_impl_t;
            /**
             * @brief 只读字典类型的迭代器
             *
             */
            typedef typename implement_t::const_iterator    const_iter_impl_t;
            class DictIteratorImpl;
            class DictConstIteratorImpl;
            
            /**
             * @brief 字典迭代器的实现
             *
             */
            class DictIteratorImpl: public IDictIteratorImpl{
                friend class DictConstIteratorImpl;
            public:

                /**
                 * @brief 复制构造函数
                 */
                DictIteratorImpl( const iter_impl_t& iter_ )
                    :_iter(iter_){}

                virtual ~DictIteratorImpl() {
                    //pass
                }

                /**
                 * @brief 得到偏移位置
                 */
                virtual const string_type& key() const {
                    return _iter->first;
                }
                /**
                 * @brief 得到偏移位置上的绑定值
                 */
                virtual IVar& value() const {
                    return _iter->second;
                }
                /**
                 * @brief 偏移位置后移
                 */
                virtual void iterate(){
                    ++ _iter;
                }
                /**
                 * @brief 将其它字典类型迭代器对其赋值
                 */
                virtual void assign( const IDictIteratorImpl& other ) {
                    const DictIteratorImpl *p = dynamic_cast<const DictIteratorImpl *>(&other);
                    if ( !p ){
                        throw bsl::BadCastException()
                            <<BSL_EARG<<"from:"<<typeid(other).name()<<" to:DictIteratorImpl";
                    }
                    _iter   = p->_iter;
                }
                /**
                 * @brief 判断是否等于其它只读字典类型迭代器
                 */
                virtual bool equal_to( const IDictIteratorImpl& other ) const;
                /**
                 * @brief 判断是否等于其它字典类型迭代器
                 */
                virtual bool equal_to( const IDictConstIteratorImpl& other ) const;

            private:
                /**
                 * @brief 字典迭代器
                 */
                iter_impl_t _iter;
            };
            /**
             * @brief 只读字典迭代器的实现
             */
            class DictConstIteratorImpl: public IDictConstIteratorImpl{
                friend class DictIteratorImpl;
            public:

                /**
                 * @brief 复制构造函数
                 */
                DictConstIteratorImpl( const const_iter_impl_t& iter_ )
                    :_iter(iter_){}

                virtual ~DictConstIteratorImpl() {
                    //pass
                }
                /**
                 * @brief 得到偏移位置
                 */
                virtual const string_type& key() const {
                    return _iter->first;
                }
                /**
                 * @brief 得到偏移位置上的绑定值
                 */
                virtual const IVar& value() const {
                    return _iter->second;
                }
                /**
                 * @brief 偏移位置后移
                 */
                virtual void iterate(){
                    ++ _iter;
                }
                /**
                 * @brief 将其它字典类型迭代器对其赋值
                 */
                virtual void assign( const IDictIteratorImpl& other ) {
                    const DictIteratorImpl *p = dynamic_cast<const DictIteratorImpl *>(&other);
                    if ( !p ){
                        throw bsl::BadCastException()
                            <<BSL_EARG<<"from:"<<typeid(other).name()<<" to:DictIteratorImpl";
                    }
                    _iter   = p->_iter;
                }
                /**
                 * @brief 将其它只读字典类型迭代器对其赋值
                 */
                virtual void assign( const IDictConstIteratorImpl& other ) {
                    const DictConstIteratorImpl *p = 
                        dynamic_cast<const DictConstIteratorImpl *>(&other);
                    if ( !p ){
                        throw bsl::BadCastException()
                            <<BSL_EARG<<"from:"<<typeid(other).name()<<" to:DictIteratorImpl";
                    }
                    _iter   = p->_iter;
                }
                /**
                 * @brief 判断是否等于其它只读字典类型迭代器
                 */
                virtual bool equal_to( const IDictConstIteratorImpl& other ) const;
                /**
                 * @brief 判断是否等于其它字典类型迭代器
                 */
                virtual bool equal_to( const IDictIteratorImpl& other ) const;

            private:
                /**
                 * @brief 只读字典迭代器
                 */
                const_iter_impl_t _iter;
            };

            /**
             * @brief 创建一个字典的迭代器
             */
            static IDictIteratorImpl * _s_create_iterator( 
                    const iter_impl_t& iter_, 
                    const void *p_alloc 
                    ){
                typedef 
                    typename allocator_type::template rebind<DictIteratorImpl>::other impl_alloc_t;
                IDictIteratorImpl *p = impl_alloc_t(*static_cast<const allocator_type*>(p_alloc)).allocate(1);   //throw
                new(p) DictIteratorImpl( iter_ );  //nothrow
                return p;
            }
            /**
             * @brief 创建一个只读字典的迭代器
             */
            static IDictConstIteratorImpl * _s_create_const_iterator( 
                    const const_iter_impl_t& iter_, 
                    const void *p_alloc 
                    ){
                typedef 
                    typename allocator_type::
                    template rebind<DictConstIteratorImpl>::other impl_alloc_t;
                IDictConstIteratorImpl *p = impl_alloc_t(*static_cast<const allocator_type*>(p_alloc)).allocate(1);   //throw
                new(p) DictConstIteratorImpl( iter_ );  //nothrow
                return p;
            }
            /**
             * @brief 克隆字典的迭代器
             */
            static IDictIteratorImpl * _s_clone_iterator( 
                    const IDictIteratorImpl *p_other, 
                    const void *p_alloc 
                    ){
                typedef 
                    typename allocator_type::template rebind<DictIteratorImpl>::other impl_alloc_t;
                const DictIteratorImpl *psrc = dynamic_cast<const DictIteratorImpl*>(p_other);
                if ( !psrc ){
                    throw bsl::BadCastException()
                        <<BSL_EARG<<"from:"<<typeid(*p_other).name()<<" to:DictIteratorImpl";
                }
                IDictIteratorImpl *p = impl_alloc_t(*static_cast<const allocator_type*>(p_alloc)).allocate(1);   //throw
                new(p) DictIteratorImpl(*psrc);  
                return p;
            }
            /**
             * @brief 克隆只读字典的迭代器
             */
            static IDictConstIteratorImpl * _s_clone_const_iterator( 
                    const IDictConstIteratorImpl *p_other, 
                    const void *p_alloc 
                    ){
                typedef typename allocator_type::
                    template rebind<DictConstIteratorImpl>::other impl_alloc_t;
                const DictConstIteratorImpl *psrc = 
                    dynamic_cast<const DictConstIteratorImpl*>(p_other);
                if ( !psrc ){
                    throw bsl::BadCastException()
                        <<BSL_EARG<<"from:"<<typeid(*p_other).name()<<" to:DictIteratorImpl";
                }
                IDictConstIteratorImpl *p = impl_alloc_t(*static_cast<const allocator_type*>(p_alloc)).allocate(1);   //throw
                new(p) DictConstIteratorImpl(*psrc);  
                return p;
            }
            /**
             * @brief 销毁字典的迭代器
             */
            static void _s_destroy_iterator( IDictIteratorImpl * p, const void *p_alloc){
                typedef 
                    typename allocator_type::template rebind<DictIteratorImpl>::other impl_alloc_t;
                DictIteratorImpl *_p = dynamic_cast<DictIteratorImpl *>(p); 
                if ( _p ){
                    _p->~DictIteratorImpl();
                    impl_alloc_t(*static_cast<const allocator_type*>(p_alloc)).deallocate( _p, 1 );
                }
            }
            /**
             * @brief 销毁只读字典的迭代器
             */
            static void _s_destroy_const_iterator( 
                    IDictConstIteratorImpl * p, 
                    const void *p_alloc 
                    ){
                typedef typename allocator_type::
                    template rebind<DictConstIteratorImpl>::other impl_alloc_t;
                DictConstIteratorImpl *_p = dynamic_cast<DictConstIteratorImpl *>(p); 
                if ( _p ){
                    _p->~DictConstIteratorImpl();
                    impl_alloc_t(*static_cast<const allocator_type*>(p_alloc)).deallocate( _p, 1 );
                }
            }
            /**
             * @brief 内部字典
             */
            implement_t     _dict;
            /**
             * @brief 内部的allocator
             */
            allocator_type  _alloc;   //TODO：为了简单直观，没有做EBO优化，以后可以加上
        };

    template<typename implement_t>
        inline bool BasicDict<implement_t>::DictIteratorImpl::equal_to( 
                const IDictIteratorImpl& other 
                ) const {
            const DictIteratorImpl *p = dynamic_cast<const DictIteratorImpl *>(&other);
            return p != NULL  && _iter == p->_iter;
        }

    template<typename implement_t>
        inline bool BasicDict<implement_t>::DictIteratorImpl::equal_to( 
                const IDictConstIteratorImpl& other 
                ) const {
            const DictConstIteratorImpl *p = dynamic_cast<const DictConstIteratorImpl *>(&other);
            return p != NULL  && p->_iter == _iter;
        }

    template<typename implement_t>
        inline bool BasicDict<implement_t>::DictConstIteratorImpl::equal_to( 
                const IDictIteratorImpl& other 
                ) const {
            const DictIteratorImpl *p = dynamic_cast<const DictIteratorImpl *>(&other);
            return p != NULL  && _iter == p->_iter; 
        }

    template<typename implement_t>
        inline bool BasicDict<implement_t>::DictConstIteratorImpl::equal_to( 
                const IDictConstIteratorImpl& other 
                ) const {
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
        class __StdMapAdapter: public std::map<
                               IVar::string_type, 
                               bsl::var::Ref, 
                               std::less<IVar::string_type>, 
                               Alloc<std::pair<IVar::string_type, bsl::var::Ref> > 
                                    >{
        public:
            /**
             * @brief 重命名std::map的接口
             *
             */
            typedef std::map<
                IVar::string_type, 
                bsl::var::Ref, 
                std::less<IVar::string_type>, 
                Alloc<std::pair<IVar::string_type, bsl::var::Ref> > 
                    > base_type;
            /**
             * @brief allocator的类型
             *
             */
            typedef typename base_type::allocator_type  allocator_type;
            /**
             * @brief 迭代器
             *
             */
            typedef typename base_type::iterator        iterator;
            /**
             * @brief 只读迭代器
             *
             */
            typedef typename base_type::const_iterator  const_iterator;

            /**
             * @brief 默认构造函数
             *
             * @see 
             * @author chenxm
             * @date 2009/04/08 11:22:53
            **/
            __StdMapAdapter()
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
            explicit __StdMapAdapter( const allocator_type& alloc_ )
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
            explicit __StdMapAdapter( size_t /*init_capacity*/, const allocator_type& alloc_ )
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
        class __GnuHashMapAdapter:  public std::tr1::unordered_map<
                                        IVar::string_type,
                                        bsl::var::Ref, 
                                        ::__gnu_cxx::hash<IVar::string_type>,
                                        std::equal_to<IVar::string_type>,
                                        Alloc<bsl::var::Ref>
                                    > {
#elif __GNUC__ >= 4 && __GNUC_MINOR__ >= 3

        class __GnuHashMapAdapter:  public std::unordered_map<
                                        IVar::string_type,
                                        bsl::var::Ref, 
                                        __gnu_cxx::hash<IVar::string_type>,
                                        std::equal_to<IVar::string_type>,
                                        Alloc<bsl::var::Ref>
                                    > {
#else
        class __GnuHashMapAdapter:  public __gnu_cxx::hash_map<
                                        IVar::string_type, 
                                        bsl::var::Ref, 
                                        __gnu_cxx::hash<IVar::string_type>, 
                                        std::equal_to<IVar::string_type>, 
                                        Alloc<bsl::var::Ref> 
                                    > {
#endif
        public:
            /**
             * @brief 重命名__gnu_cxx::hash_map接口
             *
             */
#if __GNUC__ >= 4 && __GNUC_MINOR__ >= 4
            typedef std::tr1::unordered_map<
                IVar::string_type, 
                bsl::var::Ref, 
                ::__gnu_cxx::hash<IVar::string_type>,
                std::equal_to<IVar::string_type>,
                Alloc<bsl::var::Ref>
            > base_type;
#elif __GNUC__ >= 4 && __GNUC_MINOR__ >= 3
            typedef std::unordered_map<
                IVar::string_type, 
                bsl::var::Ref, 
                __gnu_cxx::hash<IVar::string_type>,
                std::equal_to<IVar::string_type>,
                Alloc<bsl::var::Ref>
            > base_type;
#else
            typedef __gnu_cxx::hash_map<
                IVar::string_type, 
                bsl::var::Ref, 
                __gnu_cxx::hash<IVar::string_type>, 
                std::equal_to<IVar::string_type>, 
                Alloc<bsl::var::Ref> 
            > base_type;
#endif
            /**
             * @brief allocator的类型
             *
             */
            typedef typename base_type::allocator_type  allocator_type;
            /**
             * @brief 迭代器
             *
             */
            typedef typename base_type::iterator        iterator;
            /**
             * @brief 只读迭代器
             *
             */
            typedef typename base_type::const_iterator  const_iterator;

            /**
             * @brief 默认构造函数
             *
             * @see 
             * @author chenxm
             * @date 2009/04/08 11:22:15
            **/
            __GnuHashMapAdapter()
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
            explicit __GnuHashMapAdapter( const allocator_type& alloc_ )
                :base_type(
                        100, 
                        typename base_type::hasher(), 
                        typename base_type::key_equal(), 
                        alloc_ 
                        ){}

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
            explicit __GnuHashMapAdapter( size_t init_capacity, const allocator_type& alloc_ )
                :base_type(
                        init_capacity, 
                        typename base_type::hasher(), 
                        typename base_type::key_equal(), 
                        alloc_ 
                        ){}

            //inherit everything else
        };
}}   //namespace bsl::var

#endif  //__BSL_VAR_DICT_H__

/* vim: set ts=4 sw=4 sts=4 tw=100 */
