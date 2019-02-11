/***************************************************************************
 * 
 * Copyright (c) 2009 Baidu.com, Inc. All Rights Reserved
 * $Id: utils.cpp,v 1.3  2010/09/08 $ 
 * 
 **************************************************************************/
 
 
#include "bsl/var/utils.h"

/**
 * @file utils.cpp
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2010/09/08 modified by Zhu Jianwei
 * @version $Revision: 1.3 $ 
 * @brief 
 *  
 **/

namespace bsl{
    namespace var{
        void dump_to_string(const IVar& var, IVar::string_type& res, size_t verbose_level, const char *line_delimiter, size_t per_indent, size_t total_indent){
            if ( NULL == line_delimiter ){
                throw NullPointerException()<<BSL_EARG<<"line_delimiter is NULL";
            }
            if ( var.is_array() || var.is_dict() ){
                //先输出简略的信息
                res.append( var.dump(0) );
                if ( verbose_level > 0 ){
                    //输出详细内容
                    bool first = true;
                    //如果是数组，按数组方式遍历
                    if ( var.is_array() ){
                        IVar::array_const_iterator iter_= var.array_begin();
                        IVar::array_const_iterator end  = var.array_end();
                        for(; iter_ != end; ++ iter_ ){
                            if ( first ){
                                res.append("{").append(line_delimiter);
                                first = false;
                            }else{
                                res.append(",").append(line_delimiter);
                            }
                            //输出缩进及key
                            res.append( total_indent + per_indent, ' ' ).appendf("%zd", iter_->key()).append(": ");
                            //递归输出儿子结点
                            dump_to_string( iter_->value(), res, verbose_level-1, line_delimiter, per_indent, total_indent + per_indent );
                        }
                    }
                    //如果是字典，按字典方式遍历
                    if ( var.is_dict() ){
                        IVar::dict_const_iterator iter_= var.dict_begin();
                        IVar::dict_const_iterator end  = var.dict_end();
                        for(; iter_ != end; ++ iter_ ){
                            if ( first ){
                                res.append("{").append(line_delimiter);
                                first = false;
                            }else{
                                res.append(",").append(line_delimiter);
                            }
                            //输出缩进及key
                            res.append( total_indent + per_indent, ' ' ).append(iter_->key()).append(": ");
                            //递归输出儿子结点
                            dump_to_string( iter_->value(), res, verbose_level-1, line_delimiter, per_indent, total_indent + per_indent );
                        }
                    }
                    //如果曾经输出过内容，则输出闭括号
                    if ( !first ){
                        res.append(line_delimiter).append(total_indent, ' ').append("}");
                    }
                }
            }else{
                //不是数组不是字典，则按dump(verbose_level)输出
                res.append( var.dump(verbose_level) );
            }
        }

        void print( const IVar& var, size_t verbose_level ){
            IVar::string_type res;
            dump_to_string(var, res, verbose_level, "\n" );
            printf("%s\n", res.c_str() );
        }

    }//namespace var
}//namespace bsl
/* vim: set ts=4 sw=4 sts=4 tw=100 */
