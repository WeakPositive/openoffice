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
#ifndef SC_VIEWDATA_HXX
#define SC_VIEWDATA_HXX

#include <svx/zoomitem.hxx>
#include "scdllapi.h"
#include "viewopti.hxx"
#include "markdata.hxx"


// ---------------------------------------------------------------------------

#define	SC_SLIDER_SIZE		2
#define SC_SMALL3DSHADOW	COL_BLACK
#define SC_SIZE_NONE		65535
const SCCOL SC_TABSTART_NONE = SCCOL_MAX;

#define SC_FILL_NONE		0
#define SC_FILL_FILL		1
#define SC_FILL_EMBED_LT	2
#define SC_FILL_EMBED_RB	3
#define SC_FILL_MATRIX		4

enum ScSplitMode { SC_SPLIT_NONE = 0, SC_SPLIT_NORMAL, SC_SPLIT_FIX };

enum ScSplitPos { SC_SPLIT_TOPLEFT, SC_SPLIT_TOPRIGHT, SC_SPLIT_BOTTOMLEFT, SC_SPLIT_BOTTOMRIGHT };
enum ScHSplitPos { SC_SPLIT_LEFT, SC_SPLIT_RIGHT };
enum ScVSplitPos { SC_SPLIT_TOP, SC_SPLIT_BOTTOM };

inline ScHSplitPos WhichH( ScSplitPos ePos );
inline ScVSplitPos WhichV( ScSplitPos ePos );
inline ScSplitPos Which( ScHSplitPos eHPos );
inline ScSplitPos Which( ScVSplitPos eVPos );

//	Bildschirmverhalten bei Cursorbewegungen:
enum ScFollowMode { SC_FOLLOW_NONE, SC_FOLLOW_LINE, SC_FOLLOW_FIX, SC_FOLLOW_JUMP };

//	Mausmodi um Bereiche zu selektieren
enum ScRefType { SC_REFTYPE_NONE, SC_REFTYPE_REF, SC_REFTYPE_FILL,
					SC_REFTYPE_EMBED_LT, SC_REFTYPE_EMBED_RB };

/** States GetSimpleArea() returns for the underlying selection marks, so the
    caller can react if the result is not of type SC_MARK_SIMPLE. */
enum ScMarkType
{
    SC_MARK_NONE            = 0,    // Not returned by GetSimpleArea(), used internally.
                                    // Nothing marked always results in the
                                    // current cursor position being selected and a simple mark.
    SC_MARK_SIMPLE          = 1,    // Simple rectangular area marked, no filtered rows.
    SC_MARK_FILTERED        = 2,    // At least one mark contains filtered rows.
    SC_MARK_SIMPLE_FILTERED =       // Simple rectangular area marked containing filtered rows.
        SC_MARK_SIMPLE |
        SC_MARK_FILTERED,  // 3
    SC_MARK_MULTI           = 4     // Multiple selection marks.
    /* TODO: if filtered multi-selection was implemented, this would be the value to use. */
#if 0
        ,
    SC_MARK_MULTI_FILTERED  =       // Multiple selection marks containing filtered rows.
        SC_MARK_MULTI |
        SC_MARK_FILTERED   // 6
#endif
};

class ScDocShell;
class ScDocument;
class ScDBFunc;
class ScTabViewShell;
class ScDrawView;
class ScEditEngineDefaulter;
class EditView;
class EditStatus;
class Outliner;
class Window;
class SfxObjectShell;
class SfxBindings;
class SfxDispatcher;
class ScPatternAttr;
class ScRangeListRef;
class ScExtDocOptions;
class ScViewData;

//--------------------------------------------------------------------------

class ScViewDataTable							// Daten pro Tabelle
{
friend class ScViewData;
private:
    SvxZoomType     eZoomType;                  // selected zoom type (normal view)
    Fraction        aZoomX;                     // selected zoom X
    Fraction        aZoomY;                     // selected zoom Y (displayed)
    Fraction        aPageZoomX;                 // zoom in page break preview mode
    Fraction        aPageZoomY;

