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
#ifndef _SFXPTITEM_HXX
#define _SFXPTITEM_HXX

#include "svl/svldllapi.h"
#include <svl/poolitem.hxx>
#include <tools/gen.hxx>

class SvStream;

DBG_NAMEEX_VISIBILITY(SfxPointItem, SVL_DLLPUBLIC)

// -----------------------------------------------------------------------

class SVL_DLLPUBLIC SfxPointItem: public SfxPoolItem
{
	Point					 aVal;

public:
							 TYPEINFO();
							 SfxPointItem();
							 SfxPointItem( sal_uInt16 nWhich, const Point& rVal );
							 SfxPointItem( sal_uInt16 nWhich, SvStream & );
							 SfxPointItem( const SfxPointItem& );
							 ~SfxPointItem() {
								 DBG_DTOR(SfxPointItem, 0); }

	virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
									SfxMapUnit eCoreMetric,
									SfxMapUnit ePresMetric,
									XubString &rText,
                                    const IntlWrapper * = 0 ) const;

	virtual int 			 operator==( const SfxPoolItem& ) const;

	virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
	virtual SfxPoolItem*	 Create(SvStream &, sal_uInt16 nItemVersion) const;
	virtual SvStream&		 Store(SvStream &, sal_uInt16 nItemVersion) const;

	const Point&    	 	 GetValue() const { return aVal; }
			void			 SetValue( const Point& rNewVal ) {
								 DBG_ASSERT( GetRefCount() == 0, "SetValue() with pooled item" );
								 aVal = rNewVal;
							 }

	virtual	sal_Bool 			 QueryValue( com::sun::star::uno::Any& rVal,
							 			 sal_uInt8 nMemberId = 0 ) const;
	virtual	sal_Bool 			 PutValue( const com::sun::star::uno::Any& rVal,
						   			   sal_uInt8 nMemberId = 0 );
};

#endif
