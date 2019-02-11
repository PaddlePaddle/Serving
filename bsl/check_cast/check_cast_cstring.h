/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: check_cast_cstring.h,v 1.3 2009/10/14 08:24:58 chenxm Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file check_cast_cstring.h
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2008/11/11 18:36:02
 * @version $Revision: 1.3 $ 
 * @brief 
 *  
 **/
#ifndef  __CHECK_CAST_CSTRING_H_
#define  __CHECK_CAST_CSTRING_H_

//internal use
#ifndef  __CHECK_CAST_H_
#error "this file cannot be included directly, please include \"check_cast.h\" instead"
#endif

namespace bsl{
    
    //from c-style string
    template<>
        inline char check_cast<char,const char *>( const char * s ){
            if ( !s ){
                throw bsl::NullPointerException()<<BSL_EARG<<"s";
            }
            return s[0];
        }

    template<>
        long check_cast<long, const char *> ( const char * s );

    //signed char is used as int8
    template<>
        inline signed char check_cast<signed char,const char *>( const char * s ){
            return check_cast<signed char>( check_cast<long>( s ) );
        }

    //unsigned char is used as uint8
    template<>
        inline unsigned char check_cast<unsigned char,const char *>( const char * s ){
            return check_cast<unsigned char>( check_cast<long>( s ) );
        }

    template<>
        inline short check_cast<short, const char *> ( const char * s ){
            return check_cast<short>( check_cast<long>( s ) );
        }

    template<>
        inline int check_cast<int, const char *> ( const char * s ){
            return check_cast<int>( check_cast<long>( s ) );
        }


    template<>
        long long check_cast<long long, const char *>( const char * s );

    template<>
        unsigned long check_cast<unsigned long, const char *>( const char * s );

    template<>
        inline unsigned short check_cast<unsigned short, const char *>( const char * s ){
            return check_cast<unsigned short>( check_cast<unsigned long>( s ) );
        }

    template<>
        inline unsigned int check_cast<unsigned int, const char *>( const char * s ){
            return check_cast<unsigned int>( check_cast<unsigned long>( s ) );
        }


    template<>
        unsigned long long check_cast<unsigned long long, const char *>( const char * s );

    template<>
        float check_cast<float, const char *>( const char * s );

    template<>
        double check_cast<double, const char *>( const char * s );
    

    template<>
        long double check_cast<long double, const char *>( const char * s );


    template<>
        inline char check_cast<char,char *>( char *s ){
            if ( !s ){
                throw bsl::NullPointerException()<<BSL_EARG<<"s";
            }
            return s[0];
        }

    template<>
        inline long check_cast<long, char *> ( char * s ){
            return check_cast<long, const char *>(s);
        }

    template<>
        inline short check_cast<short, char *> ( char * s ){
            return check_cast<short>( check_cast<long>( s ) );
        }

    //signed char is used as int8
    template<>
        inline signed char check_cast<signed char,char *>( char * s ){
            return check_cast<signed char>( check_cast<long>( s ) );
        }

    //unsigned char is used as uint8
    template<>
        inline unsigned char check_cast<unsigned char,char *>( char * s ){
            return check_cast<unsigned char>( check_cast<long>( s ) );
        }

    template<>
        inline int check_cast<int, char *> ( char * s ){
            return check_cast<int>( check_cast<long>( s ) );
        }


    template<>
        inline long long check_cast<long long, char *>( char *s ){
            return check_cast<long long, const char *>(s);
        }

    template<>
        inline unsigned long check_cast<unsigned long, char *>( char *s ){
            return check_cast<unsigned long, const char *>(s);
        }

    template<>
        inline unsigned short check_cast<unsigned short, char *>( char *s ){
            return check_cast<unsigned short>( check_cast<unsigned long>( s ) );
        }

    template<>
        inline unsigned int check_cast<unsigned int, char *>( char *s ){
            return check_cast<unsigned int>( check_cast<unsigned long>( s ) );
        }


    template<>
        inline unsigned long long check_cast<unsigned long long, char *>( char *s ){
            return check_cast<unsigned long long, const char *>(s);
        }

    template<>
        inline float check_cast<float, char *>( char *s ){
            return check_cast<float, const char *>(s);
        }

    template<>
        inline double check_cast<double, char *>( char *s ){
            return check_cast<double, const char *>(s);
        }
    
    template<>
        inline long double check_cast<long double, char *>( char *s ){
            return check_cast<long double, const char *>(s);
        }

}

#endif  //__CHECK_CAST_CSTRING_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
