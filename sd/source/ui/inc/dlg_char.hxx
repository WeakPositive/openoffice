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


#ifndef SD_DLG_CHAR_HXX
#define SD_DLG_CHAR_HXX

#include <sfx2/tabdlg.hxx>

#include "dlg_char.hrc"

class SfxItemSet;
class SfxObjectShell;


/*************************************************************************
|*
|* Zeichen-Tab-Dialog
|*
\************************************************************************/
class SdCharDlg : public SfxTabDialog
{
private:
	const SfxItemSet&		rOutAttrs;
	const SfxObjectShell&   rDocShell;

	virtual void	PageCreated( sal_uInt16 nId, SfxTabPage &rPage );

public:

			SdCharDlg( Window* pParent, const SfxItemSet* pAttr,
						const SfxObjectShell* pDocShell );
			~SdCharDlg() {};
};

#endif // SD_DLG_CHAR_HXX

