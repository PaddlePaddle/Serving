#include <stdio.h>
#include <string.h>
#include "Bool.h"
#include "Int32.h"
#include "Int64.h"
#include "String.h"
#include "BigInt.h"
#include <gtest/gtest.h>

#define SIGNED true
#define UNSIGNED false

using namespace bsl::var;

int main(int argc, char **argv)
{
 	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
/**
 * @brief 
**/
class test_bigint_suite : public ::testing::Test{
    protected:
        test_bigint_suite(){};
        virtual ~test_bigint_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_bigint_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_bigint_suite, *)
        };
};

/**
 * @brief 
 * @begin_version 
**/
TEST_F(test_bigint_suite, case_ivar_is_bigint)
{
	//TODO
	BigIntk b;
	IVar & i = b;
	EXPECT_EQ(i.is_bigint(), true);
}


/**
 * @brief 
 * @begin_version 
**/
TEST_F(test_bigint_suite, case_ivar_string)
{
	//TODO
	bsl::string	original = "0xf234567890abcdef";
	{
		int exception_flag = 0;
		try{
			BigInt<128, UNSIGNED> a = (const char *)NULL;
		}
		catch(...){
			exception_flag = 1;
		}
		EXPECT_EQ(exception_flag, 1);
	}
	{
		// copy contructor and assignment operator
		BigInt<128, SIGNED> a = original;
		bsl::string conv = a.to_string();
		EXPECT_EQ(strcmp(conv.c_str(), original.c_str()), 0);
	}
	{
		BigInt<128, UNSIGNED> a = original;
		bsl::string conv = a.to_string();
		EXPECT_EQ(strcmp(conv.c_str(), original.c_str()), 0);
	}
	{
		String orig = original;
		BigInt<128, SIGNED> a = orig.to_bigint<128, SIGNED>();
		String conv;
		conv = a;
		EXPECT_EQ(strcmp(conv.c_str(), original.c_str()), 0);
	}
	{
		String orig = original;
		int exception_flag = 0;
		try{
			BigInt<128, UNSIGNED> a = orig.to_bigint<128, SIGNED>();
		}
		catch(...){
			exception_flag = 1;
		}
		EXPECT_EQ(exception_flag, 1);
	}
}

/**
 * @brief 
 * @begin_version 
**/
TEST_F(test_bigint_suite, case_ivar_bool)
{
	//TODO
	BigInt<1, SIGNED> a = true;		// copy contructor
	EXPECT_EQ(a.is_overflow(), true);
	
	Bool b = a.to_bool();
	EXPECT_EQ(b.to_bool(), true);
	
	a = false;	// assignment operator
	EXPECT_EQ(a.to_bool(), false);
	
	Bool c;
	c = a;
	EXPECT_EQ(c.to_bool(), false);

	a = c.to_bigint<1, SIGNED>();
	EXPECT_EQ(a.to_bool(), false);

	{
		BigIntk b = 0x00000001;
		EXPECT_EQ(b.to_bool(), true);
		b = 0x00000000;
		EXPECT_EQ(b.to_bool(), false);
		b = "0x00000000";
		EXPECT_EQ(b.to_bool(), false);
	}
}

/**
 * @brief 
 * @begin_version 
**/
TEST_F(test_bigint_suite, case_int32)
{
	//TODO
	{
		int a = 0xffa;
		BigIntk b = a;
		EXPECT_EQ(b.to_int32(), a);
		EXPECT_EQ(b.to_bool(), true);
	}
	{
		int a = -123;
		BigIntk b = a;
		EXPECT_EQ(b.to_int32(), -123);
	}
	{
		Int32 c = 0xffffffff;
		BigIntk b = c.to_bigint<1024, SIGNED>();
		EXPECT_EQ(b.is_overflow(), false);
		EXPECT_EQ(strcmp(b.to_string().c_str(), "0xffffffff"), 0);
		c = b;
		EXPECT_EQ(c.to_int32(), (signed int)0xffffffff);
	}
	{
		int a = -123;
		int exception_flag = 0;
		try{
			BigInt<1024, UNSIGNED> b;
			b = a;
		}
		catch(...){
			exception_flag = 1;
		}
		EXPECT_EQ(exception_flag, 1);
	}
	{
		Int32 a = 0x7fffffff;
		int exception_flag = 0;
		try{
			BigInt<1024, UNSIGNED> b;
			b = a.to_bigint<1024, UNSIGNED>();
		}
		catch(...){
			exception_flag = 1;
		}
		EXPECT_EQ(exception_flag, 0);
	  
		a = (signed int)0xffffffff;
		exception_flag = 0;
		try{
			BigInt<1024, UNSIGNED> b;
			b = a.to_bigint<1024, UNSIGNED>();
		}
		catch(...){
			exception_flag = 1;
		}
		EXPECT_EQ(exception_flag, 1);
	}
}

