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
#include "xihelper.hxx"
#include <svl/itemset.hxx>
#include <editeng/editobj.hxx>
#include <tools/urlobj.hxx>
#include "scitems.hxx"
#include <editeng/eeitem.hxx>
#include <editeng/flditem.hxx>
#include "document.hxx"
#include "cell.hxx"
#include "rangelst.hxx"
#include "editutil.hxx"
#include "attrib.hxx"
#include "xltracer.hxx"
#include "xistream.hxx"
#include "xistyle.hxx"

#include "excform.hxx"

// Excel->Calc cell address/range conversion ==================================

namespace {

/** Fills the passed Calc address with the passed Excel cell coordinates without checking any limits. */
inline void lclFillAddress( ScAddress& rScPos, sal_uInt16 nXclCol, sal_uInt16 nXclRow, SCTAB nScTab )
{
    rScPos.SetCol( static_cast< SCCOL >( nXclCol ) );
    rScPos.SetRow( static_cast< SCROW >( nXclRow ) );
    rScPos.SetTab( nScTab );
}

} // namespace

// ----------------------------------------------------------------------------

XclImpAddressConverter::XclImpAddressConverter( const XclImpRoot& rRoot ) :
    XclAddressConverterBase( rRoot.GetTracer(), rRoot.GetScMaxPos() )
{
}

// cell address ---------------------------------------------------------------

bool XclImpAddressConverter::CheckAddress( const XclAddress& rXclPos, bool bWarn )
{
    bool bValidCol = rXclPos.mnCol <= mnMaxCol;
    bool bValidRow = rXclPos.mnRow <= mnMaxRow;
    bool bValid = bValidCol && bValidRow;
    if( !bValid && bWarn )
    {
        mbColTrunc |= !bValidCol;
        mbRowTrunc |= !bValidRow;
        mrTracer.TraceInvalidAddress( ScAddress(
            static_cast< SCCOL >( rXclPos.mnCol ), static_cast< SCROW >( rXclPos.mnRow ), 0 ), maMaxPos );
    }
    return bValid;
}

bool XclImpAddressConverter::ConvertAddress( ScAddress& rScPos,
        const XclAddress& rXclPos, SCTAB nScTab, bool bWarn )
{
    bool bValid = CheckAddress( rXclPos, bWarn );
    if( bValid )
        lclFillAddress( rScPos, rXclPos.mnCol, rXclPos.mnRow, nScTab );
    return bValid;
}

ScAddress XclImpAddressConverter::CreateValidAddress(
        const XclAddress& rXclPos, SCTAB nScTab, bool bWarn )
{
    ScAddress aScPos( ScAddress::UNINITIALIZED );
    if( !ConvertAddress( aScPos, rXclPos, nScTab, bWarn ) )
    {
        aScPos.SetCol( static_cast< SCCOL >( ::std::min( rXclPos.mnCol, mnMaxCol ) ) );
        aScPos.SetRow( static_cast< SCROW >( ::std::min( rXclPos.mnRow, mnMaxRow ) ) );
        aScPos.SetTab( limit_cast< SCTAB >( nScTab, 0, maMaxPos.Tab() ) );
    }
    return aScPos;
}

// cell range -----------------------------------------------------------------

bool XclImpAddressConverter::CheckRange( const XclRange& rXclRange, bool bWarn )
{
    return CheckAddress( rXclRange.maFirst, bWarn ) && CheckAddress( rXclRange.maLast, bWarn );
}

bool XclImpAddressConverter::ConvertRange( ScRange& rScRange,
        const XclRange& rXclRange, SCTAB nScTab1, SCTAB nScTab2, bool bWarn )
{
    // check start position
    bool bValidStart = CheckAddress( rXclRange.maFirst, bWarn );
    if( bValidStart )
    {
        lclFillAddress( rScRange.aStart, rXclRange.maFirst.mnCol, rXclRange.maFirst.mnRow, nScTab1 );

        // check & correct end position
        sal_uInt16 nXclCol2 = rXclRange.maLast.mnCol;
        sal_uInt16 nXclRow2 = rXclRange.maLast.mnRow;
        if( !CheckAddress( rXclRange.maLast, bWarn ) )
        {
            nXclCol2 = ::std::min( nXclCol2, mnMaxCol );
            nXclRow2 = ::std::min( nXclRow2, mnMaxRow );
        }
        lclFillAddress( rScRange.aEnd, nXclCol2, nXclRow2, nScTab2 );
    }
    return bValidStart;
}

