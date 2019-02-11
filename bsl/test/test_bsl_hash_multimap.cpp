
#include <bsl/containers/hash/bsl_hash_multimap.h>
#include <gtest/gtest.h>
#include <string.h>
#include <bsl/alloc/bsl_alloc.h>
#include <bsl/alloc/bsl_sample_alloc.h>
#include <bsl/utils/bsl_utils.h>
#include <bsl/exception/bsl_exception.h>
#include <bsl/archive/bsl_filestream.h>
#include <bsl/archive/bsl_serialization.h>
#include <bsl/archive/bsl_binarchive.h>
#include <map>

using namespace std; 

typedef string _key;
typedef int _value;
typedef pair<_key, _value> _Pair;
typedef equal_to<_key> _Equl;
typedef bsl::bsl_sample_alloc<bsl::bsl_alloc<_key>, 256>  _InnerAlloc;

_Equl _equl;

typedef bsl::hash_multimap<_key, _value> hash_type;

typedef hash_type::iterator _iterator;
typedef hash_type::const_iterator _const_iterator;
typedef hash_type::iterator_pair _iterator_pair;
typedef hash_type::const_iterator_pair _const_iterator_pair;

void print_hash(_const_iterator begin, _const_iterator end) 
{
	printf("The hash_multimap can be seen as follow:\n");
	for(hash_type::const_iterator ite = begin ; ite != end; ++ite)
	{    
		//printf("%s -> %d\n", ite->first, ite->second);
		cout << ite->first << " -> " << ite->second << endl;
	}    
};

//返回第一个键值为key的迭代器
_const_iterator begin_key(const hash_type &ht, const _key &key)
{
	_const_iterator __first = ht.begin();
	while(__first != ht.end())
	{
		//if((*__first).first == key) 
		if(_equl(__first->first, key))
		{
			return __first;
		}
		++__first;
	}
	return ht.end();
};

_iterator begin_key(hash_type &ht, const _key &key)
{
	_iterator __first = ht.begin();
	while(__first != ht.end())
	{
		//if((*__first).first == key)
		if(_equl(__first->first, key))
		{
			return __first;
		}
		++__first;
	}return ht.end();
};

//返回最后一个个键值为key的迭代器的next
_const_iterator end_key(const hash_type &ht, const _key &key)
{
	_const_iterator __end = ht.begin();
	while(__end != ht.end())
	{
		//if((*__end).first == key)
		if(_equl(__end->first, key))
		{
			while(__end != ht.end() && _equl(__end->first, key))
			{
				++__end;
			}
			break;
		}
		++__end;
	}
	return __end;
};

_iterator end_key(hash_type &ht, const _key &key)
{
	_iterator __end = ht.begin();
	while(__end != ht.end())
	{
		//if((*__end).first == key)
		if(_equl(__end->first, key))
		{
			while(__end != ht.end() && _equl(__end->first, key))
			{
				++__end;
			}
			break;
		}
		++__end;
	}
	return __end;
};
	
	
int main(int argc, char **argv)
{
 	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
/**
 * @brief 
**/
class test_hash_multimap_hash_multimap_suite : public ::testing::Test{
    protected:
        test_hash_multimap_hash_multimap_suite(){};
        virtual ~test_hash_multimap_hash_multimap_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_hash_multimap_hash_multimap_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_hash_multimap_hash_multimap_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_hash_multimap_hash_multimap_suite, case_default)
{
	//TODO
	hash_type names;
}
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_hash_multimap_hash_multimap_suite, case_copy)
{
	//TODO
	hash_type names;

	if(0 != names.create(5))
	{
		printf("create hash_multimap error with size %d\n", 5);
	}
	names.set("Bob", 1, 1);
	names.set("Bob", 1, 1);
	names.set("Bob", 2, 1);
	names.set("Bob", 3, 1);
	names.set("Bob", 3, 1);
	names.set("Jack", 2, 1);
	names.set("Jones", 2, 1);
	names.set("Black", 1, 1);
	names.set("White", 1, 1);
	names.set("White", 3, 1);
	names.set("Li", 4, 1);
	names.set("Li", 4, 1);
	names.set("Zhang", 1, 1);
	
	hash_type names_copy(names);
	
	printf("*********begin**************\n");	
	hash_type hmp3(0);
	printf("**********end*************\n");

	try
	{
		hash_type hmp3(NULL);
		int ret=hmp3.is_created();
		printf("***********************ret = %d\n", ret);
	}
	catch(bsl::Exception& e){printf("%s\n", e.all());}
	
	
	//printf("****************\n");
	//printf("Source:\n");
	//print_hash(names.begin(), names.end());
	//printf("Copy:\n");
	//print_hash(names_copy.begin(), names_copy.end());
}
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_hash_multimap_hash_multimap_suite, case_normal)
{
	//TODO
	hash_type names(5);

	names.set("Jones", 2, 1);
	names.set("White", 3, 1);
	names.set("White", 3, 1);

	//print_hash(names_copy.begin(), names_copy.end());
}
 
