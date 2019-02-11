/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: test_perform_dict.cpp,v 1.2 2009/03/09 04:56:42 xiaowei Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file test_perform_dict.cpp
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2008/10/22 17:27:00
 * @version $Revision: 1.2 $ 
 * @brief 
 *  
 **/

#include <cstdlib>
#include <vector>
#include <deque>
#include<ext/hash_map>
#include <bsl/containers/hash/bsl_hashmap.h>
#include "bsl/var/implement.h"
const int MAX_STR_LEN = 1000;
const int MAX_DICT_SIZE = 100000;
char        cs[MAX_DICT_SIZE+1][MAX_STR_LEN];
std::string ss[MAX_DICT_SIZE+1];
bsl::string bs[MAX_DICT_SIZE+1];

struct bsl_str_hash_func{
    size_t operator ()( const bsl::string& str ) const {
        return __gnu_cxx::__stl_hash_string( str.c_str() );
    }
};

struct std_str_hash_func{
    size_t operator ()( const std::string& str ) const {
        return __gnu_cxx::__stl_hash_string( str.c_str() );
    }
};

char * gen_random_str(char * str, int len ){
    const char BEGIN = 'a';
    const char END   = 'z';
    for( int i = 0; i < len; ++ i ){
        str[i] = BEGIN + rand() % (END + 1 - BEGIN );
    }
    str[len] = 0;
    return str;
}

#include<typeinfo>
#include<iostream>
#define see(x) do{  \
    std::cerr<<__FILE__<<":"<<__LINE__<<": " <<(#x)<<" = "<<(x)<<" ("<<typeid(x).name()<<")"<<std::endl;\
}while(0)