	long			nTPosX[2];					// MapMode - Offset (Twips)
	long			nTPosY[2];
	long			nMPosX[2];					// MapMode - Offset (1/100 mm)
	long			nMPosY[2];
	long			nPixPosX[2];				// Offset in Pixeln
	long			nPixPosY[2];
	long			nHSplitPos;
	long			nVSplitPos;

	ScSplitMode		eHSplitMode;
	ScSplitMode		eVSplitMode;
	ScSplitPos		eWhichActive;

	SCCOL			nFixPosX;					// Zellposition des Splitters beim Fixieren
	SCROW			nFixPosY;

	SCCOL			nCurX;
	SCROW			nCurY;
	SCCOL			nOldCurX;
	SCROW			nOldCurY;
	SCCOL			nPosX[2];
	SCROW			nPosY[2];

	sal_Bool			bOldCurValid;				// "virtuelle" Cursorpos. bei zusammengefassten

					ScViewDataTable();
					~ScViewDataTable();

    void            WriteUserDataSequence(
                        com::sun::star::uno::Sequence <com::sun::star::beans::PropertyValue>& rSettings,
                        const ScViewData& rViewData, SCTAB nTab );

    void            ReadUserDataSequence(
                        const com::sun::star::uno::Sequence <com::sun::star::beans::PropertyValue>& rSettings,
                        ScViewData& rViewData, SCTAB nTab, bool& rHasZoom);
};

// ---------------------------------------------------------------------------

class SC_DLLPUBLIC ScViewData
{
private:
	double				nPPTX, nPPTY;				// Scaling-Faktoren

	ScViewDataTable*	pTabData[MAXTABCOUNT];
	ScViewDataTable*	pThisTab;					// Daten der angezeigten Tabelle
	ScDocShell*			pDocShell;
	ScDocument*			pDoc;
	ScDBFunc*			pView;
	ScTabViewShell*		pViewShell;
	EditView*			pEditView[4];				// gehoert dem Fenster
	ScViewOptions*		pOptions;
	EditView*			pSpellingView;

	ScMarkData			aMarkData;

	long				nEditMargin;

	Size				aScenButSize;				// Groesse eines Szenario-Buttons

	Size				aScrSize;
	MapMode				aLogicMode;					// skalierter 1/100mm-MapMode

    SvxZoomType         eDefZoomType;               // default zoom and type for missing TabData
    Fraction            aDefZoomX;
    Fraction            aDefZoomY;
    Fraction            aDefPageZoomX;              // zoom in page break preview mode
    Fraction            aDefPageZoomY;

	ScRefType			eRefType;

	SCTAB				nTabNo;						// angezeigte Tabelle
	SCTAB				nRefTabNo;					// Tabelle auf die sich RefInput bezieht
	SCCOL				nRefStartX;
	SCROW				nRefStartY;
	SCTAB				nRefStartZ;
	SCCOL				nRefEndX;
	SCROW				nRefEndY;
	SCTAB				nRefEndZ;
	SCCOL				nFillStartX;				// Fill-Cursor
	SCROW				nFillStartY;
	SCCOL				nFillEndX;
	SCROW				nFillEndY;
	SCCOL				nEditCol;					// Position dazu
	SCROW				nEditRow;
	SCCOL				nEditStartCol;
	SCCOL				nEditEndCol;				// Ende der Edit-View
	SCROW				nEditEndRow;
	SCCOL				nTabStartCol;				// fuer Enter nach Tab
	ScRange				aDelRange;					// fuer AutoFill-Loeschen

    ScSplitPos          eEditActivePart;            // the part that was active when edit mode was started
	sal_Bool				bEditActive[4];				// aktiv?
	sal_Bool				bActive;					// aktives Fenster ?
	sal_Bool				bIsRefMode;					// Referenzeingabe
	sal_Bool				bDelMarkValid;				// nur gueltig bei SC_REFTYPE_FILL
	sal_uInt8				nFillMode;					// Modus
	sal_Bool				bPagebreak;					// Seitenumbruch-Vorschaumodus

