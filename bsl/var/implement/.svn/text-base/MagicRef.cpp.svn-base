/***************************************************************************
 * 
 * Copyright (c) 2009 Baidu.com, Inc. All Rights Reserved
 * $Id: MagicRef.cpp,v 1.2 2010/04/28 12:45:33 scmpf Exp $ 
 * 
 **************************************************************************/



/**
 * @file MagicRef.cpp
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2009/12/15 00:19:01
 * @version $Revision: 1.2 $ 
 * @brief 
 *  
 **/
#include "MagicRef.h"
#include "implement.h"
#include <bsl/var/Number.h>

namespace bsl{ namespace var {
    MagicRef::string_type MagicRef::to_string() const {
        if ( _p ){
            return _p->to_string();
        }else{
            return bsl::var::Null::null.to_string();
        }
    }

    IVar& MagicRef::ref() const {
        if ( _p ){
            return *_p;
        }else{
            return bsl::var::Null::null;
        }
    }

    MagicRef& MagicRef::operator = ( bool val ){
        if ( _p ){
            _p->operator = ( val );
        }else{
            _p = &_rp.create<bsl::var::Bool>(val);
        }
        return *this;
    }

    MagicRef& MagicRef::operator = ( signed char val ){
        if ( _p ){
            _p->operator = ( val );
        }else{
            _p = &_rp.create<bsl::var::Number<signed char> >(val);
        }
        return *this;
    }

    MagicRef& MagicRef::operator = ( unsigned char val ){
        if ( _p ){
            _p->operator = ( val );
        }else{
            _p = &_rp.create<bsl::var::Number<unsigned char> >(val);
        }
        return *this;
    }

    MagicRef& MagicRef::operator = ( signed short val ){
        if ( _p ){
            _p->operator = ( val );
        }else{
            _p = &_rp.create<bsl::var::Number<signed short> >(val);
        }
        return *this;
    }

    MagicRef& MagicRef::operator = ( unsigned short val ){
        if ( _p ){
            _p->operator = ( val );
        }else{
            _p = &_rp.create<bsl::var::Number<unsigned short> >(val);
        }
        return *this;
    }

    MagicRef& MagicRef::operator = ( signed int val ){
        if ( _p ){
            _p->operator = ( val );
        }else{
            _p = &_rp.create<bsl::var::Number<signed int> >(val);
        }
        return *this;
    }

    MagicRef& MagicRef::operator = ( unsigned int val ){
        if ( _p ){
            _p->operator = ( val );
        }else{
            _p = &_rp.create<bsl::var::Number<unsigned int> >(val);
        }
        return *this;
    }

    MagicRef& MagicRef::operator = ( signed long long val ){
        if ( _p ){
            _p->operator = ( val );
        }else{
            _p = &_rp.create<bsl::var::Number<signed long long> >(val);
        }
        return *this;
    }

    MagicRef& MagicRef::operator = ( unsigned long long val ){
        if ( _p ){
            _p->operator = ( val );
        }else{
            _p = &_rp.create<bsl::var::Number<unsigned long long> >(val);
        }
        return *this;
    }

    MagicRef& MagicRef::operator = ( float val ){
        if ( _p ){
            _p->operator = ( val );
        }else{
            _p = &_rp.create<bsl::var::Number<float> >(val);
        }
        return *this;
    }

    MagicRef& MagicRef::operator = ( double val ){
        if ( _p ){
            _p->operator = ( val );
        }else{
            _p = &_rp.create<bsl::var::Number<double> >(val);
        }
        return *this;
    }

    MagicRef& MagicRef::operator = ( const char *val ){
        if ( _p ){
            _p->operator = ( val );
        }else{
            _p = &_rp.create<bsl::var::String>(val);
        }
        return *this;
    }

    MagicRef& MagicRef::operator = ( const string_type& val ){
        if ( _p ){
            _p->operator = ( val );
        }else{
            _p = &_rp.create<bsl::var::String>(val);
        }
        return *this;
    }

    MagicRef& MagicRef::operator = ( const raw_type& val ){
        if ( _p ){
            _p->operator = ( val );
        }else{
            _p = &_rp.create<bsl::var::ShallowRaw>(val);
        }
        return *this;
    }
    void MagicRef::set( size_t index, IVar& value ){
        if ( !_p ){
            _p = &_rp.createn<bsl::var::MagicArray>(const_cast<bsl::ResourcePool&>(_rp));
        }
        return _p->set( index, value );
    }

    bool MagicRef::del( size_t index ){
        if ( !_p ){
            _p = &_rp.createn<bsl::var::MagicArray>(_rp);
        }
        return _p->del(index);
    }

    IVar& MagicRef::operator []( int index ){
        if ( !_p ){
            _p = &_rp.createn<bsl::var::MagicArray>(_rp);
        }
        return _p->operator [](index); 
    }

    void MagicRef::set( const field_type& name, IVar& value ){
        if ( !_p ){
            _p = &_rp.createn<bsl::var::MagicDict>(_rp);
        }
        return _p->set(name, value);
    }

    bool MagicRef::del( const field_type& name ){
        if ( !_p ){
            _p = &_rp.createn<bsl::var::MagicDict>(_rp);
        }
        return _p->del(name);
    }

    IVar& MagicRef::operator []( const field_type& name ){
        if ( !_p ){
            _p = &_rp.createn<bsl::var::MagicDict>(_rp);
        }
        return _p->operator []( name );
    }
}}//end of namespace

/* vim: set ts=4 sw=4 sts=4 tw=100 */