/**
 * @brief 
**/
class test_hash_multimap_operator_equal_suite : public ::testing::Test{
    protected:
        test_hash_multimap_operator_equal_suite(){};
        virtual ~test_hash_multimap_operator_equal_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_hash_multimap_operator_equal_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_hash_multimap_operator_equal_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_hash_multimap_operator_equal_suite, case_self)
{
	//TODO
	hash_type names(5);
	names.set("Jones", 2, 1);
	names.set("White", 3, 1);
	names.set("Zhang", 1, 1);

	names = names;
	//printf("******************\n");
	//print_hash(names.begin(), names.end());
}

/**
 * @brief
 * @begin_version 1.1.15.0
**/
TEST_F(test_hash_multimap_operator_equal_suite, case_other)
{
	hash_type names(5);

	names.set("Jones", 2, 1);
	names.set("White", 3, 1);
	names.set("Zhang", 1, 1);
	names.set("Zhang", 1, 1);

	//printf("^^^^^^^^^^^^^^^^^^^^^^^^\n");
	hash_type names_des = names;
	names_des = names;
	//print_hash(names_des.begin(), names_des.end());
}
 
/**
 * @brief 
**/
class test_hash_multimap_begin_suite : public ::testing::Test{
    protected:
        test_hash_multimap_begin_suite(){};
        virtual ~test_hash_multimap_begin_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_hash_multimap_begin_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_hash_multimap_begin_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_hash_multimap_begin_suite, case_normal)
{
	//TODO
	hash_type names(100);
	_iterator _iter_begin = names.begin();
	//EXPECT_EQ(0,_iter_begin._bucketpos);
	//EXPECT_EQ(0,_iter_begin._node);
}
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_hash_multimap_begin_suite, case_const)
{
	//TODO
}
 
/**
 * @brief 
**/
class test_hash_multimap_end_suite : public ::testing::Test{
    protected:
        test_hash_multimap_end_suite(){};
        virtual ~test_hash_multimap_end_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_hash_multimap_end_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_hash_multimap_end_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_hash_multimap_end_suite, case_normal)
{
	//TODO
}
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_hash_multimap_end_suite, case_const)
{
	//TODO
}
 
/**
 * @brief 
**/
class test_hash_multimap_size_suite : public ::testing::Test{
    protected:
        test_hash_multimap_size_suite(){};
        virtual ~test_hash_multimap_size_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_hash_multimap_size_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_hash_multimap_size_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_hash_multimap_size_suite, case_size)
{
	//TODO
	hash_type names_null;
	ASSERT_EQ(0, names_null.size());
	
	hash_type names(5);
	ASSERT_EQ(0, names.size());

	names.set("Bob", 1, 1);
	names.set("Bob", 1, 1);
	names.set("Bob", 2, 1);
	names.set("Bob", 3, 1);
	names.set("Bob", 3, 1);
	names.set("Jack", 2, 1);
	names.set("Jones", 2, 1);
	names.set("Black", 1, 1);
	names.set("White", 1, 1);
	names.set("White", 3, 1);
	names.set("Li", 4, 1);
	names.set("Li", 4, 1);
	names.set("Zhang", 1, 1);
	names.set("White", 1, 1);
	ASSERT_EQ(14, names.size());
}

/**
 *  * @brief 
 *  **/
class test_hash_multimap_count_suite : public ::testing::Test{
    protected:
	test_hash_multimap_count_suite(){};
	virtual ~test_hash_multimap_count_suite(){};
	virtual void SetUp() {
		//Called befor every TEST_F(test_hash_multimap_count_suite, *)
	};  
	virtual void TearDown() {
		//Called after every TEST_F(test_hash_multimap_count_suite, *)
	};  
};

/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_hash_multimap_count_suite, case_count)
{
	//TODO
	hash_type names_null;
	ASSERT_EQ(0, names_null.count("Bob"));

	hash_type names(5);
	ASSERT_EQ(0, names.count("Bob"));

	names.set("Bob", 1, 1);
	names.set("Bob", 1, 1);
	names.set("Bob", 2, 1);
	names.set("Bob", 3, 1);
	names.set("Bob", 3, 1);
	names.set("Jack", 2, 1);
	names.set("Jones", 2, 1);
	names.set("Black", 1, 1);
	names.set("White", 1, 1);
	names.set("White", 3, 1);
	names.set("Li", 4, 1);
	names.set("Li", 4, 1);
	names.set("White", 1, 1);
	ASSERT_EQ(5, names.count("Bob"));
	ASSERT_EQ(3, names.count("White"));
	ASSERT_EQ(2, names.count("Li"));
	ASSERT_EQ(0, names.count("Huang"));
	ASSERT_EQ(1, names.count("Jack"));
}

/**
 * @brief 
**/
class test_hash_multimap_create_suite : public ::testing::Test{
    protected:
        test_hash_multimap_create_suite(){};
        virtual ~test_hash_multimap_create_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_hash_multimap_create_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_hash_multimap_create_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_hash_multimap_create_suite, case_create)
{
	//TODO
	hash_type names;

	ASSERT_EQ(0, names.create(5));
}
 
