/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: test_var_ShallowString.cpp,v 1.3 2010/04/28 12:45:33 scmpf Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file test_VarShallowString.cpp
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2008/09/29 15:04:28
 * @version $Revision: 1.3 $ 
 * @brief 
 *  
 **/
#include "bsl/var/implement.h"
#include "test_var_invalid.h"

class TestVarShallowString: public ITestVar{
    
public:
    typedef bsl::string     string_type;
    typedef bsl::string     field_type;
    typedef bsl::var::ArrayIterator      array_iterator;
    typedef bsl::var::ArrayConstIterator array_const_iterator;

    TestVarShallowString()
        :_str("Œ÷≤›Œ¢ŒË"){}

    virtual ~TestVarShallowString(){}

    //special methods
    virtual void test_special(){
        //copy ctor
        {
            const char * cs = "hello";
            bsl::var::ShallowString i = cs;
            bsl::var::ShallowString j = i;
            assert( j.is_string() );
            assert( j.c_str() == cs );
            i = "acumon";
            assert( j.to_string() == "hello" );
        }
        //copy assign
        {
            const char * cs = "hello";
            bsl::var::ShallowString i = cs;
            bsl::var::ShallowString j;
            j = i;
            assert( j.is_string() );
            assert( j.c_str() == cs );
            i = "acumon";
            assert( j.to_string() == "hello" );
        }
    }
    
    //methods for all
    virtual void test_mask(){
        test_mask_consistency( bsl::var::ShallowString("str") );
    }

    virtual void test_operator_assign(){
        // = const char*
        {
            bsl::var::ShallowString i;
            const char *cs1 = "46709394";
            i = cs1;
            assert( i.to_int32() == 46709394 );

            const char *cs2 = "abcdef";
            i = cs2;
            assert( i.c_str() == cs2 );

        }

        // = string_type
        {
            bsl::var::ShallowString i;
            string_type str1("46709394");
            i = str1;
            assert( i.c_str() == str1.c_str() );
            assert( i.to_int32() == 46709394 );

            string_type str2("abcdef");
            i = str2;
            assert( i.to_string() == "abcdef" );

        }

        // = ShallowString
        {
            bsl::var::ShallowString i, j;
            j = "a secret";
            i = j;
            assert( i.to_string() == "a secret" );
         }

        // = *this
        {
            bsl::var::ShallowString i;
            i = "self";
            i = i;
            assert( i.to_string() == "self" );
         }
                
    }

    virtual void test_clear() {
        {
            bsl::var::ShallowString i("will be cleared");
            i.clear();    //assert no-throw
            assert(i.to_string() == "");
        }
    }

    virtual void test_to_string() {
        {
            bsl::var::ShallowString i;
            assert( i.to_string() == string_type("") );
        }

        {
            bsl::var::ShallowString i("1234567");
            assert( i.to_string() == string_type("1234567") );
        }
    }

    virtual void test_dump(){
        {
            bsl::var::ShallowString i;
            assert( i.dump() == string_type("[bsl::var::ShallowString]") );
            assert( i.dump(999) == string_type("[bsl::var::ShallowString]") );
        }
        {
            bsl::var::ShallowString i("1234567");
            assert( i.dump() == string_type("[bsl::var::ShallowString]1234567") );
            assert( i.dump(999) == string_type("[bsl::var::ShallowString]1234567") );
        }
    }

    virtual void test_get_type() {
        assert( bsl::var::ShallowString().get_type() == string_type("bsl::var::ShallowString") );
    }

    //method for value
    virtual void test_bool(){
        //= bool
        {
            bsl::var::ShallowString str("Acumon");
            ASSERT_THROW( str = true, bsl::InvalidOperationException );
        }
        {
            bsl::var::ShallowString str("Acumon");
            ASSERT_THROW( str = false, bsl::InvalidOperationException );
        }
        // to bool
        {
            assert( bsl::var::ShallowString().to_bool() == false );
            assert( bsl::var::ShallowString("false").to_bool() == true );
        }
    }

    virtual void test_raw(){
        test_invalid_raw(_str);
    }

    virtual void test_number(){
        test_to_int32();
        test_to_int64();
        test_to_double();
    }
    virtual void test_clone(){
        bsl::ResourcePool rp;
        bsl::var::ShallowString ss = "abc";
        assert( ss.clone(rp).get_type() == ss.get_type() );
        assert( ss.clone(rp).c_str() == ss.c_str() );
    }
    virtual void test_string(){
        test_c_str();
    }

    //methods for array
    virtual void test_array(){
        test_invalid_array(_str);
    }

    //methods for dict
    virtual void test_dict(){
        test_invalid_dict(_str);
    }

    //methods for callable
    virtual void test_callable(){
        test_invalid_callable(_str);
    }

    virtual void test_to_int32(){
        {
            ASSERT_THROW( bsl::var::ShallowString().to_int32(), bsl::BadCastException );
            ASSERT_THROW( bsl::var::ShallowString("not a number!").to_int32(), bsl::BadCastException );

            assert( bsl::var::ShallowString("0").to_int32() == 0 );
            assert( bsl::var::ShallowString("-1").to_int32() == -1 );
            assert( bsl::var::ShallowString("1234567").to_int32() == 1234567 );
        }
    }

    virtual void test_to_int64(){
        {
            ASSERT_THROW( bsl::var::ShallowString().to_int64(), bsl::BadCastException );
            ASSERT_THROW( bsl::var::ShallowString("not a number!").to_int64(), bsl::BadCastException );

            assert( bsl::var::ShallowString("+0").to_int64() == 0 );
            assert( bsl::var::ShallowString("-123456789123").to_int64() == -123456789123LL );
            assert( bsl::var::ShallowString("+123456746709394").to_int64() == 123456746709394LL );
        }
    }

    virtual void test_to_double(){
        {
            ASSERT_THROW( bsl::var::ShallowString().to_double(), bsl::BadCastException );
            ASSERT_THROW( bsl::var::ShallowString("not a number!").to_double(), bsl::BadCastException );

            assert( bsl::var::ShallowString("-4670.9394").to_double() == -4670.9394 );
            assert( bsl::var::ShallowString("+0.00000000").to_double() == 0 );
            assert( bsl::var::ShallowString("123.456").to_double() == 123.456 );
        }
    }

    virtual void test_c_str(){
        {
            bsl::var::ShallowString i;
            assert( i.c_str() == string_type("") );
            assert( i.c_str_len() == 0 );
        }

        {
            const char * cs = "1234567";
            bsl::var::ShallowString i(cs);
            assert( i.c_str() == cs );
            assert( i.c_str_len() == strlen(cs) );
        }
    }

private:
    bsl::var::ShallowString _str;
};

int main(){
    TestVarShallowString().test_all();
    return 0;
}


/* vim: set ts=4 sw=4 sts=4 tw=100 */
