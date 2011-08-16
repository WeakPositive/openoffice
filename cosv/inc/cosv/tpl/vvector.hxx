/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef CSV_VVECTOR_HXX
#define CSV_VVECTOR_HXX

// USED SERVICES
#include <vector>
#include <cosv/tpl/tpltools.hxx>




namespace csv
{
namespace vvector
{


template <class TYPE>
struct delete_ptrs
{
    static void         Destruct(
                            std::vector< TYPE* > &
                                                v)
                        { csv::erase_container_of_heap_ptrs(v); }

    /// @precond ->it is a valid iterator within v
    static void         Erase(
                            std::vector< TYPE* > &
                                                v,
                            typename std::vector< TYPE* >::iterator
                                                it2erase )
                        { delete *it2erase; v.erase(it2erase); }

    /// @precond ->v.size() > 0
    static void         PopBack(
                            std::vector< TYPE* > &
                                                v )
                        { delete v.back(); v.pop_back(); }

    /// @precond ->it is a valid iterator
    static void         ReplacePtr(
                            typename std::vector< TYPE* >::iterator
                                                it,
                            DYN TYPE *          pass_new )
                        { delete *it; *it = pass_new; }
};


/** One helper class for the ->csv::VirtualVector.
    Implements a
*/
template <class TYPE>
struct keep_ptrs
{
    static void         Destruct(std::vector< TYPE* > & v)
                        {}

    static void         Erase(
                            std::vector< TYPE* > &
                                                v,
                            typename std::vector< TYPE* >::iterator
                                                it2erase )
                        { v.erase(it2erase); }

    static void         PopBack(
                            std::vector< TYPE* > &
                                                v )
                        { v.pop_back(); }

    /// @precond ->it is a valid iterator
    static void         ReplacePtr(
                            typename std::vector< TYPE* >::iterator
                                                it,
                            TYPE *              io_new )
                        { *it = io_new; }
};


}   // namespace vvector




/** Implements a vector of different implementations of a base
    class.

    Implementation has to be by pointers to get the polymorphic
    behaviour, however access is by references to the base class.

    @tpl XX
    The common base class of vector elements.

    @tpl PTRDEL
    Has two possible values:
        vvector::delete_ptrs<XX>    Elements have to be on the heap and
                                    are deleted when removed (default).
        vvector::keep_ptrs<XX>      Elements are only referenced and not
                                    deleted when removed.

*/
template <class XX, class PTRDEL = vvector::delete_ptrs<XX> >
class VirtualVector
{
  public:
	typedef VirtualVector<XX,PTRDEL>            self;
	typedef std::vector< DYN XX* >              impl_type;
	typedef typename impl_type::size_type       size_type;
    typedef ptrdiff_t                           difference_type;

    class const_iterator;
    class iterator;

    // LIFECYCLE
						VirtualVector();
    explicit    		VirtualVector(
						    int                 i_size );
						~VirtualVector();

    // OPERATORS
    const XX &          operator[](
                            size_type           i_pos ) const;
    XX &                operator[](
                            size_type           i_pos );

    // OPERATIONS
    void                push_back(
                            DYN XX &            i_drElement );
    void                pop_back();

    iterator            insert(
                            iterator            i_pos,
                            DYN XX &            i_drElement );
    void                erase(
                            iterator            i_pos );
    void                replace(
                            iterator            i_pos,
                            DYN XX &            i_drElement );
    void                reserve(
                            size_type           i_size );

    // INQUIRY
    bool                empty() const;
    size_t              size() const;
	const_iterator      begin() const;
	const_iterator      end() const;
    const XX &          front() const;
    const XX &          back() const;

    // ACCESS
	iterator            begin();
	iterator            end();
    XX &                front();
    XX &                back();

  private:
    // Forbidden:
                        VirtualVector(const VirtualVector&);
    VirtualVector &     operator=(const VirtualVector&);

    // DATA
    std::vector< DYN XX* >
                        aData;
};




/** Should be usable for all STL algorithms.
    Implements the Random Access Iterator concept.
*/
template <class XX, class PTRDEL>
class VirtualVector<XX,PTRDEL>::
      const_iterator

