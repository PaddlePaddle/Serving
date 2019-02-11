
#include <bsl/containers/hash/bsl_hashtable.h>
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

template <class _Pair>
struct pair_first
{
	typedef typename _Pair::first_type type;
	const type & operator () (const _Pair & __p) const
	{
		return __p.first;
	}
};

typedef pair_first<_Pair> _get_key;

struct hash_func
{
	size_t operator () (const std::string &__key) const
	{
		size_t key = 0;
		for (size_t i=0; i<__key.size(); ++i)
		{
			key = (key<<8)+__key[i];
		}
		return key;
	}
};

typedef bsl::bsl_hashtable<_key, _Pair, hash_func, _Equl, _get_key, _InnerAlloc> hash_type;

typedef hash_type::iterator _iterator;
typedef hash_type::const_iterator _const_iterator;
typedef hash_type::iterator_pair _iterator_pair;
typedef hash_type::const_iterator_pair _const_iterator_pair;

void print_hash(hash_type::const_iterator begin, hash_type::const_iterator end)
{
	printf("The hash_multimap can be seen as follow:\n");
	for(hash_type::const_iterator ite = begin ; ite != end; ++ite)
	{
		cout << ite->first << " -> " << ite->second << endl;
	}
};

//返回第一个键值为key的迭代器
_iterator begin_key(hash_type &ht, const _key &key)
{
	_iterator __first = ht.begin();
	while(__first != ht.end())
	{
		if((*__first).first == key)
		{
			return __first;
		}
		++__first;
	}
	return ht.end();
};

//返回最后一个个键值为key的迭代器的next
_iterator end_key(hash_type &ht, const _key &key)
{
	_iterator __end = ht.begin();
	while(__end != ht.end())
	{
		if((*__end).first == key)
		{
			while(__end != ht.end() && (*__end).first == key)
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
class test_bsl_hashtable_iterator_bsl_hashtable_iterator_suite : public ::testing::Test{
    protected:
        test_bsl_hashtable_iterator_bsl_hashtable_iterator_suite(){};
        virtual ~test_bsl_hashtable_iterator_bsl_hashtable_iterator_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_bsl_hashtable_iterator_bsl_hashtable_iterator_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_bsl_hashtable_iterator_bsl_hashtable_iterator_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_iterator_bsl_hashtable_iterator_suite, case_name1)
{
	//TODO
}
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_iterator_bsl_hashtable_iterator_suite, case_name2)
{
	//TODO
}
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_iterator_bsl_hashtable_iterator_suite, case_name3)
{
	//TODO
}
 
/**
 * @brief 
**/
class test_bsl_hashtable_iterator_operator_multiplies_suite : public ::testing::Test{
    protected:
        test_bsl_hashtable_iterator_operator_multiplies_suite(){};
        virtual ~test_bsl_hashtable_iterator_operator_multiplies_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_bsl_hashtable_iterator_operator_multiplies_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_bsl_hashtable_iterator_operator_multiplies_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_iterator_operator_multiplies_suite, case_name1)
{
	//TODO
}
 
/**
 * @brief 
**/
class test_bsl_hashtable_iterator_operator_suite : public ::testing::Test{
    protected:
        test_bsl_hashtable_iterator_operator_suite(){};
        virtual ~test_bsl_hashtable_iterator_operator_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_bsl_hashtable_iterator_operator._suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_bsl_hashtable_iterator_operator._suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_iterator_operator_suite, case_name1)
{
	//TODO
}
 
/**
 * @brief 
**/
class test_bsl_hashtable_iterator_operator_equalto_suite : public ::testing::Test{
    protected:
        test_bsl_hashtable_iterator_operator_equalto_suite(){};
        virtual ~test_bsl_hashtable_iterator_operator_equalto_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_bsl_hashtable_iterator_operator_equalto_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_bsl_hashtable_iterator_operator_equalto_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_iterator_operator_equalto_suite, case_name1)
{
	//TODO
}
 
/**
 * @brief 
**/
class test_bsl_hashtable_iterator_operator_notequal_to_suite : public ::testing::Test{
    protected:
        test_bsl_hashtable_iterator_operator_notequal_to_suite(){};
        virtual ~test_bsl_hashtable_iterator_operator_notequal_to_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_bsl_hashtable_iterator_operator_notequal_to_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_bsl_hashtable_iterator_operator_notequal_to_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_iterator_operator_notequal_to_suite, case_name1)
{
	//TODO
}
 
/**
 * @brief 
**/
class test_bsl_hashtable_iterator_operator_plusplus_suite : public ::testing::Test{
    protected:
        test_bsl_hashtable_iterator_operator_plusplus_suite(){};
        virtual ~test_bsl_hashtable_iterator_operator_plusplus_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_bsl_hashtable_iterator_operator_plusplus_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_bsl_hashtable_iterator_operator_plusplus_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_iterator_operator_plusplus_suite, case_name1)
{
	//TODO
}
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_iterator_operator_plusplus_suite, case_name2)
{
	//TODO
}
 
/**
 * @brief 
**/
class test_bsl_hashtable_const_iterator_bsl_hashtable_const_iterator_suite : public ::testing::Test{
    protected:
        test_bsl_hashtable_const_iterator_bsl_hashtable_const_iterator_suite(){};
        virtual ~test_bsl_hashtable_const_iterator_bsl_hashtable_const_iterator_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_bsl_hashtable_const_iterator_bsl_hashtable_const_iterator_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_bsl_hashtable_const_iterator_bsl_hashtable_const_iterator_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_const_iterator_bsl_hashtable_const_iterator_suite, case_name1)
{
	//TODO
}
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_const_iterator_bsl_hashtable_const_iterator_suite, case_name2)
{
	//TODO
}
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_const_iterator_bsl_hashtable_const_iterator_suite, case_name3)
{
	//TODO
}
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_const_iterator_bsl_hashtable_const_iterator_suite, case_name4)
{
	//TODO
}
 
