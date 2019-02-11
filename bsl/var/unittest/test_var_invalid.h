/***************************************************************************
 * 
 * Copyright (c) 2009 Baidu.com, Inc. All Rights Reserved
 * $Id: test_var_invalid.h,v 1.3 2010/04/28 12:45:33 scmpf Exp $ 
 * 
 **************************************************************************/



/**
 * @file test_var_invalid.h
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2009/05/09 16:11:45
 * @version $Revision: 1.3 $ 
 * @brief 
 *  
 **/
#ifndef  __TEST_VAR_INVALID_H_
#define  __TEST_VAR_INVALID_H_

#include "bsl/var/implement.h"
#include "test.h"

inline void test_mask_consistency(const bsl::var::IVar& var){
    using namespace bsl::var;
    if ( var.is_number() ){
        assert( var.get_mask() & IVar::IS_NUMBER );
    }
    // assert( var.\1() == bool(\2) );
    assert( var.is_number() == bool(var.get_mask() & IVar::IS_NUMBER ) );
    assert( var.is_null() == bool((var.get_mask() & 0xFF) == 0) );
    assert( var.is_ref() == bool(var.get_mask() & bsl::var::IVar::IS_REF) );
#ifdef PHP_COMLOG
    assert( var.is_bool() == bool(var.get_mask() & bsl::var::IVar::_IS_BOOL) );
#else
    assert( var.is_bool() == bool(var.get_mask() & bsl::var::IVar::IS_BOOL) );
#endif
    assert( var.is_number() == bool(var.get_mask() & bsl::var::IVar::IS_NUMBER) );
    assert( var.is_int8() == bool(bsl::var::IVar::check_mask( var.get_mask(), bsl::var::IVar::IS_NUMBER|bsl::var::IVar::IS_ONE_BYTE|bsl::var::IVar::IS_SIGNED )) );
        
    assert( var.is_uint8() == bool(bsl::var::IVar::check_mask( var.get_mask(), bsl::var::IVar::IS_NUMBER|bsl::var::IVar::IS_ONE_BYTE, bsl::var::IVar::IS_SIGNED )) );
    assert( var.is_int16() == bool(bsl::var::IVar::check_mask( var.get_mask(), bsl::var::IVar::IS_NUMBER|bsl::var::IVar::IS_TWO_BYTE|bsl::var::IVar::IS_SIGNED )) );
    assert( var.is_uint16() == bool(bsl::var::IVar::check_mask( var.get_mask(), bsl::var::IVar::IS_NUMBER|bsl::var::IVar::IS_TWO_BYTE, bsl::var::IVar::IS_SIGNED )) );
    assert( var.is_int32() == bool(bsl::var::IVar::check_mask( var.get_mask(), bsl::var::IVar::IS_NUMBER|bsl::var::IVar::IS_FOUR_BYTE|bsl::var::IVar::IS_SIGNED )) );
    assert( var.is_uint32() == bool(bsl::var::IVar::check_mask( var.get_mask(), bsl::var::IVar::IS_NUMBER|bsl::var::IVar::IS_FOUR_BYTE, bsl::var::IVar::IS_SIGNED )) );
    assert( var.is_int64() == bool(bsl::var::IVar::check_mask( var.get_mask(), bsl::var::IVar::IS_NUMBER|bsl::var::IVar::IS_EIGHT_BYTE|bsl::var::IVar::IS_SIGNED )) );
    assert( var.is_uint64() == bool(bsl::var::IVar::check_mask( var.get_mask(), bsl::var::IVar::IS_NUMBER|bsl::var::IVar::IS_EIGHT_BYTE, bsl::var::IVar::IS_SIGNED )) );
    assert( var.is_float() == bool(bsl::var::IVar::check_mask( var.get_mask(), bsl::var::IVar::IS_NUMBER|bsl::var::IVar::IS_FOUR_BYTE|bsl::var::IVar::IS_FLOATING )) );
    assert( var.is_double() == bool(bsl::var::IVar::check_mask( var.get_mask(), bsl::var::IVar::IS_NUMBER|bsl::var::IVar::IS_EIGHT_BYTE|bsl::var::IVar::IS_FLOATING )) );
#ifdef PHP_COMLOG
    assert( var.is_string() == bool(var.get_mask() & bsl::var::IVar::_IS_STRING) );
    assert( var.is_array() == bool(var.get_mask() & bsl::var::IVar::_IS_ARRAY) );
#else
    assert( var.is_string() == bool(var.get_mask() & bsl::var::IVar::IS_STRING) );
    assert( var.is_array() == bool(var.get_mask() & bsl::var::IVar::IS_ARRAY) );
#endif
    assert( var.is_dict() == bool(var.get_mask() & bsl::var::IVar::IS_DICT) );
    assert( var.is_callable() == bool(var.get_mask() & bsl::var::IVar::IS_CALLABLE) );
    assert( var.is_raw() == bool(var.get_mask() & bsl::var::IVar::IS_RAW) );
}
inline void test_invalid_bool(bsl::var::IVar&  var){
    //non-const methods
    assert( !var.is_bool() );
    ASSERT_THROW( var.to_bool(), bsl::InvalidOperationException );
    ASSERT_THROW( var = true, bsl::InvalidOperationException );
    bsl::var::Bool b;
    if ( !var.is_ref() ){
        ASSERT_THROW( var = b, bsl::InvalidOperationException );
    }

    //const methods
    const bsl::var::IVar& const_var = var;
    assert( !const_var.is_bool() );
    ASSERT_THROW( const_var.to_bool(), bsl::InvalidOperationException );
}

