
#ifndef __BSL_TEST_STRING_H_
#define __BSL_TEST_STRING_H_
#include <cxxtest/TestSuite.h>
#include "bsl_test_string.hpp"
class bsl_test_string_main : public CxxTest::TestSuite{
public:

    void test_normal_char_bsl_bsl_alloc_test_ctors(){
        return bsl_test_string<char, bsl::bsl_alloc<char> >().test_ctors();
    }


    void test_normal_char_bsl_bsl_alloc_test_c_str(){
        return bsl_test_string<char, bsl::bsl_alloc<char> >().test_c_str();
    }


    void test_normal_char_bsl_bsl_alloc_test_empty(){
        return bsl_test_string<char, bsl::bsl_alloc<char> >().test_empty();
    }


    void test_normal_char_bsl_bsl_alloc_test_size(){
        return bsl_test_string<char, bsl::bsl_alloc<char> >().test_size();
    }


    void test_normal_char_bsl_bsl_alloc_test_length(){
        return bsl_test_string<char, bsl::bsl_alloc<char> >().test_length();
    }


    void test_normal_char_bsl_bsl_alloc_test_capacity(){
        return bsl_test_string<char, bsl::bsl_alloc<char> >().test_capacity();
    }


    void test_normal_char_bsl_bsl_alloc_test_operator_square(){
        return bsl_test_string<char, bsl::bsl_alloc<char> >().test_operator_square();
    }


    void test_normal_char_bsl_bsl_alloc_test_clear(){
        return bsl_test_string<char, bsl::bsl_alloc<char> >().test_clear();
    }


    void test_normal_char_bsl_bsl_alloc_test_reserve(){
        return bsl_test_string<char, bsl::bsl_alloc<char> >().test_reserve();
    }


    void test_normal_char_bsl_bsl_alloc_test_swap(){
        return bsl_test_string<char, bsl::bsl_alloc<char> >().test_swap();
    }


    void test_normal_char_bsl_bsl_alloc_test_operator_assign_to_null(){
        return bsl_test_string<char, bsl::bsl_alloc<char> >().test_operator_assign_to_null();
    }


    void test_normal_char_bsl_bsl_alloc_test_operator_assign_to_cstring(){
        return bsl_test_string<char, bsl::bsl_alloc<char> >().test_operator_assign_to_cstring();
    }


    void test_normal_char_bsl_bsl_alloc_test_operator_assign_to_string(){
        return bsl_test_string<char, bsl::bsl_alloc<char> >().test_operator_assign_to_string();
    }


    void test_normal_char_bsl_bsl_alloc_test_operator_eq_eq(){
        return bsl_test_string<char, bsl::bsl_alloc<char> >().test_operator_eq_eq();
    }


    void test_normal_char_bsl_bsl_alloc_test_operator_not_eq(){
        return bsl_test_string<char, bsl::bsl_alloc<char> >().test_operator_not_eq();
    }


    void test_normal_char_bsl_bsl_alloc_test_operator_lt(){
        return bsl_test_string<char, bsl::bsl_alloc<char> >().test_operator_lt();
    }


    void test_normal_char_bsl_bsl_alloc_test_operator_gt(){
        return bsl_test_string<char, bsl::bsl_alloc<char> >().test_operator_gt();
    }


    void test_normal_char_bsl_bsl_alloc_test_operator_lt_eq(){
        return bsl_test_string<char, bsl::bsl_alloc<char> >().test_operator_lt_eq();
    }


    void test_normal_char_bsl_bsl_alloc_test_operator_gt_eq(){
        return bsl_test_string<char, bsl::bsl_alloc<char> >().test_operator_gt_eq();
    }


    void test_normal_char_bsl_bsl_alloc_test_operator_shift(){
        return bsl_test_string<char, bsl::bsl_alloc<char> >().test_operator_shift();
    }


    void test_normal_char_bsl_bsl_alloc_test_serialization(){
        return bsl_test_string<char, bsl::bsl_alloc<char> >().test_serialization();
    }


    void test_normal_char_bsl_bsl_alloc_test_append_str(){
        return bsl_test_string<char, bsl::bsl_alloc<char> >().test_append_str();
    }


    void test_normal_char_bsl_bsl_alloc_test_append_sub_str(){
        return bsl_test_string<char, bsl::bsl_alloc<char> >().test_append_sub_str();
    }


    void test_normal_char_bsl_bsl_alloc_test_append_cstr(){
        return bsl_test_string<char, bsl::bsl_alloc<char> >().test_append_cstr();
    }


    void test_normal_char_bsl_bsl_alloc_test_append_sub_cstr(){
        return bsl_test_string<char, bsl::bsl_alloc<char> >().test_append_sub_cstr();
    }


    void test_normal_char_bsl_bsl_alloc_test_append_n_char(){
        return bsl_test_string<char, bsl::bsl_alloc<char> >().test_append_n_char();
    }


    void test_normal_char_bsl_bsl_alloc_test_append_range(){
        return bsl_test_string<char, bsl::bsl_alloc<char> >().test_append_range();
    }


    void test_normal_char_bsl_bsl_alloc_test_push_back(){
        return bsl_test_string<char, bsl::bsl_alloc<char> >().test_push_back();
    }


    void test_normal_char_bsl_bsl_alloc_test_appendf(){
        return bsl_test_string<char, bsl::bsl_alloc<char> >().test_appendf();
    }


    void test_normal_char_bsl_bsl_alloc_test_find(){
        return bsl_test_string<char, bsl::bsl_alloc<char> >().test_find();
    }


    void test_normal_char_bsl_bsl_alloc_test_rfind(){
        return bsl_test_string<char, bsl::bsl_alloc<char> >().test_rfind();
    }