/**
 * @brief 
**/
class test_bsl_hashtable_const_iterator_operator_multiplies_suite : public ::testing::Test{
    protected:
        test_bsl_hashtable_const_iterator_operator_multiplies_suite(){};
        virtual ~test_bsl_hashtable_const_iterator_operator_multiplies_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_bsl_hashtable_const_iterator_operator_multiplies_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_bsl_hashtable_const_iterator_operator_multiplies_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_const_iterator_operator_multiplies_suite, case_name1)
{
	//TODO
}
 
/**
 * @brief 
**/
class test_bsl_hashtable_const_iterator_operator_suite : public ::testing::Test{
    protected:
        test_bsl_hashtable_const_iterator_operator_suite(){};
        virtual ~test_bsl_hashtable_const_iterator_operator_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_bsl_hashtable_const_iterator_operator._suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_bsl_hashtable_const_iterator_operator._suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_const_iterator_operator_suite, case_name1)
{
	//TODO
}
 
/**
 * @brief 
**/
class test_bsl_hashtable_const_iterator_operator_equalto_suite : public ::testing::Test{
    protected:
        test_bsl_hashtable_const_iterator_operator_equalto_suite(){};
        virtual ~test_bsl_hashtable_const_iterator_operator_equalto_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_bsl_hashtable_const_iterator_operator_equalto_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_bsl_hashtable_const_iterator_operator_equalto_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_const_iterator_operator_equalto_suite, case_name1)
{
	//TODO
}
 
/**
 * @brief 
**/
class test_bsl_hashtable_const_iterator_operator_notequal_to_suite : public ::testing::Test{
    protected:
        test_bsl_hashtable_const_iterator_operator_notequal_to_suite(){};
        virtual ~test_bsl_hashtable_const_iterator_operator_notequal_to_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_bsl_hashtable_const_iterator_operator_notequal_to_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_bsl_hashtable_const_iterator_operator_notequal_to_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_const_iterator_operator_notequal_to_suite, case_name1)
{
	//TODO
}
 
/**
 * @brief 
**/
class test_bsl_hashtable_const_iterator_operator_plusplus_suite : public ::testing::Test{
    protected:
        test_bsl_hashtable_const_iterator_operator_plusplus_suite(){};
        virtual ~test_bsl_hashtable_const_iterator_operator_plusplus_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_bsl_hashtable_const_iterator_operator_plusplus_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_bsl_hashtable_const_iterator_operator_plusplus_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_const_iterator_operator_plusplus_suite, case_name1)
{
	//TODO
}
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_const_iterator_operator_plusplus_suite, case_name2)
{
	//TODO
}
 
/**
 * @brief 
**/
class test_bsl_hashtable_bsl_hashtable_suite : public ::testing::Test{
    protected:
        test_bsl_hashtable_bsl_hashtable_suite(){};
        virtual ~test_bsl_hashtable_bsl_hashtable_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_bsl_hashtable_bsl_hashtable_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_bsl_hashtable_bsl_hashtable_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_bsl_hashtable_suite, case_name1)
{
	//TODO
}
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_bsl_hashtable_suite, case_name2)
{
	//TODO
}
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_bsl_hashtable_suite, case_name3)
{
	//TODO
}
 
/**
 * @brief 
**/
class test_bsl_hashtable_operator_equal_suite : public ::testing::Test{
    protected:
        test_bsl_hashtable_operator_equal_suite(){};
        virtual ~test_bsl_hashtable_operator_equal_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_bsl_hashtable_operator_equal_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_bsl_hashtable_operator_equal_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_operator_equal_suite, case_name1)
{
	//TODO
}
 
/**
 * @brief 
**/
class test_bsl_hashtable_create_suite : public ::testing::Test{
    protected:
        test_bsl_hashtable_create_suite(){};
        virtual ~test_bsl_hashtable_create_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_bsl_hashtable_create_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_bsl_hashtable_create_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_create_suite, case_name1)
{
	//TODO
}
 
/**
 * @brief 
**/
class test_bsl_hashtable_is_created_suite : public ::testing::Test{
    protected:
        test_bsl_hashtable_is_created_suite(){};
        virtual ~test_bsl_hashtable_is_created_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_bsl_hashtable_is_created_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_bsl_hashtable_is_created_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_is_created_suite, case_name1)
{
	//TODO
}
 
/**
 * @brief 
**/
class test_bsl_hashtable_destroy_suite : public ::testing::Test{
    protected:
        test_bsl_hashtable_destroy_suite(){};
        virtual ~test_bsl_hashtable_destroy_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_bsl_hashtable_destroy_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_bsl_hashtable_destroy_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_destroy_suite, case_name1)
{
	//TODO
}
 
/**
 * @brief 
**/
class test_bsl_hashtable_begin_suite : public ::testing::Test{
    protected:
        test_bsl_hashtable_begin_suite(){};
        virtual ~test_bsl_hashtable_begin_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_bsl_hashtable_begin_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_bsl_hashtable_begin_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_begin_suite, case_name1)
{
	//TODO
}
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_begin_suite, case_name2)
{
	//TODO
}
 
/**
 * @brief 
**/
class test_bsl_hashtable_end_suite : public ::testing::Test{
    protected:
        test_bsl_hashtable_end_suite(){};
        virtual ~test_bsl_hashtable_end_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_bsl_hashtable_end_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_bsl_hashtable_end_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_end_suite, case_name1)
{
	//TODO
}
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_end_suite, case_name2)
{
	//TODO
}
 
