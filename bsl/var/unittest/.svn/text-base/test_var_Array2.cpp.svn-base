/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: test_var_Array.cpp,v 1.4 2010/04/28 12:45:33 scmpf Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file test_VarArray.cpp
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2008/09/25 13:35:54
 * @version $Revision: 1.4 $ 
 * @brief 
 *  
 **/

#include "test_var_invalid.h"
#include <bsl/pool/bsl_poolalloc.h>
#include <bsl/pool/bsl_xmempool.h>
#include <bsl/pool/bsl_xcompool.h>
#include <bsl/pool/bsl_pool.h>
#include <bsl/exception.h>
#include <malloc.h>

#include <execinfo.h>
#include <typeinfo>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#if !(defined(__cplusplus) && (!defined(__GXX_ABI_VERSION) || __GXX_ABI_VERSION < 100))
    #include <cxxabi.h>
#endif

bsl::xcompool g_xcompool;

class TestVarArray: public ITestVar{
    
public:
    typedef bsl::var::IVar::string_type         string_type;
    typedef bsl::var::IVar::field_type          field_type;
    typedef bsl::var::ArrayIterator             array_iterator;
    typedef bsl::var::ArrayConstIterator        array_const_iterator;
    typedef bsl::var::Array::allocator_type     allocator_type;

    TestVarArray() {
        g_xcompool.create();
        allocator_type __alloc(&g_xcompool);
        _alloc = __alloc;
        bsl::var::Array __array(_alloc);
        _arr = __array;
    }

    virtual ~TestVarArray(){}
    
    //special methods
    virtual void test_special(){
        //mempool
        {
            char buff[10000];
            bsl::xmempool pool;
            pool.create(buff, sizeof(buff));
            bsl::var::Array::allocator_type pool_alloc( &pool );
            bsl::var::Array arr1( pool_alloc );
            bsl::var::Array arr2;
            bsl::var::Int32 i32(123);
            arr2[100] = i32;
            arr1 = arr2;
            assert( arr2.size() == arr2.size() );
            assert( arr2[100].to_int32() == 123 );

            bsl::var::Array arr3(arr1);
            assert( arr3.size() == arr1.size() );
            assert( arr3[100].to_int32() == 123 );

            bsl::var::Array arr4;
            arr4 = arr1;
            assert( arr4.size() == arr1.size() );
            assert( arr4[100].to_int32() == 123 );
            pool.clear();
        }
        
        //copy ctor
        {
            bsl::var::Array arr1( _alloc );
            bsl::var::Int32 i32(123);
            arr1[100] = i32;
            bsl::var::Array arr2(arr1);
            assert( arr2.size() == arr1.size() );
            assert( arr2[100].to_int32() == 123 );
        }
        
        //assign
        {
            bsl::var::Array arr1( _alloc );
            bsl::var::Int32 i32(123);
            arr1[100] = i32;
            bsl::var::Array arr2( _alloc );
            arr2 = arr1;
            assert( arr2.size() == arr1.size() );
            assert( arr2[100].to_int32() == 123 );
        }
                    
    }
    virtual void test_mask(){
        test_mask_consistency( bsl::var::Array() );
    }
    //methods for all
    virtual void test_size() {
        {
            assert( bsl::var::Array().size() == 0 );
        }
        {
            bsl::var::Array arr( _alloc );
            arr[100] = bsl::var::Null::null;
            assert( arr.size() == 101 );
        }

    }

    virtual void test_clear() {
        {
            bsl::var::Array arr( _alloc );
            arr.clear();    //assert no-throw
            assert(arr.size() == 0);
        }
    }

    virtual void test_dump() {
        stub();
    }

    virtual void test_to_string() {
        stub();
    }

    virtual void test_get_type() {
        assert( bsl::var::Array().get_type() == string_type("bsl::var::BasicArray") );
    }

    virtual void test_clone(){
        bsl::ResourcePool rp;
        bsl::var::Int32 i0(0),i1(1),i2(2);
        bsl::var::Array arr( allocator_type( &rp.get_mempool() ) );
        arr[0] = i0;
        arr[1] = i1;
        arr[2] = i2;
        bsl::var::Array& rep = arr.clone(rp);
        assert( rep.size() == arr.size() );
        assert( rep[0].to_int32() == i0.to_int32() );
        assert( rep[1].to_int32() == i1.to_int32() );
        assert( rep[2].to_int32() == i2.to_int32() );
    }

