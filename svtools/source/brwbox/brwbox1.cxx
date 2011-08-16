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
#include "precompiled_svtools.hxx"
#include <svtools/brwbox.hxx>
#include <svtools/brwhead.hxx>
#include "datwin.hxx"
#include <tools/debug.hxx>
#include <tools/stream.hxx>

#include <functional>
#include <algorithm>
#include <com/sun/star/accessibility/AccessibleTableModelChange.hpp>
#include <com/sun/star/accessibility/AccessibleTableModelChangeType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <tools/multisel.hxx>
#include "brwimpl.hxx"

DBG_NAME(BrowseBox)

extern const char* BrowseBoxCheckInvariants( const void* pVoid );

DECLARE_LIST( BrowserColumns, BrowserColumn* )

#define SCROLL_FLAGS (SCROLL_CLIP | SCROLL_NOCHILDREN)
#define getDataWindow() ((BrowserDataWin*)pDataWin)

using namespace com::sun::star::accessibility::AccessibleEventId;
using namespace com::sun::star::accessibility::AccessibleTableModelChangeType;
using com::sun::star::accessibility::AccessibleTableModelChange;
using com::sun::star::lang::XComponent;
using namespace ::com::sun::star::uno;
using namespace svt;

//-------------------------------------------------------------------

#ifdef DBG_MI
void DoLog_Impl( const BrowseBox *pThis, const char *pWhat, const char *pWho )
{
	SvFileStream aLog( "d:\\cursor.log", STREAM_WRITE|STREAM_NOCREATE );
	if ( aLog.IsOpen() )
	{
		aLog.Seek( STREAM_SEEK_TO_END );
		String aEntry( (long) pThis );
		aEntry += "(row=";
		aEntry += pThis->GetCurRow();
		aEntry += "): ";
		aEntry += pWhat;
		aEntry += " from ";
		aEntry += pWho;
		aEntry += " => ";
		aEntry += pThis->GetCursorHideCount();
		aLog.WriteLine( aEntry );
	}
}
#endif

namespace
{
	void disposeAndClearHeaderCell(::svt::BrowseBoxImpl::THeaderCellMap& _rHeaderCell)
	{
		::std::for_each(
						_rHeaderCell.begin(),
						_rHeaderCell.end(),
						::svt::BrowseBoxImpl::THeaderCellMapFunctorDispose()
							);
		_rHeaderCell.clear();
	}
}

//===================================================================

void BrowseBox::ConstructImpl( BrowserMode nMode )
{
	DBG_TRACE1( "BrowseBox: %p->ConstructImpl", this );
	bMultiSelection = sal_False;
	pColSel = 0;
	pDataWin = 0;
	pVScroll = 0;

	pDataWin = new BrowserDataWin( this );
	pCols = new BrowserColumns;
	m_pImpl.reset( new ::svt::BrowseBoxImpl() );

	aGridLineColor = Color( COL_LIGHTGRAY );
	InitSettings_Impl( this );
	InitSettings_Impl( pDataWin );

	bBootstrapped = sal_False;
	nDataRowHeight = 0;
	nTitleLines = 1;
	nFirstCol = 0;
	nTopRow = 0;
	nCurRow = BROWSER_ENDOFSELECTION;
	nCurColId = 0;
	bResizing = sal_False;
	bSelect = sal_False;
	bSelecting = sal_False;
	bScrolling = sal_False;
	bSelectionIsVisible = sal_False;
	bNotToggleSel = sal_False;
	bRowDividerDrag = sal_False;
	bHit = sal_False;
	mbInteractiveRowHeight = sal_False;
	bHideSelect = sal_False;
	bHideCursor = NO_CURSOR_HIDE;
	nRowCount = 0;
	m_bFocusOnlyCursor = sal_True;
	m_aCursorColor = COL_TRANSPARENT;
	m_nCurrentMode = 0;
    nControlAreaWidth = USHRT_MAX;
	uRow.nSel = BROWSER_ENDOFSELECTION;

	aHScroll.SetLineSize(1);
	aHScroll.SetScrollHdl( LINK( this, BrowseBox, ScrollHdl ) );
	aHScroll.SetEndScrollHdl( LINK( this, BrowseBox, EndScrollHdl ) );
	pDataWin->Show();

	SetMode( nMode );
	bSelectionIsVisible = bKeepHighlight;
	bHasFocus = HasChildPathFocus();
	getDataWindow()->nCursorHidden =
				( bHasFocus ? 0 : 1 ) + ( GetUpdateMode() ? 0 : 1 );
	LOG( this, "ConstructImpl", "*" );
}

//-------------------------------------------------------------------

BrowseBox::BrowseBox( Window* pParent, WinBits nBits, BrowserMode nMode )
	:Control( pParent, nBits | WB_3DLOOK )
	,DragSourceHelper( this )
	,DropTargetHelper( this )
	,aHScroll( this, WinBits( WB_HSCROLL ) )
{
	DBG_CTOR( BrowseBox, NULL );
	ConstructImpl( nMode );
}

//-------------------------------------------------------------------

BrowseBox::BrowseBox( Window* pParent, const ResId& rId, BrowserMode nMode )
	:Control( pParent, rId )
	,DragSourceHelper( this )
	,DropTargetHelper( this )
	,aHScroll( this, WinBits(WB_HSCROLL) )
{
	DBG_CTOR( BrowseBox, NULL );
	ConstructImpl(nMode);
}
//-------------------------------------------------------------------

BrowseBox::~BrowseBox()
{
	DBG_DTOR(BrowseBox,BrowseBoxCheckInvariants);
	DBG_TRACE1( "BrowseBox: %p~", this );

	if ( m_pImpl->m_pAccessible )
	{
		disposeAndClearHeaderCell(m_pImpl->m_aColHeaderCellMap);
		disposeAndClearHeaderCell(m_pImpl->m_aRowHeaderCellMap);
		m_pImpl->m_pAccessible->dispose();
	}

	Hide();
	delete getDataWindow()->pHeaderBar;
	delete getDataWindow()->pCornerWin;
	delete pDataWin;
	delete pVScroll;

	// free columns-space
	for ( sal_uInt16 n = 0; n < pCols->Count(); ++n )
		delete pCols->GetObject(n);
	delete pCols;
	delete pColSel;
	if ( bMultiSelection )
		delete uRow.pSel;
}

//-------------------------------------------------------------------

short BrowseBox::GetCursorHideCount() const
{
	return getDataWindow()->nCursorHidden;
}

//-------------------------------------------------------------------

void BrowseBox::DoShowCursor( const char *
#ifdef DBG_MI
pWhoLogs
#endif
)
{
	short nHiddenCount = --getDataWindow()->nCursorHidden;
	if (PaintCursorIfHiddenOnce())
	{
		if (1 == nHiddenCount)
			DrawCursor();
	}
	else
	{
		if (0 == nHiddenCount)
			DrawCursor();
	}
	LOG( this, "DoShowCursor", pWhoLogs );
}

//-------------------------------------------------------------------

void BrowseBox::DoHideCursor( const char *
#ifdef DBG_MI
pWhoLogs
#endif
)
{
	short nHiddenCount = ++getDataWindow()->nCursorHidden;
	if (PaintCursorIfHiddenOnce())
	{
		if (2 == nHiddenCount)
			DrawCursor();
	}
	else
	{
		if (1 == nHiddenCount)
			DrawCursor();
	}
	LOG( this, "DoHideCursor", pWhoLogs );
}

//-------------------------------------------------------------------

void BrowseBox::SetRealRowCount( const String &rRealRowCount )
{
	getDataWindow()->aRealRowCount = rRealRowCount;
}

//-------------------------------------------------------------------

void BrowseBox::SetFont( const Font& rNewFont )
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);
	pDataWin->SetFont( rNewFont );
	ImpGetDataRowHeight();
}

//-------------------------------------------------------------------

sal_uLong BrowseBox::GetDefaultColumnWidth( const String& _rText ) const
{
	return GetDataWindow().GetTextWidth( _rText ) + GetDataWindow().GetTextWidth( '0' ) * 4;
}

//-------------------------------------------------------------------

void BrowseBox::InsertHandleColumn( sal_uLong nWidth )
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	pCols->Insert( new BrowserColumn( 0, Image(), String(), nWidth, GetZoom(), 0 ), (sal_uLong) 0 );
	FreezeColumn( 0 );

	// Headerbar anpassen
	if ( getDataWindow()->pHeaderBar )
	{
		getDataWindow()->pHeaderBar->SetPosSizePixel(
					Point(nWidth, 0),
                    Size( GetOutputSizePixel().Width() - nWidth, GetTitleHeight() )
                    );
	}

	/*if ( getDataWindow()->pHeaderBar )
		getDataWindow()->pHeaderBar->InsertItem( USHRT_MAX - 1,
				"", nWidth, HIB_FIXEDPOS|HIB_FIXED, 0 );*/
	ColumnInserted( 0 );
}

//-------------------------------------------------------------------
void BrowseBox::InsertDataColumn( sal_uInt16 nItemId, const Image& rImage,
		long nWidth, HeaderBarItemBits nBits, sal_uInt16 nPos )
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	pCols->Insert( new BrowserColumn( nItemId, rImage, String(), nWidth, GetZoom(), nBits ),
						   Min( nPos, (sal_uInt16)(pCols->Count()) ) );
	if ( nCurColId == 0 )
		nCurColId = nItemId;
	if ( getDataWindow()->pHeaderBar )
	{
		// Handlecolumn nicht in der Headerbar
		sal_uInt16 nHeaderPos = nPos;
		if (nHeaderPos != HEADERBAR_APPEND && !GetColumnId(0))
			nHeaderPos--;
		getDataWindow()->pHeaderBar->InsertItem(
				nItemId, rImage, nWidth, nBits, nHeaderPos );
	}
	ColumnInserted( nPos );
}

//-------------------------------------------------------------------

void BrowseBox::InsertDataColumn( sal_uInt16 nItemId, const XubString& rText,
		long nWidth, HeaderBarItemBits nBits, sal_uInt16 nPos )
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	pCols->Insert( new BrowserColumn( nItemId, Image(), rText, nWidth, GetZoom(), nBits ),
						   Min( nPos, (sal_uInt16)(pCols->Count()) ) );
	if ( nCurColId == 0 )
		nCurColId = nItemId;

	if ( getDataWindow()->pHeaderBar )
	{
		// Handlecolumn nicht in der Headerbar
		sal_uInt16 nHeaderPos = nPos;
		if (nHeaderPos != HEADERBAR_APPEND && !GetColumnId(0))
			nHeaderPos--;
		getDataWindow()->pHeaderBar->InsertItem(
				nItemId, rText, nWidth, nBits, nHeaderPos );
	}
	ColumnInserted( nPos );
}

//-------------------------------------------------------------------

