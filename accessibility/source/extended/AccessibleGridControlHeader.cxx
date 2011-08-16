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
#include "precompiled_accessibility.hxx"


#include "accessibility/extended/AccessibleGridControlHeader.hxx"
#include "accessibility/extended/AccessibleGridControlHeaderCell.hxx"
#include "accessibility/extended/AccessibleGridControlTableCell.hxx"
#include <svtools/accessibletable.hxx>


// ============================================================================

using ::rtl::OUString;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::accessibility;
using namespace ::svt;
using namespace ::svt::table;

// ============================================================================

namespace accessibility {

// ============================================================================

DBG_NAME( AccessibleGridControlHeader )

AccessibleGridControlHeader::AccessibleGridControlHeader(
        const Reference< XAccessible >& rxParent,
        ::svt::table::IAccessibleTable&                      rTable,
		::svt::table::AccessibleTableControlObjType      eObjType):
		AccessibleGridControlTableBase( rxParent, rTable, eObjType )
{
    DBG_ASSERT( isRowBar() || isColumnBar(),
        "accessibility/extended/AccessibleGridControlHeaderBar - invalid object type" );
}

AccessibleGridControlHeader::~AccessibleGridControlHeader()
{
}

// XAccessibleContext ---------------------------------------------------------

Reference< XAccessible > SAL_CALL
AccessibleGridControlHeader::getAccessibleChild( sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    TCSolarGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );

    if (nChildIndex<0 || nChildIndex>=getAccessibleChildCount())
	throw IndexOutOfBoundsException();
    ensureIsAlive();
    Reference< XAccessible > xChild;
    if(m_eObjType == svt::table::TCTYPE_COLUMNHEADERBAR)
    {
	    AccessibleGridControlHeaderCell* pColHeaderCell = new AccessibleGridControlHeaderCell(nChildIndex, this, m_aTable, svt::table::TCTYPE_COLUMNHEADERCELL);
	    xChild = pColHeaderCell;
    }
    else if(m_eObjType == svt::table::TCTYPE_ROWHEADERBAR)
    {
	    AccessibleGridControlHeaderCell* pRowHeaderCell = new AccessibleGridControlHeaderCell(nChildIndex, this, m_aTable, svt::table::TCTYPE_ROWHEADERCELL);
	    xChild = pRowHeaderCell;
    }
    return xChild;
}

sal_Int32 SAL_CALL AccessibleGridControlHeader::getAccessibleIndexInParent()
    throw ( uno::RuntimeException )
{
     ensureIsAlive();
     if(m_eObjType == svt::table::TCTYPE_ROWHEADERBAR && m_aTable.HasColHeader())
	     return 1;
     else
	     return 0;
}

// XAccessibleComponent -------------------------------------------------------

Reference< XAccessible > SAL_CALL
AccessibleGridControlHeader::getAccessibleAtPoint( const awt::Point& rPoint )
    throw ( uno::RuntimeException )
{
    TCSolarGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();

    sal_Int32 nRow = 0;
    sal_Int32 nColumnPos = 0;
    sal_Bool bConverted = isRowBar() ?
    m_aTable.ConvertPointToCellAddress( nRow, nColumnPos, VCLPoint( rPoint ) ) :
    m_aTable.ConvertPointToCellAddress( nRow, nColumnPos, VCLPoint( rPoint ) );

    return bConverted ? implGetChild( nRow, nColumnPos ) : Reference< XAccessible >();
}

void SAL_CALL AccessibleGridControlHeader::grabFocus()
    throw ( uno::RuntimeException )
{
    ensureIsAlive();
    // focus on header not supported
}

Any SAL_CALL AccessibleGridControlHeader::getAccessibleKeyBinding()
    throw ( uno::RuntimeException )
{
    ensureIsAlive();
    return Any();   // no special key bindings for header
}

// XAccessibleTable -----------------------------------------------------------

OUString SAL_CALL AccessibleGridControlHeader::getAccessibleRowDescription( sal_Int32 nRow )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    TCSolarGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    ensureIsValidRow( nRow );
    return OUString();  // no headers in headers
}

OUString SAL_CALL AccessibleGridControlHeader::getAccessibleColumnDescription( sal_Int32 nColumn )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    TCSolarGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    ensureIsValidColumn( nColumn );
    return OUString();  // no headers in headers
}

Reference< XAccessibleTable > SAL_CALL AccessibleGridControlHeader::getAccessibleRowHeaders()
    throw ( uno::RuntimeException )
{
    ensureIsAlive();
    return NULL;        // no headers in headers
}