	sal_Bool				bSelCtrlMouseClick;         // special selection handling for ctrl-mouse-click

	SC_DLLPRIVATE DECL_LINK (EmptyEditHdl, EditStatus*);
	SC_DLLPRIVATE DECL_LINK (EditEngineHdl, EditStatus*);

	SC_DLLPRIVATE void			CalcPPT();
    SC_DLLPRIVATE void          CreateTabData( SCTAB nNewTab );
    SC_DLLPRIVATE void          CreateTabData( std::vector< SCTAB >& rvTabs );
    SC_DLLPRIVATE void          CreateSelectedTabData();

public:
					ScViewData( ScDocShell* pDocSh, ScTabViewShell* pViewSh );
					ScViewData( const ScViewData& rViewData );
					~ScViewData();

	void			InitData( ScDocument* pDocument );
//UNUSED2008-05  void            InitFrom( const ScViewData* pRef );
//UNUSED2008-05  void			SetDocShell( ScDocShell* pShell );


	ScDocShell*		GetDocShell() const		{ return pDocShell; }
	ScDBFunc*		GetView() const			{ return pView; }
	ScTabViewShell*	GetViewShell() const	{ return pViewShell; }

	SfxObjectShell* GetSfxDocShell() const;
	SfxBindings&	GetBindings();			// from ViewShell's ViewFrame
	SfxDispatcher&	GetDispatcher();		// from ViewShell's ViewFrame

	ScMarkData&		GetMarkData()			{ return aMarkData; }
    const ScMarkData& GetMarkData() const   { return aMarkData; }

	Window*			GetDialogParent();			// von tabvwsh weitergeleitet
	Window*			GetActiveWin();				// von View
	ScDrawView*		GetScDrawView();			// von View
	sal_Bool			IsMinimized();				// von View

	void			UpdateInputHandler( sal_Bool bForce = sal_False, sal_Bool bStopEditing = sal_True );

	void			WriteUserData(String& rData);
	void			ReadUserData(const String& rData);
    void            WriteExtOptions( ScExtDocOptions& rOpt ) const;
    void            ReadExtOptions( const ScExtDocOptions& rOpt );
	void			WriteUserDataSequence(com::sun::star::uno::Sequence <com::sun::star::beans::PropertyValue>& rSettings);
	void			ReadUserDataSequence(const com::sun::star::uno::Sequence <com::sun::star::beans::PropertyValue>& rSettings);

	ScDocument*		GetDocument() const;

	void			SetViewShell( ScTabViewShell* pViewSh );

	sal_Bool			IsActive() const			{ return bActive; }
	void			Activate(sal_Bool bActivate)	{ bActive = bActivate; }

	void			UpdateThis();

	void			InsertTab( SCTAB nTab );
	void			DeleteTab( SCTAB nTab );
	void			CopyTab( SCTAB nSrcTab, SCTAB nDestTab );
	void			MoveTab( SCTAB nSrcTab, SCTAB nDestTab );

	SCTAB			GetRefTabNo() const 					{ return nRefTabNo; }
	void			SetRefTabNo( SCTAB nNewTab )			{ nRefTabNo = nNewTab; }

	SCTAB			GetTabNo() const						{ return nTabNo; }
	ScSplitPos		GetActivePart() const					{ return pThisTab->eWhichActive; }
	SCCOL			GetPosX( ScHSplitPos eWhich ) const		{ return pThisTab->nPosX[eWhich]; }
	SCROW			GetPosY( ScVSplitPos eWhich ) const		{ return pThisTab->nPosY[eWhich]; }
	SCCOL			GetCurX() const							{ return pThisTab->nCurX; }
	SCROW			GetCurY() const							{ return pThisTab->nCurY; }
	sal_Bool			HasOldCursor() const					{ return pThisTab->bOldCurValid; }
	SCCOL			GetOldCurX() const;
	SCROW			GetOldCurY() const;
	ScSplitMode		GetHSplitMode() const					{ return pThisTab->eHSplitMode; }
	ScSplitMode		GetVSplitMode() const					{ return pThisTab->eVSplitMode; }
	long			GetHSplitPos() const					{ return pThisTab->nHSplitPos; }
	long			GetVSplitPos() const					{ return pThisTab->nVSplitPos; }
	SCCOL			GetFixPosX() const						{ return pThisTab->nFixPosX; }
	SCROW			GetFixPosY() const						{ return pThisTab->nFixPosY; }
	sal_Bool			IsPagebreakMode() const					{ return bPagebreak; }

