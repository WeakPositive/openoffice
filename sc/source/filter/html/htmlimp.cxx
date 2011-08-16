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



//------------------------------------------------------------------------

#include "scitems.hxx"
#include <editeng/eeitem.hxx>

#include <editeng/lrspitem.hxx>
#include <editeng/paperinf.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/boxitem.hxx>
#include <vcl/svapp.hxx>

#include "htmlimp.hxx"
#include "htmlpars.hxx"
#include "filter.hxx"
#include "global.hxx"
#include "document.hxx"
#include "editutil.hxx"
#include "stlpool.hxx"
#include "stlsheet.hxx"
#include "compiler.hxx"
#include "rangenam.hxx"
#include "attrib.hxx"
#include "ftools.hxx"
#include "tokenarray.hxx"


//------------------------------------------------------------------------

FltError ScFormatFilterPluginImpl::ScImportHTML( SvStream &rStream, const String& rBaseURL, ScDocument *pDoc,
		ScRange& rRange, double nOutputFactor, sal_Bool bCalcWidthHeight, SvNumberFormatter* pFormatter,
        bool bConvertDate )
{
    ScHTMLImport aImp( pDoc, rBaseURL, rRange, bCalcWidthHeight );
    FltError nErr = (FltError) aImp.Read( rStream, rBaseURL );
	ScRange aR = aImp.GetRange();
	rRange.aEnd = aR.aEnd;
	aImp.WriteToDocument( sal_True, nOutputFactor, pFormatter, bConvertDate );
	return nErr;
}

ScEEAbsImport *ScFormatFilterPluginImpl::CreateHTMLImport( ScDocument* pDocP, const String& rBaseURL, const ScRange& rRange, sal_Bool bCalcWidthHeight )
{
    return new ScHTMLImport( pDocP, rBaseURL, rRange, bCalcWidthHeight );
}

ScHTMLImport::ScHTMLImport( ScDocument* pDocP, const String& rBaseURL, const ScRange& rRange, sal_Bool bCalcWidthHeight ) :
	ScEEImport( pDocP, rRange )
{
	Size aPageSize;
	OutputDevice* pDefaultDev = Application::GetDefaultDevice();
    const String& aPageStyle = mpDoc->GetPageStyle( rRange.aStart.Tab() );
    ScStyleSheet* pStyleSheet = (ScStyleSheet*)mpDoc->
		GetStyleSheetPool()->Find( aPageStyle, SFX_STYLE_FAMILY_PAGE );
	if ( pStyleSheet )
	{
		const SfxItemSet& rSet = pStyleSheet->GetItemSet();
		const SvxLRSpaceItem* pLRItem = (const SvxLRSpaceItem*) &rSet.Get( ATTR_LRSPACE );
		long nLeftMargin   = pLRItem->GetLeft();
		long nRightMargin  = pLRItem->GetRight();
		const SvxULSpaceItem* pULItem = (const SvxULSpaceItem*) &rSet.Get( ATTR_ULSPACE );
		long nTopMargin    = pULItem->GetUpper();
		long nBottomMargin = pULItem->GetLower();
		aPageSize = ((const SvxSizeItem&) rSet.Get(ATTR_PAGE_SIZE)).GetSize();
		if ( !aPageSize.Width() || !aPageSize.Height() )
		{
			DBG_ERRORFILE("PageSize Null ?!?!?");
			aPageSize = SvxPaperInfo::GetPaperSize( PAPER_A4 );
		}
		aPageSize.Width() -= nLeftMargin + nRightMargin;
		aPageSize.Height() -= nTopMargin + nBottomMargin;
		aPageSize = pDefaultDev->LogicToPixel( aPageSize, MapMode( MAP_TWIP ) );
	}
	else
	{
		DBG_ERRORFILE("kein StyleSheet?!?");
		aPageSize = pDefaultDev->LogicToPixel(
			SvxPaperInfo::GetPaperSize( PAPER_A4 ), MapMode( MAP_TWIP ) );
	}
    if( bCalcWidthHeight )
        mpParser = new ScHTMLLayoutParser( mpEngine, rBaseURL, aPageSize, pDocP );
    else
        mpParser = new ScHTMLQueryParser( mpEngine, pDocP );
}


ScHTMLImport::~ScHTMLImport()
{
	// Reihenfolge wichtig, sonst knallt's irgendwann irgendwo in irgendeinem Dtor!
	// Ist gewaehrleistet, da ScEEImport Basisklasse ist
    delete (ScHTMLParser*) mpParser;        // vor EditEngine!
}


void ScHTMLImport::InsertRangeName( ScDocument* pDoc, const String& rName, const ScRange& rRange )
{
	ScComplexRefData aRefData;
	aRefData.InitRange( rRange );
	ScTokenArray aTokArray;
	aTokArray.AddDoubleReference( aRefData );
	ScRangeData* pRangeData = new ScRangeData( pDoc, rName, aTokArray );
	if( !pDoc->GetRangeName()->Insert( pRangeData ) )
		delete pRangeData;
}