void test_dict_random( int groups, int group_size, int dict_size, int str_len ){

    bsl::var::Dict                   vd;     //var_dict;
    std::map<std::string, void *>   sssm;   //std_str_std_map;
    std::map<bsl::string, void *>   bssm;   //bsl_str_std_map;
    bsl::hashmap<std::string, void *, std_str_hash_func> ssbm;
    bsl::hashmap<bsl::string, void *, bsl_str_hash_func> bsbm;
    __gnu_cxx::hash_map<std::string, void *, std_str_hash_func> ssgm;  //std_str_gnu_map;
    __gnu_cxx::hash_map<bsl::string, void *, bsl_str_hash_func> bsgm;  //bsl_str_gnu_map;

    ssbm.create(dict_size); //比较豪华的size
    bsbm.create(dict_size); //比较豪华的size

    int vd_time = 0;     
    int vd_get_time = 0;
    int sssm_time = 0;   
    int bssm_time = 0;   
    int ssbm_time = 0;
    int bsbm_time = 0;
    int ssgm_time = 0;  
    int bsgm_time = 0;  

    int csbs_time = 0;// time used in char * => bsl::string
    int csss_time = 0;// time used in char * => std::string
    int loop_time = 0;

    //灌数据
    for( int i = 0; i < dict_size; ++ i ){
        gen_random_str( cs[i], str_len );
        ss[i] = cs[i];  //make std str array
        bs[i] = cs[i];  //make bsl str array
        vd[bs[i]] = vd;
        sssm[ss[i]] = cs[i];    //make dictionarys    
        ssbm.set(ss[i], cs[i]);
        ssgm[ss[i]] = cs[i];
        
        bssm[bs[i]] = cs[i];
        bsbm.set(bs[i], cs[i]);
        bsgm[bs[i]] = cs[i];

    }
    see( vd.size() );
    see( sssm.size() );
    see( ssbm.size() );
    see( ssgm.size() );
    see( bssm.size() );
    see( bsbm.size() );
    see( bsgm.size() );


    for( int i = 0; i < groups; ++ i ){
        int pos = rand() % dict_size;
        struct timeval b, e;
        void * res;

        gettimeofday(&b, NULL);
        for( int j = 0; j < group_size; ++ j ){
            res = &vd[bs[pos]];
        }
        gettimeofday(&e, NULL);
        vd_time += (e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec );

        gettimeofday(&b, NULL);
        for( int j = 0; j < group_size; ++ j ){
            res = &vd.get(bs[pos]);
        }
        gettimeofday(&e, NULL);
        vd_get_time += (e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec );

        gettimeofday(&b, NULL);
        for( int j = 0; j < group_size; ++ j ){
            res = sssm[ss[pos]];
        }
        gettimeofday(&e, NULL);
        sssm_time += (e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec );

        gettimeofday(&b, NULL);
        for( int j = 0; j < group_size; ++ j ){
            ssbm.get(ss[pos],&res);
        }
        gettimeofday(&e, NULL);
        ssbm_time += (e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec );

        gettimeofday(&b, NULL);
        for( int j = 0; j < group_size; ++ j ){
            res = ssgm[ss[pos]];
        }
        gettimeofday(&e, NULL);
        ssgm_time += (e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec );

        gettimeofday(&b, NULL);
        for( int j = 0; j < group_size; ++ j ){
            res = bssm[bs[pos]];
        }
        gettimeofday(&e, NULL);
        bssm_time += (e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec );

        gettimeofday(&b, NULL);
        for( int j = 0; j < group_size; ++ j ){
            bsbm.get(bs[i], &res);
        }
        gettimeofday(&e, NULL);
        bsbm_time += (e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec );

        gettimeofday(&b, NULL);
        for( int j = 0; j < group_size; ++ j ){
            res = bsgm[bs[i]];
        }
        gettimeofday(&e, NULL);
        bsgm_time+= (e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec );

        gettimeofday(&b, NULL);
        for( int j = 0; j < group_size; ++ j ){
            bsl::basic_string<char, bsl::bsl_alloc<char> > __bs(cs[pos]);
        }
        gettimeofday(&e, NULL);
        csbs_time+= (e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec );

        gettimeofday(&b, NULL);
        for( int j = 0; j < group_size; ++ j ){
            std::string __ss(cs[pos]);
        }
        gettimeofday(&e, NULL);
        csss_time+= (e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec );

        gettimeofday(&b, NULL);
        for( int j = 0; j < group_size; ++ j ){
            ;
        }
        gettimeofday(&e, NULL);
        loop_time+= (e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec );

    }

    printf("groups:%d group_size:%d dict_size:%d str_len:%d\n", groups, group_size, dict_size, str_len );
    const char * fmt ="map: %-20s str: %-20s method: %-20s total time: %dus \n" ;
    printf(fmt, "bsl::var::Dict",        "bsl::string", "operator []", vd_time );
    printf(fmt, "bsl::var::Dict",        "bsl::string", "get()",       vd_get_time );
    printf(fmt, "std::map",             "bsl::string", "operator []", bssm_time );
    printf(fmt, "std::map",             "std::string", "operator []", sssm_time );
    printf(fmt, "__gnu_cxx::hash_map",  "bsl::string", "operator []", bsgm_time );
    printf(fmt, "__gnu_cxx::hash_map",  "std::string", "operator []", ssgm_time );
    printf(fmt, "bsl::hashmap",         "bsl::string", "get(k&, v*)", bsbm_time );
    printf(fmt, "bsl::hashmap",         "std::string", "get(k&, v*)", ssbm_time );
    printf("char * => bsl::string time:%dus\n", csbs_time );
    printf("char * => std::string time:%dus\n", csss_time );
    printf("empty loop time:%dus\n", loop_time );

    ssbm.destroy();
    bsbm.destroy();
}

