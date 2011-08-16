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
#include "precompiled_sd.hxx"

#include <osl/module.hxx>
#include <tools/urlobj.hxx>
#include <svl/itemset.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <svx/xflclit.hxx>
#include <svx/xfillit0.hxx>

#include "sdpage.hxx"
#include "drawdoc.hxx"
#include "sdcgmfilter.hxx"

// -----------
// - Defines -
// -----------

#define CGM_IMPORT_CGM		0x00000001
#define CGM_IMPORT_IM		0x00000002

#define CGM_EXPORT_IMPRESS	0x00000100
#define CGM_EXPORT_META		0x00000200
#define CGM_EXPORT_COMMENT	0x00000400

#define CGM_NO_PAD_BYTE 	0x00010000
#define CGM_BIG_ENDIAN		0x00020000
#define CGM_LITTLE_ENDIAN	0x00040000

// --------------
// - Namespaces -
// --------------

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::frame;

// ------------
// - Typedefs -
// ------------

typedef sal_uInt32 ( __LOADONCALLAPI *ImportCGM )( ::rtl::OUString&, Reference< XModel >&, sal_uInt32, Reference< XStatusIndicator >& );
typedef sal_Bool ( __LOADONCALLAPI *ExportCGM )( ::rtl::OUString&, Reference< XModel >&, Reference< XStatusIndicator >&, void* );

// ---------------
// - SdPPTFilter -
// ---------------

SdCGMFilter::SdCGMFilter( SfxMedium& rMedium, ::sd::DrawDocShell& rDocShell, sal_Bool bShowProgress ) :
	SdFilter( rMedium, rDocShell, bShowProgress )
{
}

// -----------------------------------------------------------------------------

SdCGMFilter::~SdCGMFilter()
{
}

// -----------------------------------------------------------------------------

sal_Bool SdCGMFilter::Import()
{
	::osl::Module* pLibrary = OpenLibrary( mrMedium.GetFilter()->GetUserData() );
	sal_Bool		bRet = sal_False;

	if( pLibrary && mxModel.is() )
	{
		ImportCGM		FncImportCGM = reinterpret_cast< ImportCGM >( pLibrary->getFunctionSymbol( ::rtl::OUString::createFromAscii( "ImportCGM" ) ) );
		::rtl::OUString	aFileURL( mrMedium.GetURLObject().GetMainURL( INetURLObject::NO_DECODE ) );
		sal_uInt32			nRetValue;

		if( mrDocument.GetPageCount() == 0L )
			mrDocument.CreateFirstPages();

		CreateStatusIndicator();
		nRetValue = FncImportCGM( aFileURL, mxModel, CGM_IMPORT_CGM | CGM_BIG_ENDIAN | CGM_EXPORT_IMPRESS, mxStatusIndicator );
		
		if( nRetValue )
		{
			bRet = sal_True;
			
			if( ( nRetValue &~0xff000000 ) != 0xffffff )	// maybe the backgroundcolor is already white
			{												// so we must not set a master page
				mrDocument.StopWorkStartupDelay();
                SdPage* pSdPage = mrDocument.GetMasterSdPage(0, PK_STANDARD);

                if(pSdPage)
                {
					// set PageFill to given color
				    const Color aColor((sal_uInt8)(nRetValue >> 16), (sal_uInt8)(nRetValue >> 8), (sal_uInt8)(nRetValue >> 16));
				    pSdPage->getSdrPageProperties().PutItem(XFillColorItem(String(), aColor));
				    pSdPage->getSdrPageProperties().PutItem(XFillStyleItem(XFILL_SOLID));
                }
			}
		}
	}

	delete pLibrary;

	return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool SdCGMFilter::Export()
{
	::osl::Module* pLibrary = OpenLibrary( mrMedium.GetFilter()->GetUserData() );
	sal_Bool		bRet = sal_False;

	if( pLibrary && mxModel.is() )
	{
		ExportCGM FncCGMExport = reinterpret_cast< ExportCGM >( pLibrary->getFunctionSymbol( ::rtl::OUString::createFromAscii( "ExportCGM" ) ) );

		if( FncCGMExport )
		{
			::rtl::OUString aPhysicalName( mrMedium.GetPhysicalName() );

			/* !!!
			if ( pViewShell && pViewShell->GetView() )
				pViewShell->GetView()->SdrEndTextEdit();
			*/
			CreateStatusIndicator();
			bRet = FncCGMExport( aPhysicalName, mxModel, mxStatusIndicator, NULL );
		}
	}

	delete pLibrary;

	return bRet;
}
