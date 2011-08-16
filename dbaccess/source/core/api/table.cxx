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
#include "precompiled_dbaccess.hxx"

#include "table.hxx"
#include <definitioncolumn.hxx>
#include "dbastrings.hrc"
#include "core_resource.hxx"
#include "core_resource.hrc"
#include "CIndexes.hxx"

#include <tools/debug.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/enumhelper.hxx>
#include <comphelper/container.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/types.hxx>
//#include <comphelper/extract.hxx>
#include <com/sun/star/util/XRefreshListener.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbcx/Privilege.hpp>
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>

#include <connectivity/TKeys.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/dbexception.hxx>

#include "sdbcoretools.hxx"
#include "ContainerMediator.hxx"
#include <rtl/logfile.hxx>

using namespace dbaccess;
using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::osl;
using namespace ::comphelper;
using namespace ::cppu;

typedef ::std::map <sal_Int32, OTableColumn*, std::less <sal_Int32> > OColMap;

//==========================================================================
//= ODBTable
//==========================================================================
DBG_NAME(ODBTable)
//--------------------------------------------------------------------------
ODBTable::ODBTable(connectivity::sdbcx::OCollection* _pTables
		,const Reference< XConnection >& _rxConn
		,const ::rtl::OUString& _rCatalog
		,const ::rtl::OUString& _rSchema 
		,const ::rtl::OUString& _rName
		,const ::rtl::OUString& _rType 
		,const ::rtl::OUString& _rDesc
		,const Reference< XNameAccess >& _xColumnDefinitions) throw(SQLException)
	:OTable_Base(_pTables,_rxConn,_rxConn->getMetaData().is() && _rxConn->getMetaData()->supportsMixedCaseQuotedIdentifiers(), _rName, _rType, _rDesc, _rSchema, _rCatalog )
	,m_xColumnDefinitions(_xColumnDefinitions)
	,m_nPrivileges(0)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ODBTable::ODBTable" );
	DBG_CTOR(ODBTable, NULL);
	DBG_ASSERT(getMetaData().is(), "ODBTable::ODBTable : invalid conn !");
	DBG_ASSERT(_rName.getLength(), "ODBTable::ODBTable : name !");
	// TODO : think about collecting the privileges here, as we can't ensure that in getFastPropertyValue, where
	// we do this at the moment, the statement needed can be supplied by the connection (for example the SQL-Server
	// ODBC driver does not allow more than one statement per connection, and in getFastPropertyValue it's more
	// likely that it's already used up than it's here.)
}
// -----------------------------------------------------------------------------
ODBTable::ODBTable(connectivity::sdbcx::OCollection* _pTables
				   ,const Reference< XConnection >& _rxConn)
				throw(SQLException) 
	:OTable_Base(_pTables,_rxConn, _rxConn->getMetaData().is() && _rxConn->getMetaData()->supportsMixedCaseQuotedIdentifiers())
	,m_nPrivileges(-1)
{
    DBG_CTOR(ODBTable, NULL);
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ODBTable::ODBTable" );
}
// -------------------------------------------------------------------------
ODBTable::~ODBTable()
{
	DBG_DTOR(ODBTable, NULL);
}
// -----------------------------------------------------------------------------
IMPLEMENT_FORWARD_REFCOUNT(ODBTable,OTable_Base)