	void			SetPosX( ScHSplitPos eWhich, SCCOL nNewPosX );
	void			SetPosY( ScVSplitPos eWhich, SCROW nNewPosY );
	void			SetCurX( SCCOL nNewCurX )						{ pThisTab->nCurX = nNewCurX; }
	void			SetCurY( SCROW nNewCurY )						{ pThisTab->nCurY = nNewCurY; }
	void			SetOldCursor( SCCOL nNewX, SCROW nNewY );
	void			ResetOldCursor();
	void			SetHSplitMode( ScSplitMode eMode )				{ pThisTab->eHSplitMode = eMode; }
	void			SetVSplitMode( ScSplitMode eMode )				{ pThisTab->eVSplitMode = eMode; }
	void			SetHSplitPos( long nPos )						{ pThisTab->nHSplitPos = nPos; }
	void			SetVSplitPos( long nPos )						{ pThisTab->nVSplitPos = nPos; }
	void			SetFixPosX( SCCOL nPos )						{ pThisTab->nFixPosX = nPos; }
	void			SetFixPosY( SCROW nPos )						{ pThisTab->nFixPosY = nPos; }
	void			SetPagebreakMode( sal_Bool bSet );

    void            SetZoomType( SvxZoomType eNew, sal_Bool bAll );
    void            SetZoomType( SvxZoomType eNew, std::vector< SCTAB >& tabs );
    void            SetZoom( const Fraction& rNewX, const Fraction& rNewY, std::vector< SCTAB >& tabs );
    void            SetZoom( const Fraction& rNewX, const Fraction& rNewY, sal_Bool bAll );
    void            RefreshZoom();

	void			SetSelCtrlMouseClick( sal_Bool bTmp ) { bSelCtrlMouseClick = bTmp; }

    SvxZoomType     GetZoomType() const     { return pThisTab->eZoomType; }
    const Fraction&	GetZoomX() const        { return bPagebreak ? pThisTab->aPageZoomX : pThisTab->aZoomX; }
    const Fraction&	GetZoomY() const        { return bPagebreak ? pThisTab->aPageZoomY : pThisTab->aZoomY; }

	const MapMode&	GetLogicMode( ScSplitPos eWhich );
	const MapMode&	GetLogicMode();						// Offset 0

	long			GetTPosX( ScHSplitPos eWhich ) const		{ return pThisTab->nTPosX[eWhich]; }
	long			GetTPosY( ScVSplitPos eWhich ) const		{ return pThisTab->nTPosY[eWhich]; }

	double			GetPPTX() const { return nPPTX; }
	double			GetPPTY() const	{ return nPPTY; }

	ScMarkType      GetSimpleArea( SCCOL& rStartCol, SCROW& rStartRow, SCTAB& rStartTab,
									SCCOL& rEndCol, SCROW& rEndRow, SCTAB& rEndTab ) const;
	ScMarkType      GetSimpleArea( ScRange& rRange ) const;
                    /// May modify rNewMark using MarkToSimple().
    ScMarkType      GetSimpleArea( ScRange & rRange, ScMarkData & rNewMark ) const;
	void			GetMultiArea( ScRangeListRef& rRange ) const;

	sal_Bool			SimpleColMarked();
	sal_Bool			SimpleRowMarked();

	sal_Bool			IsMultiMarked();

