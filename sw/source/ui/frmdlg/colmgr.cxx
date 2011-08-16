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
#include "precompiled_sw.hxx"



#include "hintids.hxx"
#include <editeng/lrspitem.hxx>

#include "frmmgr.hxx"
#include "frmfmt.hxx"
#include "colmgr.hxx"


// PRIVATE METHODES ------------------------------------------------------
/*------------------------------------------------------------------------
 Beschreibung:	Spaltenbreite auf aktuelle Breite einstellen
------------------------------------------------------------------------*/



void FitToActualSize(SwFmtCol& rCol, sal_uInt16 nWidth)
{
	const sal_uInt16 nCount = rCol.GetColumns().Count();
	for(sal_uInt16 i = 0; i < nCount; ++i)
	{
		const sal_uInt16 nTmp = rCol.CalcColWidth(i, nWidth);
		rCol.GetColumns()[i]->SetWishWidth(nTmp);
	}
	rCol.SetWishWidth(nWidth);
}


// PUBLIC METHODES -------------------------------------------------------
/*------------------------------------------------------------------------
 Beschreibung:	Setzen Spaltenanzahl und Gutterwidth
------------------------------------------------------------------------*/



void SwColMgr::SetCount(sal_uInt16 nCount, sal_uInt16  nGutterWidth)
{
	aFmtCol.Init(nCount, nGutterWidth, nWidth);
	aFmtCol.SetWishWidth(nWidth);
	aFmtCol.SetGutterWidth(nGutterWidth, nWidth);
}



sal_uInt16 SwColMgr::GetGutterWidth( sal_uInt16 nPos ) const
{
	sal_uInt16 nRet;
	if(nPos == USHRT_MAX )
		nRet = GetCount() > 1 ? aFmtCol.GetGutterWidth() : DEF_GUTTER_WIDTH;
	else
	{
		DBG_ASSERT(nPos < GetCount() - 1, "Spalte ueberindiziert" );
		const SwColumns& rCols = aFmtCol.GetColumns();
		nRet = rCols.GetObject(nPos)->GetRight() + rCols.GetObject(nPos + 1)->GetLeft();
	}
	return nRet;
}

/*-----------------22.10.96 14.28-------------------

--------------------------------------------------*/


void SwColMgr::SetGutterWidth(sal_uInt16 nGutterWidth, sal_uInt16 nPos )
{
	if(nPos == USHRT_MAX)
		aFmtCol.SetGutterWidth(nGutterWidth, nWidth);
	else
	{
		DBG_ASSERT(nPos < GetCount() - 1, "Spalte ueberindiziert" );
		SwColumns& rCols = aFmtCol.GetColumns();
		sal_uInt16 nGutterWidth2 = nGutterWidth / 2;
		rCols.GetObject(nPos)->SetRight(nGutterWidth2);
		rCols.GetObject(nPos + 1)->SetLeft(nGutterWidth2);
	}
}

/*------------------------------------------------------------------------
 Beschreibung:	Hoehe Trennlinie
------------------------------------------------------------------------*/



short SwColMgr::GetLineHeightPercent() const
{
	return (short)aFmtCol.GetLineHeight();
}



void SwColMgr::SetLineHeightPercent(short nPercent)
{
	ASSERT(nPercent <= 100, LineHeight darf nur bis 100 % gross  sein);
	aFmtCol.SetLineHeight((sal_uInt8)nPercent);
}
/*------------------------------------------------------------------------
 Beschreibung:	Spaltenbreite
------------------------------------------------------------------------*/



sal_uInt16 SwColMgr::GetColWidth(sal_uInt16 nIdx) const
{
	ASSERT(nIdx < GetCount(), Spaltenarray ueberindiziert.);
	return aFmtCol.CalcPrtColWidth(nIdx, nWidth);
}



void SwColMgr::SetColWidth(sal_uInt16 nIdx, sal_uInt16 nWd)
{
	ASSERT(nIdx < GetCount(), Spaltenarray ueberindiziert.);
	aFmtCol.GetColumns()[nIdx]->SetWishWidth(nWd);

}

/*--------------------------------------------------------------------
	Beschreibung: 	Groesse neu setzen
 --------------------------------------------------------------------*/



void SwColMgr::SetActualWidth(sal_uInt16 nW)
{
	nWidth = nW;
	::FitToActualSize(aFmtCol, nW);
}

/*--------------------------------------------------------------------
	Beschreibung: ctor
 --------------------------------------------------------------------*/



SwColMgr::SwColMgr(const SfxItemSet& rSet, sal_uInt16 nActWidth) :
	aFmtCol((const SwFmtCol&)rSet.Get(RES_COL)),
	nWidth(nActWidth)
{
	if(nWidth == USHRT_MAX)
	{
		nWidth = (sal_uInt16)((const SwFmtFrmSize&)rSet.Get(RES_FRM_SIZE)).GetWidth();
		if (nWidth < MINLAY)
			nWidth = USHRT_MAX;
		const SvxLRSpaceItem &rLR = (const SvxLRSpaceItem&)rSet.Get(RES_LR_SPACE);
        nWidth = nWidth - (sal_uInt16)rLR.GetLeft();
        nWidth = nWidth - (sal_uInt16)rLR.GetRight();
	}
	::FitToActualSize(aFmtCol, nWidth);
}




SwColMgr::~SwColMgr() {}






