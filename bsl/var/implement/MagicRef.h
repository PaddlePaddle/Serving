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
 *    MagicRef���ڵ�һ�θ�ֵʱ������ȷ�����͵�Ref
 *    MagicRefһ��ȷ�����ͣ��������ٴθı䡣���д������͵Ĳ����ᵼ�����쳣���������Ϊ�Ժ���ܸı䣬�벻Ҫ���������쳣���������
 **/
#ifndef  __BSL_VAR__AUTOREF_H_
#define  __BSL_VAR__AUTOREF_H_

#include <bsl/var/IRef.h>
#include <bsl/ResourcePool.h>

namespace bsl{ namespace var {

    /**
    * @brief ħ������
    *  
    *  ���ݵ�һ��ʹ���������Ӧ������
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
         * @brief ��¡����
         *
         * �ú���ֻ��¡�����㣬����¡�ӽ�㣬���������ͣ���¡ָ��Ľ��
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
         * @brief ��¡����
         * 
         * ��is_deep_copy���������Ƿ����
         * ��Ϊfalse, ֻ��¡�����㣬����¡�ӽ��
         * ��Ϊtrue, ��¡�����㣬���ҵݹ��¡�ӽ��
         * ���������ͣ���¡ָ��Ľ��
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
         * @brief ������
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
            res.appendf("@%p: ", _p);
            if ( _p ){
                res.append(_p->dump(verbose_level));
            }else{
                res.append("<magic null>");
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
        virtual string_type to_string() const ;

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
            string_type res("bsl::var::MagicRef(");
            if ( _p ){
                res.append(_p->get_type());
            }
            res.append(")");
            return res;
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
         * @brief �ж��Ƿ����ַ�������
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
         * @brief ת��Ϊ��������
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
         * @brief ת��Ϊ8λ�з����������ͣ�����is_number()����������Ͷ�֧�֣�
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
         * @brief ת��Ϊ8λ�޷����������ͣ�����is_number()����������Ͷ�֧�֣�
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
         * @brief ת��Ϊ16λ�з����������ͣ�����is_number()����������Ͷ�֧�֣�
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
         * @brief ת��Ϊ16λ�޷����������ͣ�����is_number()����������Ͷ�֧�֣�
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
         * @brief ת��Ϊ32λ�з����������ͣ�����is_number()����������Ͷ�֧�֣�
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
         * @brief ת��Ϊ32λ�޷����������ͣ�����is_number()����������Ͷ�֧�֣�
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
         * @brief ת��Ϊ64λ�з����������ͣ�����is_number()����������Ͷ�֧�֣�
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
         * @brief ת��Ϊ64λ�޷����������ͣ�����is_number()����������Ͷ�֧�֣�
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
         * @brief ת��Ϊ�����ȸ��������ͣ�����is_number()����������Ͷ�֧�֣�
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
         * @brief ת��Ϊ˫���ȸ��������ͣ�����is_number()����������Ͷ�֧�֣�
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
         * @brief ʹ��signed char���͸�ֵ
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
         * @brief ʹ��unsigned char���͸�ֵ
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
         * @brief ʹ��signed short���͸�ֵ
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
         * @brief ʹ��unsigned short���͸�ֵ
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
         * @brief ʹ��signed int���͸�ֵ
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
         * @brief ʹ��unsigned int���͸�ֵ
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
         * @brief ʹ��signed long long���͸�ֵ
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
         * @brief ʹ��unsigned long long���͸�ֵ
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
         * @brief ʹ��float���͸�ֵ
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
         * @brief ʹ��double���͸�ֵ
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
         * @brief ʹ��const char *���͸�ֵ
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
         * @brief ʹ���ַ������͸�ֵ
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
             * @brief ������Ԫ�ظ�����������ֵ䣩����֧��
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
