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

#ifndef _SDGTRITM_HXX
#define _SDGTRITM_HXX

#include <svx/sdprcitm.hxx>
#include <svx/svddef.hxx>
#include "svx/svxdllapi.h"

//--------------------------
// SdrGrafTransparenceItem -
//--------------------------

class SVX_DLLPUBLIC SdrGrafTransparenceItem : public SdrPercentItem
{
public:

							TYPEINFO();
							SdrGrafTransparenceItem( sal_uInt16 nTransparencePercent = 0 ) : SdrPercentItem( SDRATTR_GRAFTRANSPARENCE, nTransparencePercent ) {}
							SdrGrafTransparenceItem( SvStream& rIn ) : SdrPercentItem( SDRATTR_GRAFTRANSPARENCE, rIn ) {}

	virtual SfxPoolItem*	Clone( SfxItemPool* pPool = NULL ) const;
	virtual SfxPoolItem*	Create( SvStream& rIn, sal_uInt16 nVer ) const;
};

#endif // _SDGTRITM_HXX
