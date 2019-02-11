/***************************************************************************
 * 
 * Copyright (c) 2009 Baidu.com, Inc. All Rights Reserved
 * $Id: test_var_Bool.cpp,v 1.3 2010/04/28 12:45:33 scmpf Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file test_var_Bool.cpp
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2009/05/09 20:27:23
 * @version $Revision: 1.3 $ 
 * @brief 
 *  
 **/
#include "test_var_invalid.h"

class TestVarBool: public ITestVar{

public:
    typedef bsl::var::IVar::string_type   string_type;
    typedef bsl::var::IVar::field_type    field_type;
    typedef bsl::var::ArrayIterator      array_iterator;
    typedef bsl::var::ArrayConstIterator array_const_iterator;

    virtual ~TestVarBool(){}

    //speical methods
    virtual void test_special(){
        //ctor
        {
            bsl::var::Bool b;
            assert( b.to_bool() == false );
        }
        {
            bsl::var::Bool b(true);
            assert( b.to_bool() == true );
        }
        {
            bsl::var::Bool b(false);
            assert( b.to_bool() == false );
        }
        // copy ctor
        {
            bsl::var::Bool b1(true);
            bsl::var::Bool b2(b1);
            assert( b2.to_bool() == true );
           
        }
        // copy assign
        {
            bsl::var::Bool b1(true), b2(false);
            b1 = b2;
            assert( b1.to_bool() == false );
        }
    }
    virtual void test_mask(){
        test_mask_consistency( bsl::var::Bool() );
    }
    //methods for all
    virtual void test_clear() {
        {
            bsl::var::Bool b;
            b.clear();    //assert no-throw
            assert( b.to_bool() == false );
        }
        {
            bsl::var::Bool b(true);
            b.clear();
            assert( b.to_bool() == false );
        }
    }

    virtual void test_dump() {
        {
            bsl::var::Bool b;
            assert( b.dump() == "[bsl::var::Bool]false" );
            assert( b.dump(999) == "[bsl::var::Bool]false" );
        }
        {
            bsl::var::Bool b(true);
            assert( b.dump() == "[bsl::var::Bool]true" );
            assert( b.dump(999) == "[bsl::var::Bool]true" );
        }

    }

    virtual void test_to_string() {
        {
            bsl::var::Bool b;
            assert( b.to_string() == "false" );
        }
        {
            bsl::var::Bool b(true);
            assert( b.to_string() == "true" );
        }

    }

    virtual void test_get_type() {
        assert( bsl::var::Bool().get_type() == string_type("bsl::var::Bool") );
    }

    //method for value
    virtual void test_bool(){
        {
            bsl::var::Bool b;
            b = true;
            assert( b.to_bool() == true );
        }
        {
            const bsl::var::Bool b(true);
            assert( b.to_bool() == true );
        }
    }
    virtual void test_raw(){
        bsl::var::Bool b;
        test_invalid_raw(b);
    }

    virtual void test_number(){
        test_with_int32();
        test_with_int64();
        test_with_double();
        
        test_valid_number( bsl::var::Bool(true), true );
        test_valid_number( bsl::var::Bool(false), false );
    }

    virtual void test_clone(){
        bsl::ResourcePool rp;
        bsl::var::Bool b;
        assert(b.clone(rp).to_string() == b.to_string() );
        assert(b.clone(rp).get_type() == b.get_type() );
        test_valid_number( bsl::var::Bool(true).clone(rp), true );
        test_valid_number( bsl::var::Bool(false).clone(rp), false );
    }

    virtual void test_string(){
        bsl::var::Bool b;
        test_valid_string();
        test_invalid_string(b);
    }

    //methods for array and dict
    virtual void test_array(){
        bsl::var::Bool b;
        test_invalid_array(b);
    }

    //methods for dict
    virtual void test_dict(){
        bsl::var::Bool b;
        test_invalid_dict(b);
    }

    //methods for callable
    virtual void test_callable(){
        bsl::var::Bool b;
        test_invalid_callable(b);
    }

