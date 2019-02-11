/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: Double.h,v 1.4 2010/04/28 12:45:33 scmpf Exp $ 
 * 
 **************************************************************************/



/**
 * @file Double.h
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2008/09/24 01:32:42
 * @version $Revision: 1.4 $ 
 * @brief 
 *  
 **/
#ifndef  __BSL_VAR_DOUBLE_H__
#define  __BSL_VAR_DOUBLE_H__

#include "bsl/var/IVar.h"
#include "bsl/var/Ref.h"
#include "bsl/check_cast.h"

namespace bsl{
namespace var{
    class Double: public IVar{
    public:
        typedef IVar::string_type   string_type;
        typedef IVar::field_type    field_type;

    public:
        //special methods
        Double( double __value = 0 ) :_value(__value){}

        Double( const Double& other )
            :IVar(other), _value( other._value ) {}

        Double& operator = ( const Double& other ){
            _value = other._value;
            return *this;
        }

        //methods for all
        virtual Double& operator = ( IVar& var ) {
            try{
                _value = var.to_double();    //throw
            }catch(bsl::Exception& e){
                e<<"{bsl::var::Int32::operator =("<<var.dump()<<")}";
                throw;
            }
            return *this;
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
            _value = 0;
        }
 
        /**
         * @brief ��¡����
         * 
         * �ú���ֻ��¡�����㣬����¡�ӽ�㣬���������ͣ���¡ָ��Ľ��
         * 
         **@param [in] rp   : bsl::ResourcePool&
         * @return  Double&
         * @retval
         * @see 
         * @author chenxm
         * @date 2010/03/17 17:58:07
        **/
        virtual Double& clone( bsl::ResourcePool& rp) const {
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
         * @return  Double& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 17:58:07 / modified by zhujianwei at 2011/03/22
        **/
        virtual Double& clone( bsl::ResourcePool& rp, bool /*is_deep_copy*/) const {
            return rp.clone(*this);
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
        virtual string_type dump(size_t /*verbose_level*/ = 0) const {
            return  string_type("[bsl::var::Double]").appendf("%lg", _value);
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
            return  string_type().appendf("%lg", _value);
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
            return "bsl::var::Double";
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
            return IVar::IS_NUMBER | IVar::IS_FLOATING | IVar::IS_EIGHT_BYTE;
        }

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
            return _value;
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
            return check_cast<signed char>(_value);
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
            return check_cast<unsigned char>(_value);
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
            return check_cast<signed short>(_value);
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
            return check_cast<unsigned short>(_value);
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
            return check_cast<signed int>(_value);
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
            return check_cast<unsigned int>(_value);
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
            return check_cast<signed long long>(_value);
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
            return check_cast<unsigned long long>(_value);
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
            return float(_value);
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
            return _value;
        }

        virtual Double& operator = ( bool b ){
            _value = b;
            return *this;
        }

        virtual Double& operator = ( int i ){
            _value = check_cast<double>(i);
            return *this;
        }

        virtual Double& operator = ( long long ll ){
            _value = check_cast<double>(ll);
            return *this;
        }

        /**
         * @brief ʹ��float���͸�ֵ
         *
         * @param [in] val   : float
         * @return  Double& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Double& operator = ( float f ){
            _value = f;
            return *this;
        }

        /**
         * @brief ʹ��double���͸�ֵ
         *
         * @param [in] val   : double
         * @return  Double& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
       virtual Double& operator = ( double d ){
            _value = d;
            return *this;
        }

        /**
         * @brief ʹ��const char *���͸�ֵ
         *
         * @param [in] val   : const char *
         * @return  Double& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Double& operator = ( const char * cstr ){
            _value = check_cast<double>(cstr);
            return *this;
        }

        /**
         * @brief ʹ���ַ������͸�ֵ
         *
         * @param [in] val   : const string_type&
         * @return  Double& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Double& operator = ( const string_type& str ){
            _value = check_cast<double>(str.c_str());
            return *this;
        }

        //use default version for bool��raw
        using IVar::operator =;

        //testers
        virtual bool is_number() const {
            return true;
        }

        virtual bool is_double() const {
            return true;
        }

    private:
        double _value;
    };

}}   //namespace bsl::var

#endif  //__BSL_VAR_DOUBLE_H__

/* vim: set ts=4 sw=4 sts=4 tw=100 */