/**
 * @brief 
**/
class test_bsl_hashtable_size_suite : public ::testing::Test{
    protected:
        test_bsl_hashtable_size_suite(){};
        virtual ~test_bsl_hashtable_size_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_bsl_hashtable_size_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_bsl_hashtable_size_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_size_suite, case_name1)
{
	//TODO
}
 
/**
 * @brief 
**/
class test_bsl_hashtable_count_suite : public ::testing::Test{
    protected:
        test_bsl_hashtable_count_suite(){};
        virtual ~test_bsl_hashtable_count_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_bsl_hashtable_count_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_bsl_hashtable_count_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_count_suite, case_null)
{
	//TODO
	hash_type names;
	ASSERT_EQ(0, names.count(hash_func()("Bob"), "Bob"));
}

/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_count_suite, test_count__EXIST)
{
	//TODO
	hash_type names;
	if(names.create(5) != 0)
	{
		printf("create hash_multimap error with size %d\n", 5);
	}

	names.set_multimap(hash_func()("Bob"),  "Bob", 1, 1);
	names.set_multimap(hash_func()("Bob"),  "Bob", 2, 1);
	names.set_multimap(hash_func()("Bob"),  "Bob", 3, 1);
	names.set_multimap(hash_func()("Bob"),  "Bob", 3, 1);
	names.set_multimap(hash_func()("Jack"),  "Jack", 1, 1);
	names.set_multimap(hash_func()("Jones"),  "Jones", 1, 1);
	names.set_multimap(hash_func()("Black"),  "Black", 1, 1);
	names.set_multimap(hash_func()("White"),  "White", 1, 1);
	names.set_multimap(hash_func()("Li"),  "Li", 1, 1);
	names.set_multimap(hash_func()("Li"),  "Li", 2, 1);
	names.set_multimap(hash_func()("Zhang"),  "Zhang", 1, 1);

	ASSERT_EQ(4, names.count(hash_func()("Bob"), "Bob"));
	ASSERT_EQ(1, names.count(hash_func()("Jack"), "Jack"));
	ASSERT_EQ(1, names.count(hash_func()("Jones"), "Jones"));
	ASSERT_EQ(1, names.count(hash_func()("Black"), "Black"));
	ASSERT_EQ(1, names.count(hash_func()("White"), "White"));
	ASSERT_EQ(2, names.count(hash_func()("Li"), "Li"));
	ASSERT_EQ(1, names.count(hash_func()("Zhang"), "Zhang"));
}

/**
 * @brief
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_count_suite, test_count__NOEXIST)
{
	//TODO
	hash_type names;
	if(names.create(5) != 0)
	{
		printf("create hash_multimap error with size %d\n", 5);
	}

	ASSERT_EQ(0, names.count(hash_func()("Jack"), "Jack"));

	names.set_multimap(hash_func()("Bob"),  "Bob", 1, 1);
	names.set_multimap(hash_func()("Bob"),  "Bob", 2, 1);
	names.set_multimap(hash_func()("Bob"),  "Bob", 3, 1);
	names.set_multimap(hash_func()("Bob"),  "Bob", 3, 1);
	names.set_multimap(hash_func()("Jack"),  "Jack", 1, 1);
	names.set_multimap(hash_func()("Jones"),  "Jones", 1, 1);
	names.set_multimap(hash_func()("Black"),  "Black", 1, 1);
	names.set_multimap(hash_func()("White"),  "White", 1, 1);
	names.set_multimap(hash_func()("Li"),  "Li", 1, 1);
	names.set_multimap(hash_func()("Li"),  "Li", 2, 1);
	names.set_multimap(hash_func()("Zhang"),  "Zhang", 1, 1);
	
	ASSERT_EQ(0, names.count(hash_func()("Jack1"), "Jack1"));
	ASSERT_EQ(0, names.count(hash_func()("White1"), "White1"));
}

/**
 * @brief 
**/
class test_bsl_hashtable_assign_suite : public ::testing::Test{
    protected:
        test_bsl_hashtable_assign_suite(){};
        virtual ~test_bsl_hashtable_assign_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_bsl_hashtable_assign_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_bsl_hashtable_assign_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_assign_suite, case_name1)
{
	//TODO
}
 
/**
 * @brief 
**/
class test_bsl_hashtable_assign_multimap_suite : public ::testing::Test{
    protected:
        test_bsl_hashtable_assign_multimap_suite(){};
        virtual ~test_bsl_hashtable_assign_multimap_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_bsl_hashtable_assign_multimap_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_bsl_hashtable_assign_multimap_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_assign_multimap_suite, case_name_from_stlmap)
{
	//TODO
	multimap<_key, _value> st;
	st.insert(_Pair("Bob", 1));
	st.insert(_Pair("Bob", 2));
	st.insert(_Pair("Bob", 3));
	st.insert(_Pair("Jack", 1));
	st.insert(_Pair("Jones", 1));
	st.insert(_Pair("Black", 1));
	st.insert(_Pair("Li", 1));
	st.insert(_Pair("Zhang", 1));
	st.insert(_Pair("Zhang", 4));
	st.insert(_Pair("White", 1));

	hash_type names_assign;
	ASSERT_EQ(0, names_assign.assign_multimap(st.begin(), st.end()));
}

