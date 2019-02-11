/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: Ref.h,v 1.4 2010/04/28 12:45:33 scmpf Exp $ 
 * 
 **************************************************************************/



/**
 * @file Ref.h
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2008/09/24 01:30:35
 * @version $Revision: 1.4 $ 
 * @brief 
 *  
 **/
#ifndef  __BSL_VAR_REF_H__
#define  __BSL_VAR_REF_H__

#include "bsl/var/IRef.h"
#include "bsl/var/Null.h"

namespace bsl{
namespace var{
    class Ref: public IRef{
    public:
        typedef IRef::string_type   string_type;
        typedef IRef::field_type    field_type;

        //special methods
        Ref( )
            :IRef(), _p(&Null::null) {}

        Ref( const Ref& other )
            :IRef(other), _p(other._p) { }

        Ref( IVar& var )
            :IRef(), _p(&var){
            if ( var.is_ref() ){
                IRef * __p = dynamic_cast<IRef*>(&var);
                if ( NULL != __p ){
                    //make a shortcut
                    _p = &__p->ref();
                }
            }
        }

        /**
         * @brief Ref is a kind of "weak ref" which do nothing on destructor
         *
         * @see 
         * @author chenxm
         * @date 2008/09/25 12:08:53
        **/
        ~Ref(){
            //pass
        }

        Ref& operator = ( const Ref& other ){
            _p = other._p;
            return *this;
        }

        virtual IVar& ref() const {
            return *_p;
        }

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
        virtual void clear() {
            return _p->clear();
        }

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
            return _p->clone(rp);
        }

        /**
         * @brief ��¡����
         * 
         * ��is_deep_copy���������Ƿ����
         * ��Ϊfalse, ֻ��¡�����㣬����¡�ӽ��
         * ��Ϊtrue, ��¡�����㣬���ҵݹ��¡�ӽ��
         * ���������ͣ���¡ָ��Ľ��
         * 
         **@param [in] rp   : bsl::ResourcePool& 
         * @param [in] is_deep_copy : bool
         * @return  IVar& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 17:58:07 / modified by zhujianwei at 2011/03/22
        **/
        virtual IVar& clone( bsl::ResourcePool& rp, bool is_deep_copy ) const {
            return _p->clone(rp, is_deep_copy);
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
            res.append(_p->dump(verbose_level));
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
            return _p->to_string();
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
            string_type res("bsl::var::Ref(");
            res.append(_p->get_type());
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
            return IVar::IS_REF | _p->get_mask();
        }

        virtual Ref& operator = ( IVar& var ){
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
            return _p->is_null();
        }

        virtual bool is_ref() const {
            return true;
        }
        
        virtual bool is_bool() const {
            return _p->is_bool();
        }

        virtual bool is_number() const {
            return _p->is_number();
        }

        virtual bool is_int8() const {
            return _p->is_int8();
        }

        virtual bool is_uint8() const {
            return _p->is_uint8();
        }

        virtual bool is_int16() const {
            return _p->is_int16();
        }

        virtual bool is_uint16() const {
            return _p->is_uint16();
        }

        virtual bool is_int32() const {
            return _p->is_int32();
        }

        virtual bool is_uint32() const {
            return _p->is_uint32();
        }

        virtual bool is_int64() const {
            return _p->is_int64();
        }

        virtual bool is_uint64() const {
            return _p->is_uint64();
        }

        virtual bool is_float() const {
            return _p->is_float();
        }

        virtual bool is_double() const {
            return _p->is_double();
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
            return _p->is_string();
        }

        virtual bool is_array() const {
            return _p->is_array();
        }

        virtual bool is_dict() const {
            return _p->is_dict();
        }

        virtual bool is_callable() const {
            return _p->is_callable();
        }

        virtual bool is_raw() const {
            return _p->is_raw();
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
            return _p->to_bool();  
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
            return _p->to_int8();  
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
            return _p->to_uint8();  
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
            return _p->to_int16();  
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
            return _p->to_uint16();  
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
            return _p->to_int32();  
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
            return _p->to_uint32();  
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
            return _p->to_int64();  
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
            return _p->to_uint64();  
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
            return _p->to_float();
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
            return _p->to_double();
        }

        virtual raw_type to_raw() const {
            return _p->to_raw();
        }

        virtual Ref& operator = ( bool val ){
            _p->operator = ( val );
            return *this;
        }

