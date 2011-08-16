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
#ifndef _SVX_DLG_NAME_HXX
#define _SVX_DLG_NAME_HXX


#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#include <vcl/fixed.hxx>

// #i68101#
#include <svtools/svmedit.hxx>

/*************************************************************************
|*
|* Dialog zum Editieren eines Namens
|*
\************************************************************************/
class SvxNameDialog : public ModalDialog
{
private:
	FixedText		aFtDescription;
	Edit			aEdtName;
	OKButton		aBtnOK;
	CancelButton	aBtnCancel;
	HelpButton		aBtnHelp;

    Link            aCheckNameHdl;
#if _SOLAR__PRIVATE
    DECL_LINK(ModifyHdl, Edit*);
#endif

public:
	SvxNameDialog( Window* pWindow, const String& rName, const String& rDesc );

    void    GetName( String& rName ){rName = aEdtName.GetText();}

    /** add a callback Link that is called whenever the content of the edit
        field is changed.  The Link result determines wether the OK
        Button is enabled (> 0) or disabled (== 0).

        @param rLink a Callback declared with DECL_LINK and implemented with
               IMPL_LINK, that is executed on modification.

        @param bCheckImmediately If true, the Link is called directly after
               setting it. It is recommended to set this flag to true to avoid
               an inconsistent state if the initial String (given in the CTOR)
               does not satisfy the check condition.

        @todo Remove the parameter bCheckImmediately and incorporate the 'true'
              behaviour as default.
     */
    void    SetCheckNameHdl( const Link& rLink, bool bCheckImmediately = false )
    {
        aCheckNameHdl = rLink;
        if ( bCheckImmediately )
            aBtnOK.Enable( rLink.Call( this ) > 0 );
    }

    void    SetEditHelpId( const rtl::OString& aHelpId) {aEdtName.SetHelpId(aHelpId);}
};

///////////////////////////////////////////////////////////////////////////////////////////////
// #i68101#
// Dialog for editing Object Title and Description
// plus uniqueness-callback-linkHandler

class SvxObjectNameDialog : public ModalDialog
{
private:
	// name
	FixedText		aFtName;
	Edit			aEdtName;

	// separator
	FixedLine		aFlSeparator;

	// buttons
	HelpButton		aBtnHelp;
	OKButton		aBtnOK;
	CancelButton	aBtnCancel;

	// callback link for name uniqueness
    Link            aCheckNameHdl;
#if _SOLAR__PRIVATE
    DECL_LINK(ModifyHdl, Edit*);
#endif

public:
	// constructor
	SvxObjectNameDialog(Window* pWindow, const String& rName);

	// data access
    void GetName(String& rName) {rName = aEdtName.GetText(); }

	// set handler
    void SetCheckNameHdl(const Link& rLink, bool bCheckImmediately = false)
    {
        aCheckNameHdl = rLink;

		if(bCheckImmediately)
		{
            aBtnOK.Enable(rLink.Call(this) > 0);
		}
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////
// #i68101#
// Dialog for editing Object Title and Description

class SvxObjectTitleDescDialog : public ModalDialog
{
private:
	// title
	FixedText		aFtTitle;
	Edit			aEdtTitle;

	// description
	FixedText		aFtDescription;
	MultiLineEdit	aEdtDescription;

	// separator
	FixedLine		aFlSeparator;

	// buttons
	HelpButton		aBtnHelp;
	OKButton		aBtnOK;
	CancelButton	aBtnCancel;

public:
	// constructor
	SvxObjectTitleDescDialog(Window* pWindow, const String& rTitle, const String& rDesc);

	// data access
    void GetTitle(String& rTitle) {rTitle = aEdtTitle.GetText(); }
    void GetDescription(String& rDescription) {rDescription = aEdtDescription.GetText(); }
};

///////////////////////////////////////////////////////////////////////////////////////////////

/*************************************************************************
|*
|* Dialog zum Abbrechen, Speichern oder Hinzufuegen
|*
\************************************************************************/
class SvxMessDialog : public ModalDialog
{
private:
	FixedText		aFtDescription;
	PushButton		aBtn1;
	PushButton		aBtn2;
	CancelButton	aBtnCancel;
	FixedImage		aFtImage;
	Image*			pImage;
#if _SOLAR__PRIVATE
	DECL_LINK( Button1Hdl, Button * );
	DECL_LINK( Button2Hdl, Button * );
#endif
public:
	SvxMessDialog( Window* pWindow, const String& rText, const String& rDesc, Image* pImg = NULL );
	~SvxMessDialog();

	void	SetButtonText( sal_uInt16 nBtnId, const String& rNewTxt );
};



#endif // _SVX_DLG_NAME_HXX