            // This derivation provides type information for the STL
            // It introduces the types "value_type" and "difference_type".
            : public std::iterator<std::random_access_iterator_tag,
                        const XX>
{
  public:
    typedef VirtualVector<XX,PTRDEL>                            my_container;
    typedef typename my_container::impl_type::const_iterator    impl_iterator;

    // LIFECYCLE
                        const_iterator(
                            impl_iterator       i_implIter )
                            : itImpl(i_implIter) {}


    ///////////      STL ITERATOR CONCEPT IMPLEMENTATION        //////////////

    // Default Constructible functions:
                        const_iterator()
                            : itImpl() {}

    // Assignable functions:
        // Assignment and copy constructor use the compiler generated versions.

    // Equality Comparable functions:
    bool                operator==(
                            const_iterator      i_other ) const
                            { return itImpl == i_other.itImpl; }
    bool                operator!=(
                            const_iterator      i_other ) const
                            { return itImpl != i_other.itImpl; }

    // Trivial Iterator functions:
    const XX &          operator*() const
                            { return *(*itImpl); }

    // Input Iterator functions:
    const_iterator &    operator++()
                            { ++itImpl; return *this; }
    const_iterator      operator++(int)
                            { return const_iterator(itImpl++); }

    // Bidirectional Iterator functions:
    const_iterator &    operator--()
                            { --itImpl; return *this; }
    const_iterator      operator--(int)
                            { return const_iterator(itImpl--); }

    // Less Than Comparable functions:
    bool                operator<(
                            const_iterator      i_other ) const
                            { return itImpl < i_other.itImpl; }

    // Random Access Iterator functions:
    const_iterator &    operator+=(
                            difference_type     i_diff )
                            { itImpl += i_diff; return *this; }
    const_iterator      operator+(
                            difference_type     i_diff ) const
                            { const_iterator ret(itImpl);
                              return ret += i_diff; }
    const_iterator &    operator-=(
                            difference_type     i_diff )
                            { itImpl -= i_diff;  return *this; }
    const_iterator      operator-(
                            difference_type     i_diff ) const
                            { const_iterator ret(itImpl);
                              return ret -= i_diff; }
    difference_type     operator-(
                            const_iterator      i_it ) const
                            { return itImpl - i_it.itImpl; }
    const XX &          operator[](
                            difference_type     i_index )
                            { return *(*itImpl[i_index]); }

    //////////////////////////////////////////////////////////////////////////

  private:
    friend class VirtualVector<XX,PTRDEL>;
    impl_iterator       ImplValue() const       { return itImpl; }

    // DATA
    impl_iterator       itImpl;
};




/** Should be usable for all STL algorithms.
    Implements the Random Access Iterator concept.
*/
template <class XX, class PTRDEL>
class VirtualVector<XX,PTRDEL>::
      iterator