void BrowseBox::InsertDataColumn( sal_uInt16 nItemId,
		const Image& rImage, const XubString& rText,
		long nWidth, HeaderBarItemBits nBits, sal_uInt16 nPos,
		const String* pHelpText )
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	pCols->Insert( new BrowserColumn( nItemId, rImage, rText, nWidth, GetZoom(), nBits ),
						   Min( nPos, (sal_uInt16)(pCols->Count()) ) );
	if ( nCurColId == 0 )
		nCurColId = nItemId;
	if ( getDataWindow()->pHeaderBar )
	{
		// Handlecolumn nicht in der Headerbar
		sal_uInt16 nHeaderPos = nPos;
		if (nHeaderPos != HEADERBAR_APPEND && !GetColumnId(0))
			nHeaderPos--;

		getDataWindow()->pHeaderBar->InsertItem(
				nItemId, rImage, rText, nWidth, nBits, nHeaderPos );
		if( pHelpText && !rText.Len() )
		{
			getDataWindow()->pHeaderBar->SetHelpText(
				nItemId, *pHelpText );
		}
	}
	ColumnInserted( nPos );
}
//-------------------------------------------------------------------
sal_uInt16 BrowseBox::ToggleSelectedColumn()
{
	sal_uInt16 nSelectedColId = USHRT_MAX;
	if ( pColSel && pColSel->GetSelectCount() )
	{
		DoHideCursor( "ToggleSelectedColumn" );
		ToggleSelection();
		nSelectedColId = pCols->GetObject(pColSel->FirstSelected())->GetId();
		pColSel->SelectAll(sal_False);
	}
	return nSelectedColId;
}
// -----------------------------------------------------------------------------
void BrowseBox::SetToggledSelectedColumn(sal_uInt16 _nSelectedColumnId)
{
	if ( pColSel && _nSelectedColumnId != USHRT_MAX )
	{
		pColSel->Select( GetColumnPos( _nSelectedColumnId ) );
		ToggleSelection();
		DBG_TRACE1( "BrowseBox: %p->SetToggledSelectedColumn", this );
		DoShowCursor( "SetToggledSelectedColumn" );
	}
}
// -----------------------------------------------------------------------------
void BrowseBox::FreezeColumn( sal_uInt16 nItemId, sal_Bool bFreeze )
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	// never unfreeze the handle-column
	if ( nItemId == 0 && !bFreeze )
		return;

	// get the position in the current array
	sal_uInt16 nItemPos = GetColumnPos( nItemId );
	if ( nItemPos >= pCols->Count() )
		// not available!
		return;

	// doesn't the state change?
	if ( pCols->GetObject(nItemPos)->IsFrozen() == bFreeze )
		return;

	// remark the column selection
	sal_uInt16 nSelectedColId = ToggleSelectedColumn();

	// freeze or unfreeze?
	if ( bFreeze )
	{
		// to be moved?
		if ( nItemPos != 0 && !pCols->GetObject(nItemPos-1)->IsFrozen() )
		{
			// move to the right of the last frozen column
			sal_uInt16 nFirstScrollable = FrozenColCount();
			BrowserColumn *pColumn = pCols->GetObject(nItemPos);
			pCols->Remove( (sal_uLong) nItemPos );
			nItemPos = nFirstScrollable;
			pCols->Insert( pColumn, (sal_uLong) nItemPos );
		}

		// adjust the number of the first scrollable and visible column
		if ( nFirstCol <= nItemPos )
			nFirstCol = nItemPos + 1;
	}
	else
	{
		// to be moved?
		if ( nItemPos != FrozenColCount()-1 )
		{
			// move to the leftmost scrollable colum
			sal_uInt16 nFirstScrollable = FrozenColCount();
			BrowserColumn *pColumn = pCols->GetObject(nItemPos);
			pCols->Remove( (sal_uLong) nItemPos );
			nItemPos = nFirstScrollable;
			pCols->Insert( pColumn, (sal_uLong) nItemPos );
		}

		// adjust the number of the first scrollable and visible column
		nFirstCol = nItemPos;
	}

	// toggle the freeze-state of the column
	pCols->GetObject(nItemPos)->Freeze( bFreeze );

	// align the scrollbar-range
	UpdateScrollbars();

	// repaint
	Control::Invalidate();
	getDataWindow()->Invalidate();

	// remember the column selection
	SetToggledSelectedColumn(nSelectedColId);
}

//-------------------------------------------------------------------

void BrowseBox::SetColumnPos( sal_uInt16 nColumnId, sal_uInt16 nPos )
{
	// never set pos of the handle-column
	if ( nColumnId == 0 )
		return;

	// do not move handle column
	if (nPos == 0 && !pCols->GetObject(0)->GetId())
		return;

	// get the position in the current array
	sal_uInt16 nOldPos = GetColumnPos( nColumnId );
	if ( nOldPos >= pCols->Count() )
		// not available!
		return;

	// does the state change?
	if (nOldPos != nPos)
	{
		// remark the column selection
		sal_uInt16 nSelectedColId = ToggleSelectedColumn();

		// determine old column area
		Size aDataWinSize( pDataWin->GetSizePixel() );
		if ( getDataWindow()->pHeaderBar )
			aDataWinSize.Height() += getDataWindow()->pHeaderBar->GetSizePixel().Height();

		Rectangle aFromRect( GetFieldRect( nColumnId) );
		aFromRect.Right() += 2*MIN_COLUMNWIDTH;

		sal_uInt16 nNextPos = nOldPos + 1;
		if ( nOldPos > nPos )
			nNextPos = nOldPos - 1;

		BrowserColumn *pNextCol = pCols->GetObject(nNextPos);
		Rectangle aNextRect(GetFieldRect( pNextCol->GetId() ));

		// move column internally
		pCols->Insert( pCols->Remove( nOldPos ), nPos );

		// determine new column area
		Rectangle aToRect( GetFieldRect( nColumnId ) );
		aToRect.Right() += 2*MIN_COLUMNWIDTH;

		// do scroll, let redraw
		if( pDataWin->GetBackground().IsScrollable() )
		{
			long nScroll = -aFromRect.GetWidth();
			Rectangle aScrollArea;
			if ( nOldPos > nPos )
			{
				long nFrozenWidth = GetFrozenWidth();
				if ( aToRect.Left() < nFrozenWidth )
					aToRect.Left() = nFrozenWidth;
				aScrollArea = Rectangle(Point(aToRect.Left(),0),
									    Point(aNextRect.Right(),aDataWinSize.Height()));
				nScroll *= -1; // reverse direction
			}
			else
				aScrollArea = Rectangle(Point(aNextRect.Left(),0),
									    Point(aToRect.Right(),aDataWinSize.Height()));

			pDataWin->Scroll( nScroll, 0, aScrollArea );
			aToRect.Top() = 0;
			aToRect.Bottom() = aScrollArea.Bottom();
			Invalidate( aToRect );
		}
		else
			pDataWin->Window::Invalidate( INVALIDATE_NOCHILDREN );

		// adjust header bar positions
		if ( getDataWindow()->pHeaderBar )
		{
			sal_uInt16 nNewPos = nPos;
			if ( !GetColumnId(0) )
				--nNewPos;
			getDataWindow()->pHeaderBar->MoveItem(nColumnId,nNewPos);
		}
		// remember the column selection
		SetToggledSelectedColumn(nSelectedColId);

        if ( isAccessibleAlive() )
		{
			commitTableEvent(
                TABLE_MODEL_CHANGED,
				makeAny( AccessibleTableModelChange(
                            DELETE,
							0,
							GetRowCount(),
							nOldPos,
							nOldPos
                        )
                ),
                Any()
            );

			commitTableEvent(
                TABLE_MODEL_CHANGED,
			    makeAny( AccessibleTableModelChange(
                            INSERT,
							0,
							GetRowCount(),
							nPos,
							nPos
                        )
                ),
				Any()
            );
		}
	}

}

//-------------------------------------------------------------------

void BrowseBox::SetColumnMode( sal_uInt16 nColumnId, BrowserColumnMode nFlags )
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	// never set mode of the handle-column
	if ( nColumnId == 0 )
		return;

	// get the position in the current array
	sal_uInt16 nColumnPos = GetColumnPos( nColumnId );
	if ( nColumnPos >= pCols->Count() )
		// not available!
		return;

	// does the state change?
	BrowserColumn *pCol = pCols->GetObject(nColumnPos);
	if ( pCol->Flags() != nFlags )
	{
		pCol->Flags() = sal::static_int_cast< HeaderBarItemBits >(nFlags);

		// redraw visible colums
		if ( GetUpdateMode() && ( pCol->IsFrozen() || nColumnPos > nFirstCol ) )
			Invalidate( Rectangle( Point(0,0),
				Size( GetOutputSizePixel().Width(), GetTitleHeight() ) ) );
	}
}

//-------------------------------------------------------------------

void BrowseBox::SetColumnTitle( sal_uInt16 nItemId, const String& rTitle )
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	// never set title of the handle-column
	if ( nItemId == 0 )
		return;

	// get the position in the current array
	sal_uInt16 nItemPos = GetColumnPos( nItemId );
	if ( nItemPos >= pCols->Count() )
		// not available!
		return;

	// does the state change?
	BrowserColumn *pCol = pCols->GetObject(nItemPos);
	if ( pCol->Title() != rTitle )
	{
		::rtl::OUString sNew(rTitle);
		::rtl::OUString sOld(pCol->Title());

		pCol->Title() = rTitle;

		// Headerbar-Column anpassen
		if ( getDataWindow()->pHeaderBar )
			getDataWindow()->pHeaderBar->SetItemText(
					nItemId ? nItemId : USHRT_MAX - 1, rTitle );
		else
		{
			// redraw visible colums
			if ( GetUpdateMode() && ( pCol->IsFrozen() || nItemPos > nFirstCol ) )
				Invalidate( Rectangle( Point(0,0),
					Size( GetOutputSizePixel().Width(), GetTitleHeight() ) ) );
		}

        if ( isAccessibleAlive() )
		{
			commitTableEvent(	TABLE_COLUMN_DESCRIPTION_CHANGED,
			    makeAny( sNew ),
			    makeAny( sOld )
            );
		}
	}
}

//-------------------------------------------------------------------

