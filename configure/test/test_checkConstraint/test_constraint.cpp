#include <stdlib.h>
#include <Configure.h>
#include <gtest/gtest.h>
	
	
int main(int argc, char **argv)
{
 	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
/**
 * @brief 
**/
class test_load_ex2_suite : public ::testing::Test{
    protected:
        test_load_ex2_suite(){};
        virtual ~test_load_ex2_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_load_ex_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_load_ex_suite, *)
        };
};


/**
 * @brief check null param 
 * @begin_version
 **/
TEST_F(test_load_ex2_suite, case_nullParam)
{
	//TODO
	comcfg::Configure conf[3];
	int ret = conf[0].load_ex2("conf/", "ill_cons.conf", NULL);
	ASSERT_EQ(0, ret);
	ret = conf[1].load_ex2("conf/", "ill_cons.conf", "");
	ASSERT_EQ(0, ret);
	ret = conf[2].load_ex2("conf/", "ill_cons.conf");
	ASSERT_EQ(0, ret);
}

/**
 * @brief check regular range file 
 * @begin_version
 **/
TEST_F(test_load_ex2_suite, case_reg_conf)
{
	//TODO
	comcfg::Configure conf;
	int ret = conf.load_ex2("conf/", "reg_cons.conf", "cons.range");
	ASSERT_EQ(0, ret);
}
  
/**
 * @brief check illegal range file 
 * @begin_version
 **/
TEST_F(test_load_ex2_suite, case_ill_conf)
{
	//TODO
	comcfg::Configure conf;
	int ret = conf.load_ex2("conf/", "ill_cons.conf", "cons.range");
	ASSERT_NE(0, ret);
}
  
/**
 * @brief check load()+checkConstraint()
 * @begin_version
 **/
TEST_F(test_load_ex2_suite, case_load_check_conf)
{
	//TODO
	comcfg::Configure conf;
	int ret = conf.load("conf/", "ill_cons.conf", "cons.range");
	ASSERT_EQ(0, ret);
	ret = conf.checkConstraint();
	ASSERT_NE(0, ret);
}
  
/**
 * @brief check load_ex()+checkConstraint()
 * @begin_version
 **/
TEST_F(test_load_ex2_suite, case_load_ex_check_conf)
{
	//TODO
	comcfg::Configure conf;
	int ret = conf.load_ex("conf/", "ill_cons.conf", "cons.range");
	ASSERT_EQ(0, ret);
	ret = conf.checkConstraint();
	ASSERT_NE(0, ret);
}
  
/**
 * @brief check reentrant check_once()
 * @begin_version
 **/
TEST_F(test_load_ex2_suite, case_reentrant_check_once)
{
	//TODO
	comcfg::Configure conf;
	int ret = conf.load("conf/", "reg_cons.conf"); 
	ASSERT_EQ(0, ret);
	for(int i=1; i<5; i++){
		ret = conf.check_once("conf/cons.range");
		ASSERT_EQ(0, ret);
		ret = conf.checkConstraint();
		ASSERT_EQ(0, ret);

		ret = conf.check_once("conf/bad_cons.range");
		ASSERT_EQ(0, ret);
		ret = conf.checkConstraint();
		ASSERT_NE(0, ret);
	}
}
  
