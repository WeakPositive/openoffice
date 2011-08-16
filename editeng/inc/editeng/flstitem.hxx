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
#ifndef _SVX_FLSTITEM_HXX
#define _SVX_FLSTITEM_HXX

// include ---------------------------------------------------------------

#include <svl/poolitem.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Sequence.h>
#include "editeng/editengdllapi.h"

// class SvxFontListItem -------------------------------------------------

class FontList;

/*
	[Beschreibung]

	Dieses Item dient als Transport-Medium fuer eine FontListe.
	Die Fontliste wird nicht kopiert und nicht geloescht!
*/

class EDITENG_DLLPUBLIC SvxFontListItem : public SfxPoolItem
{
private:
	const FontList*                                 pFontList;
	com::sun::star::uno::Sequence< rtl::OUString >  aFontNameSeq;

public:
	TYPEINFO();

	SvxFontListItem( const FontList* pFontLst,
                     const sal_uInt16 nId  );
	SvxFontListItem( const SvxFontListItem& rItem );

	virtual int				operator==( const SfxPoolItem& ) const;
	virtual SfxPoolItem*	Clone( SfxItemPool *pPool = 0 ) const;

	virtual	sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;

	virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
									SfxMapUnit eCoreMetric,
									SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

	const FontList*			GetFontList() const { return pFontList; }
};




#endif