    void test_normal_char_std_allocator_test_ctors(){
        return bsl_test_string<char, std::allocator<char> >().test_ctors();
    }


    void test_normal_char_std_allocator_test_c_str(){
        return bsl_test_string<char, std::allocator<char> >().test_c_str();
    }


    void test_normal_char_std_allocator_test_empty(){
        return bsl_test_string<char, std::allocator<char> >().test_empty();
    }


    void test_normal_char_std_allocator_test_size(){
        return bsl_test_string<char, std::allocator<char> >().test_size();
    }


    void test_normal_char_std_allocator_test_length(){
        return bsl_test_string<char, std::allocator<char> >().test_length();
    }


    void test_normal_char_std_allocator_test_capacity(){
        return bsl_test_string<char, std::allocator<char> >().test_capacity();
    }


    void test_normal_char_std_allocator_test_operator_square(){
        return bsl_test_string<char, std::allocator<char> >().test_operator_square();
    }


    void test_normal_char_std_allocator_test_clear(){
        return bsl_test_string<char, std::allocator<char> >().test_clear();
    }


    void test_normal_char_std_allocator_test_reserve(){
        return bsl_test_string<char, std::allocator<char> >().test_reserve();
    }


    void test_normal_char_std_allocator_test_swap(){
        return bsl_test_string<char, std::allocator<char> >().test_swap();
    }


    void test_normal_char_std_allocator_test_operator_assign_to_null(){
        return bsl_test_string<char, std::allocator<char> >().test_operator_assign_to_null();
    }


    void test_normal_char_std_allocator_test_operator_assign_to_cstring(){
        return bsl_test_string<char, std::allocator<char> >().test_operator_assign_to_cstring();
    }


    void test_normal_char_std_allocator_test_operator_assign_to_string(){
        return bsl_test_string<char, std::allocator<char> >().test_operator_assign_to_string();
    }


    void test_normal_char_std_allocator_test_operator_eq_eq(){
        return bsl_test_string<char, std::allocator<char> >().test_operator_eq_eq();
    }


    void test_normal_char_std_allocator_test_operator_not_eq(){
        return bsl_test_string<char, std::allocator<char> >().test_operator_not_eq();
    }


    void test_normal_char_std_allocator_test_operator_lt(){
        return bsl_test_string<char, std::allocator<char> >().test_operator_lt();
    }


    void test_normal_char_std_allocator_test_operator_gt(){
        return bsl_test_string<char, std::allocator<char> >().test_operator_gt();
    }


    void test_normal_char_std_allocator_test_operator_lt_eq(){
        return bsl_test_string<char, std::allocator<char> >().test_operator_lt_eq();
    }


    void test_normal_char_std_allocator_test_operator_gt_eq(){
        return bsl_test_string<char, std::allocator<char> >().test_operator_gt_eq();
    }


    void test_normal_char_std_allocator_test_operator_shift(){
        return bsl_test_string<char, std::allocator<char> >().test_operator_shift();
    }


    void test_normal_char_std_allocator_test_serialization(){
        return bsl_test_string<char, std::allocator<char> >().test_serialization();
    }


    void test_normal_char_std_allocator_test_append_str(){
        return bsl_test_string<char, std::allocator<char> >().test_append_str();
    }


    void test_normal_char_std_allocator_test_append_sub_str(){
        return bsl_test_string<char, std::allocator<char> >().test_append_sub_str();
    }


    void test_normal_char_std_allocator_test_append_cstr(){
        return bsl_test_string<char, std::allocator<char> >().test_append_cstr();
    }


    void test_normal_char_std_allocator_test_append_sub_cstr(){
        return bsl_test_string<char, std::allocator<char> >().test_append_sub_cstr();
    }


    void test_normal_char_std_allocator_test_append_n_char(){
        return bsl_test_string<char, std::allocator<char> >().test_append_n_char();
    }


    void test_normal_char_std_allocator_test_append_range(){
        return bsl_test_string<char, std::allocator<char> >().test_append_range();
    }


    void test_normal_char_std_allocator_test_push_back(){
        return bsl_test_string<char, std::allocator<char> >().test_push_back();
    }


    void test_normal_char_std_allocator_test_appendf(){
        return bsl_test_string<char, std::allocator<char> >().test_appendf();
    }


    void test_normal_char_std_allocator_test_find(){
        return bsl_test_string<char, std::allocator<char> >().test_find();
    }


    void test_normal_char_std_allocator_test_rfind(){
        return bsl_test_string<char, std::allocator<char> >().test_rfind();
    }


    void test_normal_wchar_t_bsl_bsl_alloc_test_ctors(){
        return bsl_test_string<wchar_t, bsl::bsl_alloc<wchar_t> >().test_ctors();
    }


    void test_normal_wchar_t_bsl_bsl_alloc_test_c_str(){
        return bsl_test_string<wchar_t, bsl::bsl_alloc<wchar_t> >().test_c_str();
    }


    void test_normal_wchar_t_bsl_bsl_alloc_test_empty(){
        return bsl_test_string<wchar_t, bsl::bsl_alloc<wchar_t> >().test_empty();
    }


    void test_normal_wchar_t_bsl_bsl_alloc_test_size(){
        return bsl_test_string<wchar_t, bsl::bsl_alloc<wchar_t> >().test_size();
    }


    void test_normal_wchar_t_bsl_bsl_alloc_test_length(){
        return bsl_test_string<wchar_t, bsl::bsl_alloc<wchar_t> >().test_length();
    }


    void test_normal_wchar_t_bsl_bsl_alloc_test_capacity(){
        return bsl_test_string<wchar_t, bsl::bsl_alloc<wchar_t> >().test_capacity();
    }


    void test_normal_wchar_t_bsl_bsl_alloc_test_operator_square(){
        return bsl_test_string<wchar_t, bsl::bsl_alloc<wchar_t> >().test_operator_square();
    }