void BrowseBox::SetColumnWidth( sal_uInt16 nItemId, sal_uLong nWidth )
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	// get the position in the current array
	sal_uInt16 nItemPos = GetColumnPos( nItemId );
	if ( nItemPos >= pCols->Count() )
		return;

	// does the state change?
	nWidth = QueryColumnResize( nItemId, nWidth );
	if ( nWidth >= LONG_MAX || pCols->GetObject(nItemPos)->Width() != nWidth )
	{
		long nOldWidth = pCols->GetObject(nItemPos)->Width();

		// ggf. letzte Spalte anpassen
		if ( IsVisible() && nItemPos == pCols->Count() - 1 )
		{
			long nMaxWidth = pDataWin->GetSizePixel().Width();
			nMaxWidth -= getDataWindow()->bAutoSizeLastCol
					? GetFieldRect(nItemId).Left()
					: GetFrozenWidth();
			if ( ( (BrowserDataWin*)pDataWin )->bAutoSizeLastCol || nWidth > (sal_uLong)nMaxWidth )
			{
				nWidth = nMaxWidth > 16 ? nMaxWidth : nOldWidth;
				nWidth = QueryColumnResize( nItemId, nWidth );
			}
		}

		// OV
		// In AutoSizeLastColumn() wird SetColumnWidth mit nWidth==0xffff
		// gerufen. Deshalb muss hier nochmal geprueft werden, ob sich die
		// Breite tatsaechlich geaendert hat.
		if( (sal_uLong)nOldWidth == nWidth )
			return;

		// soll die Aenderung sofort dargestellt werden?
		sal_Bool bUpdate = GetUpdateMode() &&
					   ( pCols->GetObject(nItemPos)->IsFrozen() || nItemPos >= nFirstCol );

		if ( bUpdate )
		{
			// Selection hiden
			DoHideCursor( "SetColumnWidth" );
			ToggleSelection();
			//!getDataWindow()->Update();
			//!Control::Update();
		}

		// Breite setzen
		pCols->GetObject(nItemPos)->SetWidth(nWidth, GetZoom());
#if 0
		if ( nItemPos != pCols->Count() - 1 )
		{
			long nLastColMaxWidth = pDataWin->GetSizePixel().Width() -
						GetFieldRect(GetColumnId(pCols->Count()-1)).Left();
			pCols->GetObject(pCols->Count()-1)->Width() = nLastColMaxWidth;
		}
#endif

		// scroll and invalidate
		if ( bUpdate )
		{
			// X-Pos der veraenderten Spalte ermitteln
			long nX = 0;
			for ( sal_uInt16 nCol = 0; nCol < nItemPos; ++nCol )
			{
				BrowserColumn *pCol = pCols->GetObject(nCol);
				if ( pCol->IsFrozen() || nCol >= nFirstCol )
					nX += pCol->Width();
			}

			// eigentliches scroll+invalidate
			pDataWin->SetClipRegion();
			sal_Bool bSelVis = bSelectionIsVisible;
			bSelectionIsVisible = sal_False;
			if( GetBackground().IsScrollable() )
			{

				Rectangle aScrRect( nX + std::min( (sal_uLong)nOldWidth, nWidth ), 0,
									GetSizePixel().Width() , // the header is longer than the datawin
									pDataWin->GetPosPixel().Y() - 1 );
				Control::Scroll( nWidth-nOldWidth, 0, aScrRect, SCROLL_FLAGS );
				aScrRect.Bottom() = pDataWin->GetSizePixel().Height();
				getDataWindow()->Scroll( nWidth-nOldWidth, 0, aScrRect, SCROLL_FLAGS );
				Rectangle aInvRect( nX, 0, nX + std::max( nWidth, (sal_uLong)nOldWidth ), USHRT_MAX );
				Control::Invalidate( aInvRect, INVALIDATE_NOCHILDREN );
				( (BrowserDataWin*)pDataWin )->Invalidate( aInvRect );
			}
			else
			{
				Control::Invalidate( INVALIDATE_NOCHILDREN );
				getDataWindow()->Window::Invalidate( INVALIDATE_NOCHILDREN );
			}


			//!getDataWindow()->Update();
			//!Control::Update();
			bSelectionIsVisible = bSelVis;
			ToggleSelection();
			DoShowCursor( "SetColumnWidth" );
		}
		UpdateScrollbars();

		// Headerbar-Column anpassen
		if ( getDataWindow()->pHeaderBar )
			getDataWindow()->pHeaderBar->SetItemSize(
					nItemId ? nItemId : USHRT_MAX - 1, nWidth );

		// adjust last column
		if ( nItemPos != pCols->Count() - 1 )
			AutoSizeLastColumn();

	}
}

//-------------------------------------------------------------------

void BrowseBox::AutoSizeLastColumn()
{
	if ( getDataWindow()->bAutoSizeLastCol &&
		 getDataWindow()->GetUpdateMode() )
	{
		sal_uInt16 nId = GetColumnId( (sal_uInt16)pCols->Count() - 1 );
		SetColumnWidth( nId, LONG_MAX );
		ColumnResized( nId );
	}
}

//-------------------------------------------------------------------

void BrowseBox::RemoveColumn( sal_uInt16 nItemId )
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	// Spaltenposition ermitteln
	sal_uInt16 nPos = GetColumnPos(nItemId);
	if ( nPos >= ColCount() )
		// nicht vorhanden
		return;

	// Spaltenselektion korrigieren
	if ( pColSel )
		pColSel->Remove( nPos );

	// Spaltencursor korrigieren
	if ( nCurColId == nItemId )
		nCurColId = 0;

	// Spalte entfernen
	delete( pCols->Remove( (sal_uLong) nPos ));
	// OJ #93534#
	if ( nFirstCol >= nPos && nFirstCol > FrozenColCount() )
	{
		OSL_ENSURE(nFirstCol > 0,"FirstCol must be greater zero!");
		--nFirstCol;
	}

	// Handlecolumn nicht in der Headerbar
	if (nItemId)
	{
		if ( getDataWindow()->pHeaderBar )
			getDataWindow()->pHeaderBar->RemoveItem( nItemId );
	}
	else
	{
		// Headerbar anpassen
		if ( getDataWindow()->pHeaderBar )
		{
			getDataWindow()->pHeaderBar->SetPosSizePixel(
						Point(0, 0),
                        Size( GetOutputSizePixel().Width(), GetTitleHeight() )
                        );
		}
	}

	// vertikalen Scrollbar korrigieren
	UpdateScrollbars();

	// ggf. Repaint ausl"osen
	if ( GetUpdateMode() )
	{
		getDataWindow()->Invalidate();
		Control::Invalidate();
		if ( getDataWindow()->bAutoSizeLastCol && nPos ==ColCount() )
			SetColumnWidth( GetColumnId( nPos - 1 ), LONG_MAX );
	}

    if ( isAccessibleAlive() )
	{
		commitTableEvent(
            TABLE_MODEL_CHANGED,
			makeAny( AccessibleTableModelChange(	DELETE,
													0,
													GetRowCount(),
													nPos,
													nPos
                                               )
            ),
            Any()
        );

		commitHeaderBarEvent(
            CHILD,
			Any(),
			makeAny( CreateAccessibleColumnHeader( nPos ) ),
            sal_True
        );
	}
}

//-------------------------------------------------------------------

void BrowseBox::RemoveColumns()
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	unsigned int nOldCount = pCols->Count();
	// alle Spalten entfernen
	while ( pCols->Count() )
		delete ( pCols->Remove( (sal_uLong) 0 ));

	// Spaltenselektion korrigieren
	if ( pColSel )
	{
		pColSel->SelectAll(sal_False);
		pColSel->SetTotalRange( Range( 0, 0 ) );
	}

	// Spaltencursor korrigieren
	nCurColId = 0;
	nFirstCol = 0;

	if ( getDataWindow()->pHeaderBar )
		getDataWindow()->pHeaderBar->Clear( );

	// vertikalen Scrollbar korrigieren
	UpdateScrollbars();

	// ggf. Repaint ausl"osen
	if ( GetUpdateMode() )
	{
		getDataWindow()->Invalidate();
		Control::Invalidate();
	}

    if ( isAccessibleAlive() )
	{
        if ( pCols->Count() != nOldCount )
        {
		    // all columns should be removed, so we remove the column header bar and append it again
		    // to avoid to notify every column remove
		    commitBrowseBoxEvent(
                CHILD,
                Any(),
                makeAny(m_pImpl->getAccessibleHeaderBar(BBTYPE_COLUMNHEADERBAR))
            );

            // and now append it again
		    commitBrowseBoxEvent(
                CHILD,
				makeAny(m_pImpl->getAccessibleHeaderBar(BBTYPE_COLUMNHEADERBAR)),
				Any()
            );

            // notify a table model change
            commitTableEvent(
                TABLE_MODEL_CHANGED,
				makeAny ( AccessibleTableModelChange( DELETE,
                                0,
					            GetRowCount(),
					            0,
					            nOldCount
                            )
                        ),
				Any()
            );
        }
	}
}

//-------------------------------------------------------------------

String BrowseBox::GetColumnTitle( sal_uInt16 nId ) const
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	sal_uInt16 nItemPos = GetColumnPos( nId );
	if ( nItemPos >= pCols->Count() )
		return String();
	return pCols->GetObject(nItemPos)->Title();
}

//-------------------------------------------------------------------

long BrowseBox::GetRowCount() const
{
	return nRowCount;
}

//-------------------------------------------------------------------

sal_uInt16 BrowseBox::ColCount() const
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	return (sal_uInt16) pCols->Count();
}

//-------------------------------------------------------------------

long BrowseBox::ImpGetDataRowHeight() const
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	BrowseBox *pThis = (BrowseBox*)this;
	pThis->nDataRowHeight = pThis->CalcReverseZoom(pDataWin->GetTextHeight() + 2);
	pThis->Resize();
	getDataWindow()->Invalidate();
	return nDataRowHeight;
}

//-------------------------------------------------------------------

void BrowseBox::SetDataRowHeight( long nPixel )
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	nDataRowHeight = CalcReverseZoom(nPixel);
	Resize();
	getDataWindow()->Invalidate();
}

//-------------------------------------------------------------------

void BrowseBox::SetTitleLines( sal_uInt16 nLines )
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	nTitleLines = nLines;
}

//-------------------------------------------------------------------

