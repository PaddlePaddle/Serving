/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: test_var_Method.cpp,v 1.4 2010/04/28 12:45:33 scmpf Exp $ 
 * 
 **************************************************************************/



/**
 * @file test_var_Method.cpp
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2008/09/24 20:57:41
 * @version $Revision: 1.4 $ 
 * @brief 
 *  
 **/
#include "test_var_invalid.h"

bsl::var::IVar& echo( bsl::var::IVar& self, bsl::var::IVar& args, bsl::ResourcePool& rp){
    bsl::var::IVar& res = rp.create<bsl::var::Dict>();
    res["self"] = self;
    res["args"] = args;
    return res;
}

bsl::var::IVar& size( bsl::var::IVar& self, bsl::var::IVar& , bsl::ResourcePool& rp ){
    return rp.create<bsl::var::Int32>(int(self.size()));
}

class TestVarMethod: public ITestVar{
    
public:
    typedef bsl::var::IVar::string_type   string_type;
    typedef bsl::var::IVar::field_type    field_type;
    typedef bsl::var::ArrayIterator      array_iterator;
    typedef bsl::var::ArrayConstIterator array_const_iterator;

    TestVarMethod()
        :_method(echo, "method"){}

    virtual ~TestVarMethod(){}
    
    //speical methods
    virtual void test_special(){
        // ctors
        {
            bsl::var::Method func1(&echo, "echo");
            assert( func1.to_string() == "echo" );
            bsl::var::Method func2(func1);
            assert( func2.to_string() == "echo" );
        }
        // copy assign
        {
            bsl::var::Method func1(&echo, "echo");
            bsl::var::Method func2(&size, "size");
            func1 = func2;
            assert( func1.to_string() == "size" );
        }
    }
    //methods for all
    virtual void test_mask(){
        test_mask_consistency( bsl::var::Method(&echo, "echo") );
    }

    virtual void test_clear() {
        {
            bsl::var::Method func1(&echo, "echo");
            func1.clear(); //no effect
            assert( func1.to_string() == "echo" );
        }
    }

    virtual void test_dump() {
        {
            bsl::var::Method func(echo, "echo");
            assert( func.dump() == "[bsl::var::Method]echo" );
            assert( func.dump(999) == "[bsl::var::Method]echo" );
        }

    }

    virtual void test_to_string() {
        {
            bsl::var::Method func(echo, "echo");
            assert( func.to_string() == "echo" );
        }

    }

    virtual void test_get_type() {
        assert( bsl::var::Method(&echo, "echo").get_type() == string_type("bsl::var::Method") );
    }

    //method for value
    virtual void test_bool(){
        test_invalid_bool(_method);
    }
    virtual void test_raw(){
        test_invalid_raw(_method);
    }
    virtual void test_number(){
        test_invalid_number(_method);
    }
    virtual void test_clone(){
        bsl::ResourcePool rp;
        bsl::var::Method f(&echo, "echo");
        bsl::var::Method& cl = f.clone(rp);
        bsl::var::MagicRef args(rp);
        bsl::var::Dict self;
        args[0] = 123;
        args[1] = "hello";
        assert( cl.get_type() == string_type("bsl::var::Method") );
        assert( cl.to_string() == string_type("echo") );
        bsl::var::IVar& res = cl(self, args, rp);
        assert( res["args"][0].to_int32() == 123 );
        assert( res["args"][1].c_str() == args[1].c_str() );
        assert( res["self"].is_dict() );
    }
    virtual void test_string(){
        test_invalid_string(_method);
    }

    //methods for array
    virtual void test_array(){
        test_invalid_array(_method);
    }

    //methods for dict
    virtual void test_dict(){
        test_invalid_dict(_method);
    }

    //methods for callable
    virtual void test_callable(){
        test_operator_paren();
    }

    virtual void test_operator_assign(){
        //valid assign
        {
            bsl::var::Method func1(&echo, "echo"), func2(&size, "size");
            bsl::var::IVar& ivar = func1;
            func2 = ivar;
            assert( func2.to_string() == func1.to_string() );

        }
        //invalid assign
        {
            bsl::var::Method func(&echo, "echo");
            bsl::var::Int32 i32;
            ASSERT_THROW( func = 123, bsl::InvalidOperationException );
            ASSERT_THROW( func = 123LL, bsl::InvalidOperationException );
            ASSERT_THROW( func = 123.456, bsl::InvalidOperationException );
            ASSERT_THROW( func = "123", bsl::InvalidOperationException );
            ASSERT_THROW( func = i32, bsl::InvalidOperationException );
        }
    }

    virtual void test_operator_paren(){
        {
            bsl::var::Dict self;
            bsl::var::Array args;
            bsl::ResourcePool rp;
            bsl::var::IVar& res = bsl::var::Method(&echo, "echo")(self, args, rp);
            assert( &bsl::var::Ref(res["self"]).ref() == &self );
            assert( &bsl::var::Ref(res["args"]).ref() == &args );
        }
        {
            bsl::var::Array args;
            bsl::ResourcePool rp;
            bsl::var::IVar& res = bsl::var::Method(&echo, "echo")(args, rp);
            assert( res["self"].is_null() );
            assert( &bsl::var::Ref(res["args"]).ref() == &args );
        }
        {
            bsl::ResourcePool rp;
            bsl::var::Dict self;
            bsl::var::Array args;
            bsl::var::Int32 i32(123);
            bsl::var::String str("hao");
            self["str"] = str;
            self["i32"] = i32;
            bsl::var::IVar& res = bsl::var::Method(&size, "size")(self, args, rp);
            assert( res.to_int32() == 2 );
        }
    }
private:
    bsl::var::Method _method;
};

int main(){
    TestVarMethod test;
    test.test_all();
    return 0;
}

/* vim: set ts=4 sw=4 sts=4 tw=100 */
