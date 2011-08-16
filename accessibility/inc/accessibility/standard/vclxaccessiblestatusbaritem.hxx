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

#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLESTATUSBARITEM_HXX
#define ACCESSIBILITY_STANDARD_VCLXACCESSIBLESTATUSBARITEM_HXX

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <comphelper/accessibletexthelper.hxx>
#ifndef _CPPUHELPER_IMPLBASE2_HXX
#include <cppuhelper/implbase2.hxx>
#endif


class StatusBar;
class VCLExternalSolarLock;

namespace utl {
class AccessibleStateSetHelper;
}


//	----------------------------------------------------
//	class VCLXAccessibleStatusBarItem
//	----------------------------------------------------

typedef ::comphelper::OAccessibleTextHelper	AccessibleTextHelper_BASE;

typedef ::cppu::ImplHelper2<
	::com::sun::star::accessibility::XAccessible,
	::com::sun::star::lang::XServiceInfo > VCLXAccessibleStatusBarItem_BASE;

class VCLXAccessibleStatusBarItem :	public AccessibleTextHelper_BASE,
									public VCLXAccessibleStatusBarItem_BASE
{
	friend class VCLXAccessibleStatusBar;

private:
	VCLExternalSolarLock*	m_pExternalLock;
	StatusBar*				m_pStatusBar;
	sal_uInt16				m_nItemId;
	::rtl::OUString			m_sItemName;
	::rtl::OUString			m_sItemText;
	sal_Bool				m_bShowing;

protected:
	sal_Bool				IsShowing();
	void					SetShowing( sal_Bool bShowing );
	void					SetItemName( const ::rtl::OUString& sItemName );
	::rtl::OUString			GetItemName();
	void					SetItemText( const ::rtl::OUString& sItemText );
	::rtl::OUString			GetItemText();
	sal_uInt16				GetItemId() const { return m_nItemId; }

    virtual void			FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet );

	// OCommonAccessibleComponent 
	virtual ::com::sun::star::awt::Rectangle SAL_CALL	implGetBounds(  ) throw (::com::sun::star::uno::RuntimeException);

	// OCommonAccessibleText
	virtual ::rtl::OUString					implGetText();
	virtual ::com::sun::star::lang::Locale	implGetLocale();
	virtual void							implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex );

	// XComponent
	virtual void SAL_CALL	disposing();

public:
	VCLXAccessibleStatusBarItem( StatusBar* pStatusBar, sal_uInt16 nItemId );
	virtual ~VCLXAccessibleStatusBarItem();

	// XInterface
	DECLARE_XINTERFACE()

	// XTypeProvider
	DECLARE_XTYPEPROVIDER()

	// XServiceInfo
	virtual ::rtl::OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException);
	virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& rServiceName ) throw (::com::sun::star::uno::RuntimeException);
	virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException);

	// XAccessible
	virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException);

	// XAccessibleContext
	virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException);
	virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
	virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (::com::sun::star::uno::RuntimeException);
	virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) throw (::com::sun::star::uno::RuntimeException);
	virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException);
	virtual ::rtl::OUString SAL_CALL getAccessibleDescription(  ) throw (::com::sun::star::uno::RuntimeException);
	virtual ::rtl::OUString SAL_CALL getAccessibleName(  ) throw (::com::sun::star::uno::RuntimeException);
	virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (::com::sun::star::uno::RuntimeException);
	virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (::com::sun::star::uno::RuntimeException);
	virtual ::com::sun::star::lang::Locale SAL_CALL getLocale(  ) throw (::com::sun::star::accessibility::IllegalAccessibleComponentStateException, ::com::sun::star::uno::RuntimeException);

	// XAccessibleComponent
	virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException);
	virtual void SAL_CALL grabFocus(  ) throw (::com::sun::star::uno::RuntimeException);
	virtual sal_Int32 SAL_CALL getForeground(  ) throw (::com::sun::star::uno::RuntimeException);
	virtual sal_Int32 SAL_CALL getBackground(  ) throw (::com::sun::star::uno::RuntimeException);

	// XAccessibleExtendedComponent
	virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFont > SAL_CALL getFont(	) throw (::com::sun::star::uno::RuntimeException);
	virtual ::rtl::OUString SAL_CALL getTitledBorderText(  ) throw (::com::sun::star::uno::RuntimeException);
	virtual ::rtl::OUString SAL_CALL getToolTipText(  ) throw (::com::sun::star::uno::RuntimeException);

	// XAccessibleText
	virtual sal_Int32 SAL_CALL getCaretPosition() throw (::com::sun::star::uno::RuntimeException);
	virtual sal_Bool SAL_CALL setCaretPosition( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getCharacterAttributes( sal_Int32 nIndex, const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aRequestedAttributes ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
	virtual ::com::sun::star::awt::Rectangle SAL_CALL getCharacterBounds( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
	virtual sal_Int32 SAL_CALL getIndexAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException);
	virtual sal_Bool SAL_CALL setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
	virtual sal_Bool SAL_CALL copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
};

#endif // ACCESSIBILITY_STANDARD_VCLXACCESSIBLESTATUSBARITEM_HXX

