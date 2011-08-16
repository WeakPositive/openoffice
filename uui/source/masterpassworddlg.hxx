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

#ifndef UUI_MASTERPASSWORDDLG_HXX
#define UUI_MASTERPASSWORDDLG_HXX

#ifndef _COM_SUN_STAR_TASK_PASSWORDREQUESTMODE_HPP
#include <com/sun/star/task/PasswordRequestMode.hpp>
#endif
#include <svtools/stdctrl.hxx>
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <vcl/dialog.hxx>
#include <vcl/edit.hxx>
#include <vcl/group.hxx>

//============================================================================
class MasterPasswordDialog : public ModalDialog
{
	FixedText		aFTMasterPassword;
	Edit			aEDMasterPassword;
    FixedLine       aFL;
	OKButton		aOKBtn;
	CancelButton	aCancelBtn;
	HelpButton		aHelpBtn;


	DECL_LINK( OKHdl_Impl, OKButton * );

public:
	MasterPasswordDialog( Window* pParent, ::com::sun::star::task::PasswordRequestMode nDlgMode, ResMgr * pResMgr );

	String			GetMasterPassword() const { return aEDMasterPassword.GetText(); }

private:
	::com::sun::star::task::PasswordRequestMode		nDialogMode;
	ResMgr*											pResourceMgr;
};

#endif // UUI_MASTERPASSWORDDLG_HXX

