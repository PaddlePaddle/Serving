/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: Bool.h,v 1.3 2010/04/28 12:45:33 scmpf Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file Bool.h
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2008/09/24 20:53:41
 * @version $Revision: 1.3 $ 
 * @brief 
 *  
 **/
#ifndef  __BSL_VAR_BOOL_H__
#define  __BSL_VAR_BOOL_H__
#include "bsl/var/IVar.h"
#include "BigInt.h"

namespace bsl{
namespace var{
    class Bool: public IVar{
    public:
        typedef IVar::string_type           string_type;
        typedef IVar::field_type            field_type;

        //special methods
        Bool()
            :_value(false){}

        Bool( const Bool& other )
            :IVar(other), _value(other._value){ }

        Bool( bool value_ )
            :IVar(), _value(value_){}

        virtual ~Bool(){ } 

        Bool& operator = ( const Bool& other ){
            _value = other._value;
            return *this;
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
        virtual void clear(){
            _value = false;
        }

        /**
         * @brief ��¡����
         * 
         * �ú���ֻ��¡�����㣬����¡�ӽ�㣬���������ͣ���¡ָ��Ľ��
         * 
         **@param [in] rp   : bsl::ResourcePool&
         * @return  Bool&
         * @retval
         * @see 
         * @author chenxm
         * @date 2010/03/17 17:58:07
        **/
        virtual Bool& clone( bsl::ResourcePool& rp) const {
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
         * @return  Bool& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 17:58:07 / modified by zhujianwei at 2011/03/22
        **/
        virtual Bool& clone( bsl::ResourcePool& rp, bool /*is_deep_copy*/) const {
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
            if ( _value ){
                return "[bsl::var::Bool]true";
            }else{
                return "[bsl::var::Bool]false";
            }
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
            if ( _value ){
                return "true";
            }else{
                return "false";
            }
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
            return "bsl::var::Bool";
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
#ifdef PHP_COMLOG
            return IVar::_IS_BOOL;
#else
            return IVar::IS_BOOL;
#endif
        }

        virtual Bool& operator = ( IVar& other ){
            _value = other.to_bool();
            return *this;
        }

        virtual bool is_bool() const {
            return true;
        }

        //all other is_xxx() return false;

        //methods for bool
        virtual Bool& operator = ( bool val ){
            _value = val;
            return *this;
        }

        virtual Bool& operator = ( int val ){
            _value = val;
            return *this;
        }

        virtual Bool& operator = ( long long val ){
            _value = val;
            return *this;
        }

		template<size_t bits, bool sign>
        Bool& operator = ( BigInt<bits, sign> val){
			_value = val.to_bool();
            return *this;
        }

        /**
         * @brief ʹ��float���͸�ֵ
         *
         * @param [in] val   : float
         * @return  Bool& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Bool& operator = ( float f ){
            _value = f;
            return *this;
        }

        /**
         * @brief ʹ��double���͸�ֵ
         *
         * @param [in] val   : double
         * @return  Bool& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Bool& operator = ( double val ){
            _value = val;
            return *this;
        }

        /**
         * @brief ʹ��const char *���͸�ֵ
         *
         * @param [in] val   : const char *
         * @return  Bool& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Bool& operator = ( const char * val ){
            _value = (NULL != val && '\0' != val[0]);
            return *this;
        }

        /**
         * @brief ʹ���ַ������͸�ֵ
         *
         * @param [in] val   : const string_type&
         * @return  Bool& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:23:16
        **/
        virtual Bool& operator = ( const string_type& val ){
            _value = (val.c_str()[0] != '\0') ;
            return *this;
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
            return static_cast<signed char>(_value);
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
            return static_cast<unsigned char>(_value);
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
            return static_cast<signed short>(_value);
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
            return static_cast<unsigned short>(_value);
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
            return static_cast<signed int>(_value);
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
            return static_cast<unsigned int>(_value);
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
            return static_cast<signed long long>(_value);
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
            return static_cast<unsigned long long>(_value);
        }

		/**
		 * @brief conversion to bigint, not a virtual function derived
		 *
		 * @tparam bits
		 *
		 * @return 
		 *
		 * @version 1.1.24
		 * @author linjieqiong
		 * @date 2013/07/25 15:51:13
		 */
		template<size_t bits, bool sign>
        BigInt<bits, sign> to_bigint() {
			BigInt<bits, sign> tmp = _value;
			return tmp;
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
            return static_cast<float>(_value);
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
            return static_cast<double>(_value);
        }

        //using default version for raw
        using IVar::operator =;
    private:
        bool _value;
    };

}}   //namespace bsl::var
#endif  //__BSL_VAR_BOOL_H__

/* vim: set ts=4 sw=4 sts=4 tw=100 */
