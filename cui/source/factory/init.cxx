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
#include "precompiled_cui.hxx"

#include <vcl/msgbox.hxx>
#include "cuicharmap.hxx"

// ------------------------------------------------------------------------
// hook to call special character dialog for edits
// caution: needs C-Linkage since dynamically loaded via symbol name
extern "C"
{
SAL_DLLPUBLIC_EXPORT bool GetSpecialCharsForEdit(Window* i_pParent, const Font& i_rFont, String& o_rResult)
{
    bool bRet = false;
	SvxCharacterMap* aDlg = new SvxCharacterMap( i_pParent );
    aDlg->DisableFontSelection();
    aDlg->SetCharFont(i_rFont);
	if ( aDlg->Execute() == RET_OK )
    {
		o_rResult = aDlg->GetCharacters();
        bRet = true;
    }
    delete aDlg;
    return bRet;
}
}
