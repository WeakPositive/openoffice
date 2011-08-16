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
#ifndef _SFXIITM_HXX
#define _SFXIITM_HXX

#include <svl/poolitem.hxx>
#include <tools/fract.hxx>

/*************************************************************************/
/* FractionItem                                                          */
/*************************************************************************/

class SdrFractionItem: public SfxPoolItem {
	Fraction nValue;
public:
	TYPEINFO_VISIBILITY( SVX_DLLPUBLIC );
	SdrFractionItem(sal_uInt16 nId=0): SfxPoolItem(nId) {}
	SdrFractionItem(sal_uInt16 nId, const Fraction& rVal): SfxPoolItem(nId), nValue(rVal) {}
	SdrFractionItem(sal_uInt16 nId, SvStream& rIn);
	virtual int              operator==(const SfxPoolItem&) const;
    virtual SfxItemPresentation GetPresentation(SfxItemPresentation ePresentation, SfxMapUnit eCoreMetric, SfxMapUnit ePresentationMetric, String &rText, const IntlWrapper * = 0) const;
	virtual SfxPoolItem*     Create(SvStream&, sal_uInt16 nVer) const;
	virtual SvStream&        Store(SvStream&, sal_uInt16 nItemVers) const;
	virtual SfxPoolItem*     Clone(SfxItemPool *pPool=NULL) const;
			const Fraction&  GetValue() const { return nValue; }
			void             SetValue(const Fraction& rVal) { nValue = rVal; }
#ifdef SDR_ISPOOLABLE
	virtual int IsPoolable() const;
#endif
};


#endif
