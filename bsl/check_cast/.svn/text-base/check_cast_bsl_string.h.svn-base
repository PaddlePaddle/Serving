/***************************************************************************
 * 
 * Copyright (c) 2009 Baidu.com, Inc. All Rights Reserved
 * $Id: check_cast_bsl_string.h,v 1.2 2009/10/14 08:30:49 chenxm Exp $ 
 * 
 **************************************************************************/



/**
 * @file check_cast_bsl_string.h
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2009/09/24 14:18:05
 * @version $Revision: 1.2 $ 
 * @brief 
 *
 * 特别感谢：
 *      本文件部分代码由QAD李伟刚同学贡献
 *  
 **/
#ifndef  __CHECK_CAST_BSL_STRING_H_
#define  __CHECK_CAST_BSL_STRING_H_

//internal use
#ifndef  __CHECK_CAST_H_
#error "this file cannot be included directly, please include \"check_cast.h\" instead"
#endif

#include "bsl/containers/string/bsl_string.h"
namespace bsl{
    //to bsl::string
    /**
     * @brief 将字符转换成bsl::string，转换结果为一个只含有一个字符的bsl::string
     *
     * @param [in/out] value   : char
     * @return  bsl::string 
     * @retval   
     * @see 
     * @note 
     * @author lwg
     * @date 2009/08/11 17:14:10
     **/
    template<>
        inline bsl::string check_cast<bsl::string, char>(char value) {
            return bsl::string().appendf("%c", value);
        }

    /**
     * @brief 将8位整数（int8_t）转换成bsl::string，使用十进制
     *
     * @param [in/out] value   : unsigned char
     * @return  bsl::string 
     * @retval   
     * @see 
     * @note 
     * @author chenxm
     * @date 2009/08/11 17:16:20
     **/
    template<>
        inline bsl::string check_cast<bsl::string, signed char>(signed char value)
        {
            return bsl::string().appendf("%hhd", value);
        }

    /**
     * @brief 将8位整数（uint8_t）转换成bsl::string，使用十进制
     *
     * @param [in/out] value   : unsigned char
     * @return  bsl::string 
     * @retval   
     * @see 
     * @note 
     * @author chenxm
     * @date 2009/08/11 17:16:20
     **/
    template<>
        inline bsl::string check_cast<bsl::string, unsigned char>(unsigned char value)
        {
            return bsl::string().appendf("%hhu", value);
        }

    template<>
        inline bsl::string check_cast<bsl::string, short>(short value)
        {
            return bsl::string().appendf("%hd", value);
        }

    template<>
        inline bsl::string check_cast<bsl::string, unsigned short>(unsigned short value)
        {
            return bsl::string().appendf("%hu", value);
        }

    template<>
        inline bsl::string check_cast<bsl::string, int>(int value)
        {
            return bsl::string().appendf("%d", value);
        }

    template<>
        inline bsl::string check_cast<bsl::string, unsigned int>(unsigned int value)
        {
            return bsl::string().appendf("%u", value);
        }

    template<>
        inline bsl::string check_cast<bsl::string, long>(long value)
        {
            return bsl::string().appendf("%ld", value);
        }

    template<>
        inline bsl::string check_cast<bsl::string, unsigned long>(unsigned long value)
        {
            return bsl::string().appendf("%lu", value);
        }

    template<>
        inline bsl::string check_cast<bsl::string, long long>(long long value)
        {
            return bsl::string().appendf("%lld", value);
        }

    template<>
        inline bsl::string check_cast<bsl::string, unsigned long long>(unsigned long long value)
        {
            return bsl::string().appendf("%llu", value);
        }

    template<>
        inline bsl::string check_cast<bsl::string, float>(float value)
        {
            return bsl::string().appendf("%g", value);
        }

    template<>
        inline bsl::string check_cast<bsl::string, double>(double value)
        {
            return bsl::string().appendf("%lg", value);
        }

    template<>
        inline bsl::string check_cast<bsl::string, long double>(long double value)
        {
            return bsl::string().appendf("%Lg", value);
        }

    template<>
        inline bsl::string check_cast<bsl::string, char *>(char * value)
        {
            return value;
        }

    template<>
        inline bsl::string check_cast<bsl::string, const char *>(const char * value)
        {
            return value;
        }

    //from bsl::string
    /* 可能会有效率问题
    template<>
        inline char check_cast<char,bsl::string>( bsl::string s ){
            return s[0];
        }

    template<>
        inline long check_cast<long, bsl::string> ( bsl::string s ){
            return check_cast<long>(s.c_str());
        }

    //signed char is used as int8
    template<>
        inline signed char check_cast<signed char,bsl::string>( bsl::string s ){
            return check_cast<signed char>( s.c_str() );
        }

    //unsigned char is used as uint8
    template<>
        inline unsigned char check_cast<unsigned char,bsl::string>( bsl::string s ){
            return check_cast<unsigned char>( s.c_str() );
        }

    template<>
        inline short check_cast<short, bsl::string> ( bsl::string s ){
            return check_cast<short>( s.c_str() );
        }

    template<>
        inline int check_cast<int, bsl::string> ( bsl::string s ){
            return check_cast<int>( s.c_str() );
        }

    template<>
        inline long long check_cast<long long, bsl::string>( bsl::string s ){
            return check_cast<long long>( s.c_str() );

    template<>
        inline unsigned long check_cast<unsigned long, bsl::string>( bsl::string s ){
            return check_cast<unsigned long>( s.c_str() );
        }

    template<>
        inline unsigned short check_cast<unsigned short, bsl::string>( bsl::string s ){
            return check_cast<unsigned short>( s.c_str() );
        }

    template<>
        inline unsigned int check_cast<unsigned int, bsl::string>( bsl::string s ){
            return check_cast<unsigned int>( s.c_str() );
        }

    template<>
        inline unsigned long long check_cast<unsigned long long, bsl::string>( bsl::string s ){
            return check_cast<unsigned long long>( s.c_str() );
        }

    template<>
        inline float check_cast<float, bsl::string>( bsl::string s ){
            return check_cast<float>( s.c_str() );
        }

    template<>
        inline double check_cast<double, bsl::string>( bsl::string s ){
            return check_cast<double>( s.c_str() );
        }

    template<>
        inline long double check_cast<long double, bsl::string>( bsl::string s ){
            return check_cast<long double>( s.c_str() );
        }

    template<>
        inline const char * check_cast<const char *, bsl::string>( bsl::string s ){
            return s.c_str();
        }
    */
}


#endif  //__CHECK_CAST_BSL_STRING_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