            // This derivation provides type information for the STL
            // It introduces the types "value_type" and "difference_type".
            : public std::iterator<std::random_access_iterator_tag,
                        XX>
{
  public:
    typedef VirtualVector<XX,PTRDEL>                            my_container;
    typedef typename my_container::impl_type::iterator          impl_iterator;

    // LIFECYCLE
                        iterator(
                            impl_iterator       i_implIter )
                            : itImpl(i_implIter) {}


    ///////////      STL ITERATOR CONCEPT IMPLEMENTATION        //////////////

    // Default Constructible functions:
                        iterator()
                            : itImpl() {}

    // Assignable functions:
        // Assignment and copy constructor use the compiler generated versions.

    // Equality Comparable functions:
    bool                operator==(
                            iterator            i_other ) const
                            { return itImpl == i_other.itImpl; }
    bool                operator!=(
                            iterator            i_other ) const
                            { return itImpl != i_other.itImpl; }

    // Trivial Iterator functions:
    XX &                operator*() const
                            { return *(*itImpl); }

    // Input Iterator functions:
    iterator &          operator++()
                            { ++itImpl; return *this; }
    iterator            operator++(int)
                            { return iterator(itImpl++); }

    // Bidirectional Iterator functions:
    iterator &          operator--()
                            { --itImpl; return *this; }
    iterator            operator--(int)
                            { return iterator(itImpl--); }

    // Less Than Comparable functions:
    bool                operator<(
                            iterator            i_other ) const
                            { return itImpl < i_other.itImpl; }

    // Random Access Iterator functions:
    iterator &          operator+=(
                            difference_type     i_diff )
                            { itImpl += i_diff; return *this;  }
    iterator            operator+(
                            difference_type     i_diff ) const
                            { iterator ret(itImpl);
                              return ret += i_diff; }
    iterator &          operator-=(
                            difference_type     i_diff )
                            { itImpl -= i_diff; return *this;  }
    iterator            operator-(
                            difference_type     i_diff ) const
                            { iterator ret(itImpl);
                              return ret -= i_diff; }
    difference_type     operator-(
                            iterator            i_it ) const
                            { return itImpl - i_it.itImpl; }
    XX &                operator[](
                            difference_type     i_index )
                            { return *(*itImpl[i_index]); }

    //////////////////////////////////////////////////////////////////////////

  private:
    friend class VirtualVector<XX,PTRDEL>;
    impl_iterator       ImplValue() const       { return itImpl; }

    // DATA
    impl_iterator       itImpl;
};




// IMPLEMENTATION
template <class XX, class PTRDEL>
inline
VirtualVector<XX,PTRDEL>::VirtualVector()
    :   aData()
{
}

template <class XX, class PTRDEL>
inline
VirtualVector<XX,PTRDEL>::VirtualVector(int i_size)
    :   aData(i_size, 0)
{
}

template <class XX, class PTRDEL>
inline
VirtualVector<XX,PTRDEL>::~VirtualVector()
{
    PTRDEL::Destruct(aData);
}

template <class XX, class PTRDEL>
inline const XX &
VirtualVector<XX,PTRDEL>::operator[]( size_type i_pos ) const
{
    return *aData[i_pos];
}

template <class XX, class PTRDEL>
inline XX &
VirtualVector<XX,PTRDEL>::operator[]( size_type i_pos )
{
    return *aData[i_pos];
}

template <class XX, class PTRDEL>
inline void
VirtualVector<XX,PTRDEL>::push_back( DYN XX & i_drElement )
{
    aData.push_back(&i_drElement);
}

template <class XX, class PTRDEL>
inline void
VirtualVector<XX,PTRDEL>::pop_back()
{
    if (NOT aData.empty())
        PTRDEL::PopBack(aData);
}

template <class XX, class PTRDEL>
inline typename VirtualVector<XX,PTRDEL>::iterator
VirtualVector<XX,PTRDEL>::insert( iterator            i_pos,
                                  DYN XX &            i_drElement )
{
    return iterator(aData.insert(i_pos.ImplValue(), &i_drElement));
}

template <class XX, class PTRDEL>
inline void
VirtualVector<XX,PTRDEL>::erase( iterator i_pos )
{
    PTRDEL::Erase(aData, i_pos.ImplValue());
}

template <class XX, class PTRDEL>
inline void
VirtualVector<XX,PTRDEL>::replace( iterator     i_pos,
                                   DYN XX &     i_drElement )
{
    PTRDEL::ReplacePtr(*i_pos, &i_drElement);
}

template <class XX, class PTRDEL>
inline void
VirtualVector<XX,PTRDEL>::reserve( size_type i_size )
{
    aData.reserve(i_size);
}

template <class XX, class PTRDEL>
inline bool
VirtualVector<XX,PTRDEL>::empty() const
{
    return aData.empty();
}

template <class XX, class PTRDEL>
inline size_t
VirtualVector<XX,PTRDEL>::size() const
{
    return aData.size();
}

template <class XX, class PTRDEL>
inline typename VirtualVector<XX,PTRDEL>::const_iterator
VirtualVector<XX,PTRDEL>::begin() const
{
    return const_iterator(aData.begin());
}

template <class XX, class PTRDEL>
inline typename VirtualVector<XX,PTRDEL>::const_iterator
VirtualVector<XX,PTRDEL>::end() const
{
    return const_iterator(aData.end());
}

template <class XX, class PTRDEL>
inline const XX &
VirtualVector<XX,PTRDEL>::front() const
{
    return *aData.front();
}

template <class XX, class PTRDEL>
inline const XX &
VirtualVector<XX,PTRDEL>::back() const
{
    return *aData.back();
}

template <class XX, class PTRDEL>
inline typename VirtualVector<XX,PTRDEL>::iterator
VirtualVector<XX,PTRDEL>::begin()
{
    return iterator(aData.begin());
}

template <class XX, class PTRDEL>
inline typename VirtualVector<XX,PTRDEL>::iterator
VirtualVector<XX,PTRDEL>::end()
{
    return iterator(aData.end());
}

template <class XX, class PTRDEL>
inline XX &
VirtualVector<XX,PTRDEL>::front()
{
    return *aData.front();
}

template <class XX, class PTRDEL>
inline XX &
VirtualVector<XX,PTRDEL>::back()
{
    return *aData.back();
}




}   // namespace csv
#endif
