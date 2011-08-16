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



// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <editeng/eeitem.hxx>

#include <sfx2/app.hxx>
//CHINA001 #include <svx/zoom.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/passwd.hxx>
#include <sfx2/request.hxx>
#include <svl/ptitem.hxx>
#include <svl/stritem.hxx>
#include <tools/urlobj.hxx>
#include <sfx2/objface.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/vclenum.hxx>

#include "globstr.hrc"
#include "scmod.hxx"
#include "appoptio.hxx"
#include "tabvwsh.hxx"
#include "document.hxx"
#include "sc.hrc"
#include "inputwin.hxx"
#include "scresid.hxx"
#include "printfun.hxx"
#include "docsh.hxx"
#include "rangelst.hxx"
#include "prevwsh.hxx"
#include "rangeutl.hxx"
#include "reffact.hxx"
#include "uiitems.hxx"
#include "cell.hxx"
#include "inputhdl.hxx"
//CHINA001 #include "scendlg.hxx"
//CHINA001 #include "mtrindlg.hxx"
#include "autoform.hxx"
#include "autofmt.hxx"
#include "dwfunctr.hxx"
#include "shtabdlg.hxx"
#include "tabprotection.hxx"
#include "protectiondlg.hxx"

#include <svl/ilstitem.hxx>
#define _SVSTDARR_ULONGS
#include <svl/svstdarr.hxx>

#include <svx/zoomslideritem.hxx>
#include <svx/svxdlg.hxx> //CHINA001
#include <svx/dialogs.hrc> //CHINA001
#include "scabstdlg.hxx" //CHINA001

#include <memory>

using ::std::auto_ptr;

#define IS_EDITMODE() GetViewData()->HasEditView( GetViewData()->GetActivePart() )
#define IS_AVAILABLE(WhichId,ppItem) \
    (pReqArgs->GetItemState((WhichId), sal_True, ppItem ) == SFX_ITEM_SET)
#define GET_STRING(nid) ((const SfxStringItem&)pReqArgs->Get(nid)).GetValue()
#define GET_UINT16(nid) ((const SfxUInt16Item&)pReqArgs->Get(nid)).GetValue()
#define GET_BOOL(nid)   ((const SfxBoolItem&)pReqArgs->Get(nid)).GetValue()
#define RECALC_PAGE(pDocSh) ScPrintFunc( pDocSh, GetPrinter(), nCurTab ).UpdatePages()

//------------------------------------------------------------------

/** Try to parse the given range using Calc-style syntax first, then
    Excel-style if that fails. */
sal_uInt16 lcl_ParseRange(ScRange& rScRange, const String& aAddress, ScDocument* pDoc, sal_uInt16 /* nSlot */)
{
    sal_uInt16 nResult = rScRange.Parse(aAddress, pDoc);
    if ( (nResult & SCA_VALID) )
        return nResult;

    return rScRange.Parse(aAddress, pDoc, ScAddress::Details(formula::FormulaGrammar::CONV_XL_A1, 0, 0));
}

/** Try to parse the given address using Calc-style syntax first, then
    Excel-style if that fails. */
sal_uInt16 lcl_ParseAddress(ScAddress& rScAddress, const String& aAddress, ScDocument* pDoc, sal_uInt16 /* nSlot */)
{
    sal_uInt16 nResult = rScAddress.Parse(aAddress, pDoc);
    if ( (nResult & SCA_VALID) )
        return nResult;

    return rScAddress.Parse(aAddress, pDoc, ScAddress::Details(formula::FormulaGrammar::CONV_XL_A1, 0, 0));
}