/**
 * @brief 
**/
class test_hash_multimap_is_created_suite : public ::testing::Test{
    protected:
        test_hash_multimap_is_created_suite(){};
        virtual ~test_hash_multimap_is_created_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_hash_multimap_is_created_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_hash_multimap_is_created_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_hash_multimap_is_created_suite, case_is_create)
{
	//TODO
	hash_type names;
	ASSERT_EQ(false, names.is_created());
	printf("**********%d\n", names.is_created());

	names.create(5);
	ASSERT_EQ(true, names.is_created());
	printf("**********%d\n", names.is_created());

	names.destroy();
	ASSERT_EQ(false, names.is_created());
}
 
/**
 * @brief 
**/
class test_hash_multimap_destroy_suite : public ::testing::Test{
    protected:
        test_hash_multimap_destroy_suite(){};
        virtual ~test_hash_multimap_destroy_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_hash_multimap_destroy_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_hash_multimap_destroy_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_hash_multimap_destroy_suite, case_destroy)
{
	//TODO
	hash_type names;
	ASSERT_EQ(0, names.destroy());

	names.create(5);
	names.set("Bob", 2, 1);
	names.set("Bob", 3, 1);
	names.set("Bob", 3, 1);
	names.set("Jack", 2, 1);
	names.set("Jones", 2, 1);
	names.set("Black", 1, 1);
	names.set("White", 1, 1);
	names.set("White", 3, 1);
	ASSERT_EQ(0, names.destroy());
	ASSERT_EQ(false, names.is_created());
	ASSERT_EQ(0, names.size());
}
 
/**
 * @brief 
**/
class test_hash_multimap_get_suite : public ::testing::Test{
    protected:
        test_hash_multimap_get_suite(){};
        virtual ~test_hash_multimap_get_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_hash_multimap_get_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_hash_multimap_get_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_hash_multimap_get_suite, case_normal_null)
{
	//TODO
	hash_type names;
	ASSERT_EQ(_iterator_pair(names.end(),names.end()), names.get("Li"));
}

/**
 * @brief
 * @begin_version 1.1.15.0
**/
TEST_F(test_hash_multimap_get_suite, case_const_null)
{
	hash_type names;
	const hash_type names_const(names);
	ASSERT_EQ(_const_iterator_pair(names_const.end(),names_const.end()), names_const.get("Li"));
}

/**
 * @brief
 * @begin_version 1.1.15.0
**/
TEST_F(test_hash_multimap_get_suite, case_normal_noexist)
{
	//TODO
	hash_type names;
	names.create(5);
	ASSERT_EQ(_iterator_pair(names.end(),names.end()), names.get("Li"));

	names.set("Bob", 1, 1);
	names.set("Bob", 1, 1);
	names.set("Bob", 2, 1);
	names.set("Bob", 3, 1);
	names.set("Bob", 3, 1);
	names.set("Jack", 2, 1);
	names.set("Jones", 2, 1);
	names.set("Black", 1, 1);
	names.set("White", 1, 1);
	names.set("White", 3, 1);
	names.set("Li", 4, 1);
	names.set("Li", 4, 1);
	names.set("White", 1, 1);

	ASSERT_EQ(_iterator_pair(names.end(),names.end()), names.get("Li1"));
	ASSERT_EQ(_iterator_pair(names.end(),names.end()), names.get("Tod"));
}

/**
 * @brief
 * @begin_version 1.1.15.0
**/
TEST_F(test_hash_multimap_get_suite, case_const_noexist)
{
	//TODO
	hash_type names;
	names.create(5);
	const hash_type names_empty(names);

	ASSERT_EQ(_const_iterator_pair(names_empty.end(),names_empty.end()), names_empty.get("Li"));

	names.set("Bob", 1, 1);
	names.set("Bob", 1, 1);
	names.set("Bob", 2, 1);
	names.set("Bob", 3, 1);
	names.set("Bob", 3, 1);
	names.set("Jack", 2, 1);
	names.set("Jones", 2, 1);
	names.set("Black", 1, 1);
	names.set("White", 1, 1);
	names.set("White", 3, 1);
	names.set("Li", 4, 1);
	names.set("Li", 4, 1);
	names.set("White", 1, 1);

	const hash_type names_const(names);
	
	ASSERT_EQ(_const_iterator_pair(names_const.end(),names_const.end()), names_const.get("Li1"));
	//_const_iterator_pair __const_iterator_pair = names_const.get("Li1");
	//print_hash(__const_iterator_pair.first, __const_iterator_pair.second);
	ASSERT_EQ(_const_iterator_pair(names_const.end(),names_const.end()), names_const.get("Tod"));
}