long BrowseBox::ScrollColumns( long nCols )
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	if ( nFirstCol + nCols < 0 ||
		 nFirstCol + nCols >= (long)pCols->Count() )
		 //?MI: pCols->GetObject( nFirstCol + nCols )->IsFrozen() )
		return 0;

	// implicitly hides cursor while scrolling
	StartScroll();
	bScrolling = sal_True;
	sal_Bool bScrollable = pDataWin->GetBackground().IsScrollable();
	sal_Bool bInvalidateView = sal_False;

	// scrolling one column to the right?
	if ( nCols == 1 )
	{
		// update internal value and scrollbar
		++nFirstCol;
		aHScroll.SetThumbPos( nFirstCol - FrozenColCount() );

		if ( !bScrollable )
        {
            bInvalidateView = sal_True;
        }
        else
        {
		    long nDelta = pCols->GetObject(nFirstCol-1)->Width();
		    long nFrozenWidth = GetFrozenWidth();

            Rectangle aScrollRect(  Point( nFrozenWidth + nDelta, 0 ),
                                    Size ( GetOutputSizePixel().Width() - nFrozenWidth - nDelta,
                                           GetTitleHeight() - 1
                                         ) );

		    // scroll the header bar area (if there is no dedicated HeaderBar control)
		    if ( !getDataWindow()->pHeaderBar && nTitleLines )
		    {
                // actually scroll
                Scroll( -nDelta, 0, aScrollRect, SCROLL_FLAGS );

                // invalidate the area of the column which was scrolled out to the left hand side
                Rectangle aInvalidateRect( aScrollRect );
                aInvalidateRect.Left() = nFrozenWidth;
                aInvalidateRect.Right() = nFrozenWidth + nDelta - 1;
                Invalidate( aInvalidateRect );
		    }

		    // scroll the data-area
            aScrollRect.Bottom() = pDataWin->GetOutputSizePixel().Height();

            // actually scroll
		    pDataWin->Scroll( -nDelta, 0, aScrollRect, SCROLL_FLAGS );

            // invalidate the area of the column which was scrolled out to the left hand side
	        aScrollRect.Left() = nFrozenWidth;
	        aScrollRect.Right() = nFrozenWidth + nDelta - 1;
	        getDataWindow()->Invalidate( aScrollRect );
        }
	}

	// scrolling one column to the left?
	else if ( nCols == -1 )
	{
		--nFirstCol;
		aHScroll.SetThumbPos( nFirstCol - FrozenColCount() );

		if ( !bScrollable )
		{
			bInvalidateView = sal_True;
        }
        else
        {
		    long nDelta = pCols->GetObject(nFirstCol)->Width();
		    long nFrozenWidth = GetFrozenWidth();

            Rectangle aScrollRect(  Point(  nFrozenWidth, 0 ),
			                        Size (  GetOutputSizePixel().Width() - nFrozenWidth,
				                            GetTitleHeight() - 1
                                         ) );

		    // scroll the header bar area (if there is no dedicated HeaderBar control)
		    if ( !getDataWindow()->pHeaderBar && nTitleLines )
		    {
			    Scroll( nDelta, 0, aScrollRect, SCROLL_FLAGS );
		    }

		    // scroll the data-area
            aScrollRect.Bottom() = pDataWin->GetOutputSizePixel().Height();
		    pDataWin->Scroll( nDelta, 0, aScrollRect, SCROLL_FLAGS );
        }
	}
	else
	{
		if ( GetUpdateMode() )
		{
			Invalidate( Rectangle(
				Point( GetFrozenWidth(), 0 ),
				Size( GetOutputSizePixel().Width(), GetTitleHeight() ) ) );
			getDataWindow()->Invalidate( Rectangle(
				Point( GetFrozenWidth(), 0 ),
				pDataWin->GetSizePixel() ) );
		}

		nFirstCol = nFirstCol + (sal_uInt16)nCols;
		aHScroll.SetThumbPos( nFirstCol - FrozenColCount() );
	}

	// ggf. externe Headerbar anpassen
	if ( getDataWindow()->pHeaderBar )
	{
		long nWidth = 0;
		for ( sal_uInt16 nCol = 0;
			  nCol < pCols->Count() && nCol < nFirstCol;
			  ++nCol )
		{
			// HandleColumn nicht
			if ( pCols->GetObject(nCol)->GetId() )
				nWidth += pCols->GetObject(nCol)->Width();
		}

		getDataWindow()->pHeaderBar->SetOffset( nWidth );
	}

	if( bInvalidateView )
	{
		Control::Invalidate( INVALIDATE_NOCHILDREN );
		pDataWin->Window::Invalidate( INVALIDATE_NOCHILDREN );
	}

	// implicitly show cursor after scrolling
	if ( nCols )
	{
		getDataWindow()->Update();
		Update();
	}
	bScrolling = sal_False;
	EndScroll();

	return nCols;
}

//-------------------------------------------------------------------

long BrowseBox::ScrollRows( long nRows )
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	// out of range?
	if ( getDataWindow()->bNoScrollBack && nRows < 0 )
		return 0;

	// compute new top row
	long nTmpMin = Min( (long)(nTopRow + nRows), (long)(nRowCount - 1) );

	long nNewTopRow = Max( (long)nTmpMin, (long)0 );

	if ( nNewTopRow == nTopRow )
		return 0;

	sal_uInt16 nVisibleRows =
		(sal_uInt16)(pDataWin->GetOutputSizePixel().Height() / GetDataRowHeight() + 1);

	VisibleRowsChanged(nNewTopRow, nVisibleRows);

	// compute new top row again (nTopRow might have changed!)
	nTmpMin = Min( (long)(nTopRow + nRows), (long)(nRowCount - 1) );

	nNewTopRow = Max( (long)nTmpMin, (long)0 );

	StartScroll();

	// scroll area on screen and/or repaint
	long nDeltaY = GetDataRowHeight() * ( nNewTopRow - nTopRow );
	long nOldTopRow = nTopRow;
	nTopRow = nNewTopRow;

	if ( GetUpdateMode() )
	{
		pVScroll->SetRange( Range( 0L, nRowCount ) );
		pVScroll->SetThumbPos( nTopRow );

		if( pDataWin->GetBackground().IsScrollable() &&
			Abs( nDeltaY ) > 0 &&
			Abs( nDeltaY ) < pDataWin->GetSizePixel().Height() )
		{
			pDataWin->Scroll( 0, (short)-nDeltaY, SCROLL_FLAGS );
		}
		else
			getDataWindow()->Invalidate();

		if ( nTopRow - nOldTopRow )
			getDataWindow()->Update();
	}

	EndScroll();

	return nTopRow - nOldTopRow;
}

//-------------------------------------------------------------------

long BrowseBox::ScrollPages( long )
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	return ScrollRows( pDataWin->GetSizePixel().Height() / GetDataRowHeight() );
}

//-------------------------------------------------------------------

void BrowseBox::RowModified( long nRow, sal_uInt16 nColId )
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	if ( !GetUpdateMode() )
		return;

	Rectangle aRect;
	if ( nColId == USHRT_MAX )
		// invalidate the whole row
		aRect = Rectangle( Point( 0, (nRow-nTopRow) * GetDataRowHeight() ),
					Size( pDataWin->GetSizePixel().Width(), GetDataRowHeight() ) );
	else
	{
		// invalidate the specific field
		aRect = GetFieldRectPixel( nRow, nColId, sal_False );
	}
	getDataWindow()->Invalidate( aRect );
}

//-------------------------------------------------------------------

void BrowseBox::Clear()
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	// adjust the total number of rows
	DoHideCursor( "Clear" );
	long nOldRowCount = nRowCount;
	nRowCount = 0;
	nCurRow = BROWSER_ENDOFSELECTION;
	nTopRow = 0;
	nCurColId = 0;

	// nFirstCol darf nicht zurueckgesetzt werden, da ansonsten das Scrollen
	// total durcheinander kommt
	// nFirstCol darf nur beim Hinzufuegen oder Loeschen von Spalten geaendert werden
	// nFirstCol = 0; ->Falsch!!!!
	aHScroll.SetThumbPos( 0 );
	pVScroll->SetThumbPos( 0 );

	Invalidate();
	UpdateScrollbars();
	SetNoSelection();
	DoShowCursor( "Clear" );
	CursorMoved();

    if ( isAccessibleAlive() )
	{
		// all rows should be removed, so we remove the row header bar and append it again
		// to avoid to notify every row remove
        if ( nOldRowCount != nRowCount )
        {
		    commitBrowseBoxEvent(
                CHILD,
				Any(),
				makeAny( m_pImpl->getAccessibleHeaderBar( BBTYPE_ROWHEADERBAR ) )
            );

		    // and now append it again
		    commitBrowseBoxEvent(
                CHILD,
				makeAny( m_pImpl->getAccessibleHeaderBar( BBTYPE_ROWHEADERBAR ) ),
				Any()
            );

            // notify a table model change
            commitTableEvent(
                TABLE_MODEL_CHANGED,
				makeAny( AccessibleTableModelChange( DELETE,
                    0,
					nOldRowCount,
					0,
					GetColumnCount())
                ),
				Any()
            );
        }
	}
}
// -----------------------------------------------------------------------------
void BrowseBox::RowInserted( long nRow, long nNumRows, sal_Bool bDoPaint, sal_Bool bKeepSelection )
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	if (nRow < 0)
		nRow = 0;
	else if (nRow > nRowCount) // maximal = nRowCount
		nRow = nRowCount;

	if ( nNumRows <= 0 )
		return;

#if 0
	// Zerlegung in einzelne RowInserted-Aufrufe:
	if (nNumRows > 1)
	{
		for (long i = 0; i < nNumRows; i++)
			RowInserted(nRow + i,1,bDoPaint);
		return;
	}
#endif

	// adjust total row count
	sal_Bool bLastRow = nRow >= nRowCount;
	nRowCount += nNumRows;

	DoHideCursor( "RowInserted" );

	// must we paint the new rows?
	long nOldCurRow = nCurRow;
	Size aSz = pDataWin->GetOutputSizePixel();
	if ( bDoPaint && nRow >= nTopRow &&
		 nRow <= nTopRow + aSz.Height() / GetDataRowHeight() )
	{
		long nY = (nRow-nTopRow) * GetDataRowHeight();
		if ( !bLastRow )
		{
			// scroll down the rows behind the new row
			pDataWin->SetClipRegion();
			if( pDataWin->GetBackground().IsScrollable() )
			{
				pDataWin->Scroll( 0, GetDataRowHeight() * nNumRows,
								Rectangle( Point( 0, nY ),
										Size( aSz.Width(), aSz.Height() - nY ) ),
								SCROLL_FLAGS );
			}
			else
				pDataWin->Window::Invalidate( INVALIDATE_NOCHILDREN );
		}
		else
			// scroll would cause a repaint, so we must explicitly invalidate
			pDataWin->Invalidate( Rectangle( Point( 0, nY ),
						 Size( aSz.Width(), nNumRows * GetDataRowHeight() ) ) );
	}

	// ggf. Top-Row korrigieren
	if ( nRow < nTopRow )
		nTopRow += nNumRows;

	// adjust the selection
	if ( bMultiSelection )
		uRow.pSel->Insert( nRow, nNumRows );
	else if ( uRow.nSel != BROWSER_ENDOFSELECTION && nRow <= uRow.nSel )
		uRow.nSel += nNumRows;

	// adjust the cursor
	if ( nCurRow == BROWSER_ENDOFSELECTION )
		GoToRow( 0, sal_False, bKeepSelection );
	else if ( nRow <= nCurRow )
		GoToRow( nCurRow += nNumRows, sal_False, bKeepSelection );

	// adjust the vertical scrollbar
	if ( bDoPaint )
	{
		UpdateScrollbars();
		AutoSizeLastColumn();
	}

	DoShowCursor( "RowInserted" );
	// notify accessible that rows were inserted
    if ( isAccessibleAlive() )
	{
		commitTableEvent(
            TABLE_MODEL_CHANGED,
			makeAny( AccessibleTableModelChange(
                        INSERT,
						nRow,
						nRow + nNumRows,
						0,
						GetColumnCount()
                    )
            ),
			Any()
        );

		for (sal_Int32 i = nRow+1 ; i <= nRowCount ; ++i)
		{
			commitHeaderBarEvent(
                CHILD,
				makeAny( CreateAccessibleRowHeader( i ) ),
				Any(),
				sal_False
            );
		}
	}

	if ( nCurRow != nOldCurRow )
		CursorMoved();

	DBG_ASSERT(nRowCount > 0,"BrowseBox: nRowCount <= 0");
	DBG_ASSERT(nCurRow >= 0,"BrowseBox: nCurRow < 0");
	DBG_ASSERT(nCurRow < nRowCount,"nCurRow >= nRowCount");
}

//-------------------------------------------------------------------