Reference< XAccessibleTable > SAL_CALL AccessibleGridControlHeader::getAccessibleColumnHeaders()
    throw ( uno::RuntimeException )
{
    ensureIsAlive();
    return NULL;        // no headers in headers
}
//not selectable
Sequence< sal_Int32 > SAL_CALL AccessibleGridControlHeader::getSelectedAccessibleRows()
    throw ( uno::RuntimeException )
{
    Sequence< sal_Int32 > aSelSeq(0);
    return aSelSeq;
}
//columns aren't selectable
Sequence< sal_Int32 > SAL_CALL AccessibleGridControlHeader::getSelectedAccessibleColumns()
    throw ( uno::RuntimeException )
{
    Sequence< sal_Int32 > aSelSeq(0);
    return aSelSeq;
}
//row headers not selectable
sal_Bool SAL_CALL AccessibleGridControlHeader::isAccessibleRowSelected( sal_Int32 /*nRow*/ )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    return sal_False;
}
//columns aren't selectable
sal_Bool SAL_CALL AccessibleGridControlHeader::isAccessibleColumnSelected( sal_Int32 nColumn )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    (void)nColumn;
    return sal_False;
}
//not implemented
Reference< XAccessible > SAL_CALL AccessibleGridControlHeader::getAccessibleCellAt(
        sal_Int32 /*nRow*/, sal_Int32 /*nColumn*/ )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    return NULL;
}
// not selectable
sal_Bool SAL_CALL AccessibleGridControlHeader::isAccessibleSelected(
        sal_Int32 /*nRow*/, sal_Int32 /*nColumn */)
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    return sal_False;
}

// XServiceInfo ---------------------------------------------------------------

OUString SAL_CALL AccessibleGridControlHeader::getImplementationName()
    throw ( uno::RuntimeException )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.accessibility.AccessibleGridControlHeader" ) );
}

Sequence< sal_Int8 > SAL_CALL AccessibleGridControlHeader::getImplementationId()
    throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( getOslGlobalMutex() );
    static Sequence< sal_Int8 > aId;
    implCreateUuid( aId );
    return aId;
}

// internal virtual methods ---------------------------------------------------

Rectangle AccessibleGridControlHeader::implGetBoundingBox()
{
    Window* pParent = m_aTable.GetAccessibleParentWindow();
	Rectangle aGridRect( m_aTable.GetWindowExtentsRelative( pParent ) );
	Rectangle aHeaderRect (m_aTable.calcHeaderRect(isColumnBar()));
	if(isColumnBar())
		return Rectangle(aGridRect.TopLeft(), Size(aGridRect.getWidth(),aHeaderRect.getHeight()));
	else
		return Rectangle(aGridRect.TopLeft(), Size(aHeaderRect.getWidth(),aGridRect.getHeight()));

}

Rectangle AccessibleGridControlHeader::implGetBoundingBoxOnScreen()
{
    Rectangle aGridRect( m_aTable.GetWindowExtentsRelative( NULL ) );
	Rectangle aHeaderRect (m_aTable.calcHeaderRect(isColumnBar()));
	if(isColumnBar())
		return Rectangle(aGridRect.TopLeft(), Size(aGridRect.getWidth(),aHeaderRect.getHeight()));
	else
		return Rectangle(aGridRect.TopLeft(), Size(aHeaderRect.getWidth(),aGridRect.getHeight()));
}

sal_Int32 AccessibleGridControlHeader::implGetRowCount() const
{
    return 1;
}

sal_Int32 AccessibleGridControlHeader::implGetColumnCount() const
{
    return 1;
}

// internal helper methods ----------------------------------------------------

Reference< XAccessible > AccessibleGridControlHeader::implGetChild(
        sal_Int32 nRow, sal_uInt32 nColumnPos )
{
    Reference< XAccessible > xChild;
    if(m_eObjType == svt::table::TCTYPE_COLUMNHEADERBAR)
    {
	    AccessibleGridControlHeaderCell* pColHeaderCell = new AccessibleGridControlHeaderCell(nColumnPos, this, m_aTable, svt::table::TCTYPE_COLUMNHEADERCELL);
	    xChild = pColHeaderCell;
    }
    else if(m_eObjType == svt::table::TCTYPE_ROWHEADERBAR)
    {
	    AccessibleGridControlHeaderCell* pRowHeaderCell = new AccessibleGridControlHeaderCell(nRow, this, m_aTable, svt::table::TCTYPE_ROWHEADERCELL);
	    xChild = pRowHeaderCell;
    }
    return xChild;
}

// ============================================================================

} // namespace accessibility

// ============================================================================