//--------------------------------------------------------------------------
OColumn* ODBTable::createColumn(const ::rtl::OUString& _rName) const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ODBTable::createColumn" );
	OColumn* pReturn = NULL;

	Reference<XPropertySet> xProp;
	if ( m_xDriverColumns.is() && m_xDriverColumns->hasByName(_rName) )
	{
		xProp.set(m_xDriverColumns->getByName(_rName),UNO_QUERY);
	}
	else
	{
		OColumns* pColumns = static_cast<OColumns*>(m_pColumns);
		xProp.set(pColumns->createBaseObject(_rName),UNO_QUERY);
	}

	Reference<XPropertySet> xColumnDefintion;
	if ( m_xColumnDefinitions.is() && m_xColumnDefinitions->hasByName(_rName) )
		xColumnDefintion.set(m_xColumnDefinitions->getByName(_rName),UNO_QUERY);
	pReturn = new OTableColumnWrapper( xProp, xColumnDefintion, false );

	return pReturn;
}
// -----------------------------------------------------------------------------
void ODBTable::columnAppended( const Reference< XPropertySet >& /*_rxSourceDescriptor*/ )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ODBTable::columnAppended" );
    // not interested in
}
// -----------------------------------------------------------------------------
void ODBTable::columnDropped(const ::rtl::OUString& _sName)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ODBTable::columnDropped" );
	Reference<XDrop> xDrop(m_xColumnDefinitions,UNO_QUERY);
	if ( xDrop.is() && m_xColumnDefinitions->hasByName(_sName) )
	{
		xDrop->dropByName(_sName);
	}
}
//--------------------------------------------------------------------------
Sequence< sal_Int8 > ODBTable::getImplementationId() throw (RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ODBTable::getImplementationId" );
	static OImplementationId * pId = 0;
	if (! pId)
	{
		MutexGuard aGuard( Mutex::getGlobalMutex() );
		if (! pId)
		{
			static OImplementationId aId;
			pId = &aId;
		}
	}
	return pId->getImplementationId();
}

// OComponentHelper
//------------------------------------------------------------------------------
void SAL_CALL ODBTable::disposing()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ODBTable::disposing" );
	OPropertySetHelper::disposing();
	OTable_Base::disposing();
	m_xColumnDefinitions = NULL;
	m_xDriverColumns = NULL;
	m_pColumnMediator = NULL;
}