	void			SetFillMode( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow );
	void			SetDragMode( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
									sal_uInt8 nMode );
	void			GetFillData( SCCOL& rStartCol, SCROW& rStartRow,
								 SCCOL& rEndCol, SCROW& rEndRow );
	void			ResetFillMode();
	sal_Bool			IsAnyFillMode()				{ return nFillMode != SC_FILL_NONE; }
	sal_Bool			IsFillMode()				{ return nFillMode == SC_FILL_FILL; }
	sal_uInt8			GetFillMode()				{ return nFillMode; }

					// TRUE: Zelle ist zusammengefasst
	sal_Bool			GetMergeSizePixel( SCCOL nX, SCROW nY, long& rSizeXPix, long& rSizeYPix );
	sal_Bool			GetPosFromPixel( long nClickX, long nClickY, ScSplitPos eWhich,
										SCsCOL& rPosX, SCsROW& rPosY,
										sal_Bool bTestMerge = sal_True, sal_Bool bRepair = sal_False,
										sal_Bool bNextIfLarge = sal_True );
	void			GetMouseQuadrant( const Point& rClickPos, ScSplitPos eWhich,
										SCsCOL nPosX, SCsROW nPosY, sal_Bool& rLeft, sal_Bool& rTop );

	sal_Bool			IsRefMode() const						{ return bIsRefMode; }
	ScRefType		GetRefType() const						{ return eRefType; }
	SCCOL			GetRefStartX() const					{ return nRefStartX; }
	SCROW			GetRefStartY() const					{ return nRefStartY; }
	SCTAB			GetRefStartZ() const					{ return nRefStartZ; }
	SCCOL			GetRefEndX() const						{ return nRefEndX; }
	SCROW			GetRefEndY() const						{ return nRefEndY; }
	SCTAB			GetRefEndZ() const						{ return nRefEndZ; }

	void			SetRefMode( sal_Bool bNewMode, ScRefType eNewType )
									{ bIsRefMode = bNewMode; eRefType = eNewType; }

	void			SetRefStart( SCCOL nNewX, SCROW nNewY, SCTAB nNewZ )
							{ nRefStartX = nNewX; nRefStartY = nNewY; nRefStartZ = nNewZ; }
	void			SetRefEnd( SCCOL nNewX, SCROW nNewY, SCTAB nNewZ )
							{ nRefEndX = nNewX; nRefEndY = nNewY; nRefEndZ = nNewZ; }

	void			ResetDelMark()							{ bDelMarkValid = sal_False; }
	void			SetDelMark( const ScRange& rRange )
							{ aDelRange = rRange; bDelMarkValid = sal_True; }

	sal_Bool			GetDelMark( ScRange& rRange ) const
							{ rRange = aDelRange; return bDelMarkValid; }

	inline void		GetMoveCursor( SCCOL& rCurX, SCROW& rCurY );

	const ScViewOptions&	GetOptions() const { return *pOptions; }
	void					SetOptions( const ScViewOptions& rOpt );

	sal_Bool	IsGridMode		() const			{ return pOptions->GetOption( VOPT_GRID ); }
	void	SetGridMode		( sal_Bool bNewMode )	{ pOptions->SetOption( VOPT_GRID, bNewMode ); }
	sal_Bool	IsSyntaxMode	() const			{ return pOptions->GetOption( VOPT_SYNTAX ); }
	void	SetSyntaxMode	( sal_Bool bNewMode )	{ pOptions->SetOption( VOPT_SYNTAX, bNewMode ); }
	sal_Bool	IsHeaderMode	() const			{ return pOptions->GetOption( VOPT_HEADER ); }
	void	SetHeaderMode	( sal_Bool bNewMode )	{ pOptions->SetOption( VOPT_HEADER, bNewMode ); }
	sal_Bool	IsTabMode		() const			{ return pOptions->GetOption( VOPT_TABCONTROLS ); }
	void	SetTabMode		( sal_Bool bNewMode )	{ pOptions->SetOption( VOPT_TABCONTROLS, bNewMode ); }
	sal_Bool	IsVScrollMode	() const			{ return pOptions->GetOption( VOPT_VSCROLL ); }
	void	SetVScrollMode	( sal_Bool bNewMode )	{ pOptions->SetOption( VOPT_VSCROLL, bNewMode ); }
	sal_Bool	IsHScrollMode	() const			{ return pOptions->GetOption( VOPT_HSCROLL ); }
	void	SetHScrollMode	( sal_Bool bNewMode )	{ pOptions->SetOption( VOPT_HSCROLL, bNewMode ); }
	sal_Bool	IsOutlineMode	() const			{ return pOptions->GetOption( VOPT_OUTLINER ); }
	void	SetOutlineMode	( sal_Bool bNewMode )	{ pOptions->SetOption( VOPT_OUTLINER, bNewMode ); }

