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

#ifndef CONNECTIVITY_HSQLDB_TABLE_HXX
#define CONNECTIVITY_HSQLDB_TABLE_HXX

#include "connectivity/TTableHelper.hxx"
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <comphelper/IdPropArrayHelper.hxx>

namespace connectivity
{
	namespace hsqldb
	{

        ::rtl::OUString getTypeString(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xColProp);

		class OHSQLTable;
		typedef ::comphelper::OIdPropertyArrayUsageHelper< OHSQLTable >	OHSQLTable_PROP;
		class OHSQLTable :	public OTableHelper
							,public OHSQLTable_PROP
		{
			sal_Int32 m_nPrivileges; // we have to set our privileges by our own

			/** executes the statmenmt.
				@param	_rStatement
					The statement to execute.
				*/
			void executeStatement(const ::rtl::OUString& _rStatement );
		protected:

			/** creates the column collection for the table
				@param	_rNames
					The column names.
			*/
			virtual sdbcx::OCollection* createColumns(const TStringVector& _rNames);

			/** creates the key collection for the table
				@param	_rNames
					The key names.
			*/
			virtual sdbcx::OCollection* createKeys(const TStringVector& _rNames);

			/** creates the index collection for the table
				@param	_rNames
					The index names.
			*/
			virtual sdbcx::OCollection* createIndexes(const TStringVector& _rNames);

			/** used to implement the creation of the array helper which is shared amongst all instances of the class.
				This method needs to be implemented in derived classes.
				<BR>
				The method gets called with s_aMutex acquired.
				<BR>
				as long as IPropertyArrayHelper has no virtual destructor, the implementation of ~OPropertyArrayUsageHelper
				assumes that you created an ::cppu::OPropertyArrayHelper when deleting s_pProps.
				@return							an pointer to the newly created array helper. Must not be NULL.
			*/
			virtual ::cppu::IPropertyArrayHelper* createArrayHelper(sal_Int32 nId) const;
			virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();
			
		public:
			OHSQLTable(	sdbcx::OCollection* _pTables,
							const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xConnection);
			OHSQLTable(	sdbcx::OCollection* _pTables,
							const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xConnection,
							const ::rtl::OUString& _Name,
							const ::rtl::OUString& _Type,
							const ::rtl::OUString& _Description = ::rtl::OUString(),
							const ::rtl::OUString& _SchemaName = ::rtl::OUString(),
							const ::rtl::OUString& _CatalogName = ::rtl::OUString(),
							sal_Int32 _nPrivileges = 0
				);

			// ODescriptor
			virtual void construct();
			// com::sun::star::lang::XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
            static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();

			virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
			//XTypeProvider
			virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
			// XAlterTable
            virtual void SAL_CALL alterColumnByName( const ::rtl::OUString& colName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
            
            // XRename
            virtual void SAL_CALL rename( const ::rtl::OUString& newName ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException);

			/**
				returns the ALTER TABLE XXX COLUMN statement
			*/
			::rtl::OUString getAlterTableColumnPart();

			// some methods to alter table structures
			void alterColumnType(sal_Int32 nNewType,const ::rtl::OUString& _rColName,const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _xDescriptor);
			void alterDefaultValue(const ::rtl::OUString& _sNewDefault,const ::rtl::OUString& _rColName);
			void dropDefaultValue(const ::rtl::OUString& _sNewDefault);

		};
	}
}
#endif // CONNECTIVITY_HSQLDB_TABLE_HXX