void test_dict_iterator( int groups, int group_size, int dict_size, int str_len ){

    bsl::var::Dict                   vd;     //var_dict;
    std::map<std::string, void *>   sssm;   //std_str_std_map;
    std::map<bsl::string, void *>   bssm;   //bsl_str_std_map;
    bsl::hashmap<std::string, void *, std_str_hash_func> ssbm;
    bsl::hashmap<bsl::string, void *, bsl_str_hash_func> bsbm;
    __gnu_cxx::hash_map<std::string, void *, std_str_hash_func> ssgm;  //std_str_gnu_map;
    __gnu_cxx::hash_map<bsl::string, void *, bsl_str_hash_func> bsgm;  //bsl_str_gnu_map;

    ssbm.create(dict_size); //比较豪华的size
    bsbm.create(dict_size); //比较豪华的size

    int vd_time = 0;     
    int sssm_time = 0;   
    int bssm_time = 0;   
    int ssbm_time = 0;
    int bsbm_time = 0;
    int ssgm_time = 0;  
    int bsgm_time = 0;  

    int csbs_time = 0;// time used in char * => bsl::string
    int csss_time = 0;// time used in char * => std::string
    int loop_time = 0;

    //灌数据
    for( int i = 0; i < dict_size; ++ i ){
        gen_random_str( cs[i], str_len );
        ss[i] = cs[i];  //make std str array
        bs[i] = cs[i];  //make bsl str array
        vd[bs[i]] = vd;
        sssm[ss[i]] = cs[i];    //make dictionarys    
        ssbm.set(ss[i], cs[i]);
        ssgm[ss[i]] = cs[i];
        
        bssm[bs[i]] = cs[i];
        bsbm.set(bs[i], cs[i]);
        bsgm[bs[i]] = cs[i];

    }
    see( vd.size() );
    see( sssm.size() );
    see( ssbm.size() );
    see( ssgm.size() );
    see( bssm.size() );
    see( bsbm.size() );
    see( bsgm.size() );


    for( int i = 0; i < groups; ++ i ){
        int pos = rand() % dict_size;
        struct timeval b, e;

        gettimeofday(&b, NULL);
        for( int j = 0; j < group_size; ++ j ){
            for( bsl::var::IVar::dict_iterator iter = vd.dict_begin(); iter != vd.dict_end(); ++ iter )
                ;
        }
        gettimeofday(&e, NULL);
        vd_time += (e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec );

        gettimeofday(&b, NULL);
        for( int j = 0; j < group_size; ++ j ){
            for( std::map<std::string, void*>::iterator iter = sssm.begin(); iter != sssm.end(); ++ iter )
                ;
        }
        gettimeofday(&e, NULL);
        sssm_time += (e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec );

        gettimeofday(&b, NULL);
        for( int j = 0; j < group_size; ++ j ){
            for( bsl::hashmap<std::string, void*, std_str_hash_func>::iterator iter = ssbm.begin(); iter != ssbm.end(); ++ iter )
                ;
        }
        gettimeofday(&e, NULL);
        ssbm_time += (e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec );

        gettimeofday(&b, NULL);
        for( int j = 0; j < group_size; ++ j ){
            for( __gnu_cxx::hash_map<std::string, void*, std_str_hash_func>::iterator iter = ssgm.begin(); iter != ssgm.end(); ++ iter )
                ;
        }
        gettimeofday(&e, NULL);
        ssgm_time += (e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec );

        gettimeofday(&b, NULL);
        for( int j = 0; j < group_size; ++ j ){
            for( std::map<bsl::string, void*>::iterator iter = bssm.begin(); iter != bssm.end(); ++ iter )
                ;
        }
        gettimeofday(&e, NULL);
        bssm_time += (e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec );

        gettimeofday(&b, NULL);
        for( int j = 0; j < group_size; ++ j ){
            for( bsl::hashmap<bsl::string, void*, bsl_str_hash_func>::iterator iter = bsbm.begin(); iter != bsbm.end(); ++ iter )
                ;
        }
        gettimeofday(&e, NULL);
        bsbm_time += (e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec );

        gettimeofday(&b, NULL);
        for( int j = 0; j < group_size; ++ j ){
            for( __gnu_cxx::hash_map<bsl::string, void*, bsl_str_hash_func>::iterator iter = bsgm.begin(); iter != bsgm.end(); ++ iter )
                ;
        }
        gettimeofday(&e, NULL);
        bsgm_time+= (e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec );

        gettimeofday(&b, NULL);
        for( int j = 0; j < group_size; ++ j ){
            bsl::basic_string<char, bsl::bsl_alloc<char> > __bs(cs[pos]);
        }
        gettimeofday(&e, NULL);
        csbs_time+= (e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec );

        gettimeofday(&b, NULL);
        for( int j = 0; j < group_size; ++ j ){
            std::string __ss(cs[pos]);
        }
        gettimeofday(&e, NULL);
        csss_time+= (e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec );

        gettimeofday(&b, NULL);
        for( int j = 0; j < group_size; ++ j ){
            ;
        }
        gettimeofday(&e, NULL);
        loop_time+= (e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec );

    }

    printf("groups:%d group_size:%d dict_size:%d str_len:%d\n", groups, group_size, dict_size, str_len );
    const char * fmt ="map: %-20s str: %-20s total time: %dus \n" ;
    printf(fmt, "bsl::var::Dict",        "bsl::string",  vd_time );
    printf(fmt, "std::map",             "bsl::string",  bssm_time );
    printf(fmt, "std::map",             "std::string",  sssm_time );
    printf(fmt, "__gnu_cxx::hash_map",  "bsl::string",  bsgm_time );
    printf(fmt, "__gnu_cxx::hash_map",  "std::string",  ssgm_time );
    printf(fmt, "bsl::hashmap",         "bsl::string",  bsbm_time );
    printf(fmt, "bsl::hashmap",         "std::string",  ssbm_time );

    ssbm.destroy();
    bsbm.destroy();
}


