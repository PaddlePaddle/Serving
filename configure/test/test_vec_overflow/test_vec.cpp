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
class test_vof_suite : public ::testing::Test{
    protected:
        test_vof_suite(){};
        virtual ~test_vof_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_vof_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_vof_suite, *)
        };
};


/**
 * @brief check the log to see the info, and the ret is always 0
 * @begin_version
 **/
TEST_F(test_vof_suite, case_vof)
{
	//TODO
	//comcfg::Log::openLowLevel();
	const char *res_file = "vec.res";
	freopen(res_file, "w+", stderr);
	comcfg::Configure conf;
	int ret = conf.load("./", "vec.conf", "vec.range");
	ASSERT_NE(-1, ret);
	
	fflush(stderr);
	fclose(stderr);
	ASSERT_COMLOG_INCLUDE(res_file, "array has too many elements!");
	remove(res_file);
}
