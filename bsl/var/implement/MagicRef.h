/***************************************************************************
 * 
 * Copyright (c) 2009 Baidu.com, Inc. All Rights Reserved
 * $Id: MagicRef.h,v 1.2 2010/04/28 12:45:33 scmpf Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file MagicRef.h
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2009/12/14 18:04:30
 * @version $Revision: 1.2 $ 
 * @brief 
 *    MagicRef，在第一次赋值时才智能确定类型的Ref
 *    MagicRef一经确定类型，即不可再次改变。进行错误类型的操作会导致抛异常。但这个行为以后可能改变，请不要依赖（抛异常）这个特性
 **/
#ifndef  __BSL_VAR__AUTOREF_H_
#define  __BSL_VAR__AUTOREF_H_

#include <bsl/var/IRef.h>
#include <bsl/ResourcePool.h>

namespace bsl{ namespace var {

    /**
    * @brief 魔法引用
    *  
    *  根据第一次使用来变成相应的类型
    */
    class MagicRef: public bsl::var::IRef{
    public:
        MagicRef(ResourcePool& rp)
            :IRef(), _p(NULL), _rp(rp){}
    
        MagicRef( const MagicRef& other)
            :IRef(other), _p(other._p), _rp(other._rp){}

        MagicRef& operator = ( const MagicRef& other ){
            _p = other._p;
            return *this;
        }

        virtual ~MagicRef(){}
        