void BrowseBox::RowRemoved( long nRow, long nNumRows, sal_Bool bDoPaint )
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	if ( nRow < 0 )
		nRow = 0;
	else if ( nRow >= nRowCount )
		nRow = nRowCount - 1;

	if ( nNumRows <= 0 )
		return;

	if ( nRowCount <= 0 )
		return;

	if ( bDoPaint )
	{
		// hide cursor and selection
		DBG_TRACE1( "BrowseBox: %p->HideCursor", this );
		ToggleSelection();
		DoHideCursor( "RowRemoved" );
	}

	// adjust total row count
	nRowCount -= nNumRows;
	if (nRowCount < 0) nRowCount = 0;
	long nOldCurRow = nCurRow;

	// adjust the selection
	if ( bMultiSelection )
		// uRow.pSel->Remove( nRow, nNumRows );
		for ( long i = 0; i < nNumRows; i++ )
			uRow.pSel->Remove( nRow );
	else if ( nRow < uRow.nSel && uRow.nSel >= nNumRows )
		uRow.nSel -= nNumRows;
	else if ( nRow <= uRow.nSel )
		uRow.nSel = BROWSER_ENDOFSELECTION;

	// adjust the cursor
	if ( nRowCount == 0 )	// don't compare nRowCount with nNumRows as nNumRows already was subtracted from nRowCount
		nCurRow = BROWSER_ENDOFSELECTION;
	else if ( nRow < nCurRow )
	{
		nCurRow -= Min( nCurRow - nRow, nNumRows );
		// with the above nCurRow points a) to the first row after the removed block or b) to the same line
		// as before, but moved up nNumRows
		// case a) needs an additional correction if the last n lines were deleted, as 'the first row after the
		// removed block' is an invalid position then
		// FS - 09/28/99 - 68429
		if (nCurRow == nRowCount)
			--nCurRow;
	}
	else if( nRow == nCurRow && nCurRow == nRowCount )
		nCurRow = nRowCount-1;

	// is the deleted row visible?
	Size aSz = pDataWin->GetOutputSizePixel();
	if ( nRow >= nTopRow &&
		 nRow <= nTopRow + aSz.Height() / GetDataRowHeight() )
	{
		if ( bDoPaint )
		{
			// scroll up the rows behind the deleted row
			// if there are Rows behind
			if (nRow < nRowCount)
			{
				long nY = (nRow-nTopRow) * GetDataRowHeight();
				pDataWin->SetClipRegion();
				if( pDataWin->GetBackground().IsScrollable() )
				{
					pDataWin->Scroll( 0, - (short) GetDataRowHeight() * nNumRows,
						Rectangle( Point( 0, nY ), Size( aSz.Width(),
							aSz.Height() - nY + nNumRows*GetDataRowHeight() ) ),
							SCROLL_FLAGS );
				}
				else
					pDataWin->Window::Invalidate( INVALIDATE_NOCHILDREN );
			}
			else
			{
				// Repaint the Rect of the deleted row
				Rectangle aRect(
						Point( 0, (nRow-nTopRow)*GetDataRowHeight() ),
						Size( pDataWin->GetSizePixel().Width(),
							  nNumRows * GetDataRowHeight() ) );
				pDataWin->Invalidate( aRect );
			}
		}
	}
	// is the deleted row above of the visible area?
	else if ( nRow < nTopRow )
		nTopRow = nTopRow >= nNumRows ? nTopRow-nNumRows : 0;

	if ( bDoPaint )
	{
		// reshow cursor and selection
		ToggleSelection();
		DBG_TRACE1( "BrowseBox: %p->ShowCursor", this );
		DoShowCursor( "RowRemoved" );

		// adjust the vertical scrollbar
		UpdateScrollbars();
		AutoSizeLastColumn();
	}

    if ( isAccessibleAlive() )
	{
		if ( nRowCount == 0 )
		{
			// all columns should be removed, so we remove the column header bar and append it again
			// to avoid to notify every column remove
			commitBrowseBoxEvent(
                CHILD,
				Any(),
				makeAny( m_pImpl->getAccessibleHeaderBar( BBTYPE_ROWHEADERBAR ) )
            );

            // and now append it again
			commitBrowseBoxEvent(
                CHILD,
				makeAny(m_pImpl->getAccessibleHeaderBar(BBTYPE_ROWHEADERBAR)),
				Any()
            );
			commitBrowseBoxEvent(
                CHILD,
				Any(),
				makeAny( m_pImpl->getAccessibleTable() )
            );

            // and now append it again
			commitBrowseBoxEvent(
                CHILD,
				makeAny( m_pImpl->getAccessibleTable() ),
				Any()
            );
		}
		else
		{
			commitTableEvent(
                TABLE_MODEL_CHANGED,
				makeAny( AccessibleTableModelChange(
                            DELETE,
							nRow,
							nRow + nNumRows,
							0,
							GetColumnCount()
                            )
                ),
				Any()
            );

            for (sal_Int32 i = nRow+1 ; i <= (nRow+nNumRows) ; ++i)
			{
				commitHeaderBarEvent(
                    CHILD,
					Any(),
					makeAny( CreateAccessibleRowHeader( i ) ),
                    sal_False
                );
			}
		}
	}

	if ( nOldCurRow != nCurRow )
		CursorMoved();

	DBG_ASSERT(nRowCount >= 0,"BrowseBox: nRowCount < 0");
	DBG_ASSERT(nCurRow >= 0 || nRowCount == 0,"BrowseBox: nCurRow < 0 && nRowCount != 0");
	DBG_ASSERT(nCurRow < nRowCount,"nCurRow >= nRowCount");
}

//-------------------------------------------------------------------

sal_Bool BrowseBox::GoToRow( long nRow)
{
	return GoToRow(nRow, sal_False, sal_False);
}

//-------------------------------------------------------------------

sal_Bool BrowseBox::GoToRowAndDoNotModifySelection( long nRow )
{
	return GoToRow( nRow, sal_False, sal_True );
}

//-------------------------------------------------------------------
sal_Bool BrowseBox::GoToRow( long nRow, sal_Bool bRowColMove, sal_Bool bKeepSelection )
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	long nOldCurRow = nCurRow;

	// nothing to do?
	if ( nRow == nCurRow && ( bMultiSelection || uRow.nSel == nRow ) )
		return sal_True;

	// out of range?
	if ( nRow < 0 || nRow >= nRowCount )
		return sal_False;

	// nicht erlaubt?
	if ( ( !bRowColMove && !IsCursorMoveAllowed( nRow, nCurColId ) ) )
		return sal_False;

	if ( getDataWindow()->bNoScrollBack && nRow < nTopRow )
		nRow = nTopRow;

	// compute the last visible row
	Size aSz( pDataWin->GetSizePixel() );
	sal_uInt16 nVisibleRows = sal_uInt16( aSz.Height() / GetDataRowHeight() - 1 );
	long nLastRow = nTopRow + nVisibleRows;

	// suspend Updates
	getDataWindow()->EnterUpdateLock();

	// ggf. altes Highlight weg
	if ( !bMultiSelection && !bKeepSelection )
		ToggleSelection();
	DoHideCursor( "GoToRow" );

	// must we scroll?
	sal_Bool bWasVisible = bSelectionIsVisible;
	if (! bMultiSelection)
	{
		if( !bKeepSelection )
			bSelectionIsVisible = sal_False;
	}
	if ( nRow < nTopRow )
		ScrollRows( nRow - nTopRow );
	else if ( nRow > nLastRow )
		ScrollRows( nRow - nLastRow );
	bSelectionIsVisible = bWasVisible;

	// adjust cursor (selection) and thumb
	if ( GetUpdateMode() )
		pVScroll->SetThumbPos( nTopRow );

	// relative positioning (because nCurRow might have changed in the meantime)!
	if (nCurRow != BROWSER_ENDOFSELECTION )
		nCurRow = nCurRow + (nRow - nOldCurRow);

	// make sure that the current position is valid
	if (nCurRow == BROWSER_ENDOFSELECTION && nRowCount > 0)
		nCurRow = 0;
	else if ( nCurRow >= nRowCount )
		nCurRow = nRowCount - 1;
	aSelRange = Range( nCurRow, nCurRow );

	// ggf. neues Highlight anzeigen
	if ( !bMultiSelection && !bKeepSelection )
		uRow.nSel = nRow;

	// resume Updates
	getDataWindow()->LeaveUpdateLock();

	// Cursor+Highlight
	if ( !bMultiSelection && !bKeepSelection)
		ToggleSelection();
	DoShowCursor( "GoToRow" );
	if ( !bRowColMove  && nOldCurRow != nCurRow )
		CursorMoved();

	if ( !bMultiSelection && !bKeepSelection )
	{
		if ( !bSelecting )
			Select();
		else
			bSelect = sal_True;
	}
	return sal_True;
}

//-------------------------------------------------------------------

sal_Bool BrowseBox::GoToColumnId( sal_uInt16 nColId)
{
	return GoToColumnId(nColId,sal_True,sal_False);
}


sal_Bool BrowseBox::GoToColumnId( sal_uInt16 nColId, sal_Bool bMakeVisible, sal_Bool bRowColMove)
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	if (!bColumnCursor)
		return sal_False;

	// erlaubt?
	if (!bRowColMove && !IsCursorMoveAllowed( nCurRow, nColId ) )
		return sal_False;

	if ( nColId != nCurColId || (bMakeVisible && !IsFieldVisible(nCurRow, nColId, sal_True)))
	{
        sal_uInt16 nNewPos = GetColumnPos(nColId);
        BrowserColumn* pColumn = pCols->GetObject( nNewPos );
        DBG_ASSERT( pColumn, "no column object - invalid id?" );
        if ( !pColumn )
            return sal_False;

		DoHideCursor( "GoToColumnId" );
		nCurColId = nColId;

		sal_uInt16 nFirstPos = nFirstCol;
        sal_uInt16 nWidth = (sal_uInt16)pColumn->Width();
		sal_uInt16 nLastPos = GetColumnAtXPosPixel(
							pDataWin->GetSizePixel().Width()-nWidth, sal_False );
		sal_uInt16 nFrozen = FrozenColCount();
		if ( bMakeVisible && nLastPos &&
			 nNewPos >= nFrozen && ( nNewPos < nFirstPos || nNewPos > nLastPos ) )
		{
			if ( nNewPos < nFirstPos )
				ScrollColumns( nNewPos-nFirstPos );
			else if ( nNewPos > nLastPos )
				ScrollColumns( nNewPos-nLastPos );
		}

		DoShowCursor( "GoToColumnId" );
		if (!bRowColMove)
			CursorMoved();
		return sal_True;
	}
	return sal_True;
}

//-------------------------------------------------------------------

