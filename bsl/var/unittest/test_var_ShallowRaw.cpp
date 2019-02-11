/***************************************************************************
 * 
 * Copyright (c) 2009 Baidu.com, Inc. All Rights Reserved
 * $Id: test_var_ShallowRaw.cpp,v 1.3 2010/04/28 12:45:33 scmpf Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file test_var_ShallowRaw.cpp
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2009/05/09 20:27:23
 * @version $Revision: 1.3 $ 
 * @brief 
 *  
 **/
#include "test_var_invalid.h"

class TestVarShallowRaw: public ITestVar{

public:
    typedef bsl::var::IVar::string_type   string_type;
    typedef bsl::var::IVar::field_type    field_type;
    typedef bsl::var::ArrayIterator      array_iterator;
    typedef bsl::var::ArrayConstIterator array_const_iterator;

    virtual ~TestVarShallowRaw(){}

    //speical methods
    virtual void test_special(){
        //ctor
        {
            bsl::var::ShallowRaw raw;
            assert( raw.to_raw().data == NULL );
            assert( raw.to_raw().length == 0 );
        }
        {
            bsl::var::raw_t raw_(static_cast<const void *>("hello"), strlen("hello")+1);
            bsl::var::ShallowRaw raw(raw_);
            assert( raw.to_raw().data == raw_.data );
            assert( raw.to_raw().length == raw_.length );
        }
        {
            bsl::var::raw_t raw_("hello", strlen("hello")+1);
            bsl::var::ShallowRaw raw( raw_.data, raw_.length );
            assert( raw.to_raw().data == raw_.data );
            assert( raw.to_raw().length == raw_.length );
        }
        // copy ctor
        {
            bsl::var::raw_t raw_("hello", strlen("hello")+1);
            bsl::var::ShallowRaw raw1(raw_);
            bsl::var::ShallowRaw raw2(raw1);
            assert( raw2.to_raw().data == raw_.data );
            assert( raw2.to_raw().length == raw_.length );
           
        }
        // copy assign
        {
            bsl::var::raw_t raw_("hello", strlen("hello")+1);
            bsl::var::ShallowRaw raw1(raw_);
            bsl::var::ShallowRaw raw2;
            raw2 = raw1;
            assert( raw2.to_raw().data == raw_.data );
            assert( raw2.to_raw().length == raw_.length );
            raw2 = raw2;
            assert( raw2.to_raw().data == raw_.data );
            assert( raw2.to_raw().length == raw_.length );
        }
    }
    //methods for all
    virtual void test_mask(){
        test_mask_consistency( bsl::var::ShallowRaw("hello",1) );
    }

    virtual void test_clear() {
        {
            bsl::var::ShallowRaw raw;
            raw.clear();
            assert( raw.to_raw().data == NULL );
            assert( raw.to_raw().length == 0 );
        }
        {
            bsl::var::ShallowRaw raw("hi", 1);
            raw.clear();
            assert( raw.to_raw().data == NULL );
            assert( raw.to_raw().length == 0 );
        }
    }

    virtual void test_dump(){
        stub();
    }

    virtual void test_to_string() {
        stub();
    }

    virtual void test_get_type() {
        assert( bsl::var::ShallowRaw().get_type() == string_type("bsl::var::ShallowRaw") );
    }

    //method for value
    virtual void test_bool(){
        bsl::var::ShallowRaw raw;
        test_invalid_bool(raw);
    }
    virtual void test_raw(){
        bsl::var::raw_t raw_("Acumon", 3);
        bsl::var::ShallowRaw raw;
        raw = raw_;
        assert( raw.to_raw().data == raw_.data );
        assert( raw.to_raw().length == raw_.length );

        const bsl::var::ShallowRaw const_raw(raw_);
        assert( const_raw.to_raw().data == raw_.data );
        assert( const_raw.to_raw().length == raw_.length );
        
    }
    virtual void test_number(){
        bsl::var::ShallowRaw raw;
        test_invalid_number(raw);
    }
    virtual void test_clone(){
        bsl::var::raw_t raw_("hello", 5);
        bsl::var::ShallowRaw raw(raw_);
        bsl::ResourcePool rp;
        assert( raw.clone(rp).get_type() == raw.get_type() );
        assert( raw.clone(rp).to_raw().data == raw_.data );
        assert( raw.clone(rp).to_raw().length == raw_.length );
    }

    virtual void test_string(){
        bsl::var::ShallowRaw raw;
        test_invalid_string(raw);
    }

    //methods for array and dict
    virtual void test_array(){
        bsl::var::ShallowRaw raw;
        test_invalid_array(raw);
    }

    //methods for dict
    virtual void test_dict(){
        bsl::var::ShallowRaw raw;
        test_invalid_dict(raw);
    }

    //methods for callable
    virtual void test_callable(){
        bsl::var::ShallowRaw raw;
        test_invalid_callable(raw);
    }

    virtual void test_operator_assign(){
        //valid assign
        {
            bsl::var::raw_t raw_("", 1);
            bsl::var::ShallowRaw raw1, raw2(raw_);
            raw1 = (bsl::var::IVar&)(raw2);
            assert( raw1.to_raw().data == raw_.data );
            assert( raw1.to_raw().length == raw_.length );
        }
        {
            bsl::var::raw_t raw_("", 1);
            bsl::var::ShallowRaw raw1, raw2(raw_);
            bsl::var::Ref ref = raw2;
            raw1 = raw2;
            assert( raw1.to_raw().data == raw_.data );
            assert( raw1.to_raw().length == raw_.length );
        }

    }

private:
};

int main(){
    TestVarShallowRaw test;
    test.test_all();
    return 0;
}

/* vim: set ts=4 sw=4 sts=4 tw=100 */
