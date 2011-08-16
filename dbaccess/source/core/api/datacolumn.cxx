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
#ifndef _DBACORE_DATACOLUMN_HXX_
#include "datacolumn.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETMETADATASUPPLIER_HPP_
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif

using namespace dbaccess;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::osl;
using namespace ::comphelper;
using namespace ::cppu;

DBG_NAME(ODataColumn)
//--------------------------------------------------------------------------
ODataColumn::ODataColumn(
						 const Reference < XResultSetMetaData >& _xMetaData,
						 const Reference < XRow >& _xRow,
						 const Reference < XRowUpdate >& _xRowUpdate,
						 sal_Int32 _nPos,
                         const Reference< XDatabaseMetaData >& _rxDBMeta)
					 :OResultColumn(_xMetaData, _nPos, _rxDBMeta)
					 ,m_xRow(_xRow)
					 ,m_xRowUpdate(_xRowUpdate)
{
	DBG_CTOR(ODataColumn,NULL);
}
// -----------------------------------------------------------------------------
ODataColumn::~ODataColumn()
{
	DBG_DTOR(ODataColumn,NULL);
}

// com::sun::star::lang::XTypeProvider
//--------------------------------------------------------------------------
Sequence< Type > ODataColumn::getTypes() throw (RuntimeException)
{
	OTypeCollection aTypes(::getCppuType( (const Reference< XColumn > *)0 ),
						   ::getCppuType( (const Reference< XColumnUpdate > *)0 ),
						   OColumn::getTypes());
	return aTypes.getTypes();
}