	void 			KillEditView();
	void			ResetEditView();
	void			SetEditEngine( ScSplitPos eWhich,
									ScEditEngineDefaulter* pNewEngine,
									Window* pWin, SCCOL nNewX, SCROW nNewY );
	void			GetEditView( ScSplitPos eWhich, EditView*& rViewPtr, SCCOL& rCol, SCROW& rRow );
	sal_Bool			HasEditView( ScSplitPos eWhich ) const
										{ return pEditView[eWhich] && bEditActive[eWhich]; }
	EditView*		GetEditView( ScSplitPos eWhich ) const
										{ return pEditView[eWhich]; }

	void			EditGrowX();
	void			EditGrowY( sal_Bool bInitial = sal_False );

    ScSplitPos      GetEditActivePart() const       { return eEditActivePart; }
	SCCOL			GetEditViewCol() const			{ return nEditCol; }
	SCROW			GetEditViewRow() const			{ return nEditRow; }
	SCCOL			GetEditStartCol() const			{ return nEditStartCol; }
	SCROW			GetEditStartRow() const			{ return nEditRow; }		// never editing above the cell
	SCCOL			GetEditEndCol() const			{ return nEditEndCol; }
	SCROW			GetEditEndRow() const			{ return nEditEndRow; }

	Rectangle		GetEditArea( ScSplitPos eWhich, SCCOL nPosX, SCROW nPosY, Window* pWin,
									const ScPatternAttr* pPattern, sal_Bool bForceToTop );

	void			SetTabNo( SCTAB nNewTab );
	void			SetActivePart( ScSplitPos eNewActive );

	Point			GetScrPos( SCCOL nWhereX, SCROW nWhereY, ScSplitPos eWhich,
								sal_Bool bAllowNeg = sal_False ) const;
	Point			GetScrPos( SCCOL nWhereX, SCROW nWhereY, ScHSplitPos eWhich ) const;
	Point			GetScrPos( SCCOL nWhereX, SCROW nWhereY, ScVSplitPos eWhich ) const;

	SCCOL			CellsAtX( SCsCOL nPosX, SCsCOL nDir, ScHSplitPos eWhichX, sal_uInt16 nScrSizeY = SC_SIZE_NONE ) const;
	SCROW			CellsAtY( SCsROW nPosY, SCsROW nDir, ScVSplitPos eWhichY, sal_uInt16 nScrSizeX = SC_SIZE_NONE ) const;

	SCCOL			VisibleCellsX( ScHSplitPos eWhichX ) const;		// angezeigte komplette Zellen
	SCROW			VisibleCellsY( ScVSplitPos eWhichY ) const;
	SCCOL			PrevCellsX( ScHSplitPos eWhichX ) const;		// Zellen auf der vorgehenden Seite
	SCROW			PrevCellsY( ScVSplitPos eWhichY ) const;
//UNUSED2008-05  SCCOL           LastCellsX( ScHSplitPos eWhichX ) const;        // Zellen auf der letzten Seite
//UNUSED2008-05  SCROW           LastCellsY( ScVSplitPos eWhichY ) const;

	sal_Bool			IsOle();
//UNUSED2008-05  void            UpdateOle( ScSplitPos eWhich );
	void			SetScreen( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 );
	void			SetScreen( const Rectangle& rVisArea );
	void			SetScreenPos( const Point& rVisAreaStart );

	void			UpdateScreenZoom( const Fraction& rNewX, const Fraction& rNewY );

