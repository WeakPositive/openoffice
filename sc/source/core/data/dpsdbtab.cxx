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
#include "precompiled_sc.hxx"



// INCLUDE --------------------------------------------------------------

#include <tools/debug.hxx>
#include <vcl/msgbox.hxx>
#include <svl/zforlist.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/types.hxx>

#include <com/sun/star/sheet/DataImportMode.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdb/XCompletedExecution.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sheet/DataPilotFieldFilter.hpp>

#include "dpsdbtab.hxx"
#include "collect.hxx"
#include "global.hxx"
#include "globstr.hrc"
#include "dpcachetable.hxx"
#include "dptabres.hxx"
#include "document.hxx"
#include "dpobject.hxx"

using namespace com::sun::star;

using ::std::vector;
using ::std::hash_map;
using ::std::hash_set;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::UNO_QUERY;

#define SC_SERVICE_ROWSET			"com.sun.star.sdb.RowSet"
#define SC_SERVICE_INTHANDLER		"com.sun.star.task.InteractionHandler"

//!	move to a header file?
#define SC_DBPROP_DATASOURCENAME	"DataSourceName"
#define SC_DBPROP_COMMAND			"Command"
#define SC_DBPROP_COMMANDTYPE		"CommandType"
// -----------------------------------------------------------------------
// Wang Xu Ming -- 2009-9-15
// DataPilot Migration - Cache&&Performance
 ScDPTableDataCache* ScImportSourceDesc::GetExistDPObjectCache( ScDocument* pDoc ) const
{
    ScDPTableDataCache* pCache = NULL;
    ScDPCollection* pDPCollection= pDoc->GetDPCollection();
    sal_uInt16 nCount = pDPCollection->GetCount();
    
    for ( short i=nCount-1; i>=0 ; i--)
    {
        if ( const ScImportSourceDesc* pUsedDesc = (*pDPCollection)[i]->GetImportSourceDesc() )
            if ( *this == *pUsedDesc )
            {
                long nID = (*pDPCollection)[i]->GetCacheId();
                if ( nID >= 0  )
                    pCache= pDoc->GetDPObjectCache( nID );
                if ( pCache )
                    return pCache;
            }
    }
    return NULL;
}