    void test_normal_wchar_t_bsl_bsl_alloc_test_clear(){
        return bsl_test_string<wchar_t, bsl::bsl_alloc<wchar_t> >().test_clear();
    }


    void test_normal_wchar_t_bsl_bsl_alloc_test_reserve(){
        return bsl_test_string<wchar_t, bsl::bsl_alloc<wchar_t> >().test_reserve();
    }


    void test_normal_wchar_t_bsl_bsl_alloc_test_swap(){
        return bsl_test_string<wchar_t, bsl::bsl_alloc<wchar_t> >().test_swap();
    }


    void test_normal_wchar_t_bsl_bsl_alloc_test_operator_assign_to_null(){
        return bsl_test_string<wchar_t, bsl::bsl_alloc<wchar_t> >().test_operator_assign_to_null();
    }


    void test_normal_wchar_t_bsl_bsl_alloc_test_operator_assign_to_cstring(){
        return bsl_test_string<wchar_t, bsl::bsl_alloc<wchar_t> >().test_operator_assign_to_cstring();
    }


    void test_normal_wchar_t_bsl_bsl_alloc_test_operator_assign_to_string(){
        return bsl_test_string<wchar_t, bsl::bsl_alloc<wchar_t> >().test_operator_assign_to_string();
    }


    void test_normal_wchar_t_bsl_bsl_alloc_test_operator_eq_eq(){
        return bsl_test_string<wchar_t, bsl::bsl_alloc<wchar_t> >().test_operator_eq_eq();
    }


    void test_normal_wchar_t_bsl_bsl_alloc_test_operator_not_eq(){
        return bsl_test_string<wchar_t, bsl::bsl_alloc<wchar_t> >().test_operator_not_eq();
    }


    void test_normal_wchar_t_bsl_bsl_alloc_test_operator_lt(){
        return bsl_test_string<wchar_t, bsl::bsl_alloc<wchar_t> >().test_operator_lt();
    }


    void test_normal_wchar_t_bsl_bsl_alloc_test_operator_gt(){
        return bsl_test_string<wchar_t, bsl::bsl_alloc<wchar_t> >().test_operator_gt();
    }


    void test_normal_wchar_t_bsl_bsl_alloc_test_operator_lt_eq(){
        return bsl_test_string<wchar_t, bsl::bsl_alloc<wchar_t> >().test_operator_lt_eq();
    }


    void test_normal_wchar_t_bsl_bsl_alloc_test_operator_gt_eq(){
        return bsl_test_string<wchar_t, bsl::bsl_alloc<wchar_t> >().test_operator_gt_eq();
    }


    void test_normal_wchar_t_bsl_bsl_alloc_test_operator_shift(){
        return bsl_test_string<wchar_t, bsl::bsl_alloc<wchar_t> >().test_operator_shift();
    }


    void test_normal_wchar_t_bsl_bsl_alloc_test_serialization(){
        return bsl_test_string<wchar_t, bsl::bsl_alloc<wchar_t> >().test_serialization();
    }


    void test_normal_wchar_t_bsl_bsl_alloc_test_append_str(){
        return bsl_test_string<wchar_t, bsl::bsl_alloc<wchar_t> >().test_append_str();
    }


    void test_normal_wchar_t_bsl_bsl_alloc_test_append_sub_str(){
        return bsl_test_string<wchar_t, bsl::bsl_alloc<wchar_t> >().test_append_sub_str();
    }


    void test_normal_wchar_t_bsl_bsl_alloc_test_append_cstr(){
        return bsl_test_string<wchar_t, bsl::bsl_alloc<wchar_t> >().test_append_cstr();
    }


    void test_normal_wchar_t_bsl_bsl_alloc_test_append_sub_cstr(){
        return bsl_test_string<wchar_t, bsl::bsl_alloc<wchar_t> >().test_append_sub_cstr();
    }


    void test_normal_wchar_t_bsl_bsl_alloc_test_append_n_char(){
        return bsl_test_string<wchar_t, bsl::bsl_alloc<wchar_t> >().test_append_n_char();
    }


    void test_normal_wchar_t_bsl_bsl_alloc_test_append_range(){
        return bsl_test_string<wchar_t, bsl::bsl_alloc<wchar_t> >().test_append_range();
    }


    void test_normal_wchar_t_bsl_bsl_alloc_test_push_back(){
        return bsl_test_string<wchar_t, bsl::bsl_alloc<wchar_t> >().test_push_back();
    }


    void test_normal_wchar_t_bsl_bsl_alloc_test_appendf(){
        return bsl_test_string<wchar_t, bsl::bsl_alloc<wchar_t> >().test_appendf();
    }


    void test_normal_wchar_t_bsl_bsl_alloc_test_find(){
        return bsl_test_string<wchar_t, bsl::bsl_alloc<wchar_t> >().test_find();
    }


    void test_normal_wchar_t_bsl_bsl_alloc_test_rfind(){
        return bsl_test_string<wchar_t, bsl::bsl_alloc<wchar_t> >().test_rfind();
    }


    void test_normal_wchar_t_std_allocator_test_ctors(){
        return bsl_test_string<wchar_t, std::allocator<wchar_t> >().test_ctors();
    }


    void test_normal_wchar_t_std_allocator_test_c_str(){
        return bsl_test_string<wchar_t, std::allocator<wchar_t> >().test_c_str();
    }


    void test_normal_wchar_t_std_allocator_test_empty(){
        return bsl_test_string<wchar_t, std::allocator<wchar_t> >().test_empty();
    }


    void test_normal_wchar_t_std_allocator_test_size(){
        return bsl_test_string<wchar_t, std::allocator<wchar_t> >().test_size();
    }


