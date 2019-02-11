/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: ShallowRaw.h,v 1.3 2010/04/28 12:45:33 scmpf Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file ShallowRaw.h
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2008/09/24 20:53:41
 * @version $Revision: 1.3 $ 
 * @brief 
 *  
 **/
#ifndef  __BSL_VAR_SHALLOW_RAW_H__
#define  __BSL_VAR_SHALLOW_RAW_H__
#include "bsl/var/IVar.h"

namespace bsl{
namespace var{
    /**
    * @brief ShallowRaw是一种raw类型，但它只维护指针和长度，不维护数据本身，固名
    *  
    *  
    */
    class ShallowRaw: public IVar{
    public:
        typedef IVar::string_type           string_type;
        typedef IVar::field_type            field_type;
        typedef IVar::raw_type              raw_type;

        //special methods
        ShallowRaw()
            :_value(){}

        ShallowRaw( const ShallowRaw& other )
            :IVar(other), _value(other._value){ }

        explicit ShallowRaw( const raw_type& value_ )
            :IVar(), _value(value_){}

        ShallowRaw( const void *data_, size_t size_ )
            :IVar(), _value(data_, size_) {}

        virtual ~ShallowRaw(){ } 

        ShallowRaw& operator = ( const ShallowRaw& other ){
            if ( this != &other ){
                _value = other._value;
            }
            return *this;
        }

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
            _value.data = NULL;
            _value.length = 0;
        }

        /**
         * @brief 克隆函数
         *
         * 该函数只克隆本身结点，不克隆子结点，对引用类型，克隆指向的结点
         *
         * @param [in] rp   : bsl::ResourcePool&
         * @return  ShallowRaw& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 17:58:07
        **/
        virtual ShallowRaw& clone( bsl::ResourcePool& rp ) const {
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
         * @return  ShallowRaw& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 17:58:07 / modified by zhujianwei at 2011/03/22
        **/
        virtual ShallowRaw& clone( bsl::ResourcePool& rp, bool /*is_deep_copy*/ ) const {
            return rp.clone(*this);
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
            res.appendf("[bsl::var::ShallowRaw] data[%p] length[%zd]", _value.data, _value.length);
            if ( _value.data != NULL && verbose_level > 0 ){
                res.append(" value[");
                const char * bytes = static_cast<const char*>(_value.data);
                for( size_t i = 0; i < _value.length; ++ i){
                    res.appendf("\\x%02hhx", bytes[i]);
                }
                res.append("]");
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
         * @brief 获取类型字符串（所有var类型都支持，仅用于调试使用，勿用于类型判断）
         *
         * @return  string_type 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 19:44:20
        **/
        virtual string_type get_type() const {
            return "bsl::var::ShallowRaw";
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
            return IVar::IS_RAW;
        }

        virtual ShallowRaw& operator = ( IVar& other ){
            if ( this != &other ){
                _value = other.to_raw();
            }
            return *this;
        }

        virtual bool is_raw() const {
            return true;
        }

        //all other is_xxx() return false;

        //methods for raw
        virtual ShallowRaw& operator = ( const raw_type& val ){
            _value = val;
            return *this;
        }

        virtual raw_type to_raw() const {
            return _value;
        }

        using IVar::operator =;
    private:
        raw_type _value;
    };

}}   //namespace bsl::var
#endif  //__BSL_VAR_SHALLOW_RAW_H__

/* vim: set ts=4 sw=4 sts=4 tw=100 */
