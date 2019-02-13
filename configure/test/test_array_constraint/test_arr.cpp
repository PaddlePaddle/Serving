#include <stdlib.h>
#include <Configure.h>
#include <gtest/gtest.h>
#include "plugin/plgcomlog.h"
	
int main(int argc, char **argv)
{
 	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
/**
 * @brief check if the lib reports the vector overflow situation 
**/
class test_arr_suite : public ::testing::Test{
    protected:
        test_arr_suite(){};
        virtual ~test_arr_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_arr_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_arr_suite, *)
        };
};

/**
 * @brief check if the defaultly created array can pass the constraint check
 * @begin_version
 **/
TEST_F(test_arr_suite, case_constraint1)
{
	//TODO
	//comcfg::Log::openLowLevel();
	comcfg::Configure conf;
	int ret = conf.load("conf/", "sample.conf", "sample.conf.range");
	ASSERT_EQ(0, ret);

	EXPECT_EQ(conf["sample_level"][0]["sample"].selfType(), comcfg::CONFIG_ARRAY_TYPE);
	EXPECT_EQ(conf["sample_level"][0]["sample"].size(), 0);
}

/**
 * @brief check if the defaultly created array can pass the constraint check
 * @begin_version
 **/
TEST_F(test_arr_suite, case_constraint2)
{
	//TODO
	//comcfg::Log::openLowLevel();
	comcfg::Configure conf;
	int ret = conf.load("conf/", "sample2.conf", "sample2.conf.range");
	ASSERT_EQ(0, ret);

	EXPECT_EQ(conf["sample_level"][0]["sample"].selfType(), comcfg::CONFIG_ARRAY_TYPE);
	EXPECT_EQ(conf["sample_level"][0]["sample"].size(), 0);
}
