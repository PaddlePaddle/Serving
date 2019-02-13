#include <stdlib.h>
#include <Configure.h>
#include <gtest/gtest.h>
//#include "compiler/src/idl_conf_if.h"
#include "constraint/cc_default.h"
#include "constraint/ConstraintFunction.h"
	
int main(int argc, char **argv)
{
 	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
/**
 * @brief 
**/
class test_globalConstraint_suite : public ::testing::Test{
    protected:
        test_globalConstraint_suite(){};
        virtual ~test_globalConstraint_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_globalConstaint_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_globalConstraint_suite, *)
        };
};


/**
 * @brief check printKeyConstraint
 * @begin_version
 **/
TEST_F(test_globalConstraint_suite, case_printKeyConstraint)
{
	//TODO
	comcfg::Configure conf;
	int ret = conf.load("conf/", "getKey.conf");
	ASSERT_EQ(0, ret);
	//range file not specified
	ret = conf.printKeyConstraint("company.svr.ip");
	ASSERT_NE(0, ret);
	//wrongly formatted key_path
	ret = conf.printKeyConstraint(".svr.ip", "conf/getKey.range");
	ASSERT_NE(0, ret);
	//group not found
	ret = conf.printKeyConstraint("company.notfound.ip", "conf/getKey.range");
	ASSERT_NE(0, ret);
	//right long key
	ret = conf.printKeyConstraint("company.svr.ip", "conf/getKey.range");
	ASSERT_EQ(0, ret);
	//right short key
	ret = conf.printKeyConstraint("position");
	ASSERT_EQ(0, ret);
	//key not found
	ret = conf.printKeyConstraint("not_found_key");
	ASSERT_NE(0, ret);
}

/**
 * @brief test global constraint function regexp
 * @begin_version
 **/
TEST_F(test_globalConstraint_suite, case_regexp_wrong_arg_num)
{
	comcfg::GlobalConsFun<bsl::string> gFun("regexp_wrong_arg_num", comcfg::CFdefault::gcons_regexp);
	gFun._arg.push_back("(\\d)+");
	gFun._arg.push_back(".{1,64}");
	comcfg::ConstraintLibrary::getInstance()->registGlobalConstraint(gFun);

	comcfg::Configure conf;
	int ret = conf.load("conf/", "regexp.conf");
	ASSERT_NE(0, ret);
	
	comcfg::ConstraintLibrary::destroyInstance();
}

/**
 * @brief test global constraint function regexp
 * @begin_version
 **/
TEST_F(test_globalConstraint_suite, case_regexp_match)
{
	comcfg::GlobalConsFun<bsl::string> gFun("regexp_match", comcfg::CFdefault::gcons_regexp);
	gFun._arg.push_back("(\\d)+");
	comcfg::ConstraintLibrary::getInstance()->registGlobalConstraint(gFun);

	comcfg::Configure conf;
	int ret = conf.load("conf/", "regexp.conf");
	ASSERT_EQ(0, ret);
	
	comcfg::ConstraintLibrary::destroyInstance();
}

/**
 * @brief test global constraint function regexp
 * @begin_version
 **/
TEST_F(test_globalConstraint_suite, case_regexp_not_match)
{
	comcfg::GlobalConsFun<bsl::string> gFun("regexp_not_match", comcfg::CFdefault::gcons_regexp);
	gFun._arg.push_back(".{1,5}");
	comcfg::ConstraintLibrary::getInstance()->registGlobalConstraint(gFun);

	comcfg::Configure conf;
	int ret = conf.load("conf/", "regexp.conf");
	ASSERT_NE(0, ret);
	
	comcfg::ConstraintLibrary::destroyInstance();
}

/**
 * @brief test global constraint function ip
 * @begin_version
 **/
TEST_F(test_globalConstraint_suite, case_ip_match)
{
	comcfg::GlobalConsFun<bsl::string> gFun("ip_match", comcfg::CFdefault::gcons_ip);
	comcfg::ConstraintLibrary::getInstance()->registGlobalConstraint(gFun);

	comcfg::Configure conf;
	int ret = conf.load("conf/", "ip.conf");
	ASSERT_EQ(0, ret);
	
	comcfg::ConstraintLibrary::destroyInstance();
}

/**
 * @brief test global constraint function ip
 * @begin_version
 **/
TEST_F(test_globalConstraint_suite, case_ip_not_match)
{
	comcfg::GlobalConsFun<bsl::string> gFun("ip_not_match", comcfg::CFdefault::gcons_ip);
	comcfg::ConstraintLibrary::getInstance()->registGlobalConstraint(gFun);

	comcfg::Configure conf;
	int ret = conf.load("conf/", "regexp.conf");
	ASSERT_NE(0, ret);
	
	comcfg::ConstraintLibrary::destroyInstance();
}