inline void test_invalid_raw(bsl::var::IVar&  var){
    //non-const methods
    assert(!var.is_raw());
    ASSERT_THROW( var.to_raw(), bsl::InvalidOperationException );
    bsl::var::IVar::raw_type raw_;
    ASSERT_THROW( var = raw_, bsl::InvalidOperationException );
    bsl::var::ShallowRaw raw(raw_);
    if ( typeid(var) != typeid(bsl::var::String) && !var.is_ref() ){
        ASSERT_THROW( var = raw, bsl::InvalidOperationException );
    }

    //const methods
    const bsl::var::IVar& const_var = var;
    assert(!const_var.is_raw());
    ASSERT_THROW( const_var.to_raw(), bsl::InvalidOperationException );
}

template<typename PT> //primitive type
inline void test_valid_number(const bsl::var::IVar& var, PT pv ){
/*
signed char	int8
unsigned char	uint8
signed short	int16
unsigned short	uint16
signed int	int32
unsigned int	uint32
signed long long	int64
unsigned long long	uint64
*/
    if ( typeid(pv) == typeid(signed char) ){
        assert(var.is_int8());
        assert(var.to_int8() == static_cast<signed char>(pv) );
    }else{
        assert(!var.is_int8());
    }

    if ( typeid(pv) == typeid(unsigned char) ){
        assert(var.is_uint8());
        assert(var.to_uint8() == static_cast<unsigned char>(pv) );
    }else{
        assert(!var.is_uint8());
    }

    if ( typeid(pv) == typeid(signed short) ){
        assert(var.is_int16());
        assert(var.to_int16() == static_cast<signed short>(pv) );
    }else{
        assert(!var.is_int16());
    }

    if ( typeid(pv) == typeid(unsigned short) ){
        assert(var.is_uint16());
        assert(var.to_uint16() == static_cast<unsigned short>(pv) );
    }else{
        assert(!var.is_uint16());
    }

    if ( typeid(pv) == typeid(signed int) ){
        assert(var.is_int32());
        assert(var.to_int32() == static_cast<signed int>(pv) );
    }else{
        assert(!var.is_int32());
    }

    if ( typeid(pv) == typeid(unsigned int) ){
        assert(var.is_uint32());
        assert(var.to_uint32() == static_cast<unsigned int>(pv) );
    }else{
        assert(!var.is_uint32());
    }

    if ( typeid(pv) == typeid(signed long long) ){
        assert(var.is_int64());
        assert(var.to_int64() == static_cast<signed long long>(pv) );
    }else{
        assert(!var.is_int64());
    }

    if ( typeid(pv) == typeid(unsigned long long) ){
        assert(var.is_uint64());
        assert(var.to_uint64() == static_cast<unsigned long long>(pv) );
    }else{
        assert(!var.is_uint64());
    }

#define CHECK_THROWN( expr, except, thrown )\
    do{ \
        try{ expr; thrown = 0;\
        }catch( except& ){ thrown = 1; \
        }catch( ... ){ thrown = 2; } \
    }while(0)

    /*
     see(var.to_string());
     see(pv);
     see(v_thrown);
     see(p_thrown);
     see(v_res);
     see(p_res);
     */
    int v_thrown = 0;
    int p_thrown = 0;
    {
        signed char v_res = 0, p_res = 0;
        CHECK_THROWN( p_res = bsl::check_cast<signed char>(pv), bsl::OverflowException, p_thrown );
        CHECK_THROWN( v_res = var.to_int8(), bsl::OverflowException, v_thrown );
     see(var.to_string());
     see(pv);
     see(v_thrown);
     see(p_thrown);
     see(v_res);
     see(p_res);
        assert( p_thrown == v_thrown && p_res == v_res );
        CHECK_THROWN( p_res = bsl::check_cast<signed char>(pv), bsl::UnderflowException, p_thrown );
        CHECK_THROWN( v_res = var.to_int8(), bsl::UnderflowException, v_thrown );
        assert( p_thrown == v_thrown && p_res == v_res );
    }

    {
        unsigned char v_res = 0, p_res = 0;
        CHECK_THROWN( p_res = bsl::check_cast<unsigned char>(pv), bsl::OverflowException, p_thrown );
        CHECK_THROWN( v_res = var.to_uint8(), bsl::OverflowException, v_thrown );
        assert( p_thrown == v_thrown && p_res == v_res );
        CHECK_THROWN( p_res = bsl::check_cast<unsigned char>(pv), bsl::UnderflowException, p_thrown );
        CHECK_THROWN( v_res = var.to_uint8(), bsl::UnderflowException, v_thrown );
        assert( p_thrown == v_thrown && p_res == v_res );
    }

    {
        signed short v_res = 0, p_res = 0;
        CHECK_THROWN( p_res = bsl::check_cast<signed short>(pv), bsl::OverflowException, p_thrown );
        CHECK_THROWN( v_res = var.to_int16(), bsl::OverflowException, v_thrown );
        assert( p_thrown == v_thrown && p_res == v_res );
        CHECK_THROWN( p_res = bsl::check_cast<signed short>(pv), bsl::UnderflowException, p_thrown );
        CHECK_THROWN( v_res = var.to_int16(), bsl::UnderflowException, v_thrown );
        assert( p_thrown == v_thrown && p_res == v_res );
    }

    {
        unsigned short v_res = 0, p_res = 0;
        CHECK_THROWN( p_res = bsl::check_cast<unsigned short>(pv), bsl::OverflowException, p_thrown );
        CHECK_THROWN( v_res = var.to_uint16(), bsl::OverflowException, v_thrown );
        assert( p_thrown == v_thrown && p_res == v_res );
        CHECK_THROWN( p_res = bsl::check_cast<unsigned short>(pv), bsl::UnderflowException, p_thrown );
        CHECK_THROWN( v_res = var.to_uint16(), bsl::UnderflowException, v_thrown );
        assert( p_thrown == v_thrown && p_res == v_res );
    }

    {
        signed int v_res = 0, p_res = 0;
        CHECK_THROWN( p_res = bsl::check_cast<signed int>(pv), bsl::OverflowException, p_thrown );
        CHECK_THROWN( v_res = var.to_int32(), bsl::OverflowException, v_thrown );
        assert( p_thrown == v_thrown && p_res == v_res );
        CHECK_THROWN( p_res = bsl::check_cast<signed int>(pv), bsl::UnderflowException, p_thrown );
        CHECK_THROWN( v_res = var.to_int32(), bsl::UnderflowException, v_thrown );
        assert( p_thrown == v_thrown && p_res == v_res );
    }

    {
        unsigned int v_res = 0, p_res = 0;
        CHECK_THROWN( p_res = bsl::check_cast<unsigned int>(pv), bsl::OverflowException, p_thrown );
        CHECK_THROWN( v_res = var.to_uint32(), bsl::OverflowException, v_thrown );
        assert( p_thrown == v_thrown && p_res == v_res );
        CHECK_THROWN( p_res = bsl::check_cast<unsigned int>(pv), bsl::UnderflowException, p_thrown );
        CHECK_THROWN( v_res = var.to_uint32(), bsl::UnderflowException, v_thrown );
        assert( p_thrown == v_thrown && p_res == v_res );
    }

    {
        signed long long v_res = 0, p_res = 0;
        CHECK_THROWN( p_res = bsl::check_cast<signed long long>(pv), bsl::OverflowException, p_thrown );
        CHECK_THROWN( v_res = var.to_int64(), bsl::OverflowException, v_thrown );
        assert( p_thrown == v_thrown && p_res == v_res );
        CHECK_THROWN( p_res = bsl::check_cast<signed long long>(pv), bsl::UnderflowException, p_thrown );
        CHECK_THROWN( v_res = var.to_int64(), bsl::UnderflowException, v_thrown );
        assert( p_thrown == v_thrown && p_res == v_res );
    }

    {
        unsigned long long v_res = 0, p_res = 0;
        CHECK_THROWN( p_res = bsl::check_cast<unsigned long long>(pv), bsl::OverflowException, p_thrown );
        CHECK_THROWN( v_res = var.to_uint64(), bsl::OverflowException, v_thrown );
        assert( p_thrown == v_thrown && p_res == v_res );
        CHECK_THROWN( p_res = bsl::check_cast<unsigned long long>(pv), bsl::UnderflowException, p_thrown );
        CHECK_THROWN( v_res = var.to_uint64(), bsl::UnderflowException, v_thrown );
        assert( p_thrown == v_thrown && p_res == v_res );
    }

#undef CHECK_THROWN
        
}