/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_hash_multimap_get_suite, case_normal_exist)
{
	//TODO
	hash_type names(5);
	names.set("Bob", 1, 1);
	names.set("Bob", 1, 1);
	names.set("Bob", 2, 1);
	names.set("Bob", 3, 1);
	names.set("Bob", 3, 1);
	names.set("Jack", 2, 1);
	names.set("Jones", 2, 1);
	names.set("Black", 1, 1);
	names.set("White", 1, 1);
	names.set("White", 3, 1);
	names.set("Li", 4, 1);
	names.set("Li", 4, 1);
	names.set("White", 1, 1);

	_iterator __begin;
	_iterator __end;
	_iterator_pair __ite_pair;

	//print_hash(names.begin(), names.end());

	__begin = begin_key(names, "Bob");
	__end = end_key(names, "Bob");
	printf("equal:\n");
	EXPECT_EQ(_iterator_pair(__begin, __end), names.get("Bob"));
	//printf("%s = begin---end\n", "Bob");
	//print_hash(__begin, __end);

	__begin = begin_key(names, "White");
	__end = end_key(names, "White");
	EXPECT_EQ(_iterator_pair(__begin, __end), names.get("White"));
	//printf("%s = begin---end\n", "White");
	//print_hash(__begin, __end);

	__begin = begin_key(names, "Jack");
	__end = end_key(names, "Jack");
	EXPECT_EQ(_iterator_pair(__begin, __end), names.get("Jack"));
	//printf("%s = begin---end\n", "Jack");
	//print_hash(__begin, __end);

	__begin = begin_key(names, "Jones");
	__end = end_key(names, "Jones");
	EXPECT_EQ(_iterator_pair(__begin, __end), names.get("Jones"));
	//printf("%s = begin---end\n", "Jones");
	//print_hash(__begin, __end);

	__begin = begin_key(names, "Black");
	__end = end_key(names, "Black");
	EXPECT_EQ(_iterator_pair(__begin, __end), names.get("Black"));
	//printf("%s = begin---end\n", "Black");
	//print_hash(__begin, __end);

	__begin = begin_key(names, "Li");
	__end = end_key(names, "Li");
	EXPECT_EQ(_iterator_pair(__begin, __end), names.get("Li"));
	//printf("%s = begin---end\n", "Li");
	//print_hash(__begin, __end);
}

/**
 * @brief
 * @begin_version 1.1.15.0
**/
TEST_F(test_hash_multimap_get_suite, case_const_exist)
{
	hash_type names(5);
	names.set("Bob", 1, 1);
	names.set("Bob", 1, 1);
	names.set("Bob", 2, 1);
	names.set("Bob", 3, 1);
	names.set("Bob", 3, 1);
	names.set("Jack", 2, 1);
	names.set("Jones", 2, 1);
	names.set("Black", 1, 1);
	names.set("White", 1, 1);
	names.set("White", 3, 1);
	names.set("Li", 4, 1);
	names.set("Li", 4, 1);
	names.set("White", 1, 1);

	const hash_type names_const(names);

	_const_iterator __begin;
	_const_iterator __end;
	_const_iterator_pair __ite_pair;
	//print_hash(names_const.begin(), names_const.end());

	__begin = begin_key(names_const, "Bob");
	__end = end_key(names_const, "Bob");
	//printf("equal:\n");
	EXPECT_EQ(_const_iterator_pair(__begin, __end), names_const.get("Bob"));
	//printf("%s = begin---end\n", "Bob");
	//print_hash(__begin, __end);

	__begin = begin_key(names_const, "Jack");
	__end = end_key(names_const, "Jack");
	//printf("equal:\n");
	EXPECT_EQ(_const_iterator_pair(__begin, __end), names_const.get("Jack"));
	//printf("%s = begin---end\n", "Jack");
	//print_hash(__begin, __end);

	__begin = begin_key(names_const, "Jones");
	__end = end_key(names_const, "Jones");
	//printf("equal:\n");
	EXPECT_EQ(_const_iterator_pair(__begin, __end), names_const.get("Jones"));
	//printf("%s = begin---end\n", "Jones");
	//print_hash(__begin, __end);

	__begin = begin_key(names_const, "Black");
	__end = end_key(names_const, "Black");
	//printf("equal:\n");
	EXPECT_EQ(_const_iterator_pair(__begin, __end), names_const.get("Black"));
	//printf("%s = begin---end\n", "Black");
	//print_hash(__begin, __end);

	__begin = begin_key(names_const, "White");
	__end = end_key(names_const, "White");
	//printf("equal:\n");
	EXPECT_EQ(_const_iterator_pair(__begin, __end), names_const.get("White"));
	//printf("%s = begin---end\n", "White");
	//print_hash(__begin, __end);

	__begin = begin_key(names_const, "Li");
	__end = end_key(names_const, "Li");
	//printf("equal:\n");
	EXPECT_EQ(_const_iterator_pair(__begin, __end), names_const.get("Li"));
	//printf("%s = begin---end\n", "Li");
	//print_hash(__begin, __end);

	__begin = begin_key(names_const, "Zhang");
	__end = end_key(names_const, "Zhang");
	//printf("equal:\n");
	EXPECT_EQ(_const_iterator_pair(__begin, __end), names_const.get("Zhang"));
	//printf("%s = begin---end\n", "Zhang");
	//print_hash(__begin, __end);
}
 
