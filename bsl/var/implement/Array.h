/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: Array.h,v 1.4 2010/04/28 12:45:33 scmpf Exp $ 
 * 
 **************************************************************************/



/**
 * @file Array.h
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2008/09/24 01:36:18
 * @version $Revision: 1.4 $ 
 * @brief 
 *  
 **/
#ifndef  __BSL_VAR_ARRAY_H__
#define  __BSL_VAR_ARRAY_H__

#include <deque>
#include "bsl/pool/bsl_pool.h"
#include "bsl/var/IVar.h"
#include "bsl/var/Ref.h"
#include "bsl/var/Null.h"
#include "bsl/var/utils.h"

namespace bsl{
namespace var{
    //typedefs
    //template<template<class>class allocator_t>
    template<typename implement_t> 
        class BasicArray;
    
    /**
     * @brief 封装了std::deque<Ref,bsl::pool_allocator<Ref> >的数组类型
     *
     **/
    typedef BasicArray<std::deque<Ref,pool_allocator<Ref> > > Array;

    /**
    * @brief var::Array数组类型,与var::Dict保持类似的接口
    *         implement_t为BasicArray的内部实现
    *  
    *  
    */
    template<typename implement_t>
        class BasicArray: public IVar{
        public:
            /**
             * @brief Var::Array的string类型
             *
             */
            typedef IVar::string_type                       string_type;
            /**
             * @brief Var::Array的字段类型
             */
            typedef IVar::field_type                        field_type;
            /**
             * @brief Var::Array的allocator
             */
            typedef typename implement_t::allocator_type    allocator_type;
            /**
             * @brief Var::Array的迭代器
             */
            typedef ArrayIterator                           array_iterator;
            /**
             * @brief Var::Array的const迭代器
             */
            typedef ArrayConstIterator                      array_const_iterator;
            /**
             * @brief Var::Array的reference_type
             */
            typedef Ref                                     reference_type;

            /**
             * @brief var::Array内部的数组实现别名
             *
             **/
            typedef implement_t                             array_type;

        public:
            BasicArray()
                :_array() {  }

            /**
             * @brief BasicArray的构造函数
             *          
             * @param [in] allocator    : allocator_type&
             * @return
             * @retval
             * @see
             * @author liaoshangbin
             * @data 2010/08/10 11:27:05
             **/
            explicit BasicArray( const allocator_type& _alloc ) :_array(_alloc){}
            
            /**
             * @brief BasicArray的复制构造函数
             *          
             * @param [in] other    : const BasicArray&
             * @return
             * @retval
             * @see
             * @author liaoshangbin
             * @data 2010/08/10 11:27:58
             **/
            BasicArray( const BasicArray& other ) :IVar(other), _array(other._array){}

