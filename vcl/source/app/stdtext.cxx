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
#include "precompiled_vcl.hxx"

#include <vcl/msgbox.hxx>
#include <vcl/stdtext.hxx>

#include <svids.hrc>
#include <svdata.hxx>


// =======================================================================

XubString GetStandardText( sal_uInt16 nStdText )
{
    ResMgr* pResMgr = ImplGetResMgr();
	XubString aText;
    if( pResMgr )
        aText = XubString( ResId( nStdText-STANDARD_TEXT_FIRST+SV_STDTEXT_FIRST, *pResMgr ) );
	return aText;
}

// =======================================================================

void ShowServiceNotAvailableError( Window* pParent,
								   const XubString& rServiceName, sal_Bool bError )
{
	XubString aText( GetStandardText( STANDARD_TEXT_SERVICE_NOT_AVAILABLE ) );
	aText.SearchAndReplaceAscii( "%s", rServiceName );
	if ( bError )
	{
		ErrorBox aBox( pParent, WB_OK | WB_DEF_OK, aText );
		aBox.Execute();
	}
	else
	{
		WarningBox aBox( pParent, WB_OK | WB_DEF_OK, aText );
		aBox.Execute();
	}
}