inline void test_invalid_number(bsl::var::IVar&  var){
    //non-const methods
    assert(!var.is_number());
    assert(!var.is_int8());
    assert(!var.is_uint8());
    assert(!var.is_int16());
    assert(!var.is_uint16());
    assert(!var.is_int32());
    assert(!var.is_uint32());
    assert(!var.is_int64());
    assert(!var.is_uint64());
    assert(!var.is_double());

    ASSERT_THROW( var.to_int8(), bsl::InvalidOperationException );
    ASSERT_THROW( var.to_uint8(), bsl::InvalidOperationException );
    ASSERT_THROW( var.to_int16(), bsl::InvalidOperationException );
    ASSERT_THROW( var.to_uint16(), bsl::InvalidOperationException );
    ASSERT_THROW( var.to_int32(), bsl::InvalidOperationException );
    ASSERT_THROW( var.to_uint32(), bsl::InvalidOperationException );
    ASSERT_THROW( var.to_int64(), bsl::InvalidOperationException );
    ASSERT_THROW( var.to_uint64(), bsl::InvalidOperationException );
    ASSERT_THROW( var.to_double(), bsl::InvalidOperationException );

    ASSERT_THROW( var = static_cast<signed char>(123), bsl::InvalidOperationException );
    ASSERT_THROW( var = static_cast<unsigned char>(123), bsl::InvalidOperationException );
    ASSERT_THROW( var = static_cast<signed short>(123), bsl::InvalidOperationException );
    ASSERT_THROW( var = static_cast<unsigned short>(123), bsl::InvalidOperationException );
    ASSERT_THROW( var = static_cast<signed int>(123), bsl::InvalidOperationException );
    ASSERT_THROW( var = static_cast<unsigned int>(123), bsl::InvalidOperationException );
    ASSERT_THROW( var = static_cast<signed long long>(123), bsl::InvalidOperationException );
    ASSERT_THROW( var = static_cast<unsigned long long>(123), bsl::InvalidOperationException );
    ASSERT_THROW( var = 4670.9394, bsl::InvalidOperationException );

    bsl::var::Number<signed char> i8;
    bsl::var::Number<unsigned char> u8;
    bsl::var::Number<signed short> i16;
    bsl::var::Number<unsigned short> u16;
    bsl::var::Number<signed int> i32;
    bsl::var::Number<unsigned int> u32;
    bsl::var::Number<signed long long> i64;
    bsl::var::Number<unsigned long long> u64;
    bsl::var::Number<double> dbl;
    if ( !var.is_ref() ){
        ASSERT_THROW( var = i8, bsl::InvalidOperationException );
        ASSERT_THROW( var = u8, bsl::InvalidOperationException );
        ASSERT_THROW( var = i16, bsl::InvalidOperationException );
        ASSERT_THROW( var = u16, bsl::InvalidOperationException );
        ASSERT_THROW( var = i32, bsl::InvalidOperationException );
        ASSERT_THROW( var = u32, bsl::InvalidOperationException );
        ASSERT_THROW( var = i64, bsl::InvalidOperationException );
        ASSERT_THROW( var = u64, bsl::InvalidOperationException );
        ASSERT_THROW( var = dbl, bsl::InvalidOperationException );
    }

    //const methods
    const bsl::var::IVar& const_var = var;

    assert(!const_var.is_number());
    assert(!const_var.is_int8());
    assert(!const_var.is_uint8());
    assert(!const_var.is_int16());
    assert(!const_var.is_uint16());
    assert(!const_var.is_int32());
    assert(!const_var.is_uint32());
    assert(!const_var.is_int64());
    assert(!const_var.is_uint64());
    assert(!const_var.is_double());

    ASSERT_THROW( const_var.to_int8(), bsl::InvalidOperationException );
    ASSERT_THROW( const_var.to_uint8(), bsl::InvalidOperationException );
    ASSERT_THROW( const_var.to_int16(), bsl::InvalidOperationException );
    ASSERT_THROW( const_var.to_uint16(), bsl::InvalidOperationException );
    ASSERT_THROW( const_var.to_int32(), bsl::InvalidOperationException );
    ASSERT_THROW( const_var.to_uint32(), bsl::InvalidOperationException );
    ASSERT_THROW( const_var.to_int64(), bsl::InvalidOperationException );
    ASSERT_THROW( const_var.to_uint64(), bsl::InvalidOperationException );
    ASSERT_THROW( const_var.to_double(), bsl::InvalidOperationException );

}