            /**
             * @brief BasicArray的复制赋值运算符
             *          
             * @param [in] other    : const BasicArray&
             * @return
             * @retval
             * @see
             * @author liaoshangbin
             * @data 2010/08/10 11:28:47
             **/
            BasicArray& operator = ( const BasicArray& other ){
                try{
                    _array = other._array;
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
                
            virtual ~BasicArray() {}

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
                return _array.size();
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
                return _array.clear();
            }
            
            /**
             * @brief 克隆函数
             * 
             * 该函数只克隆本身结点，不克隆子结点，对引用类型，克隆指向的结点
             * 
             **@param [in] rp   : bsl::ResourcePool&
             * @return  BasicArray&
             * @retval
             * @see 
             * @author chenxm
             * @date 2010/03/17 17:58:07
            **/
            virtual BasicArray& clone( bsl::ResourcePool& rp) const {
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
             **@param [in] rp   : bsl::ResourcePool& 
             * @param [in] is_deep_copy : bool
             * @return  BasicArray& 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2010/03/17 17:58:07 / modified by zhujianwei at 2010/03/22
            **/
            virtual BasicArray& clone( bsl::ResourcePool& rp, bool is_deep_copy ) const {
                BasicArray& res = rp.clone(*this);
                if(is_deep_copy){
                    size_t n = _array.size();
                    for(size_t i = 0; i < n; ++i){
                       res.set(i, _array[i].clone(rp, is_deep_copy));
                    }
                }
                return res;
            }

            /**
             * @brief 打印Array的内部状态
             *          
             * @param [in] verbose_level    : size_t
             * @return string_type
             * @retval
             * @see
             * @author liaoshangbin
             * @data 2010/08/10 11:32:56
             **/
            virtual string_type dump(size_t verbose_level = 0) const {
                string_type res;
                if ( verbose_level == 0 ){
                    res.appendf(
                            "[bsl::var::BasicArray] this[%p] size[%zd]", 
                            this, _array.size() 
                            ); 
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
             * @brief 将IVar类型转化为Array类型
             *          
             * @param  [in] var     : IVar
             * @return BasicArray&
             * @retval
             * @see
             * @author liaoshangbin
             * @data 2010/08/10 11:34:53
             **/
            virtual BasicArray& operator = ( IVar& var ) {
                if ( typeid(var) == typeid(*this) ){
                    return operator =(dynamic_cast<BasicArray&>(var));
                }else if ( var.is_array() ){
                    size_t var_size = var.size();
                    try{
                        _array.resize( var_size );
                    }catch(bsl::Exception& e){
                        throw;
                    }catch(std::exception& e){
                        throw StdException(e)<<BSL_EARG;
                    }catch(...){
                        throw UnknownException()<<BSL_EARG;
                    }
                    for( size_t i = 0; i < var_size; ++ i ){
                        _array[i] = var[int(i)];
                    }
                }else{
                    throw bsl::InvalidOperationException()<<BSL_EARG<<"cast from "<<var.dump(0);
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
                return "bsl::var::BasicArray";
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
#ifdef PHP_COMLOG
                return IVar::_IS_ARRAY;
#else
                return IVar::IS_ARRAY;
#endif
            }

            /**
             * @brief 是否为数组类型
             *          
             * @param
             * @return bool
             * @retval
             * @see
             * @author liaoshangbin
             * @data 2010/08/10 11:37:20
             **/
            //testers
            virtual bool is_array() const {
                return true;
            }

            //converters
            using IVar::operator =;

            /**
             * @brief 获取下标idx处的IVar对象的引用对象
             *        所有数组类型is_array()返回true都支持
             *          
             * @param  [in] idx     : size_t
             * @return IVar&
             * @retval
             * @see
             * @author liaoshangbin
             * @data 2010/08/10 11:38:05
             **/
            //methods for array
            virtual IVar& get( size_t idx ) {
                if ( idx >= _array.size() ){
                    return bsl::var::Null::null;
                }
                return _array[idx];
            }

            /**
             * @brief 获取下标idx处的IVar对象的引用对象
             *        如果下标越界，返回一个默认值
             *          
             * @param  [in] idx             : size_t
             * @param  [in] default_value   : IVar&
             * @return IVar&
             * @retval
             * @see
             * @author liaoshangbin
             * @data 2010/08/10 11:39:57
             **/
            virtual IVar& get( size_t idx, IVar& default_value ) {
                if ( idx >= _array.size() ){
                    return default_value;
                }
                return _array[idx];
            }

            /**
             * @brief 获取下标idx处的IVar对象的引用对象
             *        如果下标越界，返回var::Null
             *          
             * @param  [in] idx             : size_t
             * @param  [in] default_value   : IVar&
             * @return IVar&
             * @retval
             * @see
             * @author liaoshangbin
             * @data 2010/08/11 15:54:07
             **/
            virtual const IVar& get( size_t idx ) const {
                if ( idx >= _array.size() ){
                    return bsl::var::Null::null;
                }
                return _array[idx];
            }

            /**
             * @brief 获取下标idx处的IVar对象的引用对象
             *        如果下标越界，返回一个默认值，const版本
             *          
             * @param  [in] idx             : size_t
             * @param  [in] default_value   : const IVar&
             * @return const IVar&
             * @retval
             * @see
             * @author liaoshangbin
             * @data 2010/08/12 15:54:57
             **/
            virtual const IVar& get( size_t idx, const IVar& default_value ) const {
                if ( idx >= _array.size() ){
                    return default_value;
                }
                return _array[idx];
            }

            /**
             * @brief 设置下标idx上的绑定
             *          
             * @param  [in] idx         : size_t
             * @param  [in] value       : IVar&
             * @return 
             * @retval
             * @see
             * @author liaoshangbin
             * @data 2010/08/12 15:56:03
             **/
            virtual void set( size_t idx, IVar& value ){
                if ( idx >= _array.size() ){
                    try{
                        _array.resize(idx + 1);
                    }catch(bsl::Exception& e){
                        throw;
                    }catch(std::exception& e){
                        throw StdException(e)<<BSL_EARG;
                    }catch(...){
                        throw UnknownException()<<BSL_EARG;
                    }
                }
                _array[idx] = value;
            }

            /**
             * @brief 删除下标idx上的绑定
             *        如果下标存在返回true，不存在返回false
             *          
             * @param  [in] idx         : size_t
             * @return bool
             * @retval
             * @see
             * @author liaoshangbin
             * @data 2010/08/12 15:58:18
             **/
            virtual bool del( size_t idx ){
                if ( idx >= _array.size() || _array[idx].is_null() ){
                    return false;
                }else{
                    _array[idx] = Null::null;
                    return true;
                }
            }

            /**
             * @brief 返回只读起始数组迭代器
             *        is_array()返回true的IVar实现类都必须支持该方法
             *          
             * @param
             * @return array_const_iterator
             * @retval
             * @see
             * @author liaoshangbin
             * @data 2010/08/12 15:59:58
             **/
            virtual array_const_iterator array_begin() const {
                return array_const_iterator( 
                        _s_create_const_iterator( &_array, 0 ), 
                        _s_clone_const_iterator, 
                        _s_destroy_const_iterator 
                        );
            }

            /**
             * @brief 返回起始数组迭代器
             *          
             * @param
             * @return array_iterator
             * @retval
             * @see
             * @author liaoshangbin
             * @data 2010/08/12 16:01:21
             **/
            virtual array_iterator array_begin() {
                return array_iterator( 
                        _s_create_iterator( &_array, 0 ), 
                        _s_clone_iterator, 
                        _s_destroy_iterator 
                        );
            }

            /**
             * @brief 返回只读末尾数组迭代器
             *          
             * @param
             * @return array_const_iterator
             * @retval
             * @see
             * @author liaoshangbin
             * @data 2010/08/12 16:01:53
             **/
            virtual array_const_iterator array_end() const {
                return array_const_iterator( 
                        _s_create_const_iterator( &_array, _array.size() ), 
                        _s_clone_const_iterator, 
                        _s_destroy_const_iterator 
                        );
            }

            /**
             * @brief 返回数组末尾迭代器
             *          
             * @param 
             * @return array_iterator
             * @retval
             * @see
             * @author liaoshangbin
             * @data 2010/08/12 16:02:29
             **/
            virtual array_iterator array_end() {
                return array_iterator( 
                        _s_create_iterator( &_array, _array.size() ), 
                        _s_clone_iterator, 
                        _s_destroy_iterator 
                        );
            }

            /**
             * @brief 返回/设置下标绑定
             *          
             * @param  [in] idx     : int
             * @return const IVar&
             * @retval
             * @see
             * @author liaoshangbin
             * @data 2010/08/12 16:03:09
             **/
            virtual const IVar& operator []( int idx ) const {
                return this->get( idx >= 0 ? size_t(idx) : size_t(_array.size() + idx) );
            }

            /**
             * @brief 返回/设置下标绑定
             *          
             * @param  [in] idx     : int
             * @return IVar&
             * @retval
             * @see
             * @author liaoshangbin
             * @data 2010/08/12 16:05:00
             **/
            virtual IVar& operator []( int idx ){
                if ( idx >= int(_array.size()) ){
                    try{
                        _array.resize( idx + 1 ); //自动扩展数组大小
                    }catch(bsl::Exception& e){
                        throw;
                    }catch(std::exception& e){
                        throw StdException(e)<<BSL_EARG;
                    }catch(...){
                        throw UnknownException()<<BSL_EARG;
                    }
                }                
                return  this->get( idx >= 0 ? size_t(idx) : size_t(_array.size() + idx) );
            }

            //methods for dict
#if __GNUC__ > 2
            using IVar::operator [];
            using IVar::get;
            using IVar::set;
            using IVar::del;
#else
            //avoid using bug of g++ 2.96
            
            /**
             * @brief 返回字段名绑定 
             *        所有is_dict()返回true的IVar实现类都必须支持该方法
             *          
             * @param
             * @return
             * @retval
             * @see
             * @author liaoshangbin
             * @data 2010/08/12 16:06:54
             **/
            virtual IVar& get( const field_type& name ) {
                throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] name["<<name<<"]";
            }

            /**
             * @brief 返回字段名绑定，若字段名不存在，返回默认值
             *
             **/
            virtual IVar& get( const field_type& name, IVar& default_value ) {
                throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] name["<<name<<"] default["<<default_value.dump(0)<<"]";
            }

            /**
             * @brief 返回字段名绑定
             *
             **/
            virtual const IVar& get( const field_type& name ) const {
                throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] name["<<name<<"]";
            }

            /**
             * @brief 返回字段名绑定，若字段名不存在，返回默认值
             *
             **/
            virtual const IVar& get( const field_type& name, const IVar& default_value ) const {
                throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] name["<<name<<"] default["<<default_value.dump(0)<<"]";
            }

            /**
             * @brief 设置字段名绑定
             *
             **/
            virtual void set( const field_type& name, IVar& value_ ){
                throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] name["<<name<<"] value["<<value_.dump(0)<<"]";
            }