sal_Bool BrowseBox::GoToRowColumnId( long nRow, sal_uInt16 nColId )
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	// out of range?
	if ( nRow < 0 || nRow >= nRowCount )
		return sal_False;

	if (!bColumnCursor)
		return sal_False;

	// nothing to do ?
	if ( nRow == nCurRow && ( bMultiSelection || uRow.nSel == nRow ) &&
		 nColId == nCurColId && IsFieldVisible(nCurRow, nColId, sal_True))
		return sal_True;

	// erlaubt?
	if (!IsCursorMoveAllowed(nRow, nColId))
		return sal_False;

	DoHideCursor( "GoToRowColumnId" );
	sal_Bool bMoved = GoToRow(nRow, sal_True) && GoToColumnId(nColId, sal_True, sal_True);
	DoShowCursor( "GoToRowColumnId" );

	if (bMoved)
		CursorMoved();

	return bMoved;
}

//-------------------------------------------------------------------

void BrowseBox::SetNoSelection()
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	// is there no selection
	if ( ( !pColSel || !pColSel->GetSelectCount() ) &&
		 ( ( !bMultiSelection && uRow.nSel == BROWSER_ENDOFSELECTION ) ||
		   ( bMultiSelection && !uRow.pSel->GetSelectCount() ) ) )
		// nothing to do
		return;

	DBG_TRACE1( "BrowseBox: %p->HideCursor", this );
	ToggleSelection();

	// unselect all
	if ( bMultiSelection )
		uRow.pSel->SelectAll(sal_False);
	else
		uRow.nSel = BROWSER_ENDOFSELECTION;
	if ( pColSel )
		pColSel->SelectAll(sal_False);
	if ( !bSelecting )
		Select();
	else
		bSelect = sal_True;

	// restore screen
	DBG_TRACE1( "BrowseBox: %p->ShowCursor", this );

    if ( isAccessibleAlive() )
	{
		commitTableEvent(
            SELECTION_CHANGED,
			Any(),
			Any()
        );
	}
}

//-------------------------------------------------------------------

void BrowseBox::SetSelection( const MultiSelection &rSel )
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);
	DBG_ASSERT( bMultiSelection, "SetSelection only allowed with Multi-Selection-Mode" );

	// prepare inverted areas
	DBG_TRACE1( "BrowseBox: %p->HideCursor", this );
	ToggleSelection();

	// assign Selection
	*uRow.pSel = rSel;

	// only highlight painted areas
	pDataWin->Update();

	// notify derived class
	if ( !bSelecting )
		Select();
	else
		bSelect = sal_True;

	// restore screen
	ToggleSelection();
	DBG_TRACE1( "BrowseBox: %p->ShowCursor", this );

    if ( isAccessibleAlive() )
	{
		commitTableEvent(
            SELECTION_CHANGED,
			Any(),
			Any()
        );
	}
}

//-------------------------------------------------------------------

void BrowseBox::SelectAll()
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	if ( !bMultiSelection )
		return;

	DBG_TRACE1( "BrowseBox: %p->HideCursor", this );
	ToggleSelection();

	// select all rows
	if ( pColSel )
		pColSel->SelectAll(sal_False);
	uRow.pSel->SelectAll(sal_True);

	// Handle-Column nicht highlighten
	BrowserColumn *pFirstCol = pCols->GetObject(0);
	long nOfsX = pFirstCol->GetId() ? 0 : pFirstCol->Width();

	// highlight the row selection
	if ( !bHideSelect )
	{
		Rectangle aHighlightRect;
		sal_uInt16 nVisibleRows =
			(sal_uInt16)(pDataWin->GetOutputSizePixel().Height() / GetDataRowHeight() + 1);
		for ( long nRow = Max( nTopRow, uRow.pSel->FirstSelected() );
			  nRow != BROWSER_ENDOFSELECTION && nRow < nTopRow + nVisibleRows;
			  nRow = uRow.pSel->NextSelected() )
			aHighlightRect.Union( Rectangle(
				Point( nOfsX, (nRow-nTopRow)*GetDataRowHeight() ),
				Size( pDataWin->GetSizePixel().Width(), GetDataRowHeight() ) ) );
		pDataWin->Invalidate( aHighlightRect );
	}

	if ( !bSelecting )
		Select();
	else
		bSelect = sal_True;

	// restore screen
	DBG_TRACE1( "BrowseBox: %p->ShowCursor", this );

    if ( isAccessibleAlive() )
	{
		commitTableEvent(
            SELECTION_CHANGED,
			Any(),
			Any()
        );
		commitHeaderBarEvent(
            SELECTION_CHANGED,
			Any(),
			Any(),
			sal_True
        ); // column header event

        commitHeaderBarEvent(
            SELECTION_CHANGED,
			Any(),
			Any(),
			sal_False
        ); // row header event
	}
}

//-------------------------------------------------------------------

void BrowseBox::SelectRow( long nRow, sal_Bool _bSelect, sal_Bool bExpand )
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	if ( !bMultiSelection )
	{
		// deselecting is impossible, selecting via cursor
		if ( _bSelect )
			GoToRow(nRow, sal_False);
		return;
	}

	DBG_TRACE1( "BrowseBox: %p->HideCursor", this );

	// remove old selection?
	if ( !bExpand || !bMultiSelection )
	{
		ToggleSelection();
		if ( bMultiSelection )
			uRow.pSel->SelectAll(sal_False);
		else
			uRow.nSel = BROWSER_ENDOFSELECTION;
		if ( pColSel )
			pColSel->SelectAll(sal_False);
	}

	// set new selection
	if	(	!bHideSelect
		&&	(	(	bMultiSelection
				&&	uRow.pSel->GetTotalRange().Max() >= nRow
				&&	uRow.pSel->Select( nRow, _bSelect )
				)
			||	(	!bMultiSelection
				&&	( uRow.nSel = nRow ) != BROWSER_ENDOFSELECTION )
				)
			)
	{
		// Handle-Column nicht highlighten
		BrowserColumn *pFirstCol = pCols->GetObject(0);
		long nOfsX = pFirstCol->GetId() ? 0 : pFirstCol->Width();

		// highlight only newly selected part
		Rectangle aRect(
			Point( nOfsX, (nRow-nTopRow)*GetDataRowHeight() ),
			Size( pDataWin->GetSizePixel().Width(), GetDataRowHeight() ) );
		pDataWin->Invalidate( aRect );
	}

	if ( !bSelecting )
		Select();
	else
		bSelect = sal_True;

	// restore screen
	DBG_TRACE1( "BrowseBox: %p->ShowCursor", this );

    if ( isAccessibleAlive() )
	{
		commitTableEvent(
            SELECTION_CHANGED,
			Any(),
			Any()
        );
		commitHeaderBarEvent(
            SELECTION_CHANGED,
			Any(),
			Any(),
			sal_False
        ); // row header event
	}
}

//-------------------------------------------------------------------

long BrowseBox::GetSelectRowCount() const
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	return bMultiSelection ? uRow.pSel->GetSelectCount() :
		   uRow.nSel == BROWSER_ENDOFSELECTION ? 0 : 1;
}

//-------------------------------------------------------------------

void BrowseBox::SelectColumnPos( sal_uInt16 nNewColPos, sal_Bool _bSelect, sal_Bool bMakeVisible )
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	if ( !bColumnCursor || nNewColPos == BROWSER_INVALIDID )
		return;

	if ( !bMultiSelection )
	{
		if ( _bSelect )
			GoToColumnId( pCols->GetObject(nNewColPos)->GetId(), bMakeVisible );
		return;
	}
	else
	{
		if ( !GoToColumnId( pCols->GetObject( nNewColPos )->GetId(), bMakeVisible ) )
			return;
	}

	DBG_TRACE1( "BrowseBox: %p->HideCursor", this );
	ToggleSelection();
	if ( bMultiSelection )
		uRow.pSel->SelectAll(sal_False);
	else
		uRow.nSel = BROWSER_ENDOFSELECTION;
	pColSel->SelectAll(sal_False);

	if ( pColSel->Select( nNewColPos, _bSelect ) )
	{
		// GoToColumnId( pCols->GetObject(nNewColPos)->GetId(), bMakeVisible );

		// only highlight painted areas
		pDataWin->Update();
		Rectangle aFieldRectPix( GetFieldRectPixel( nCurRow, nCurColId, sal_False ) );
		Rectangle aRect(
			Point( aFieldRectPix.Left() - MIN_COLUMNWIDTH, 0 ),
			Size( pCols->GetObject(nNewColPos)->Width(),
				  pDataWin->GetOutputSizePixel().Height() ) );
		pDataWin->Invalidate( aRect );
		if ( !bSelecting )
			Select();
		else
			bSelect = sal_True;

        if ( isAccessibleAlive() )
		{
			commitTableEvent(
                SELECTION_CHANGED,
				Any(),
				Any()
            );
			commitHeaderBarEvent(
                SELECTION_CHANGED,
				Any(),
				Any(),
				sal_True
            ); // column header event
		}
	}

	// restore screen
	DBG_TRACE1( "BrowseBox: %p->ShowCursor", this );
}

//-------------------------------------------------------------------

sal_uInt16 BrowseBox::GetSelectColumnCount() const
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	// while bAutoSelect (==!pColSel), 1 if any rows (yes rows!) else none
	return pColSel ? (sal_uInt16) pColSel->GetSelectCount() :
		   nCurRow >= 0 ? 1 : 0;
}

//-------------------------------------------------------------------
long BrowseBox::FirstSelectedColumn( ) const
{
	return pColSel ? pColSel->FirstSelected() : BROWSER_ENDOFSELECTION;
}

//-------------------------------------------------------------------
long BrowseBox::NextSelectedColumn( ) const
{
	return pColSel ? pColSel->NextSelected() : BROWSER_ENDOFSELECTION;
}

//-------------------------------------------------------------------

long BrowseBox::FirstSelectedRow( sal_Bool bInverse )
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	return bMultiSelection ? uRow.pSel->FirstSelected(bInverse) : uRow.nSel;
}

//-------------------------------------------------------------------

long BrowseBox::NextSelectedRow()
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	return bMultiSelection ? uRow.pSel->NextSelected() : BROWSER_ENDOFSELECTION;
}

//-------------------------------------------------------------------

long BrowseBox::PrevSelectedRow()
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	return bMultiSelection ? uRow.pSel->PrevSelected() : BROWSER_ENDOFSELECTION;
}

//-------------------------------------------------------------------

long BrowseBox::LastSelectedRow()
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	return bMultiSelection ? uRow.pSel->LastSelected() : uRow.nSel;
}

//-------------------------------------------------------------------

bool BrowseBox::IsRowSelected( long nRow ) const
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	return bMultiSelection ? uRow.pSel->IsSelected(nRow) : nRow == uRow.nSel;
}

//-------------------------------------------------------------------

bool BrowseBox::IsColumnSelected( sal_uInt16 nColumnId ) const
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	return pColSel ? pColSel->IsSelected( GetColumnPos(nColumnId) ) :
					 nCurColId == nColumnId;
}

//-------------------------------------------------------------------

sal_Bool BrowseBox::IsAllSelected() const
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    return bMultiSelection && uRow.pSel->IsAllSelected();
}

//-------------------------------------------------------------------

sal_Bool BrowseBox::MakeFieldVisible
(
	long	nRow,		// Zeilen-Nr des Feldes (beginnend mit 0)
	sal_uInt16	nColId, 	// Spalten-Id des Feldes
	sal_Bool	bComplete	// (== sal_False), sal_True => vollst"andig sichtbar machen
)