void test_dict_insert( int groups, int group_size, int dict_size, int str_len ){

    bsl::var::Dict                   vd;     //var_dict;
    std::map<std::string, void *>   sssm;   //std_str_std_map;
    std::map<bsl::string, void *>   bssm;   //bsl_str_std_map;
    bsl::hashmap<std::string, void *, std_str_hash_func> ssbm;
    bsl::hashmap<bsl::string, void *, bsl_str_hash_func> bsbm;
    __gnu_cxx::hash_map<std::string, void *, std_str_hash_func> ssgm;  //std_str_gnu_map;
    __gnu_cxx::hash_map<bsl::string, void *, bsl_str_hash_func> bsgm;  //bsl_str_gnu_map;

    ssbm.create(dict_size); //比较豪华的size
    bsbm.create(dict_size); //比较豪华的size

    int vd_time = 0;     
    int sssm_time = 0;   
    int bssm_time = 0;   
    int ssbm_time = 0;
    int bsbm_time = 0;
    int ssgm_time = 0;  
    int bsgm_time = 0;  

    int csbs_time = 0;// time used in char * => bsl::string
    int csss_time = 0;// time used in char * => std::string
    int loop_time = 0;

    //灌数据
    for( int i = 0; i < dict_size; ++ i ){
        gen_random_str( cs[i], str_len );
        ss[i] = cs[i];  //make std str array
        bs[i] = cs[i];  //make bsl str array
        /*
        vd[bs[i]] = vd;
        sssm[ss[i]] = cs[i];    //make dictionarys    
        ssbm.set(ss[i], cs[i]);
        ssgm[ss[i]] = cs[i];
        
        bssm[bs[i]] = cs[i];
        bsbm.set(bs[i], cs[i]);
        bsgm[bs[i]] = cs[i];
        */

    }

    for( int i = 0; i < groups; ++ i ){
        int pos = rand() % dict_size;
        struct timeval b, e;

        gettimeofday(&b, NULL);
        for( int j = 0; j < group_size; ++ j ){
            for( int k = 0; k < dict_size; ++ k ){
                vd[bs[k]] = vd;
            }
            vd.clear();
        }
        gettimeofday(&e, NULL);
        vd_time += (e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec );

        gettimeofday(&b, NULL);
        for( int j = 0; j < group_size; ++ j ){
            for( int k = 0; k< dict_size; ++ k ){
                sssm[ss[i]] = cs[i];
            }
            sssm.clear();
        }
        gettimeofday(&e, NULL);
        sssm_time += (e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec );

        gettimeofday(&b, NULL);
        for( int j = 0; j < group_size; ++ j ){
            for( int k = 0; k < dict_size; ++ k ){
                ssbm.set(ss[i], cs[i]);
            }
            ssbm.clear();
        }
        gettimeofday(&e, NULL);
        ssbm_time += (e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec );

        gettimeofday(&b, NULL);
        for( int j = 0; j < group_size; ++ j ){
            for( int k = 0; k < dict_size; ++ k ){
                ssgm[ss[i]] = cs[i];
            }
            ssgm.clear();
        }
        gettimeofday(&e, NULL);
        ssgm_time += (e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec );

        gettimeofday(&b, NULL);
        for( int j = 0; j < group_size; ++ j ){
            for( int k = 0; k < dict_size; ++ k ){
                bssm[bs[i]] = cs[i];
            }
            bssm.clear();
        }
        gettimeofday(&e, NULL);
        bssm_time += (e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec );

        gettimeofday(&b, NULL);
        for( int j = 0; j < group_size; ++ j ){
            for( int k = 0; k < dict_size; ++ k ){
                bsbm.set(bs[i], cs[i]);
            }
            bsbm.clear();
        }
        gettimeofday(&e, NULL);
        bsbm_time += (e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec );

        gettimeofday(&b, NULL);
        for( int j = 0; j < group_size; ++ j ){
            for( int k = 0; k < dict_size; ++ k ){
                bsgm[bs[i]] = cs[i];
            }
            bsgm.clear();
        }
        gettimeofday(&e, NULL);
        bsgm_time+= (e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec );

        gettimeofday(&b, NULL);
        for( int j = 0; j < group_size; ++ j ){
            bsl::basic_string<char, bsl::bsl_alloc<char> > __bs(cs[pos]);
        }
        gettimeofday(&e, NULL);
        csbs_time+= (e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec );

        gettimeofday(&b, NULL);
        for( int j = 0; j < group_size; ++ j ){
            std::string __ss(cs[pos]);
        }
        gettimeofday(&e, NULL);
        csss_time+= (e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec );

        gettimeofday(&b, NULL);
        for( int j = 0; j < group_size; ++ j ){
            ;
        }
        gettimeofday(&e, NULL);
        loop_time+= (e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec );

    }

    printf("groups:%d group_size:%d dict_size:%d str_len:%d\n", groups, group_size, dict_size, str_len );
    const char * fmt ="map: %-20s str: %-20s total time: %dus \n" ;
    printf(fmt, "bsl::var::Dict",        "bsl::string",  vd_time );
    printf(fmt, "std::map",             "bsl::string",  bssm_time );
    printf(fmt, "std::map",             "std::string",  sssm_time );
    printf(fmt, "__gnu_cxx::hash_map",  "bsl::string",  bsgm_time );
    printf(fmt, "__gnu_cxx::hash_map",  "std::string",  ssgm_time );
    printf(fmt, "bsl::hashmap",         "bsl::string",  bsbm_time );
    printf(fmt, "bsl::hashmap",         "std::string",  ssbm_time );

    ssbm.destroy();
    bsbm.destroy();
}
int usage(){
    fprintf( stderr, "usage:\n./test_perform_dict [random|iterator|insert] [groups] [group_size] [dict_size(<=%d)] [str_len(<%d)]\n",
            MAX_DICT_SIZE, MAX_STR_LEN);
    return 0;
}

