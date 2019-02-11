/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: ITestVar.h,v 1.3 2010/04/28 12:45:33 scmpf Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file ITestVar.h
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2008/09/28 16:55:49
 * @version $Revision: 1.3 $ 
 * @brief 
 *  
 **/
#ifndef  __ITESTVAR_H_
#define  __ITESTVAR_H_

#ifndef LLONG_MAX
#define LLONG_MAX	9223372036854775807LL
#endif

#ifndef LLONG_MIN
#define LLONG_MIN	(-LLONG_MAX-1LL)
#endif

#ifndef ULLONG_MAX
#define ULLONG_MAX	18446744073709551615ULL
#endif

#define ECHO_DO( __STATEMENT__ ) \
    do{ \
        std::cout<<"\n/""/"<< /*__FILE__ <<":"<< __LINE__<<":\t"<<*/ (#__STATEMENT__)<<";"<<std::endl;   \
        __STATEMENT__;  \
    }while(0)   

#include<typeinfo>

class ITestVar{
public:
    typedef bsl::var::IVar::string_type   string_type;
    typedef bsl::var::IVar::field_type    field_type;
    typedef bsl::var::ArrayIterator      array_iterator;
    typedef bsl::var::ArrayConstIterator array_const_iterator;

    //special methods
    virtual void test_special() = 0;

    //methods for all
    virtual void test_clear() = 0;
    virtual void test_dump() = 0;
    virtual void test_to_string() = 0;
    virtual void test_get_type() = 0;
    virtual void test_operator_assign() = 0;
    virtual void test_clone() = 0;
    virtual void test_mask() = 0;

    //method for value
    virtual void test_bool()    = 0;
    virtual void test_raw()     = 0;
    virtual void test_number()  = 0;
    virtual void test_string()  = 0;

    //methods for array and dict
    virtual void test_array() = 0;

    //methods for dict
    virtual void test_dict() = 0;

    //methods for callable
    virtual void test_callable() = 0;

    virtual void test_all( ){
        //special methods
        ECHO_DO( test_special() );

        //methods for all
        ECHO_DO( test_clear() );
        ECHO_DO( test_dump() );
        ECHO_DO( test_to_string() );
        ECHO_DO( test_get_type() );
        ECHO_DO( test_operator_assign() );
        ECHO_DO( test_clone() );
       // ECHO_DO( test_mask() );
        
        //methods for different categories
        ECHO_DO( test_bool()    );
        ECHO_DO( test_raw()     );
        ECHO_DO( test_number()  );
        ECHO_DO( test_string()  );
        ECHO_DO( test_array() );
        ECHO_DO( test_dict() );
        ECHO_DO( test_callable() );
        printf("test:%s Accepted!\n",typeid(*this).name());

    }
};


#endif  //__ITESTVAR_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
