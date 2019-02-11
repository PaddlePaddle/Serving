#include "bsl/var/implement.h"
#include "test_var_invalid.h"

class TestVarInt32: public ITestVar{
    
public:
    typedef bsl::string     string_type;
    typedef bsl::string     field_type;
    typedef bsl::var::ArrayIterator      array_iterator;
    typedef bsl::var::ArrayConstIterator array_const_iterator;

    TestVarInt32()
        :_i32(123){}

    virtual ~TestVarInt32(){}

    //special methods
    virtual void test_special(){
        //copy ctor
        {
            bsl::var::Int32 i = 123;
            bsl::var::Int32 j = i;
            assert( j.is_int32() );
            assert( j.to_int32() == 123 );
            i = 456;
            assert( j.to_int32() == 123 );
        }
        //copy assign
        {
            bsl::var::Int32 i = 123;
            bsl::var::Int32 j;
            j = i;
            assert( j.is_int32() );
            assert( j.to_int32() == 123 );
            i = 456;
            assert( j.to_int32() == 123 );
        }
    }
    
    //methods for all
    virtual void test_mask(){
        test_mask_consistency( bsl::var::Int32() );
    }

    virtual void test_operator_assign(){
        // = int
        {
            bsl::var::Int32 i;
            i = 123;
            assert( i.to_int32() == 123 );
            assert( i.to_int64() == 123 );
        }

        // = long long
        {

            bsl::var::Int32 i;
            ASSERT_THROW( i = 1234567891234567LL, bsl::OverflowException );
        }

        // = double
        {
            bsl::var::Int32 i;
            i = 4670.9394;
            assert( i.to_int32() == 4670 );
            assert( i.to_int64() == 4670 );
            assert( i.to_double() == 4670 );
        }

        // = const char *
        {
            bsl::var::Int32 i;
            i = "46709394";
            assert( i.to_int32() == 46709394 );
        }

        // = string_type
        {
            bsl::var::Int32 i;
            i = bsl::var::IVar::string_type("46709394");
            assert( i.to_int32() == 46709394 );
        }

        // = Int32
        {
            bsl::var::Int32 i, j;
            j = 123;
            i = j;
            assert( i.to_int32() == 123 );
            assert( i.to_int64() == 123 );
         }

        // = *this
        {
            bsl::var::Int32 i;
            i = 123;
            i = i;
            assert( i.to_int32() == 123 );
            assert( i.to_int64() == 123 );
         }
                
    }

    virtual void test_clear() {
        {
            bsl::var::Int32 i(123);
            i.clear();    //assert no-throw
            assert(i.to_int32() == 0);
        }
    }

    virtual void test_dump() {
        {
            bsl::var::Int32 i;
            assert( i.dump() == string_type("[bsl::var::Int32]0") );
            assert( i.dump(999) == string_type("[bsl::var::Int32]0") );
        }

        {
            bsl::var::Int32 i(1234567);
            assert( i.dump() == string_type("[bsl::var::Int32]1234567") );
            assert( i.dump(999) == string_type("[bsl::var::Int32]1234567") );
        }
    }

    virtual void test_to_string() {
        {
            bsl::var::Int32 i;
            assert( i.to_string() == string_type("0") );
        }

        {
            bsl::var::Int32 i(1234567);
            assert( i.to_string() == string_type("1234567") );
        }
    }

    virtual void test_get_type() {
        assert( bsl::var::Int32().get_type() == string_type("bsl::var::Int32") );
    }

    //method for value
    virtual void test_bool(){
        //= bool
        {
            bsl::var::Int32 i32(123);
            i32 = true;
            assert( i32.to_int32() == 1 );
        }
        {
            bsl::var::Int32 i32(123);
            i32 = false;
            assert( i32.to_int32() == 0 );
        }
        // to bool
        {
            assert( bsl::var::Int32().to_bool() == false );
            assert( bsl::var::Int32(-1).to_bool() == true );
        }
    }
    virtual void test_raw(){
        test_invalid_raw(_i32);
    }
    virtual void test_number(){
        test_to_int32();
        test_to_int64();
        test_to_double();

        test_valid_number( bsl::var::Int32(INT_MIN), static_cast<int>(INT_MIN) );
        test_valid_number( bsl::var::Int32(INT_MAX), static_cast<int>(INT_MAX) );
    }

    virtual void test_clone(){
        bsl::ResourcePool rp;
        bsl::var::Int32 i32;
        assert( i32.clone(rp).get_type() == i32.get_type() );
        assert( i32.clone(rp).to_int32() == i32.to_int32() );

        test_valid_number( bsl::var::Int32(INT_MIN).clone(rp), static_cast<int>(INT_MIN) );
        test_valid_number( bsl::var::Int32(INT_MAX).clone(rp), static_cast<int>(INT_MAX) );
    }
    virtual void test_string(){
        test_invalid_string(_i32);
    }

    //methods for array
    virtual void test_array(){
        test_invalid_array(_i32);
    }

    //methods for dict
    virtual void test_dict(){
        test_invalid_dict(_i32);
    }

    //methods for callable
    virtual void test_callable(){
        test_invalid_callable(_i32);
    }

    virtual void test_to_int32(){
        {
            assert( bsl::var::Int32().to_int32() == 0 );
            assert( bsl::var::Int32(-1).to_int32() == -1 );
            assert( bsl::var::Int32(1234567).to_int32() == 1234567 );
        }
    }

    virtual void test_to_int64(){
        {
            assert( bsl::var::Int32().to_int64() == 0 );
            assert( bsl::var::Int32(-1).to_int64() == -1 );
            assert( bsl::var::Int32(1234567).to_int64() == 1234567 );
        }
    }

    virtual void test_to_double(){
        {
            assert( bsl::var::Int32().to_double() == 0 );
            assert( bsl::var::Int32(-1).to_double() == -1 );
            assert( bsl::var::Int32(1234567).to_double() == 1234567 );
        }
    }

private:
    bsl::var::Int32 _i32;
};

int main(){
    TestVarInt32().test_all();
    return 0;
}


