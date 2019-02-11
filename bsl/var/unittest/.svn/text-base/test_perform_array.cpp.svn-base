/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: test_perform_array.cpp,v 1.2 2009/03/09 04:56:42 xiaowei Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file test_perform_array.cpp
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2008/10/22 17:27:00
 * @version $Revision: 1.2 $ 
 * @brief 
 *  
 **/

#include <cstdlib>
#include <vector>
#include <deque>
#include "bsl/var/implement.h"

void test_array_1(){
    int group_size = 1000000;
    int array_len = 10;

    bsl::var::Array          va;
    std::vector<void *>     ve;
    std::deque<void *>      de;
    void *                  ar[array_len];

    int va_time = 0;
    int va_get_time = 0;
    int ve_time = 0;
    int de_time = 0;
    int ar_time = 0;

    ve.assign( array_len, &ve );
    de.assign( array_len, &de );
    for( int i = 0; i < array_len; ++ i ){
        va[array_len] = va;
        ar[array_len] = ar;
    }

    int pos = rand() % array_len;
    struct timeval b, e;
    void * res;

    gettimeofday(&b, NULL);
    for( int j = 0; j < group_size; ++ j ){
        res = &va[pos];        
    }
    gettimeofday(&e, NULL);
    va_time += (e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec );

    gettimeofday(&b, NULL);
    for( int j = 0; j < group_size; ++ j ){
        res = &va.get(pos);        
    }
    gettimeofday(&e, NULL);
    va_get_time += (e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec );

    gettimeofday(&b, NULL);
    for( int j = 0; j < group_size; ++ j ){
        res = ve[pos];        
    }
    gettimeofday(&e, NULL);
    ve_time += (e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec );

    gettimeofday(&b, NULL);
    for( int j = 0; j < group_size; ++ j ){
        res = de[pos];        
    }
    gettimeofday(&e, NULL);
    de_time += (e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec );

    gettimeofday(&b, NULL);
    for( int j = 0; j < group_size; ++ j ){
        res = ar[pos];        
    }
    gettimeofday(&e, NULL);
    ar_time += (e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec );


    printf("group_size:%d array_len:%d\n", group_size, array_len );
    printf("%20s: %dus, %gtimes/s\n", "bsl::var::Array", va_time,  1e6 * group_size / va_time );
    printf("%20s: %dus, %gtimes/s\n", "bsl::var::Array.get()", va_get_time,  1e6 * group_size / va_get_time );
    printf("%20s: %dus, %gtimes/s\n", "std::vector",    ve_time,  1e6 * group_size / ve_time );
    printf("%20s: %dus, %gtimes/s\n", "std::deque",     de_time,  1e6 * group_size / de_time );
    printf("%20s: %dus, %gtimes/s\n", "void *[]",       ar_time,  1e6 * group_size / ar_time );

}

void test_array_2(){
    int group_size = 1000;
    int array_len = 10000;

    bsl::var::Array          va;
    bsl::var::Array          va2;
    std::vector<void *>     ve;
    std::vector<void *>     ve2;
    std::deque<void *>      de;
    std::deque<void *>      de2;
    void *                  ar[array_len];

    int va_time = 0;
    int va_get_time = 0;
    int va2_time = 0;
    int ve_time = 0;
    int ve2_time = 0;
    int de_time = 0;
    int de2_time = 0;
    int ar_time = 0;

    ve.reserve( array_len );
    de.resize( array_len );
    va[array_len -1];   //自动增长

    struct timeval b, e;

    for( int i = 0; i < group_size; ++ i ){
        gettimeofday(&b, NULL);
        for( int j = 1; j < array_len; ++ j ){
            va[j] = va[j-1];
        }
        gettimeofday(&e, NULL);
        va_time += (e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec );

        gettimeofday(&b, NULL);
        for( int j = 1; j < array_len; ++ j ){
            va.set(j, va.get(j-1)); 
        }
        gettimeofday(&e, NULL);
        va_get_time += (e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec );

        gettimeofday(&b, NULL);
        for( int j = 1; j < array_len; ++ j ){
            va2[j] = va2[j-1];
        }
        gettimeofday(&e, NULL);
        va2_time += (e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec );


        gettimeofday(&b, NULL);
        for( int j = 1; j < array_len; ++ j ){
            ve[j] = ve[j-1];
        }
        gettimeofday(&e, NULL);
        ve_time += (e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec );

        gettimeofday(&b, NULL);
        for( int j = 1; j < array_len; ++ j ){
            ve2.reserve(j+1);
            ve2[j] = ve2[j-1];
        }
        gettimeofday(&e, NULL);
        ve2_time += (e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec );

        gettimeofday(&b, NULL);
        for( int j = 1; j < array_len; ++ j ){
            de[j] = de[j-1];
        }
        gettimeofday(&e, NULL);
        de_time += (e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec );

        gettimeofday(&b, NULL);
        for( int j = 1; j < array_len; ++ j ){
            if ( size_t(j) > de2.size() ){
                de2.resize(j+1);
            }
            de2[j] = de2[j-1];
        }
        gettimeofday(&e, NULL);
        de2_time += (e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec );


        gettimeofday(&b, NULL);
        for( int j = 1; j < array_len; ++ j ){
            ar[j] = ar[j-1];
        }
        gettimeofday(&e, NULL);
        ar_time += (e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec );

    }

    printf("group_size:%d array_len:%d\n", group_size, array_len );
    printf("%20s: %dus, %gtimes/s\n", "bsl::var::Array", va_time,  1e6 * group_size / va_time );
    printf("%20s: %dus, %gtimes/s\n", "bsl::var::Array 2", va2_time,  1e6 * group_size / va2_time );
    printf("%20s: %dus, %gtimes/s\n", "bsl::var::Array.get()", va_get_time,  1e6 * group_size / va_get_time );
    printf("%20s: %dus, %gtimes/s\n", "std::vector",    ve_time,  1e6 * group_size / ve_time );
    printf("%20s: %dus, %gtimes/s\n", "std::vector 2",    ve2_time,  1e6 * group_size / ve2_time );
    printf("%20s: %dus, %gtimes/s\n", "std::deque",     de_time,  1e6 * group_size / de_time );
    printf("%20s: %dus, %gtimes/s\n", "std::deque 2",     de2_time,  1e6 * group_size / de2_time );
    printf("%20s: %dus, %gtimes/s\n", "void *[]",       ar_time,  1e6 * group_size / ar_time );

}

int main(){
    return 0;
    //test_array_1();
    test_array_2();
    return 0;
}
/* vim: set ts=4 sw=4 sts=4 tw=100 */
