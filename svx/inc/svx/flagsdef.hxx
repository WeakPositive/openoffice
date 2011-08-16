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
#ifndef _SVX_FLAGSDEF_HXX
#define _SVX_FLAGSDEF_HXX

// defines ---------------------------------------------------------------

//CHINA001 copy from border.hxx
// Border-Modes for paragraphs, textframes and tables
#define SW_BORDER_MODE_PARA 	0x01
#define SW_BORDER_MODE_TABLE    0x02
#define SW_BORDER_MODE_FRAME    0x04

//CHINA001 flags for SvxBackgroundTabPage
#define SVX_SHOW_SELECTOR		0x01
#define SVX_SHOW_PARACTL		0x02
#define SVX_ENABLE_TRANSPARENCY		0x04
#define SVX_SHOW_TBLCTL			0x08

//CHINA001 flags for SvxBorderTabPage
#define SVX_HIDESHADOWCTL	0x01

//CHINA001 copy from chardlg.hxx
#define DISABLE_CASEMAP				((sal_uInt16)0x0001)
#define DISABLE_WORDLINE			((sal_uInt16)0x0002)
#define DISABLE_BLINK				((sal_uInt16)0x0004)
#define DISABLE_UNDERLINE_COLOR		((sal_uInt16)0x0008)

#define DISABLE_LANGUAGE			((sal_uInt16)0x0010)
#define DISABLE_HIDE_LANGUAGE		((sal_uInt16)0x0020)

//CHINA001 flags for SvxCharBasePage's child class
#define SVX_PREVIEW_CHARACTER 	0x01
//CHINA001 flags for SvxCharNamePage
#define SVX_RELATIVE_MODE		0x02
//CHINA001 flags for SvxCharEffectsPage
#define SVX_ENABLE_FLASH	0x04


//CHINA001 copy from numfmt.hxx
#define SVX_NUMVAL_STANDARD		-1234.12345678901234
#define SVX_NUMVAL_CURRENCY		-1234
#define SVX_NUMVAL_PERCENT		-0.1295
#define SVX_NUMVAL_TIME 		36525.5678935185
#define SVX_NUMVAL_DATE 		36525.5678935185
#define SVX_NUMVAL_BOOLEAN 		1

//CHINA001 copy from page.hxx
// enum ------------------------------------------------------------------

enum SvxModeType
{
	SVX_PAGE_MODE_STANDARD,
	SVX_PAGE_MODE_CENTER,
	SVX_PAGE_MODE_PRESENTATION
};
// define ----------------------------------------------------------------

// 1/2 cm in TWIPS
// wird auch fuer Minimalgrosse der LayFrms aller Arten benutzt
#define MM50   283  //from original svx/inc/paragrph.hxx

//--------------from original svx/inc/tabstpge.hxx
#define TABTYPE_LEFT		0x0001
#define TABTYPE_RIGHT		0x0002
#define TABTYPE_CENTER		0x0004
#define TABTYPE_DEZIMAL		0x0008
#define TABTYPE_ALL			0x000F

#define TABFILL_NONE		0x0010
#define TABFILL_POINT		0x0020
#define TABFILL_DASHLINE 	0x0040
#define TABFILL_SOLIDLINE	0x0080
#define TABFILL_SPECIAL		0x0100
#define TABFILL_ALL			0x01F0

//-----

#endif
