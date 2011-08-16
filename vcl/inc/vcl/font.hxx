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

#ifndef _SV_FONT_HXX
#define _SV_FONT_HXX

#include <vcl/dllapi.h>
#include <tools/gen.hxx>
#include <tools/string.hxx>
#include <i18npool/lang.h>
#include <tools/color.hxx>
#include <vcl/vclenum.hxx>
#include <vcl/fntstyle.hxx>

class SvStream;
#define FontAlign TextAlign

class Impl_Font;
class ImplFontAttributes;

// --------
// - Font -
// --------

class VCL_DLLPUBLIC Font
{
private:
	Impl_Font*			mpImplFont;
	void				MakeUnique();

public:
						Font();
						Font( const Font& );
						Font( const String& rFamilyName, const Size& );
						Font( const String& rFamilyName, const String& rStyleName, const Size& );
						Font( FontFamily eFamily, const Size& );
						~Font();

	void				SetColor( const Color& );
	const Color&		GetColor() const;
	void				SetFillColor( const Color& );
	const Color&		GetFillColor() const;
	void				SetTransparent( sal_Bool bTransparent );
	sal_Bool				IsTransparent() const;
	void				SetAlign( FontAlign );
	FontAlign			GetAlign() const;

	void				SetName( const String& rFamilyName );
	const String&		GetName() const;
	void				SetStyleName( const String& rStyleName );
	const String&		GetStyleName() const;
	void				SetSize( const Size& );
	const Size& 		GetSize() const;
	void				SetHeight( long nHeight );
	long				GetHeight() const;
	void				SetWidth( long nWidth );
	long				GetWidth() const;

	void				SetFamily( FontFamily );
	FontFamily			GetFamily() const;
	void				SetCharSet( rtl_TextEncoding );
	rtl_TextEncoding	GetCharSet() const;
	void				SetLanguage( LanguageType );
	LanguageType		GetLanguage() const;
	void				SetCJKContextLanguage( LanguageType );
	LanguageType		GetCJKContextLanguage() const;
	void				SetPitch( FontPitch ePitch );
	FontPitch			GetPitch() const;

	void				SetOrientation( short nLineOrientation );
	short				GetOrientation() const;
	void				SetVertical( sal_Bool bVertical );
	sal_Bool				IsVertical() const;
	void				SetKerning( FontKerning nKerning );
	FontKerning 		GetKerning() const;
	sal_Bool				IsKerning() const;

	void				SetWeight( FontWeight );
	FontWeight			GetWeight() const;
	void				SetWidthType( FontWidth );
	FontWidth			GetWidthType() const;
	void				SetItalic( FontItalic );
	FontItalic			GetItalic() const;
	void				SetOutline( sal_Bool bOutline );
	sal_Bool				IsOutline() const;
	void				SetShadow( sal_Bool bShadow );
	sal_Bool				IsShadow() const;
	void				SetRelief( FontRelief );
	FontRelief			GetRelief() const;
	void				SetUnderline( FontUnderline );
	FontUnderline		GetUnderline() const;
	void				SetOverline( FontUnderline );
	FontUnderline		GetOverline() const;
	void				SetStrikeout( FontStrikeout );
	FontStrikeout		GetStrikeout() const;
	void				SetEmphasisMark( FontEmphasisMark );
	FontEmphasisMark	GetEmphasisMark() const;
	void				SetWordLineMode( sal_Bool bWordLine );
	sal_Bool				IsWordLineMode() const;

	void				Merge( const Font& rFont );
	void                GetFontAttributes( ImplFontAttributes& rAttrs ) const;

	Font&				operator=( const Font& );
	sal_Bool				operator==( const Font& ) const;
	sal_Bool				operator!=( const Font& rFont ) const
							{ return !(Font::operator==( rFont )); }
	sal_Bool				IsSameInstance( const Font& ) const;

	friend VCL_DLLPUBLIC SvStream&	operator>>( SvStream& rIStm, Font& );
	friend VCL_DLLPUBLIC SvStream&	operator<<( SvStream& rOStm, const Font& );
    
    static Font identifyFont( const void* pBuffer, sal_uInt32 nLen );
};

#endif	// _VCL_FONT_HXX