//------------------------------------------------------------------------------
void ODBTable::getFastPropertyValue(Any& _rValue, sal_Int32 _nHandle) const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ODBTable::getFastPropertyValue" );
	if ((PROPERTY_ID_PRIVILEGES == _nHandle) && (-1 == m_nPrivileges))
	{	// somebody is asking for the privileges an we do not know them, yet
		const_cast<ODBTable*>(this)->m_nPrivileges = ::dbtools::getTablePrivileges(getMetaData(),m_CatalogName,m_SchemaName, m_Name);
	}

	OTable_Base::getFastPropertyValue(_rValue, _nHandle);
}
// -------------------------------------------------------------------------
void ODBTable::construct()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ODBTable::construct" );
	::osl::MutexGuard aGuard(m_aMutex);
	
	// we don't collect the privileges here, this is potentially expensive. Instead we determine them on request.
	// (see getFastPropertyValue)
	m_nPrivileges = -1;

	OTable_Base::construct();

	registerProperty(PROPERTY_FILTER, PROPERTY_ID_FILTER, PropertyAttribute::BOUND,
					&m_sFilter, ::getCppuType(&m_sFilter));

	registerProperty(PROPERTY_ORDER, PROPERTY_ID_ORDER, PropertyAttribute::BOUND,
					&m_sOrder, ::getCppuType(&m_sOrder));

	registerProperty(PROPERTY_APPLYFILTER, PROPERTY_ID_APPLYFILTER, PropertyAttribute::BOUND,
					&m_bApplyFilter, ::getBooleanCppuType());

	registerProperty(PROPERTY_FONT, PROPERTY_ID_FONT, PropertyAttribute::BOUND,
					&m_aFont, ::getCppuType(&m_aFont));

	registerMayBeVoidProperty(PROPERTY_ROW_HEIGHT, PROPERTY_ID_ROW_HEIGHT, PropertyAttribute::BOUND | PropertyAttribute::MAYBEVOID,
					&m_aRowHeight, ::getCppuType(static_cast<sal_Int32*>(NULL)));

	registerMayBeVoidProperty(PROPERTY_TEXTCOLOR, PROPERTY_ID_TEXTCOLOR, PropertyAttribute::BOUND | PropertyAttribute::MAYBEVOID,
					&m_aTextColor, ::getCppuType(static_cast<sal_Int32*>(NULL)));

	registerProperty(PROPERTY_PRIVILEGES, PROPERTY_ID_PRIVILEGES, PropertyAttribute::BOUND | PropertyAttribute::READONLY,
					&m_nPrivileges, ::getCppuType(static_cast<sal_Int32*>(NULL)));
	
	registerMayBeVoidProperty(PROPERTY_TEXTLINECOLOR, PROPERTY_ID_TEXTLINECOLOR, PropertyAttribute::BOUND | PropertyAttribute::MAYBEVOID,
					&m_aTextLineColor, ::getCppuType(static_cast<sal_Int32*>(NULL)));
	
	registerProperty(PROPERTY_TEXTEMPHASIS, PROPERTY_ID_TEXTEMPHASIS, PropertyAttribute::BOUND,
					&m_nFontEmphasis, ::getCppuType(&m_nFontEmphasis));

	registerProperty(PROPERTY_TEXTRELIEF, PROPERTY_ID_TEXTRELIEF, PropertyAttribute::BOUND,
					&m_nFontRelief, ::getCppuType(&m_nFontRelief));

	registerProperty(PROPERTY_FONTNAME,			PROPERTY_ID_FONTNAME,		 PropertyAttribute::BOUND,&m_aFont.Name,			::getCppuType(&m_aFont.Name));
	registerProperty(PROPERTY_FONTHEIGHT,		PROPERTY_ID_FONTHEIGHT,		 PropertyAttribute::BOUND,&m_aFont.Height,			::getCppuType(&m_aFont.Height));
	registerProperty(PROPERTY_FONTWIDTH,		PROPERTY_ID_FONTWIDTH,		 PropertyAttribute::BOUND,&m_aFont.Width,			::getCppuType(&m_aFont.Width));
	registerProperty(PROPERTY_FONTSTYLENAME,	PROPERTY_ID_FONTSTYLENAME,	 PropertyAttribute::BOUND,&m_aFont.StyleName,		::getCppuType(&m_aFont.StyleName));
	registerProperty(PROPERTY_FONTFAMILY,		PROPERTY_ID_FONTFAMILY,		 PropertyAttribute::BOUND,&m_aFont.Family,			::getCppuType(&m_aFont.Family));
	registerProperty(PROPERTY_FONTCHARSET,		PROPERTY_ID_FONTCHARSET,	 PropertyAttribute::BOUND,&m_aFont.CharSet,			::getCppuType(&m_aFont.CharSet));
	registerProperty(PROPERTY_FONTPITCH,		PROPERTY_ID_FONTPITCH,		 PropertyAttribute::BOUND,&m_aFont.Pitch,			::getCppuType(&m_aFont.Pitch));
	registerProperty(PROPERTY_FONTCHARWIDTH,	PROPERTY_ID_FONTCHARWIDTH,	 PropertyAttribute::BOUND,&m_aFont.CharacterWidth,	::getCppuType(&m_aFont.CharacterWidth));
	registerProperty(PROPERTY_FONTWEIGHT,		PROPERTY_ID_FONTWEIGHT,		 PropertyAttribute::BOUND,&m_aFont.Weight,			::getCppuType(&m_aFont.Weight));
	registerProperty(PROPERTY_FONTSLANT,		PROPERTY_ID_FONTSLANT,		 PropertyAttribute::BOUND,&m_aFont.Slant,			::getCppuType(&m_aFont.Slant));
	registerProperty(PROPERTY_FONTUNDERLINE,	PROPERTY_ID_FONTUNDERLINE,	 PropertyAttribute::BOUND,&m_aFont.Underline,		::getCppuType(&m_aFont.Underline));
	registerProperty(PROPERTY_FONTSTRIKEOUT,	PROPERTY_ID_FONTSTRIKEOUT,	 PropertyAttribute::BOUND,&m_aFont.Strikeout,		::getCppuType(&m_aFont.Strikeout));
	registerProperty(PROPERTY_FONTORIENTATION,	PROPERTY_ID_FONTORIENTATION, PropertyAttribute::BOUND,&m_aFont.Orientation,		::getCppuType(&m_aFont.Orientation));
	registerProperty(PROPERTY_FONTKERNING,		PROPERTY_ID_FONTKERNING,	 PropertyAttribute::BOUND,&m_aFont.Kerning,			::getCppuType(&m_aFont.Kerning));
	registerProperty(PROPERTY_FONTWORDLINEMODE, PROPERTY_ID_FONTWORDLINEMODE,PropertyAttribute::BOUND,&m_aFont.WordLineMode,	::getCppuType(&m_aFont.WordLineMode));
	registerProperty(PROPERTY_FONTTYPE,			PROPERTY_ID_FONTTYPE,		 PropertyAttribute::BOUND,&m_aFont.Type,			::getCppuType(&m_aFont.Type));

	refreshColumns();
}
// -----------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* ODBTable::createArrayHelper( sal_Int32 _nId) const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ODBTable::createArrayHelper" );
	Sequence< Property > aProps;
	describeProperties(aProps);
	if(!_nId)
	{
		Property* pIter	= aProps.getArray();
		Property* pEnd	= pIter + aProps.getLength();
		for(;pIter != pEnd;++pIter)
		{
			if (0 == pIter->Name.compareToAscii(PROPERTY_CATALOGNAME))
				pIter->Attributes = PropertyAttribute::READONLY;
			else if (0 == pIter->Name.compareToAscii(PROPERTY_SCHEMANAME))
				pIter->Attributes = PropertyAttribute::READONLY;
			else if (0 == pIter->Name.compareToAscii(PROPERTY_DESCRIPTION))
				pIter->Attributes = PropertyAttribute::READONLY;
			else if (0 == pIter->Name.compareToAscii(PROPERTY_NAME))
				pIter->Attributes = PropertyAttribute::READONLY;
		}
	}
	
	return new ::cppu::OPropertyArrayHelper(aProps);			
}
// -----------------------------------------------------------------------------																
::cppu::IPropertyArrayHelper & SAL_CALL ODBTable::getInfoHelper() 
{																
	return *ODBTable_PROP::getArrayHelper(isNew() ? 1 : 0);
}
// -------------------------------------------------------------------------
// XServiceInfo
IMPLEMENT_SERVICE_INFO1(ODBTable, "com.sun.star.sdb.dbaccess.ODBTable", SERVICE_SDBCX_TABLE)
// -------------------------------------------------------------------------
Any SAL_CALL ODBTable::queryInterface( const Type & rType ) throw(RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ODBTable::getInfoHelper" );
	if(rType == getCppuType( (Reference<XRename>*)0) && !getRenameService().is() )
		return Any();
	if(rType == getCppuType( (Reference<XAlterTable>*)0) && !getAlterService().is() )
		return Any();
	return OTable_Base::queryInterface( rType);
}
// -------------------------------------------------------------------------
Sequence< Type > SAL_CALL ODBTable::getTypes(  ) throw(RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ODBTable::getTypes" );
	Type aRenameType = getCppuType( (Reference<XRename>*)0);
	Type aAlterType = getCppuType( (Reference<XAlterTable>*)0);

	Sequence< Type > aTypes(OTable_Base::getTypes());
	::std::vector<Type> aOwnTypes;
	aOwnTypes.reserve(aTypes.getLength());	

	const Type* pIter = aTypes.getConstArray();
	const Type* pEnd = pIter + aTypes.getLength();
	for(;pIter != pEnd ;++pIter)
	{
		if( (*pIter != aRenameType || getRenameService().is()) && (*pIter != aAlterType || getAlterService().is()))
			aOwnTypes.push_back(*pIter);
	}
	
	Type* pTypes = aOwnTypes.empty() ? 0 : &aOwnTypes[0];
	return Sequence< Type >(pTypes, aOwnTypes.size());
}
// XRename,
//------------------------------------------------------------------------------
void SAL_CALL ODBTable::rename( const ::rtl::OUString& _rNewName ) throw(SQLException, ElementExistException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ODBTable::rename" );
    ::osl::MutexGuard aGuard(m_aMutex);
	checkDisposed(connectivity::sdbcx::OTableDescriptor_BASE::rBHelper.bDisposed);
    if ( !getRenameService().is() )
	    throw SQLException(DBACORE_RESSTRING(RID_STR_NO_TABLE_RENAME),*this,SQLSTATE_GENERAL,1000,Any() );

    Reference<XPropertySet> xTable(this);
    getRenameService()->rename(xTable,_rNewName);
    ::connectivity::OTable_TYPEDEF::rename(_rNewName);
}