/**
 * @brief
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_assign_multimap_suite, case_name_unequal_from_bsl_multimap)
{
	hash_type names_equal;
	names_equal.set_multimap(hash_func()("Bob"),  "Bob", 1, 1);
	names_equal.set_multimap(hash_func()("Bob"),  "Bob", 1, 1);
	names_equal.set_multimap(hash_func()("Bob"),  "Bob", 2, 1);
	names_equal.set_multimap(hash_func()("Bob"),  "Bob", 3, 1);
	names_equal.set_multimap(hash_func()("Bob"),  "Bob", 3, 1);
	names_equal.set_multimap(hash_func()("Jack"),  "Jack", 1, 1);
	names_equal.set_multimap(hash_func()("Jones"),  "Jones", 1, 1);
	names_equal.set_multimap(hash_func()("Black"),  "Black", 1, 1);
	names_equal.set_multimap(hash_func()("White"),  "White", 1, 1);
	names_equal.set_multimap(hash_func()("Li"),  "Li", 1, 1);
	names_equal.set_multimap(hash_func()("Li"),  "Li", 2, 1);
	names_equal.set_multimap(hash_func()("Zhang"),  "Zhang", 1, 1);

	hash_type names_assign;
	ASSERT_EQ(0, names_assign.assign_multimap(names_equal.begin(), names_equal.end()));
}

/**
 * @brief
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_assign_multimap_suite, case_name_equal_from_bsl_multimap)
{
	hash_type names_equal;

	names_equal.set_multimap(hash_func()("Bob"),  "Bob", 1, 1);
	names_equal.set_multimap(hash_func()("Bob"),  "Bob", 2, 1);
	names_equal.set_multimap(hash_func()("Bob"),  "Bob", 3, 1);
	names_equal.set_multimap(hash_func()("Bob"),  "Bob", 3, 1);
	names_equal.set_multimap(hash_func()("Jack"),  "Jack", 1, 1);
	names_equal.set_multimap(hash_func()("Jones"),  "Jones", 1, 1);
	names_equal.set_multimap(hash_func()("Black"),  "Black", 1, 1);
	names_equal.set_multimap(hash_func()("White"),  "White", 1, 1);
	names_equal.set_multimap(hash_func()("Li"),  "Li", 1, 1);
	names_equal.set_multimap(hash_func()("Li"),  "Li", 2, 1);
	names_equal.set_multimap(hash_func()("Zhang"),  "Zhang", 1, 1);

	hash_type names_assign;
	ASSERT_EQ(0, names_assign.assign_multimap(names_equal.begin(), names_equal.end(), 1));
}

/**
 * @brief 
**/
class test_bsl_hashtable_find_suite : public ::testing::Test{
    protected:
        test_bsl_hashtable_find_suite(){};
        virtual ~test_bsl_hashtable_find_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_bsl_hashtable_find_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_bsl_hashtable_find_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_find_suite, case_name1)
{
	//TODO
}
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_find_suite, case_name2)
{
	//TODO
}
 
/**
 * @brief 
**/
class test_bsl_hashtable_find_pair_suite : public ::testing::Test{
    protected:
        test_bsl_hashtable_find_pair_suite(){};
        virtual ~test_bsl_hashtable_find_pair_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_bsl_hashtable_find_pair_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_bsl_hashtable_find_pair_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_find_pair_suite, case_null)
{
	//TODO
	hash_type names_find;
	ASSERT_EQ(-1, names_find.find_pair(hash_func()("Li"),  "Li", 2));
}
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_find_pair_suite, case_exist)
{
	//TODO
	hash_type names_find;
	if(names_find.create(5) != 0)
	{
		printf("create hash_multimap error with size %d\n", 5);
	}

	names_find.set_multimap(hash_func()("Bob"),  "Bob", 1, 1);
	names_find.set_multimap(hash_func()("Bob"),  "Bob", 2, 1);
	names_find.set_multimap(hash_func()("Bob"),  "Bob", 3, 1);
	names_find.set_multimap(hash_func()("Bob"),  "Bob", 3, 1);
	names_find.set_multimap(hash_func()("Jack"),  "Jack", 1, 1);
	names_find.set_multimap(hash_func()("Jones"),  "Jones", 1, 1);
	names_find.set_multimap(hash_func()("Black"),  "Black", 1, 1);
	names_find.set_multimap(hash_func()("White"),  "White", 1, 1);
	names_find.set_multimap(hash_func()("Li"),  "Li", 1, 1);
	names_find.set_multimap(hash_func()("Li"),  "Li", 2, 1);
	names_find.set_multimap(hash_func()("Zhang"),  "Zhang", 1, 1);

	ASSERT_EQ(bsl::HASH_EXIST, names_find.find_pair(hash_func()("Bob"), "Bob", 1)); 
	ASSERT_EQ(bsl::HASH_EXIST, names_find.find_pair(hash_func()("Bob"), "Bob", 2)); 
	ASSERT_EQ(bsl::HASH_EXIST, names_find.find_pair(hash_func()("Bob"), "Bob", 3)); 
	ASSERT_EQ(bsl::HASH_EXIST, names_find.find_pair(hash_func()("Jack"), "Jack", 1)); 
	ASSERT_EQ(bsl::HASH_EXIST, names_find.find_pair(hash_func()("Jones"), "Jones", 1)); 
	ASSERT_EQ(bsl::HASH_EXIST, names_find.find_pair(hash_func()("Black"), "Black", 1)); 
	ASSERT_EQ(bsl::HASH_EXIST, names_find.find_pair(hash_func()("White"), "White", 1)); 
	ASSERT_EQ(bsl::HASH_EXIST, names_find.find_pair(hash_func()("Li"), "Li", 1)); 
	ASSERT_EQ(bsl::HASH_EXIST, names_find.find_pair(hash_func()("Li"), "Li", 2)); 
	ASSERT_EQ(bsl::HASH_EXIST, names_find.find_pair(hash_func()("Zhang"), "Zhang", 1)); 
}