        /**
         * @brief 克隆函数
         *
         * 该函数只克隆本身结点，不克隆子结点，对引用类型，克隆指向的结点
         *
         * @param [in] rp   : bsl::ResourcePool&
         * @return  IVar& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 17:58:07
        **/
        virtual IVar& clone( bsl::ResourcePool& rp ) const {
            if ( !_p ){
                return rp.clone(*this);
            }else{
                return _p->clone(rp);
            }
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
         * @return  IVar& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 17:58:07 / modified by zhujianwei at 2011/03/22
        **/
        virtual IVar& clone( bsl::ResourcePool& rp, bool is_deep_copy ) const {
            if ( !_p ){
                return rp.clone(*this);
            }else{
                return _p->clone(rp, is_deep_copy);
            }
        }

        //methods for ref
        virtual IVar& ref() const ;

        //methods for all
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
            if ( _p ){
                _p->clear();
            }
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
            res.appendf("@%p: ", _p);
            if ( _p ){
                res.append(_p->dump(verbose_level));
            }else{
                res.append("<magic null>");
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
        virtual string_type to_string() const ;

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
            string_type res("bsl::var::MagicRef(");
            if ( _p ){
                res.append(_p->get_type());
            }
            res.append(")");
            return res;
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
            return IVar::mask_type( IVar::IS_REF | ( _p ? _p->get_mask() : 0 ) );
        }

        virtual MagicRef& operator = ( IVar& var ){
            if ( var.is_ref() ){
                IRef * __p = dynamic_cast<IRef*>(&var);
                if ( NULL != __p ){
                    //make a shortcut
                    _p = &__p->ref();
                }else{
                    //unknown ref type
                    _p = &var;
                }
            }else{
                //non-ref value;
                _p = &var;
            }
            return *this;
        }


        //methods for all, test methods
        virtual bool is_null() const {
            if ( _p ){
                return _p->is_null();
            }else{
                return true;
            }
        }

        virtual bool is_ref() const {
            return true;
        }
        
        virtual bool is_bool() const {
            if ( _p ){
                return _p->is_bool();
            }else{
                return false;
            }
        }

        virtual bool is_number() const {
            if ( _p ){
                return _p->is_number();
            }else{
                return false;
            }
        }

        virtual bool is_int8() const {
            if ( _p ){
                return _p->is_int8();
            }else{
                return false;
            }
        }

        virtual bool is_uint8() const {
            if ( _p ){
                return _p->is_uint8();
            }else{
                return false;
            }
        }

        virtual bool is_int16() const {
            if ( _p ){
                return _p->is_int16();
            }else{
                return false;
            }
        }

        virtual bool is_uint16() const {
            if ( _p ){
                return _p->is_uint16();
            }else{
                return false;
            }
        }

        virtual bool is_int32() const {
            if ( _p ){
                return _p->is_int32();
            }else{
                return false;
            }
        }

        virtual bool is_uint32() const {
            if ( _p ){
                return _p->is_uint32();
            }else{
                return false;
            }
        }

        virtual bool is_int64() const {
            if ( _p ){
                return _p->is_int64();
            }else{
                return false;
            }
        }

        virtual bool is_uint64() const {
            if ( _p ){
                return _p->is_uint64();
            }else{
                return false;
            }
        }

        virtual bool is_float() const {
            if ( _p ){
                return _p->is_float();
            }else{
                return false;
            }
        }

        virtual bool is_double() const {
            if ( _p ){
                return _p->is_double();
            }else{
                return false;
            }
        }

        /**
         * @brief 判断是否是字符串类型
         *
         * @return  bool 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 18:05:44
        **/
        virtual bool is_string() const {
            if ( _p ){
                return _p->is_string();
            }else{
                return false;
            }
        }

        virtual bool is_array() const {
            if ( _p ){
                return _p->is_array();
            }else{
                return false;
            }
        }

        virtual bool is_dict() const {
            if ( _p ){
                return _p->is_dict();
            }else{
                return false;
            };
        }

        virtual bool is_callable() const {
            if ( _p ){
                return _p->is_callable();
            }else{
                return false;
            }
        }

        virtual bool is_raw() const {
            if ( _p ){
                return _p->is_raw();
            }else{
                return false;
            }
        }

        //methods for value
        /**
         * @brief 转化为布尔类型
         *
         * @return  bool 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 18:09:14
        **/
        virtual bool to_bool() const {
            if ( _p ){
                return _p->to_bool();  
            }else{
                return IRef::to_bool();
            }
        }

        /**
         * @brief 转化为8位有符号整数类型（所有is_number()返回真的类型都支持）
         *
         * @return  signed char 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 18:09:14
        **/
        virtual signed char to_int8() const {
            if ( _p ){
                return _p->to_int8();  
            }else{
                return IRef::to_int8();
            }
        }

        /**
         * @brief 转化为8位无符号整数类型（所有is_number()返回真的类型都支持）
         *
         * @return  unsigned char 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 18:09:14
        **/
        virtual unsigned char to_uint8() const {
            if ( _p ){
                return _p->to_uint8();  
            }else{
                return IRef::to_uint8();
            }
        }

        /**
         * @brief 转化为16位有符号整数类型（所有is_number()返回真的类型都支持）
         *
         * @return  signed short 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 18:09:14
        **/
        virtual signed short to_int16() const {
            if ( _p ){
                return _p->to_int16();  
            }else{
                return IRef::to_int16();
            }
        }

        /**
         * @brief 转化为16位无符号整数类型（所有is_number()返回真的类型都支持）
         *
         * @return  unsigned short 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 18:09:14
        **/
        virtual unsigned short to_uint16() const {
            if ( _p ){
                return _p->to_uint16();  
            }else{
                return IRef::to_uint16();
            }
        }

        /**
         * @brief 转化为32位有符号整数类型（所有is_number()返回真的类型都支持）
         *
         * @return  signed int 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 18:09:14
        **/
        virtual signed int to_int32() const {
            if ( _p ){
                return _p->to_int32();  
            }else{
                return IRef::to_int32();
            }
        }

        /**
         * @brief 转化为32位无符号整数类型（所有is_number()返回真的类型都支持）
         *
         * @return  unsigned int 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 18:09:14
        **/
        virtual unsigned int to_uint32() const {
            if ( _p ){
                return _p->to_uint32();  
            }else{
                return IRef::to_uint32();
            }
        }

        /**
         * @brief 转化为64位有符号整数类型（所有is_number()返回真的类型都支持）
         *
         * @return  signed long long 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 18:09:14
        **/
        virtual signed long long to_int64() const {
            if ( _p ){
                return _p->to_int64();  
            }else{
                return IRef::to_int64();
            }
        }

        /**
         * @brief 转化为64位无符号整数类型（所有is_number()返回真的类型都支持）
         *
         * @return  unsigned long long 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 18:09:14
        **/
        virtual unsigned long long to_uint64() const {
            if ( _p ){
                return _p->to_uint64();  
            }else{
                return IRef::to_uint64();
            }
        }

        /**
         * @brief 转化为单精度浮点数类型（所有is_number()返回真的类型都支持）
         *
         * @return  float 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 18:09:14
        **/
        virtual float to_float() const {
            if ( _p ){
                return _p->to_float();  
            }else{
                return IRef::to_float();
            }
        }

        /**
         * @brief 转化为双精度浮点数类型（所有is_number()返回真的类型都支持）
         *
         * @return  double 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 18:09:14
        **/
        virtual double to_double() const {
            if ( _p ){
                return _p->to_double();  
            }else{
                return IRef::to_double();
            }
        }

        virtual raw_type to_raw() const {
            if ( _p ){
                return _p->to_raw();  
            }else{
                return IRef::to_raw();
            }
        }

        virtual MagicRef& operator = ( bool val );

        /**
         * @brief 使用signed char类型赋值
         *
         * @param [in] val   : signed char
         * @return  MagicRef& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual MagicRef& operator = ( signed char val );

        /**
         * @brief 使用unsigned char类型赋值
         *
         * @param [in] val   : unsigned char
         * @return  MagicRef& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual MagicRef& operator = ( unsigned char val );

        /**
         * @brief 使用signed short类型赋值
         *
         * @param [in] val   : signed short
         * @return  MagicRef& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual MagicRef& operator = ( signed short val );

        /**
         * @brief 使用unsigned short类型赋值
         *
         * @param [in] val   : unsigned short
         * @return  MagicRef& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual MagicRef& operator = ( unsigned short val );

        /**
         * @brief 使用signed int类型赋值
         *
         * @param [in] val   : signed int
         * @return  MagicRef& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual MagicRef& operator = ( signed int val );

        /**
         * @brief 使用unsigned int类型赋值
         *
         * @param [in] val   : unsigned int
         * @return  MagicRef& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual MagicRef& operator = ( unsigned int val );

        /**
         * @brief 使用signed long long类型赋值
         *
         * @param [in] val   : signed long long
         * @return  MagicRef& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual MagicRef& operator = ( signed long long val );

        /**
         * @brief 使用unsigned long long类型赋值
         *
         * @param [in] val   : unsigned long long
         * @return  MagicRef& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual MagicRef& operator = ( unsigned long long val );

        /**
         * @brief 使用float类型赋值
         *
         * @param [in] val   : float
         * @return  MagicRef& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual MagicRef& operator = ( float val );

        /**
         * @brief 使用double类型赋值
         *
         * @param [in] val   : double
         * @return  MagicRef& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual MagicRef& operator = ( double val );

        /**
         * @brief 使用const char *类型赋值
         *
         * @param [in] val   : const char *
         * @return  MagicRef& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual MagicRef& operator = ( const char *val );

        /**
         * @brief 使用字符串类型赋值
         *
         * @param [in] val   : const string_type&
         * @return  MagicRef& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual MagicRef& operator = ( const string_type& val );

        virtual MagicRef& operator = ( const raw_type& val );
        
        //methods for string
        virtual const char *c_str() const {
            if ( _p ){
                return _p->c_str();
            }else{
                return IRef::c_str();
            }
        }

        virtual size_t c_str_len() const {
            if ( _p ){
                return _p->c_str_len();
            }else{
                return IRef::c_str_len();
            }
        }

        //methods for array or dict
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
            if ( _p ){
                return _p->size();
            }else{
                return IRef::size();
            }
        }

        //methods for array
        virtual IVar& get( size_t index ) {
            if ( _p ){
                return _p->get(index);
            }else{
                return IRef::get(index);
            }
        }

        virtual IVar& get( size_t index, IVar& default_value ) {
            if ( _p ){
                return _p->get(index, default_value);
            }else{
                return IRef::get(index, default_value);
            }
            
        }

        virtual const IVar& get( size_t index ) const {
            if ( _p ){
                return _p->get(index);
            }else{
                return IRef::get(index);
            }
        }

        virtual const IVar& get( size_t index, const IVar& default_value ) const {
            if ( _p ){
                return _p->get(index,default_value);
            }else{
                return IRef::get(index, default_value);
            }
        }

        virtual void set( size_t index, IVar& value );

        virtual bool del( size_t index );

        virtual array_const_iterator array_begin() const {
            if ( _p ){
                return const_cast<const IVar *>(_p)->array_begin();
            }else{
                return IRef::array_begin();
            }
        }

        virtual array_iterator array_begin() {
            if ( _p ){
                return _p->array_begin();
            }else{
                return IRef::array_begin();
            }
        }

        virtual array_const_iterator array_end() const {
            if ( _p ){
                return const_cast<const IVar *>(_p)->array_end();
            }else{
                return IRef::array_end();
            }
        }

        virtual array_iterator array_end() {
            if ( _p ){
                return _p->array_end();
            }else{
                return IRef::array_end();
            }
        }

        virtual const IVar& operator []( int index ) const {
            if ( _p ){
                return _p->operator [](index);
            }else{
                return IRef::operator[](index);
            }
        }

        virtual IVar& operator []( int index );

        //methods for dict
        virtual IVar& get( const field_type& name ) {
            if ( _p ){
                return _p->get(name);
            }else{
                return IRef::get(name);
            }
        }

        virtual IVar& get( const field_type& name, IVar& default_value ) {
            if ( _p ){
                return _p->get(name, default_value);
            }else{
                return IRef::get(name, default_value);
            }
        }

        virtual const IVar& get( const field_type& name ) const {
            if ( _p ){
                return _p->get(name);
            }else{
                return IRef::get(name);
            }
        }

        virtual const IVar& get( const field_type& name, const IVar& default_value ) const {
            if ( _p ){
                return _p->get(name, default_value);
            }else{
                return IRef::get(name, default_value);
            }
        }

        virtual void set( const field_type& name, IVar& value );

        virtual bool del( const field_type& name );

        virtual const IVar& operator []( const field_type& name ) const {
            if ( _p ){
                return _p->operator [](name);
            }else{
                return IRef::operator [](name);
            }
        }

        virtual IVar& operator []( const field_type& name );

        virtual dict_const_iterator dict_begin() const {
            if ( _p ){
                return const_cast<const IVar *>(_p)->dict_begin();
            }else{
                return IRef::dict_begin();
            }
        }

        virtual dict_iterator dict_begin() {
            if ( _p ){
                return _p->dict_begin();
            }else{
                return IRef::dict_begin();
            }
        }

        virtual dict_const_iterator dict_end() const {
            if ( _p ){
                return const_cast<const IVar *>(_p)->dict_end();
            }else{
                return IRef::dict_end();
            }
        }

        virtual dict_iterator dict_end() {
            if ( _p ){
                return _p->dict_end();
            }else{
                return IRef::dict_end();
            }
        }

        virtual IVar& operator()(IVar& param, bsl::ResourcePool& rp ){
            if ( _p ){
                return (*_p)(param, rp);
            }else{
                return IRef::operator()(param, rp);
            }
        }

        virtual IVar& operator()(IVar& self, IVar& param, bsl::ResourcePool& rp ){
            if ( _p ){
                return (*_p)(self, param, rp);
            }else{
                return IRef::operator()(self, param, rp);
            }
        }
    private:
        bsl::var::IVar* _p;
        bsl::ResourcePool& _rp;
        
    };  //end of class 
}}//end of namespace
#endif  //__BSL_VAR__AUTOREF_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