    virtual void test_bool(){
        test_invalid_bool(_arr);
    }

    virtual void test_raw(){
        test_invalid_raw(_arr);
    }

    virtual void test_number(){
        test_invalid_number(_arr);
    }

    virtual void test_string(){
        test_invalid_string(_arr);
    }

    virtual void test_array(){
        test_array_get();
        test_array_set();
        test_array_del();
        test_array_iterator();
        test_array_const_iterator();
        test_array_operator_square();
    }

    virtual void test_dict(){
        test_invalid_dict(_arr);
    }

    virtual void test_callable(){
        test_invalid_callable(_arr);
    }

    //methods for array
    virtual void test_array_get(){
        //normal get
        {
            assert( bsl::var::Array().get(0).is_null() );
        }
        {
            bsl::var::Array arr( _alloc );
            arr[0] = null;
            assert( arr.get(0).is_null() );
        }
        {
            bsl::var::Array arr( _alloc );
            bsl::var::Int32 i32 = 1949;
            arr[123] = i32;
            assert( arr.get(123).to_int32() == 1949 );
            assert( arr.get(122).is_null() );
            assert( arr.get(456).is_null() );
        }
        //geek get
        {
            bsl::var::Array arr( _alloc );
            bsl::var::Int32 i32 = 1234;
            arr[0] = i32;
            arr.get(0) = 4321;
            assert( arr[0].to_int32() == 4321 );
        }
        //get with default
        {
            bsl::var::Array arr( _alloc );
            bsl::var::Int32 i32 = 9394;
            bsl::var::Ref ref;
            bsl::var::Ref ref_i32 = i32;

            arr[9] = i32;
            arr[0] = null;

            assert( arr.get(9, ref).to_int32() == i32.to_int32() );
            assert( &bsl::var::Ref(arr.get(9, ref)).ref() == &i32 );
            assert( arr.get(0, ref).is_null() );
            assert( arr.get(9999, ref_i32).to_int32() == i32.to_int32() );
            assert( arr.get(99999, null).is_null() );
        }
    }

    virtual void test_array_set(){
        {
            bsl::var::Array arr( _alloc );
            arr.set(127, null );
            assert( arr.size() == 128 );
            assert( arr[127].is_null() );
        }
    }

    virtual void test_array_del(){
        {
            assert( bsl::var::Array().del(0) == false );
        }
    }

    virtual void test_array_iterator(){
        {
            bsl::var::Array arr( _alloc );
            //empty array:
            assert( arr.array_begin() == arr.array_end() );
        }
        {
            //iterators from different array are not equal
            assert( bsl::var::Array().array_begin() != bsl::var::Array().array_begin() );
            assert( bsl::var::Array().array_end() != bsl::var::Array().array_end() );
        }

        {
            bsl::ResourcePool rp;
            bsl::var::Array arr( _alloc );
            arr[0] = rp.create<bsl::var::Int32>(123);
            arr[2] = rp.create<bsl::var::Int32>(456);
            bsl::var::Array::array_iterator iter = arr.array_begin();

            //iter => arr[0]
            assert( iter == arr.array_begin() );
            assert( iter != arr.array_end() );
            assert( iter->key() == 0 );
            assert( iter->value().is_int32() );
            assert( iter->value().to_int32() == 123 );
            assert( (++ iter)->key() == 1 );

            //iter => arr[1]
            assert( iter != arr.array_end() );
            assert( iter->key() == 1 );
            assert( iter->value().is_null() );
            iter->value() = rp.create<bsl::var::Int32>(789);
            assert( arr[1].is_int32() );
            assert( arr[1].to_int32() == 789 );
            assert( (++ iter)->key() == 2 );

            //iter => arr[2]
            assert( iter != arr.array_end() );
            assert( iter->key() == 2 );
            assert( iter->value().is_int32() );
            assert( iter->value().to_int32() == 456 );
            assert( (++ iter) == arr.array_end() );

            //iter == arr.array_end()
            assert( iter == arr.array_end() );
        }

        {
            bsl::ResourcePool rp;
            bsl::var::Array arr( _alloc );
            bsl::var::Array::array_iterator iter = arr.array_begin();
            iter = arr.array_end();
            bsl::var::Array::array_iterator iter2(iter);
            assert( iter2 == arr.array_end() );
        }
    }