/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_find_pair_suite, case_no_exist)
{
	hash_type names_find;
	if(names_find.create(5) != 0)
	{
		printf("create hash_multimap error with size %d\n", 5);
	}
	ASSERT_EQ(bsl::HASH_NOEXIST, names_find.find_pair(hash_func()("Bob"), "Bob", 1));
	names_find.set_multimap(hash_func()("Bob"),  "Bob", 1, 1);
	names_find.set_multimap(hash_func()("Bob"),  "Bob", 2, 1);
	names_find.set_multimap(hash_func()("Bob"),  "Bob", 3, 1);
	names_find.set_multimap(hash_func()("Bob"),  "Bob", 3, 1);
	names_find.set_multimap(hash_func()("Jack"),  "Jack", 1, 1);
	names_find.set_multimap(hash_func()("Jones"),  "Jones", 1, 1);
	names_find.set_multimap(hash_func()("Black"),  "Black", 1, 1);
	names_find.set_multimap(hash_func()("White"),  "White", 1, 1);
	names_find.set_multimap(hash_func()("Li"),  "Li", 1, 1);
	names_find.set_multimap(hash_func()("Li"),  "Li", 2, 1);
	names_find.set_multimap(hash_func()("Zhang"),  "Zhang", 1, 1);
	ASSERT_EQ(bsl::HASH_NOEXIST, names_find.find_pair(hash_func()("Bob"), "Bob", 4));
	ASSERT_EQ(bsl::HASH_NOEXIST, names_find.find_pair(hash_func()("Jack"), "Jack", 0));
	ASSERT_EQ(bsl::HASH_NOEXIST, names_find.find_pair(hash_func()("Li"), "Li", 5));
	ASSERT_EQ(bsl::HASH_NOEXIST, names_find.find_pair(hash_func()("Li1"), "Li1", 1));
}
 
/**
 * @brief 
**/
class test_bsl_hashtable_find_multimap_suite : public ::testing::Test{
    protected:
        test_bsl_hashtable_find_multimap_suite(){};
        virtual ~test_bsl_hashtable_find_multimap_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_bsl_hashtable_find_multimap_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_bsl_hashtable_find_multimap_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_find_multimap_suite, case_null)
{
	//TODO
	hash_type names_find;

	ASSERT_EQ(_iterator_pair(names_find.end(),names_find.end()), names_find.find_multimap(hash_func()("Li"), "Li"));

	if(names_find.create(5) != 0)
	{
		printf("create hash_multimap error with size %d\n", 5);
	}
	ASSERT_EQ(_iterator_pair(names_find.end(),names_find.end()), names_find.find_multimap(hash_func()("Li"), "Li"));
}
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_find_multimap_suite, case_noexist)
{
	//TODO
	hash_type names_find;
	if(names_find.create(5) != 0)
	{
		printf("create hash_multimap error with size %d\n", 5);
	}

	ASSERT_EQ(_iterator_pair(names_find.end(),names_find.end()), names_find.find_multimap(hash_func()("Li"), "Li"));

	names_find.set_multimap(hash_func()("Bob"),  "Bob", 1, 1);
	names_find.set_multimap(hash_func()("Bob"),  "Bob", 2, 1);
	names_find.set_multimap(hash_func()("Bob"),  "Bob", 3, 1);
	names_find.set_multimap(hash_func()("Bob"),  "Bob", 3, 1);
	names_find.set_multimap(hash_func()("Jack"),  "Jack", 1, 1);
	names_find.set_multimap(hash_func()("Jones"),  "Jones", 1, 1);
	names_find.set_multimap(hash_func()("Black"),  "Black", 1, 1);
	names_find.set_multimap(hash_func()("White"),  "White", 1, 1);
	names_find.set_multimap(hash_func()("Li"),  "Li", 1, 1);
	names_find.set_multimap(hash_func()("Li"),  "Li", 2, 1);
	names_find.set_multimap(hash_func()("Zhang"),  "Zhang", 1, 1);

	ASSERT_EQ(_iterator_pair(names_find.end(),names_find.end()), names_find.find_multimap(hash_func()("Li1"), "Li1"));
	ASSERT_EQ(_iterator_pair(names_find.end(),names_find.end()), names_find.find_multimap(hash_func()("Boc"), "Boc"));
}