//--------------------------------------------------------------------------
Sequence< sal_Int8 > ODataColumn::getImplementationId() throw (RuntimeException)
{
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

//------------------------------------------------------------------------------
Any SAL_CALL ODataColumn::queryInterface( const Type & _rType ) throw (RuntimeException)
{
	Any aReturn = OResultColumn::queryInterface(_rType);
	if (!aReturn.hasValue())
		aReturn = ::cppu::queryInterface(_rType,
			static_cast< XColumn* >(this),
			static_cast< XColumnUpdate* >(this)
		);
	return aReturn;
}

// XServiceInfo
//------------------------------------------------------------------------------
rtl::OUString ODataColumn::getImplementationName(  ) throw(RuntimeException)
{
	return rtl::OUString::createFromAscii("com.sun.star.sdb.ODataColumn");
}

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > ODataColumn::getSupportedServiceNames(  ) throw (RuntimeException)
{
	Sequence< ::rtl::OUString > aSNS( 3 );
	aSNS[0] = SERVICE_SDBCX_COLUMN;
	aSNS[1] = SERVICE_SDB_RESULTCOLUMN;
	aSNS[2] = SERVICE_SDB_DATACOLUMN;
	return aSNS;
}

// OComponentHelper
//------------------------------------------------------------------------------
void ODataColumn::disposing()
{
	OResultColumn::disposing();

	m_xRow = NULL;
	m_xRowUpdate = NULL;
}

// ::com::sun::star::sdb::XColumn
//------------------------------------------------------------------------------
sal_Bool ODataColumn::wasNull(void) throw( SQLException, RuntimeException )
{
	MutexGuard aGuard(m_aMutex);
	::connectivity::checkDisposed(!m_xRow.is());
	
	return m_xRow->wasNull();
}

//------------------------------------------------------------------------------
rtl::OUString ODataColumn::getString(void) throw( SQLException, RuntimeException )
{
	MutexGuard aGuard(m_aMutex);
	::connectivity::checkDisposed(!m_xRow.is());

	return m_xRow->getString(m_nPos);
}

//------------------------------------------------------------------------------
sal_Bool ODataColumn::getBoolean(void) throw( SQLException, RuntimeException )
{
	MutexGuard aGuard(m_aMutex);
	::connectivity::checkDisposed(!m_xRow.is());

	return m_xRow->getBoolean(m_nPos);
}

//------------------------------------------------------------------------------
sal_Int8 ODataColumn::getByte(void) throw( SQLException, RuntimeException )
{
	MutexGuard aGuard(m_aMutex);
	::connectivity::checkDisposed(!m_xRow.is());

	return m_xRow->getByte(m_nPos);
}

//------------------------------------------------------------------------------
sal_Int16 ODataColumn::getShort(void) throw( SQLException, RuntimeException )
{
	MutexGuard aGuard(m_aMutex);
	::connectivity::checkDisposed(!m_xRow.is());

	return m_xRow->getShort(m_nPos);
}

//------------------------------------------------------------------------------
sal_Int32 ODataColumn::getInt(void) throw( SQLException, RuntimeException )
{
	MutexGuard aGuard(m_aMutex);
	::connectivity::checkDisposed(!m_xRow.is());

	return m_xRow->getInt(m_nPos);
}

//------------------------------------------------------------------------------
sal_Int64 ODataColumn::getLong(void) throw( SQLException, RuntimeException )
{
	MutexGuard aGuard(m_aMutex);
	::connectivity::checkDisposed(!m_xRow.is());

	return m_xRow->getLong(m_nPos);
}

//------------------------------------------------------------------------------
float ODataColumn::getFloat(void) throw( SQLException, RuntimeException )
{
	MutexGuard aGuard(m_aMutex);
	::connectivity::checkDisposed(!m_xRow.is());

	return m_xRow->getFloat(m_nPos);
}
//------------------------------------------------------------------------------
double ODataColumn::getDouble(void) throw( SQLException, RuntimeException )
{
	MutexGuard aGuard(m_aMutex);
	::connectivity::checkDisposed(!m_xRow.is());

	return m_xRow->getDouble(m_nPos);
}

//------------------------------------------------------------------------------
Sequence< sal_Int8 > ODataColumn::getBytes(void) throw( SQLException, RuntimeException )
{
	MutexGuard aGuard(m_aMutex);
	::connectivity::checkDisposed(!m_xRow.is());

	return m_xRow->getBytes(m_nPos);
}
//------------------------------------------------------------------------------
com::sun::star::util::Date ODataColumn::getDate(void) throw( SQLException, RuntimeException )
{
	MutexGuard aGuard(m_aMutex);
	::connectivity::checkDisposed(!m_xRow.is());

	return m_xRow->getDate(m_nPos);
}

//------------------------------------------------------------------------------
com::sun::star::util::Time ODataColumn::getTime(void) throw( SQLException, RuntimeException )
{
	MutexGuard aGuard(m_aMutex);
	::connectivity::checkDisposed(!m_xRow.is());

	return m_xRow->getTime(m_nPos);
}
//------------------------------------------------------------------------------
com::sun::star::util::DateTime ODataColumn::getTimestamp(void) throw( SQLException, RuntimeException )
{
	MutexGuard aGuard(m_aMutex);
	::connectivity::checkDisposed(!m_xRow.is());

	return m_xRow->getTimestamp(m_nPos);
}

//------------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream >  ODataColumn::getBinaryStream(void) throw( SQLException, RuntimeException )
{
	MutexGuard aGuard(m_aMutex);
	::connectivity::checkDisposed(!m_xRow.is());

	return m_xRow->getBinaryStream(m_nPos);
}

//------------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream >  ODataColumn::getCharacterStream(void) throw( SQLException, RuntimeException )
{
	MutexGuard aGuard(m_aMutex);
	::connectivity::checkDisposed(!m_xRow.is());

	return m_xRow->getCharacterStream(m_nPos);
}

//------------------------------------------------------------------------------
Any ODataColumn::getObject(const Reference< ::com::sun::star::container::XNameAccess > & typeMap) throw( SQLException, RuntimeException )
{
	MutexGuard aGuard(m_aMutex);
	::connectivity::checkDisposed(!m_xRow.is());

	return m_xRow->getObject(m_nPos, typeMap);
}

//------------------------------------------------------------------------------
Reference< XRef >  ODataColumn::getRef(void) throw( SQLException, RuntimeException )
{
	MutexGuard aGuard(m_aMutex);
	::connectivity::checkDisposed(!m_xRow.is());

	return m_xRow->getRef(m_nPos);
}

//------------------------------------------------------------------------------
Reference< XBlob >  ODataColumn::getBlob(void) throw( SQLException, RuntimeException )
{
	MutexGuard aGuard(m_aMutex);
	::connectivity::checkDisposed(!m_xRow.is());

	return m_xRow->getBlob(m_nPos);
}

//------------------------------------------------------------------------------
Reference< XClob >  ODataColumn::getClob(void) throw( SQLException, RuntimeException )
{
	MutexGuard aGuard(m_aMutex);
	::connectivity::checkDisposed(!m_xRow.is());

	return m_xRow->getClob(m_nPos);
}

//------------------------------------------------------------------------------
Reference< XArray >  ODataColumn::getArray(void) throw( SQLException, RuntimeException )
{
	MutexGuard aGuard(m_aMutex);
	::connectivity::checkDisposed(!m_xRow.is());

	return m_xRow->getArray(m_nPos);
}

// ::com::sun::star::sdb::XColumnUpdate
//------------------------------------------------------------------------------
void ODataColumn::updateNull(void) throw( SQLException, RuntimeException )
{
	MutexGuard aGuard( m_aMutex );
	::connectivity::checkDisposed(!m_xRowUpdate.is());

	m_xRowUpdate->updateNull(m_nPos);
}

//------------------------------------------------------------------------------
void ODataColumn::updateBoolean(sal_Bool x) throw( SQLException, RuntimeException )
{
	MutexGuard aGuard( m_aMutex );
	::connectivity::checkDisposed(!m_xRowUpdate.is());

	m_xRowUpdate->updateBoolean(m_nPos, x);
}

//------------------------------------------------------------------------------
void ODataColumn::updateByte(sal_Int8 x) throw( SQLException, RuntimeException )
{
	MutexGuard aGuard( m_aMutex );
	::connectivity::checkDisposed(!m_xRowUpdate.is());

	m_xRowUpdate->updateByte(m_nPos, x);
}

//------------------------------------------------------------------------------
void ODataColumn::updateShort(sal_Int16 x) throw( SQLException, RuntimeException )
{
	MutexGuard aGuard( m_aMutex );
	::connectivity::checkDisposed(!m_xRowUpdate.is());

	m_xRowUpdate->updateShort(m_nPos, x);
}

//------------------------------------------------------------------------------
void ODataColumn::updateInt(sal_Int32 x) throw( SQLException, RuntimeException )
{
	MutexGuard aGuard( m_aMutex );
	::connectivity::checkDisposed(!m_xRowUpdate.is());

	m_xRowUpdate->updateInt(m_nPos, x);
}

//------------------------------------------------------------------------------
void ODataColumn::updateLong(sal_Int64 x) throw( SQLException, RuntimeException )
{
	MutexGuard aGuard( m_aMutex );
	::connectivity::checkDisposed(!m_xRowUpdate.is());

	m_xRowUpdate->updateLong(m_nPos, x);
}

//------------------------------------------------------------------------------
void ODataColumn::updateFloat(float x) throw( SQLException, RuntimeException )
{
	MutexGuard aGuard( m_aMutex );
	::connectivity::checkDisposed(!m_xRowUpdate.is());

	m_xRowUpdate->updateFloat(m_nPos, x);
}

//------------------------------------------------------------------------------
void ODataColumn::updateDouble(double x) throw( SQLException, RuntimeException )
{
	MutexGuard aGuard( m_aMutex );
	::connectivity::checkDisposed(!m_xRowUpdate.is());

	m_xRowUpdate->updateDouble(m_nPos, x);
}

//------------------------------------------------------------------------------
void ODataColumn::updateString(const rtl::OUString& x) throw( SQLException, RuntimeException )
{
	MutexGuard aGuard( m_aMutex );
	::connectivity::checkDisposed(!m_xRowUpdate.is());

	m_xRowUpdate->updateString(m_nPos, x);
}

//------------------------------------------------------------------------------
void ODataColumn::updateBytes(const Sequence< sal_Int8 >& x) throw( SQLException, RuntimeException )
{
	MutexGuard aGuard( m_aMutex );
	::connectivity::checkDisposed(!m_xRowUpdate.is());

	m_xRowUpdate->updateBytes(m_nPos, x);
}

//------------------------------------------------------------------------------
void ODataColumn::updateDate(const com::sun::star::util::Date& x) throw( SQLException, RuntimeException )
{
	MutexGuard aGuard( m_aMutex );
	::connectivity::checkDisposed(!m_xRowUpdate.is());

	m_xRowUpdate->updateDate(m_nPos, x);
}

//------------------------------------------------------------------------------
void ODataColumn::updateTime(const ::com::sun::star::util::Time& x) throw( SQLException, RuntimeException )
{
	MutexGuard aGuard( m_aMutex );
	::connectivity::checkDisposed(!m_xRowUpdate.is());

	m_xRowUpdate->updateTime(m_nPos, x);
}

//------------------------------------------------------------------------------
void ODataColumn::updateTimestamp(const ::com::sun::star::util::DateTime& x) throw( SQLException, RuntimeException )
{
	MutexGuard aGuard( m_aMutex );
	::connectivity::checkDisposed(!m_xRowUpdate.is());

	m_xRowUpdate->updateTimestamp(m_nPos, x);
}

//------------------------------------------------------------------------------
void ODataColumn::updateCharacterStream(const Reference< ::com::sun::star::io::XInputStream > & x, sal_Int32 length) throw( SQLException, RuntimeException )
{
	MutexGuard aGuard( m_aMutex );
	::connectivity::checkDisposed(!m_xRowUpdate.is());

	m_xRowUpdate->updateCharacterStream(m_nPos, x, length);
}

//------------------------------------------------------------------------------
void ODataColumn::updateBinaryStream(const Reference< ::com::sun::star::io::XInputStream > & x, sal_Int32 length) throw( SQLException, RuntimeException )
{
	MutexGuard aGuard( m_aMutex );
	::connectivity::checkDisposed(!m_xRowUpdate.is());

	m_xRowUpdate->updateBinaryStream(m_nPos, x, length);
}

//------------------------------------------------------------------------------
void ODataColumn::updateNumericObject(const Any& x, sal_Int32 scale) throw( SQLException, RuntimeException )
{
	MutexGuard aGuard( m_aMutex );
	::connectivity::checkDisposed(!m_xRowUpdate.is());

	m_xRowUpdate->updateNumericObject(m_nPos, x, scale);
}

//------------------------------------------------------------------------------
void ODataColumn::updateObject(const Any& x) throw( SQLException, RuntimeException )
{
	MutexGuard aGuard( m_aMutex );
	::connectivity::checkDisposed(!m_xRowUpdate.is());
	
	m_xRowUpdate->updateObject(m_nPos, x);
}