    void test_normal_wchar_t_std_allocator_test_length(){
        return bsl_test_string<wchar_t, std::allocator<wchar_t> >().test_length();
    }


    void test_normal_wchar_t_std_allocator_test_capacity(){
        return bsl_test_string<wchar_t, std::allocator<wchar_t> >().test_capacity();
    }


    void test_normal_wchar_t_std_allocator_test_operator_square(){
        return bsl_test_string<wchar_t, std::allocator<wchar_t> >().test_operator_square();
    }


    void test_normal_wchar_t_std_allocator_test_clear(){
        return bsl_test_string<wchar_t, std::allocator<wchar_t> >().test_clear();
    }


    void test_normal_wchar_t_std_allocator_test_reserve(){
        return bsl_test_string<wchar_t, std::allocator<wchar_t> >().test_reserve();
    }


    void test_normal_wchar_t_std_allocator_test_swap(){
        return bsl_test_string<wchar_t, std::allocator<wchar_t> >().test_swap();
    }


    void test_normal_wchar_t_std_allocator_test_operator_assign_to_null(){
        return bsl_test_string<wchar_t, std::allocator<wchar_t> >().test_operator_assign_to_null();
    }


    void test_normal_wchar_t_std_allocator_test_operator_assign_to_cstring(){
        return bsl_test_string<wchar_t, std::allocator<wchar_t> >().test_operator_assign_to_cstring();
    }


    void test_normal_wchar_t_std_allocator_test_operator_assign_to_string(){
        return bsl_test_string<wchar_t, std::allocator<wchar_t> >().test_operator_assign_to_string();
    }


    void test_normal_wchar_t_std_allocator_test_operator_eq_eq(){
        return bsl_test_string<wchar_t, std::allocator<wchar_t> >().test_operator_eq_eq();
    }


    void test_normal_wchar_t_std_allocator_test_operator_not_eq(){
        return bsl_test_string<wchar_t, std::allocator<wchar_t> >().test_operator_not_eq();
    }


    void test_normal_wchar_t_std_allocator_test_operator_lt(){
        return bsl_test_string<wchar_t, std::allocator<wchar_t> >().test_operator_lt();
    }


    void test_normal_wchar_t_std_allocator_test_operator_gt(){
        return bsl_test_string<wchar_t, std::allocator<wchar_t> >().test_operator_gt();
    }


    void test_normal_wchar_t_std_allocator_test_operator_lt_eq(){
        return bsl_test_string<wchar_t, std::allocator<wchar_t> >().test_operator_lt_eq();
    }


    void test_normal_wchar_t_std_allocator_test_operator_gt_eq(){
        return bsl_test_string<wchar_t, std::allocator<wchar_t> >().test_operator_gt_eq();
    }


    void test_normal_wchar_t_std_allocator_test_operator_shift(){
        return bsl_test_string<wchar_t, std::allocator<wchar_t> >().test_operator_shift();
    }


    void test_normal_wchar_t_std_allocator_test_serialization(){
        return bsl_test_string<wchar_t, std::allocator<wchar_t> >().test_serialization();
    }


    void test_normal_wchar_t_std_allocator_test_append_str(){
        return bsl_test_string<wchar_t, std::allocator<wchar_t> >().test_append_str();
    }


    void test_normal_wchar_t_std_allocator_test_append_sub_str(){
        return bsl_test_string<wchar_t, std::allocator<wchar_t> >().test_append_sub_str();
    }


    void test_normal_wchar_t_std_allocator_test_append_cstr(){
        return bsl_test_string<wchar_t, std::allocator<wchar_t> >().test_append_cstr();
    }


    void test_normal_wchar_t_std_allocator_test_append_sub_cstr(){
        return bsl_test_string<wchar_t, std::allocator<wchar_t> >().test_append_sub_cstr();
    }


    void test_normal_wchar_t_std_allocator_test_append_n_char(){
        return bsl_test_string<wchar_t, std::allocator<wchar_t> >().test_append_n_char();
    }


    void test_normal_wchar_t_std_allocator_test_append_range(){
        return bsl_test_string<wchar_t, std::allocator<wchar_t> >().test_append_range();
    }


    void test_normal_wchar_t_std_allocator_test_push_back(){
        return bsl_test_string<wchar_t, std::allocator<wchar_t> >().test_push_back();
    }


    void test_normal_wchar_t_std_allocator_test_appendf(){
        return bsl_test_string<wchar_t, std::allocator<wchar_t> >().test_appendf();
    }


    void test_normal_wchar_t_std_allocator_test_find(){
        return bsl_test_string<wchar_t, std::allocator<wchar_t> >().test_find();
    }


    void test_normal_wchar_t_std_allocator_test_rfind(){
        return bsl_test_string<wchar_t, std::allocator<wchar_t> >().test_rfind();
    }


    void test_normal_unsigned_short_bsl_bsl_alloc_test_ctors(){
        return bsl_test_string<unsigned short, bsl::bsl_alloc<unsigned short> >().test_ctors();
    }


    void test_normal_unsigned_short_bsl_bsl_alloc_test_c_str(){
        return bsl_test_string<unsigned short, bsl::bsl_alloc<unsigned short> >().test_c_str();
    }


    void test_normal_unsigned_short_bsl_bsl_alloc_test_empty(){
        return bsl_test_string<unsigned short, bsl::bsl_alloc<unsigned short> >().test_empty();
    }


    void test_normal_unsigned_short_bsl_bsl_alloc_test_size(){
        return bsl_test_string<unsigned short, bsl::bsl_alloc<unsigned short> >().test_size();
    }


    void test_normal_unsigned_short_bsl_bsl_alloc_test_length(){
        return bsl_test_string<unsigned short, bsl::bsl_alloc<unsigned short> >().test_length();
    }