//UNUSED2009-05 ScRange XclImpAddressConverter::CreateValidRange(
//UNUSED2009-05         const XclRange& rXclRange, SCTAB nScTab1, SCTAB nScTab2, bool bWarn )
//UNUSED2009-05 {
//UNUSED2009-05     return ScRange(
//UNUSED2009-05         CreateValidAddress( rXclRange.maFirst, nScTab1, bWarn ),
//UNUSED2009-05         CreateValidAddress( rXclRange.maLast,  nScTab2, bWarn ) );
//UNUSED2009-05 }

// cell range list ------------------------------------------------------------

//UNUSED2009-05 bool XclImpAddressConverter::CheckRangeList( const XclRangeList& rXclRanges, bool bWarn )
//UNUSED2009-05 {
//UNUSED2009-05     for( XclRangeList::const_iterator aIt = rXclRanges.begin(), aEnd = rXclRanges.end(); aIt != aEnd; ++aIt )
//UNUSED2009-05         if( !CheckRange( *aIt, bWarn ) )
//UNUSED2009-05             return false;
//UNUSED2009-05     return true;
//UNUSED2009-05 }

void XclImpAddressConverter::ConvertRangeList( ScRangeList& rScRanges,
        const XclRangeList& rXclRanges, SCTAB nScTab, bool bWarn )
{
    rScRanges.RemoveAll();
    for( XclRangeList::const_iterator aIt = rXclRanges.begin(), aEnd = rXclRanges.end(); aIt != aEnd; ++aIt )
    {
        ScRange aScRange( ScAddress::UNINITIALIZED );
        if( ConvertRange( aScRange, *aIt, nScTab, nScTab, bWarn ) )
            rScRanges.Append( aScRange );
    }
}

// String->EditEngine conversion ==============================================

namespace {

EditTextObject* lclCreateTextObject( const XclImpRoot& rRoot,
        const XclImpString& rString, XclFontItemType eType, sal_uInt16 nXFIndex )
{
    EditTextObject* pTextObj = 0;

    const XclImpXFBuffer& rXFBuffer = rRoot.GetXFBuffer();
    const XclImpFont* pFirstFont = rXFBuffer.GetFont( nXFIndex );
    bool bFirstEscaped = pFirstFont && pFirstFont->HasEscapement();

    if( rString.IsRich() || bFirstEscaped )
    {
        const XclImpFontBuffer& rFontBuffer = rRoot.GetFontBuffer();
        const XclFormatRunVec& rFormats = rString.GetFormats();

        ScEditEngineDefaulter& rEE = (eType == EXC_FONTITEM_NOTE) ?
            static_cast< ScEditEngineDefaulter& >( rRoot.GetDoc().GetNoteEngine() ) : rRoot.GetEditEngine();
        rEE.SetText( rString.GetText() );

        SfxItemSet aItemSet( rEE.GetEmptyItemSet() );
        if( bFirstEscaped )
            rFontBuffer.FillToItemSet( aItemSet, eType, rXFBuffer.GetFontIndex( nXFIndex ) );
        ESelection aSelection;

        XclFormatRun aNextRun;
        XclFormatRunVec::const_iterator aIt = rFormats.begin();
        XclFormatRunVec::const_iterator aEnd = rFormats.end();

        if( aIt != aEnd )
            aNextRun = *aIt++;
        else
            aNextRun.mnChar = 0xFFFF;

        xub_StrLen nLen = rString.GetText().Len();
        for( sal_uInt16 nChar = 0; nChar < nLen; ++nChar )
        {
            // reached new different formatted text portion
            if( nChar >= aNextRun.mnChar )
            {
                // send items to edit engine
                rEE.QuickSetAttribs( aItemSet, aSelection );

                // start new item set
                aItemSet.ClearItem();
                rFontBuffer.FillToItemSet( aItemSet, eType, aNextRun.mnFontIdx );

                // read new formatting information
                if( aIt != aEnd )
                    aNextRun = *aIt++;
                else
                    aNextRun.mnChar = 0xFFFF;

                // reset selection start to current position
                aSelection.nStartPara = aSelection.nEndPara;
                aSelection.nStartPos = aSelection.nEndPos;
            }

            // set end of selection to current position
            if( rString.GetText().GetChar( nChar ) == '\n' )
            {
                ++aSelection.nEndPara;
                aSelection.nEndPos = 0;
            }
            else
                ++aSelection.nEndPos;
        }

        // send items of last text portion to edit engine
        rEE.QuickSetAttribs( aItemSet, aSelection );

        pTextObj = rEE.CreateTextObject();
    }

    return pTextObj;
}

} // namespace

