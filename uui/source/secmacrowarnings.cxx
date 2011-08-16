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

#include <com/sun/star/xml/crypto/XSecurityEnvironment.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/security/XDocumentDigitalSignatures.hpp>
#include <comphelper/sequence.hxx>
#include "comphelper/documentconstants.hxx"
#include <comphelper/processfactory.hxx>

#include <vcl/msgbox.hxx>
#include <com/sun/star/security/NoPasswordException.hpp>

using namespace ::com::sun::star::security;

#include "ids.hrc"
#include "secmacrowarnings.hxx"
#include "secmacrowarnings.hrc"

#ifdef _MSC_VER
#pragma warning (disable : 4355)	// 4355: this used in initializer-list
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star;


// HACK!!! copied from xmlsecurity/source/dialog/resourcemanager.cxx

namespace
{
	String GetContentPart( const String& _rRawString, const String& _rPartId )
	{
		String		s;

		xub_StrLen	nContStart = _rRawString.Search( _rPartId );
		if( nContStart != STRING_NOTFOUND )
		{
			nContStart = nContStart + _rPartId.Len();
			++nContStart;					// now it's start of content, directly after Id

			xub_StrLen	nContEnd = _rRawString.Search( sal_Unicode( ',' ), nContStart );

			s = String( _rRawString, nContStart, nContEnd - nContStart );
		}

		return s;
	}
}


MacroWarning::MacroWarning( Window* _pParent, bool _bWithSignatures, ResMgr& rResMgr )
	:ModalDialog			( _pParent, ResId( RID_XMLSECDLG_MACROWARN, rResMgr ) )
	,mpInfos				( NULL )
	,maSymbolImg			( this, ResId( IMG_SYMBOL, rResMgr ) )
	,maDocNameFI			( this, ResId( FI_DOCNAME, rResMgr ) )
	,maDescr1aFI			( this, ResId( FI_DESCR1A, rResMgr ) )
	,maDescr1bFI			( this, ResId( FI_DESCR1B, rResMgr ) )
	,maSignsFI				( this, ResId( FI_SIGNS, rResMgr ) )
	,maViewSignsBtn			( this, ResId( PB_VIEWSIGNS, rResMgr ) )
	,maDescr2FI				( this, ResId( FI_DESCR2, rResMgr ) )
	,maAlwaysTrustCB		( this, ResId( CB_ALWAYSTRUST, rResMgr ) )
	,maBottomSepFL			( this, ResId( FL_BOTTOM_SEP, rResMgr ) )
	,maEnableBtn			( this, ResId( PB_ENABLE, rResMgr ) )
	,maDisableBtn			( this, ResId( PB_DISABLE, rResMgr ) )
	,maHelpBtn				( this, ResId( BTN_HELP, rResMgr ) )
	,mbSignedMode			( true )
    ,mbShowSignatures       ( _bWithSignatures )
	,mnActSecLevel			( 0 )
{
	FreeResource();

    InitControls();

    maDisableBtn.SetClickHdl( LINK( this, MacroWarning, DisableBtnHdl ) );
	maEnableBtn.SetClickHdl( LINK( this, MacroWarning, EnableBtnHdl ) );
    maDisableBtn.GrabFocus(); // Default button, but focus is on view button
}

MacroWarning::~MacroWarning()
{
}

short MacroWarning::Execute()
{
    FitControls();
    return ModalDialog::Execute();
}

void MacroWarning::SetDocumentURL( const String& rDocURL )
{
    maDocNameFI.SetText( rDocURL );
}