/*	[Beschreibung]

	Macht das durch 'nRow' und 'nColId' beschriebene Feld durch
	entsprechendes scrollen sichtbar. Ist 'bComplete' gesetzt, dann wird
	gefordert, da\s das Feld ganz sichtbar wird.

	[R"uckgabewert]

	sal_Bool			sal_True
					Das angegebene Feld wurde sichtbar gemacht, bzw. war
					bereits sichtbar.

					sal_False
					Das angegebene Feld konnte nicht sichtbar bzw. bei
					'bComplete' nicht vollst"andig sichtbar gemacht werden.
*/

{
	Size aTestSize = pDataWin->GetSizePixel();

	if ( !bBootstrapped ||
		 ( aTestSize.Width() == 0 && aTestSize.Height() == 0 ) )
		return sal_False;

	// ist es schon sichtbar?
	sal_Bool bVisible = IsFieldVisible( nRow, nColId, bComplete );
	if ( bVisible )
		return sal_True;

	// Spaltenposition und Feld-Rechteck und Ausgabebereich berechnen
	sal_uInt16 nColPos = GetColumnPos( nColId );
	Rectangle aFieldRect = GetFieldRectPixel( nRow, nColId, sal_False );
	Rectangle aDataRect = Rectangle( Point(0, 0), pDataWin->GetSizePixel() );

	// links au\serhalb?
	if ( nColPos >= FrozenColCount() && nColPos < nFirstCol )
		// => nach rechts scrollen
		ScrollColumns( nColPos - nFirstCol );

	// solange rechts au\serhalb
	while ( aDataRect.Right() < ( bComplete
				? aFieldRect.Right()
				: aFieldRect.Left()+aFieldRect.GetWidth()/2 ) )
	{
		// => nach links scrollen
		if ( ScrollColumns( 1 ) != 1 )
			// nichts mehr zu scrollen
			break;
		aFieldRect = GetFieldRectPixel( nRow, nColId, sal_False );
	}

	// oben au\serhalb?
	if ( nRow < nTopRow )
		// nach unten scrollen
		ScrollRows( nRow - nTopRow );

	// unten au\serhalb?
	long nBottomRow = nTopRow + GetVisibleRows();
	// OV: damit nBottomRow die Nummer der letzten sichtbaren Zeile ist
	// (Zaehlung ab Null!), muss sie dekrementiert werden.
	// Beispiel: BrowseBox enthaelt genau einen Eintrag. nBottomRow := 0 + 1 - 1
	if( nBottomRow )
		nBottomRow--;

	if ( nRow > nBottomRow )
		// nach oben scrollen
		ScrollRows( nRow - nBottomRow );

	// jetzt kann es immer noch nicht passen, z.B. weil Window zu klein
	return IsFieldVisible( nRow, nColId, bComplete );
}

//-------------------------------------------------------------------

sal_Bool BrowseBox::IsFieldVisible( long nRow, sal_uInt16 nColumnId,
								sal_Bool bCompletely ) const
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	// durch frozen-Column verdeckt?
	sal_uInt16 nColPos = GetColumnPos( nColumnId );
	if ( nColPos >= FrozenColCount() && nColPos < nFirstCol )
		return sal_False;

	Rectangle aRect( ImplFieldRectPixel( nRow, nColumnId ) );
	if ( aRect.IsEmpty() )
		return sal_False;

	// get the visible area
	Rectangle aOutRect( Point(0, 0), pDataWin->GetOutputSizePixel() );

	if ( bCompletely )
		// test if the field is completely visible
		return aOutRect.IsInside( aRect );
	else
		// test if the field is partly of completely visible
		return !aOutRect.Intersection( aRect ).IsEmpty();
}

//-------------------------------------------------------------------

Rectangle BrowseBox::GetFieldRectPixel( long nRow, sal_uInt16 nColumnId,
										sal_Bool bRelToBrowser) const
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	// get the rectangle relative to DataWin
	Rectangle aRect( ImplFieldRectPixel( nRow, nColumnId ) );
	if ( aRect.IsEmpty() )
		return aRect;

	// adjust relative to BrowseBox's output area
	Point aTopLeft( aRect.TopLeft() );
	if ( bRelToBrowser )
	{
		aTopLeft = pDataWin->OutputToScreenPixel( aTopLeft );
		aTopLeft = ScreenToOutputPixel( aTopLeft );
	}

	return Rectangle( aTopLeft, aRect.GetSize() );
}

//-------------------------------------------------------------------

Rectangle BrowseBox::GetRowRectPixel( long nRow, sal_Bool bRelToBrowser  ) const
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	// get the rectangle relative to DataWin
	Rectangle aRect;
	if ( nTopRow > nRow )
		// row is above visible area
		return aRect;
	aRect = Rectangle(
		Point( 0, GetDataRowHeight() * (nRow-nTopRow) ),
		Size( pDataWin->GetOutputSizePixel().Width(), GetDataRowHeight() ) );
	if ( aRect.TopLeft().Y() > pDataWin->GetOutputSizePixel().Height() )
		// row is below visible area
		return aRect;

	// adjust relative to BrowseBox's output area
	Point aTopLeft( aRect.TopLeft() );
	if ( bRelToBrowser )
	{
		aTopLeft = pDataWin->OutputToScreenPixel( aTopLeft );
		aTopLeft = ScreenToOutputPixel( aTopLeft );
	}

	return Rectangle( aTopLeft, aRect.GetSize() );
}

//-------------------------------------------------------------------

Rectangle BrowseBox::ImplFieldRectPixel( long nRow, sal_uInt16 nColumnId ) const
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	// compute the X-coordinte realtiv to DataWin by accumulation
	long nColX = 0;
	sal_uInt16 nFrozenCols = FrozenColCount();
	sal_uInt16 nCol;
	for ( nCol = 0;
		  nCol < pCols->Count() && pCols->GetObject(nCol)->GetId() != nColumnId;
		  ++nCol )
		if ( pCols->GetObject(nCol)->IsFrozen() || nCol >= nFirstCol )
			nColX += pCols->GetObject(nCol)->Width();

	if ( nCol >= pCols->Count() || ( nCol >= nFrozenCols && nCol < nFirstCol ) )
		return Rectangle();

	// compute the Y-coordinate relative to DataWin
	long nRowY = GetDataRowHeight();
	if ( nRow != BROWSER_ENDOFSELECTION ) // #105497# OJ
		nRowY = ( nRow - nTopRow ) * GetDataRowHeight();

	// assemble the Rectangle relative to DataWin
	return Rectangle(
		Point( nColX + MIN_COLUMNWIDTH, nRowY ),
		Size( pCols->GetObject(nCol)->Width() - 2*MIN_COLUMNWIDTH,
			  GetDataRowHeight() - 1 ) );
}

//-------------------------------------------------------------------

long BrowseBox::GetRowAtYPosPixel( long nY, sal_Bool bRelToBrowser ) const
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	// compute the Y-coord
	if ( bRelToBrowser )
	{
		Point aDataTopLeft = pDataWin->OutputToScreenPixel( Point(0, 0) );
		Point aTopLeft = OutputToScreenPixel( Point(0, 0) );
		nY -= aDataTopLeft.Y() - aTopLeft.Y();
	}

	// no row there (e.g. in the header)
	if ( nY < 0 || nY >= pDataWin->GetOutputSizePixel().Height() )
		return -1;

	return nY / GetDataRowHeight() + nTopRow;
}

//-------------------------------------------------------------------

Rectangle BrowseBox::GetFieldRect( sal_uInt16 nColumnId ) const
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	return GetFieldRectPixel( nCurRow, nColumnId );
}

//-------------------------------------------------------------------

sal_uInt16 BrowseBox::GetColumnAtXPosPixel( long nX, sal_Bool ) const
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	// accumulate the withds of the visible columns
	long nColX = 0;
	sal_uInt16 nCol;
	for ( nCol = 0; nCol < sal_uInt16(pCols->Count()); ++nCol )
	{
		BrowserColumn *pCol = pCols->GetObject(nCol);
		if ( pCol->IsFrozen() || nCol >= nFirstCol )
			nColX += pCol->Width();

		if ( nColX > nX )
			return nCol;
	}

	return BROWSER_INVALIDID;
}

//-------------------------------------------------------------------

void BrowseBox::ReserveControlArea( sal_uInt16 nWidth )
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	if ( nWidth != nControlAreaWidth )
	{
        OSL_ENSURE(nWidth,"Control aera of 0 is not allowed, Use USHRT_MAX instead!");
		nControlAreaWidth = nWidth;
		UpdateScrollbars();
	}
}

//-------------------------------------------------------------------

Rectangle BrowseBox::GetControlArea() const
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	return Rectangle(
		Point( 0, GetOutputSizePixel().Height() - aHScroll.GetSizePixel().Height() ),
		Size( GetOutputSizePixel().Width() - aHScroll.GetSizePixel().Width(),
			 aHScroll.GetSizePixel().Height() ) );
}

//-------------------------------------------------------------------

void BrowseBox::SetMode( BrowserMode nMode )
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

#ifdef DBG_MIx
	Sound::Beep();
	nMode =
//  		BROWSER_COLUMNSELECTION |
//  		BROWSER_MULTISELECTION |
			BROWSER_THUMBDRAGGING |
			BROWSER_KEEPHIGHLIGHT |
			BROWSER_HLINES |
			BROWSER_VLINES |
// 			BROWSER_HIDECURSOR |
//  		BROWSER_NO_HSCROLL |
//  		BROWSER_NO_SCROLLBACK |
			BROWSER_AUTO_VSCROLL |
			BROWSER_AUTO_HSCROLL |
			BROWSER_TRACKING_TIPS |
//  		BROWSER_HIGHLIGHT_NONE |
			BROWSER_HEADERBAR_NEW |
//			BROWSER_AUTOSIZE_LASTCOL |
			0;