EditTextObject* XclImpStringHelper::CreateTextObject(
        const XclImpRoot& rRoot, const XclImpString& rString )
{
    return lclCreateTextObject( rRoot, rString, EXC_FONTITEM_EDITENG, 0 );
}

//UNUSED2009-05 EditTextObject* XclImpStringHelper::CreateNoteObject(
//UNUSED2009-05         const XclImpRoot& rRoot, const XclImpString& rString )
//UNUSED2009-05 {
//UNUSED2009-05     return lclCreateTextObject( rRoot, rString, EXC_FONTITEM_NOTE, 0 );
//UNUSED2009-05 }

ScBaseCell* XclImpStringHelper::CreateCell(
        const XclImpRoot& rRoot, const XclImpString& rString, sal_uInt16 nXFIndex )
{
    ScBaseCell* pCell = 0;

    if( rString.GetText().Len() )
    {
        ::std::auto_ptr< EditTextObject > pTextObj( lclCreateTextObject( rRoot, rString, EXC_FONTITEM_EDITENG, nXFIndex ) );
        ScDocument& rDoc = rRoot.GetDoc();

        if( pTextObj.get() )
            // ScEditCell creates own copy of text object
            pCell = new ScEditCell( pTextObj.get(), &rDoc, rRoot.GetEditEngine().GetEditTextObjectPool() );
        else
            pCell = ScBaseCell::CreateTextCell( rString.GetText(), &rDoc );
    }

    return pCell;
}

// Header/footer conversion ===================================================

XclImpHFConverter::XclImpHFPortionInfo::XclImpHFPortionInfo() :
    mnHeight( 0 ),
    mnMaxLineHt( 0 )
{
    maSel.nStartPara = maSel.nEndPara = 0;
    maSel.nStartPos = maSel.nEndPos = 0;
}

// ----------------------------------------------------------------------------

XclImpHFConverter::XclImpHFConverter( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot ),
    mrEE( rRoot.GetHFEditEngine() ),
    mxFontData( new XclFontData ),
    meCurrObj( EXC_HF_CENTER )
{
}

XclImpHFConverter::~XclImpHFConverter()
{
}