inline void test_invalid_string(bsl::var::IVar&  var){
    //non-const methods
    assert( !var.is_string() );
    ASSERT_THROW( var.c_str(), bsl::InvalidOperationException );
    ASSERT_THROW( var.c_str_len(), bsl::InvalidOperationException );
    if ( !var.is_number() && !var.is_bool() ){
        //number类别可能支持下列方法
        ASSERT_THROW( var = "abc", bsl::InvalidOperationException );
        ASSERT_THROW( var = bsl::var::IVar::string_type("abc"), bsl::InvalidOperationException );
        if ( !var.is_ref() ){
            bsl::var::String str("abc");
            ASSERT_THROW( var = str, bsl::InvalidOperationException );
        }
    }

    //const methods
    const bsl::var::IVar& const_var = var;
    assert( !const_var.is_string() );
    ASSERT_THROW( const_var.c_str(), bsl::InvalidOperationException );
    ASSERT_THROW( const_var.c_str_len(), bsl::InvalidOperationException );
}

inline void test_invalid_array(bsl::var::IVar&  var){
    //non-const methods
    assert( !var.is_array() );
    //size()
    if ( !var.is_dict() ){
        ASSERT_THROW( var.size(), bsl::InvalidOperationException );
    }
    //get()
    ASSERT_THROW( var.get(0), bsl::InvalidOperationException );
    //set()
    ASSERT_THROW( var.set(0, var), bsl::InvalidOperationException );
    //del()
    ASSERT_THROW( var.del(0), bsl::InvalidOperationException );
    //operator []()
    ASSERT_THROW( var[-1], bsl::InvalidOperationException );
    ASSERT_THROW( var[0], bsl::InvalidOperationException );
    ASSERT_THROW( var[1], bsl::InvalidOperationException );
    //iterator
    ASSERT_THROW( var.array_begin(), bsl::InvalidOperationException );
    ASSERT_THROW( var.array_end(), bsl::InvalidOperationException );
    //operator =
    bsl::var::Array arr;
    if ( typeid(var) != typeid(bsl::var::String) && !var.is_ref() ){
        ASSERT_THROW( var = arr, bsl::InvalidOperationException );
    }

    //const methods
    const bsl::var::IVar& const_var = var;
    //get()
    ASSERT_THROW( const_var.get(0), bsl::InvalidOperationException );
    //operator []()
    ASSERT_THROW( const_var[-1], bsl::InvalidOperationException );
    ASSERT_THROW( const_var[0], bsl::InvalidOperationException );
    ASSERT_THROW( const_var[1], bsl::InvalidOperationException );
    //iterator
    ASSERT_THROW( const_var.array_begin(), bsl::InvalidOperationException );
    ASSERT_THROW( const_var.array_end(), bsl::InvalidOperationException );
}

