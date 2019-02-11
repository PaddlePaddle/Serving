/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: test_var_Function.cpp,v 1.4 2010/04/28 12:45:33 scmpf Exp $ 
 * 
 **************************************************************************/



/**
 * @file test_var_Function.cpp
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2008/09/24 20:57:41
 * @version $Revision: 1.4 $ 
 * @brief 
 *  
 **/
#include "test_var_invalid.h"

bsl::var::IVar& echo( bsl::var::IVar& args, bsl::ResourcePool& ){
    return args;
}

bsl::var::IVar& join( bsl::var::IVar& args, bsl::ResourcePool& rp ){
    bsl::var::IVar::string_type res;
    for( size_t i = 0; i < args.size(); ++ i ){
        res.append(args[int(i)].to_string());
    }
    return rp.create<bsl::var::String>(res);
}

class TestVarFunction: public ITestVar{
public:
    typedef bsl::var::IVar::string_type   string_type;
    typedef bsl::var::IVar::field_type    field_type;
    typedef bsl::var::ArrayIterator      array_iterator;
    typedef bsl::var::ArrayConstIterator array_const_iterator;

    TestVarFunction()
        :_func(echo, "echo"){}

    virtual ~TestVarFunction(){}
    
    //speical methods
    virtual void test_special(){
        // ctors
        {
            bsl::var::Function func1(&echo, "echo");
            assert( func1.to_string() == "echo" );
            bsl::var::Function func2(func1);
            assert( func2.to_string() == "echo" );
        }
        // copy assign
        {
            bsl::var::Function func1(&echo, "echo");
            bsl::var::Function func2(&join, "join");
            func1 = func2;
            assert( func1.to_string() == "join" );
        }
    }
    //methods for all
    virtual void test_mask(){
        test_mask_consistency( bsl::var::Function(&echo, "echo") );
    }


    virtual void test_clear() {
        {
            bsl::var::Function func1(&echo, "echo");
            func1.clear(); //no effect
            assert( func1.to_string() == "echo" );
        }
    }

    virtual void test_dump() {
        {
            bsl::var::Function func(echo, "echo");
            assert( func.dump() == "[bsl::var::Function]echo" );
            assert( func.dump(999) == "[bsl::var::Function]echo" );
        }

    }

    virtual void test_to_string() {
        {
            bsl::var::Function func(echo, "echo");
            assert( func.to_string() == "echo" );
        }

    }

    virtual void test_get_type() {
        assert( bsl::var::Function(&echo, "echo").get_type() == string_type("bsl::var::Function") );
    }

    //method for value
    virtual void test_bool(){
        test_invalid_bool(_func);
    }
    virtual void test_raw(){
        test_invalid_raw(_func);
    }
    virtual void test_number(){
        test_invalid_number(_func);
    }
    virtual void test_clone(){
        bsl::ResourcePool rp;
        bsl::var::Function f(&echo, "echo");
        bsl::var::Function& cl = f.clone(rp);
        bsl::var::MagicRef args(rp);
        args[0] = 123;
        args[1] = "hello";
        assert( cl.get_type() == string_type("bsl::var::Function") );
        assert( cl.to_string() == string_type("echo") );
        bsl::var::IVar& res = cl(args, rp);
        assert( res[0].to_int32() == 123 );
        assert( res[1].c_str() == args[1].c_str() );
    }
        
    virtual void test_string(){
        test_invalid_string(_func);
    }

    //methods for array
    virtual void test_array(){
        test_invalid_array(_func);
    }

    //methods for dict
    virtual void test_dict(){
        test_invalid_dict(_func);
    }

    //methods for callable
    virtual void test_callable(){
        test_operator_paren();
    }

    virtual void test_operator_assign(){
        //valid assign
        {
            bsl::var::Function func1(&echo, "echo"), func2(&join, "join");
            bsl::var::IVar& ivar = func1;
            func2 = ivar;
            assert( func2.to_string() == func1.to_string() );

        }
        //invalid assign
        {
            bsl::var::Function func(&echo, "echo");
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
            bsl::var::Array args;
            bsl::ResourcePool rp;
            bsl::var::IVar& res = bsl::var::Function(&echo, "echo")(args, rp);
            assert( &res == &args );
        }
        {
            bsl::ResourcePool rp;
            bsl::var::Array args;
            bsl::var::Int32 i32(123);
            bsl::var::String str("hao");
            args[0] = str;
            args[1] = i32;
            bsl::var::IVar& res = bsl::var::Function(&join, "join")(args, rp);
            assert( res.to_string() == "hao123" );
        }
        {
            bsl::ResourcePool rp;
            bsl::var::Dict self;
            bsl::var::Array args;
            bsl::var::Int32 i32(123);
            bsl::var::String str("hao");
            args[0] = str;
            args[1] = i32;
            bsl::var::IVar& res = bsl::var::Function(&join, "join")(self, args, rp);
            assert( res.to_string() == "hao123" );
        }
    }
private:
    bsl::var::Function _func;
};

int main(){
    TestVarFunction test;
    test.test_all();
    return 0;
}

/* vim: set ts=4 sw=4 sts=4 tw=100 */