/**
 * @brief 
 * @begin_version 
**/
TEST_F(test_bigint_suite, case_int64)
{
	//TODO
	{
		long long a = -123;
		BigIntk b = a;
		EXPECT_EQ(b.to_int64(), -123);
	}
	{
		Int64 c = 0xffffffffffffffff;
		BigIntk b = c.to_bigint<1024, SIGNED>();
		EXPECT_EQ(b.is_overflow(), false);
		EXPECT_EQ(strcmp(b.to_string().c_str(), "0xffffffffffffffff"), 0);
		c = b;
		EXPECT_EQ(c.to_int64(), (signed long long)0xffffffffffffffff);
	}
	{
		BigInt<1024, UNSIGNED> b = "0xffffffffffffffff";
		EXPECT_EQ(b.is_overflow(), false);
		EXPECT_EQ(b.to_uint64(), (unsigned long long)0xffffffffffffffff);
	}
	{
		long long a = -123;
		int exception_flag = 0;
		try{
			BigInt<1024, UNSIGNED> b;
			b = a;
		}
		catch(...){
			exception_flag = 1;
		}
		EXPECT_EQ(exception_flag, 1);
	}
	{
		Int64 a = 0x7fffffffffffffff;
		int exception_flag = 0;
		try{
			BigInt<1024, UNSIGNED> b;
			b = a.to_bigint<1024, UNSIGNED>();
		}
		catch(...){
			exception_flag = 1;
		}
		EXPECT_EQ(exception_flag, 0);
	  
		a = (signed long long)0xffffffffffffffff;
		exception_flag = 0;
		try{
			BigInt<1024, UNSIGNED> b;
			b = a.to_bigint<1024, UNSIGNED>();
		}
		catch(...){
			exception_flag = 1;
		}
		EXPECT_EQ(exception_flag, 1);
	}
}

/**
 * @brief float and double conversion, not supported
 * @begin_version 
**/
TEST_F(test_bigint_suite, case_float_double)
{
	//TODO
	{
		float a = 3.14;
		int exception_flag = 0;
		try{
			BigInt<1024, UNSIGNED> b;
			b = a;
		}
		catch(...){
			exception_flag = 1;
		}
		EXPECT_EQ(exception_flag, 1);
	}
	{
		double a = 3.14;
		int exception_flag = 0;
		try{
			BigInt<1024, UNSIGNED> b;
			b = a;
		}
		catch(...){
			exception_flag = 1;
		}
		EXPECT_EQ(exception_flag, 1);
	}
}
 
/**
 * @brief 
 * @begin_version 
**/
TEST_F(test_bigint_suite, case_overflow)
{
	//TODO
	{
		BigInt<32, UNSIGNED> b;
		b = "0xffffffff";
		EXPECT_EQ(b.is_overflow(), false);

		bsl::var::BigInt<32, SIGNED> c;
		c = "0xffffffff";
		EXPECT_EQ(c.is_overflow(), false);
	}
	{
		BigInt<32, UNSIGNED> b;
		b = "0xfffffffff";
		EXPECT_EQ(b.is_overflow(), true);

		bsl::var::BigInt<32, SIGNED> c;
		c = "0xfffffffff";
		EXPECT_EQ(c.is_overflow(), true);
	}
}
 
/**
 * @brief 
 * @begin_version 
**/
TEST_F(test_bigint_suite, case_resourcepool)
{
	//TODO
	{
		BigInt<128, UNSIGNED> b;
		b = "0xffffffff";

		bsl::ResourcePool rp;
		BigInt<128, UNSIGNED> & c = b.clone(rp);
		EXPECT_EQ(strcmp(b.to_string().c_str(), c.to_string().c_str()), 0);
	}

}

