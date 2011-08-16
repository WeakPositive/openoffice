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
#ifndef _SVX_ULSPITEM_HXX
#define _SVX_ULSPITEM_HXX

// include ---------------------------------------------------------------

#include <svl/poolitem.hxx>
#include <editeng/editengdllapi.h>

namespace rtl
{
	class OUString;
}

// class SvxULSpaceItem --------------------------------------------------

/*
[Beschreibung]
Dieses Item beschreibt den oberen und unteren Rand einer Seite oder Absatz.
*/

#define	ULSPACE_16_VERSION	((sal_uInt16)0x0001)

class EDITENG_DLLPUBLIC SvxULSpaceItem : public SfxPoolItem
{
	sal_uInt16 nUpper;  //Oberer Rand
	sal_uInt16 nLower;  //Unterer Rand
	sal_uInt16 nPropUpper, nPropLower;		// relativ oder absolut (=100%)
public:
	TYPEINFO();

    SvxULSpaceItem( const sal_uInt16 nId  );
	SvxULSpaceItem( const sal_uInt16 nUp, const sal_uInt16 nLow,
                    const sal_uInt16 nId  );
	inline SvxULSpaceItem& operator=( const SvxULSpaceItem &rCpy );

	// "pure virtual Methoden" vom SfxPoolItem
	virtual int 			 operator==( const SfxPoolItem& ) const;

	virtual	sal_Bool        	 QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
	virtual	sal_Bool			 PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

	virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
									SfxMapUnit eCoreMetric,
									SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

	virtual SfxPoolItem*	 Clone( SfxItemPool *pPool = 0 ) const;
	virtual SfxPoolItem*	 Create(SvStream &, sal_uInt16) const;
	virtual SvStream&		 Store(SvStream &, sal_uInt16 nItemVersion ) const;
	virtual sal_uInt16			 GetVersion( sal_uInt16 nFileVersion ) const;
	virtual int				 ScaleMetrics( long nMult, long nDiv );
	virtual	int				 HasMetrics() const;

	inline void SetUpper( const sal_uInt16 nU, const sal_uInt16 nProp = 100 );
	inline void SetLower( const sal_uInt16 nL, const sal_uInt16 nProp = 100 );

	void SetUpperValue( const sal_uInt16 nU ) { nUpper = nU; }
	void SetLowerValue( const sal_uInt16 nL ) { nLower = nL; }
	void SetPropUpper( const sal_uInt16 nU ) { nPropUpper = nU; }
	void SetPropLower( const sal_uInt16 nL ) { nPropLower = nL; }

	sal_uInt16 GetUpper() const { return nUpper; }
	sal_uInt16 GetLower() const { return nLower; }
	sal_uInt16 GetPropUpper() const { return nPropUpper; }
	sal_uInt16 GetPropLower() const { return nPropLower; }
};

inline SvxULSpaceItem &SvxULSpaceItem::operator=( const SvxULSpaceItem &rCpy )
{
	nUpper = rCpy.GetUpper();
	nLower = rCpy.GetLower();
	nPropUpper = rCpy.GetPropUpper();
	nPropLower = rCpy.GetPropLower();
	return *this;
}

inline void SvxULSpaceItem::SetUpper( const sal_uInt16 nU, const sal_uInt16 nProp )
{
	nUpper = sal_uInt16((sal_uInt32(nU) * nProp ) / 100); nPropUpper = nProp;
}
inline void SvxULSpaceItem::SetLower( const sal_uInt16 nL, const sal_uInt16 nProp )
{
	nLower = sal_uInt16((sal_uInt32(nL) * nProp ) / 100); nPropLower = nProp;
}

#endif

