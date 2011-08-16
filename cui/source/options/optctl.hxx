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
#ifndef _SVX_OPTCTL_HXX
#define _SVX_OPTCTL_HXX

#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <sfx2/tabdlg.hxx>

// class SvxCTLOptionsPage -----------------------------------------------------

class SvxCTLOptionsPage : public SfxTabPage
{
private:
	FixedLine			m_aSequenceCheckingFL;
	CheckBox			m_aSequenceCheckingCB;
	CheckBox			m_aRestrictedCB;
    CheckBox            m_aTypeReplaceCB;

	FixedLine			m_aCursorControlFL;
	FixedText			m_aMovementFT;
	RadioButton			m_aMovementLogicalRB;
	RadioButton			m_aMovementVisualRB;

	FixedLine			m_aGeneralFL;
	FixedText			m_aNumeralsFT;
	ListBox				m_aNumeralsLB;

	DECL_LINK( SequenceCheckingCB_Hdl, void* );

	SvxCTLOptionsPage( Window* pParent, const SfxItemSet& rSet );

public:

	virtual ~SvxCTLOptionsPage();

	static SfxTabPage*	Create( Window* pParent, const SfxItemSet& rAttrSet );
	virtual	sal_Bool 		FillItemSet( SfxItemSet& rSet );
	virtual	void 		Reset( const SfxItemSet& rSet );
};

#endif // #ifndef _SVX_OPTCTL_HXX

