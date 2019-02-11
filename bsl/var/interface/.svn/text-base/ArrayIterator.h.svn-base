/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: ArrayIterator.h,v 1.3 2009/06/15 06:29:05 chenxm Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file ArrayIterator.h
 * @author chenxm(chenxiaoming@baidu.com)
 * @date 2008/09/25 11:00:45
 * @version $Revision: 1.3 $ 
 * @brief 
 *  
 **/
#ifndef  __BSL_VAR_ARRAY_ITERATOR_H__
#define  __BSL_VAR_ARRAY_ITERATOR_H__
namespace bsl{
namespace var{
    //forward declaration
    class IVar;
    class IArrayConstIteratorImpl;
    class IArrayIteratorImpl;
    class ArrayConstIterator;
    class ArrayIterator;

    //interface definition
    class IArrayIteratorImpl{
    public:
        typedef IArrayIteratorImpl        implement_type;
        typedef IArrayConstIteratorImpl   const_implement_type;

        IArrayIteratorImpl(){}
        IArrayIteratorImpl( const IArrayIteratorImpl& ) {}

        virtual ~IArrayIteratorImpl() {}
        virtual size_t  key() const = 0;
        virtual IVar& value() const = 0;

        virtual void    assign( const implement_type& ) = 0;
        virtual bool    equal_to( const implement_type& ) const = 0;
        virtual bool    equal_to( const const_implement_type& ) const = 0;
        virtual void    iterate() = 0;
    };

    class IArrayConstIteratorImpl{
    public:
        typedef IArrayIteratorImpl        implement_type;
        typedef IArrayConstIteratorImpl   const_implement_type;

        IArrayConstIteratorImpl(){}
        IArrayConstIteratorImpl( const IArrayConstIteratorImpl& ){}

        virtual ~IArrayConstIteratorImpl() {}
        virtual size_t  key() const = 0;
        virtual const IVar& value() const = 0;

        virtual void    assign( const implement_type& ) = 0;
        virtual void    assign( const const_implement_type& ) = 0;
        virtual bool    equal_to( const implement_type& ) const = 0;
        virtual bool    equal_to( const const_implement_type& ) const = 0;
        virtual void    iterate() = 0;
    };

    //iterator defintion
    class ArrayIterator{
    public:
        typedef ArrayIterator            iterator_type;
        typedef ArrayConstIterator       const_iterator_type;
        typedef IArrayIteratorImpl        implement_type;
        typedef IArrayConstIteratorImpl   const_implement_type;
        typedef implement_type *(* cloner_type )( const implement_type * );
        typedef void (* destroyer_type )( implement_type * ); 

        ArrayIterator( implement_type * p_impl, cloner_type cloner, destroyer_type destroyer )
            :_p_impl(p_impl), _cloner(cloner), _destroyer(destroyer) {}

        ArrayIterator( const ArrayIterator& other )
            :_p_impl( other._cloner(other._p_impl) ), _cloner(other._cloner), _destroyer(other._destroyer) { }

        ~ArrayIterator(){
            _destroyer( _p_impl );
        }

        ArrayIterator& operator = ( const ArrayIterator& other ){
            _p_impl->assign( *other._p_impl );            
            return *this;
        }

        const implement_type* operator ->() const {
            return _p_impl;
        }

        const implement_type& operator *() const {
            return *_p_impl;
        }

        ArrayIterator& operator ++(){
            _p_impl->iterate();
            return *this;
        }

    private:
        implement_type *_p_impl;
        cloner_type     _cloner;
        destroyer_type  _destroyer;
    };

    class ArrayConstIterator{
    public:
        typedef ArrayIterator            iterator_type;
        typedef ArrayConstIterator       const_iterator_type;
        typedef IArrayIteratorImpl        implement_type;
        typedef IArrayConstIteratorImpl   const_implement_type;
        typedef const_implement_type *(* cloner_type )( const const_implement_type * ); 
        typedef void (* destroyer_type )( const_implement_type * ); 

        ArrayConstIterator( const_implement_type * p_impl, cloner_type cloner, destroyer_type destroyer )
            :_p_impl(p_impl), _cloner(cloner), _destroyer(destroyer) {}

        ArrayConstIterator( const ArrayConstIterator& other )
            :_p_impl( other._cloner(other._p_impl) ), _cloner(other._cloner), _destroyer(other._destroyer) { }

        ~ArrayConstIterator(){
            _destroyer( _p_impl );
        }

        ArrayConstIterator& operator = ( const ArrayConstIterator& other ){
            _p_impl->assign( *other._p_impl );            
            return *this;
        }

        const const_implement_type* operator ->() const {
            return _p_impl;
        }

        const const_implement_type& operator *() const {
            return *_p_impl;
        }

        ArrayConstIterator& operator ++(){
            _p_impl->iterate();
            return *this;
        }

    private:
        const_implement_type *_p_impl;
        cloner_type     _cloner;
        destroyer_type  _destroyer;
    };

    inline bool operator == ( const ArrayConstIterator& i, const ArrayConstIterator & j ) {
        return i->equal_to(*j);
    }

    inline bool operator == ( const ArrayConstIterator& i, const ArrayIterator & j ) {
        return i->equal_to(*j);
    }

    inline bool operator == ( const ArrayIterator& i, const ArrayConstIterator & j ) {
        return i->equal_to(*j);
    }

    inline bool operator == ( const ArrayIterator& i, const ArrayIterator & j ) {
        return i->equal_to(*j);
    }

    inline bool operator != ( const ArrayConstIterator& i, const ArrayConstIterator & j ) {
        return !i->equal_to(*j);
    }

    inline bool operator != ( const ArrayConstIterator& i, const ArrayIterator & j ) {
        return !i->equal_to(*j);
    }

    inline bool operator != ( const ArrayIterator& i, const ArrayConstIterator & j ) {
        return !i->equal_to(*j);
    }

    inline bool operator != ( const ArrayIterator& i, const ArrayIterator & j ) {
        return !i->equal_to(*j);
    }


}}   //namespace var


#endif  //__BSL_VAR_ARRAY_ITERATOR_H__

/* vim: set ts=4 sw=4 sts=4 tw=100 */
