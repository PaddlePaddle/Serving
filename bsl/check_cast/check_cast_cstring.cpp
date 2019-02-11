/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: check_cast_cstring.cpp,v 1.3 2009/10/14 08:24:58 chenxm Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file check_cast_cstring.cpp
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2008/11/12 20:44:10
 * @version $Revision: 1.3 $ 
 * @brief 
 *  
 **/

#include <cerrno>
#include "bsl/check_cast.h"

namespace bsl{
    template<>
        long check_cast<long, const char *> ( const char * s ){
            if ( !s ){
                throw bsl::NullPointerException()<<BSL_EARG<<"s["<<s<<"]"<<"s";
            }
            errno = 0;
            char *end_ptr;
            long res = strtol(s, &end_ptr, 0);  
            switch( errno ){
                case 0:
                    if ( end_ptr == s ){
                        //not a numerial string, or format not recongnized
                        throw bsl::BadCastException()<<BSL_EARG<<"s["<<s<<"]";
                    }
                    return res;
                case ERANGE:
                    if ( res < 0 ){
                        throw bsl::UnderflowException()<<BSL_EARG<<"s["<<s<<"]";
                    }else{
                        throw bsl::OverflowException()<<BSL_EARG<<"s["<<s<<"]";
                    }
                default:
                    throw bsl::BadCastException()<<BSL_EARG<<"s["<<s<<"]";

            }

        }

    template<>
        long long check_cast<long long, const char *>( const char * s ){
            if ( !s ){
                throw bsl::NullPointerException()<<BSL_EARG<<"s";
            }
            errno = 0;
            char *end_ptr;
            long long res = strtoll(s, &end_ptr, 0);  
            switch( errno ){
                case 0:
                    if ( end_ptr == s ){
                        //not a numerial string, or format not recongnized
                        throw bsl::BadCastException()<<BSL_EARG<<"s["<<s<<"]";
                    }
                    return res;
                case ERANGE:
                    if ( res < 0 ){
                        throw bsl::UnderflowException()<<BSL_EARG<<"s["<<s<<"]";
                    }else{
                        throw bsl::OverflowException()<<BSL_EARG<<"s["<<s<<"]";
                    }
                default:
                    throw bsl::BadCastException()<<BSL_EARG<<"s["<<s<<"]";

            }

        }

    template<>
        unsigned long check_cast<unsigned long, const char *>( const char * s ){
            if ( !s ){
                throw bsl::NullPointerException()<<BSL_EARG<<"s";
            }
            errno = 0;
            char *end_ptr;
            unsigned long res = strtoul(s, &end_ptr, 0);  

            if ( memchr( s, '-', end_ptr - s ) != NULL ){
                throw bsl::UnderflowException()<<BSL_EARG<<"s["<<s<<"]";    
            }

            switch( errno ){
                case 0:
                    if ( end_ptr == s ){
                        //not a numerial string, or format not recongnized
                        throw bsl::BadCastException()<<BSL_EARG<<"s["<<s<<"]";
                    }
                    return res;
                case ERANGE:
                    throw bsl::OverflowException()<<BSL_EARG<<"s["<<s<<"]";
                default:
                    throw bsl::BadCastException()<<BSL_EARG<<"s["<<s<<"]";

            }
        }

    template<>
        unsigned long long check_cast<unsigned long long, const char *>( const char * s ){
            if ( !s ){
                throw bsl::NullPointerException()<<BSL_EARG<<"s";
            }
            errno = 0;
            char *end_ptr;
            unsigned long long res = strtoull(s, &end_ptr, 0);  

            if ( memchr( s, '-', end_ptr - s ) != NULL ){
                throw bsl::UnderflowException()<<BSL_EARG<<"s["<<s<<"]";    
            }

            switch( errno ){
                case 0:
                    if ( end_ptr == s ){
                        //not a numerial string, or format not recongnized
                        throw bsl::BadCastException()<<BSL_EARG<<"not numerial string: s["<<s<<"]";
                    }
                    return res;
                case ERANGE:
                    throw bsl::OverflowException()<<BSL_EARG<<"s["<<s<<"]";
                default:
                    throw bsl::BadCastException()<<BSL_EARG<<"s["<<s<<"]";

            }
        }

    template<>
        float check_cast<float, const char *>( const char * s ){
            if ( !s ){
                throw bsl::NullPointerException()<<BSL_EARG<<"s";
            }
            errno = 0;
            char *end_ptr;
#if __GNUC__ <= 2
            float res = float(strtod(s, &end_ptr)); //pray you don't have a really huge number, amen
#else
            float res = strtof(s, &end_ptr);  
#endif
            switch( errno ){
                case 0:
                    if ( end_ptr == s ){
                        //not a numerial string, or format not recongnized
                        throw bsl::BadCastException()<<BSL_EARG<<"not numerial string: s["<<s<<"]";
                    }
                    return res;
                case ERANGE:
                    if ( res < 0 ){
                        throw bsl::UnderflowException()<<BSL_EARG<<"s["<<s<<"]";
                    }else{
                        throw bsl::OverflowException()<<BSL_EARG<<"s["<<s<<"]";
                    }

                default:
                    throw bsl::BadCastException()<<BSL_EARG<<"s["<<s<<"]";

            }
        }

    template<>
        double check_cast<double, const char *>( const char * s ){
            if ( !s ){
                throw bsl::NullPointerException()<<BSL_EARG<<"s";
            }
            errno = 0;
            char *end_ptr;
            double res = strtod(s, &end_ptr);  

            switch( errno ){
                case 0:
                    if ( end_ptr == s ){
                        //not a numerial string, or format not recongnized
                        throw bsl::BadCastException()<<BSL_EARG<<"not numerial string: s["<<s<<"]";
                    }
                    return res;
                case ERANGE:
                    if ( res < 0 ){
                        throw bsl::UnderflowException()<<BSL_EARG<<"s["<<s<<"]";
                    }else{
                        throw bsl::OverflowException()<<BSL_EARG<<"s["<<s<<"]";
                    }

                default:
                    throw bsl::BadCastException()<<BSL_EARG<<"s["<<s<<"]";

            }
        }

    template<>
        long double check_cast<long double, const char *>( const char * s ){
            if ( !s ){
                throw bsl::NullPointerException()<<BSL_EARG<<"s";
            }
            errno = 0;
            char *end_ptr;
#if __GNUC__ <= 2
            long double res = strtod(s, &end_ptr); //pray you don't have a really huge number, amen
#else
            long double res = strtold(s, &end_ptr);  
#endif

            switch( errno ){
                case 0:
                    if ( end_ptr == s ){
                        //not a numerial string, or format not recongnized
                        throw bsl::BadCastException()<<BSL_EARG<<"not numerial string: s["<<s<<"]";
                    }
                    return res;
                case ERANGE:
                    if ( res < 0 ){
                        throw bsl::UnderflowException()<<BSL_EARG<<"s["<<s<<"]";
                    }else{
                        throw bsl::OverflowException()<<BSL_EARG<<"s["<<s<<"]";
                    }

                default:
                    throw bsl::BadCastException()<<BSL_EARG<<"s["<<s<<"]";

            }
        }

}   //namespace bsl
/* vim: set ts=4 sw=4 sts=4 tw=100 */