/**
 * @brief 
**/
class test_hash_multimap_find_suite : public ::testing::Test{
    protected:
        test_hash_multimap_find_suite(){};
        virtual ~test_hash_multimap_find_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_hash_multimap_find_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_hash_multimap_find_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_hash_multimap_find_suite, case_normal)
{
	//TODO
	//int find(const key_type &k, value_type &val)
	hash_type names;
	ASSERT_EQ(-1, names.find("Bob", 1));

	names.create(5);
	ASSERT_EQ(bsl::HASH_NOEXIST, names.find("Bob", 1));

	names.set("Bob", 1, 1);
	names.set("Bob", 1, 1);
	names.set("Bob", 2, 1);
	names.set("Bob", 3, 1);
	names.set("Bob", 3, 1);
	names.set("Jack", 2, 1);
	names.set("Jones", 2, 1);
	names.set("Black", 1, 1);
	names.set("White", 1, 1);
	names.set("White", 3, 1);
	names.set("Li", 4, 1);
	names.set("Li", 4, 1);
	names.set("White", 1, 1);

	ASSERT_EQ(bsl::HASH_EXIST, names.find("Bob", 1));
	ASSERT_EQ(bsl::HASH_NOEXIST, names.find("Bob", 5));
	ASSERT_EQ(bsl::HASH_NOEXIST, names.find("Tod", 1));
}
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_hash_multimap_find_suite, case_const)
{
	//TODO
	hash_type names;
	const hash_type names_const1(names);
	ASSERT_EQ(bsl::HASH_NOEXIST, names_const1.find("Bob", 1));

	names.create(5);
	const hash_type names_const2 = names;
	ASSERT_EQ(bsl::HASH_NOEXIST, names_const2.find("Bob", 1));

	names.set("Bob", 1, 1);
	names.set("Bob", 1, 1);
	names.set("Bob", 2, 1);
	names.set("Bob", 3, 1);
	names.set("Bob", 3, 1);
	names.set("Jack", 2, 1);
	names.set("Jones", 2, 1);
	names.set("Black", 1, 1);
	names.set("White", 1, 1);
	names.set("White", 3, 1);
	names.set("Li", 4, 1);
	names.set("Li", 4, 1);
	names.set("White", 1, 1);
	const hash_type names_const3 = names;
	ASSERT_EQ(bsl::HASH_EXIST, names.find("Bob", 1));
	ASSERT_EQ(bsl::HASH_EXIST, names.find("White", 1));
	ASSERT_EQ(bsl::HASH_EXIST, names.find("Bob", 3));
	ASSERT_EQ(bsl::HASH_NOEXIST, names.find("Bob", 5));
	ASSERT_EQ(bsl::HASH_NOEXIST, names.find("Tod", 1));
}
 
/**
 * @brief 
**/
class test_hash_multimap_set_suite : public ::testing::Test{
    protected:
        test_hash_multimap_set_suite(){};
        virtual ~test_hash_multimap_set_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_hash_multimap_set_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_hash_multimap_set_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_hash_multimap_set_suite, case_null)
{
	//TODO
	hash_type names;
	ASSERT_EQ( -1, names.set("Jack", 1) );
	ASSERT_EQ( -1, names.set("Jack", 1, 1) );
}

/**
 * @brief
 * @begin_version 1.1.15.0
**/
TEST_F(test_hash_multimap_set_suite, case_unequal)
{
	hash_type names;
	if(names.create(5) != 0)
	{
		printf("create hash_multimap error with size %d\n", 5);
	}
	ASSERT_EQ( bsl::HASH_INSERT_SEC, names.set("Bob", 1) );
	ASSERT_EQ( bsl::HASH_INSERT_SEC, names.set("Bob", 2) );
	ASSERT_EQ( bsl::HASH_INSERT_SEC, names.set("Bob", 3) );
	ASSERT_EQ( bsl::HASH_INSERT_SEC, names.set("Jack", 1) );
	ASSERT_EQ( bsl::HASH_INSERT_SEC, names.set("Jones", 2) );
	ASSERT_EQ( bsl::HASH_INSERT_SEC, names.set("Black", 1) );
	ASSERT_EQ( bsl::HASH_INSERT_SEC, names.set("White", 1) );
	ASSERT_EQ( bsl::HASH_INSERT_SEC, names.set("White", 3) );
	ASSERT_EQ( bsl::HASH_INSERT_SEC, names.set("Li", 4) ); 
	ASSERT_EQ( bsl::HASH_INSERT_SEC, names.set("Zhang", 1) );
	ASSERT_EQ( bsl::HASH_EXIST, names.set("Bob", 1) );
	ASSERT_EQ( bsl::HASH_EXIST, names.set("Li", 4) ); 

	//print_hash(names.begin(), names.end());
}