            /**
             * @brief 删除字段名绑定
             *
             **/
            virtual bool del( const field_type& name ) {
                throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] name["<<name<<"]";
            }

            /**
             * @brief 获取/设置字段名绑定
             *
             **/
            virtual const IVar& operator []( const field_type& name ) const {
                throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] name["<<name<<"]";
            }

            /**
             * @brief 获取/设置字段名绑定
             *
             **/
            virtual IVar& operator []( const field_type& name ){
                throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] name["<<name<<"]";
            }

#endif

        private:
           
            class ArrayIteratorImpl;
            class ArrayConstIteratorImpl;
            /**
             * @brief 数组类型迭代器实现
             *          
             **/
            class ArrayIteratorImpl: public IArrayIteratorImpl{
                friend class ArrayConstIteratorImpl;
            public:

                /**
                 * @brief 数组类型迭代器的构造函数
                 *          
                 * @param  [in] p_array     : array_type*
                 * @param  [in] offset      : size_t
                 * @return
                 * @retval
                 * @see
                 * @author liaoshangbin
                 * @data 2010/08/12 16:37:40
                 **/
                ArrayIteratorImpl( array_type* p_array, size_t offset )
                    :_p_array(p_array), _offset(offset){}

                virtual ~ArrayIteratorImpl() {
                    //pass
                }
                /**
                 * @brief 得到偏移位置
                 *
                 **/
                virtual size_t  key() const {
                    return _offset;
                }
                /**
                 * @brief 得到偏移位置上的绑定值
                 *
                 **/
                virtual IVar& value() const {
                    return (*_p_array)[_offset];
                }
                /**
                 * @brief 偏移位置后移
                 *
                 **/
                virtual void iterate(){
                    ++ _offset;
                }
                /**
                 * @brief 将其它数组类型迭代器对其赋值
                 *          
                 * @param  [in] other   : const IArrayIteratorImpl&
                 * @return
                 * @retval
                 * @see
                 * @author liaoshangbin
                 * @data 2010/08/12 17:17:38
                 **/
                virtual void    assign( const IArrayIteratorImpl& other ) {
                    const ArrayIteratorImpl *p = dynamic_cast<const ArrayIteratorImpl *>(&other);
                    if ( !p ){
                        throw bsl::BadCastException()
                            <<BSL_EARG<<"from:"<<typeid(other).name()<<" to:ArrayIteratorImpl";
                    }
                    _p_array = p->_p_array;
                    _offset  = p->_offset;
                }

                /**
                 * @brief 判断是否等于其它数组类型迭代器
                 *          
                 * @param  [in] other   : const IArrayIteratorImpl&
                 * @return bool
                 * @retval
                 * @see
                 * @author liaoshangbin
                 * @data 2010/08/12 17:18:51
                 **/
                virtual bool    equal_to( const IArrayIteratorImpl& other ) const;
                /**
                 * @brief 判断是否等于其它只读数组类型迭代器
                 *          
                 * @param  [in] other   : const IArrayConstIteratorImpl&
                 * @return bool
                 * @retval
                 * @see
                 * @author liaoshangbin
                 * @data 2010/08/12 17:19:54
                 **/
                virtual bool    equal_to( const IArrayConstIteratorImpl& other ) const;

            private:
                /**
                 * @brief 数组指针
                 *
                 **/
                array_type *_p_array;
                /**
                 * @brief 数组指针的偏移位置
                 *
                 **/
                size_t      _offset;
            };

            /**
             * @brief 只读数组类型代器实现
             *          
             **/
            class ArrayConstIteratorImpl: public IArrayConstIteratorImpl{
                friend class ArrayIteratorImpl;
            public:
                /**
                 * @brief 只读数组类型迭代器的构造函数
                 *          
                 * @param  [in] p_array     : const array_type*
                 * @param  [in] offset      : size_t
                 * @return 
                 * @retval
                 * @see
                 * @author liaoshangbin
                 * @data 2010/08/12 16:32:33
                 **/
                ArrayConstIteratorImpl( const array_type* p_array, size_t offset )
                    :_p_array(p_array), _offset(offset){}

                virtual ~ArrayConstIteratorImpl() {
                    //pass
                }

                /**
                 * @brief 得到偏移位置
                 *
                 **/
                virtual size_t  key() const {
                    return _offset;
                }

                /**
                 * @brief 得到偏移位置上的绑定值
                 *
                 *
                 **/
                virtual const IVar& value() const {
                    return (*_p_array)[_offset];
                }
                /**
                 * @brief 偏移位置后移
                 *
                 **/
                virtual void iterate(){
                    ++ _offset;
                }
                /**
                 * @brief 将其它数组类型迭代器对其赋值
                 *          
                 * @param  [in] other   : const IArrayIteratorImpl&
                 * @return
                 * @retval
                 * @see
                 * @author liaoshangbin
                 * @data 2010/08/12 17:23:51
                 **/
                virtual void    assign( const IArrayIteratorImpl& other ) {
                    const ArrayIteratorImpl *p = dynamic_cast<const ArrayIteratorImpl *>(&other);
                    if ( !p ){
                        throw bsl::BadCastException()
                            <<BSL_EARG<<"from:"<<typeid(other).name()<<" to:ArrayIteratorImpl";
                    }
                    _p_array = p->_p_array;
                    _offset  = p->_offset;
                }
                /**
                 * @brief 将其它只读数组类型迭代器对其赋值
                 *          
                 * @param  [in] other   : const IArrayConstIteratorImpl&
                 * @return
                 * @retval
                 * @see
                 * @author liaoshangbin
                 * @data 2010/08/12 17:24:55
                 **/
                virtual void    assign( const IArrayConstIteratorImpl& other ) {
                    const ArrayConstIteratorImpl *p = 
                        dynamic_cast<const ArrayConstIteratorImpl *>(&other);
                    if ( !p ){
                        throw bsl::BadCastException()
                            <<BSL_EARG<<"from:"<<typeid(other).name()<<" to:ArrayIteratorImpl";
                    }
                    _p_array = p->_p_array;
                    _offset  = p->_offset;
                }

                /**
                 * @brief 判断是否等于其它只读数组类型迭代器
                 *          
                 * @param  [in] other   : const IArrayConstIteratorImpl& 
                 * @return bool
                 * @retval
                 * @see
                 * @author liaoshangbin
                 * @data 2010/08/12 17:26:22
                 **/
                virtual bool    equal_to( const IArrayConstIteratorImpl& other ) const;
                
                /**
                 * @brief 判断是否等于其它数组类型迭代器
                 *          
                 * @param  [in] other   : const IArrayIteratorImpl&
                 * @return bool
                 * @retval
                 * @see
                 * @author liaoshangbin
                 * @data 2010/08/12 17:27:09
                 **/
                virtual bool    equal_to( const IArrayIteratorImpl& other ) const;

            private:
                /**
                 * @brief 数组指针
                 *
                 **/
                const array_type *  _p_array;
                /**
                 * @brief 数组指针的偏移位置
                 *
                 **/
                size_t              _offset;
            };

            /**
             * @brief 创建一个数组的迭代器
             *          
             * @param  [in] p_array         : array_type*
             * @param  [in] offset          : size_t
             * @return IArrayIteratorImpl *
             * @retval
             * @see
             * @author liaoshangbin
             * @data 2010/08/12 16:14:45
             **/
            static IArrayIteratorImpl * _s_create_iterator( array_type* p_array, size_t offset){
                typedef typename allocator_type::
                    template rebind<ArrayIteratorImpl>::other impl_alloc_t;
                IArrayIteratorImpl *p = impl_alloc_t().allocate(1);   //throw
                new(p) ArrayIteratorImpl(p_array, offset );  //nothrow
                return p;
            }

            /**
             * @brief 创建一个只读数组的迭代器          
             * @param  [in] p_array         : const array_type*
             * @param  [in] offset          : size_t
             * @return IArrayConstIteratorImpl *
             * @retval
             * @see
             * @author liaoshangbin
             * @data 2010/08/12 16:16:18
             **/
            static IArrayConstIteratorImpl * _s_create_const_iterator( 
                    const array_type* p_array, 
                    size_t offset
                    ){
                typedef typename allocator_type::
                    template rebind<ArrayConstIteratorImpl>::other impl_alloc_t;
                IArrayConstIteratorImpl *p = impl_alloc_t().allocate(1);   //throw
                new(p) ArrayConstIteratorImpl(p_array, offset );  //nothrow
                return p;
            }

            /**
             * @brief 克隆数组的迭代器
             *          
             * @param  [in] p_other         : const IArrayIteratorImpl*
             * @return IArrayIteratorImpl*
             * @retval
             * @see
             * @author liaoshangbin
             * @data 2010/08/12 16:17:48
             **/
            static IArrayIteratorImpl * _s_clone_iterator( const IArrayIteratorImpl *p_other ){
                typedef typename allocator_type::
                    template rebind<ArrayIteratorImpl>::other impl_alloc_t;
                const ArrayIteratorImpl *psrc = dynamic_cast<const ArrayIteratorImpl*>(p_other);
                if ( !psrc ){
                    throw bsl::BadCastException()
                        <<BSL_EARG<<"from:"<<typeid(*p_other).name()<<" to:ArrayIteratorImpl";
                }
                IArrayIteratorImpl *p = impl_alloc_t().allocate(1);   //throw
                new(p) ArrayIteratorImpl(*psrc);  
                return p;
            }

            /**
             * @brief 克隆只读数组的迭代器   
             * @param
             * @return
             * @retval
             * @see
             * @author liaoshangbin
             * @data 2010/08/12 16:19:13
             **/
            static IArrayConstIteratorImpl * _s_clone_const_iterator( 
                    const IArrayConstIteratorImpl *p_other 
                    ){
                typedef typename allocator_type::
                    template rebind<ArrayConstIteratorImpl>::other impl_alloc_t;               
                const ArrayConstIteratorImpl *psrc = 
                    dynamic_cast<const ArrayConstIteratorImpl*>(p_other);
                if ( !psrc ){
                    throw bsl::BadCastException()
                        <<BSL_EARG<<"from:"<<typeid(*p_other).name()<<" to:ArrayIteratorImpl";
                }
                IArrayConstIteratorImpl *p = impl_alloc_t().allocate(1);   //throw
                new(p) ArrayConstIteratorImpl(*psrc);  
                return p;
            }

            /**
             * @brief 销毁数组的迭代器
             *          
             * @param   [in] p      : IArrayIteratorImpl
             * @return  void
             * @retval
             * @see
             * @author liaoshangbin
             * @data 2010/08/12 16:19:47
             **/
            static void _s_destroy_iterator( IArrayIteratorImpl * p){
                typedef typename allocator_type::
                    template rebind<ArrayIteratorImpl>::other impl_alloc_t;
                ArrayIteratorImpl *_p = dynamic_cast<ArrayIteratorImpl *>(p); 
                if ( _p ){
                    _p->~ArrayIteratorImpl();
                    impl_alloc_t().deallocate( _p, 1 );
                }
            }

            /**
             * @brief 销毁只读数组的迭代器 
             * @param   [in] p      : IArrayIteratorImpl
             * @return  void
             * @retval
             * @see
             * @author liaoshangbin
             * @data 2010/08/12 16:20:38
             **/
            static void _s_destroy_const_iterator( IArrayConstIteratorImpl * p){
                typedef typename allocator_type::
                    template rebind<ArrayConstIteratorImpl>::other impl_alloc_t;
                ArrayConstIteratorImpl *_p = dynamic_cast<ArrayConstIteratorImpl *>(p); 
                if ( _p ){
                    _p->~ArrayConstIteratorImpl();
                    impl_alloc_t().deallocate( _p, 1 );
                }
            }
            
            /**
             * @brief 内部封装的数组
             *
             **/
            array_type _array;
        };

    /**
     * @brief 判断数组两个迭代器是否相同
     *          
     * @param  [in] other       : const IArrayIteratroImpl&
     * @return bool
     * @retval
     * @see
     * @author liaoshangbin
     * @data 2010/08/12 16:26:48
     **/
    template<typename implement_t>
        inline bool BasicArray<implement_t>::
                    ArrayIteratorImpl::equal_to( const IArrayIteratorImpl& other ) const {
            const ArrayIteratorImpl *p = 
                dynamic_cast<const ArrayIteratorImpl *>(&other);
            return p != NULL  && _p_array == p->_p_array && _offset == p->_offset;
        }

    /**
     * @brief 判断数组的迭代器与只读数组的迭代器是否相同
     *          
     * @param  [in] other       : const IArrayConstIteratroImpl&
     * @return bool
     * @retval
     * @see
     * @author liaoshangbin
     * @data 2010/08/12 16:27:47
     **/
    template<typename implement_t>
        inline bool BasicArray<implement_t>::
                    ArrayIteratorImpl::equal_to( const IArrayConstIteratorImpl& other ) const {
            const ArrayConstIteratorImpl *p = 
                dynamic_cast<const ArrayConstIteratorImpl *>(&other);
            return p != NULL  && _p_array == p->_p_array && _offset == p->_offset;
        }

    /**
     * @brief 判断只读数组的迭代器与数组的迭代器是否相同
     *          
     * @param  [in] other       : const IArrayIteratroImpl&
     * @return bool
     * @retval
     * @see
     * @author liaoshangbin
     * @data 2010/08/12 16:28:37
     **/
    template<typename implement_t>
        inline bool BasicArray<implement_t>::
                    ArrayConstIteratorImpl::equal_to( const IArrayIteratorImpl& other ) const {
            const ArrayIteratorImpl *p = dynamic_cast<const ArrayIteratorImpl *>(&other);
            return p != NULL  && _p_array == p->_p_array && _offset == p->_offset;
        }
    
    /**
     * @brief 判断两个只读数组的迭代器是否相同
     *          
     * @param  [in] other       : const IArrayConstIteratroImpl&
     * @return bool
     * @retval
     * @see
     * @author liaoshangbin
     * @data 2010/08/12 16:29:20
     **/
    template<typename implement_t>
        inline bool BasicArray<implement_t>::
                    ArrayConstIteratorImpl::
                    equal_to( const IArrayConstIteratorImpl& other ) const {
            const ArrayConstIteratorImpl *p = 
                dynamic_cast<const ArrayConstIteratorImpl *>(&other);
            return p != NULL  && _p_array == p->_p_array && _offset == p->_offset;
        }

}}   //namespace bsl::var

#endif  //__BSL_VAR_ARRAY_H__

/* vim: set ts=4 sw=4 sts=4 tw=100 */