/**
 * @brief
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_find_multimap_suite, case_exist)
{
	hash_type names_find;
	if(names_find.create(5) != 0)
	{
		printf("create hash_multimap error with size %d\n", 5);
	}

	names_find.set_multimap(hash_func()("Bob"),  "Bob", 1, 1);
	names_find.set_multimap(hash_func()("Bob"),  "Bob", 2, 1);
	names_find.set_multimap(hash_func()("Bob"),  "Bob", 3, 1);
	names_find.set_multimap(hash_func()("Bob"),  "Bob", 3, 1);
	names_find.set_multimap(hash_func()("Jack"),  "Jack", 1, 1);
	names_find.set_multimap(hash_func()("Jones"),  "Jones", 1, 1);
	names_find.set_multimap(hash_func()("Black"),  "Black", 1, 1);
	names_find.set_multimap(hash_func()("White"),  "White", 1, 1);
	names_find.set_multimap(hash_func()("Li"),  "Li", 1, 1);
	names_find.set_multimap(hash_func()("Li"),  "Li", 2, 1);
	names_find.set_multimap(hash_func()("Zhang"),  "Zhang", 1, 1);

	//手工查找range
	_iterator __begin;
	_iterator __end;
	_iterator_pair __ite_pair;

	print_hash(names_find.begin(), names_find.end());

	printf("begin:\n");
	__begin = begin_key(names_find, "Bob");
	printf("end:\n");
	__end = end_key(names_find, "Bob");
	printf("equal:\n");
	EXPECT_EQ(_iterator_pair(__begin, __end), names_find.find_multimap(hash_func()("Bob"), "Bob"));
	
	printf("begin:\n");
	__begin = begin_key(names_find, "Jack");
	printf("end:\n");
	__end = end_key(names_find, "Jack");
	printf("equal:\n");
	EXPECT_EQ(_iterator_pair(__begin, __end), names_find.find_multimap(hash_func()("Jack"), "Jack"));

	__begin = begin_key(names_find, "Jones");
	__end = end_key(names_find, "Jones");
	EXPECT_EQ(_iterator_pair(__begin, __end), names_find.find_multimap(hash_func()("Jones"), "Jones"));

	__begin = begin_key(names_find, "Black");
	__end = end_key(names_find, "Black");
	EXPECT_EQ(_iterator_pair(__begin, __end), names_find.find_multimap(hash_func()("Black"), "Black"));

	__begin = begin_key(names_find, "White");
	__end = end_key(names_find, "White");
	EXPECT_EQ(_iterator_pair(__begin, __end), names_find.find_multimap(hash_func()("White"), "White"));

	__begin = begin_key(names_find, "Li");
	__end = end_key(names_find, "Li");
	EXPECT_EQ(_iterator_pair(__begin, __end), names_find.find_multimap(hash_func()("Li"), "Li"));

	__begin = begin_key(names_find, "Zhang");
	__end = end_key(names_find, "Zhang");
	EXPECT_EQ(_iterator_pair(__begin, __end), names_find.find_multimap(hash_func()("Zhang"), "Zhang"));
}
 
/**
 * @brief 
**/
class test_bsl_hashtable_set_suite : public ::testing::Test{
    protected:
        test_bsl_hashtable_set_suite(){};
        virtual ~test_bsl_hashtable_set_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_bsl_hashtable_set_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_bsl_hashtable_set_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_set_suite, case_name1)
{
	//TODO
}
 
/**
 * @brief 
**/
class test_bsl_hashtable_set_map_suite : public ::testing::Test{
    protected:
        test_bsl_hashtable_set_map_suite(){};
        virtual ~test_bsl_hashtable_set_map_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_bsl_hashtable_set_map_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_bsl_hashtable_set_map_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_set_map_suite, case_name1)
{
	//TODO
}
 
/**
 * @brief 
**/
class test_bsl_hashtable_set_multimap_suite : public ::testing::Test{
    protected:
        test_bsl_hashtable_set_multimap_suite(){};
        virtual ~test_bsl_hashtable_set_multimap_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_bsl_hashtable_set_multimap_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_bsl_hashtable_set_multimap_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_set_multimap_suite, case_null)
{
	//TODO
	hash_type names;
	ASSERT_EQ( -1, names.set_multimap(hash_func()("Jack"), "Jack", 1) );
	ASSERT_EQ( -1, names.set_multimap(hash_func()("Jack"), "Jack", 1, 1) );
}

/**
 * @brief
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_set_multimap_suite, case_unequal)
{
	hash_type names;
	if(names.create(5) != 0)
	{
		printf("create hash_multimap error with size %d\n", 5);
	}
	ASSERT_EQ( bsl::HASH_INSERT_SEC, names.set_multimap(hash_func()("Bob"), "Bob", 1) );
	ASSERT_EQ( bsl::HASH_EXIST, names.set_multimap(hash_func()("Bob"), "Bob", 1) );
	ASSERT_EQ( bsl::HASH_INSERT_SEC, names.set_multimap(hash_func()("Bob"), "Bob", 2) );
	ASSERT_EQ( bsl::HASH_INSERT_SEC, names.set_multimap(hash_func()("Jack"), "Jack", 1) );
	ASSERT_EQ( bsl::HASH_INSERT_SEC, names.set_multimap(hash_func()("Jones"), "Jones", 1) );
	ASSERT_EQ( bsl::HASH_INSERT_SEC, names.set_multimap(hash_func()("Black"), "Black", 1) );
	ASSERT_EQ( bsl::HASH_EXIST, names.set_multimap(hash_func()("Black"), "Black", 1) );
	ASSERT_EQ( bsl::HASH_INSERT_SEC, names.set_multimap(hash_func()("White"), "White", 1) );
	ASSERT_EQ( bsl::HASH_INSERT_SEC, names.set_multimap(hash_func()("White"), "White", 2) );
	ASSERT_EQ( bsl::HASH_INSERT_SEC, names.set_multimap(hash_func()("Li"), "Li", 1) );
	ASSERT_EQ( bsl::HASH_INSERT_SEC, names.set_multimap(hash_func()("Zhang"), "Zhang", 1) );
}

/**
 * @brief
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_set_multimap_suite, case_equal)
{
	hash_type names;
	if(names.create(5) != 0)
	{

		printf("create hash_multimap error with size %d\n", 5);
	}
	ASSERT_EQ( bsl::HASH_INSERT_SEC, names.set_multimap(hash_func()("Bob"), "Bob", 1, 1) );
	ASSERT_EQ( bsl::HASH_INSERT_SEC, names.set_multimap(hash_func()("Bob"), "Bob", 1, 1) );
	ASSERT_EQ( bsl::HASH_INSERT_SEC, names.set_multimap(hash_func()("Bob"), "Bob", 2, 1) );
	ASSERT_EQ( bsl::HASH_INSERT_SEC, names.set_multimap(hash_func()("Jack"), "Jack", 1, 1) );
	ASSERT_EQ( bsl::HASH_INSERT_SEC, names.set_multimap(hash_func()("Jones"), "Jones", 1, 1) );
	ASSERT_EQ( bsl::HASH_INSERT_SEC, names.set_multimap(hash_func()("Black"), "Black", 1, 1) );
	ASSERT_EQ( bsl::HASH_INSERT_SEC, names.set_multimap(hash_func()("Black"), "Black", 1, 1) );
	ASSERT_EQ( bsl::HASH_INSERT_SEC, names.set_multimap(hash_func()("White"), "White", 1, 1) );
	ASSERT_EQ( bsl::HASH_INSERT_SEC, names.set_multimap(hash_func()("White"), "White", 2, 1) );
	ASSERT_EQ( bsl::HASH_INSERT_SEC, names.set_multimap(hash_func()("Li"), "Li", 1, 1) );
	ASSERT_EQ( bsl::HASH_INSERT_SEC, names.set_multimap(hash_func()("Zhang"), "Zhang", 1, 1) );
}

/**
 * @brief 
**/
class test_bsl_hashtable_erase_suite : public ::testing::Test{
    protected:
        test_bsl_hashtable_erase_suite(){};
        virtual ~test_bsl_hashtable_erase_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_bsl_hashtable_erase_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_bsl_hashtable_erase_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_erase_suite, case_name1)
{
	//TODO
}
 