/**
 * @brief
 * @begin_version 1.1.15.0
**/
TEST_F(test_hash_multimap_set_suite, case_equal)
{
	//TODO
	hash_type names;
	if(names.create(5) != 0)
	{
		printf("create hash_multimap error with size %d\n", 5);
	}
	ASSERT_EQ( bsl::HASH_INSERT_SEC, names.set("Bob", 1, 1) );
	ASSERT_EQ( bsl::HASH_INSERT_SEC, names.set("Bob", 1, 1) );
	ASSERT_EQ( bsl::HASH_INSERT_SEC, names.set("Bob", 2, 1) );
	ASSERT_EQ( bsl::HASH_INSERT_SEC, names.set("Bob", 3, 1) );
	ASSERT_EQ( bsl::HASH_INSERT_SEC, names.set("Jack", 1, 1) );
	ASSERT_EQ( bsl::HASH_INSERT_SEC, names.set("Jones", 2, 1) );
	ASSERT_EQ( bsl::HASH_INSERT_SEC, names.set("Black", 1, 1) );
	ASSERT_EQ( bsl::HASH_INSERT_SEC, names.set("White", 1, 1) );
	ASSERT_EQ( bsl::HASH_INSERT_SEC, names.set("White", 3, 1) );
	ASSERT_EQ( bsl::HASH_INSERT_SEC, names.set("Li", 4, 1) );
	ASSERT_EQ( bsl::HASH_INSERT_SEC, names.set("Li", 4, 1) );
	ASSERT_EQ( bsl::HASH_INSERT_SEC, names.set("Zhang", 1, 1) );

	//print_hash(names.begin(), names.end());
}
 
/**
 * @brief 
**/
class test_hash_multimap_erase_suite : public ::testing::Test{
    protected:
        test_hash_multimap_erase_suite(){};
        virtual ~test_hash_multimap_erase_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_hash_multimap_erase_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_hash_multimap_erase_suite, *)
        };
};
 
/**
 * @brief
 * @begin_version 1.1.15.0
**/
TEST_F(test_hash_multimap_erase_suite, case_null)
{
	hash_type names;
	ASSERT_EQ(-1, names.erase("Bob"));
}

TEST_F(test_hash_multimap_erase_suite, case_no_exist)
{
	hash_type names;
	if(names.create(5) != 0)
	{
		printf("create hash_multimap error with size %d\n", 5);
	}

	ASSERT_EQ(0, names.erase("Bob"));

	names.set("Bob", 1, 1);
	names.set("Bob", 1, 1);
	names.set("Bob", 2, 1);
	names.set("Bob", 3, 1);
	names.set("Bob", 3, 1);
	names.set("Jack", 2, 1);
	names.set("Jones", 2, 1);
	names.set("Black", 1, 1);
	names.set("White", 1, 1);
	names.set("White", 3, 1);
	names.set("Li", 4, 1);
	names.set("Li", 4, 1);
	names.set("Zhang", 1, 1);

	//printf("BEFORE erase:");
	//print_hash(names.begin(), names.end());

	ASSERT_EQ(0, names.erase("Bob1"));
	ASSERT_EQ(0, names.erase("White1"));
	
	//printf("After erase:");
	//print_hash(names.begin(), names.end());
}

/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_hash_multimap_erase_suite, case_exist)
{
	//TODO
	hash_type names;

	if(names.create(5) != 0)
	{
		printf("create hash_multimap error with size %d\n", 5);
	}
	names.set("Bob", 1, 1);
	names.set("Bob", 1, 1);
	names.set("Bob", 2, 1);
	names.set("Bob", 3, 1);
	names.set("Bob", 3, 1);
	names.set("Jack", 2, 1);
	names.set("Jones", 2, 1);
	names.set("Black", 1, 1);
	names.set("White", 1, 1);
	names.set("White", 3, 1);
	names.set("Li", 4, 1);
	names.set("Li", 4, 1);
	names.set("Zhang", 1, 1);

	//printf("BEFORE erase:");
	//print_hash(names.begin(), names.end());

	ASSERT_EQ(5, names.erase("Bob"));
	//printf("AFTER erase %s:", "Bob");
	//print_hash(names.begin(), names.end());

	ASSERT_EQ(1, names.erase("Jack"));
	ASSERT_EQ(1, names.erase("Jones"));
	ASSERT_EQ(2, names.erase("White"));
	ASSERT_EQ(1, names.erase("Black"));
	ASSERT_EQ(2, names.erase("Li"));
	ASSERT_EQ(1, names.erase("Zhang"));

	//printf("AFTER erase:");
	//print_hash(names.begin(), names.end());
}
 
/**
 * @brief 
**/
class test_hash_multimap_erase_pair_suite : public ::testing::Test{
    protected:
        test_hash_multimap_erase_pair_suite(){};
        virtual ~test_hash_multimap_erase_pair_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_hash_multimap_erase_pair_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_hash_multimap_erase_pair_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_hash_multimap_erase_pair_suite, case_null)
{
	//TODO
	hash_type names;
	ASSERT_EQ(-1, names.erase_pair("Bob", 1));
}