IMPL_LINK( MacroWarning, ViewSignsBtnHdl, void*, EMPTYARG )
{
	DBG_ASSERT( mxCert.is(), "*MacroWarning::ViewSignsBtnHdl(): no certificate set!" );

    uno::Sequence< uno::Any > aArgs( 1 );
    aArgs[0] = uno::makeAny( maODFVersion );
	uno::Reference< security::XDocumentDigitalSignatures > xD(
		comphelper::getProcessServiceFactory()->createInstanceWithArguments( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.security.DocumentDigitalSignatures" ) ), aArgs ), uno::UNO_QUERY );
	if( xD.is() )
	{
		if( mxCert.is() )
			xD->showCertificate( mxCert );
		else if( mxStore.is() )
			xD->showScriptingContentSignatures( mxStore, uno::Reference< io::XInputStream >() );
	}

    return 0;
}

IMPL_LINK( MacroWarning, EnableBtnHdl, void*, EMPTYARG )
{
	if( mbSignedMode && maAlwaysTrustCB.IsChecked() )
	{	// insert path into trusted path list
        uno::Sequence< uno::Any > aArgs( 1 );
        aArgs[0] = uno::makeAny( maODFVersion );
        uno::Reference< security::XDocumentDigitalSignatures > xD(
            comphelper::getProcessServiceFactory()->createInstanceWithArguments( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.security.DocumentDigitalSignatures" ) ), aArgs ), uno::UNO_QUERY );
		if( xD.is() )
		{
			if( mxCert.is() )
				xD->addAuthorToTrustedSources( mxCert );
			else if( mxStore.is() )
			{
				DBG_ASSERT( mpInfos, "-MacroWarning::EnableBtnHdl(): no infos, search in nirvana..." );

				sal_Int32	nCnt = mpInfos->getLength();
				for( sal_Int32 i = 0 ; i < nCnt ; ++i )
					xD->addAuthorToTrustedSources( (*mpInfos)[ i ].Signer );
			}
		}
	}

	EndDialog( RET_OK );
    return 0;
}

IMPL_LINK( MacroWarning, DisableBtnHdl, void*, EMPTYARG )
{
	EndDialog( RET_CANCEL );
    return 0;
}

IMPL_LINK( MacroWarning, AlwaysTrustCheckHdl, void*, EMPTYARG )
{
	bool bEnable = ( mnActSecLevel < 2 || maAlwaysTrustCB.IsChecked() );
	maEnableBtn.Enable( bEnable );
	maDisableBtn.Enable( !maAlwaysTrustCB.IsChecked() );

	return 0;
}

void MacroWarning::InitControls()
{
    // set warning image
    Image aImg( WarningBox::GetStandardImage() );
    maSymbolImg.SetImage( aImg );
    maSymbolImg.SetSizePixel( aImg.GetSizePixel() );
    // set bold font and path ellipsis for docname fixedtext
    Font aTmpFont = maDocNameFI.GetControlFont();
    aTmpFont.SetWeight( WEIGHT_BOLD );
    maDocNameFI.SetControlFont( aTmpFont );
    WinBits nStyle = maDocNameFI.GetStyle();
    nStyle |= WB_PATHELLIPSIS;
    maDocNameFI.SetStyle( nStyle );
    // show signature controls?
    if( mbShowSignatures )
    {
        maDescr1bFI.Hide();
        maViewSignsBtn.SetClickHdl( LINK( this, MacroWarning, ViewSignsBtnHdl ) );
        maViewSignsBtn.Disable();   // default
        maAlwaysTrustCB.SetClickHdl( LINK( this, MacroWarning, AlwaysTrustCheckHdl ) );

        mnActSecLevel = SvtSecurityOptions().GetMacroSecurityLevel();
        if ( mnActSecLevel >= 2 )
            maEnableBtn.Disable();
    }
    else
    {
        maDescr1aFI.Hide();
        maSignsFI.Hide();
        maViewSignsBtn.Hide();
        maAlwaysTrustCB.Hide();

        // move hint up to position of signer list
        maDescr2FI.SetPosPixel( maSignsFI.GetPosPixel() );
    }
    // without signature controls could be smaller
    if ( !mbShowSignatures )
    {
        Point aPos = maDescr2FI.GetPosPixel();
        aPos.Y() += maDescr2FI.GetSizePixel().Height();
        aPos.Y() += LogicToPixel( Size( 3, 3 ) ).Height();
        long nDelta = maBottomSepFL.GetPosPixel().Y() - aPos.Y();
        Window* pWins[] =
        {
            &maBottomSepFL, &maEnableBtn, &maDisableBtn, &maHelpBtn
        };
        Window** pCurrent = pWins;
        for ( sal_uInt32 i = 0; i < sizeof( pWins ) / sizeof( pWins[ 0 ] ); ++i, ++pCurrent )
        {
            Point aNewPos = (*pCurrent)->GetPosPixel();
            aNewPos.Y() -= nDelta;
            (*pCurrent)->SetPosPixel( aNewPos );
        }

        Size aDlgSz = GetSizePixel();
        aDlgSz.Height() -= nDelta;
        SetSizePixel( aDlgSz );
    }

    // check if some buttontexts are to wide
    String sText = maViewSignsBtn.GetText();
    long nTxtW = maViewSignsBtn.GetTextWidth( sText );
    const long nOffset = 12;
    if ( sText.Search( '~' ) == STRING_NOTFOUND )
        nTxtW += nOffset;
    long nBtnW = maViewSignsBtn.GetSizePixel().Width();
    if ( nTxtW >= nBtnW )
    {
        // broaden the button
        long nDelta = Max( nTxtW - nBtnW, nOffset/3 );
        Size aNewSize = maViewSignsBtn.GetSizePixel();
        aNewSize.Width() += nDelta;
        maViewSignsBtn.SetSizePixel( aNewSize );
        // and give it a new position
        Point aNewPos = maViewSignsBtn.GetPosPixel();
        aNewPos.X() -= nDelta;
        maViewSignsBtn.SetPosPixel( aNewPos );
        // the the left fixedtext must be smaller
        aNewSize = maSignsFI.GetSizePixel();
        aNewSize.Width() -= nDelta;
        maSignsFI.SetSizePixel( aNewSize );
    }
    // if the button text (we compare with the longest of both) is too wide, then broaden the buttons
    String sText1 = maEnableBtn.GetText();
    long nTxtW1 = maEnableBtn.GetTextWidth( sText1 );
    if ( sText1.Search( '~' ) == STRING_NOTFOUND )
        nTxtW1 += nOffset;
    String sText2 = maDisableBtn.GetText();
    long nTxtW2 = maDisableBtn.GetTextWidth( sText2 );
    if ( sText2.Search( '~' ) == STRING_NOTFOUND )
        nTxtW2 += nOffset;
    nTxtW = Max( nTxtW1, nTxtW2 );
    nBtnW = maEnableBtn.GetSizePixel().Width();
    if ( nTxtW > nBtnW )
    {
        // broaden both buttons
        long nDelta = nTxtW - nBtnW;
        Size aNewSize = maEnableBtn.GetSizePixel();
        aNewSize.Width() += nDelta;
        maEnableBtn.SetSizePixel( aNewSize );
        maDisableBtn.SetSizePixel( aNewSize );
        // and give them a new position
        Point aNewPos = maEnableBtn.GetPosPixel();
        aNewPos.X() -= (2*nDelta);
        maEnableBtn.SetPosPixel( aNewPos );
        aNewPos = maDisableBtn.GetPosPixel();
        aNewPos.X() -= nDelta;
        maDisableBtn.SetPosPixel( aNewPos );
    }
}

void MacroWarning::FitControls()
{
    Size a3Size = LogicToPixel( Size( 3, 3 ), MAP_APPFONT );
    Size aNewSize, aMinSize;
    long nTxtH = 0;
    long nCtrlH = 0;
    long nDelta = 0;

    if ( mbShowSignatures )
    {
        aMinSize = maSignsFI.CalcMinimumSize( maSignsFI.GetSizePixel().Width() );
        nTxtH = Max( aMinSize.Height(), maViewSignsBtn.GetSizePixel().Height() );
        nTxtH += a3Size.Height() / 2;
        nCtrlH = maSignsFI.GetSizePixel().Height();
        nDelta = Max( nCtrlH - nTxtH, static_cast< long >( -100 ) ); // not too large
        aNewSize = maSignsFI.GetSizePixel();
        aNewSize.Height() -= nDelta;
        maSignsFI.SetSizePixel( aNewSize );
    }

    aMinSize = maDescr2FI.CalcMinimumSize( maDescr2FI.GetSizePixel().Width() );
    nTxtH = aMinSize.Height();
    nCtrlH = maDescr2FI.GetSizePixel().Height();
    long nDelta2 = ( nCtrlH - nTxtH );
    aNewSize = maDescr2FI.GetSizePixel();
    aNewSize.Height() -= nDelta2;
    maDescr2FI.SetSizePixel( aNewSize );

    // new position for the succeeding windows
    Window* pWins[] =
    {
        &maDescr2FI, &maAlwaysTrustCB, &maBottomSepFL, &maEnableBtn, &maDisableBtn, &maHelpBtn
    };
    Window** pCurrent = pWins;
    for ( sal_uInt32 i = 0; i < sizeof( pWins ) / sizeof( pWins[ 0 ] ); ++i, ++pCurrent )
    {
        Point aNewPos = (*pCurrent)->GetPosPixel();
        aNewPos.Y() -= nDelta;
        (*pCurrent)->SetPosPixel( aNewPos );

        if ( *pCurrent == &maDescr2FI )
            nDelta += nDelta2;
    }

    // new size of the dialog
    aNewSize = GetSizePixel();
    aNewSize.Height() -= nDelta;
    SetSizePixel( aNewSize );
}

void MacroWarning::SetStorage( const cssu::Reference < css::embed::XStorage >& rxStore,
                               const ::rtl::OUString& aODFVersion,
                               const cssu::Sequence< security::DocumentSignatureInformation >& rInfos )
{
	mxStore = rxStore;
    maODFVersion = aODFVersion;
	sal_Int32	nCnt = rInfos.getLength();
	if( mxStore.is() && nCnt > 0 )
	{
		mpInfos = &rInfos;
		String		aCN_Id( String::CreateFromAscii( "CN" ) );
		String		s;
		s = GetContentPart( rInfos[ 0 ].Signer->getSubjectName(), aCN_Id );

		for( sal_Int32 i = 1 ; i < nCnt ; ++i )
		{
			s.AppendAscii( "\n" );
			s += GetContentPart( rInfos[ i ].Signer->getSubjectName(), aCN_Id );
		}

        maSignsFI.SetText( s );
		maViewSignsBtn.Enable();
	}
}

void MacroWarning::SetCertificate( const cssu::Reference< css::security::XCertificate >& _rxCert )
{
	mxCert = _rxCert;
	if( mxCert.is() )
	{
		String	aCN_Id( String::CreateFromAscii( "CN" ) );
		String	s;
		s = GetContentPart( mxCert->getSubjectName(), aCN_Id );
        maSignsFI.SetText( s );
		maViewSignsBtn.Enable();
	}
}

