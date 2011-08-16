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

#ifndef _CONNECTIVITY_MOZAB_TABLE_HXX_
#define _CONNECTIVITY_MOZAB_TABLE_HXX_

#include "connectivity/TTableHelper.hxx"
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#ifndef _CONNECTIVITY_MOZAB_BCONNECTION_HXX_
#include "MConnection.hxx"
#endif

namespace connectivity
{
	namespace mozab
	{
        typedef ::connectivity::OTableHelper OTable_Base;

		class OTable :	public OTable_Base
		{
            OConnection*    m_pConnection;

		public:
			OTable(	sdbcx::OCollection* _pTables, OConnection* _pConnection);
			OTable(	sdbcx::OCollection* _pTables, 
                    OConnection* _pConnection,
					const ::rtl::OUString& _Name,
					const ::rtl::OUString& _Type,
					const ::rtl::OUString& _Description );

            OConnection* getConnection() { return m_pConnection;}

            sal_Bool isReadOnly() const { return sal_False; }

			::rtl::OUString getTableName() const { return m_Name; }
			::rtl::OUString getSchema() const { return m_SchemaName; }

            // OTableHelper overridables
            virtual sdbcx::OCollection* createColumns( const TStringVector& _rNames );
		    virtual sdbcx::OCollection* createKeys(const TStringVector& _rNames);
		    virtual sdbcx::OCollection* createIndexes(const TStringVector& _rNames);
		private:
			using OTable_Base::getConnection;
		};
	}
}
#endif // _CONNECTIVITY_MOZAB_TABLE_HXX_