    virtual void test_array_const_iterator(){
        {
            const bsl::var::Array arr( _alloc );
            //empty array:
            assert( arr.array_begin() == arr.array_end() );
        }
        {
            //iterators from different array are not equal
            assert( bsl::var::Array().array_begin() != bsl::var::Array().array_begin() );
            assert( bsl::var::Array().array_end() != bsl::var::Array().array_end() );
        }

        {
            bsl::ResourcePool rp;
            bsl::var::Array arr1( _alloc );
            arr1[0] = rp.create<bsl::var::Int32>(123);
            arr1[2] = rp.create<bsl::var::Int32>(456);
            const bsl::var::Array& arr = arr1;
            bsl::var::Array::array_const_iterator iter = arr.array_begin();

            //iter => arr[0]
            assert( iter == arr.array_begin() );
            assert( iter != arr.array_end() );
            assert( iter->key() == 0 );
            assert( iter->value().is_int32() );
            assert( iter->value().to_int32() == 123 );
            assert( (++ iter)->key() == 1 );

            //iter => arr[1]
            assert( iter != arr.array_end() );
            assert( iter->key() == 1 );
            assert( iter->value().is_null() );
            assert( (++ iter)->key() == 2 );

            //iter => arr[2]
            assert( iter != arr.array_end() );
            assert( iter->key() == 2 );
            assert( iter->value().is_int32() );
            assert( iter->value().to_int32() == 456 );
            assert( (++ iter) == arr.array_end() );

            //iter == arr.array_end()
            assert( iter == arr.array_end() );
        }

    }

    virtual void test_array_operator_square(){
        bsl::ResourcePool rp;

        //non-const
        {
            bsl::var::Array arr( _alloc );
            arr[100] = rp.create<bsl::var::Int32>(123);
            assert( arr.size() == 101 );
            assert( arr[100].is_int32() );
            assert( arr[100].to_int32() == 123);
        }
        {
            bsl::var::Array arr( _alloc );
            assert( arr[456].is_ref() );
            assert( arr[789].is_null() );
        }

        //const
        {
            bsl::var::Array arr1( _alloc );
            arr1[100] = rp.create<bsl::var::Int32>(123);
            const bsl::var::Array& arr = arr1;
            assert( arr.size() == 101 );
            assert( arr[100].is_int32() );
            assert( arr[100].to_int32() == 123);

            assert( arr[0].is_null() );
        }

    }

    virtual void test_operator_assign(){
        //valid assignments
        {
            //prepare
            bsl::var::Array arr1( _alloc );
            bsl::var::Array arr2( _alloc );
            bsl::var::Int32 i32 = 123;
            arr1[0] = i32;

            //test
            arr2 = arr1;
            assert( arr2.size() == 1 );
            assert( arr2[0].to_int32() == 123 );

            arr2 = arr2;    //self assignment!
            assert( arr2.size() == 1 );
            assert( arr2[0].to_int32() == 123 );

        }
        //invalid assignments
        {
            bsl::var::Array arr( _alloc );
            bsl::var::Int32 i32 = 123;
            ASSERT_THROW( arr = 123, bsl::InvalidOperationException );
            ASSERT_THROW( arr = 123LL, bsl::InvalidOperationException );
            ASSERT_THROW( arr = 123.456, bsl::InvalidOperationException );
            ASSERT_THROW( arr = "123", bsl::InvalidOperationException );
            ASSERT_THROW( arr = i32, bsl::InvalidOperationException );
        }
    }

    virtual void test_operator_paren(){
        {
            bsl::var::Array args( _alloc );
            bsl::ResourcePool rp;
            ASSERT_THROW( bsl::var::Array()(args, rp), bsl::InvalidOperationException );
        }
        {
            bsl::var::Dict self;
            bsl::var::Array args;
            bsl::ResourcePool rp;
            ASSERT_THROW( bsl::var::Array()(self, args, rp), bsl::InvalidOperationException );
        }
    }
private:
    bsl::var::Null   null;
    bsl::var::Array  _arr;
    allocator_type   _alloc;
};


int main(){
    TestVarArray().test_all();
    return 0;
}
/* vim: set ts=4 sw=4 sts=4 tw=100 */
