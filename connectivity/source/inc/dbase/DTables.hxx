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

#ifndef _CONNECTIVITY_DBASE_TABLES_HXX_
#define _CONNECTIVITY_DBASE_TABLES_HXX_

#include "file/FTables.hxx"

namespace connectivity
{
	namespace dbase
	{
                //      namespace ::com::sun::star::sdbcx             = ::com::sun::star::sdbcx;
		typedef file::OTables ODbaseTables_BASE;

		class ODbaseTables : public ODbaseTables_BASE
		{
		protected:
            virtual sdbcx::ObjectType createObject(const ::rtl::OUString& _rName);
			virtual void impl_refresh() throw(::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > createDescriptor();
            virtual sdbcx::ObjectType appendObject( const ::rtl::OUString& _rForName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor );
			virtual void dropObject(sal_Int32 _nPos,const ::rtl::OUString _sElementName);
		public:
            ODbaseTables(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& _rMetaData,::cppu::OWeakObject& _rParent, ::osl::Mutex& _rMutex,
				const TStringVector &_rVector) : ODbaseTables_BASE(_rMetaData,_rParent,_rMutex,_rVector)
			{}

			virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
		};
	}
}
#endif // _CONNECTIVITY_DBASE_TABLES_HXX_

