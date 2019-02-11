/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: Function.h,v 1.4 2010/04/28 12:45:33 scmpf Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file Function.h
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2008/09/24 20:53:41
 * @version $Revision: 1.4 $ 
 * @brief 
 *  
 **/
#ifndef  __BSL_VAR_FUNCTION_H__
#define  __BSL_VAR_FUNCTION_H__
#include "bsl/var/IVar.h"
#include "bsl/var/Ref.h"

namespace bsl{
namespace var{
    class Function: public IVar{
    public:
        typedef IVar::string_type          string_type;
        typedef IVar::field_type           field_type;
        typedef IVar& (* function_type )(IVar&, bsl::ResourcePool& );
        typedef IVar::array_iterator       array_iterator;
        typedef IVar::array_const_iterator array_const_iterator;

        //special methods
        Function( function_type func, const string_type& name )
            :IVar(), _func(func), _name(name){ } 

        Function( const Function& other )
            :IVar(other), _func(other._func), _name(other._name){ }

        virtual ~Function(){ } 

        Function& operator = ( const Function& other ){
            _func = other._func;
            _name = other._name;
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
            // pass
        }
        
        /**
         * @brief 克隆函数
         *
         * 该函数只克隆本身结点，不克隆子结点，对引用类型，克隆指向的结点
         *
         * @param [in] rp   : bsl::ResourcePool&
         * @return  Function& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 17:58:07
        **/
        virtual Function& clone( bsl::ResourcePool& rp ) const {
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
         * @return  Function& 
         * @retval   
         * @see 
         * @author chenxm
         * @date 2010/03/17 17:58:07 / modified by zhujianwei at 2011/03/22
        **/
        virtual Function& clone( bsl::ResourcePool& rp, bool /*is_deep_copy*/ ) const {
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
        virtual string_type dump(size_t /*verbose_level*/ = 0) const {
            return string_type("[bsl::var::Function]").append(_name);
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
            return _name;
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
            return "bsl::var::Function";
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
            return IVar::IS_CALLABLE;
        }

        virtual Function& operator = ( IVar& var ){
            Function * vfp = dynamic_cast<Function*>(&var);
            if ( vfp == NULL ){
                throw bsl::InvalidOperationException()<<BSL_EARG<<"cast from "<<var.dump();
            }else{
                _name = vfp->_name;
                _func = vfp->_func;
                return *this;
            }
            
        }

        virtual bool is_callable() const {
            return true;
        }

        //converters
        using IVar::operator =;

        virtual IVar& operator()(IVar& args, bsl::ResourcePool& _rp ){
            return (*_func)(args, _rp);
        }

        virtual IVar& operator()(IVar& /*self*/, IVar& args, bsl::ResourcePool& _rp ){
            return (*_func)(args, _rp);
        }

    private:
        function_type   _func;
        string_type     _name;
    };

}}   //namespace bsl::var
#endif  //__BSL_VAR_FUNCTION_H__

/* vim: set ts=4 sw=4 sts=4 tw=100 */
