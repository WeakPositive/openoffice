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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_comphelper.hxx"

#include <comphelper/numberedcollection.hxx>

//_______________________________________________
// includes

#include <com/sun/star/frame/UntitledNumbersConst.hpp>

//_______________________________________________
// namespace

namespace comphelper{

namespace css = ::com::sun::star;

//_______________________________________________
// definitions

static const ::rtl::OUString ERRMSG_INVALID_COMPONENT_PARAM = ::rtl::OUString::createFromAscii("NULL as component reference not allowed.");
static const ::rtl::OUString ERRMSG_INVALID_NUMBER_PARAM    = ::rtl::OUString::createFromAscii("Special valkud INVALID_NUMBER not allowed as input parameter.");

//-----------------------------------------------
NumberedCollection::NumberedCollection()
    : ::cppu::BaseMutex ()
    , m_sUntitledPrefix ()
    , m_lComponents     ()
    , m_xOwner          ()
{
}

//-----------------------------------------------
NumberedCollection::~NumberedCollection()
{
}

//-----------------------------------------------
void NumberedCollection::setOwner(const css::uno::Reference< css::uno::XInterface >& xOwner)
{
    // SYNCHRONIZED ->
    ::osl::ResettableMutexGuard aLock(m_aMutex);

        m_xOwner = xOwner;

    // <- SYNCHRONIZED
}

//-----------------------------------------------
void NumberedCollection::setUntitledPrefix(const ::rtl::OUString& sPrefix)
{
    // SYNCHRONIZED ->
    ::osl::ResettableMutexGuard aLock(m_aMutex);

        m_sUntitledPrefix = sPrefix;

    // <- SYNCHRONIZED
}

//-----------------------------------------------
::sal_Int32 SAL_CALL NumberedCollection::leaseNumber(const css::uno::Reference< css::uno::XInterface >& xComponent)
    throw (css::lang::IllegalArgumentException,
           css::uno::RuntimeException         )
{
    // SYNCHRONIZED ->
    ::osl::ResettableMutexGuard aLock(m_aMutex);
    
        if ( ! xComponent.is ())
            throw css::lang::IllegalArgumentException (ERRMSG_INVALID_COMPONENT_PARAM, m_xOwner.get(), 1);

        long                              pComponent = (long) xComponent.get ();
        TNumberedItemHash::const_iterator pIt        = m_lComponents.find (pComponent);
        
        // a) component already exists - return it's number directly
        if (pIt != m_lComponents.end())
            return pIt->second.nNumber;
        
        // b) component must be added new to this container
        
        // b1) collection is full - no further components possible
        //     -> return INVALID_NUMBER
        ::sal_Int32 nFreeNumber = impl_searchFreeNumber();
        if (nFreeNumber == css::frame::UntitledNumbersConst::INVALID_NUMBER)
            return css::frame::UntitledNumbersConst::INVALID_NUMBER;
        
        // b2) add component to collection and return its number
        TNumberedItem aItem;
        aItem.xItem   = css::uno::WeakReference< css::uno::XInterface >(xComponent);
        aItem.nNumber = nFreeNumber;
        m_lComponents[pComponent] = aItem;
        
        return nFreeNumber;
    
    // <- SYNCHRONIZED
}
           
//-----------------------------------------------
void SAL_CALL NumberedCollection::releaseNumber(::sal_Int32 nNumber)
    throw (css::lang::IllegalArgumentException,
           css::uno::RuntimeException         )
{
    // SYNCHRONIZED ->
    ::osl::ResettableMutexGuard aLock(m_aMutex);
    
        if (nNumber == css::frame::UntitledNumbersConst::INVALID_NUMBER)
            throw css::lang::IllegalArgumentException (ERRMSG_INVALID_NUMBER_PARAM, m_xOwner.get(), 1);
        
        TDeadItemList               lDeadItems;
        TNumberedItemHash::iterator pComponent;
        
        for (  pComponent  = m_lComponents.begin ();
               pComponent != m_lComponents.end   ();
             ++pComponent                          )
        {
            const TNumberedItem&                              rItem = pComponent->second;
            const css::uno::Reference< css::uno::XInterface > xItem = rItem.xItem.get();
        
            if ( ! xItem.is ())
            {
                lDeadItems.push_back(pComponent->first);
                continue;
            }
        
            if (rItem.nNumber == nNumber)
            {
                m_lComponents.erase (pComponent);
                break;
            }
        }
        
        impl_cleanUpDeadItems(m_lComponents, lDeadItems);
    
    // <- SYNCHRONIZED
}
           
//-----------------------------------------------
void SAL_CALL NumberedCollection::releaseNumberForComponent(const css::uno::Reference< css::uno::XInterface >& xComponent)
    throw (css::lang::IllegalArgumentException,
           css::uno::RuntimeException         )
{
    // SYNCHRONIZED ->
    ::osl::ResettableMutexGuard aLock(m_aMutex);
    
        if ( ! xComponent.is ())
            throw css::lang::IllegalArgumentException (ERRMSG_INVALID_COMPONENT_PARAM, m_xOwner.get(), 1);
    
        long                        pComponent = (long) xComponent.get ();
        TNumberedItemHash::iterator pIt        = m_lComponents.find (pComponent);
    
        // a) component exists and will be removed
        if (pIt != m_lComponents.end())
            m_lComponents.erase(pIt);
        
        // else
        // b) component does not exists - nothing todo here (ignore request!)
    
    // <- SYNCHRONIZED
}

//-----------------------------------------------
::rtl::OUString SAL_CALL NumberedCollection::getUntitledPrefix()
    throw (css::uno::RuntimeException)
{
    // SYNCHRONIZED ->
    ::osl::ResettableMutexGuard aLock(m_aMutex);

        return m_sUntitledPrefix;

    // <- SYNCHRONIZED
}

//-----------------------------------------------
/** create an ordered list of all possible numbers ...
    e.g. {1,2,3,...,N} Max size of these list will be
    current size of component list + 1 .

    "+1" ... because in case all numbers in range 1..n
    are in use we need a new number n+1 :-)

    Every item which is already used as unique number
    will be removed. At the end a list of e.g. {3,6,...,M}
    exists where the first item represent the lowest free
    number (in this example 3).
 */
::sal_Int32 NumberedCollection::impl_searchFreeNumber ()
{
    // create ordered list of all possible numbers.
    ::std::vector< ::sal_Int32 > lPossibleNumbers;
    ::sal_Int32                  c = (::sal_Int32)m_lComponents.size ();
    ::sal_Int32                  i = 1;

    // c cant be less then 0 ... otherwhise hash.size() has an error :-)
    // But we need at least n+1 numbers here.
	c += 1;

    for (i=1; i<=c; ++i)
        lPossibleNumbers.push_back (i);

    // SYNCHRONIZED ->
    ::osl::ResettableMutexGuard aLock(m_aMutex);
    
        TDeadItemList                     lDeadItems;
        TNumberedItemHash::const_iterator pComponent;
    
        for (  pComponent  = m_lComponents.begin ();
               pComponent != m_lComponents.end   ();
             ++pComponent                          )
        {
            const TNumberedItem&                              rItem = pComponent->second;
            const css::uno::Reference< css::uno::XInterface > xItem = rItem.xItem.get();
        
            if ( ! xItem.is ())
            {
                lDeadItems.push_back(pComponent->first);
                continue;
            }
        
            ::std::vector< ::sal_Int32 >::iterator pPossible = ::std::find(lPossibleNumbers.begin (), lPossibleNumbers.end (), rItem.nNumber);
            if (pPossible != lPossibleNumbers.end ())
                lPossibleNumbers.erase (pPossible);
        }
    
        impl_cleanUpDeadItems(m_lComponents, lDeadItems);
    
        // a) non free numbers ... return INVALID_NUMBER
        if (lPossibleNumbers.size () < 1)
            return css::frame::UntitledNumbersConst::INVALID_NUMBER;
        
        // b) return first free number
        return *(lPossibleNumbers.begin ());
        
    // <- SYNCHRONIZED
}

void NumberedCollection::impl_cleanUpDeadItems (      TNumberedItemHash& lItems    ,
                                                const TDeadItemList&     lDeadItems)
{
    TDeadItemList::const_iterator pIt;

    for (  pIt  = lDeadItems.begin ();
           pIt != lDeadItems.end   ();
         ++pIt                       )
    {
        const long& rDeadItem = *pIt;
        lItems.erase(rDeadItem);
    }
}

} // namespace comphelper
