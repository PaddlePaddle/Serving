/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: MagicArray.h,v 1.2 2010/04/28 12:45:33 scmpf Exp $ 
 * 
 **************************************************************************/



/**
 * @file MagicBasicArray.h
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2008/09/24 01:36:18
 * @version $Revision: 1.2 $ 
 * @brief 
 *  
 **/
#ifndef  __BSL_VAR_MAGIC_ARRAY_H__
#define  __BSL_VAR_MAGIC_ARRAY_H__

#include <deque>
#include "bsl/pool/bsl_pool.h"
#include "bsl/var/IVar.h"
#include "bsl/var/MagicRef.h"
#include "bsl/var/Null.h"
#include "bsl/var/utils.h"

namespace bsl{
namespace var{
    //typedefs
    template<typename allocator_t>
        class MagicBasicArray;

    typedef MagicBasicArray<bsl::pool_allocator<void> > MagicArray;

    template<typename allocator_t>
        class MagicBasicArray: public IVar{
        public:
            typedef IVar::string_type   string_type;
            typedef IVar::field_type    field_type;
            typedef allocator_t         allocator_type;
            typedef ArrayIterator       array_iterator;
            typedef ArrayConstIterator  array_const_iterator;
            typedef MagicRef            reference_type;

        public:
            MagicBasicArray(bsl::ResourcePool& rp)
                :_array(), _rp(rp) {}

            MagicBasicArray( const MagicBasicArray& other )
                :IVar(other), _array(other._array), _rp(other._rp){}