    void test_normal_unsigned_short_bsl_bsl_alloc_test_capacity(){
        return bsl_test_string<unsigned short, bsl::bsl_alloc<unsigned short> >().test_capacity();
    }


    void test_normal_unsigned_short_bsl_bsl_alloc_test_operator_square(){
        return bsl_test_string<unsigned short, bsl::bsl_alloc<unsigned short> >().test_operator_square();
    }


    void test_normal_unsigned_short_bsl_bsl_alloc_test_clear(){
        return bsl_test_string<unsigned short, bsl::bsl_alloc<unsigned short> >().test_clear();
    }


    void test_normal_unsigned_short_bsl_bsl_alloc_test_reserve(){
        return bsl_test_string<unsigned short, bsl::bsl_alloc<unsigned short> >().test_reserve();
    }


    void test_normal_unsigned_short_bsl_bsl_alloc_test_swap(){
        return bsl_test_string<unsigned short, bsl::bsl_alloc<unsigned short> >().test_swap();
    }


    void test_normal_unsigned_short_bsl_bsl_alloc_test_operator_assign_to_null(){
        return bsl_test_string<unsigned short, bsl::bsl_alloc<unsigned short> >().test_operator_assign_to_null();
    }


    void test_normal_unsigned_short_bsl_bsl_alloc_test_operator_assign_to_cstring(){
        return bsl_test_string<unsigned short, bsl::bsl_alloc<unsigned short> >().test_operator_assign_to_cstring();
    }


    void test_normal_unsigned_short_bsl_bsl_alloc_test_operator_assign_to_string(){
        return bsl_test_string<unsigned short, bsl::bsl_alloc<unsigned short> >().test_operator_assign_to_string();
    }


    void test_normal_unsigned_short_bsl_bsl_alloc_test_operator_eq_eq(){
        return bsl_test_string<unsigned short, bsl::bsl_alloc<unsigned short> >().test_operator_eq_eq();
    }


    void test_normal_unsigned_short_bsl_bsl_alloc_test_operator_not_eq(){
        return bsl_test_string<unsigned short, bsl::bsl_alloc<unsigned short> >().test_operator_not_eq();
    }


    void test_normal_unsigned_short_bsl_bsl_alloc_test_operator_lt(){
        return bsl_test_string<unsigned short, bsl::bsl_alloc<unsigned short> >().test_operator_lt();
    }


    void test_normal_unsigned_short_bsl_bsl_alloc_test_operator_gt(){
        return bsl_test_string<unsigned short, bsl::bsl_alloc<unsigned short> >().test_operator_gt();
    }


    void test_normal_unsigned_short_bsl_bsl_alloc_test_operator_lt_eq(){
        return bsl_test_string<unsigned short, bsl::bsl_alloc<unsigned short> >().test_operator_lt_eq();
    }


    void test_normal_unsigned_short_bsl_bsl_alloc_test_operator_gt_eq(){
        return bsl_test_string<unsigned short, bsl::bsl_alloc<unsigned short> >().test_operator_gt_eq();
    }


    void test_normal_unsigned_short_bsl_bsl_alloc_test_operator_shift(){
        return bsl_test_string<unsigned short, bsl::bsl_alloc<unsigned short> >().test_operator_shift();
    }


    void test_normal_unsigned_short_bsl_bsl_alloc_test_serialization(){
        return bsl_test_string<unsigned short, bsl::bsl_alloc<unsigned short> >().test_serialization();
    }


    void test_normal_unsigned_short_bsl_bsl_alloc_test_append_str(){
        return bsl_test_string<unsigned short, bsl::bsl_alloc<unsigned short> >().test_append_str();
    }


    void test_normal_unsigned_short_bsl_bsl_alloc_test_append_sub_str(){
        return bsl_test_string<unsigned short, bsl::bsl_alloc<unsigned short> >().test_append_sub_str();
    }


    void test_normal_unsigned_short_bsl_bsl_alloc_test_append_cstr(){
        return bsl_test_string<unsigned short, bsl::bsl_alloc<unsigned short> >().test_append_cstr();
    }


    void test_normal_unsigned_short_bsl_bsl_alloc_test_append_sub_cstr(){
        return bsl_test_string<unsigned short, bsl::bsl_alloc<unsigned short> >().test_append_sub_cstr();
    }


    void test_normal_unsigned_short_bsl_bsl_alloc_test_append_n_char(){
        return bsl_test_string<unsigned short, bsl::bsl_alloc<unsigned short> >().test_append_n_char();
    }


    void test_normal_unsigned_short_bsl_bsl_alloc_test_append_range(){
        return bsl_test_string<unsigned short, bsl::bsl_alloc<unsigned short> >().test_append_range();
    }


    void test_normal_unsigned_short_bsl_bsl_alloc_test_push_back(){
        return bsl_test_string<unsigned short, bsl::bsl_alloc<unsigned short> >().test_push_back();
    }


    void test_normal_unsigned_short_bsl_bsl_alloc_test_appendf(){
        return bsl_test_string<unsigned short, bsl::bsl_alloc<unsigned short> >().test_appendf();
    }


    void test_normal_unsigned_short_bsl_bsl_alloc_test_find(){
        return bsl_test_string<unsigned short, bsl::bsl_alloc<unsigned short> >().test_find();
    }


    void test_normal_unsigned_short_bsl_bsl_alloc_test_rfind(){
        return bsl_test_string<unsigned short, bsl::bsl_alloc<unsigned short> >().test_rfind();
    }


    void test_normal_unsigned_short_std_allocator_test_ctors(){
        return bsl_test_string<unsigned short, std::allocator<unsigned short> >().test_ctors();
    }


    void test_normal_unsigned_short_std_allocator_test_c_str(){
        return bsl_test_string<unsigned short, std::allocator<unsigned short> >().test_c_str();
    }


