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
class test_load_ex_suite : public ::testing::Test{
    protected:
        test_load_ex_suite(){};
        virtual ~test_load_ex_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_load_ex_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_load_ex_suite, *)
        };
};


/**
 * @brief check invalid @dupLevel
 * @begin_version
 **/
TEST_F(test_load_ex_suite, case_invalidParam)
{
	//TODO
	comcfg::Configure conf[3];
	int ret = conf[0].load_ex("conf/", "nullgroup.conf", NULL, -1);
	ASSERT_NE(0, ret);
	ret = conf[1].load_ex("conf/", "nullgroup.conf", NULL, 4);
	ASSERT_NE(0, ret);
	ret = conf[2].load_ex("conf/", "nullgroup.conf", NULL, 999);
	ASSERT_NE(0, ret);
}
  
  /**
   * @brief check return value with non-duplicate conf
   * @begin_version
   **/
 TEST_F(test_load_ex_suite, case_nodupGroup)
 {
	 //TODO
	 comcfg::Configure conf[4];
	 int ret = conf[0].load_ex("conf/", "nodupgroup.conf", NULL, 0);
	 ASSERT_EQ(0, ret);
	 ret = conf[1].load_ex("conf/", "nodupgroup.conf", NULL, 1);
	 ASSERT_EQ(0, ret);
	 ret = conf[2].load_ex("conf/", "nodupgroup.conf", NULL, 2);
	 ASSERT_EQ(0, ret);
	 ret = conf[3].load_ex("conf/", "nodupgroup.conf", NULL, 3);
	 ASSERT_EQ(0, ret);
 }
  
  
  /**
   * @brief check return value with non-duplicate conf
   * @begin_version
   **/
 TEST_F(test_load_ex_suite, case_dupGroup)
 {
      //TODO
      comcfg::Configure conf[4];
	  int ret = conf[0].load_ex("conf/", "dupgroup.conf", NULL, 0);
	  ASSERT_EQ(0, ret);
	  ret = conf[1].load_ex("conf/", "dupgroup.conf", NULL, 1);
	  ASSERT_EQ(12, ret);
	  ret = conf[2].load_ex("conf/", "dupgroup.conf", NULL, 2);
	  ASSERT_EQ(12, ret);
	  ret = conf[3].load_ex("conf/", "dupgroup.conf", NULL, 3);
	  ASSERT_EQ(0, ret);
 }

  /**
   * @brief test big configure file 
   * @begin_version
   **/
 TEST_F(test_load_ex_suite, case_bigConf)
 {
     //TODO
	 //call genconf.sh to generate a bigconf.conf
	 comcfg::Configure conf;
	 int ret = conf.load_ex("conf/", "bigconf.conf", NULL, 3);
	 ASSERT_EQ(0, ret);
 }

  /**
   * @brief test vector in configure file, which should not be regarded as duplicate groups, even not a group
   * @begin_version
   **/
 TEST_F(test_load_ex_suite, case_vectorInConf)
 {
     //TODO
      comcfg::Configure conf[4];
	  int ret = conf[0].load_ex("conf/", "vecgroup.conf", NULL, 0);
	  ASSERT_EQ(0, ret);
	  ret = conf[1].load_ex("conf/", "vecgroup.conf", NULL, 1);
	  ASSERT_EQ(0, ret);
	  ret = conf[2].load_ex("conf/", "vecgroup.conf", NULL, 2);
	  ASSERT_EQ(0, ret);
	  ret = conf[3].load_ex("conf/", "vecgroup.conf", NULL, 3);
	  ASSERT_EQ(0, ret);
 }