void ScHTMLImport::WriteToDocument( 
    sal_Bool bSizeColsRows, double nOutputFactor, SvNumberFormatter* pFormatter, bool bConvertDate )
{
	ScEEImport::WriteToDocument( bSizeColsRows, nOutputFactor, pFormatter, bConvertDate );

    const ScHTMLParser* pParser = GetParser();
    const ScHTMLTable* pGlobTable = pParser->GetGlobalTable();
    if( !pGlobTable )
		return;

	// set cell borders for HTML table cells
    pGlobTable->ApplyCellBorders( mpDoc, maRange.aStart );

	// correct cell borders for merged cells
	for ( ScEEParseEntry* pEntry = pParser->First(); pEntry; pEntry = pParser->Next() )
	{
		if( (pEntry->nColOverlap > 1) || (pEntry->nRowOverlap > 1) )
		{
            SCTAB nTab = maRange.aStart.Tab();
            const ScMergeAttr* pItem = (ScMergeAttr*) mpDoc->GetAttr( pEntry->nCol, pEntry->nRow, nTab, ATTR_MERGE );
			if( pItem->IsMerged() )
			{
				SCCOL nColMerge = pItem->GetColMerge();
				SCROW nRowMerge = pItem->GetRowMerge();

				const SvxBoxItem* pToItem = (const SvxBoxItem*)
                    mpDoc->GetAttr( pEntry->nCol, pEntry->nRow, nTab, ATTR_BORDER );
				SvxBoxItem aNewItem( *pToItem );
				if( nColMerge > 1 )
				{
					const SvxBoxItem* pFromItem = (const SvxBoxItem*)
                        mpDoc->GetAttr( pEntry->nCol + nColMerge - 1, pEntry->nRow, nTab, ATTR_BORDER );
					aNewItem.SetLine( pFromItem->GetLine( BOX_LINE_RIGHT ), BOX_LINE_RIGHT );
				}
				if( nRowMerge > 1 )
				{
					const SvxBoxItem* pFromItem = (const SvxBoxItem*)
                        mpDoc->GetAttr( pEntry->nCol, pEntry->nRow + nRowMerge - 1, nTab, ATTR_BORDER );
					aNewItem.SetLine( pFromItem->GetLine( BOX_LINE_BOTTOM ), BOX_LINE_BOTTOM );
				}
                mpDoc->ApplyAttr( pEntry->nCol, pEntry->nRow, nTab, aNewItem );
			}
		}
	}

	// create ranges for HTML tables
 	// 1 - entire document
    ScRange aNewRange( maRange.aStart );
    aNewRange.aEnd.IncCol( static_cast<SCsCOL>(pGlobTable->GetDocSize( tdCol )) - 1 );
    aNewRange.aEnd.IncRow( pGlobTable->GetDocSize( tdRow ) - 1 );
    InsertRangeName( mpDoc, ScfTools::GetHTMLDocName(), aNewRange );

	// 2 - all tables
    InsertRangeName( mpDoc, ScfTools::GetHTMLTablesName(), ScRange( maRange.aStart ) );

	// 3 - single tables
    SCsCOL nColDiff = (SCsCOL)maRange.aStart.Col();
    SCsROW nRowDiff = (SCsROW)maRange.aStart.Row();
    SCsTAB nTabDiff = (SCsTAB)maRange.aStart.Tab();

    ScHTMLTable* pTable = NULL;
    ScHTMLTableId nTableId = SC_HTML_GLOBAL_TABLE;
    while( (pTable = pGlobTable->FindNestedTable( ++nTableId )) != 0 )
	{
        pTable->GetDocRange( aNewRange );
		aNewRange.Move( nColDiff, nRowDiff, nTabDiff );
		// insert table number as name
        InsertRangeName( mpDoc, ScfTools::GetNameFromHTMLIndex( nTableId ), aNewRange );
		// insert table id as name
		if( pTable->GetTableName().Len() )
		{
            String aName( ScfTools::GetNameFromHTMLName( pTable->GetTableName() ) );
			sal_uInt16 nPos;
            if( !mpDoc->GetRangeName()->SearchName( aName, nPos ) )
                InsertRangeName( mpDoc, aName, aNewRange );
		}
	}
}

String ScFormatFilterPluginImpl::GetHTMLRangeNameList( ScDocument* pDoc, const String& rOrigName )
{
    return ScHTMLImport::GetHTMLRangeNameList( pDoc, rOrigName );
}

String ScHTMLImport::GetHTMLRangeNameList( ScDocument* pDoc, const String& rOrigName )
{
	DBG_ASSERT( pDoc, "ScHTMLImport::GetHTMLRangeNameList - missing document" );

	String aNewName;
	ScRangeName* pRangeNames = pDoc->GetRangeName();
	ScRangeList aRangeList;
	xub_StrLen nTokenCnt = rOrigName.GetTokenCount( ';' );
	xub_StrLen nStringIx = 0;
	for( xub_StrLen nToken = 0; nToken < nTokenCnt; nToken++ )
	{
		String aToken( rOrigName.GetToken( 0, ';', nStringIx ) );
        if( pRangeNames && ScfTools::IsHTMLTablesName( aToken ) )
		{	// build list with all HTML tables
			sal_uLong nIndex = 1;
			sal_uInt16 nPos;
			sal_Bool bLoop = sal_True;
			while( bLoop )
			{
                aToken = ScfTools::GetNameFromHTMLIndex( nIndex++ );
				bLoop = pRangeNames->SearchName( aToken, nPos );
				if( bLoop )
				{
					const ScRangeData* pRangeData = (*pRangeNames)[ nPos ];
					ScRange aRange;
					if( pRangeData && pRangeData->IsReference( aRange ) && !aRangeList.In( aRange ) )
					{
                        ScGlobal::AddToken( aNewName, aToken, ';' );
						aRangeList.Append( aRange );
					}
				}
			}
		}
		else
            ScGlobal::AddToken( aNewName, aToken, ';' );
	}
	return aNewName;
}