/**
 * @brief test global constraint function length
 * @begin_version
 **/
TEST_F(test_globalConstraint_suite, case_length_int)
{
	typedef int ARG_TYPE;
	ARG_TYPE DEF_VALUE = 100;
	comcfg::GlobalConsFun<ARG_TYPE> gFun("length_int", comcfg::CFdefault::gcons_length);
	gFun._arg.push_back(DEF_VALUE);
	comcfg::ConstraintLibrary::getInstance()->registGlobalConstraint(gFun);

	comcfg::Configure conf;
	int ret = conf.load("conf/", "length.conf");
	ASSERT_EQ(0, ret);

	comcfg::ConstraintLibrary::destroyInstance();
}

/**
 * @brief test global constraint function length
 * @begin_version
 **/
TEST_F(test_globalConstraint_suite, case_length_longlong)
{
	typedef long long ARG_TYPE;
	ARG_TYPE DEF_VALUE = 5;
	comcfg::GlobalConsFun<ARG_TYPE> gFun("length_longlong", comcfg::CFdefault::gcons_length);
	gFun._arg.push_back(DEF_VALUE);
	comcfg::ConstraintLibrary::getInstance()->registGlobalConstraint(gFun);

	comcfg::Configure conf;
	int ret = conf.load("conf/", "length.conf");
	ASSERT_NE(0, ret);

	comcfg::ConstraintLibrary::destroyInstance();
}

/**
 * @brief test global constraint function default
 * @begin_version
 **/
TEST_F(test_globalConstraint_suite, case_default_int)
{
	typedef int ARG_TYPE;
	const ARG_TYPE DEF_VALUE = 100;
	comcfg::GlobalConsFun<ARG_TYPE> gFun("default_int", comcfg::CFdefault::gcons_default);
	gFun._arg.push_back(DEF_VALUE);
	comcfg::ConstraintLibrary::getInstance()->registGlobalConstraint(gFun);

	comcfg::Configure conf;
	int ret = conf.load("conf/", "default.conf");
	ASSERT_EQ(0, ret);

	int errcode = 0;
	int value = conf["def_value"].to_int32(&errcode);
	if(errcode == 0){
		ASSERT_EQ(DEF_VALUE, value);
	}
	errcode = 0;
	value = conf["not_def_value"].to_int32(&errcode);
	if(errcode == 0){
		ASSERT_NE(DEF_VALUE, value);
	}
	
	comcfg::ConstraintLibrary::destroyInstance();
}

/**
 * @brief test global constraint function default
 * @begin_version
 **/
TEST_F(test_globalConstraint_suite, case_default_longlong)
{
	typedef long long ARG_TYPE;
	const ARG_TYPE DEF_VALUE = 100;
	comcfg::GlobalConsFun<ARG_TYPE> gFun("default_longlong", comcfg::CFdefault::gcons_default);
	gFun._arg.push_back(DEF_VALUE);
	comcfg::ConstraintLibrary::getInstance()->registGlobalConstraint(gFun);

	comcfg::Configure conf;
	int ret = conf.load("conf/", "default.conf");
	ASSERT_EQ(0, ret);

	int errcode = 0;
	ARG_TYPE value = conf["def_value"].to_int64(&errcode);
	if(errcode == 0){
		ASSERT_EQ(DEF_VALUE, value);
	}
	errcode = 0;
	value = conf["not_def_value"].to_int64(&errcode);
	if(errcode == 0){
		ASSERT_NE(DEF_VALUE, value);
	}
	
	comcfg::ConstraintLibrary::destroyInstance();
}

/**
 * @brief test global constraint function default
 * @begin_version
 **/
TEST_F(test_globalConstraint_suite, case_default_double)
{
	typedef double ARG_TYPE;
	const ARG_TYPE DEF_VALUE = 100;
	comcfg::GlobalConsFun<ARG_TYPE> gFun("default_double", comcfg::CFdefault::gcons_default);
	gFun._arg.push_back(DEF_VALUE);
	comcfg::ConstraintLibrary::getInstance()->registGlobalConstraint(gFun);

	comcfg::Configure conf;
	int ret = conf.load("conf/", "default.conf");
	ASSERT_EQ(0, ret);

	int errcode = 0;
	ARG_TYPE value = conf["def_value"].to_double(&errcode);
	if(errcode == 0){
		ASSERT_DOUBLE_EQ(DEF_VALUE, value);
	}

	comcfg::ConstraintLibrary::destroyInstance();
}

/**
 * @brief test global constraint function default
 * @begin_version
 **/