/**
 * @brief 
**/
class test_bsl_hashtable_erase_multimap_suite : public ::testing::Test{
    protected:
        test_bsl_hashtable_erase_multimap_suite(){};
        virtual ~test_bsl_hashtable_erase_multimap_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_bsl_hashtable_erase_multimap_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_bsl_hashtable_erase_multimap_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_erase_multimap_suite, case_null)
{
	//TODO
	hash_type names;
	ASSERT_EQ(-1, names.erase_multimap(hash_func()("Bob"), "Bob"));
}

/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_erase_multimap_suite, case_exist)
{
	hash_type names;
	if(names.create(5) != 0)
	{
		printf("create hash_multimap error with size %d\n", 5);
	}

	names.set_multimap(hash_func()("Bob"),  "Bob", 1, 1);
	names.set_multimap(hash_func()("Bob"),  "Bob", 2, 1);
	names.set_multimap(hash_func()("Bob"),  "Bob", 3, 1);
	names.set_multimap(hash_func()("Bob"),  "Bob", 3, 1);
	names.set_multimap(hash_func()("Jack"),  "Jack", 1, 1);
	names.set_multimap(hash_func()("Jones"),  "Jones", 1, 1);
	names.set_multimap(hash_func()("Black"),  "Black", 1, 1);
	names.set_multimap(hash_func()("White"),  "White", 1, 1);
	names.set_multimap(hash_func()("Li"),  "Li", 1, 1);
	names.set_multimap(hash_func()("Li"),  "Li", 2, 1);
	names.set_multimap(hash_func()("Zhang"),  "Zhang", 1, 1);

	ASSERT_EQ(4, names.erase_multimap(hash_func()("Bob"), "Bob"));
	ASSERT_EQ(1, names.erase_multimap(hash_func()("Jack"), "Jack"));
	ASSERT_EQ(1, names.erase_multimap(hash_func()("Jones"), "Jones"));
	ASSERT_EQ(1, names.erase_multimap(hash_func()("Black"), "Black"));
	ASSERT_EQ(1, names.erase_multimap(hash_func()("White"), "White"));
	ASSERT_EQ(2, names.erase_multimap(hash_func()("Li"), "Li"));
	ASSERT_EQ(1, names.erase_multimap(hash_func()("Zhang"), "Zhang"));
}

/**
 * @brief
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_erase_multimap_suite, case_no_exist)
{
	hash_type names;
	if(names.create(5) != 0)
	{
		printf("create hash_multimap error with size %d\n", 5);
	}

	names.set_multimap(hash_func()("Bob"),  "Bob", 1, 1);
	names.set_multimap(hash_func()("Bob"),  "Bob", 2, 1);
	names.set_multimap(hash_func()("Bob"),  "Bob", 3, 1);
	names.set_multimap(hash_func()("Bob"),  "Bob", 3, 1);
	names.set_multimap(hash_func()("Jack"),  "Jack", 1, 1);
	names.set_multimap(hash_func()("Jones"),  "Jones", 1, 1);
	names.set_multimap(hash_func()("Black"),  "Black", 1, 1);
	names.set_multimap(hash_func()("White"),  "White", 1, 1);
	names.set_multimap(hash_func()("Li"),  "Li", 1, 1);
	names.set_multimap(hash_func()("Li"),  "Li", 2, 1);
	names.set_multimap(hash_func()("Zhang"),  "Zhang", 1, 1);

	ASSERT_EQ(0, names.erase_multimap(hash_func()("Bob1"), "Bob1"));
	ASSERT_EQ(0, names.erase_multimap(hash_func()("Li1"), "Li1"));
}
 
/**
 * @brief 
**/
class test_bsl_hashtable_erase_pair_suite : public ::testing::Test{
    protected:
        test_bsl_hashtable_erase_pair_suite(){};
        virtual ~test_bsl_hashtable_erase_pair_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_bsl_hashtable_erase_pair_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_bsl_hashtable_erase_pair_suite, *)
        };
};
 