inline void test_invalid_dict(bsl::var::IVar&  var){
    //non-const methods
    assert( !var.is_dict() );
    //size()
    if ( !var.is_array() ){
        ASSERT_THROW( var.size(), bsl::InvalidOperationException );
    }
    //get()
    ASSERT_THROW( var.get(""), bsl::InvalidOperationException );
    ASSERT_THROW( var.get("a key"), bsl::InvalidOperationException );
    ASSERT_THROW( var.get(bsl::var::IVar::string_type("another key")), bsl::InvalidOperationException );
    //set()
    ASSERT_THROW( var.set(bsl::var::IVar::string_type(""),var), bsl::InvalidOperationException );
    ASSERT_THROW( var.set("some key",var), bsl::InvalidOperationException );
    ASSERT_THROW( var.set(bsl::var::IVar::string_type("another key"),var), bsl::InvalidOperationException );
    //del()
    ASSERT_THROW( var.del("whatever"), bsl::InvalidOperationException );
    //operator []
    ASSERT_THROW( var[""], bsl::InvalidOperationException );
    ASSERT_THROW( var["awesome key"], bsl::InvalidOperationException );
    //iterator
    ASSERT_THROW( var.dict_begin(), bsl::InvalidOperationException );
    ASSERT_THROW( var.dict_end(), bsl::InvalidOperationException );
    //operator =
    bsl::var::Dict d;
    if ( typeid(var) != typeid(bsl::var::String) && !var.is_ref() ){
        ASSERT_THROW( var = d, bsl::InvalidOperationException );
    }

    //const methods
    const bsl::var::IVar& const_var = var;
    //get()
    ASSERT_THROW( const_var.get(""), bsl::InvalidOperationException );
    ASSERT_THROW( const_var.get("a key"), bsl::InvalidOperationException );
    ASSERT_THROW( const_var.get(bsl::var::IVar::string_type("another key")), bsl::InvalidOperationException );
    //operator []
    ASSERT_THROW( const_var[""], bsl::InvalidOperationException );
    ASSERT_THROW( const_var["awesome key"], bsl::InvalidOperationException );
    //iterator
    ASSERT_THROW( const_var.dict_begin(), bsl::InvalidOperationException );
    ASSERT_THROW( const_var.dict_end(), bsl::InvalidOperationException );
}