TEST_F(test_globalConstraint_suite, case_default_string)
{
	typedef bsl::string ARG_TYPE;
	const ARG_TYPE DEF_VALUE = "hello, default string";
	comcfg::GlobalConsFun<ARG_TYPE> gFun("default_string", comcfg::CFdefault::gcons_default);
	gFun._arg.push_back(DEF_VALUE);
	comcfg::ConstraintLibrary::getInstance()->registGlobalConstraint(gFun);

	comcfg::Configure conf;
	int ret = conf.load("conf/", "default.conf");
	ASSERT_EQ(0, ret);

	int errcode = 0;
	ARG_TYPE value = conf["def_value"].to_bsl_string(&errcode);
	if(errcode == 0){
		ASSERT_STREQ(DEF_VALUE.c_str(), value.c_str());
	}
	errcode = 0;
	value = conf["not_def_value"].to_bsl_string(&errcode);
	if(errcode == 0){
		ASSERT_STRNE(DEF_VALUE.c_str(), value.c_str());
	}
	
	comcfg::ConstraintLibrary::destroyInstance();
}
	
/**
 * @brief test global constraint function default
 * @begin_version
 **/
TEST_F(test_globalConstraint_suite, case_default_wrong_type)
{
	typedef long ARG_TYPE;
	ARG_TYPE DEF_VALUE = 100;
	comcfg::GlobalConsFun<ARG_TYPE> gFun("default_long", comcfg::CFdefault::gcons_default);
	gFun._arg.push_back(DEF_VALUE);
	comcfg::ConstraintLibrary::getInstance()->registGlobalConstraint(gFun);

	comcfg::Configure conf;
	int ret = conf.load("conf/", "default.conf");
	ASSERT_NE(0, ret);

	comcfg::ConstraintLibrary::destroyInstance();
}

/**
 * @brief test global constraint function enum
 * @begin_version
 **/
TEST_F(test_globalConstraint_suite, case_enum_int)
{
	typedef int ARG_TYPE;
	comcfg::GlobalConsFun<ARG_TYPE> gFun("enum_int", comcfg::CFdefault::gcons_enum);
	gFun._arg.push_back(1);
	gFun._arg.push_back(10);
	gFun._arg.push_back(100);
	gFun._arg.push_back(1000);
	comcfg::ConstraintLibrary::getInstance()->registGlobalConstraint(gFun);

	comcfg::Configure conf;
	int ret = conf.load("conf/", "enum.conf");
	ASSERT_EQ(0, ret);

	comcfg::ConstraintLibrary::destroyInstance();
}

/**
 * @brief test global constraint function enum
 * @begin_version
 **/
TEST_F(test_globalConstraint_suite, case_enum_longlong)
{
	typedef long long ARG_TYPE;
	comcfg::GlobalConsFun<ARG_TYPE> gFun("enum_longlong", comcfg::CFdefault::gcons_enum);
	gFun._arg.push_back(1);
	gFun._arg.push_back(100);
	gFun._arg.push_back(1000);
	comcfg::ConstraintLibrary::getInstance()->registGlobalConstraint(gFun);

	comcfg::Configure conf;
	int ret = conf.load("conf/", "enum.conf");
	ASSERT_NE(0, ret);

	comcfg::ConstraintLibrary::destroyInstance();
}

/**
 * @brief test global constraint function enum
 * @begin_version
 **/
TEST_F(test_globalConstraint_suite, case_enum_string)
{
	typedef bsl::string ARG_TYPE;
	comcfg::GlobalConsFun<ARG_TYPE> gFun("enum_string", comcfg::CFdefault::gcons_enum);
	gFun._arg.push_back("1");
	gFun._arg.push_back("10");
	gFun._arg.push_back("100");
	gFun._arg.push_back("1000");
	comcfg::ConstraintLibrary::getInstance()->registGlobalConstraint(gFun);

	comcfg::Configure conf;
	int ret = conf.load("conf/", "enum.conf");
	ASSERT_EQ(0, ret);

	comcfg::ConstraintLibrary::destroyInstance();
}

/**
 * @brief test global constraint function enum
 * @begin_version
 **/
TEST_F(test_globalConstraint_suite, case_enum_double)
{
	typedef double ARG_TYPE;
	comcfg::GlobalConsFun<ARG_TYPE> gFun("enum_double", comcfg::CFdefault::gcons_enum);
	gFun._arg.push_back(1);
	gFun._arg.push_back(10);
	gFun._arg.push_back(1000);
	comcfg::ConstraintLibrary::getInstance()->registGlobalConstraint(gFun);

	comcfg::Configure conf;
	int ret = conf.load("conf/", "enum.conf");
	ASSERT_NE(0, ret);

	comcfg::ConstraintLibrary::destroyInstance();
}

