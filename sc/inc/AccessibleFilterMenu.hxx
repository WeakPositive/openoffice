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

#ifndef SC_ACCESSIBLEFILTERMENU_HXX
#define SC_ACCESSIBLEFILTERMENU_HXX

#include "AccessibleContextBase.hxx"
#include "cppuhelper/implbase1.hxx"

#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/accessibility/XAccessibleText.hpp>
#include <com/sun/star/accessibility/XAccessibleTextAttributes.hpp>
#include <com/sun/star/accessibility/TextSegment.hpp>

#include <vector>

namespace com { namespace sun { namespace star {
    namespace accessibility {
        struct AccessibleEventObject;
    }
}}}

class ScDocument;
class ScMenuFloatingWindow;

typedef ::cppu::ImplHelper1<
        ::com::sun::star::accessibility::XAccessibleSelection > ScAccessibleFilterMenu_BASE;

class ScAccessibleFilterMenu :
    public ScAccessibleContextBase,
    public ScAccessibleFilterMenu_BASE
{
public:
    explicit ScAccessibleFilterMenu(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible>& rxParent,
            ScMenuFloatingWindow* pWin, const ::rtl::OUString& rName, size_t nMenuPos, ScDocument* pDoc);
    virtual ~ScAccessibleFilterMenu();

	// XAccessibleComponent

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
		SAL_CALL getAccessibleAtPoint( const ::com::sun::star::awt::Point& rPoint )
		    throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL isVisible()
		throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL grabFocus()
		throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getForeground()
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getBackground()
        throw (::com::sun::star::uno::RuntimeException);

	// XAccessibleContext

	virtual ::rtl::OUString SAL_CALL getAccessibleName()
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getAccessibleChildCount()
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL
    	getAccessibleChild(sal_Int32 nIndex)
            throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IndexOutOfBoundsException);

	virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleStateSet> SAL_CALL
    	getAccessibleStateSet()
            throw (::com::sun::star::uno::RuntimeException);

	virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw (::com::sun::star::uno::RuntimeException);

    // XAccessibleEventBroadcaster

    using ScAccessibleContextBase::addEventListener;
    using ScAccessibleContextBase::removeEventListener;

	virtual void SAL_CALL
    	addEventListener(
        	const ::com::sun::star::uno::Reference<
                ::com::sun::star::accessibility::XAccessibleEventListener>& xListener)
        throw (com::sun::star::uno::RuntimeException);

    //	Remove an existing event listener.
	virtual void SAL_CALL
    	removeEventListener(
			const ::com::sun::star::uno::Reference<
                ::com::sun::star::accessibility::XAccessibleEventListener>& xListener)
        throw (com::sun::star::uno::RuntimeException);

    // XAccessibleSelection

    virtual void SAL_CALL selectAccessibleChild(sal_Int32 nChildIndex)
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL isAccessibleChildSelected(sal_Int32 nChildIndex)
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL clearAccessibleSelection()
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL selectAllAccessibleChildren()
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::sal_Int32 SAL_CALL getSelectedAccessibleChildCount()
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL
        getSelectedAccessibleChild(sal_Int32 nChildIndex)
            throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL deselectAccessibleChild(sal_Int32 nChildIndex)
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    // XInterface

	virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
		::com::sun::star::uno::Type const & rType )
		    throw (::com::sun::star::uno::RuntimeException);

	virtual void SAL_CALL acquire() throw ();
	virtual void SAL_CALL release() throw ();

    // XTypeProvider

    virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL getImplementationId()
        throw (::com::sun::star::uno::RuntimeException);

    // non-UNO methods

    void appendMenuItem(const ::rtl::OUString& rName, bool bEnabled, size_t nMenuPos);
    void setMenuPos(size_t nMenuPos);
    void setEnabled(bool bEnabled);

protected:

    sal_Int32 getMenuItemCount() const;

	virtual Rectangle GetBoundingBoxOnScreen() const
		throw (::com::sun::star::uno::RuntimeException);

	virtual Rectangle GetBoundingBox() const
		throw (::com::sun::star::uno::RuntimeException);

private:
    bool isSelected() const;
    bool isFocused() const;

    void updateStates();

private:
    ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > > maMenuItems;
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > mxStateSet;

    size_t mnMenuPos;
    ScMenuFloatingWindow* mpWindow;
    ScDocument* mpDoc;

    bool mbEnabled:1;
};

#endif
