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
#ifndef _SVX_PRNTITEM_HXX
#define _SVX_PRNTITEM_HXX

// include ---------------------------------------------------------------

#include <svl/eitem.hxx>
#include "editeng/editengdllapi.h"



// class SvxPrintItem ----------------------------------------------------


/*
[Beschreibung]
Dieses Item beschreibt eine logische Variable "Drucken ja oder nein".
*/


class EDITENG_DLLPUBLIC SvxPrintItem : public SfxBoolItem
{
public:
	TYPEINFO();

    SvxPrintItem( const sal_uInt16 nId , const sal_Bool bPrt = sal_True );
	inline SvxPrintItem &operator=( const SvxPrintItem &rCpy );

	// "pure virtual Methoden" vom SfxPoolItem
	virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
	virtual SfxPoolItem*    Create(SvStream &, sal_uInt16) const;
	virtual SvStream&		Store(SvStream &, sal_uInt16 nItemVersion ) const;

	virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
									SfxMapUnit eCoreMetric,
									SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;
};

inline SvxPrintItem::SvxPrintItem( const sal_uInt16 nId, const sal_Bool bPrt )
	: SfxBoolItem( nId, bPrt )
{}

inline SvxPrintItem &SvxPrintItem::operator=( const SvxPrintItem &rCpy )
{
	SetValue( rCpy.GetValue() );
	return *this;
}




#endif

