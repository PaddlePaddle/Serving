
#include <Configure.h>
#include <gtest/gtest.h>
#include "spreg.h"	
	
int main(int argc, char **argv)
{
 	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
/**
 * @brief 
**/
class test_dump_ex_suite : public ::testing::Test{
    protected:
        test_dump_ex_suite(){};
        virtual ~test_dump_ex_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_dump_ex_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_dump_ex_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 
**/
TEST_F(test_dump_ex_suite, case_ivalid_param1)
{
	//TODO
	//comcfg::Log::openLowLevel();
	comcfg::Configure conf;
	int ret = conf.load("conf", "sample.conf");
	ASSERT_EQ(ret, 0);

	char buf[1024];

	ret = conf.dump_ex(NULL, 1024, 1);
	EXPECT_NE(ret, 0);

	ret = conf.dump_ex(buf, 0, 1);
	EXPECT_NE(ret, 0);

	ret = conf.dump_ex(buf, 1024, -1);
	EXPECT_NE(ret, 0);

	ret = conf.dump_ex(buf, 1024, 3);
	EXPECT_NE(ret, 0);

	ret = conf.dump_ex(buf, 100, 0);
	EXPECT_NE(ret, 0);
}

/**
 * @brief 
 * @begin_version 
**/
TEST_F(test_dump_ex_suite, case_Comment)
{
	//TODO
	//comcfg::Log::openLowLevel();
	comcfg::Configure conf;
	int ret = conf.load("conf", "sample.conf");
	ASSERT_EQ(ret, 0);

	char buf[10240];

	ret = conf.dump_ex(buf, 10240, 0);
	EXPECT_GT(ret, 0);

	char * comment_pos  = strstr(buf,"#you see, \n#Joan is a girl, which");
	ASSERT_NE((long)comment_pos, 0);

	comment_pos = strstr(buf, "#this is the reception floor");
	ASSERT_NE((long)comment_pos, 0);

	comment_pos = strstr(buf, "#GLOBAL: this comment won't be dumped");
	ASSERT_EQ((long)comment_pos, 0);

	comment_pos = strstr(buf, "#END: this comment won't be dumped");
	ASSERT_EQ((long)comment_pos, 0);
}

/**
 * @brief 
 * @begin_version 
**/
TEST_F(test_dump_ex_suite, case_NoComment)
{
	//TODO
	//comcfg::Log::openLowLevel();
	comcfg::Configure conf;
	int ret = conf.load("conf", "sample.conf");
	ASSERT_EQ(ret, 0);

	char buf[10240];

	ret = conf.dump_ex(buf, 10240, 1);
	EXPECT_GT(ret, 0);

	char * comment_pos = strchr(buf, '#');
	ASSERT_EQ((long)comment_pos, 0);
}

/**
 * @brief 
 * @begin_version 
**/
TEST_F(test_dump_ex_suite, case_Position)
{
	//TODO
	//comcfg::Log::openLowLevel();
	comcfg::Configure conf;
	int ret = conf.load("conf", "sample.conf");
	ASSERT_EQ(ret, 0);

	char buf[10240];

	ret = conf.dump_ex(buf, 10240, 2);
	EXPECT_GT(ret, 0);

	//std::cout<<buf<<endl;
	/*
	const char * err;
	spreg_t *re = spreg_init("^#\[.*:\d+\]$", &err);
	ASSERT_EQ((long)err, 0);

	ret = spreg_search(re, buf, ret, 0, 0);
	spreg_destroy(re);
	ASSERT_EQ(ret, 0);
	*/
	char *position_pos = strstr(buf, "\n#[conf/sample.conf:");
	ASSERT_NE((long)position_pos, 0);
}

TEST_F(test_dump_ex_suite, case_modify_unit)
{
	//TODO
	//comcfg::Log::openLowLevel();
	comcfg::Configure conf;
	int ret = conf.load("conf", "sample.conf");
	ASSERT_EQ(ret, 0);

	conf["company"]["staff"].add_unit("average_salary", "20000");
	conf["company"]["staff"]["employee"].del_unit("1");

	int as = conf["company"]["staff"]["average_salary"].to_int32();
	ASSERT_EQ(as, 20000);
	
	char buf[10240];
	ret = conf.dump_ex(buf, 10240, 2);
	EXPECT_GT(ret, 0);

	//std::cout<<buf<<endl;
	char *add_pos = strstr(buf, "\naverage_salary : 20000\n");
	EXPECT_NE((long)add_pos, 0);

	char * del_pos = strstr(buf, "Jay");
	EXPECT_EQ((long)del_pos, 0);
	
}

/**
 * @brief 
 * @begin_version 
**/
TEST_F(test_dump_ex_suite, case_rebuild)
{
	//TODO
	//comcfg::Log::openLowLevel();
	comcfg::Configure conf;
	int ret = conf.load("conf", "sample.conf");
	ASSERT_EQ(ret, 0);

	char buf[10240];

	ret = conf.dump_ex(buf, 10240, 2);
	EXPECT_GT(ret, 0);

	comcfg::Configure conf2;
	char *confstr = conf2.getRebuildBuffer(ret);
	strcpy(confstr, buf);
	ret = conf2.rebuild();
	EXPECT_EQ(ret, 0);
}

TEST_F(test_dump_ex_suite, case_modify_array)
{
	//TODO
	//comcfg::Log::openLowLevel();
	comcfg::Configure conf;
	int ret = conf.load("conf", "array.conf");
	ASSERT_EQ(ret, 0);

	comcfg::ConfigUnit & floorArray = conf["building"][0]["floor"];
	if(floorArray.selfType() == comcfg::CONFIG_ARRAY_TYPE){
		floorArray.add_unit("@floor", "4");
	}
	conf["building"][0].add_unit("@floor", "5");

	char buf[10240];
	ret = conf.dump_ex(buf, 10240, 1);
	EXPECT_GT(ret, 0);

	//std::cout<<buf<<endl;
}

/**
 * @brief 
 * @begin_version 
**/
TEST_F(test_dump_ex_suite, case_rebuild2)
{
	//TODO
	//comcfg::Log::openLowLevel();
	comcfg::Configure conf;
	int ret = conf.load("conf", "sample.conf");
	ASSERT_EQ(ret, 0);

	int dump_type = 0;
	char buf[10240];

	ret = conf.dump_ex(buf, 10240, dump_type);
	EXPECT_GT(ret, 0);

	//comcfg::Log::openLowLevel();
	comcfg::Configure conf2;
	char *confstr = conf2.getRebuildBuffer(ret);
	strcpy(confstr, buf);
	ret = conf2.rebuild();
	EXPECT_EQ(ret, 0);

	ret = conf2.dump_ex(buf, 10240, dump_type);
	ASSERT_GT(ret, 0);
	//std::cout<<"\n\n\n"<<buf<<endl;
	
	char * comment_pos = strchr(buf, '#');
	ASSERT_NE((long)comment_pos, 0);

}
