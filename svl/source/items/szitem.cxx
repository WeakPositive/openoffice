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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svl.hxx"

#include <svl/szitem.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/awt/Size.hpp>
#include <tools/stream.hxx>
#include <tools/gen.hxx>

#include <svl/poolitem.hxx>
#include <svl/memberid.hrc>

// STATIC DATA -----------------------------------------------------------

DBG_NAME(SfxSizeItem)

// -----------------------------------------------------------------------

TYPEINIT1_AUTOFACTORY(SfxSizeItem, SfxPoolItem);

// -----------------------------------------------------------------------

SfxSizeItem::SfxSizeItem()
{
	DBG_CTOR(SfxSizeItem, 0);
}

// -----------------------------------------------------------------------

SfxSizeItem::SfxSizeItem( sal_uInt16 nW, const Size& rVal ) :
	SfxPoolItem( nW ),
	aVal( rVal )
{
	DBG_CTOR(SfxSizeItem, 0);
}

// -----------------------------------------------------------------------

SfxSizeItem::SfxSizeItem( sal_uInt16 nW, SvStream &rStream ) :
	SfxPoolItem( nW )
{
	DBG_CTOR(SfxSizeItem, 0);
	rStream >> aVal;
}

// -----------------------------------------------------------------------

SfxSizeItem::SfxSizeItem( const SfxSizeItem& rItem ) :
	SfxPoolItem( rItem ),
	aVal( rItem.aVal )
{
	DBG_CTOR(SfxSizeItem, 0);
}

// -----------------------------------------------------------------------

SfxItemPresentation SfxSizeItem::GetPresentation
(
	SfxItemPresentation 	/*ePresentation*/,
	SfxMapUnit				/*eCoreMetric*/,
	SfxMapUnit				/*ePresentationMetric*/,
	XubString& 				rText,
    const IntlWrapper *
)	const
{
	DBG_CHKTHIS(SfxSizeItem, 0);
	rText = UniString::CreateFromInt32(aVal.Width());
	rText.AppendAscii(RTL_CONSTASCII_STRINGPARAM(", "));
	rText += UniString::CreateFromInt32(aVal.Height());
	rText.AppendAscii(RTL_CONSTASCII_STRINGPARAM(", "));
	return SFX_ITEM_PRESENTATION_NAMELESS;
}

// -----------------------------------------------------------------------

int SfxSizeItem::operator==( const SfxPoolItem& rItem ) const
{
	DBG_CHKTHIS(SfxSizeItem, 0);
	DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );
	return ((SfxSizeItem&)rItem).aVal == aVal;
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxSizeItem::Clone(SfxItemPool *) const
{
	DBG_CHKTHIS(SfxSizeItem, 0);
	return new SfxSizeItem( *this );
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxSizeItem::Create(SvStream &rStream, sal_uInt16 ) const
{
	DBG_CHKTHIS(SfxSizeItem, 0);
	Size aStr;
	rStream >> aStr;
	return new SfxSizeItem(Which(), aStr);
}

// -----------------------------------------------------------------------

SvStream& SfxSizeItem::Store(SvStream &rStream, sal_uInt16 ) const
{
	DBG_CHKTHIS(SfxSizeItem, 0);
	rStream << aVal;
	return rStream;
}

// -----------------------------------------------------------------------
sal_Bool  SfxSizeItem::QueryValue( com::sun::star::uno::Any& rVal,
							   sal_uInt8 nMemberId ) const
{
	sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
	nMemberId &= ~CONVERT_TWIPS;

	Size aTmp(aVal);
    if( bConvert )
	{
		aTmp.Height() = ( aTmp.Height() * 127 + 36) / 72;
		aTmp.Width() = ( aTmp.Width() * 127 + 36) / 72;
	}

    switch ( nMemberId )
    {
        case 0:
        {
            rVal <<= com::sun::star::awt::Size( aTmp.getWidth(), aTmp.getHeight() );
            break;
        }
        case MID_WIDTH:
            rVal <<= aTmp.getWidth(); break;
        case MID_HEIGHT:
            rVal <<= aTmp.getHeight(); break;
        default: DBG_ERROR("Wrong MemberId!"); return sal_False;
    }

	return sal_True;
}

// -----------------------------------------------------------------------
sal_Bool SfxSizeItem::PutValue( const com::sun::star::uno::Any& rVal,
							sal_uInt8 nMemberId )
{
	sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
	nMemberId &= ~CONVERT_TWIPS;

	sal_Bool bRet = sal_False;
    com::sun::star::awt::Size aValue;
    sal_Int32 nVal = 0;
    if ( !nMemberId )
        bRet = ( rVal >>= aValue );
    else
    {
        bRet = ( rVal >>= nVal );
        if ( nMemberId == MID_WIDTH )
        {
            aValue.Width = nVal;
            aValue.Height = aVal.Height();
        }
        else
        {
            aValue.Height = nVal;
            aValue.Width = aVal.Width();
        }
    }

    if ( bRet )
    {
        Size aTmp( aValue.Width, aValue.Height );
        if( bConvert )
        {
            aTmp.Height() = ( aTmp.Height() * 72 + 63) / 127;
            aTmp.Width() = ( aTmp.Width() * 72 + 63) / 127;
        }

		aVal = aTmp;
	}

	return bRet;
}