void XclImpHFConverter::ParseString( const String& rHFString )
{
    // edit engine objects
    mrEE.SetText( EMPTY_STRING );
    maInfos.clear();
    maInfos.resize( EXC_HF_PORTION_COUNT );
    meCurrObj = EXC_HF_CENTER;

    // parser temporaries
    maCurrText.Erase();
    String aReadFont;           // current font name
    String aReadStyle;          // current font style
    sal_uInt16 nReadHeight = 0; // current font height
    ResetFontData();

    /** State of the parser. */
    enum XclHFParserState
    {
        xlPSText,           /// Read text, search for functions.
        xlPSFunc,           /// Read function (token following a '&').
        xlPSFont,           /// Read font name ('&' is followed by '"', reads until next '"' or ',').
        xlPSFontStyle,      /// Read font style name (font part after ',', reads until next '"').
        xlPSHeight          /// Read font height ('&' is followed by num. digits, reads until non-digit).
    } eState = xlPSText;

    const sal_Unicode* pChar = rHFString.GetBuffer();
    const sal_Unicode* pNull = pChar + rHFString.Len(); // pointer to teminating null char
    while( *pChar )
    {
        switch( eState )
        {

// --- read text character ---

            case xlPSText:
            {
                switch( *pChar )
                {
                    case '&':           // new command
                        InsertText();
                        eState = xlPSFunc;
                    break;
                    case '\n':          // line break
                        InsertText();
                        InsertLineBreak();
                    break;
                    default:
                        maCurrText += *pChar;
                }
            }
            break;

// --- read control sequence ---

            case xlPSFunc:
            {
                eState = xlPSText;
                switch( *pChar )
                {
                    case '&':   maCurrText += '&';  break;  // the '&' character

                    case 'L':   SetNewPortion( EXC_HF_LEFT );   break;  // Left portion
                    case 'C':   SetNewPortion( EXC_HF_CENTER ); break;  // Center portion
                    case 'R':   SetNewPortion( EXC_HF_RIGHT );  break;  // Right portion

                    case 'P':   InsertField( SvxFieldItem( SvxPageField(), EE_FEATURE_FIELD ) );      break;  // page
                    case 'N':   InsertField( SvxFieldItem( SvxPagesField(), EE_FEATURE_FIELD ) );     break;  // page count
                    case 'D':   InsertField( SvxFieldItem( SvxDateField(), EE_FEATURE_FIELD ) );      break;  // date
                    case 'T':   InsertField( SvxFieldItem( SvxTimeField(), EE_FEATURE_FIELD ) );      break;  // time
                    case 'A':   InsertField( SvxFieldItem( SvxTableField(), EE_FEATURE_FIELD ) );     break;  // table name

                    case 'Z':           // file path
                        InsertField( SvxFieldItem( SvxExtFileField(), EE_FEATURE_FIELD ) );   // convert to full name
                        if( (pNull - pChar >= 2) && (*(pChar + 1) == '&') && (*(pChar + 2) == 'F') )
                        {
                            // &Z&F found - ignore the &F part
                            pChar += 2;
                        }
                    break;
                    case 'F':           // file name
                        InsertField( SvxFieldItem( SvxExtFileField( EMPTY_STRING, SVXFILETYPE_VAR, SVXFILEFORMAT_NAME_EXT ), EE_FEATURE_FIELD ) );
                    break;

                    case 'U':           // underline
                        SetAttribs();
                        mxFontData->mnUnderline = (mxFontData->mnUnderline == EXC_FONTUNDERL_SINGLE) ?
                            EXC_FONTUNDERL_NONE : EXC_FONTUNDERL_SINGLE;
                    break;
                    case 'E':           // double underline
                        SetAttribs();
                        mxFontData->mnUnderline = (mxFontData->mnUnderline == EXC_FONTUNDERL_DOUBLE) ?
                            EXC_FONTUNDERL_NONE : EXC_FONTUNDERL_DOUBLE;
                    break;
                    case 'S':           // strikeout
                        SetAttribs();
                        mxFontData->mbStrikeout = !mxFontData->mbStrikeout;
                    break;
                    case 'X':           // superscript
                        SetAttribs();
                        mxFontData->mnEscapem = (mxFontData->mnEscapem == EXC_FONTESC_SUPER) ?
                            EXC_FONTESC_NONE : EXC_FONTESC_SUPER;
                    break;
                    case 'Y':           // subsrcipt
                        SetAttribs();
                        mxFontData->mnEscapem = (mxFontData->mnEscapem == EXC_FONTESC_SUB) ?
                            EXC_FONTESC_NONE : EXC_FONTESC_SUB;
                    break;

                    case '\"':          // font name
                        aReadFont.Erase();
                        aReadStyle.Erase();
                        eState = xlPSFont;
                    break;
                    default:
                        if( ('0' <= *pChar) && (*pChar <= '9') )    // font size
                        {
                            nReadHeight = *pChar - '0';
                            eState = xlPSHeight;
                        }
                }
            }
            break;

// --- read font name ---

            case xlPSFont:
            {
                switch( *pChar )
                {
                    case '\"':
                        --pChar;
                        // run through
                    case ',':
                        eState = xlPSFontStyle;
                    break;
                    default:
                        aReadFont += *pChar;
                }
            }
            break;

// --- read font style ---

            case xlPSFontStyle:
            {
                switch( *pChar )
                {
                    case '\"':
                        SetAttribs();
                        if( aReadFont.Len() )
                            mxFontData->maName = aReadFont;
                        mxFontData->maStyle = aReadStyle;
                        eState = xlPSText;
                    break;
                    default:
                        aReadStyle += *pChar;
                }
            }
            break;

// --- read font height ---

            case xlPSHeight:
            {
                if( ('0' <= *pChar) && (*pChar <= '9') )
                {
                    if( nReadHeight != 0xFFFF )
                    {
                        nReadHeight *= 10;
                        nReadHeight += (*pChar - '0');
                        if( nReadHeight > 1600 )    // max 1600pt = 32000twips
                            nReadHeight = 0xFFFF;
                    }
                }
                else
                {
                    if( (nReadHeight != 0) && (nReadHeight != 0xFFFF) )
                    {
                        SetAttribs();
                        mxFontData->mnHeight = nReadHeight * 20;
                    }
                    --pChar;
                    eState = xlPSText;
                }
            }
            break;
        }
        ++pChar;
    }

    // finalize
    CreateCurrObject();
    maInfos[ EXC_HF_LEFT   ].mnHeight += GetMaxLineHeight( EXC_HF_LEFT );
    maInfos[ EXC_HF_CENTER ].mnHeight += GetMaxLineHeight( EXC_HF_CENTER );
    maInfos[ EXC_HF_RIGHT  ].mnHeight += GetMaxLineHeight( EXC_HF_RIGHT );
}