    void test_normal_unsigned_short_std_allocator_test_empty(){
        return bsl_test_string<unsigned short, std::allocator<unsigned short> >().test_empty();
    }


    void test_normal_unsigned_short_std_allocator_test_size(){
        return bsl_test_string<unsigned short, std::allocator<unsigned short> >().test_size();
    }


    void test_normal_unsigned_short_std_allocator_test_length(){
        return bsl_test_string<unsigned short, std::allocator<unsigned short> >().test_length();
    }


    void test_normal_unsigned_short_std_allocator_test_capacity(){
        return bsl_test_string<unsigned short, std::allocator<unsigned short> >().test_capacity();
    }


    void test_normal_unsigned_short_std_allocator_test_operator_square(){
        return bsl_test_string<unsigned short, std::allocator<unsigned short> >().test_operator_square();
    }


    void test_normal_unsigned_short_std_allocator_test_clear(){
        return bsl_test_string<unsigned short, std::allocator<unsigned short> >().test_clear();
    }


    void test_normal_unsigned_short_std_allocator_test_reserve(){
        return bsl_test_string<unsigned short, std::allocator<unsigned short> >().test_reserve();
    }


    void test_normal_unsigned_short_std_allocator_test_swap(){
        return bsl_test_string<unsigned short, std::allocator<unsigned short> >().test_swap();
    }


    void test_normal_unsigned_short_std_allocator_test_operator_assign_to_null(){
        return bsl_test_string<unsigned short, std::allocator<unsigned short> >().test_operator_assign_to_null();
    }


    void test_normal_unsigned_short_std_allocator_test_operator_assign_to_cstring(){
        return bsl_test_string<unsigned short, std::allocator<unsigned short> >().test_operator_assign_to_cstring();
    }


    void test_normal_unsigned_short_std_allocator_test_operator_assign_to_string(){
        return bsl_test_string<unsigned short, std::allocator<unsigned short> >().test_operator_assign_to_string();
    }


    void test_normal_unsigned_short_std_allocator_test_operator_eq_eq(){
        return bsl_test_string<unsigned short, std::allocator<unsigned short> >().test_operator_eq_eq();
    }


    void test_normal_unsigned_short_std_allocator_test_operator_not_eq(){
        return bsl_test_string<unsigned short, std::allocator<unsigned short> >().test_operator_not_eq();
    }


    void test_normal_unsigned_short_std_allocator_test_operator_lt(){
        return bsl_test_string<unsigned short, std::allocator<unsigned short> >().test_operator_lt();
    }


    void test_normal_unsigned_short_std_allocator_test_operator_gt(){
        return bsl_test_string<unsigned short, std::allocator<unsigned short> >().test_operator_gt();
    }


    void test_normal_unsigned_short_std_allocator_test_operator_lt_eq(){
        return bsl_test_string<unsigned short, std::allocator<unsigned short> >().test_operator_lt_eq();
    }


    void test_normal_unsigned_short_std_allocator_test_operator_gt_eq(){
        return bsl_test_string<unsigned short, std::allocator<unsigned short> >().test_operator_gt_eq();
    }


    void test_normal_unsigned_short_std_allocator_test_operator_shift(){
        return bsl_test_string<unsigned short, std::allocator<unsigned short> >().test_operator_shift();
    }


    void test_normal_unsigned_short_std_allocator_test_serialization(){
        return bsl_test_string<unsigned short, std::allocator<unsigned short> >().test_serialization();
    }


    void test_normal_unsigned_short_std_allocator_test_append_str(){
        return bsl_test_string<unsigned short, std::allocator<unsigned short> >().test_append_str();
    }


    void test_normal_unsigned_short_std_allocator_test_append_sub_str(){
        return bsl_test_string<unsigned short, std::allocator<unsigned short> >().test_append_sub_str();
    }


    void test_normal_unsigned_short_std_allocator_test_append_cstr(){
        return bsl_test_string<unsigned short, std::allocator<unsigned short> >().test_append_cstr();
    }


    void test_normal_unsigned_short_std_allocator_test_append_sub_cstr(){
        return bsl_test_string<unsigned short, std::allocator<unsigned short> >().test_append_sub_cstr();
    }


    void test_normal_unsigned_short_std_allocator_test_append_n_char(){
        return bsl_test_string<unsigned short, std::allocator<unsigned short> >().test_append_n_char();
    }


    void test_normal_unsigned_short_std_allocator_test_append_range(){
        return bsl_test_string<unsigned short, std::allocator<unsigned short> >().test_append_range();
    }


    void test_normal_unsigned_short_std_allocator_test_push_back(){
        return bsl_test_string<unsigned short, std::allocator<unsigned short> >().test_push_back();
    }


    void test_normal_unsigned_short_std_allocator_test_appendf(){
        return bsl_test_string<unsigned short, std::allocator<unsigned short> >().test_appendf();
    }


    void test_normal_unsigned_short_std_allocator_test_find(){
        return bsl_test_string<unsigned short, std::allocator<unsigned short> >().test_find();
    }


    void test_normal_unsigned_short_std_allocator_test_rfind(){
        return bsl_test_string<unsigned short, std::allocator<unsigned short> >().test_rfind();
    }


    void test_normal_unsigned_int_bsl_bsl_alloc_test_ctors(){
        return bsl_test_string<unsigned int, bsl::bsl_alloc<unsigned int> >().test_ctors();
    }


    void test_normal_unsigned_int_bsl_bsl_alloc_test_c_str(){
        return bsl_test_string<unsigned int, bsl::bsl_alloc<unsigned int> >().test_c_str();
    }


    void test_normal_unsigned_int_bsl_bsl_alloc_test_empty(){
        return bsl_test_string<unsigned int, bsl::bsl_alloc<unsigned int> >().test_empty();
    }


