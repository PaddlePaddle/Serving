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
    * @brief ʹg++2.96ҲͳһΪ__gnu_cxx�����ռ�
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
    * @brief ��װ��std::map����ʹ��bsl::pool_allocator���ֵ�
    *  
    */
    typedef BasicDict<__StdMapAdapter<bsl::pool_allocator> >    StdMapDict;

    /**
    * @brief ��װ��__gnu_cxx::hash_map����ʹ��bsl::pool_allocator���ֵ�
    *  
    *  
    */
    typedef BasicDict<__GnuHashMapAdapter<bsl::pool_allocator> >GnuHashDict;

    /**
    * @brief ���Ƽ�ʹ�õ��ֵ�
    *  
    *  Ŀǰ���Ƽ�ʹ�õ��Ƿ�װ��std::map����ʹ��bsl::pool_allocator��StdMapDict
    */
    typedef StdMapDict                                          Dict;


    /**
     * @brief �ֵ�����
     *          
     **/

    template<typename implement_t>
        class BasicDict: public IVar{
        public:
            /**
             * @brief �ַ�������
             */
            typedef IVar::string_type                       string_type;
            /**
             * @brief �ֶ�����
             */
            typedef IVar::field_type                        field_type;
            /**
             * @brief allocator����
             */
            typedef typename implement_t::allocator_type    allocator_type;
            /**
             * @brief �ֵ������
             */
            typedef DictIterator                            dict_iterator;
            /**
             * @brief ֻ���ֵ������
             */
            typedef DictConstIterator                       dict_const_iterator;
            /**
             * @brief ���͵����� 
             */
            typedef Ref                                     reference_type;

        public:
            BasicDict()
                :_dict(), _alloc() {}

            /**
             * @brief ���캯��
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
             * @brief ���캯��
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
             * @brief ���ƹ��캯��
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
             * @brief ���Ƹ�ֵ�����
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
             * @brief ������Ԫ�ظ�����������ֵ䣩����֧��
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
             * @brief ������
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
             * @brief ��¡����
             * 
             * �ú���ֻ��¡�����㣬����¡�ӽ�㣬���������ͣ���¡ָ��Ľ��
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
             * @brief ��¡����
             * 
             * ��is_deep_copy���������Ƿ����
             * ��Ϊfalse, ֻ��¡�����㣬����¡�ӽ��
             * ��Ϊtrue, ��¡�����㣬���ҵݹ��¡�ӽ��
             * ���������ͣ���¡ָ��Ľ��
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
             * @brief �ݹ��ӡ�����¼������Ϣ
             *
             * @param [in] verbose_level   : size_t
             * @return  string_type 
             * @retval   
             * @see 
             * @author chenxm
             * @date 2009/05/14 17:34:58
            **/
            /**
             * @brief ���������Ϣ�������ڵ��ԣ���ͨ��verbose_level���Ƶݹ����
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
             * @brief ת��Ϊ�ַ���
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
             * @brief ��IVar����ת��ΪDict����
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
             * @brief ��ȡ�����ַ���������var���Ͷ�֧�֣������ڵ���ʹ�ã������������жϣ�
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
             * @brief ��ȡ�������루����var���Ͷ�֧�֣�
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
             * @brief �Ƿ�Ϊ�ֵ�����
             *
             **/
            //testers
            virtual bool is_dict() const {
                return true;
            }

            //converters
            using IVar::operator =;

            /**
             * @brief �����ֶ����� 
             *        �ֵ�����֧�֣�is_dict()����true
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
             * @brief �����ֶ����󶨣����ֶ��������ڣ�����Ĭ��ֵ
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
             * @brief �����ֶ�����
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
             * @brief �����ֶ����󶨣����ֶ��������ڣ�����Ĭ��ֵ
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
             * @brief �����ֶ�����
             *
             **/
            virtual void set( const field_type& name, IVar& value ){
                _dict[name] = value;
            }

            /**
             * @brief ɾ���ֶ�����
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
             * @brief ����ֻ����ʼ�ֵ������
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
             * @brief ������ʼ�ֵ������
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
             * @brief ����ֻ��ĩβ�ֵ������
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
             * @brief ����ĩβ�ֵ������
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
             * @brief ����/�����±��
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
             * @brief ����/�����±��
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
             * @brief ��ȡ�±�idx����IVar��������ö�������ӿ�
             *
             */
            virtual IVar& get( size_t idx ) {
                throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] index["<<idx<<"]";
            }
            /**
             * @brief ��ȡ�±�idx����IVar��������ö�������ӿ�
             *
             */
            virtual IVar& get( size_t idx, IVar& default_value ) {
                throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] index["<<idx<<"] default["<<default_value.dump(0)<<"]";
            }
            /**
             * @brief ��ȡ�±�idx����IVar��������ö�������ӿ�
             *
             */
            virtual const IVar& get( size_t idx ) const {
                throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] index["<<idx<<"]";
            }
            /**
             * @brief ��ȡ�±�idx����IVar��������ö�������ӿ�
             *
             */
            virtual const IVar& get( size_t idx, const IVar& default_value ) const {
                throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] index["<<idx<<"] default["<<default_value.dump(0)<<"]";
            }
            /**
             * @brief �����±�idx�ϵİ�
             *
             */
            virtual void set( size_t idx, IVar& value_ ){
                throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] index["<<idx<<"] value["<<value_.dump(0)<<"]";
            }
            /**
             * @brief ɾ���±�idx�ϵİ�
             *
             */
            virtual bool del( size_t idx ){
                throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] index["<<idx<<"]";
            }
            /**
             * @brief ����/�����±��
             *
             */
            virtual const IVar& operator []( int idx ) const {
                throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] index["<<idx<<"]";
            }
            /**
             * @brief ����/�����±��
             *
             */
            virtual IVar& operator []( int idx ){
                throw bsl::InvalidOperationException()<<BSL_EARG<<"type["<<typeid(*this).name()<<"] this["<<dump(0)<<"] index["<<idx<<"]";
            }