void XclImpHFConverter::FillToItemSet( SfxItemSet& rItemSet, sal_uInt16 nWhichId ) const
{
    ScPageHFItem aHFItem( nWhichId );
    if( maInfos[ EXC_HF_LEFT ].mxObj.get() )
        aHFItem.SetLeftArea( *maInfos[ EXC_HF_LEFT ].mxObj );
    if( maInfos[ EXC_HF_CENTER ].mxObj.get() )
        aHFItem.SetCenterArea( *maInfos[ EXC_HF_CENTER ].mxObj );
    if( maInfos[ EXC_HF_RIGHT ].mxObj.get() )
        aHFItem.SetRightArea( *maInfos[ EXC_HF_RIGHT ].mxObj );
    rItemSet.Put( aHFItem );
}

sal_Int32 XclImpHFConverter::GetTotalHeight() const
{
    return ::std::max( maInfos[ EXC_HF_LEFT ].mnHeight,
        ::std::max( maInfos[ EXC_HF_CENTER ].mnHeight, maInfos[ EXC_HF_RIGHT ].mnHeight ) );
}

// private --------------------------------------------------------------------

sal_uInt16 XclImpHFConverter::GetMaxLineHeight( XclImpHFPortion ePortion ) const
{
    sal_uInt16 nMaxHt = maInfos[ ePortion ].mnMaxLineHt;
    return (nMaxHt == 0) ? mxFontData->mnHeight : nMaxHt;
}

sal_uInt16 XclImpHFConverter::GetCurrMaxLineHeight() const
{
    return GetMaxLineHeight( meCurrObj );
}

void XclImpHFConverter::UpdateMaxLineHeight( XclImpHFPortion ePortion )
{
    sal_uInt16& rnMaxHt = maInfos[ ePortion ].mnMaxLineHt;
    rnMaxHt = ::std::max( rnMaxHt, mxFontData->mnHeight );
}

void XclImpHFConverter::UpdateCurrMaxLineHeight()
{
    UpdateMaxLineHeight( meCurrObj );
}

void XclImpHFConverter::SetAttribs()
{
    ESelection& rSel = GetCurrSel();
    if( (rSel.nStartPara != rSel.nEndPara) || (rSel.nStartPos != rSel.nEndPos) )
    {
        SfxItemSet aItemSet( mrEE.GetEmptyItemSet() );
        XclImpFont aFont( GetRoot(), *mxFontData );
        aFont.FillToItemSet( aItemSet, EXC_FONTITEM_HF );
        mrEE.QuickSetAttribs( aItemSet, rSel );
        rSel.nStartPara = rSel.nEndPara;
        rSel.nStartPos = rSel.nEndPos;
    }
}