ScDPTableDataCache* ScImportSourceDesc::CreateCache( ScDocument* pDoc , long nID  ) const
{
    if ( !pDoc )
		return NULL;

    sal_Int32 nSdbType = -1;
  
    switch ( nType )
    {
    case sheet::DataImportMode_SQL:        nSdbType = sdb::CommandType::COMMAND;  break;
    case sheet::DataImportMode_TABLE:   nSdbType = sdb::CommandType::TABLE;      break;
    case sheet::DataImportMode_QUERY:  nSdbType = sdb::CommandType::QUERY;     break;
    default:
        return NULL;
    }


   ScDPTableDataCache* pCache = GetExistDPObjectCache( pDoc );

    if ( pCache && ( nID < 0 || nID == pCache->GetId() ) )
        return pCache;

    if ( pCache == NULL )
		pCache = new ScDPTableDataCache( pDoc );
	
    uno::Reference<sdbc::XRowSet> xRowSet ;
    try
    {
        xRowSet = uno::Reference<sdbc::XRowSet>(
            comphelper::getProcessServiceFactory()->createInstance(
            rtl::OUString::createFromAscii( SC_SERVICE_ROWSET ) ),
            uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xRowProp( xRowSet, uno::UNO_QUERY );
        DBG_ASSERT( xRowProp.is(), "can't get RowSet" );
        if ( xRowProp.is() )
        {
            //
            //  set source parameters
            //
            uno::Any aAny;
            aAny <<= rtl::OUString( aDBName );
            xRowProp->setPropertyValue(
                rtl::OUString::createFromAscii(SC_DBPROP_DATASOURCENAME), aAny );

            aAny <<= rtl::OUString( aObject );
            xRowProp->setPropertyValue(
                rtl::OUString::createFromAscii(SC_DBPROP_COMMAND), aAny );

            aAny <<= nSdbType;
            xRowProp->setPropertyValue(
                rtl::OUString::createFromAscii(SC_DBPROP_COMMANDTYPE), aAny );

            uno::Reference<sdb::XCompletedExecution> xExecute( xRowSet, uno::UNO_QUERY );
            if ( xExecute.is() )
            {
                uno::Reference<task::XInteractionHandler> xHandler(
                    comphelper::getProcessServiceFactory()->createInstance(
                    rtl::OUString::createFromAscii( SC_SERVICE_INTHANDLER ) ),
                    uno::UNO_QUERY);
                xExecute->executeWithCompletion( xHandler );
            }
            else
                xRowSet->execute();
            SvNumberFormatter aFormat( pDoc->GetServiceManager(), ScGlobal::eLnge);
            pCache->InitFromDataBase( xRowSet, *aFormat.GetNullDate() );
            pCache->SetId( nID );
            pDoc->AddDPObjectCache( pCache );
            DBG_TRACE1("Create a cache id = %d \n", pCache->GetId() );
        }
    }
    catch ( sdbc::SQLException& rError )
    {
        //! store error message
        delete pCache;
        pCache = NULL;
        InfoBox aInfoBox( 0, String(rError.Message) );
        aInfoBox.Execute();
    }
    catch ( uno::Exception& )
    {
        delete pCache;
        pCache = NULL;
        DBG_ERROR("Unexpected exception in database");
    }


    ::comphelper::disposeComponent( xRowSet );
     return pCache;
}

ScDPTableDataCache* ScImportSourceDesc::GetCache( ScDocument* pDoc, long nID ) const
{
    ScDPTableDataCache* pCache = pDoc->GetDPObjectCache( nID );
    if ( NULL == pCache && pDoc )
        pCache = GetExistDPObjectCache( pDoc);
    if ( NULL == pCache )
        pCache = CreateCache( pDoc , nID );    
    return pCache;
}

long ScImportSourceDesc:: GetCacheId( ScDocument* pDoc, long nID ) const
{
	ScDPTableDataCache* pCache = GetCache( pDoc,  nID);
	if ( NULL == pCache )
		return -1;
	else 
		return pCache->GetId();
}

// -----------------------------------------------------------------------

ScDatabaseDPData::ScDatabaseDPData(
    ScDocument* pDoc,
    const ScImportSourceDesc& rImport, long nCacheId /*=-1 */ ) :
    ScDPTableData(pDoc, rImport.GetCacheId( pDoc, nCacheId) ),
    aCacheTable( pDoc, GetCacheId() )     // base class ID is initialized with the GetCacheId call above
{

}

ScDatabaseDPData::~ScDatabaseDPData()
{
}

void ScDatabaseDPData::DisposeData()
{
	//!	use OpenDatabase here?
     aCacheTable.clear();
}

long ScDatabaseDPData::GetColumnCount()
{
    CreateCacheTable();
    return GetCacheTable().getColSize();
}

// End Comments

String ScDatabaseDPData::getDimensionName(long nColumn)
{
	if (getIsDataLayoutDimension(nColumn))
	{
		//!	different internal and display names?
		//return "Data";
		return ScGlobal::GetRscString(STR_PIVOT_DATA);
	}

    CreateCacheTable();
	return aCacheTable.getFieldName((SCCOL)nColumn);
}

sal_Bool ScDatabaseDPData::getIsDataLayoutDimension(long nColumn)
{
	return ( nColumn == GetCacheTable().getColSize());
}

sal_Bool ScDatabaseDPData::IsDateDimension(long /* nDim */)
{
	//!	later...
	return sal_False;
}

void ScDatabaseDPData::SetEmptyFlags( sal_Bool /* bIgnoreEmptyRows */, sal_Bool /* bRepeatIfEmpty */ )
{
	//	not used for database data
	//!	disable flags
}

void ScDatabaseDPData::CreateCacheTable()
{
    if (!aCacheTable.empty())
        return;

    aCacheTable.fillTable();
}

void ScDatabaseDPData::FilterCacheTable(const vector<ScDPCacheTable::Criterion>& rCriteria, const hash_set<sal_Int32>& rCatDims)
{
    CreateCacheTable();
    aCacheTable.filterByPageDimension(
        rCriteria, (IsRepeatIfEmpty() ? rCatDims : hash_set<sal_Int32>()));
}

void ScDatabaseDPData::GetDrillDownData(const vector<ScDPCacheTable::Criterion>& rCriteria, const hash_set<sal_Int32>& rCatDims, Sequence< Sequence<Any> >& rData)
{
    CreateCacheTable();
    sal_Int32 nRowSize = aCacheTable.getRowSize();
    if (!nRowSize)
        return;

    aCacheTable.filterTable(
        rCriteria, rData, IsRepeatIfEmpty() ? rCatDims : hash_set<sal_Int32>());
}

void ScDatabaseDPData::CalcResults(CalcInfo& rInfo, bool bAutoShow)
{
    CreateCacheTable();
    CalcResultsFromCacheTable( aCacheTable, rInfo, bAutoShow);
}

const ScDPCacheTable& ScDatabaseDPData::GetCacheTable() const
{
    return aCacheTable;
}

// -----------------------------------------------------------------------





