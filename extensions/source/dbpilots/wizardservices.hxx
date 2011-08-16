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

#ifndef _EXTENSIONS_DBW_WIZARDSERVICES_HXX_
#define _EXTENSIONS_DBW_WIZARDSERVICES_HXX_

#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ustring.hxx>

//.........................................................................
namespace dbp
{
//.........................................................................

	//=====================================================================
	//= OGroupBoxSI
	//=====================================================================
	/// service info for the group box wizard
	struct OGroupBoxSI
	{
	public:
		::rtl::OUString			getImplementationName() const;
		::com::sun::star::uno::Sequence< ::rtl::OUString >
								getServiceNames() const;
	};

	//=====================================================================
	//= OListComboSI
	//=====================================================================
	/// service info for the list/combo box wizard
	struct OListComboSI
	{
	public:
		::rtl::OUString			getImplementationName() const;
		::com::sun::star::uno::Sequence< ::rtl::OUString >
								getServiceNames() const;
	};

	//=====================================================================
	//= OGridSI
	//=====================================================================
	/// service info for the grid wizard
	struct OGridSI
	{
	public:
		::rtl::OUString			getImplementationName() const;
		::com::sun::star::uno::Sequence< ::rtl::OUString >
								getServiceNames() const;
	};

//.........................................................................
}	// namespace dbp
//.........................................................................

#endif // _EXTENSIONS_DBW_WIZARDSERVICES_HXX_

