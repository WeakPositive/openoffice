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

#ifndef _SVX_SHOWCOLS_HXX
#define _SVX_SHOWCOLS_HXX

#include <vcl/dialog.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>

#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>

//==========================================================================
//	FmShowColsDialog
//==========================================================================

class FmShowColsDialog : public ModalDialog
{
	ListBox			m_aList;
	FixedText		m_aLabel;
	OKButton		m_aOK;
	CancelButton	m_aCancel;

	::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > 	m_xColumns;

public:
	FmShowColsDialog( Window* pParent );
	virtual ~FmShowColsDialog();

	void SetColumns(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer>& xCols);

protected:
	DECL_LINK( OnClickedOk, Button* );
};

#endif // _SVX_SHOWCOLS_HXX