            MagicBasicArray& operator = ( const MagicBasicArray& other ){
                try{
                    _array = other._array;
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
                
            virtual ~MagicBasicArray() {}

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
             * @param [in] rp   : bsl::ResourcePool&
             * @return  MagicBasicArray& 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2010/03/17 17:58:07
            **/
            virtual MagicBasicArray& clone( bsl::ResourcePool& rp ) const {
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
             * @return MagicBasicArray& 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2010/03/17 17:58:07 / modified by zhujianwei at 2011/03/22
            **/
            virtual MagicBasicArray& clone( bsl::ResourcePool& rp, bool is_deep_copy ) const {
                MagicBasicArray& res = rp.clone(*this);
                if(is_deep_copy){
                    size_t n = _array.size();
                    for(size_t i = 0; i < n; ++i){
                        res.set(i, _array[i].clone(rp, is_deep_copy));
                    }
                }
                return res;
            }

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
                    res.appendf("[bsl::var::MagicBasicArray] this[%p] size[%zd]", this, _array.size() ); 
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

            virtual MagicBasicArray& operator = ( IVar& var ) {
                if ( typeid(var) == typeid(*this) ){
                    return operator =(dynamic_cast<MagicBasicArray&>(var));
                }else if ( var.is_array() ){
                    size_t var_size = var.size();
                    try{
                        _array.resize( var_size, reference_type(_rp) );
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
                return "bsl::var::MagicBasicArray";
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

            //testers
            virtual bool is_array() const {
                return true;
            }

            //converters
            using IVar::operator =;

            //methods for array
            virtual IVar& get( size_t idx ) {
                if ( idx >= _array.size() ){
                    return bsl::var::Null::null;
                }
                return _array[idx];
            }

            virtual IVar& get( size_t idx, IVar& default_value ) {
                if ( idx >= _array.size() ){
                    return default_value;
                }
                return _array[idx];
            }

            virtual const IVar& get( size_t idx ) const {
                if ( idx >= _array.size() ){
                    return bsl::var::Null::null;
                }
                return _array[idx];
            }

            virtual const IVar& get( size_t idx, const IVar& default_value ) const {
                if ( idx >= _array.size() ){
                    return default_value;
                }
                return _array[idx];
            }

            virtual void set( size_t idx, IVar& value ){
                if ( idx >= _array.size() ){
                    try{
                        _array.resize(idx + 1, reference_type(_rp) );
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

            virtual bool del( size_t idx ){
                if ( idx >= _array.size() || _array[idx].is_null() ){
                    return false;
                }else{
                    _array[idx] = Null::null;
                    return true;
                }
            }

            virtual array_const_iterator array_begin() const {
                return array_const_iterator( _s_create_const_iterator( &_array, 0 ), _s_clone_const_iterator, _s_destroy_const_iterator );
            }

            virtual array_iterator array_begin() {
                return array_iterator( _s_create_iterator( &_array, 0 ), _s_clone_iterator, _s_destroy_iterator );
            }

            virtual array_const_iterator array_end() const {
                return array_const_iterator( _s_create_const_iterator( &_array, _array.size() ), _s_clone_const_iterator, _s_destroy_const_iterator );
            }

            virtual array_iterator array_end() {
                return array_iterator( _s_create_iterator( &_array, _array.size() ), _s_clone_iterator, _s_destroy_iterator );
            }

            virtual const IVar& operator []( int idx ) const {
                return this->get( idx >= 0 ? size_t(idx) : size_t(_array.size() + idx) );
            }

            virtual IVar& operator []( int idx ){
                if ( idx >= int(_array.size()) ){
                    try{
                        _array.resize( idx + 1, reference_type(_rp) );
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
            virtual IVar& get( const field_type& name ) {
                throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] name["<<name<<"]";
            }

            virtual IVar& get( const field_type& name, IVar& default_value ) {
                throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] name["<<name<<"] default["<<default_value.dump(0)<<"]";
            }

            virtual const IVar& get( const field_type& name ) const {
                throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] name["<<name<<"]";
            }

            virtual const IVar& get( const field_type& name, const IVar& default_value ) const {
                throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] name["<<name<<"] default["<<default_value.dump(0)<<"]";
            }

            virtual void set( const field_type& name, IVar& value_ ){
                throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] name["<<name<<"] value["<<value_.dump(0)<<"]";
            }

            virtual bool del( const field_type& name ) {
                throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] name["<<name<<"]";
            }

            virtual const IVar& operator []( const field_type& name ) const {
                throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] name["<<name<<"]";
            }

            virtual IVar& operator []( const field_type& name ){
                throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] name["<<name<<"]";
            }

#endif

        private:
            typedef std::deque<reference_type> array_type;
            class ArrayIteratorImpl;
            class ArrayConstIteratorImpl;

            class ArrayIteratorImpl: public IArrayIteratorImpl{
                friend class ArrayConstIteratorImpl;
            public:

                ArrayIteratorImpl( array_type* p_array, size_t offset )
                    :_p_array(p_array), _offset(offset){}

                virtual ~ArrayIteratorImpl() {
                    //pass
                }

                virtual size_t  key() const {
                    return _offset;
                }

                virtual IVar& value() const {
                    return (*_p_array)[_offset];
                }

                virtual void iterate(){
                    ++ _offset;
                }

                virtual void    assign( const IArrayIteratorImpl& other ) {
                    const ArrayIteratorImpl *p = dynamic_cast<const ArrayIteratorImpl *>(&other);
                    if ( !p ){
                        throw bsl::BadCastException()<<BSL_EARG<<"from:"<<typeid(other).name()<<" to:ArrayIteratorImpl";
                    }
                    _p_array = p->_p_array;
                    _offset  = p->_offset;
                }

                virtual bool    equal_to( const IArrayIteratorImpl& other ) const;

                virtual bool    equal_to( const IArrayConstIteratorImpl& other ) const;

            private:
                array_type *_p_array;
                size_t      _offset;
            };


            class ArrayConstIteratorImpl: public IArrayConstIteratorImpl{
                friend class ArrayIteratorImpl;
            public:

                ArrayConstIteratorImpl( const array_type* p_array, size_t offset )
                    :_p_array(p_array), _offset(offset){}

                virtual ~ArrayConstIteratorImpl() {
                    //pass
                }

                virtual size_t  key() const {
                    return _offset;
                }

                virtual const IVar& value() const {
                    return (*_p_array)[_offset];
                }

                virtual void iterate(){
                    ++ _offset;
                }

                virtual void    assign( const IArrayIteratorImpl& other ) {
                    const ArrayIteratorImpl *p = dynamic_cast<const ArrayIteratorImpl *>(&other);
                    if ( !p ){
                        throw bsl::BadCastException()<<BSL_EARG<<"from:"<<typeid(other).name()<<" to:ArrayIteratorImpl";
                    }
                    _p_array = p->_p_array;
                    _offset  = p->_offset;
                }

                virtual void    assign( const IArrayConstIteratorImpl& other ) {
                    const ArrayConstIteratorImpl *p = dynamic_cast<const ArrayConstIteratorImpl *>(&other);
                    if ( !p ){
                        throw bsl::BadCastException()<<BSL_EARG<<"from:"<<typeid(other).name()<<" to:ArrayIteratorImpl";
                    }
                    _p_array = p->_p_array;
                    _offset  = p->_offset;
                }

                virtual bool    equal_to( const IArrayConstIteratorImpl& other ) const;

                virtual bool    equal_to( const IArrayIteratorImpl& other ) const;

            private:
                const array_type *  _p_array;
                size_t              _offset;
            };

            static IArrayIteratorImpl * _s_create_iterator( array_type* p_array, size_t offset){
                typedef typename allocator_t::template rebind<ArrayIteratorImpl>::other impl_alloc_t;
                IArrayIteratorImpl *p = impl_alloc_t().allocate(1);   //throw
                new(p) ArrayIteratorImpl(p_array, offset );  //nothrow
                return p;
            }

            static IArrayConstIteratorImpl * _s_create_const_iterator( const array_type* p_array, size_t offset){
                typedef typename allocator_t::template rebind<ArrayConstIteratorImpl>::other impl_alloc_t;
                IArrayConstIteratorImpl *p = impl_alloc_t().allocate(1);   //throw
                new(p) ArrayConstIteratorImpl(p_array, offset );  //nothrow
                return p;
            }

            static IArrayIteratorImpl * _s_clone_iterator( const IArrayIteratorImpl *p_other ){
                typedef typename allocator_t::template rebind<ArrayIteratorImpl>::other impl_alloc_t;
                const ArrayIteratorImpl *psrc = dynamic_cast<const ArrayIteratorImpl*>(p_other);
                if ( !psrc ){
                    throw bsl::BadCastException()<<BSL_EARG<<"from:"<<typeid(*p_other).name()<<" to:ArrayIteratorImpl";
                }
                IArrayIteratorImpl *p = impl_alloc_t().allocate(1);   //throw
                new(p) ArrayIteratorImpl(*psrc);  
                return p;
            }

            static IArrayConstIteratorImpl * _s_clone_const_iterator( const IArrayConstIteratorImpl *p_other ){
                typedef typename allocator_t::template rebind<ArrayConstIteratorImpl>::other impl_alloc_t;
                const ArrayConstIteratorImpl *psrc = dynamic_cast<const ArrayConstIteratorImpl*>(p_other);
                if ( !psrc ){
                    throw bsl::BadCastException()<<BSL_EARG<<"from:"<<typeid(*p_other).name()<<" to:ArrayIteratorImpl";
                }
                IArrayConstIteratorImpl *p = impl_alloc_t().allocate(1);   //throw
                new(p) ArrayConstIteratorImpl(*psrc);  
                return p;
            }

            static void _s_destroy_iterator( IArrayIteratorImpl * p){
                typedef typename allocator_t::template rebind<ArrayIteratorImpl>::other impl_alloc_t;
                ArrayIteratorImpl *_p = dynamic_cast<ArrayIteratorImpl *>(p); 
                if ( _p ){
                    _p->~ArrayIteratorImpl();
                    impl_alloc_t().deallocate( _p, 1 );
                }
            }

            static void _s_destroy_const_iterator( IArrayConstIteratorImpl * p){
                typedef typename allocator_t::template rebind<ArrayConstIteratorImpl>::other impl_alloc_t;
                ArrayConstIteratorImpl *_p = dynamic_cast<ArrayConstIteratorImpl *>(p); 
                if ( _p ){
                    _p->~ArrayConstIteratorImpl();
                    impl_alloc_t().deallocate( _p, 1 );
                }
            }

            array_type _array;
            bsl::ResourcePool& _rp;
        };

    template<typename allocator_t>
        inline bool MagicBasicArray<allocator_t>::ArrayIteratorImpl::equal_to( const IArrayIteratorImpl& other ) const {
            const ArrayIteratorImpl *p = dynamic_cast<const ArrayIteratorImpl *>(&other);
            return p != NULL  && _p_array == p->_p_array && _offset == p->_offset;
        }

    template<typename allocator_t>
        inline bool MagicBasicArray<allocator_t>::ArrayIteratorImpl::equal_to( const IArrayConstIteratorImpl& other ) const {
            const ArrayConstIteratorImpl *p = dynamic_cast<const ArrayConstIteratorImpl *>(&other);
            return p != NULL  && _p_array == p->_p_array && _offset == p->_offset;
        }

    template<typename allocator_t>
        inline bool MagicBasicArray<allocator_t>::ArrayConstIteratorImpl::equal_to( const IArrayIteratorImpl& other ) const {
            const ArrayIteratorImpl *p = dynamic_cast<const ArrayIteratorImpl *>(&other);
            return p != NULL  && _p_array == p->_p_array && _offset == p->_offset;
        }

    template<typename allocator_t>
        inline bool MagicBasicArray<allocator_t>::ArrayConstIteratorImpl::equal_to( const IArrayConstIteratorImpl& other ) const {
            const ArrayConstIteratorImpl *p = dynamic_cast<const ArrayConstIteratorImpl *>(&other);
            return p != NULL  && _p_array == p->_p_array && _offset == p->_offset;
        }

}}   //namespace bsl::var

#endif  //__BSL_VAR_MAGIC_ARRAY_H__

/* vim: set ts=4 sw=4 sts=4 tw=100 */