void XclImpHFConverter::ResetFontData()
{
    if( const XclImpFont* pFirstFont = GetFontBuffer().GetFont( EXC_FONT_APP ) )
        *mxFontData = pFirstFont->GetFontData();
    else
    {
        mxFontData->Clear();
        mxFontData->mnHeight = 200;
    }
}

void XclImpHFConverter::InsertText()
{
    if( maCurrText.Len() )
    {
        ESelection& rSel = GetCurrSel();
        mrEE.QuickInsertText( maCurrText, ESelection( rSel.nEndPara, rSel.nEndPos, rSel.nEndPara, rSel.nEndPos ) );
        rSel.nEndPos = rSel.nEndPos + maCurrText.Len();
        maCurrText.Erase();
        UpdateCurrMaxLineHeight();
    }
}

void XclImpHFConverter::InsertField( const SvxFieldItem& rFieldItem )
{
    ESelection& rSel = GetCurrSel();
    mrEE.QuickInsertField( rFieldItem, ESelection( rSel.nEndPara, rSel.nEndPos, rSel.nEndPara, rSel.nEndPos ) );
    ++rSel.nEndPos;
    UpdateCurrMaxLineHeight();
}

void XclImpHFConverter::InsertLineBreak()
{
    ESelection& rSel = GetCurrSel();
    mrEE.QuickInsertText( String( '\n' ), ESelection( rSel.nEndPara, rSel.nEndPos, rSel.nEndPara, rSel.nEndPos ) );
    ++rSel.nEndPara;
    rSel.nEndPos = 0;
    GetCurrInfo().mnHeight += GetCurrMaxLineHeight();
    GetCurrInfo().mnMaxLineHt = 0;
}

void XclImpHFConverter::CreateCurrObject()
{
    InsertText();
    SetAttribs();
    GetCurrObj().reset( mrEE.CreateTextObject() );
}

void XclImpHFConverter::SetNewPortion( XclImpHFPortion eNew )
{
    if( eNew != meCurrObj )
    {
        CreateCurrObject();
        meCurrObj = eNew;
        if( GetCurrObj().get() )
            mrEE.SetText( *GetCurrObj() );
        else
            mrEE.SetText( EMPTY_STRING );
        ResetFontData();
    }
}

// URL conversion =============================================================

namespace {

void lclAppendUrlChar( String& rUrl, sal_Unicode cChar )
{
    // #126855# encode special characters
    switch( cChar )
    {
        case '#':   rUrl.AppendAscii( "%23" );  break;
        case '%':   rUrl.AppendAscii( "%25" );  break;
        default:    rUrl.Append( cChar );
    }
}

} // namespace