	Size			GetScrSize() const				{ return aScrSize; }

	void			RecalcPixPos();
	Point			GetPixPos( ScSplitPos eWhich ) const
					{ return Point( pThisTab->nPixPosX[WhichH(eWhich)],
									pThisTab->nPixPosY[WhichV(eWhich)] ); }
	void 			SetSpellingView( EditView* pSpView) { pSpellingView = pSpView; }
	EditView*		GetSpellingView() const { return pSpellingView; }

	void			UpdateOutlinerFlags( Outliner& rOutl ) const;

	Point			GetMousePosPixel();

	sal_Bool			UpdateFixX(SCTAB nTab = MAXTAB+1);
	sal_Bool			UpdateFixY(SCTAB nTab = MAXTAB+1);

	SCCOL			GetTabStartCol() const			{ return nTabStartCol; }
	void			SetTabStartCol(SCCOL nNew)		{ nTabStartCol = nNew; }

	ScAddress		GetCurPos() const;

	const Size&		GetScenButSize() const				{ return aScenButSize; }
	void			SetScenButSize(const Size& rNew)	{ aScenButSize = rNew; }

	sal_Bool			IsSelCtrlMouseClick() { return bSelCtrlMouseClick; }

	static inline long ToPixel( sal_uInt16 nTwips, double nFactor );

    /** while (rScrY <= nEndPixels && rPosY <= nEndRow) add pixels of row 
        heights converted with nPPTY to rScrY, optimized for row height 
        segments. Upon return rPosY is the last row evaluated <= nEndRow, rScrY 
        may be > nEndPixels! 
     */
    static void     AddPixelsWhile( long & rScrY, long nEndPixels,
                                    SCROW & rPosY, SCROW nEndRow, double nPPTY,
                                    const ScDocument * pDoc, SCTAB nTabNo );

    /** while (rScrY <= nEndPixels && rPosY >= nStartRow) add pixels of row 
        heights converted with nPPTY to rScrY, optimized for row height 
        segments. Upon return rPosY is the last row evaluated >= nStartRow, 
        rScrY may be > nEndPixels! 
     */
    static void     AddPixelsWhileBackward( long & rScrY, long nEndPixels,
                                    SCROW & rPosY, SCROW nStartRow, double nPPTY,
                                    const ScDocument * pDoc, SCTAB nTabNo );
};


// ---------------------------------------------------------------------------

inline long ScViewData::ToPixel( sal_uInt16 nTwips, double nFactor )
{
	long nRet = (long)( nTwips * nFactor );
	if ( !nRet && nTwips )
		nRet = 1;
	return nRet;
}

inline void ScViewData::GetMoveCursor( SCCOL& rCurX, SCROW& rCurY )
{
	if ( bIsRefMode )
	{
		rCurX = nRefEndX;
		rCurY = nRefEndY;
	}
	else
	{
		rCurX = GetCurX();
		rCurY = GetCurY();
	}
}

inline ScHSplitPos WhichH( ScSplitPos ePos )
{
	return (ePos==SC_SPLIT_TOPLEFT || ePos==SC_SPLIT_BOTTOMLEFT) ?
				SC_SPLIT_LEFT : SC_SPLIT_RIGHT;
}

inline ScVSplitPos WhichV( ScSplitPos ePos )
{
	return (ePos==SC_SPLIT_TOPLEFT || ePos==SC_SPLIT_TOPRIGHT) ?
				SC_SPLIT_TOP : SC_SPLIT_BOTTOM;
}

inline ScSplitPos Which( ScHSplitPos eHPos )
{
	return (eHPos==SC_SPLIT_LEFT) ?
				SC_SPLIT_BOTTOMLEFT : SC_SPLIT_BOTTOMRIGHT;
}

inline ScSplitPos Which( ScVSplitPos eVPos )
{
	return (eVPos==SC_SPLIT_TOP) ?
				SC_SPLIT_TOPLEFT : SC_SPLIT_BOTTOMLEFT;
}



#endif