/**
 * @brief test global constraint function enum
 * @begin_version
 **/
TEST_F(test_globalConstraint_suite, case_enum_wrong_type)
{
	typedef char ARG_TYPE;
	comcfg::GlobalConsFun<ARG_TYPE> gFun("enum_char", comcfg::CFdefault::gcons_enum);
	gFun._arg.push_back('1');
	gFun._arg.push_back('2');
	gFun._arg.push_back('3');
	comcfg::ConstraintLibrary::getInstance()->registGlobalConstraint(gFun);

	comcfg::Configure conf;
	int ret = conf.load("conf/", "enum.conf");
	ASSERT_NE(0, ret);

	comcfg::ConstraintLibrary::destroyInstance();
}

/**
 * @brief test global constraint function range
 * @begin_version
 **/
TEST_F(test_globalConstraint_suite, case_range_int)
{
	typedef int ARG_TYPE;
	comcfg::GlobalConsFun<ARG_TYPE> gFun("range_int", comcfg::CFdefault::gcons_range);
	gFun._arg.push_back(1);
	gFun._arg.push_back(1000);
	comcfg::ConstraintLibrary::getInstance()->registGlobalConstraint(gFun);

	comcfg::Configure conf;
	int ret = conf.load("conf/", "range.conf");
	ASSERT_EQ(0, ret);

	comcfg::ConstraintLibrary::destroyInstance();
}

/**
 * @brief test global constraint function range
 * @begin_version
 **/
TEST_F(test_globalConstraint_suite, case_range_longlong)
{
	typedef long long ARG_TYPE;
	comcfg::GlobalConsFun<ARG_TYPE> gFun("range_longlong", comcfg::CFdefault::gcons_range);
	gFun._arg.push_back(1);
	gFun._arg.push_back(500);
	comcfg::ConstraintLibrary::getInstance()->registGlobalConstraint(gFun);

	comcfg::Configure conf;
	int ret = conf.load("conf/", "range.conf");
	ASSERT_NE(0, ret);

	comcfg::ConstraintLibrary::destroyInstance();
}

/**
 * @brief test global constraint function range
 * @begin_version
 **/
TEST_F(test_globalConstraint_suite, case_range_double)
{
	typedef double ARG_TYPE;
	comcfg::GlobalConsFun<ARG_TYPE> gFun("range_double", comcfg::CFdefault::gcons_range);
	gFun._arg.push_back(1);
	gFun._arg.push_back(1000);
	comcfg::ConstraintLibrary::getInstance()->registGlobalConstraint(gFun);

	comcfg::Configure conf;
	int ret = conf.load("conf/", "range.conf");
	ASSERT_EQ(0, ret);

	comcfg::ConstraintLibrary::destroyInstance();
}

/**
 * @brief test variable parameters
 * @begin_version
 **/
TEST_F(test_globalConstraint_suite, case_variable_parameters_int)
{
	comcfg::ConstraintLibrary::getInstance()->registGlobalConstraint("range_int_vp",comcfg::CFdefault::gcons_range<int>, 2, 1, 1000);

	comcfg::Configure conf;
	int ret = conf.load("conf/", "range.conf");
	ASSERT_EQ(0, ret);

	comcfg::ConstraintLibrary::destroyInstance();
}

/**
 * @brief test variable parameters
 * @begin_version
 **/
TEST_F(test_globalConstraint_suite, case_variable_parameters_double)
{
	//comcfg::ConstraintLibrary::getInstance()->registGlobalConstraint("range_double_vp",comcfg::CFdefault::gcons_range<double>, 2, 1, 1000);
	comcfg::ConstraintLibrary::getInstance()->registGlobalConstraint("range_double_vp",comcfg::CFdefault::gcons_range<double>, 2, 1.0, 1000.0);

	comcfg::Configure conf;
	int ret = conf.load("conf/", "range.conf");
	//ASSERT_NE(0, ret);
	ASSERT_EQ(0, ret);

	comcfg::ConstraintLibrary::destroyInstance();
}

/**
 * @brief test variable parameters
 * @note the program will crash if the variable parameters are not of POD type. This must be paid great attention.
 * @begin_version
 **/
TEST_F(test_globalConstraint_suite, case_variable_parameters_string)
{
	/*
	comcfg::ConstraintLibrary::getInstance()->registGlobalConstraint("enum_string_vp",comcfg::CFdefault::gcons_enum<bsl::string>, 4, "1", "10", "100", "1000");

	comcfg::Configure conf;
	int ret = conf.load("conf/", "enum.conf");
	ASSERT_EQ(0, ret);

	comcfg::ConstraintLibrary::destroyInstance();
	*/
}