#endif

	getDataWindow()->bAutoHScroll = BROWSER_AUTO_HSCROLL == ( nMode & BROWSER_AUTO_HSCROLL );
	getDataWindow()->bAutoVScroll = BROWSER_AUTO_VSCROLL == ( nMode & BROWSER_AUTO_VSCROLL );
	getDataWindow()->bNoHScroll   = BROWSER_NO_HSCROLL   == ( nMode & BROWSER_NO_HSCROLL );
    getDataWindow()->bNoVScroll   = BROWSER_NO_VSCROLL   == ( nMode & BROWSER_NO_VSCROLL );

    DBG_ASSERT( !( getDataWindow()->bAutoHScroll && getDataWindow()->bNoHScroll ),
        "BrowseBox::SetMode: AutoHScroll *and* NoHScroll?" );
    DBG_ASSERT( !( getDataWindow()->bAutoVScroll && getDataWindow()->bNoVScroll ),
        "BrowseBox::SetMode: AutoVScroll *and* NoVScroll?" );
    if ( getDataWindow()->bAutoHScroll )
        getDataWindow()->bNoHScroll = sal_False;
    if ( getDataWindow()->bAutoVScroll )
        getDataWindow()->bNoVScroll = sal_False;

    if ( getDataWindow()->bNoHScroll )
		aHScroll.Hide();

    nControlAreaWidth = USHRT_MAX;

	getDataWindow()->bNoScrollBack =
			BROWSER_NO_SCROLLBACK == ( nMode & BROWSER_NO_SCROLLBACK);

	long nOldRowSel = bMultiSelection ? uRow.pSel->FirstSelected() : uRow.nSel;
	MultiSelection *pOldRowSel = bMultiSelection ? uRow.pSel : 0;
	MultiSelection *pOldColSel = pColSel;

	delete pVScroll;

	bThumbDragging = ( nMode & BROWSER_THUMBDRAGGING ) == BROWSER_THUMBDRAGGING;
	bMultiSelection = ( nMode & BROWSER_MULTISELECTION ) == BROWSER_MULTISELECTION;
	bColumnCursor = ( nMode & BROWSER_COLUMNSELECTION ) == BROWSER_COLUMNSELECTION;
	bKeepHighlight = ( nMode & BROWSER_KEEPSELECTION ) == BROWSER_KEEPSELECTION;

	bHideSelect = ((nMode & BROWSER_HIDESELECT) == BROWSER_HIDESELECT);
	// default: do not hide the cursor at all (untaken scrolling and such)
	bHideCursor = NO_CURSOR_HIDE;

	if ( BROWSER_SMART_HIDECURSOR == ( nMode & BROWSER_SMART_HIDECURSOR ) )
	{	// smart cursor hide overrules hard cursor hide
		bHideCursor = SMART_CURSOR_HIDE;
	}
	else if ( BROWSER_HIDECURSOR == ( nMode & BROWSER_HIDECURSOR ) )
	{
		bHideCursor = HARD_CURSOR_HIDE;
	}

	m_bFocusOnlyCursor = ((nMode & BROWSER_CURSOR_WO_FOCUS) == 0);

	bHLines = ( nMode & BROWSER_HLINESFULL ) == BROWSER_HLINESFULL;
	bVLines = ( nMode & BROWSER_VLINESFULL ) == BROWSER_VLINESFULL;
	bHDots  = ( nMode & BROWSER_HLINESDOTS ) == BROWSER_HLINESDOTS;
	bVDots  = ( nMode & BROWSER_VLINESDOTS ) == BROWSER_VLINESDOTS;

	WinBits nVScrollWinBits =
		WB_VSCROLL | ( ( nMode & BROWSER_THUMBDRAGGING ) ? WB_DRAG : 0 );
	pVScroll = ( nMode & BROWSER_TRACKING_TIPS ) == BROWSER_TRACKING_TIPS
				? new BrowserScrollBar( this, nVScrollWinBits,
										(BrowserDataWin*) pDataWin )
				: new ScrollBar( this, nVScrollWinBits );
	pVScroll->SetLineSize( 1 );
	pVScroll->SetPageSize(1);
	pVScroll->SetScrollHdl( LINK( this, BrowseBox, ScrollHdl ) );
	pVScroll->SetEndScrollHdl( LINK( this, BrowseBox, EndScrollHdl ) );

	getDataWindow()->bAutoSizeLastCol =
			BROWSER_AUTOSIZE_LASTCOL == ( nMode & BROWSER_AUTOSIZE_LASTCOL );
	getDataWindow()->bOwnDataChangedHdl =
			BROWSER_OWN_DATACHANGED == ( nMode & BROWSER_OWN_DATACHANGED );

	// Headerbar erzeugen, was passiert, wenn eine erzeugt werden mu� und schon Spalten bestehen ?
	if ( BROWSER_HEADERBAR_NEW == ( nMode & BROWSER_HEADERBAR_NEW ) )
	{
		if (!getDataWindow()->pHeaderBar)
			getDataWindow()->pHeaderBar = CreateHeaderBar( this );
	}
	else
	{
		DELETEZ(getDataWindow()->pHeaderBar);
	}



	if ( bColumnCursor )
	{
		pColSel = pOldColSel ? pOldColSel : new MultiSelection;
		pColSel->SetTotalRange( Range( 0, pCols->Count()-1 ) );
	}
	else
	{
		pColSel = 0;
		delete pColSel;
	}

	if ( bMultiSelection )
	{
		if ( pOldRowSel )
			uRow.pSel = pOldRowSel;
		else
			uRow.pSel = new MultiSelection;
	}
	else
	{
		uRow.nSel = nOldRowSel;
		delete pOldRowSel;
	}

	if ( bBootstrapped )
	{
		StateChanged( STATE_CHANGE_INITSHOW );
		if ( bMultiSelection && !pOldRowSel &&
			 nOldRowSel != BROWSER_ENDOFSELECTION )
			uRow.pSel->Select( nOldRowSel );
	}

	if ( pDataWin )
		pDataWin->Invalidate();

	// kein Cursor auf Handle-Column
	if ( nCurColId == 0 )
		nCurColId = GetColumnId( 1 );

	m_nCurrentMode = nMode;
}

//-------------------------------------------------------------------

void BrowseBox::VisibleRowsChanged( long, sal_uInt16 )
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

	// Das alte Verhalten: NumRows automatisch korrigieren:
	if ( nRowCount < GetRowCount() )
	{
		RowInserted(nRowCount,GetRowCount() - nRowCount,sal_False);
	}
	else if ( nRowCount > GetRowCount() )
	{
		RowRemoved(nRowCount-(nRowCount - GetRowCount()),nRowCount - GetRowCount(),sal_False);
	}
}

//-------------------------------------------------------------------

sal_Bool BrowseBox::IsCursorMoveAllowed( long, sal_uInt16 ) const

/*	[Beschreibung]

	Diese virtuelle Methode wird immer gerufen bevor der Cursor direkt
	bewegt werden soll. Durch 'return sal_False' kann verhindert werden, da\s
	dies geschieht, wenn z.B. ein Datensatz irgendwelchen Rules widerspricht.

	Diese Methode wird nicht gerufen, wenn die Cursorbewegung durch
	ein L"oschen oder Einf"ugen (einer Zeile/Spalte) ausgel"ost wird, also
	genaugenommen nur eine Cursor-Korrektur vorliegt.

	Die Basisimplementierung liefert derzeit immer sal_True.
*/

{
	return sal_True;
}

//-------------------------------------------------------------------

long BrowseBox::GetDataRowHeight() const
{
	return CalcZoom(nDataRowHeight ? nDataRowHeight : ImpGetDataRowHeight());
}

//-------------------------------------------------------------------

Window& BrowseBox::GetEventWindow() const
{
	return *getDataWindow()->pEventWin;
}

//-------------------------------------------------------------------

BrowserHeader* BrowseBox::CreateHeaderBar( BrowseBox* pParent )
{
	BrowserHeader* pNewBar = new BrowserHeader( pParent );
	pNewBar->SetStartDragHdl( LINK( this, BrowseBox, StartDragHdl ) );
	return pNewBar;
}

void BrowseBox::SetHeaderBar( BrowserHeader* pHeaderBar )
{
	delete ( (BrowserDataWin*)pDataWin )->pHeaderBar;
	( (BrowserDataWin*)pDataWin )->pHeaderBar = pHeaderBar;
	( (BrowserDataWin*)pDataWin )->pHeaderBar->SetStartDragHdl( LINK( this, BrowseBox, StartDragHdl ) );
}
//-------------------------------------------------------------------

#ifdef DBG_UTIL
const char* BrowseBoxCheckInvariants( const void * pVoid )
{
	const BrowseBox * p = (const BrowseBox *)pVoid;

	if (p->nRowCount < 0) return "BrowseBox: nRowCount < 0";
	if (p->nTopRow < 0) return "BrowseBox: nTopRow < 0";
	if (p->nTopRow >= p->nRowCount && p->nRowCount != 0) return "BrowseBox: nTopRow >= nRowCount && nRowCount != 0";
	if (p->nCurRow < -1) return "BrowseBox: nCurRow < -1";
	if (p->nCurRow > p->nRowCount) return "BrowseBox: nCurRow > nRowCount";

	// Leider waehrend der Bearbeitung nicht immer der Fall:
	//if (p->nCurRow < 0 && p->nRowCount != 0) return "nCurRow < 0 && nRowCount != 0";
	//if (p->nCurRow >= p->nRowCount && p->nRowCount != 0) return "nCurRow >= nRowCount && nRowCount != 0";

	return NULL;
}
#endif

//-------------------------------------------------------------------
long BrowseBox::GetTitleHeight() const
{
	long nHeight;
	// ask the header bar for the text height (if possible), as the header bar's font is adjusted with
	// our (and the header's) zoom factor
	HeaderBar* pHeaderBar = ( (BrowserDataWin*)pDataWin )->pHeaderBar;
	if ( pHeaderBar )
		nHeight = pHeaderBar->GetTextHeight();
	else
		nHeight = GetTextHeight();

	return nTitleLines ? nTitleLines * nHeight + 4 : 0;
}

//-------------------------------------------------------------------
long BrowseBox::CalcReverseZoom(long nVal)
{
	if (IsZoom())
	{
		const Fraction& rZoom = GetZoom();
		double n = (double)nVal;
		n *= (double)rZoom.GetDenominator();
		n /= (double)rZoom.GetNumerator();
		nVal = n>0 ? (long)(n + 0.5) : -(long)(-n + 0.5);
	}

	return nVal;
}

//-------------------------------------------------------------------
HeaderBar* BrowseBox::GetHeaderBar() const
{
	return getDataWindow()->pHeaderBar;
}
//-------------------------------------------------------------------

void BrowseBox::CursorMoved()
{
	// before implementing more here, please adjust the EditBrowseBox
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    if ( isAccessibleAlive() && HasFocus() )
		commitTableEvent(
            ACTIVE_DESCENDANT_CHANGED,
            makeAny( CreateAccessibleCell( GetCurRow(),GetColumnPos( GetCurColumnId() ) ) ),
            Any()
        );
}

//-------------------------------------------------------------------

void BrowseBox::LoseFocus()
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);
	DBG_TRACE1( "BrowseBox: %p->LoseFocus", this );

	if ( bHasFocus )
	{
		DBG_TRACE1( "BrowseBox: %p->HideCursor", this );
		DoHideCursor( "LoseFocus" );

		if ( !bKeepHighlight )
		{
			ToggleSelection();
			bSelectionIsVisible = sal_False;
		}

		bHasFocus = sal_False;
	}
	Control::LoseFocus();
}

//-------------------------------------------------------------------

void BrowseBox::GetFocus()
{
	DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);
	DBG_TRACE1( "BrowseBox: %p->GetFocus", this );

	if ( !bHasFocus )
	{
		if ( !bSelectionIsVisible )
		{
			bSelectionIsVisible = sal_True;
			if ( bBootstrapped )
				ToggleSelection();
		}

		bHasFocus = sal_True;
		DoShowCursor( "GetFocus" );
	}
	Control::GetFocus();
}