void XclImpUrlHelper::DecodeUrl(
        String& rUrl, String& rTabName, bool& rbSameWb,
        const XclImpRoot& rRoot, const String& rEncodedUrl )
{
    enum
    {
        xlUrlInit,              /// Initial state, read string mode character.
        xlUrlPath,              /// Read URL path.
        xlUrlFileName,          /// Read file name.
        xlUrlSheetName,         /// Read sheet name.
        xlUrlRaw                /// Raw mode. No control characters will occur.
    } eState = xlUrlInit;

    bool bEncoded = true;
    rbSameWb = false;

    sal_Unicode cCurrDrive = 0;
    String aDosBase( INetURLObject( rRoot.GetBasePath() ).getFSysPath( INetURLObject::FSYS_DOS ) );
    if( (aDosBase.Len() > 2) && aDosBase.EqualsAscii( ":\\", 1, 2 ) )
        cCurrDrive = aDosBase.GetChar( 0 );

    const sal_Unicode* pChar = rEncodedUrl.GetBuffer();
    while( *pChar )
    {
        switch( eState )
        {

// --- first character ---

            case xlUrlInit:
            {
                switch( *pChar )
                {
                    case EXC_URLSTART_ENCODED:
                        eState = xlUrlPath;
                    break;
                    case EXC_URLSTART_SELF:
                    case EXC_URLSTART_SELFENCODED:
                        rbSameWb = true;
                        eState = xlUrlSheetName;
                    break;
                    case '[':
                        bEncoded = false;
                        eState = xlUrlFileName;
                    break;
                    default:
                        bEncoded = false;
                        lclAppendUrlChar( rUrl, *pChar );
                        eState = xlUrlPath;
                }
            }
            break;

// --- URL path ---

            case xlUrlPath:
            {
                switch( *pChar )
                {
                    case EXC_URL_DOSDRIVE:
                    {
                        if( *(pChar + 1) )
                        {
                            ++pChar;
                            if( *pChar == '@' )
                                rUrl.AppendAscii( "\\\\" );
                            else
                            {
                                lclAppendUrlChar( rUrl, *pChar );
                                rUrl.AppendAscii( ":\\" );
                            }
                        }
                        else
                            rUrl.AppendAscii( "<NULL-DRIVE!>" );
                    }
                    break;
                    case EXC_URL_DRIVEROOT:
                        if( cCurrDrive )
                        {
                            lclAppendUrlChar( rUrl, cCurrDrive );
                            rUrl.Append( ':' );
                        }
                        // run through
                    case EXC_URL_SUBDIR:
                        if( bEncoded )
                            rUrl.Append( '\\' );
                        else    // control character in raw name -> DDE link
                        {
                            rUrl.Append( EXC_DDE_DELIM );
                            eState = xlUrlRaw;
                        }
                    break;
                    case EXC_URL_PARENTDIR:
                        rUrl.AppendAscii( "..\\" );
                    break;
                    case EXC_URL_RAW:
                    {
                        if( *(pChar + 1) )
                        {
                            xub_StrLen nLen = *++pChar;
                            for( xub_StrLen nChar = 0; (nChar < nLen) && *(pChar + 1); ++nChar )
                                lclAppendUrlChar( rUrl, *++pChar );
//                            rUrl.Append( ':' );
                        }
                    }
                    break;
                    case '[':
                        eState = xlUrlFileName;
                    break;
                    default:
                        lclAppendUrlChar( rUrl, *pChar );
                }
            }
            break;

// --- file name ---

            case xlUrlFileName:
            {
                switch( *pChar )
                {
                    case ']':   eState = xlUrlSheetName;    break;
                    default:    lclAppendUrlChar( rUrl, *pChar );
                }
            }
            break;

// --- sheet name ---

            case xlUrlSheetName:
                rTabName.Append( *pChar );
            break;

// --- raw read mode ---

            case xlUrlRaw:
                lclAppendUrlChar( rUrl, *pChar );
            break;
        }

        ++pChar;
    }
}

void XclImpUrlHelper::DecodeUrl(
        String& rUrl, bool& rbSameWb, const XclImpRoot& rRoot, const String& rEncodedUrl )
{
    String aTabName;
    DecodeUrl( rUrl, aTabName, rbSameWb, rRoot, rEncodedUrl );
    DBG_ASSERT( !aTabName.Len(), "XclImpUrlHelper::DecodeUrl - sheet name ignored" );
}

bool XclImpUrlHelper::DecodeLink( String& rApplic, String& rTopic, const String rEncUrl )
{
    xub_StrLen nPos = rEncUrl.Search( EXC_DDE_DELIM );
    if( (nPos != STRING_NOTFOUND) && (0 < nPos) && (nPos + 1 < rEncUrl.Len()) )
    {
        rApplic = rEncUrl.Copy( 0, nPos );
        rTopic = rEncUrl.Copy( nPos + 1 );
        return true;
    }
    return false;
}

// Cached Values ==============================================================