        /**
         * @brief ʹ��signed char���͸�ֵ
         *
         * @param [in] val   : signed char
         * @return  Ref& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Ref& operator = ( signed char val ){
            _p->operator = ( val );
            return *this;
        }

        /**
         * @brief ʹ��unsigned char���͸�ֵ
         *
         * @param [in] val   : unsigned char
         * @return  Ref& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Ref& operator = ( unsigned char val ){
            _p->operator = ( val );
            return *this;
        }

        /**
         * @brief ʹ��signed short���͸�ֵ
         *
         * @param [in] val   : signed short
         * @return  Ref& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Ref& operator = ( signed short val ){
            _p->operator = ( val );
            return *this;
        }

        /**
         * @brief ʹ��unsigned short���͸�ֵ
         *
         * @param [in] val   : unsigned short
         * @return  Ref& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Ref& operator = ( unsigned short val ){
            _p->operator = ( val );
            return *this;
        }

        /**
         * @brief ʹ��signed int���͸�ֵ
         *
         * @param [in] val   : signed int
         * @return  Ref& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Ref& operator = ( signed int val ){
            _p->operator = ( val );
            return *this;
        }

        /**
         * @brief ʹ��unsigned int���͸�ֵ
         *
         * @param [in] val   : unsigned int
         * @return  Ref& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Ref& operator = ( unsigned int val ){
            _p->operator = ( val );
            return *this;
        }

        /**
         * @brief ʹ��signed long long���͸�ֵ
         *
         * @param [in] val   : signed long long
         * @return  Ref& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Ref& operator = ( signed long long val ){
            _p->operator = ( val );
            return *this;
        }

        /**
         * @brief ʹ��unsigned long long���͸�ֵ
         *
         * @param [in] val   : unsigned long long
         * @return  Ref& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Ref& operator = ( unsigned long long val ){
            _p->operator = ( val );
            return *this;
        }

        /**
         * @brief ʹ��float���͸�ֵ
         *
         * @param [in] val   : float
         * @return  Ref& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Ref& operator = ( float val ){
            _p->operator = ( val );
            return *this;
        }

        /**
         * @brief ʹ��double���͸�ֵ
         *
         * @param [in] val   : double
         * @return  Ref& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Ref& operator = ( double val ){
            _p->operator = ( val );
            return *this;
        }

        /**
         * @brief ʹ��const char *���͸�ֵ
         *
         * @param [in] val   : const char *
         * @return  Ref& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Ref& operator = ( const char *val ){
            _p->operator = ( val );
            return *this;
        }

        /**
         * @brief ʹ���ַ������͸�ֵ
         *
         * @param [in] val   : const string_type&
         * @return  Ref& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Ref& operator = ( const string_type& val ){
            _p->operator = ( val );
            return *this;
        }

        virtual Ref& operator = ( const raw_type& val ){
            _p->operator = ( val );
            return *this;
        }
        
        //methods for string
        virtual const char *c_str() const {
            return _p->c_str();
        }

        virtual size_t c_str_len() const {
            return _p->c_str_len();
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
            return _p->size();
        }

        //methods for array
        virtual IVar& get( size_t index ) {
            return _p->get(index);
        }

        virtual IVar& get( size_t index, IVar& default_value ) {
            return _p->get(index, default_value);
        }

        virtual const IVar& get( size_t index ) const {
            return _p->get(index);
        }

        virtual const IVar& get( size_t index, const IVar& default_value ) const {
            return _p->get(index,default_value);
        }

        virtual void set( size_t index, IVar& value ){
            return _p->set( index, value );
        }

        virtual bool del( size_t index ){
            return _p->del(index);
        }

        virtual array_const_iterator array_begin() const {
            return const_cast<const IVar *>(_p)->array_begin();
        }

        virtual array_iterator array_begin() {
            return _p->array_begin();
        }

        virtual array_const_iterator array_end() const {
            return const_cast<const IVar *>(_p)->array_end();
        }

        virtual array_iterator array_end() {
            return _p->array_end();
        }

        virtual const IVar& operator []( int index ) const {
            return _p->operator [](index);
        }

        virtual IVar& operator []( int index ){
            return _p->operator [](index); 
        }

        //methods for dict
        virtual IVar& get( const field_type& name ) {
            return _p->get(name);
        }

        virtual IVar& get( const field_type& name, IVar& default_value ) {
            return _p->get(name, default_value);
        }

        virtual const IVar& get( const field_type& name ) const {
            return _p->get(name);
        }

        virtual const IVar& get( const field_type& name, const IVar& default_value ) const {
            return _p->get(name, default_value);
        }

        virtual void set( const field_type& name, IVar& value ){
            return _p->set(name, value);
        }

        virtual bool del( const field_type& name ){
            return _p->del(name);
        }

        virtual const IVar& operator []( const field_type& name ) const {
            return _p->operator [](name);
        }

        virtual IVar& operator []( const field_type& name ){
            return _p->operator []( name );
        }

        virtual dict_const_iterator dict_begin() const {
            return const_cast<const IVar *>(_p)->dict_begin();
        }

        virtual dict_iterator dict_begin() {
            return _p->dict_begin();
        }

        virtual dict_const_iterator dict_end() const {
            return const_cast<const IVar *>(_p)->dict_end();
        }

        virtual dict_iterator dict_end() {
            return _p->dict_end();
        }

        virtual IVar& operator()(IVar& param, bsl::ResourcePool& rp ){
            return (*_p)(param, rp);
        }

        virtual IVar& operator()(IVar& self, IVar& param, bsl::ResourcePool& rp ){
            return (*_p)(self, param, rp);
        }
    private:
        IVar * _p;
    };

}}   //namespace bsl::var

#endif  //__BSL_VAR_REF_H__

/* vim: set ts=4 sw=4 sts=4 tw=100 */