/**
 * @brief
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_erase_pair_suite, case_null)
{
	hash_type names;
	ASSERT_EQ(-1, names.erase_pair(hash_func()("Bob"), "Bob", 1));
}

/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_erase_pair_suite, case_exist)
{
	//TODO
	hash_type names;
	if(names.create(5) != 0)
	{
		printf("create hash_multimap error with size %d\n", 5);
	}

	names.set_multimap(hash_func()("Bob"),  "Bob", 1, 1);
	names.set_multimap(hash_func()("Bob"),  "Bob", 2, 1);
	names.set_multimap(hash_func()("Bob"),  "Bob", 3, 1);
	names.set_multimap(hash_func()("Bob"),  "Bob", 3, 1);
	names.set_multimap(hash_func()("Jack"),  "Jack", 1, 1);
	names.set_multimap(hash_func()("Jones"),  "Jones", 1, 1);
	names.set_multimap(hash_func()("Black"),  "Black", 1, 1);
	names.set_multimap(hash_func()("White"),  "White", 1, 1);
	names.set_multimap(hash_func()("Li"),  "Li", 1, 1);
	names.set_multimap(hash_func()("Li"),  "Li", 2, 1);
	names.set_multimap(hash_func()("Zhang"),  "Zhang", 1, 1);

	ASSERT_EQ(1 ,names.erase_pair(hash_func()("Bob"), "Bob", 2));
	ASSERT_EQ(1 ,names.erase_pair(hash_func()("Zhang"), "Zhang", 1));
	ASSERT_EQ(1 ,names.erase_pair(hash_func()("Black"), "Black", 1));
	ASSERT_EQ(2 ,names.erase_pair(hash_func()("Bob"), "Bob", 3));
	ASSERT_EQ(1 ,names.erase_pair(hash_func()("Li"), "Li", 2));
}

/**
 * @brief
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_erase_pair_suite, case_no_exist)
{
	hash_type names;
	if(names.create(5) != 0)
	{
		printf("create hash_multimap error with size %d\n", 5);
	}
	ASSERT_EQ(0 ,names.erase_pair(hash_func()("Bob"), "Bob", 1));

	names.set_multimap(hash_func()("Bob"),  "Bob", 1, 1);
	names.set_multimap(hash_func()("Bob"),  "Bob", 2, 1);
	names.set_multimap(hash_func()("Bob"),  "Bob", 3, 1);
	names.set_multimap(hash_func()("Bob"),  "Bob", 3, 1);
	names.set_multimap(hash_func()("Jack"),  "Jack", 1, 1);
	names.set_multimap(hash_func()("Jones"),  "Jones", 1, 1);
	names.set_multimap(hash_func()("Black"),  "Black", 1, 1);
	names.set_multimap(hash_func()("White"),  "White", 1, 1);
	names.set_multimap(hash_func()("Li"),  "Li", 1, 1);
	names.set_multimap(hash_func()("Li"),  "Li", 2, 1);
	names.set_multimap(hash_func()("Zhang"),  "Zhang", 1, 1);

	ASSERT_EQ(0 ,names.erase_pair(hash_func()("Bob"), "Bob", 4));
	ASSERT_EQ(0 ,names.erase_pair(hash_func()("Bob1"), "Bob1", 2));
}
 
/**
 * @brief 
**/
class test_bsl_hashtable_serialization_suite : public ::testing::Test{
    protected:
        test_bsl_hashtable_serialization_suite(){};
        virtual ~test_bsl_hashtable_serialization_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_bsl_hashtable_serialization_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_bsl_hashtable_serialization_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_serialization_suite, case_name1)
{
	//TODO
}

/**
 * @brief 
**/
class test_bsl_hashtable_deserialization_multimap_suite : public ::testing::Test{
    protected:
        test_bsl_hashtable_deserialization_multimap_suite(){};
        virtual ~test_bsl_hashtable_deserialization_multimap_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_bsl_hashtable_deserialization_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_bsl_hashtable_deserialization_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_deserialization_multimap_suite, case_null)
{
	//TODO
	bsl::filestream fs;

	hash_type names_in;
	hash_type names_out;

	fs.open("archivefile","w+");
	fs.close();

	fs.open("archivefile","r");
	bsl::binarchive ar0(fs);
	ASSERT_EQ(-1, names_out.deserialization_multimap(ar0));
	fs.close();
		
	if(names_in.create(5) != 0)
	{
        	printf("create hash_multimap error with size %d\n", 5);
	}
	names_in.set_multimap(hash_func()("Bob"),  "Bob", 1, 1);
	names_in.set_multimap(hash_func()("Bob"),  "Bob", 2, 1);
	names_in.set_multimap(hash_func()("Bob"),  "Bob", 3, 1);
	names_in.set_multimap(hash_func()("Bob"),  "Bob", 3, 1);
	names_in.set_multimap(hash_func()("Jack"),  "Jack", 1, 1);
	names_in.set_multimap(hash_func()("Jones"),  "Jones", 1, 1);
	names_in.set_multimap(hash_func()("Black"),  "Black", 1, 1);
	names_in.set_multimap(hash_func()("White"),  "White", 1, 1);
	names_in.set_multimap(hash_func()("Li"),  "Li", 1, 1);
	names_in.set_multimap(hash_func()("Li"),  "Li", 2, 1);
	names_in.set_multimap(hash_func()("Zhang"),  "Zhang", 1, 1);
	
	cout << "Input:\n";
	print_hash(names_in.begin(),names_in.end());

	fs.open("archivefile","w+");
	bsl::binarchive ar(fs);
	serialization(ar,names_in);
	fs.close();

	fs.open("archivefile","r");
	bsl::binarchive ar1(fs);
	//deserialization(ar1,names_out);
	ASSERT_EQ(0, names_out.deserialization_multimap(ar1));
	fs.close();
	
	cout << "Output:\n";
	print_hash(names_out.begin(),names_out.end());

}
 
/**
 * @brief 
**/
class test_bsl_hashtable_clear_suite : public ::testing::Test{
    protected:
        test_bsl_hashtable_clear_suite(){};
        virtual ~test_bsl_hashtable_clear_suite(){};
        virtual void SetUp() {
            //Called befor every TEST_F(test_bsl_hashtable_clear_suite, *)
        };
        virtual void TearDown() {
            //Called after every TEST_F(test_bsl_hashtable_clear_suite, *)
        };
};
 
/**
 * @brief 
 * @begin_version 1.1.15.0
**/
TEST_F(test_bsl_hashtable_clear_suite, case_name1)
{
	//TODO
}
 