void ScTabViewShell::Execute( SfxRequest& rReq )
{
	SfxViewFrame*		pThisFrame  = GetViewFrame();
	SfxBindings&		rBindings	= pThisFrame->GetBindings();
	ScModule*			pScMod		= SC_MOD();
	const SfxItemSet*	pReqArgs	= rReq.GetArgs();
	sal_uInt16				nSlot		= rReq.GetSlot();

	if (nSlot != SID_CURRENTCELL)		// der kommt beim MouseButtonUp
		HideListBox();					// Autofilter-DropDown-Listbox

	switch ( nSlot )
	{
		case FID_INSERT_FILE:
			{
				const SfxPoolItem* pItem;
				if ( pReqArgs &&
					 pReqArgs->GetItemState(FID_INSERT_FILE,sal_True,&pItem) == SFX_ITEM_SET )
				{
					String aFileName = ((const SfxStringItem*)pItem)->GetValue();

						// Einfuege-Position

					Point aInsertPos;
					if ( pReqArgs->GetItemState(FN_PARAM_1,sal_True,&pItem) == SFX_ITEM_SET )
						aInsertPos = ((const SfxPointItem*)pItem)->GetValue();
					else
						aInsertPos = GetInsertPos();

						//	als Link?

					sal_Bool bAsLink = sal_False;
					if ( pReqArgs->GetItemState(FN_PARAM_2,sal_True,&pItem) == SFX_ITEM_SET )
						bAsLink = ((const SfxBoolItem*)pItem)->GetValue();

						// ausfuehren

					PasteFile( aInsertPos, aFileName, bAsLink );
				}
			}
			break;

		case SID_OPENDLG_EDIT_PRINTAREA:
			{
				sal_uInt16			nId  = ScPrintAreasDlgWrapper::GetChildWindowId();
				SfxChildWindow* pWnd = pThisFrame->GetChildWindow( nId );

				pScMod->SetRefDialog( nId, pWnd ? sal_False : sal_True );
			}
			break;

		case SID_CHANGE_PRINTAREA:
			{
				if ( pReqArgs )			// OK aus Dialog
				{
                    String aPrintStr;
                    String aRowStr;
                    String aColStr;
                    sal_Bool bEntire = sal_False;
                    const SfxPoolItem* pItem;
                    if ( pReqArgs->GetItemState( SID_CHANGE_PRINTAREA, sal_True, &pItem ) == SFX_ITEM_SET )
                        aPrintStr = static_cast<const SfxStringItem*>(pItem)->GetValue();
                    if ( pReqArgs->GetItemState( FN_PARAM_2, sal_True, &pItem ) == SFX_ITEM_SET )
                        aRowStr = static_cast<const SfxStringItem*>(pItem)->GetValue();
                    if ( pReqArgs->GetItemState( FN_PARAM_3, sal_True, &pItem ) == SFX_ITEM_SET )
                        aColStr = static_cast<const SfxStringItem*>(pItem)->GetValue();
                    if ( pReqArgs->GetItemState( FN_PARAM_4, sal_True, &pItem ) == SFX_ITEM_SET )
                        bEntire = static_cast<const SfxBoolItem*>(pItem)->GetValue();

                    SetPrintRanges( bEntire, &aPrintStr, &aColStr, &aRowStr, sal_False );

					rReq.Done();
				}
			}
			break;

		case SID_ADD_PRINTAREA:
		case SID_DEFINE_PRINTAREA:		// Menue oder Basic
			{
				sal_Bool bAdd = ( nSlot == SID_ADD_PRINTAREA );
				if ( pReqArgs )
				{
                    String aPrintStr;
                    const SfxPoolItem* pItem;
                    if ( pReqArgs->GetItemState( SID_DEFINE_PRINTAREA, sal_True, &pItem ) == SFX_ITEM_SET )
                        aPrintStr = static_cast<const SfxStringItem*>(pItem)->GetValue();
                    SetPrintRanges( sal_False, &aPrintStr, NULL, NULL, bAdd );
				}
				else
				{
                    SetPrintRanges( sal_False, NULL, NULL, NULL, bAdd );      // aus Selektion
					rReq.Done();
				}
			}
			break;

		case SID_DELETE_PRINTAREA:
			{
				String aEmpty;
                SetPrintRanges( sal_False, &aEmpty, NULL, NULL, sal_False );        // Druckbereich loeschen
				rReq.Done();
			}
			break;

		case FID_DEL_MANUALBREAKS:
			RemoveManualBreaks();
			rReq.Done();
			break;

		case FID_ADJUST_PRINTZOOM:
			AdjustPrintZoom();
			rReq.Done();
			break;

		case FID_RESET_PRINTZOOM:
			SetPrintZoom( 100, 0 );		// 100%, nicht auf Seiten
			rReq.Done();
			break;

		case SID_FORMATPAGE:
		case SID_STATUS_PAGESTYLE:
		case SID_HFEDIT:
			GetViewData()->GetDocShell()->
				ExecutePageStyle( *this, rReq, GetViewData()->GetTabNo() );
			break;

		case SID_JUMPTOMARK:
		case SID_CURRENTCELL:
			if ( pReqArgs )
			{
				String aAddress;
				const SfxPoolItem* pItem;
				if ( pReqArgs->GetItemState( nSlot, sal_True, &pItem ) == SFX_ITEM_SET )
					aAddress = ((const SfxStringItem*)pItem)->GetValue();
				else if ( nSlot == SID_JUMPTOMARK && pReqArgs->GetItemState(
											SID_JUMPTOMARK, sal_True, &pItem ) == SFX_ITEM_SET )
					aAddress = ((const SfxStringItem*)pItem)->GetValue();

				//	#i14927# SID_CURRENTCELL with a single cell must unmark if FN_PARAM_1
				//	isn't set (for recorded macros, because IsAPI is no longer available).
				//	ScGridWindow::MouseButtonUp no longer executes the slot for a single
				//	cell if there is a multi selection.
				sal_Bool bUnmark = ( nSlot == SID_CURRENTCELL );
				if ( pReqArgs->GetItemState( FN_PARAM_1, sal_True, &pItem ) == SFX_ITEM_SET )
					bUnmark = ((const SfxBoolItem*)pItem)->GetValue();

				if ( nSlot == SID_JUMPTOMARK )
				{
					//	#106586# URL has to be decoded for escaped characters (%20)
					aAddress = INetURLObject::decode( aAddress, INET_HEX_ESCAPE,
							   				INetURLObject::DECODE_WITH_CHARSET,
											RTL_TEXTENCODING_UTF8 );
				}

				sal_Bool bFound = sal_False;
				ScViewData* pViewData = GetViewData();
				ScDocument* pDoc	  = pViewData->GetDocument();
				ScMarkData& rMark	  = pViewData->GetMarkData();
				ScRange		aScRange;
				ScAddress	aScAddress;
                sal_uInt16      nResult = lcl_ParseRange(aScRange, aAddress, pDoc, nSlot);
				SCTAB		nTab = pViewData->GetTabNo();
				sal_Bool		bMark = sal_True;

                // Is this a range ?
				if( nResult & SCA_VALID )
				{
					if ( nResult & SCA_TAB_3D )
					{
						if( aScRange.aStart.Tab() != nTab )
							SetTabNo( nTab = aScRange.aStart.Tab() );
					}
					else
					{
						aScRange.aStart.SetTab( nTab );
						aScRange.aEnd.SetTab( nTab );
					}
				}
                // Is this a cell ?
                else if ( (nResult = lcl_ParseAddress(aScAddress, aAddress, pDoc, nSlot)) & SCA_VALID )
				{
					if ( nResult & SCA_TAB_3D )
					{
						if( aScAddress.Tab() != nTab )
							SetTabNo( nTab = aScAddress.Tab() );
					}
					else
						aScAddress.SetTab( nTab );

					aScRange = ScRange( aScAddress, aScAddress );
					// Zellen sollen nicht markiert werden
					bMark = sal_False;
				}
				// Ist es benahmster Bereich (erst Namen dann DBBereiche) ?
				else
				{
					ScRangeUtil		aRangeUtil;
                    formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();
					if(	aRangeUtil.MakeRangeFromName( aAddress, pDoc, nTab, aScRange, RUTL_NAMES, eConv ) ||
						aRangeUtil.MakeRangeFromName( aAddress, pDoc, nTab, aScRange, RUTL_DBASE, eConv ) )
					{
						nResult |= SCA_VALID;
						if( aScRange.aStart.Tab() != nTab )
							SetTabNo( nTab = aScRange.aStart.Tab() );
					}
				}

				if ( !(nResult & SCA_VALID) &&
						ByteString(aAddress, RTL_TEXTENCODING_ASCII_US).IsNumericAscii() )
				{
					sal_Int32 nNumeric = aAddress.ToInt32();
					if ( nNumeric > 0 && nNumeric <= MAXROW+1 )
					{
						//	1-basierte Zeilennummer

						aScAddress.SetRow( (SCROW)(nNumeric - 1) );
						aScAddress.SetCol( pViewData->GetCurX() );
						aScAddress.SetTab( nTab );
						aScRange = ScRange( aScAddress, aScAddress );
						bMark	 = sal_False;
						nResult  = SCA_VALID;
					}
				}

				if ( !ValidRow(aScRange.aStart.Row()) || !ValidRow(aScRange.aEnd.Row()) )
					nResult = 0;

				// wir haben was gefunden
				if( nResult & SCA_VALID )
				{
					bFound = sal_True;
					SCCOL nCol = aScRange.aStart.Col();
					SCROW nRow = aScRange.aStart.Row();
					sal_Bool bNothing = ( pViewData->GetCurX()==nCol && pViewData->GetCurY()==nRow );

					// markieren
					if( bMark )
					{
						if (rMark.IsMarked())			// ist derselbe Bereich schon markiert?
						{
							ScRange aOldMark;
							rMark.GetMarkArea( aOldMark );
							aOldMark.Justify();
							ScRange aCurrent = aScRange;
							aCurrent.Justify();
							bNothing = ( aCurrent == aOldMark );
						}
						else
							bNothing = sal_False;

						if (!bNothing)
							MarkRange( aScRange, sal_False );	// Cursor kommt hinterher...
					}
					else
					{
						//	remove old selection, unless bUnmark argument is sal_False (from navigator)
						if( bUnmark )
						{
							MoveCursorAbs( nCol, nRow,
								SC_FOLLOW_NONE, sal_False, sal_False );
						}
					}

					// und Cursor setzen

					// zusammengefasste Zellen beruecksichtigen:
					while ( pDoc->IsHorOverlapped( nCol, nRow, nTab ) )		//! ViewData !!!
						--nCol;
					while ( pDoc->IsVerOverlapped( nCol, nRow, nTab ) )
						--nRow;

					//	Navigator-Aufrufe sind nicht API!!!

					if( bNothing )
					{
						if (rReq.IsAPI())
							rReq.Ignore();		// wenn Makro, dann gar nix
						else
							rReq.Done();		// sonst wenigstens aufzeichnen
					}
					else
					{
						pViewData->ResetOldCursor();
						SetCursor( nCol, nRow );
						AlignToCursor( nCol, nRow, SC_FOLLOW_JUMP );
						rBindings.Invalidate( SID_CURRENTCELL );
						rBindings.Update( nSlot );

						if (!rReq.IsAPI())
							rReq.Done();
					}

					rReq.SetReturnValue( SfxStringItem( SID_CURRENTCELL, aAddress ) );
				}

				if (!bFound)	// kein gueltiger Bereich
				{
					//	wenn es ein Tabellenname ist, umschalten (fuer Navigator/URL's)

					SCTAB nNameTab;
					if ( pDoc->GetTable( aAddress, nNameTab ) )
					{
						bFound = sal_True;
						if ( nNameTab != nTab )
							SetTabNo( nNameTab );
					}
				}

				if ( !bFound && nSlot == SID_JUMPTOMARK )
				{
					//	Grafik-Objekte probieren (nur bei URL's)

					bFound = SelectObject( aAddress );
				}

				if (!bFound && !rReq.IsAPI())
					ErrorMessage( STR_ERR_INVALID_AREA );
			}
			break;

		case SID_CURRENTOBJECT:
			if ( pReqArgs )
			{
				String aName = ((const SfxStringItem&)pReqArgs->Get(nSlot)).GetValue();
				SelectObject( aName );
			}
			break;

		case SID_CURRENTTAB:
			if ( pReqArgs )
			{
				//	Tabelle fuer Basic ist 1-basiert
				SCTAB nTab = ((const SfxUInt16Item&)pReqArgs->Get(nSlot)).GetValue() - 1;
				ScDocument* pDoc = GetViewData()->GetDocument();
				if ( nTab < pDoc->GetTableCount() )
				{
					SetTabNo( nTab );
					rBindings.Update( nSlot );

					if( ! rReq.IsAPI() )
						rReq.Done();
				}
				//!	sonst Fehler ?
			}
			break;

		case SID_CURRENTDOC:
			if ( pReqArgs )
			{
				String aStrDocName( ((const SfxStringItem&)pReqArgs->
										Get(nSlot)).GetValue() );

				SfxViewFrame*	pViewFrame = NULL;
				ScDocShell* 	pDocSh = (ScDocShell*)SfxObjectShell::GetFirst();
				sal_Bool			bFound = sal_False;

				// zu aktivierenden ViewFrame suchen

				while ( pDocSh && !bFound )
				{
					if ( pDocSh->GetTitle() == aStrDocName )
					{
						pViewFrame = SfxViewFrame::GetFirst( pDocSh );
						bFound = ( NULL != pViewFrame );
					}

					pDocSh = (ScDocShell*)SfxObjectShell::GetNext( *pDocSh );
				}

				if ( bFound )
					pViewFrame->GetFrame().Appear();

				rReq.Ignore();//XXX wird von SFX erledigt
			}

		case SID_ATTR_SIZE://XXX ???
			break;


		case SID_PRINTPREVIEW:
			{
                if ( !pThisFrame->GetFrame().IsInPlace() )          // nicht bei OLE
				{
					//	print preview is now always in the same frame as the tab view
					//	-> always switch this frame back to normal view
					//	(ScPreviewShell ctor reads view data)

                    // #102785#; finish input
                    pScMod->InputEnterHandler();

                    pThisFrame->GetDispatcher()->Execute( SID_VIEWSHELL1, SFX_CALLMODE_ASYNCHRON );
				}
				//	else Fehler (z.B. Ole)
			}
			break;

		case SID_DETECTIVE_DEL_ALL:
			DetectiveDelAll();
			rReq.Done();
			break;

		//	SID_TABLE_ACTIVATE und SID_MARKAREA werden von Basic aus an der versteckten
		//	View aufgerufen, um auf der sichtbaren View zu markieren/umzuschalten:

		case SID_TABLE_ACTIVATE:
			DBG_ERROR("old slot SID_TABLE_ACTIVATE");
			break;

		case SID_REPAINT:
			PaintGrid();
			PaintTop();
			PaintLeft();
			PaintExtras();
			rReq.Done();
			break;

        case FID_NORMALVIEWMODE:
		case FID_PAGEBREAKMODE:
			{
                sal_Bool bWantPageBreak = nSlot == FID_PAGEBREAKMODE;

                // check whether there is an explicit argument, use it
				const SfxPoolItem* pItem;
                if ( pReqArgs && pReqArgs->GetItemState(nSlot, sal_True, &pItem) == SFX_ITEM_SET )
                {
                    sal_Bool bItemValue = ((const SfxBoolItem*)pItem)->GetValue();
                    bWantPageBreak = (nSlot == FID_PAGEBREAKMODE) == bItemValue;
                }

                if( GetViewData()->IsPagebreakMode() != bWantPageBreak )
                {
                    SetPagebreakMode( bWantPageBreak );
                    UpdatePageBreakData();
                    SetCurSubShell( GetCurObjectSelectionType(), sal_True );
                    PaintGrid();
                    PaintTop();
                    PaintLeft();
                    rBindings.Invalidate( nSlot );
                    rReq.AppendItem( SfxBoolItem( nSlot, sal_True ) );
                    rReq.Done();
                }
			}
			break;

		case FID_FUNCTION_BOX:
			{
                sal_uInt16 nChildId = ScFunctionChildWindow::GetChildWindowId();
				if ( rReq.GetArgs() )
                    pThisFrame->SetChildWindow( nChildId, ((const SfxBoolItem&) (rReq.GetArgs()->Get(FID_FUNCTION_BOX))).GetValue());
				else
                {
                    pThisFrame->ToggleChildWindow( nChildId );
                    rReq.AppendItem( SfxBoolItem( FID_FUNCTION_BOX , pThisFrame->HasChildWindow( nChildId ) ) );
                }

				GetViewFrame()->GetBindings().Invalidate(FID_FUNCTION_BOX);
				rReq.Done ();
			}
			break;


		case FID_TOGGLESYNTAX:
			{
				sal_Bool bSet = !GetViewData()->IsSyntaxMode();
				const SfxPoolItem* pItem;
				if ( pReqArgs && pReqArgs->GetItemState(nSlot, sal_True, &pItem) == SFX_ITEM_SET )
					bSet = ((const SfxBoolItem*)pItem)->GetValue();
				GetViewData()->SetSyntaxMode( bSet );
				PaintGrid();
				rBindings.Invalidate( FID_TOGGLESYNTAX );
                rReq.AppendItem( SfxBoolItem( nSlot, bSet ) );
				rReq.Done();
			}
			break;
		case FID_TOGGLEHEADERS:
			{
				sal_Bool bSet = !GetViewData()->IsHeaderMode();
				const SfxPoolItem* pItem;
				if ( pReqArgs && pReqArgs->GetItemState(nSlot, sal_True, &pItem) == SFX_ITEM_SET )
					bSet = ((const SfxBoolItem*)pItem)->GetValue();
				GetViewData()->SetHeaderMode( bSet );
				RepeatResize();
				rBindings.Invalidate( FID_TOGGLEHEADERS );
                rReq.AppendItem( SfxBoolItem( nSlot, bSet ) );
				rReq.Done();
			}
			break;

        case FID_TOGGLEFORMULA:
            {
                ScViewData* pViewData = GetViewData();
                const ScViewOptions& rOpts = pViewData->GetOptions();
                sal_Bool bFormulaMode = !rOpts.GetOption( VOPT_FORMULAS );
                const SfxPoolItem *pItem;
                if( pReqArgs && pReqArgs->GetItemState(nSlot, sal_True, &pItem) == SFX_ITEM_SET )
                    bFormulaMode = ((const SfxBoolItem *)pItem)->GetValue();

                ScViewOptions rSetOpts = ScViewOptions( rOpts );
                rSetOpts.SetOption( VOPT_FORMULAS, bFormulaMode );
                pViewData->SetOptions( rSetOpts );

                pViewData->GetDocShell()->PostPaintGridAll();

                rBindings.Invalidate( FID_TOGGLEFORMULA );
                rReq.AppendItem( SfxBoolItem( nSlot, bFormulaMode ) );
                rReq.Done();
            }
            break;

		case FID_TOGGLEINPUTLINE:
			{
				sal_uInt16			nId  = ScInputWindowWrapper::GetChildWindowId();
				SfxChildWindow* pWnd = pThisFrame->GetChildWindow( nId );
				sal_Bool bSet = ( pWnd == NULL );
				const SfxPoolItem* pItem;
				if ( pReqArgs && pReqArgs->GetItemState(nSlot, sal_True, &pItem) == SFX_ITEM_SET )
					bSet = ((const SfxBoolItem*)pItem)->GetValue();

				pThisFrame->SetChildWindow( nId, bSet );
				rBindings.Invalidate( FID_TOGGLEINPUTLINE );
                rReq.AppendItem( SfxBoolItem( nSlot, bSet ) );
                rReq.Done();
			}
			break;

		case SID_ATTR_ZOOM: // Statuszeile
		case FID_SCALE:
			{
                sal_Bool bSyncZoom = SC_MOD()->GetAppOptions().GetSynchronizeZoom();
				SvxZoomType	eOldZoomType = GetZoomType();
				SvxZoomType	eNewZoomType = eOldZoomType;
				const Fraction& rOldY = GetViewData()->GetZoomY();	// Y wird angezeigt
				sal_uInt16 nOldZoom = (sal_uInt16)(( rOldY.GetNumerator() * 100 )
											/ rOldY.GetDenominator());
				sal_uInt16 nZoom = nOldZoom;
				sal_Bool bCancel = sal_False;

				if ( pReqArgs )
				{
					const SvxZoomItem& rZoomItem = (const SvxZoomItem&)
												   pReqArgs->Get(SID_ATTR_ZOOM);

					eNewZoomType = rZoomItem.GetType();
					nZoom	  = rZoomItem.GetValue();
				}
				else
				{
					SfxItemSet		aSet	 ( GetPool(), SID_ATTR_ZOOM, SID_ATTR_ZOOM );
					SvxZoomItem		aZoomItem( eOldZoomType, nOldZoom, SID_ATTR_ZOOM );
					//CHINA001 SvxZoomDialog*	pDlg = NULL;
					AbstractSvxZoomDialog* pDlg = NULL;
					ScMarkData&		rMark = GetViewData()->GetMarkData();
					sal_uInt16			nBtnFlags =   SVX_ZOOM_ENABLE_50
												| SVX_ZOOM_ENABLE_75
												| SVX_ZOOM_ENABLE_100
												| SVX_ZOOM_ENABLE_150
												| SVX_ZOOM_ENABLE_200
												| SVX_ZOOM_ENABLE_WHOLEPAGE
												| SVX_ZOOM_ENABLE_PAGEWIDTH;

					if ( rMark.IsMarked() || rMark.IsMultiMarked() )
						nBtnFlags = nBtnFlags | SVX_ZOOM_ENABLE_OPTIMAL;

					aZoomItem.SetValueSet( nBtnFlags );
					aSet.Put( aZoomItem );
					//CHINA001 pDlg = new SvxZoomDialog( GetDialogParent(), aSet );
					SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
					if(pFact)
					{
						pDlg = pFact->CreateSvxZoomDialog(GetDialogParent(), aSet );
						DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001
					}
					pDlg->SetLimits( MINZOOM, MAXZOOM );

					bCancel = ( RET_CANCEL == pDlg->Execute() );

					if ( !bCancel )
					{
						const SvxZoomItem&	rZoomItem = (const SvxZoomItem&)
												pDlg->GetOutputItemSet()->
													Get( SID_ATTR_ZOOM );

						eNewZoomType = rZoomItem.GetType();
						nZoom	  = rZoomItem.GetValue();
					}

					delete pDlg;
				}

				if ( !bCancel )
				{
					if ( eNewZoomType == SVX_ZOOM_PERCENT )
					{
						if ( nZoom < MINZOOM )	nZoom = MINZOOM;
						if ( nZoom > MAXZOOM )	nZoom = MAXZOOM;
					}
					else
					{
						nZoom = CalcZoom( eNewZoomType, nOldZoom );
						bCancel = nZoom == 0;
					}

					switch ( eNewZoomType )
					{
						case SVX_ZOOM_WHOLEPAGE:
						case SVX_ZOOM_PAGEWIDTH:
                            SetZoomType( eNewZoomType, bSyncZoom );
							break;

						default:
                            SetZoomType( SVX_ZOOM_PERCENT, bSyncZoom );
					}
				}

				if ( nZoom != nOldZoom && !bCancel )
				{
					if (!GetViewData()->IsPagebreakMode())
					{
						ScAppOptions aNewOpt = pScMod->GetAppOptions();
						aNewOpt.SetZoom( nZoom );
						aNewOpt.SetZoomType( GetZoomType() );
						pScMod->SetAppOptions( aNewOpt );
					}
					Fraction aFract( nZoom, 100 );
                    SetZoom( aFract, aFract, bSyncZoom );
					PaintGrid();
					PaintTop();
					PaintLeft();
					rBindings.Invalidate( SID_ATTR_ZOOM );
                    rReq.AppendItem( SvxZoomItem( GetZoomType(), nZoom, nSlot ) );
					rReq.Done();
				}
			}
			break;

        case SID_ATTR_ZOOMSLIDER:
            {
                const SfxPoolItem* pItem = NULL;
                sal_Bool bSyncZoom = SC_MOD()->GetAppOptions().GetSynchronizeZoom();
                if ( pReqArgs && pReqArgs->GetItemState(SID_ATTR_ZOOMSLIDER, sal_True, &pItem) == SFX_ITEM_SET )
                {
                    const sal_uInt16 nCurrentZoom = ((const SvxZoomSliderItem *)pItem)->GetValue();
                    if( nCurrentZoom )
                    {
                        SetZoomType( SVX_ZOOM_PERCENT, bSyncZoom );
                        if (!GetViewData()->IsPagebreakMode())
                        {
                            ScAppOptions aNewOpt = pScMod->GetAppOptions();
                            aNewOpt.SetZoom( nCurrentZoom );
                            aNewOpt.SetZoomType( GetZoomType() );
                            pScMod->SetAppOptions( aNewOpt );
                        }
                        Fraction aFract( nCurrentZoom,100 );
                        SetZoom( aFract, aFract, bSyncZoom );
                        PaintGrid();
                        PaintTop();
                        PaintLeft();
                        rBindings.Invalidate( SID_ATTR_ZOOMSLIDER );
                        rReq.Done();
                    }
                }
            }
            break;

		//----------------------------------------------------------------

		case FID_TAB_SELECTALL:
			SelectAllTables();
			rReq.Done();
			break;

        case FID_TAB_DESELECTALL:
            DeselectAllTables();
            rReq.Done();
            break;

        case SID_SELECT_TABLES:
        {
            ScViewData& rViewData = *GetViewData();
            ScDocument& rDoc = *rViewData.GetDocument();
            ScMarkData& rMark = rViewData.GetMarkData();
            SCTAB nTabCount = rDoc.GetTableCount();
            SCTAB nTab;

            SvULongs aIndexList( 4, 4 );
            SFX_REQUEST_ARG( rReq, pItem, SfxIntegerListItem, SID_SELECT_TABLES, sal_False );
            if ( pItem )
                pItem->GetList( aIndexList );
            else
            {
                //CHINA001 ScShowTabDlg* pDlg = new ScShowTabDlg( GetDialogParent() );
				ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
				DBG_ASSERT(pFact, "ScAbstractFactory create fail!");//CHINA001

				AbstractScShowTabDlg* pDlg = pFact->CreateScShowTabDlg( GetDialogParent(), RID_SCDLG_SHOW_TAB);
				DBG_ASSERT(pDlg, "Dialog create fail!");//CHINA001
                pDlg->SetDescription(
                    String( ScResId( STR_DLG_SELECTTABLES_TITLE ) ),
                    String( ScResId( STR_DLG_SELECTTABLES_LBNAME ) ),
                    GetStaticInterface()->GetSlot(SID_SELECT_TABLES)->GetCommand(), HID_SELECTTABLES );

                // fill all table names with selection state
                String aTabName;
                for( nTab = 0; nTab < nTabCount; ++nTab )
                {
                    rDoc.GetName( nTab, aTabName );
                    pDlg->Insert( aTabName, rMark.GetTableSelect( nTab ) );
                }

                if( pDlg->Execute() == RET_OK )
                {
                    sal_uInt16 nSelCount = pDlg->GetSelectEntryCount();
                    sal_uInt16 nSelIx;
                    for( nSelIx = 0; nSelIx < nSelCount; ++nSelIx )
                        aIndexList.Insert( pDlg->GetSelectEntryPos( nSelIx ), nSelIx );
                    delete pDlg;
                    rReq.AppendItem( SfxIntegerListItem( SID_SELECT_TABLES, aIndexList ) );
                }
                else
                    rReq.Ignore();
            }

            if ( aIndexList.Count() )
            {
                sal_uInt16 nSelCount = aIndexList.Count();
                sal_uInt16 nSelIx;
                SCTAB nFirstVisTab = 0;

                // special case: only hidden tables selected -> do nothing
                sal_Bool bVisSelected = sal_False;
                for( nSelIx = 0; !bVisSelected && (nSelIx < nSelCount); ++nSelIx )
                    bVisSelected = rDoc.IsVisible( nFirstVisTab = static_cast<SCTAB>(aIndexList[nSelIx]) );
                if( !bVisSelected )
                    nSelCount = 0;

                // select the tables
                if( nSelCount )
                {
                    for( nTab = 0; nTab < nTabCount; ++nTab )
                        rMark.SelectTable( nTab, sal_False );

                    for( nSelIx = 0; nSelIx < nSelCount; ++nSelIx )
                        rMark.SelectTable( static_cast<SCTAB>(aIndexList[nSelIx]), sal_True );

                    // activate another table, if current is deselected
                    if( !rMark.GetTableSelect( rViewData.GetTabNo() ) )
                    {
                        rMark.SelectTable( nFirstVisTab, sal_True );
                        SetTabNo( nFirstVisTab );
                    }

                    rViewData.GetDocShell()->PostPaintExtras();
                    SfxBindings& rBind = rViewData.GetBindings();
                    rBind.Invalidate( FID_FILL_TAB );
                    rBind.Invalidate( FID_TAB_DESELECTALL );
                }

                rReq.Done();
            }
        }
        break;


		case SID_OUTLINE_DELETEALL:
			RemoveAllOutlines();
			rReq.Done();
			break;

		case SID_AUTO_OUTLINE:
			AutoOutline();
			rReq.Done();
			break;


		case SID_WINDOW_SPLIT:
			{
				ScSplitMode eHSplit = GetViewData()->GetHSplitMode();
				ScSplitMode eVSplit = GetViewData()->GetVSplitMode();
				if ( eHSplit == SC_SPLIT_NORMAL || eVSplit == SC_SPLIT_NORMAL )		// aufheben
					RemoveSplit();
				else if ( eHSplit == SC_SPLIT_FIX || eVSplit == SC_SPLIT_FIX )		// normal
					FreezeSplitters( sal_False );
				else																// erzeugen
					SplitAtCursor();
				rReq.Done();

				InvalidateSplit();
			}
			break;

		case SID_WINDOW_FIX:
			{
				ScSplitMode eHSplit = GetViewData()->GetHSplitMode();
				ScSplitMode eVSplit = GetViewData()->GetVSplitMode();
				if ( eHSplit == SC_SPLIT_FIX || eVSplit == SC_SPLIT_FIX )			// aufheben
					RemoveSplit();
				else
					FreezeSplitters( sal_True );		// erzeugen oder fixieren
				rReq.Done();

				InvalidateSplit();
			}
			break;

		//	----------------------------------------------------------------

		case FID_CHG_SHOW:
			{
				sal_uInt16			nId  = ScHighlightChgDlgWrapper::GetChildWindowId();
				SfxChildWindow* pWnd = pThisFrame->GetChildWindow( nId );

				pScMod->SetRefDialog( nId, pWnd ? sal_False : sal_True );
			}
			break;

		case FID_CHG_ACCEPT:
			{
				pThisFrame->ToggleChildWindow(ScAcceptChgDlgWrapper::GetChildWindowId());
				GetViewFrame()->GetBindings().Invalidate(FID_CHG_ACCEPT);
				rReq.Done ();

				/*
				sal_uInt16			nId  = ScAcceptChgDlgWrapper::GetChildWindowId();
				SfxChildWindow* pWnd = pThisFrame->GetChildWindow( nId );

				pScMod->SetRefDialog( nId, pWnd ? sal_False : sal_True );
				*/
			}
			break;

		case FID_CHG_COMMENT:
			{
				ScViewData* pData = GetViewData();
				ScAddress aCursorPos( pData->GetCurX(), pData->GetCurY(), pData->GetTabNo() );
				ScDocShell* pDocSh = pData->GetDocShell();

				ScChangeAction*	pAction = pDocSh->GetChangeAction( aCursorPos );
				if ( pAction )
				{
					const SfxPoolItem* pItem;
					if ( pReqArgs &&
						 pReqArgs->GetItemState( nSlot, sal_True, &pItem ) == SFX_ITEM_SET &&
						 pItem->ISA( SfxStringItem ) )
					{
						String aComment = ((const SfxStringItem*)pItem)->GetValue();
						pDocSh->SetChangeComment( pAction, aComment );
						rReq.Done();
					}
					else
					{
						pDocSh->ExecuteChangeCommentDialog( pAction, GetDialogParent() );
						rReq.Done();
					}
				}
			}
			break;

		case SID_CREATE_SW_DRAWVIEW:
			//	wird von den Forms gerufen, wenn die DrawView mit allem Zubehoer
			//	angelegt werden muss
			if (!GetScDrawView())
			{
				GetViewData()->GetDocShell()->MakeDrawLayer();
				rBindings.InvalidateAll(sal_False);
			}
			break;

		case FID_PROTECT_DOC:
			{
				ScDocument* 		pDoc = GetViewData()->GetDocument();
				SfxPasswordDialog*	pDlg;

				if( pReqArgs )
				{
					const SfxPoolItem* pItem;
                    if( IS_AVAILABLE( FID_PROTECT_DOC, &pItem ) &&
                        ((const SfxBoolItem*)pItem)->GetValue() == pDoc->IsDocProtected() )
                    {
                        rReq.Ignore();
                        break;
                    }
                }

                ScDocProtection* pProtect = pDoc->GetDocProtection();
                if (pProtect && pProtect->isProtected())
				{
					sal_Bool	bCancel = sal_False;
					String	aPassword;

                    if (pProtect->isProtectedWithPass())
					{
						String	aText( ScResId(SCSTR_PASSWORD) );

						pDlg = new SfxPasswordDialog( 	GetDialogParent(), &aText );
						pDlg->SetText( ScResId(SCSTR_UNPROTECTDOC) );
						pDlg->SetMinLen( 0 );
						pDlg->SetHelpId( GetStaticInterface()->GetSlot(FID_PROTECT_DOC)->GetCommand() );
						pDlg->SetEditHelpId( HID_PASSWD_DOC );

						if (pDlg->Execute() == RET_OK)
							aPassword = pDlg->GetPassword();
						else
							bCancel = sal_True;
						delete pDlg;
					}
					if (!bCancel)
					{
						Unprotect( TABLEID_DOC, aPassword );
                        rReq.AppendItem( SfxBoolItem( FID_PROTECT_DOC, sal_False ) );
						rReq.Done();
					}
				}
				else
				{
					String aText( ScResId(SCSTR_PASSWORDOPT) );

					pDlg = new SfxPasswordDialog( 	GetDialogParent(), &aText );
					pDlg->SetText( ScResId(SCSTR_PROTECTDOC) );
					pDlg->SetMinLen( 0 );
    				pDlg->SetHelpId( GetStaticInterface()->GetSlot(FID_PROTECT_DOC)->GetCommand() );
					pDlg->SetEditHelpId( HID_PASSWD_DOC );
					pDlg->ShowExtras( SHOWEXTRAS_CONFIRM );

					if (pDlg->Execute() == RET_OK)
					{
						String aPassword = pDlg->GetPassword();
                        Protect( TABLEID_DOC, aPassword );
                        rReq.AppendItem( SfxBoolItem( FID_PROTECT_DOC, sal_True ) );
                        rReq.Done();
					}

					delete pDlg;
				}
				rBindings.Invalidate( FID_PROTECT_DOC );
			}
			break;


		case FID_PROTECT_TABLE:
        {
            ScDocument* pDoc = GetViewData()->GetDocument();
            SCTAB		nTab = GetViewData()->GetTabNo();
            bool        bOldProtection = pDoc->IsTabProtected(nTab);

#if ENABLE_SHEET_PROTECTION

            if( pReqArgs )
            {
                const SfxPoolItem* pItem;
                bool bNewProtection = !bOldProtection;
                if( IS_AVAILABLE( FID_PROTECT_TABLE, &pItem ) )
                    bNewProtection = ((const SfxBoolItem*)pItem)->GetValue();
                if( bNewProtection == bOldProtection )
                {
                    rReq.Ignore();
                    break;
                }
            }

            if (bOldProtection)
            {
                // Unprotect a protected sheet.

                ScTableProtection* pProtect = pDoc->GetTabProtection(nTab);
                if (pProtect && pProtect->isProtectedWithPass())
                {
                    String aText( ScResId(SCSTR_PASSWORDOPT) );
                    auto_ptr<SfxPasswordDialog> pDlg(new SfxPasswordDialog(GetDialogParent(), &aText));
                    pDlg->SetText( ScResId(SCSTR_UNPROTECTTAB) );
                    pDlg->SetMinLen( 0 );
					pDlg->SetHelpId( GetStaticInterface()->GetSlot(FID_PROTECT_TABLE)->GetCommand() );
                    pDlg->SetEditHelpId( HID_PASSWD_TABLE );

                    if (pDlg->Execute() == RET_OK)
                    {
                        String aPassword = pDlg->GetPassword();
                        Unprotect(nTab, aPassword);
                    }
                }
                else
                    // this sheet is not password-protected.
                    Unprotect(nTab, String());

                if (!pReqArgs)
                {
                    rReq.AppendItem( SfxBoolItem(FID_PROTECT_TABLE, false) );
                    rReq.Done();
                }
            }
            else
            {
                // Protect a current sheet.

                auto_ptr<ScTableProtectionDlg> pDlg(new ScTableProtectionDlg(GetDialogParent()));

                ScTableProtection* pProtect = pDoc->GetTabProtection(nTab);
                if (pProtect)
                    pDlg->SetDialogData(*pProtect);

                if (pDlg->Execute() == RET_OK)
                {
                    pScMod->InputEnterHandler();

                    ScTableProtection aNewProtect;
                    pDlg->WriteData(aNewProtect);
                    ProtectSheet(nTab, aNewProtect);
                    if (!pReqArgs)
                    {
                        rReq.AppendItem( SfxBoolItem(FID_PROTECT_TABLE, true) );
                        rReq.Done();
                    }
                }
            }
#else
            auto_ptr<SfxPasswordDialog> pDlg;
            String				aPassword;
            sal_Bool				bCancel = sal_False;
            bool                bNewProtection = ! bOldProtection;

            if( pReqArgs )
            {
                const SfxPoolItem* pItem;
                if( IS_AVAILABLE( FID_PROTECT_TABLE, &pItem ) )
                    bNewProtection = ((const SfxBoolItem*)pItem)->GetValue();
                if( bNewProtection == bOldProtection )
                {
                    rReq.Ignore();
                    break;
                }
            }

            if ( bOldProtection)
            {
                // Unprotect a protected sheet.

                ScTableProtection* pProtect = pDoc->GetTabProtection(nTab);
                if (pProtect && pProtect->isProtectedWithPass())
                {
                    String aText( ScResId(SCSTR_PASSWORDOPT) );
                    pDlg.reset(new SfxPasswordDialog(GetDialogParent(), &aText));
                    pDlg->SetText( ScResId(SCSTR_UNPROTECTTAB) );
                    pDlg->SetMinLen( 0 );
					pDlg->SetHelpId( GetStaticInterface()->GetSlot(FID_PROTECT_TABLE)->GetCommand() );
                    pDlg->SetEditHelpId( HID_PASSWD_TABLE );

                    if (pDlg->Execute() == RET_OK)
                        aPassword = pDlg->GetPassword();
                    else
                        bCancel = sal_True;
                }

                if (!pReqArgs)
                {
                    rReq.AppendItem( SfxBoolItem(FID_PROTECT_TABLE, false) );
                    rReq.Done();
                }
            }
            else
            {
                String aText( ScResId(SCSTR_PASSWORDOPT) );

                pDlg.reset(new SfxPasswordDialog(GetDialogParent(), &aText));
                pDlg->SetText( ScResId(SCSTR_PROTECTTAB) );
                pDlg->SetMinLen( 0 );
                pDlg->SetHelpId( GetStaticInterface()->GetSlot(FID_PROTECT_TABLE)->GetCommand() );
                pDlg->SetEditHelpId( HID_PASSWD_TABLE );
                pDlg->ShowExtras( SHOWEXTRAS_CONFIRM );

                if (pDlg->Execute() == RET_OK)
                    aPassword = pDlg->GetPassword();
                else
                    bCancel = sal_True;
            }

            if( !bCancel )
            {
                if ( bOldProtection )
                    Unprotect( nTab, aPassword );
                else
                {
                    pScMod->InputEnterHandler();

                    Protect( nTab, aPassword );
                }

                if( !pReqArgs )
                {
                    rReq.AppendItem( SfxBoolItem( FID_PROTECT_TABLE, bNewProtection ) );
                    rReq.Done();
                }
            }
#endif
            TabChanged();
            UpdateInputHandler(true);   // damit sofort wieder eingegeben werden kann
            SelectionChanged();
        }
        break;

        case SID_OPT_LOCALE_CHANGED :
            {   // locale changed, SYSTEM number formats changed => repaint cell contents
                PaintGrid();
				rReq.Done();
			}
            break;

		default:
			DBG_ERROR("Unbekannter Slot bei ScTabViewShell::Execute");
			break;
	}
}



