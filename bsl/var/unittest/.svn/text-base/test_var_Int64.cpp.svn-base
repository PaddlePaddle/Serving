#include "bsl/var/Int64.h"
#include "test_var_invalid.h"

class TestVarInt64: public ITestVar{

public:
    typedef bsl::string     string_type;
    typedef bsl::string     field_type;
    typedef bsl::var::ArrayIterator      array_iterator;
    typedef bsl::var::ArrayConstIterator array_const_iterator;
    TestVarInt64()
        :_i64(1234567891234567LL){}

    virtual ~TestVarInt64(){}

    //special methods
    virtual void test_special(){
        //copy ctor
        {
            bsl::var::Int64 i = 1234567891234567LL;
            bsl::var::Int64 j = i;
            assert( j.is_int64() );
            assert( j.to_int64() == 1234567891234567LL );
            i = 987654321098765LL;
            assert( j.to_int64() == 1234567891234567LL );
        }
        //copy assign
        {
            bsl::var::Int64 i = 1234567891234567LL;
            bsl::var::Int64 j;
            j = i;
            assert( j.is_int64() );
            assert( j.to_int64() == 1234567891234567LL );
            i = 987654321098765LL;
            assert( j.to_int64() == 1234567891234567LL );
        }
    }

    //methods for all
    virtual void test_mask(){
        test_mask_consistency( bsl::var::Int64() );
    }

    virtual void test_operator_assign(){
        // = int
        {
            bsl::var::Int64 i;
            i = 123;
            assert( i.to_int32() == 123 );
            assert( i.to_int64() == 123 );
        }
        // = long long
        {
            bsl::var::Int64 i;
            i = 1234567891234567LL;
            ASSERT_THROW( i.to_int32(), bsl::OverflowException );
            assert( i.to_int64() == 1234567891234567LL );
            assert( i.to_double() == double(1234567891234567LL) );
        }
        // = double
        {
            bsl::var::Int64 i;
            i = 12345678904670.9394;
            ASSERT_THROW( i.to_int32(), bsl::OverflowException );
            assert( i.to_int64() == (long long)(12345678904670.9394) );
            assert( i.to_double() ==(long long)(12345678904670.9394) );
        }

        // = const char *
        {
            bsl::var::Int64 i;
            i = "123456746709394";
            assert( i.to_int64() == 123456746709394LL );
        }

        // = string_type
        {
            bsl::var::Int64 i;
            i = bsl::var::IVar::string_type("123456746709394");
            assert( i.to_int64() == 123456746709394LL );
        }

        // = Int32
        {
            bsl::var::Int32 j;
            bsl::var::Int64 i;
            j = 123;
            i = j;
            assert( i.to_int32() == 123 );
            assert( i.to_int64() == 123 );
        }

        // = Int64
        {
            bsl::var::Int64 i, j;
            j = 1234567891234567LL;
            i = j;
            ASSERT_THROW( i.to_int32(), bsl::OverflowException );
            assert( i.to_int64() == 1234567891234567LL );
        }

        // = *this
        {
            bsl::var::Int64 i;
            i = 123;
            i = i;
            assert( i.to_int32() == 123 );
            assert( i.to_int64() == 123 );
        }

    }

    virtual void test_clear() {
        {
            bsl::var::Int64 i(1234567891234LL);
            i.clear();    //assert no-throw
            assert(i.to_int64() == 0);
        }
    }

    virtual void test_dump() {
        {
            bsl::var::Int64 i;
            assert( i.dump() == string_type("[bsl::var::Int64]0") );
            assert( i.dump(999) == string_type("[bsl::var::Int64]0") );
        }

        {
            bsl::var::Int64 i(1234567891234567LL);
            assert( i.dump() == string_type("[bsl::var::Int64]1234567891234567") );
            assert( i.dump(999) == string_type("[bsl::var::Int64]1234567891234567") );
        }

    }

    virtual void test_to_string() {
        {
            bsl::var::Int64 i;
            assert( i.to_string() == string_type("0") );
        }

        {
            bsl::var::Int64 i(1234567891234567LL);
            assert( i.to_string() == string_type("1234567891234567") );
        }

    }

    virtual void test_get_type() {
        assert( bsl::var::Int64().get_type() == string_type("bsl::var::Int64") );
    }

    //method for value
    virtual void test_bool(){
        //= bool
        {
            bsl::var::Int64 i64(123);
            i64 = true;
            assert( i64.to_int64() == 1 );
        }
        {
            bsl::var::Int64 i64(123);
            i64 = false;
            assert( i64.to_int64() == 0 );
        }
        // to bool
        {
            assert( bsl::var::Int64().to_bool() == false );
            assert( bsl::var::Int64(-1).to_bool() == true );
        }
    }

    virtual void test_raw(){
        test_invalid_raw(_i64);
    }

    virtual void test_number(){
        test_to_int32();
        test_to_int64();
        test_to_double();

        test_valid_number( bsl::var::Int64(-1), -1LL );
        test_valid_number( bsl::var::Int64(0), 0LL );
        test_valid_number( bsl::var::Int64(LLONG_MIN), LLONG_MIN );
        test_valid_number( bsl::var::Int64(LLONG_MAX), LLONG_MAX );
    }

    virtual void test_clone(){
        bsl::ResourcePool rp;
        bsl::var::Int64 v(123);
        assert(v.clone(rp).to_string() == v.to_string() );
        assert(v.clone(rp).get_type() == v.get_type() );

        test_valid_number( bsl::var::Int64(-1).clone(rp), -1LL );
        test_valid_number( bsl::var::Int64(0).clone(rp), 0LL );
        test_valid_number( bsl::var::Int64(LLONG_MIN).clone(rp), LLONG_MIN );
        test_valid_number( bsl::var::Int64(LLONG_MAX).clone(rp), LLONG_MAX );
    }

    virtual void test_string(){
        test_invalid_string(_i64);
    }

    //methods for array
    virtual void test_array(){
        test_invalid_array(_i64);
    }

    //methods for dict
    virtual void test_dict(){
        test_invalid_dict(_i64);
    }

    //methods for callable
    virtual void test_callable(){
        test_invalid_callable(_i64);
    }


    virtual void test_to_int32(){
        {
            assert( bsl::var::Int64().to_int32() == 0 );
            assert( bsl::var::Int64(-1).to_int32() == -1 );
            assert( bsl::var::Int64(1234567).to_int32() == 1234567 );
        }
    }

    virtual void test_to_int64(){
        {
            assert( bsl::var::Int64().to_int64() == 0 );
            assert( bsl::var::Int64(-1).to_int64() == -1 );
            assert( bsl::var::Int64(1234567).to_int64() == 1234567 );
        }
    }

    virtual void test_to_double(){
        {
            assert( bsl::var::Int64().to_double() == 0 );
            assert( bsl::var::Int64(-1).to_double() == -1 );
            assert( bsl::var::Int64(1234567).to_double() == 1234567 );
        }
    }

private:
    bsl::var::Int64 _i64;
};

int main(){
    TestVarInt64().test_all();
    return 0;
}


