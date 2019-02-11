/***************************************************************************
 * 
 * Copyright (c) 2010 Baidu.com, Inc. All Rights Reserved
 * $Id$ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file bsl_test_bin_buffer.h
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2010/08/17 11:29:41
 * @version $Revision$ 
 * @brief 
 *  
 **/

#include <cstring>
#include <cassert>

#include "cxxtest/TestSuite.h"

#include <bsl/pool/bsl_xcompool.h>
#include <bsl/BinBuffer.h>
       
class test_BinBuffer_main: public CxxTest::TestSuite
{

    template<class Tp>
    Tp get_bin_value(const char * cstr, int len) {
        Tp val;
        char* ch = (char *)&val;
        for (int i = 0; i < len; i ++) {
            ch[i] = cstr[i];
        }
        return val;
    }

    template<class Tp>
    Tp min(Tp a,Tp b) {
        return a < b ? a : b;
    }

    template<class Tp>
    Tp max(Tp a,Tp b) {
        return a > b ? a : b;
    }
    void get_str(char * str,const char* cstr,size_t len) {
        size_t i;
        for (i = 0; i < len; i ++) {
            str[i] = cstr[i];
        }
        str[i] = '\0';
    }

     
public:
    test_BinBuffer_main() {
        printf("begin\n");
    }
    void test_special()
    {
        bsl::xcompool g_xcompool;
        g_xcompool.create();
        // push char
        {
            char str[100];
            bsl::BinBuffer buf(g_xcompool,0);
            buf.push('a');
            get_str(str,buf.data(),buf.size());
            TS_ASSERT( 0 == strcmp(str,"a") );
            buf.push('a');
            get_str(str,buf.data(),buf.size());            
            TS_ASSERT( 0 == strcmp(str,"aa") );
            buf.push("aaa",3);
            TS_ASSERT( 0 == strcmp(buf.data(),"aaaaa") );
            buf.push('\0');
            TS_ASSERT( buf.size() == 6 );
            buf.push('\0').push('\0');
            TS_ASSERT( buf.size() == 8 );
        }
        // push signed char
        {
            bsl::BinBuffer buf(g_xcompool,10);
            signed char sc = 12;
            buf.push(sc);
            signed char sc2 = get_bin_value<signed char>(buf.data(),sizeof(signed char));
            TS_ASSERT( sc == sc2 );
            sc = 15;
            buf.push(sc);
            sc2 = get_bin_value<signed char>(buf.data()+min(buf.get_pack(),sizeof(signed char)),sizeof(signed char));
            TS_ASSERT( sc == sc2 );
            //printf("=%hhd=\n",sc);
            //printf("=%d=\n",buf.size());
        }
        // push unsigned char
        {
            bsl::BinBuffer buf(g_xcompool,10);
            unsigned char sc = 12;
            buf.push(sc);
            unsigned char sc2 = get_bin_value<signed char>(buf.data(),sizeof(unsigned char));
            TS_ASSERT( sc == sc2 );
            sc = 15;
            buf.push(sc);
            sc2 = get_bin_value<unsigned char>(buf.data()+min(buf.get_pack(),sizeof(unsigned char)),sizeof(unsigned char));
            TS_ASSERT( sc == sc2 );
        }
        // push count char
        {
            bsl::BinBuffer buf(g_xcompool,0);
            buf.push(5,'a');
            TS_ASSERT( 0 == strcmp(buf.data(),"aaaaa") );
            buf.push(5,'\0');
            TS_ASSERT( 0 == strcmp(buf.data(),"aaaaa") );
            buf.push(5,'b');
            //TS_ASSERT( 0 == strcmp(buf.data(),"aaaaabbbbb") );
            TS_ASSERT( 15 == buf.size() );
        }
        // push wchar_t
        {
            bsl::BinBuffer buf(g_xcompool,0);
            wchar_t wc = L'c';
            buf.push(wc);
            TS_ASSERT( 0 == strcmp(buf.data(),"c") );
            buf.push(wc);
            //printf("==%d==\n",sizeof(wchar_t));
            //printf("==%d==\n",buf.size());
            //printf("==%s==\n",buf.data());
            //TS_ASSERT( 0 == strcmp(buf.data(),"cc") );
            //buf.push("ccc");
            //TS_ASSERT( 0 == strcmp(buf.data(),"ccccc") );
        }
        // push wchar_t*
        {
            /*
            bsl::BinBuffer buf(g_xcompool,100);
            buf.push("百度");
            TS_ASSERT(100 == buf.capacity()); 
            TS_ASSERT(4 == buf.size());
            buf.push0("百度");
            TS_ASSERT(9 == buf.size());
            printf("==%s==\n",buf.data());
            */
        }
        // push int
        {
            bsl::BinBuffer buf(g_xcompool,20);
            int val = 18977;
            buf.push(val);
            int val2 = get_bin_value<int>(buf.data(),sizeof(int));
            TS_ASSERT( val == val2 );
            val = 889922;
            buf.push(val);
            val2 = get_bin_value<int>(buf.data()+min(buf.get_pack(),sizeof(int)),sizeof(int));
            TS_ASSERT( val == val2 );
        }
        // push long int
        {
            bsl::BinBuffer buf(g_xcompool,20);
            long int val = 18977;
            buf.push(val);
            long int val2 = get_bin_value<long int>(buf.data(),sizeof(long int));
            TS_ASSERT( val == val2 );
            val = 889922;
            buf.push(val);
            val2 = get_bin_value<long int>(buf.data()+max(buf.get_pack(),sizeof(long int)),sizeof(long int));
            TS_ASSERT( val == val2 );
        }
        // push unsigned int
        {
            bsl::BinBuffer buf(g_xcompool,20);
            unsigned int val = 18977;
            buf.push(val);
            unsigned int val2 = get_bin_value<unsigned int>(buf.data(),sizeof(unsigned int));
            TS_ASSERT( val == val2 );
            val = 889922;
            buf.push(val);
            val2 = get_bin_value<unsigned int>(buf.data()+min(buf.get_pack(),sizeof(unsigned int)),sizeof(unsigned int));
            TS_ASSERT( val == val2 );
        }
        // push unsigned long int
        {
            bsl::BinBuffer buf(g_xcompool,20);
            unsigned long int val = 18977;
            buf.push(val);
            unsigned long int val2 = get_bin_value<unsigned long int>(buf.data(),sizeof(unsigned long int));
            TS_ASSERT( val == val2 );
            val = 889922;
            buf.push(val);
            val2 = get_bin_value<unsigned long int>(buf.data()+max(buf.get_pack(),sizeof(unsigned long int)),sizeof(unsigned long int));
            TS_ASSERT( val == val2 );
        }
        // push char *
        {
            bsl::BinBuffer buf(g_xcompool,0);
            buf.push("abcdef",6);
            TS_ASSERT( 0 == strcmp(buf.data(),"abcdef") );
            buf.push("aa",2);
            TS_ASSERT( buf.size() == 8 );
            buf.push("bb",2);
            TS_ASSERT( buf.size() == 10 );
        }
        // push char *
        {
            bsl::BinBuffer buf(g_xcompool,100);
            buf.push("abcdef",6);
            char str[100];
            get_str(str,buf.data(),buf.size());
            TS_ASSERT( 0 == strcmp(str,"abcdef") );
            buf.push("aa",2);
            TS_ASSERT( buf.size() == 8 );
            buf.push("bb",2);
            TS_ASSERT( buf.size() == 10 );
        }

        // push char * , sub_str_len
        {
            bsl::BinBuffer buf(g_xcompool,0);
            buf.push("abcdef",3);
            char str[1000];
            get_str(str,buf.data(),buf.size());
            TS_ASSERT( 0 == strcmp(str,"abc" ) );
            TS_ASSERT( !buf.truncated() );
            TS_ASSERT( !buf.ever_truncated() );
            buf.push("ABDEF",5);
            get_str(str,buf.data(),buf.size());
            TS_ASSERT( 0 == strcmp(str,"abcABDEF") );
            buf.push("e",1);
            buf.push("f",100);
            TS_ASSERT( 109 == buf.size() );
            buf.clear();
            buf.push("abcdefg",3);
            get_str(str,buf.data(),buf.size());
            TS_ASSERT( 0 == strcmp(str,"abc") );
            TS_ASSERT( 3 == buf.size() );
            buf.push("abc",4);
            TS_ASSERT( 7 == buf.size() ); 
        }
 
        // push char * , sub_str_len
        {
            bsl::BinBuffer buf(g_xcompool,99);
            buf.push("abcdef",3);
            TS_ASSERT( 0 == strcmp(buf.data(),"abc" ) );
            TS_ASSERT( !buf.truncated() );
            TS_ASSERT( !buf.ever_truncated() );
            buf.push("ABDEF",5);
            char str[100];
            get_str(str,buf.data(),buf.size());
            TS_ASSERT( 0 == strcmp(str,"abcABDEF") );
            buf.push("e",1);
            buf.push("f",100);
            TS_ASSERT( 109 == buf.size() );
            buf.clear();
            buf.push("abcdefg",3);
            get_str(str,buf.data(),buf.size());
            TS_ASSERT( 0 == strcmp(str,"abc") );
            TS_ASSERT( 3 == buf.size() );
            buf.push("abc",4);
            TS_ASSERT( 7 == buf.size() ); 
        }
        /*
        // push capacity char *
        {
            // no 1 NULL;
            bsl::BinBuffer buf(g_xcompool,0);
            buf.push(0,"a");
            TS_ASSERT( 0 == buf.size() );
            // cap < size
            buf.push(1,"abcd");
            TS_ASSERT( 1 == buf.size() );
            TS_ASSERT( buf.truncated() );
            TS_ASSERT( buf.ever_truncated() );
            // cap > size
            buf.push(10,"efgh");
            TS_ASSERT( 11 == buf.size() );
            TS_ASSERT( !buf.truncated() );
            // cap == size
            buf.push(4,"xyzz");
            TS_ASSERT( 15 == buf.size() );
            TS_ASSERT( !buf.truncated() );
            TS_ASSERT( buf.ever_truncated() );
        }
         // push capacity char *
        {
            // no 1 NULL;
            bsl::BinBuffer buf(g_xcompool,100);
            buf.push(0,"a");
            TS_ASSERT( 0 == buf.size() );
            // cap < size
            buf.push(1,"abcd");
            TS_ASSERT( 1 == buf.size() );
            TS_ASSERT( buf.truncated() );
            TS_ASSERT( buf.ever_truncated() );
            // cap > size
            buf.push(10,"efgh");
            TS_ASSERT( 11 == buf.size() );
            TS_ASSERT( !buf.truncated() );
            // cap == size
            buf.push(4,"xyzz");
            TS_ASSERT( 15 == buf.size() );
            TS_ASSERT( !buf.truncated() );
            TS_ASSERT( buf.ever_truncated() );
        }

        // push0 capacity char *
        {
            // no 1 NULL;
            bsl::BinBuffer buf(g_xcompool,0);
            buf.push0(0,"a");
            TS_ASSERT( 0 == buf.size() );
            
            // cap < size
            buf.push0(1,"abcd");
            TS_ASSERT( 1 == buf.size() );
            TS_ASSERT( 0 == strcmp(buf.data(),"\0") );
            TS_ASSERT( buf.truncated() );
            TS_ASSERT( buf.ever_truncated() );
            // cap > size
            buf.push0(10,"efgh");
            TS_ASSERT( 11 == buf.size() );
            TS_ASSERT( !buf.truncated() );
            // cap == size
            buf.push0(4,"xyzz");
            TS_ASSERT( 15 == buf.size() );
            TS_ASSERT( buf.truncated() );
            TS_ASSERT( buf.ever_truncated() );
        }
        // push0 capacity char *
        {
            // no 1 NULL;
            bsl::BinBuffer buf(g_xcompool,100);
            buf.push0(0,"a");
            TS_ASSERT( 0 == buf.size() );
            
            // cap < size
            buf.push0(1,"abcd");
            TS_ASSERT( 1 == buf.size() );
            TS_ASSERT( 0 == strcmp(buf.data(),"\0") );
            TS_ASSERT( buf.truncated() );
            TS_ASSERT( buf.ever_truncated() );
            // cap > size
            buf.push0(10,"efgh");
            TS_ASSERT( 11 == buf.size() );
            TS_ASSERT( !buf.truncated() );
            // cap == size
            buf.push0(4,"xyzz");
            TS_ASSERT( 15 == buf.size() );
            TS_ASSERT( buf.truncated() );
            TS_ASSERT( buf.ever_truncated() );
        }
        // push capacity char * sub_str_len
        {
            // no 1 NULL;
            bsl::BinBuffer buf(g_xcompool,0);
            buf.push(0,"a");
            TS_ASSERT( 0 == buf.size() );
            // cap < size
            buf.push(1,"abcd",10);
            TS_ASSERT( 1 == buf.size() );
            TS_ASSERT( buf.truncated() );
            TS_ASSERT( buf.ever_truncated() );
            // cap > size
            buf.push(10,"efgh",1);
            TS_ASSERT( 11 == buf.size() );
            TS_ASSERT( !buf.truncated() );
            // cap == size
            buf.push(4,"xyzz",10);
            TS_ASSERT( 15 == buf.size() );
            TS_ASSERT( buf.truncated() );
            TS_ASSERT( buf.ever_truncated() );
        }
        // push capacity char *
        {
            // no 1 NULL;
            bsl::BinBuffer buf(g_xcompool,100);
            buf.push(0,"a");
            TS_ASSERT( 0 == buf.size() );
            // cap < size
            buf.push(1,"abcd",10);
            TS_ASSERT( 1 == buf.size() );
            TS_ASSERT( buf.truncated() );
            TS_ASSERT( buf.ever_truncated() );
            // cap > size
            buf.push(10,"efgh",1);
            TS_ASSERT( 11 == buf.size() );
            TS_ASSERT( !buf.truncated() );
            // cap == size
            buf.push(4,"xyzz",10);
            TS_ASSERT( 15 == buf.size() );
            TS_ASSERT( buf.truncated() );
            TS_ASSERT( buf.ever_truncated() );
        }
        // push0 capacity char * sub_str_len
        {
            // no 1 NULL;
            bsl::BinBuffer buf(g_xcompool,0);
            buf.push0(0,"a");
            TS_ASSERT( 0 == buf.size() );
            
            // cap < size
            buf.push0(1,"abcd",10);
            TS_ASSERT( 1 == buf.size() );
            TS_ASSERT( 0 == strcmp(buf.data(),"\0") );
            TS_ASSERT( buf.truncated() );
            TS_ASSERT( buf.ever_truncated() );
            // cap > size
            buf.push0(10,"efgh",1);
            TS_ASSERT( 11 == buf.size() );
            TS_ASSERT( !buf.truncated() );
            // cap == size
            buf.push0(4,"xyzz",10);
            TS_ASSERT( 15 == buf.size() );
            TS_ASSERT( buf.truncated() );
            TS_ASSERT( buf.ever_truncated() );
        }
        // push0 capacity char *
        {
            // no 1 NULL;
            bsl::BinBuffer buf(g_xcompool,100);
            buf.push0(0,"a");
            TS_ASSERT( 0 == buf.size() );
            
            // cap < size
            buf.push0(1,"abcd",10);
            TS_ASSERT( 1 == buf.size() );
            TS_ASSERT( 0 == strcmp(buf.data(),"\0") );
            TS_ASSERT( buf.truncated() );
            TS_ASSERT( buf.ever_truncated() );
            // cap > size
            buf.push0(10,"efgh",1);
            TS_ASSERT( 11 == buf.size() );
            TS_ASSERT( !buf.truncated() );
            // cap == size
            buf.push0(4,"xyzz",10);
            TS_ASSERT( 15 == buf.size() );
            TS_ASSERT( buf.truncated() );
            TS_ASSERT( buf.ever_truncated() );
        }*/
        // push long long
        {
            bsl::BinBuffer buf(g_xcompool,20);
            long long val = 333333318977LL;
            buf.push(val);
            long long val2 = get_bin_value<long long>(buf.data(),sizeof(long long));
            TS_ASSERT( val == val2 );
            val = 889922LL;
            buf.push(val);
            val2 = get_bin_value<long long>(buf.data()+max(buf.get_pack(),sizeof(long long)),sizeof(long long));
            TS_ASSERT( val == val2 );
        }
        // push unsigned long long
        {
            bsl::BinBuffer buf(g_xcompool,20);
            unsigned long long val = 333333318977ULL;
            buf.push(val);
            unsigned long long val2 = get_bin_value<unsigned long long>(buf.data(),sizeof(unsigned long long));
            TS_ASSERT( val == val2 );
            val = 889922ULL;
            buf.push(val);
            val2 = get_bin_value<unsigned long long>(buf.data()+max(buf.get_pack(),sizeof(unsigned long long)),sizeof(unsigned long long));
            TS_ASSERT( val == val2 );
        }

        // push double
        {
            bsl::BinBuffer buf(g_xcompool,20);
            double val = 18977;
            buf.push(val);
            double val2 = get_bin_value<double>(buf.data(),sizeof(double));
            TS_ASSERT( val == val2 );
            val = 889922;
            buf.push(val);
            val2 = get_bin_value<double>(buf.data()+max(buf.get_pack(),sizeof(double)),sizeof(double));
            TS_ASSERT( val == val2 );
        }
        // push long double
        {
            bsl::BinBuffer buf(g_xcompool,20);
            long double val = 18977;
            buf.push(val);
            long double val2 = get_bin_value<long double>(buf.data(),sizeof(long double));
            TS_ASSERT( val == val2 );
            val = 889922;
            buf.push(val);
            val2 = get_bin_value<long double>(buf.data()+max(buf.get_pack(),sizeof(long double)),sizeof(long double));
            TS_ASSERT( val == val2 );
        }
        // push short
        {
            bsl::BinBuffer buf;
            short val = 100;
            buf.push(val);
            short val2 = get_bin_value<short>(buf.data(),sizeof(short));
            TS_ASSERT( val == val2 );
        }
        // push unsigned short
        {
            bsl::BinBuffer buf;
            unsigned short val = 200;
            buf.push(val);
            unsigned short val2 = get_bin_value<unsigned short>(buf.data(),sizeof(unsigned short));
            TS_ASSERT( val == val2 );
        }
        /*
        // push format ...
        {
            bsl::BinBuffer buf(g_xcompool,0);
            buf.pushf("%d%s",100,"hehe");
            TS_ASSERT( 7 == buf.size() ); 
            buf.push(100);
            TS_ASSERT( 8 + sizeof(int) == buf.size() );
        }
        // push format ...
        {
            bsl::BinBuffer buf(g_xcompool,0);
            buf.pushf0("%d%s",100,"hehe");
            TS_ASSERT( 8 == buf.size() );  
        }
        // push capacity format ...
        {
            bsl::BinBuffer buf(g_xcompool,0);
            buf.pushf(5,"%d%s",100,"hehe");
            TS_ASSERT( buf.truncated() );
            TS_ASSERT( buf.ever_truncated() );
            TS_ASSERT( 5 == buf.size() );
        }
        // push capacity format ...
        {
            bsl::BinBuffer buf(g_xcompool,0);
            buf.pushf0(5,"%d%s",100,"hehe");
            TS_ASSERT( buf.truncated() );
            TS_ASSERT( buf.ever_truncated() );
            TS_ASSERT( 5 == buf.size() );
            TS_ASSERT( 0 == strcmp(buf.data(),"100h") );
        }
        */
        // transfer_from
        {
            bsl::BinBuffer buf(g_xcompool,100);
            buf.push("a").push("b").push("asdfad;fkja;sdfja;sdfjdjf");
            size_t size = buf.size();
            bsl::BinBuffer buf2;
            buf2.transfer_from(buf);
            TS_ASSERT( size == buf2.size() );
            buf.swap(buf2);
            TS_ASSERT( buf2.empty() );
            TS_ASSERT( size == buf.size() );
            buf.clear();
            buf.reserve(10);
            buf.push("1234567890");
            TS_ASSERT( !buf.full() );
            TS_ASSERT( 100 == buf.capacity() );
        }
        // push true false
        {
            bsl::BinBuffer buf;
            buf.push(true);
            bool tmp = get_bin_value<bool>(buf.data(),sizeof(bool));
            TS_ASSERT( tmp == true );
            TS_ASSERT( tmp != false );
            buf.clear();
            buf.push(false);
            tmp = get_bin_value<bool>(buf.data(),sizeof(bool));
            TS_ASSERT( tmp == false );
            TS_ASSERT( tmp != true );
        }
        // pack
        {
            bsl::BinBuffer buf;
            size_t pack = sizeof(long);
            size_t size = 0;
            TS_ASSERT(true == buf.set_pack(pack));
            TS_ASSERT(size == buf.size());
            buf.push('a');
            size = 1;
            TS_ASSERT(size == buf.size());
            buf.push(short(1));
            size = 4;
            TS_ASSERT(size == buf.size());
            buf.push(0L);
            size = ((size + pack - 1) & (~(pack - 1))) + sizeof(long);
            TS_ASSERT(size == buf.size());
            buf.push(1.0);
            size = ((size + pack - 1) & (~(pack - 1))) + sizeof(double);
            TS_ASSERT(size == buf.size());
        }
        /*
        // pushf format ...
        {
            bsl::BinBuffer buf;
            buf.pushf("%d",100).pushf("%d",100);
            TS_ASSERT( 6 == buf.size() );
            buf.pushf0("%d",100).pushf0("%d",100);
            TS_ASSERT( 14 == buf.size() );
        }
        // push capacity format ...
        {
            bsl::BinBuffer buf;
            buf.pushf(4,"%s","abcdef").pushf(3,"%s","12345");
            TS_ASSERT( 7 == buf.size() );
            //TS_ASSERT( 0 == strcmp(buf.data(),"abcd123") );
        }
        // pushf0
        {
            bsl::BinBuffer buf;
            buf.pushf0(4,"%s","abcdef").pushf0(3,"%s","12345");
            TS_ASSERT( 7 == buf.size() );
        }
        */
        // push
        {
            bsl::BinBuffer buf;
            buf.push("123456789",3).push("abcdef",5);
            TS_ASSERT( 8 == buf.size() );
        }
        /*
        // push
        {
            bsl::BinBuffer buf;
            buf.push0(3,"123456789",3).push0(5,"abcdef",1).push0(1,"abcd",1);
        }*/
        // construct
        {
            bsl::BinBuffer buf(size_t(-100));
            TS_ASSERT(buf.capacity() == 0);
            bsl::BinBuffer buf2(1, size_t(-1));
            TS_ASSERT(buf2.get_pack() == bsl::BinBuffer::DEFAULT_PACK);
            bsl::BinBuffer buf3(1,1000);
            TS_ASSERT(buf3.get_pack() == bsl::BinBuffer::DEFAULT_PACK );
        }
        {
            bsl::BinBuffer buf;
            const char *data = "hello world";
            buf<<data;
        }
        // bool..
        {
            bsl::BinBuffer buf;
            buf.push(false);
            TS_ASSERT( buf.size() == 1 );
        }
        // null
        {
            bsl::BinBuffer buf;
            buf.push((const char*)NULL,100);
        }
        // pack
        {
            bsl::BinBuffer buf;
            int a = 100;
            double b = 1.0;
            char ch = 'a';
            buf.push(a).push(b).push(ch);
            TS_ASSERT( 13 == buf.size() );
            bsl::BinBuffer buf2;
            buf2.push('c');
            buf2.push(buf);
            TS_ASSERT( 17 == buf2.size() );
            buf2.clear();
            buf2.push(1);
            buf.push(buf2);
            TS_ASSERT( 20 == buf.size() );
        }
        {
            bsl::BinBuffer buf;
            int a = 100;
            double b = 1.0;
            char ch = 'a';
            buf<<(a)<<(b)<<(ch);
            TS_ASSERT( 13 == buf.size() );
            bsl::BinBuffer buf2;
            buf2<<('c');
            buf2<<(buf);
            TS_ASSERT( 17 == buf2.size() );
            buf2.clear();
            buf2<<(1);
            buf<<(buf2);
            TS_ASSERT( 20 == buf.size() );


        }

    }
};


/* vim: set ts=4 sw=4 sts=4 tw=100 */
