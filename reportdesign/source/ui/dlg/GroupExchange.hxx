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
#ifndef RPTUI_GROUP_EXCHANGE_HXX
#define RPTUI_GROUP_EXCHANGE_HXX

#include <com/sun/star/beans/PropertyValue.hpp>
#include <cppuhelper/implbase2.hxx>
#include <svtools/transfer.hxx>
#include "GroupsSorting.hxx"

namespace rptui
{
	/** clipboard class for group rows in the groups and sorting dialog
	*/
	class OGroupExchange : public TransferableHelper
	{
		::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any> m_aGroupRow;
	public:
		OGroupExchange(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any>& _aGroupRow);

        static sal_uInt32   getReportGroupId();
	protected:
		virtual void		AddSupportedFormats();
		virtual sal_Bool	GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
		virtual void		ObjectReleased();
	};
}
#endif // RPTUI_GROUP_EXCHANGE_HXX