inline bsl::var::IVar& echo_f( bsl::var::IVar& args, bsl::ResourcePool& ){
    return args;
}

inline bsl::var::IVar& echo_m( bsl::var::IVar& self, bsl::var::IVar& args, bsl::ResourcePool& rp){
    bsl::var::IVar& res = rp.create<bsl::var::Dict>();
    res["self"] = self;
    res["args"] = args;
    return res;
}

inline void test_invalid_callable(bsl::var::IVar&  var){
    //non-const methods
    assert( !var.is_callable() );
    bsl::var::Dict  self;
    bsl::var::Array params;
    bsl::ResourcePool rp;

    ASSERT_THROW( var(params, rp), bsl::InvalidOperationException );
    ASSERT_THROW( var(self, params, rp), bsl::InvalidOperationException );
    bsl::var::Function func(echo_f, "echo_f");
    if ( typeid(var) != typeid(bsl::var::String) && !var.is_ref() ){
        ASSERT_THROW( var = func, bsl::InvalidOperationException );
    }
    bsl::var::Function method(echo_f, "echo_f");
    if ( typeid(var) != typeid(bsl::var::String) && !var.is_ref() ){
        ASSERT_THROW( var = method, bsl::InvalidOperationException );
    }

    //no const methods defined
}
#endif  //__TEST_VAR_INVALID_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
