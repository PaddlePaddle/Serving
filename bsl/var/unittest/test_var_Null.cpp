/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: test_var_Null.cpp,v 1.4 2010/04/28 12:45:33 scmpf Exp $ 
 * 
 **************************************************************************/



/**
 * @file test_VarNull.cpp
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2008/09/24 20:57:41
 * @version $Revision: 1.4 $ 
 * @brief 
 *  
 **/
#include "test_var_invalid.h"

class TestVarNull: public ITestVar{

public:
    typedef bsl::var::IVar::string_type   string_type;
    typedef bsl::var::IVar::field_type    field_type;
    typedef bsl::var::ArrayIterator      array_iterator;
    typedef bsl::var::ArrayConstIterator array_const_iterator;

    virtual ~TestVarNull(){}

    //speical methods
    virtual void test_special(){
        // copy ctor
        {
            bsl::var::Null null1;
            bsl::var::Null null2(null1);
            assert( null2.is_null() );
        }
        // copy assign
        {
            bsl::var::Null null1, null2;
            null2 = null1;
            assert( null2.is_null() );
        }
    }
    //methods for all
    virtual void test_mask(){
        test_mask_consistency( bsl::var::Null() );
    }

    virtual void test_clear() {
        {
            bsl::var::Null null;
            null.clear();    //assert no-throw
        }
    }

    virtual void test_dump() {
        {
            bsl::var::Null null;
            assert( null.dump() == string_type("[bsl::var::Null]null") );
            assert( null.dump(999) == string_type("[bsl::var::Null]null") );
        }

    }

    virtual void test_to_string() {
        {
            bsl::var::Null null;
            assert( null.to_string() == string_type("null") );
        }

    }

    virtual void test_get_type() {
        assert( bsl::var::Null().get_type() == string_type("bsl::var::Null") );
    }

    //method for value
    virtual void test_bool(){
        test_invalid_bool(bsl::var::Null::null);
    }
    virtual void test_raw(){
        test_invalid_raw(bsl::var::Null::null);
    }
    virtual void test_number(){
        test_invalid_number(bsl::var::Null::null);
    }
    virtual void test_clone(){
        bsl::ResourcePool rp;
        assert( bsl::var::Null::null.clone(rp).is_null() );
    }

    virtual void test_string(){
        test_invalid_string(bsl::var::Null::null);
    }

    //methods for array
    virtual void test_array(){
        test_invalid_array(bsl::var::Null::null);
    }

    //methods for dict
    virtual void test_dict(){
        test_invalid_dict(bsl::var::Null::null);
    }

    //methods for callable
    virtual void test_callable(){
        test_invalid_callable(bsl::var::Null::null);
    }

    virtual void test_operator_assign(){
        //valid assign
        {
            bsl::var::Null null1, null2;
            bsl::var::Ref ref_to_null;
            null1 = null1;
            assert( null1.is_null() );

            null1 = null2;
            assert( null1.is_null() );

            null1 = (bsl::var::IVar&)(null2);
            assert( null1.is_null() );

            null1 = ref_to_null;
            assert( null1.is_null() );

        }
        //invalid assign
        {
            bsl::var::Null null;
            bsl::var::Int32 i32;
            ASSERT_THROW( null = 123, bsl::InvalidOperationException );
            ASSERT_THROW( null = 123LL, bsl::InvalidOperationException );
            ASSERT_THROW( null = 123.456, bsl::InvalidOperationException );
            ASSERT_THROW( null = "123", bsl::InvalidOperationException );
            ASSERT_THROW( null = i32, bsl::InvalidOperationException );
        }
    }

private:
};

int main(){
    TestVarNull test;
    test.test_all();
    return 0;
}

/* vim: set ts=4 sw=4 sts=4 tw=100 */