    virtual void test_operator_assign(){
        //valid assign
        {
            bsl::var::Bool b1, b2(true);
            b1 = (bsl::var::IVar&)(b2);
            assert( b1.to_bool() == true );
        }
        {
            bsl::var::Bool b1, b2(true);
            bsl::var::Ref ref = b2;
            b1 = ref;
            assert( b1.to_bool() == true );
        }

    }

    virtual void test_valid_string(){
        //= const char *
        {
            bsl::var::Bool b(true);
            b = static_cast<const char *>(NULL);
            assert( b.to_bool() == false );
        }
        {
            bsl::var::Bool b(true);
            b = "";
            assert( b.to_bool() == false );
        }
        {
            bsl::var::Bool b(true);
            b = "abc";
            assert( b.to_bool() == true );
        }

        //= string_type
        {
            bsl::var::Bool b(true);
            b = string_type("");
            assert( b.to_bool() == false );
        }
        {
            bsl::var::Bool b(true);
            b = string_type("false");   //wierd but correct
            assert( b.to_bool() == true );
        }

    }
    virtual void test_with_int32(){
        //operator =
        {
            bsl::var::Bool b;
            b = 123;
            assert( b.to_bool() == true );
            b = 0;
            assert( b.to_bool() == false );
        }
        //to_int32()
        {
            assert( bsl::var::Bool().to_int32() == 0 );
            assert( bsl::var::Bool(true).to_int32() == 1 );
        }
        //from Int32
        {
            bsl::var::Int32 i32(9394);
            bsl::var::Bool b;
            b = i32;
            assert( b.to_bool() == true );
        }
        {
            bsl::var::Int32 i32;
            bsl::var::Bool b;
            b = i32;
            assert( b.to_bool() == false );
        }
        //to Int32
        {
            bsl::var::Bool b(true);
            bsl::var::Int32 i32;
            i32 = b;
            assert( i32.to_int32() == 1 );
        }
        {
            bsl::var::Bool b;
            bsl::var::Int32 i32;
            i32 = b;
            assert( i32.to_int32() == 0 );
        }
    }

    virtual void test_with_int64(){
        //operator =
        {
            bsl::var::Bool b;
            b = 123LL;
            assert( b.to_bool() == true );
            b = 0LL;
            assert( b.to_bool() == false );
        }
        //to_int64()
        {
            assert( bsl::var::Bool().to_int64() == 0 );
            assert( bsl::var::Bool(true).to_int64() == 1 );
        }
        //from Int64
        {
            bsl::var::Int64 i64(9394);
            bsl::var::Bool b;
            b = i64;
            assert( b.to_bool() == true );
        }
        {
            bsl::var::Int64 i64;
            bsl::var::Bool b;
            b = i64;
            assert( b.to_bool() == false );
        }
        //to Int64
        {
            bsl::var::Bool b(true);
            bsl::var::Int64 i64;
            i64 = b;
            assert( i64.to_int64() == 1 );
        }
        {
            bsl::var::Bool b;
            bsl::var::Int64 i64;
            i64 = b;
            assert( i64.to_int64() == 0 );
        }
    }

    virtual void test_with_double(){
        //operator =
        {
            bsl::var::Bool b;
            b = 123.456;
            assert( b.to_bool() == true );
            b = 0.0;
            assert( b.to_bool() == false );
        }
        //to_double()
        {
            assert( bsl::var::Bool().to_double() == 0 );
            assert( bsl::var::Bool(true).to_double() == 1 );
        }
        //from Double
        {
            bsl::var::Double dbl(4670.9394);
            bsl::var::Bool b;
            b = dbl;
            assert( b.to_bool() == true );
        }
        {
            bsl::var::Double dbl;
            bsl::var::Bool b;
            b = dbl;
            assert( b.to_bool() == false );
        }
        //to Double
        {
            bsl::var::Bool b(true);
            bsl::var::Double dbl;
            dbl = b;
            assert( dbl.to_double() == 1 );
        }
        {
            bsl::var::Bool b;
            bsl::var::Double dbl;
            dbl = b;
            assert( dbl.to_double() == 0 );
        }
    }

private:
};

int main(){
    TestVarBool test;
    test.test_all();
    return 0;
}

/* vim: set ts=4 sw=4 sts=4 tw=100 */