XclImpCachedValue::XclImpCachedValue( XclImpStream& rStrm ) :
    mfValue( 0.0 ),
    mnBoolErr( 0 )
{
    rStrm >> mnType;
    switch( mnType )
    {
        case EXC_CACHEDVAL_EMPTY:
            rStrm.Ignore( 8 );
        break;
        case EXC_CACHEDVAL_DOUBLE:
            rStrm >> mfValue;
        break;
        case EXC_CACHEDVAL_STRING:
            mxStr.reset( new String( rStrm.ReadUniString() ) );
        break;
        case EXC_CACHEDVAL_BOOL:
        case EXC_CACHEDVAL_ERROR:
        {
            double fVal;
            rStrm >> mnBoolErr;
            rStrm.Ignore( 7 );

            const ScTokenArray* pScTokArr = rStrm.GetRoot().GetOldFmlaConverter().GetBoolErr(
                XclTools::ErrorToEnum( fVal, mnType == EXC_CACHEDVAL_ERROR, mnBoolErr ) );
            if( pScTokArr )
                mxTokArr.reset( pScTokArr->Clone() );
        }
        break;
        default:
            DBG_ERRORFILE( "XclImpCachedValue::XclImpCachedValue - unknown data type" );
    }
}

XclImpCachedValue::~XclImpCachedValue()
{
}

sal_uInt16 XclImpCachedValue::GetScError() const
{
    return (mnType == EXC_CACHEDVAL_ERROR) ? XclTools::GetScErrorCode( mnBoolErr ) : 0;
}

// Matrix Cached Values ==============================================================

XclImpCachedMatrix::XclImpCachedMatrix( XclImpStream& rStrm ) :
    mnScCols( 0 ),
    mnScRows( 0 )
{
    mnScCols = rStrm.ReaduInt8();
    mnScRows = rStrm.ReaduInt16();

    if( rStrm.GetRoot().GetBiff() <= EXC_BIFF5 )
    {
        // in BIFF2-BIFF7: 256 columns represented by 0 columns
        if( mnScCols == 0 )
            mnScCols = 256;
    }
    else
    {
        // in BIFF8: columns and rows decreaed by 1
        ++mnScCols;
        ++mnScRows;
    }

    for( SCSIZE nScRow = 0; nScRow < mnScRows; ++nScRow )
        for( SCSIZE nScCol = 0; nScCol < mnScCols; ++nScCol )
            maValueList.Append( new XclImpCachedValue( rStrm ) );
}

XclImpCachedMatrix::~XclImpCachedMatrix()
{
}

ScMatrixRef XclImpCachedMatrix::CreateScMatrix() const
{
    ScMatrixRef xScMatrix;
    DBG_ASSERT( mnScCols * mnScRows == maValueList.Count(), "XclImpCachedMatrix::CreateScMatrix - element count mismatch" );
    if( mnScCols && mnScRows && static_cast< sal_uLong >( mnScCols * mnScRows ) <= maValueList.Count() )
    {
        xScMatrix = new ScMatrix( mnScCols, mnScRows );
        const XclImpCachedValue* pValue = maValueList.First();
        for( SCSIZE nScRow = 0; nScRow < mnScRows; ++nScRow )
        {
            for( SCSIZE nScCol = 0; nScCol < mnScCols; ++nScCol )
            {
                switch( pValue->GetType() )
                {
                    case EXC_CACHEDVAL_EMPTY:
                        // Excel shows 0.0 here, not an empty cell
                        xScMatrix->PutEmpty( nScCol, nScRow );
                    break;
                    case EXC_CACHEDVAL_DOUBLE:
                        xScMatrix->PutDouble( pValue->GetValue(), nScCol, nScRow );
                    break;
                    case EXC_CACHEDVAL_STRING:
                        xScMatrix->PutString( pValue->GetString(), nScCol, nScRow );
                    break;
                    case EXC_CACHEDVAL_BOOL:
                        xScMatrix->PutBoolean( pValue->GetBool(), nScCol, nScRow );
                    break;
                    case EXC_CACHEDVAL_ERROR:
                        xScMatrix->PutError( pValue->GetScError(), nScCol, nScRow );
                    break;
                    default:
                        DBG_ERRORFILE( "XclImpCachedMatrix::CreateScMatrix - unknown value type" );
                        xScMatrix->PutEmpty( nScCol, nScRow );
                }
                pValue = maValueList.Next();
            }
        }
    }
    return xScMatrix;
}

// ============================================================================