    void test_normal_unsigned_int_bsl_bsl_alloc_test_size(){
        return bsl_test_string<unsigned int, bsl::bsl_alloc<unsigned int> >().test_size();
    }


    void test_normal_unsigned_int_bsl_bsl_alloc_test_length(){
        return bsl_test_string<unsigned int, bsl::bsl_alloc<unsigned int> >().test_length();
    }


    void test_normal_unsigned_int_bsl_bsl_alloc_test_capacity(){
        return bsl_test_string<unsigned int, bsl::bsl_alloc<unsigned int> >().test_capacity();
    }


    void test_normal_unsigned_int_bsl_bsl_alloc_test_operator_square(){
        return bsl_test_string<unsigned int, bsl::bsl_alloc<unsigned int> >().test_operator_square();
    }


    void test_normal_unsigned_int_bsl_bsl_alloc_test_clear(){
        return bsl_test_string<unsigned int, bsl::bsl_alloc<unsigned int> >().test_clear();
    }


    void test_normal_unsigned_int_bsl_bsl_alloc_test_reserve(){
        return bsl_test_string<unsigned int, bsl::bsl_alloc<unsigned int> >().test_reserve();
    }


    void test_normal_unsigned_int_bsl_bsl_alloc_test_swap(){
        return bsl_test_string<unsigned int, bsl::bsl_alloc<unsigned int> >().test_swap();
    }


    void test_normal_unsigned_int_bsl_bsl_alloc_test_operator_assign_to_null(){
        return bsl_test_string<unsigned int, bsl::bsl_alloc<unsigned int> >().test_operator_assign_to_null();
    }


    void test_normal_unsigned_int_bsl_bsl_alloc_test_operator_assign_to_cstring(){
        return bsl_test_string<unsigned int, bsl::bsl_alloc<unsigned int> >().test_operator_assign_to_cstring();
    }


    void test_normal_unsigned_int_bsl_bsl_alloc_test_operator_assign_to_string(){
        return bsl_test_string<unsigned int, bsl::bsl_alloc<unsigned int> >().test_operator_assign_to_string();
    }


    void test_normal_unsigned_int_bsl_bsl_alloc_test_operator_eq_eq(){
        return bsl_test_string<unsigned int, bsl::bsl_alloc<unsigned int> >().test_operator_eq_eq();
    }


    void test_normal_unsigned_int_bsl_bsl_alloc_test_operator_not_eq(){
        return bsl_test_string<unsigned int, bsl::bsl_alloc<unsigned int> >().test_operator_not_eq();
    }


    void test_normal_unsigned_int_bsl_bsl_alloc_test_operator_lt(){
        return bsl_test_string<unsigned int, bsl::bsl_alloc<unsigned int> >().test_operator_lt();
    }


    void test_normal_unsigned_int_bsl_bsl_alloc_test_operator_gt(){
        return bsl_test_string<unsigned int, bsl::bsl_alloc<unsigned int> >().test_operator_gt();
    }


    void test_normal_unsigned_int_bsl_bsl_alloc_test_operator_lt_eq(){
        return bsl_test_string<unsigned int, bsl::bsl_alloc<unsigned int> >().test_operator_lt_eq();
    }


    void test_normal_unsigned_int_bsl_bsl_alloc_test_operator_gt_eq(){
        return bsl_test_string<unsigned int, bsl::bsl_alloc<unsigned int> >().test_operator_gt_eq();
    }


    void test_normal_unsigned_int_bsl_bsl_alloc_test_operator_shift(){
        return bsl_test_string<unsigned int, bsl::bsl_alloc<unsigned int> >().test_operator_shift();
    }


    void test_normal_unsigned_int_bsl_bsl_alloc_test_serialization(){
        return bsl_test_string<unsigned int, bsl::bsl_alloc<unsigned int> >().test_serialization();
    }


    void test_normal_unsigned_int_bsl_bsl_alloc_test_append_str(){
        return bsl_test_string<unsigned int, bsl::bsl_alloc<unsigned int> >().test_append_str();
    }


    void test_normal_unsigned_int_bsl_bsl_alloc_test_append_sub_str(){
        return bsl_test_string<unsigned int, bsl::bsl_alloc<unsigned int> >().test_append_sub_str();
    }


    void test_normal_unsigned_int_bsl_bsl_alloc_test_append_cstr(){
        return bsl_test_string<unsigned int, bsl::bsl_alloc<unsigned int> >().test_append_cstr();
    }


    void test_normal_unsigned_int_bsl_bsl_alloc_test_append_sub_cstr(){
        return bsl_test_string<unsigned int, bsl::bsl_alloc<unsigned int> >().test_append_sub_cstr();
    }


    void test_normal_unsigned_int_bsl_bsl_alloc_test_append_n_char(){
        return bsl_test_string<unsigned int, bsl::bsl_alloc<unsigned int> >().test_append_n_char();
    }


    void test_normal_unsigned_int_bsl_bsl_alloc_test_append_range(){
        return bsl_test_string<unsigned int, bsl::bsl_alloc<unsigned int> >().test_append_range();
    }


    void test_normal_unsigned_int_bsl_bsl_alloc_test_push_back(){
        return bsl_test_string<unsigned int, bsl::bsl_alloc<unsigned int> >().test_push_back();
    }


    void test_normal_unsigned_int_bsl_bsl_alloc_test_appendf(){
        return bsl_test_string<unsigned int, bsl::bsl_alloc<unsigned int> >().test_appendf();
    }


    void test_normal_unsigned_int_bsl_bsl_alloc_test_find(){
        return bsl_test_string<unsigned int, bsl::bsl_alloc<unsigned int> >().test_find();
    }