#endif

        private:
            /**
             * @brief �ֵ����͵ĵ�����
             *
             */
            typedef typename implement_t::iterator          iter_impl_t;
            /**
             * @brief ֻ���ֵ����͵ĵ�����
             *
             */
            typedef typename implement_t::const_iterator    const_iter_impl_t;
            class DictIteratorImpl;
            class DictConstIteratorImpl;
            
            /**
             * @brief �ֵ��������ʵ��
             *
             */
            class DictIteratorImpl: public IDictIteratorImpl{
                friend class DictConstIteratorImpl;
            public:

                /**
                 * @brief ���ƹ��캯��
                 */
                DictIteratorImpl( const iter_impl_t& iter_ )
                    :_iter(iter_){}

                virtual ~DictIteratorImpl() {
                    //pass
                }

                /**
                 * @brief �õ�ƫ��λ��
                 */
                virtual const string_type& key() const {
                    return _iter->first;
                }
                /**
                 * @brief �õ�ƫ��λ���ϵİ�ֵ
                 */
                virtual IVar& value() const {
                    return _iter->second;
                }
                /**
                 * @brief ƫ��λ�ú���
                 */
                virtual void iterate(){
                    ++ _iter;
                }
                /**
                 * @brief �������ֵ����͵��������丳ֵ
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
                 * @brief �ж��Ƿ��������ֻ���ֵ����͵�����
                 */
                virtual bool equal_to( const IDictIteratorImpl& other ) const;
                /**
                 * @brief �ж��Ƿ���������ֵ����͵�����
                 */
                virtual bool equal_to( const IDictConstIteratorImpl& other ) const;

            private:
                /**
                 * @brief �ֵ������
                 */
                iter_impl_t _iter;
            };
            /**
             * @brief ֻ���ֵ��������ʵ��
             */
            class DictConstIteratorImpl: public IDictConstIteratorImpl{
                friend class DictIteratorImpl;
            public:

                /**
                 * @brief ���ƹ��캯��
                 */
                DictConstIteratorImpl( const const_iter_impl_t& iter_ )
                    :_iter(iter_){}

                virtual ~DictConstIteratorImpl() {
                    //pass
                }
                /**
                 * @brief �õ�ƫ��λ��
                 */
                virtual const string_type& key() const {
                    return _iter->first;
                }
                /**
                 * @brief �õ�ƫ��λ���ϵİ�ֵ
                 */
                virtual const IVar& value() const {
                    return _iter->second;
                }
                /**
                 * @brief ƫ��λ�ú���
                 */
                virtual void iterate(){
                    ++ _iter;
                }
                /**
                 * @brief �������ֵ����͵��������丳ֵ
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
                 * @brief ������ֻ���ֵ����͵��������丳ֵ
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
                 * @brief �ж��Ƿ��������ֻ���ֵ����͵�����
                 */
                virtual bool equal_to( const IDictConstIteratorImpl& other ) const;
                /**
                 * @brief �ж��Ƿ���������ֵ����͵�����
                 */
                virtual bool equal_to( const IDictIteratorImpl& other ) const;

            private:
                /**
                 * @brief ֻ���ֵ������
                 */
                const_iter_impl_t _iter;
            };

            /**
             * @brief ����һ���ֵ�ĵ�����
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
             * @brief ����һ��ֻ���ֵ�ĵ�����
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
             * @brief ��¡�ֵ�ĵ�����
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
             * @brief ��¡ֻ���ֵ�ĵ�����
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
             * @brief �����ֵ�ĵ�����
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
             * @brief ����ֻ���ֵ�ĵ�����
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
             * @brief �ڲ��ֵ�
             */
            implement_t     _dict;
            /**
             * @brief �ڲ���allocator
             */
            allocator_type  _alloc;   //TODO��Ϊ�˼�ֱ�ۣ�û����EBO�Ż����Ժ���Լ���
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
    * @brief ����һ��std::map�Ľӿڣ��ṩһ������allocator�Ĺ��캯��
    *  
    *  ���͵�adapterģʽ
    *  ��adapter����ʹ������ʱ����û����ô����:-P
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
             * @brief ������std::map�Ľӿ�
             *
             */
            typedef std::map<
                IVar::string_type, 
                bsl::var::Ref, 
                std::less<IVar::string_type>, 
                Alloc<std::pair<IVar::string_type, bsl::var::Ref> > 
                    > base_type;
            /**
             * @brief allocator������
             *
             */
            typedef typename base_type::allocator_type  allocator_type;
            /**
             * @brief ������
             *
             */
            typedef typename base_type::iterator        iterator;
            /**
             * @brief ֻ��������
             *
             */
            typedef typename base_type::const_iterator  const_iterator;

            /**
             * @brief Ĭ�Ϲ��캯��
             *
             * @see 
             * @author chenxm
             * @date 2009/04/08 11:22:53
            **/
            __StdMapAdapter()
                :base_type(){}

            /**
             * @brief ʹ��allocator��ʼ���Ĺ��캯��
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
             * @brief ʹ��init_capacity��allocator��ʼ���Ĺ��캯��
             *
             * ����std::map��init_capacityû�����壬ֱ�Ӻ���
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
    * @brief ����һ��__gnu_cxx::hash_map�Ľӿڣ��ṩһ������allocator�Ĺ��캯��
    *  
    *  ���͵�adapterģʽ
    *  ��adapter����ʹ������ʱ����û����ô����:-P
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
             * @brief ������__gnu_cxx::hash_map�ӿ�
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
             * @brief allocator������
             *
             */
            typedef typename base_type::allocator_type  allocator_type;
            /**
             * @brief ������
             *
             */
            typedef typename base_type::iterator        iterator;
            /**
             * @brief ֻ��������
             *
             */
            typedef typename base_type::const_iterator  const_iterator;

            /**
             * @brief Ĭ�Ϲ��캯��
             *
             * @see 
             * @author chenxm
             * @date 2009/04/08 11:22:15
            **/
            __GnuHashMapAdapter()
                :base_type(){}

            /**
             * @brief ʹ��allocator��ʼ���Ĺ��캯��
             *
             * 100��__gnu_cxx::hash_mapʹ�õ�Ĭ������ֵ
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
             * @brief ʹ��init_capacity��allocator��ʼ���Ĺ��캯��
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