/**
 * @brief
 * @begin_version 1.1.15.0
**/
TEST_F(test_hash_multimap_erase_pair_suite, case_exist)
{
	//TODO
	hash_type names;
	if(names.create(5) != 0)
	{
		printf("create hash_multimap error with size %d\n", 5);
	}
	names.set("Bob", 1, 1);
	names.set("Bob", 1, 1);
	names.set("Bob", 2, 1);
	names.set("Bob", 3, 1);
	names.set("Bob", 3, 1);
	names.set("Jack", 2, 1);
	names.set("Jones", 2, 1);
	names.set("Black", 1, 1);
	names.set("White", 1, 1);
	names.set("White", 3, 1);
	names.set("Li", 4, 1);
	names.set("Li", 4, 1);
	names.set("Zhang", 1, 1);
	
	//printf("BEFORE erase:");
	//print_hash(names.begin(), names.end());

	ASSERT_EQ(2, names.erase_pair("Bob", 3));

	//print_hash(names.begin(), names.end());

	ASSERT_EQ(2, names.erase_pair("Bob", 1));
	ASSERT_EQ(1, names.erase_pair("White", 3));
	ASSERT_EQ(1, names.erase_pair("Bob", 2));
	ASSERT_EQ(1, names.erase_pair("Jack", 2));
	ASSERT_EQ(1, names.erase_pair("Jones", 2));
	ASSERT_EQ(1, names.erase_pair("Black", 1));
	ASSERT_EQ(1, names.erase_pair("White", 1));
	ASSERT_EQ(2, names.erase_pair("Li", 4));
	ASSERT_EQ(1, names.erase_pair("Zhang", 1));
	//printf("AFTER erase ALL:");
	//print_hash(names.begin(), names.end());
}

/**
 * @brief
 * @begin_version 1.1.15.0
**/
TEST_F(test_hash_multimap_erase_pair_suite, case_no_exist)
{
	//TODO
	hash_type names;

	if(names.create(5) != 0)
		printf("create hash_multimap error with size %d\n", 5);

	ASSERT_EQ(0, names.erase_pair("Bob", 3));

	names.set("Bob", 1, 1);
	names.set("Bob", 1, 1);
	names.set("Bob", 2, 1);
	names.set("Bob", 3, 1);
	names.set("Bob", 3, 1);
	names.set("Jack", 2, 1);
	names.set("Jones", 2, 1);
	names.set("Black", 1, 1);
	names.set("White", 1, 1);
	names.set("White", 3, 1);
	names.set("Li", 4, 1);
	names.set("Li", 4, 1);
	names.set("Zhang", 1, 1);

	ASSERT_EQ(0, names.erase_pair("Bob1", 3));
	ASSERT_EQ(0, names.erase_pair("White", 5));
}
 
/**
 * @brief 
**/
class test_hash_multimap_assign_suite : public ::testing::Test{
    protected:
        test_hash_multimap_assign_suite(){};
        virtual ~test_hash_multimap_assign_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_hash_multimap_assign_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_hash_multimap_assign_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_hash_multimap_assign_suite, case_from_stlmap)
{
	//TODO
	multimap<_key, _value> st;
	st.insert(_Pair("Bob", 1));
	st.insert(_Pair("Bob", 2));
	st.insert(_Pair("Bob", 3));
	st.insert(_Pair("Bob", 3));
	st.insert(_Pair("Bob", 3));
	st.insert(_Pair("Jack", 1));
	st.insert(_Pair("Jones", 1));
	st.insert(_Pair("Black", 1));
	st.insert(_Pair("Li", 1));
	st.insert(_Pair("Zhang", 1));
	st.insert(_Pair("Zhang", 4));
	st.insert(_Pair("White", 1));

	hash_type names_assign;
	ASSERT_EQ(0, names_assign.assign(st.begin(), st.end()));
	//print_hash(names_assign.begin(), names_assign.end());

	ASSERT_EQ(0, names_assign.assign(st.begin(), st.end(), 1));
	//print_hash(names_assign.begin(), names_assign.end());
}

/**
 * @brief
 * @begin_version 1.1.15.0
**/
TEST_F(test_hash_multimap_assign_suite, case_from_bsl_multimap)
{
	//TODO
	hash_type names;
	names.create(5);

	names.set("Bob", 1, 1);
	names.set("Bob", 1, 1);
	names.set("Bob", 2, 1);
	names.set("Bob", 3, 1);
	names.set("Bob", 3, 1);
	names.set("Jack", 2, 1);
	names.set("Jones", 2, 1);
	names.set("Black", 1, 1);
	names.set("White", 1, 1);
	names.set("White", 3, 1);
	names.set("Li", 4, 1);
	names.set("Li", 4, 1);
	names.set("Zhang", 1, 1);

	hash_type names_assign;

	//printf("**************************************************");
	
	ASSERT_EQ(0, names_assign.assign(names.begin(), names.end()));	
	//print_hash(names_assign.begin(), names_assign.end());
	names_assign.destroy();
	
	ASSERT_EQ(0, names_assign.assign(names.begin(), names.end(), 1));
	//print_hash(names_assign.begin(), names_assign.end());
	names_assign.destroy();

	names_assign.create(5);
	ASSERT_EQ(0, names_assign.assign(names.begin(), names.end()));
	//print_hash(names_assign.begin(), names_assign.end());
	names_assign.destroy();

	names.create(5);
	ASSERT_EQ(0, names_assign.assign(names.begin(), names.end(), 1));
	//print_hash(names_assign.begin(), names_assign.end());
	names_assign.destroy();
}

/**
 * @brief 
**/
class test_hash_multimap_serialization_suite : public ::testing::Test{
    protected:
        test_hash_multimap_serialization_suite(){};
        virtual ~test_hash_multimap_serialization_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_hash_multimap_serialization_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_hash_multimap_serialization_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_hash_multimap_serialization_suite, case_null)
{
	//TODO
	hash_type names;
	bsl::filestream fs;

	fs.open("archivefile","w+");
	bsl::binarchive ar(fs);
	serialization(ar,names);
	fs.close();
}