    void test_normal_unsigned_int_bsl_bsl_alloc_test_rfind(){
        return bsl_test_string<unsigned int, bsl::bsl_alloc<unsigned int> >().test_rfind();
    }


    void test_normal_unsigned_int_std_allocator_test_ctors(){
        return bsl_test_string<unsigned int, std::allocator<unsigned int> >().test_ctors();
    }


    void test_normal_unsigned_int_std_allocator_test_c_str(){
        return bsl_test_string<unsigned int, std::allocator<unsigned int> >().test_c_str();
    }


    void test_normal_unsigned_int_std_allocator_test_empty(){
        return bsl_test_string<unsigned int, std::allocator<unsigned int> >().test_empty();
    }


    void test_normal_unsigned_int_std_allocator_test_size(){
        return bsl_test_string<unsigned int, std::allocator<unsigned int> >().test_size();
    }


    void test_normal_unsigned_int_std_allocator_test_length(){
        return bsl_test_string<unsigned int, std::allocator<unsigned int> >().test_length();
    }


    void test_normal_unsigned_int_std_allocator_test_capacity(){
        return bsl_test_string<unsigned int, std::allocator<unsigned int> >().test_capacity();
    }


    void test_normal_unsigned_int_std_allocator_test_operator_square(){
        return bsl_test_string<unsigned int, std::allocator<unsigned int> >().test_operator_square();
    }


    void test_normal_unsigned_int_std_allocator_test_clear(){
        return bsl_test_string<unsigned int, std::allocator<unsigned int> >().test_clear();
    }


    void test_normal_unsigned_int_std_allocator_test_reserve(){
        return bsl_test_string<unsigned int, std::allocator<unsigned int> >().test_reserve();
    }


    void test_normal_unsigned_int_std_allocator_test_swap(){
        return bsl_test_string<unsigned int, std::allocator<unsigned int> >().test_swap();
    }


    void test_normal_unsigned_int_std_allocator_test_operator_assign_to_null(){
        return bsl_test_string<unsigned int, std::allocator<unsigned int> >().test_operator_assign_to_null();
    }


    void test_normal_unsigned_int_std_allocator_test_operator_assign_to_cstring(){
        return bsl_test_string<unsigned int, std::allocator<unsigned int> >().test_operator_assign_to_cstring();
    }


    void test_normal_unsigned_int_std_allocator_test_operator_assign_to_string(){
        return bsl_test_string<unsigned int, std::allocator<unsigned int> >().test_operator_assign_to_string();
    }


    void test_normal_unsigned_int_std_allocator_test_operator_eq_eq(){
        return bsl_test_string<unsigned int, std::allocator<unsigned int> >().test_operator_eq_eq();
    }


    void test_normal_unsigned_int_std_allocator_test_operator_not_eq(){
        return bsl_test_string<unsigned int, std::allocator<unsigned int> >().test_operator_not_eq();
    }


    void test_normal_unsigned_int_std_allocator_test_operator_lt(){
        return bsl_test_string<unsigned int, std::allocator<unsigned int> >().test_operator_lt();
    }


    void test_normal_unsigned_int_std_allocator_test_operator_gt(){
        return bsl_test_string<unsigned int, std::allocator<unsigned int> >().test_operator_gt();
    }


    void test_normal_unsigned_int_std_allocator_test_operator_lt_eq(){
        return bsl_test_string<unsigned int, std::allocator<unsigned int> >().test_operator_lt_eq();
    }


    void test_normal_unsigned_int_std_allocator_test_operator_gt_eq(){
        return bsl_test_string<unsigned int, std::allocator<unsigned int> >().test_operator_gt_eq();
    }


    void test_normal_unsigned_int_std_allocator_test_operator_shift(){
        return bsl_test_string<unsigned int, std::allocator<unsigned int> >().test_operator_shift();
    }


    void test_normal_unsigned_int_std_allocator_test_serialization(){
        return bsl_test_string<unsigned int, std::allocator<unsigned int> >().test_serialization();
    }


    void test_normal_unsigned_int_std_allocator_test_append_str(){
        return bsl_test_string<unsigned int, std::allocator<unsigned int> >().test_append_str();
    }


    void test_normal_unsigned_int_std_allocator_test_append_sub_str(){
        return bsl_test_string<unsigned int, std::allocator<unsigned int> >().test_append_sub_str();
    }


    void test_normal_unsigned_int_std_allocator_test_append_cstr(){
        return bsl_test_string<unsigned int, std::allocator<unsigned int> >().test_append_cstr();
    }


    void test_normal_unsigned_int_std_allocator_test_append_sub_cstr(){
        return bsl_test_string<unsigned int, std::allocator<unsigned int> >().test_append_sub_cstr();
    }


    void test_normal_unsigned_int_std_allocator_test_append_n_char(){
        return bsl_test_string<unsigned int, std::allocator<unsigned int> >().test_append_n_char();
    }


    void test_normal_unsigned_int_std_allocator_test_append_range(){
        return bsl_test_string<unsigned int, std::allocator<unsigned int> >().test_append_range();
    }


    void test_normal_unsigned_int_std_allocator_test_push_back(){
        return bsl_test_string<unsigned int, std::allocator<unsigned int> >().test_push_back();
    }


    void test_normal_unsigned_int_std_allocator_test_appendf(){
        return bsl_test_string<unsigned int, std::allocator<unsigned int> >().test_appendf();
    }


    void test_normal_unsigned_int_std_allocator_test_find(){
        return bsl_test_string<unsigned int, std::allocator<unsigned int> >().test_find();
    }


    void test_normal_unsigned_int_std_allocator_test_rfind(){
        return bsl_test_string<unsigned int, std::allocator<unsigned int> >().test_rfind();
    }


};
#endif  //__BSL_TEST_STRING_H_
