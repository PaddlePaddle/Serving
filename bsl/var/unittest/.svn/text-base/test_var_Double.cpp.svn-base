/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: test_var_Double.cpp,v 1.4 2010/04/28 12:45:33 scmpf Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file test_VarDouble.cpp
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2008/09/29 17:33:45
 * @version $Revision: 1.4 $ 
 * @brief 
 *  
 **/
#include "bsl/var/Double.h"
#include "test_var_invalid.h"

class TestVarDouble: public ITestVar{

public:
    typedef bsl::string     string_type;
    typedef bsl::string     field_type;
    typedef bsl::var::ArrayIterator      array_iterator;
    typedef bsl::var::ArrayConstIterator array_const_iterator;

    TestVarDouble()
        :_dbl(2008.0512){}

    virtual ~TestVarDouble(){}

    //special methods
    virtual void test_special(){
        //copy ctor
        {
            bsl::var::Double var = 123456789.1234567;
            bsl::var::Double j = var;
            assert( j.is_double() );
            assert( j.to_double() == 123456789.1234567 );
            var = 987654321.098765;
            assert( j.to_double() == 123456789.1234567 );
        }
        //copy assign
        {
            bsl::var::Double var = 123456789.1234567;
            bsl::var::Double j;
            j = var;
            assert( j.is_double() );
            assert( j.to_double() == 123456789.1234567 );
            var = 987654321.098765;
            assert( j.to_double() == 123456789.1234567 );
        }
    }

    //methods for all
    virtual void test_mask(){
        test_mask_consistency( bsl::var::Double() );
    }

    virtual void test_operator_assign(){
        // = int
        {
            bsl::var::Double var;
            var = 123;
            assert( var.to_int32() == 123 );
            assert( var.to_int64() == 123 );
        }
        // = long long
        {
            bsl::var::Double var;
            var = 1234567891234567LL;
            assert( var.to_double() == double(1234567891234567LL) );
            assert( var.to_int64() == 1234567891234567LL );
        }
        // = double
        {
            bsl::var::Double var;
            var = 12345678904670.9394;
            assert( var.to_double() ==12345678904670.9394 );
        }
        // = const char *
        {
            bsl::var::Double var;
            var = "1234567.46709394";
            assert( var.to_double() == 1234567.46709394 );
        }

        // = string_type
        {
            bsl::var::Double var;
            var = bsl::var::IVar::string_type("1234567.46709394");
            assert( var.to_double() == 1234567.46709394 );
        }

        // = Int32
        {
            bsl::var::Int32 j;
            bsl::var::Double var;
            j = 123;
            var = j;
            assert( var.to_int32() == 123 );
            assert( var.to_int64() == 123 );
        }

        // = Double
        {
            bsl::var::Double var, j;
            j = 123456789.1234567;
            var = j;
            assert( var.to_double() == 123456789.1234567 );
        }

        // = *this
        {
            bsl::var::Double var;
            var = 123;
            var = var;
            assert( var.to_double() == 123 );
        }

    }

    virtual void test_clear() {
        {
            bsl::var::Double var(123456.7891234);
            var.clear();    //assert no-throw
            assert(var.to_int64() == 0);
        }
    }

    virtual void test_dump() {
        {
            bsl::var::Double var;
            assert( var.dump() == string_type("[bsl::var::Double]0") );
            assert( var.dump(999) == string_type("[bsl::var::Double]0") );
        }

        {
            bsl::var::Double var(123.45);
            assert( var.dump() == string_type("[bsl::var::Double]123.45") );
            assert( var.dump(999) == string_type("[bsl::var::Double]123.45") );
        }

        {
            bsl::var::Double var(-1e-100);
            assert( var.dump() == string_type("[bsl::var::Double]-1e-100") );
            assert( var.dump(999) == string_type("[bsl::var::Double]-1e-100") );
        }

    }

    virtual void test_to_string() {
        {
            bsl::var::Double var;
            assert( var.to_string() == string_type("0") );
        }

        {
            bsl::var::Double var(123.45);
            assert( var.to_string() == string_type("123.45") );
        }

        {
            bsl::var::Double var(-1e-100);
            assert( var.to_string() == string_type("-1e-100") );
        }

    }

    virtual void test_get_type() {
        assert( bsl::var::Double().get_type() == string_type("bsl::var::Double") );
    }

    //method for value
    virtual void test_bool(){
        //= bool
        {
            bsl::var::Double dbl(123);
            dbl = true;
            assert( dbl.to_double() == 1 );
        }
        {
            bsl::var::Double dbl(123);
            dbl = false;
            assert( dbl.to_double() == 0 );
        }
        // to bool
        {
            assert( bsl::var::Double().to_bool() == false );
            assert( bsl::var::Double(-1).to_bool() == true );
        }
    }

    virtual void test_raw(){
        test_invalid_raw(_dbl);
    }

    virtual void test_number(){
        test_to_int32();
        test_to_int64();
        test_to_double();

        test_valid_number( bsl::var::Double(-1), double(-1.0) );
        test_valid_number( bsl::var::Double(0), double(0.0) );
        test_valid_number( bsl::var::Double(double(LLONG_MAX)*double(LLONG_MAX)), double(LLONG_MAX)*double(LLONG_MAX) );
        test_valid_number( bsl::var::Double(-double(LLONG_MAX)*double(LLONG_MAX)), -double(LLONG_MAX)*double(LLONG_MAX) );
    }

    virtual void test_clone(){
        bsl::ResourcePool rp;
        bsl::var::Double v(123);
        assert(v.clone(rp).to_string() == v.to_string() );
        assert(v.clone(rp).get_type() == v.get_type() );

        test_valid_number( bsl::var::Double(-1).clone(rp), double(-1.0) );
        test_valid_number( bsl::var::Double(0).clone(rp), double(0.0) );
        test_valid_number( bsl::var::Double(double(LLONG_MAX)*double(LLONG_MAX)).clone(rp), double(LLONG_MAX)*double(LLONG_MAX) );
        test_valid_number( bsl::var::Double(-double(LLONG_MAX)*double(LLONG_MAX)).clone(rp), -double(LLONG_MAX)*double(LLONG_MAX) );
    }

    virtual void test_string(){
        test_invalid_string(_dbl);
    }

    //methods for array
    virtual void test_array(){
        test_invalid_array(_dbl);
    }

    //methods for dict
    virtual void test_dict(){
        test_invalid_dict(_dbl);
    }

    //methods for callable
    virtual void test_callable(){
        test_invalid_callable(_dbl);
    }

    virtual void test_to_int32(){
        {
            assert( bsl::var::Double().to_int32() == 0 );
            assert( bsl::var::Double(-1).to_int32() == -1 );
            assert( bsl::var::Double(1234567).to_int32() == 1234567 );
        }
    }

    virtual void test_to_int64(){
        {
            assert( bsl::var::Double().to_int64() == 0 );
            assert( bsl::var::Double(-1).to_int64() == -1 );
            assert( bsl::var::Double(1234567).to_int64() == 1234567 );
        }
    }

    virtual void test_to_double(){
        {
            assert( bsl::var::Double().to_double() == 0 );
            assert( bsl::var::Double(-1.2345).to_double() == -1.2345 );
            assert( bsl::var::Double(1234567.3435).to_double() == 1234567.3435 );
        }
    }

private:
    bsl::var::Double _dbl;
};

int main(){
    TestVarDouble().test_all();
    return 0;
}


/* vim: set ts=4 sw=4 sts=4 tw=100 */