// XAlterTable,
//------------------------------------------------------------------------------
void SAL_CALL ODBTable::alterColumnByName( const ::rtl::OUString& _rName, const Reference< XPropertySet >& _rxDescriptor ) throw(SQLException, NoSuchElementException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ODBTable::alterColumnByName" );
	::osl::MutexGuard aGuard(m_aMutex);
	checkDisposed(connectivity::sdbcx::OTableDescriptor_BASE::rBHelper.bDisposed);
    if ( !getAlterService().is() )
	    throw SQLException(DBACORE_RESSTRING(RID_STR_NO_TABLE_RENAME),*this,SQLSTATE_GENERAL,1000,Any() );

    if ( !m_pColumns->hasByName(_rName) )
        throw SQLException(DBACORE_RESSTRING(RID_STR_COLUMN_NOT_VALID),*this,SQLSTATE_GENERAL,1000,Any() );

    Reference<XPropertySet> xTable(this);
    getAlterService()->alterColumnByName(xTable,_rName,_rxDescriptor);
	m_pColumns->refresh();
}
// -----------------------------------------------------------------------------
sal_Int64 SAL_CALL ODBTable::getSomething( const Sequence< sal_Int8 >& rId ) throw(RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ODBTable::getSomething" );
	sal_Int64 nRet(0);
	if (rId.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
		nRet = reinterpret_cast<sal_Int64>(this);
	else
		nRet = OTable_Base::getSomething(rId);

	return nRet;
}
// -----------------------------------------------------------------------------
Sequence< sal_Int8 > ODBTable::getUnoTunnelImplementationId()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ODBTable::getUnoTunnelImplementationId" );
	static ::cppu::OImplementationId * pId = 0;
	if (! pId)
	{
		::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
		if (! pId)
		{
			static ::cppu::OImplementationId aId;
			pId = &aId;
		}
	}
	return pId->getImplementationId();
}
// -----------------------------------------------------------------------------
Reference< XPropertySet > ODBTable::createColumnDescriptor()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ODBTable::createColumnDescriptor" );
	return new OTableColumnDescriptor( true );
}
// -----------------------------------------------------------------------------
sdbcx::OCollection* ODBTable::createColumns(const TStringVector& _rNames)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ODBTable::createColumns" );
	Reference<XDatabaseMetaData> xMeta = getMetaData();
	OColumns* pCol = new OColumns(*this, m_aMutex, NULL, isCaseSensitive(), _rNames, this,this,
									getAlterService().is() || (xMeta.is() && xMeta->supportsAlterTableWithAddColumn()),
									getAlterService().is() || (xMeta.is() && xMeta->supportsAlterTableWithDropColumn()));
	static_cast<OColumnsHelper*>(pCol)->setParent(this);
	pCol->setParent(*this);
	m_pColumnMediator = new OContainerMediator( pCol, m_xColumnDefinitions, getConnection(), OContainerMediator::eColumns );
	pCol->setMediator( m_pColumnMediator.get() );
	return pCol;
}
// -----------------------------------------------------------------------------
sdbcx::OCollection* ODBTable::createKeys(const TStringVector& _rNames)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ODBTable::createKeys" );
	return new connectivity::OKeysHelper(this,m_aMutex,_rNames);
}
// -----------------------------------------------------------------------------
sdbcx::OCollection* ODBTable::createIndexes(const TStringVector& _rNames)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ODBTable::createIndexes" );
	return new OIndexes(this,m_aMutex,_rNames,NULL);
}
// -----------------------------------------------------------------------------