/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_hash_multimap_serialization_suite, case_empty)
{
	//TODO
	hash_type names(5);
	bsl::filestream fs;
	fs.open("archivefile","w+");
	bsl::binarchive ar(fs);
	serialization(ar,names);
	fs.close();
}

/**
 * @brief 
 *  @begin_version 1.1.15.0
**/
TEST_F(test_hash_multimap_serialization_suite, case_no_empty)
{
	hash_type names(5);
	names.set("Bob", 1, 1);
	names.set("Bob", 1, 1);
	names.set("Bob", 2, 1);
	names.set("Bob", 3, 1);
	names.set("Bob", 3, 1);
	names.set("Jack", 2, 1);
	names.set("Jones", 2, 1);
	names.set("Black", 1, 1);
	names.set("White", 1, 1);
	names.set("White", 3, 1);
	names.set("Li", 4, 1);
	names.set("Li", 4, 1);
	names.set("Zhang", 1, 1);

	bsl::filestream fs;
	fs.open("archivefile","w+");
	bsl::binarchive ar(fs);
	serialization(ar,names);
	fs.close();
}
 
/**
 * @brief 
**/
class test_hash_multimap_deserialization_suite : public ::testing::Test{
    protected:
        test_hash_multimap_deserialization_suite(){};
        virtual ~test_hash_multimap_deserialization_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_hash_multimap_deserialization_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_hash_multimap_deserialization_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_hash_multimap_deserialization_suite, case_null)
{
	//TODO
	hash_type names;
	hash_type names_out;
	bsl::filestream fs;

	fs.open("archivefile","w+");
	bsl::binarchive ar(fs);
	serialization(ar,names);
	fs.close();

	fs.open("archivefile","r");
	bsl::binarchive ar1(fs);
	ASSERT_EQ(-1, deserialization(ar1,names_out));
	fs.close();

	//反序列化中，使用的是create而非recreate，所以判断出_bitems==0后，直接跳出返回-1
	ASSERT_EQ(0, names_out.size());
	ASSERT_EQ(false, names_out.is_created());
}

/**
 * @brief
 * @begin_version 1.1.15.0
**/
TEST_F(test_hash_multimap_deserialization_suite, case_empty)
{
	hash_type names(5);
	hash_type names_out;

	bsl::filestream fs;

	fs.open("archivefile","w+");
	bsl::binarchive ar(fs);
	serialization(ar,names);
	fs.close();

	fs.open("archivefile","r");
	bsl::binarchive ar1(fs);
	ASSERT_EQ(0, deserialization(ar1,names_out));
	fs.close();

	ASSERT_EQ(0, names_out.size());
	ASSERT_EQ(true, names_out.is_created());
}

/**
 * @brief
 * @begin_version 1.1.15.0
**/
TEST_F(test_hash_multimap_deserialization_suite, case_no_empty)
{
	hash_type names(5);
	hash_type names_out;

	names.set("Bob", 1, 1);
	names.set("Bob", 1, 1);
	names.set("Bob", 2, 1);
	names.set("Bob", 3, 1);
	names.set("Bob", 3, 1);
	names.set("Jack", 2, 1);
	names.set("Jones", 2, 1);
	names.set("Black", 1, 1);
	names.set("White", 1, 1);
	names.set("White", 3, 1);
	names.set("Li", 4, 1);
	names.set("Li", 4, 1);
	names.set("Zhang", 1, 1);

	bsl::filestream fs;

	fs.open("archivefile","w+");
	bsl::binarchive ar(fs);
	serialization(ar,names);
	fs.close();
	fs.open("archivefile","r");
	bsl::binarchive ar1(fs);
	ASSERT_EQ(0, deserialization(ar1,names_out));
	fs.close();

	ASSERT_EQ(13, names_out.size());

	//printf("The source hash_multimap is: \n");
	//print_hash(names.begin(), names.end());
	//printf("The deserialization result is: \n");
	//print_hash(names_out.begin(), names_out.end());
}
 
/**
 * @brief 
**/
class test_hash_multimap_clear_suite : public ::testing::Test{
    protected:
        test_hash_multimap_clear_suite(){};
        virtual ~test_hash_multimap_clear_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_hash_multimap_clear_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_hash_multimap_clear_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_hash_multimap_clear_suite, case_null)
{
	//TODO
	hash_type names;
	names.clear();
}

/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_hash_multimap_clear_suite, case_normal)
{
	hash_type names(5);

	names.clear();

	names.set("Bob", 1, 1);
	names.set("Bob", 1, 1);
	names.set("Bob", 2, 1);
	names.set("Bob", 3, 1);
	names.set("Bob", 3, 1);
	names.set("Jack", 2, 1);
	names.set("Jones", 2, 1);
	names.set("Black", 1, 1);
	names.set("White", 1, 1);
	names.set("White", 3, 1);
	names.set("Li", 4, 1);
	names.set("Li", 4, 1);
	names.set("Zhang", 1, 1);

	names.clear();

	ASSERT_EQ(0, names.size());
}
 