int timval_diff( const timeval& begin, const timeval& end ){
    return ( end.tv_sec - begin.tv_sec ) * 100 * 10000 + ( end.tv_usec - begin.tv_usec );
}

int string_compare( ){
    struct timeval b,e;
    char s[100001];
    memset(s,'a',sizeof(s));
    s[100000] = 0;
    
    gettimeofday(&b,NULL);
    std::string ss1(s), ss2(s);
    gettimeofday(&e,NULL);
    printf("std::string::string(): %dus\n", timval_diff( b, e ) );

    gettimeofday(&b,NULL);
    bsl::string bs1(s), bs2(s);
    gettimeofday(&e,NULL);

    printf("bsl::string::string(): %dus\n", timval_diff( b, e ) );

    gettimeofday(&b,NULL);
    ss1 == ss2;
    gettimeofday(&e,NULL);

    printf("std::string::operator ==: %dus\n", timval_diff( b, e ) );

    gettimeofday(&b,NULL);
    bs1 == bs2;
    gettimeofday(&e,NULL);

    printf("bsl::string::operator ==: %dus\n", timval_diff( b, e ) );

    gettimeofday(&b,NULL);
    assert(!memcmp( bs1.c_str(), bs2.c_str(), bs1.length() ));
    gettimeofday(&e,NULL);

    printf("memcmp: %dus\n", timval_diff( b, e ) );

    return 0;
}

int main(int argc, char *argv[]){

    if ( argc < 2 ){
        return usage();
    }
    int groups, group_size, dict_size, str_len;
    if ( 0 == strcmp(argv[1], "random") ){
        if ( argc < 6 ){
            return usage();
        }
        sscanf( argv[2], "%d", &groups );
        sscanf( argv[3], "%d", &group_size );
        sscanf( argv[4], "%d", &dict_size );
        sscanf( argv[5], "%d", &str_len );
        test_dict_random( groups, group_size, dict_size, str_len );
        return 0;
    }else if ( 0 == strcmp( argv[1], "iterator" ) ){
        if ( argc < 6 ){
            return usage();
        }
        sscanf( argv[2], "%d", &groups );
        sscanf( argv[3], "%d", &group_size );
        sscanf( argv[4], "%d", &dict_size );
        sscanf( argv[5], "%d", &str_len );
        test_dict_iterator( groups, group_size, dict_size, str_len );
        return 0;
        
    }else if ( 0 == strcmp( argv[1], "insert" ) ){
        if ( argc < 6 ){
            return usage();
        }
        sscanf( argv[2], "%d", &groups );
        sscanf( argv[3], "%d", &group_size );
        sscanf( argv[4], "%d", &dict_size );
        sscanf( argv[5], "%d", &str_len );
        test_dict_insert( groups, group_size, dict_size, str_len );
        return 0;
        
    }else{
        return usage();
    }
    
}
/* vim: set ts=4 sw=4 sts=4 tw=100 */
/* vim: set ts=4 sw=4 sts=4 tw=100 */
