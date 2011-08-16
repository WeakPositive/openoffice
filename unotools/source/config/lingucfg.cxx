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
#include "precompiled_unotools.hxx"


#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include "com/sun/star/util/XMacroExpander.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"
#include <rtl/uri.hxx>
#include <vos/mutex.hxx>
#include <i18npool/mslangid.hxx>
#include <tools/debug.hxx>
#include <tools/string.hxx>
#include <unotools/lingucfg.hxx>
#include <unotools/linguprops.hxx>

#include <comphelper/processfactory.hxx>

#include <itemholder1.hxx>

using namespace rtl;
using namespace com::sun::star;

#define A2OU(x)        ::rtl::OUString::createFromAscii( x )
#define EXPAND_PROTOCOL     "vnd.sun.star.expand:"
#define FILE_PROTOCOL       "file:///"

///////////////////////////////////////////////////////////////////////////


static osl::Mutex &  GetOwnMutex()
{
    static osl::Mutex aMutex;
    return aMutex;
}


///////////////////////////////////////////////////////////////////////////


static sal_Bool lcl_SetLocale( sal_Int16 &rLanguage, const uno::Any &rVal )
{
    sal_Bool bSucc = sal_False;

    lang::Locale aNew;
    if (rVal >>= aNew)	// conversion successful?
    {
        sal_Int16 nNew = MsLangId::convertLocaleToLanguage( aNew );
        if (nNew != rLanguage)
        {
            rLanguage = nNew;
            bSucc = sal_True;
        }
    }
    return bSucc;
}


static inline const OUString lcl_LanguageToCfgLocaleStr( sal_Int16 nLanguage )
{
    OUString aRes;
    if (LANGUAGE_SYSTEM != nLanguage)
        aRes = MsLangId::convertLanguageToIsoString( nLanguage );
    return aRes;
}


static sal_Int16 lcl_CfgAnyToLanguage( const uno::Any &rVal )
{
    OUString aTmp;
    rVal >>= aTmp;
    return (aTmp.getLength() == 0) ? LANGUAGE_SYSTEM : MsLangId::convertIsoStringToLanguage( aTmp );
}


//////////////////////////////////////////////////////////////////////

SvtLinguOptions::SvtLinguOptions()
{
    nDefaultLanguage = LANGUAGE_NONE;
	nDefaultLanguage_CJK = LANGUAGE_NONE;
	nDefaultLanguage_CTL = LANGUAGE_NONE;

	// general options
	bIsUseDictionaryList	=
	bIsIgnoreControlCharacters	= sal_True;

	// spelling options
	bIsSpellCapitalization	=
	bIsSpellSpecial			= sal_True;
	bIsSpellAuto			=
	bIsSpellReverse			=
	bIsSpellWithDigits		=
	bIsSpellUpperCase		= sal_False;

    // text conversion options
    bIsIgnorePostPositionalWord     = sal_True;
    bIsAutoCloseDialog              =
    bIsShowEntriesRecentlyUsedFirst =
    bIsAutoReplaceUniqueEntries     = sal_False;
    bIsDirectionToSimplified        = sal_True;
    bIsUseCharacterVariants         =
    bIsTranslateCommonTerms         =
    bIsReverseMapping               = sal_False;

    bROIsDirectionToSimplified      =
    bROIsUseCharacterVariants       =
    bROIsTranslateCommonTerms       =
    bROIsReverseMapping             = sal_False;

	// hyphenation options
	bIsHyphSpecial			= sal_True;
	bIsHyphAuto				= sal_False;
	nHyphMinLeading			=
	nHyphMinTrailing		= 2;
	nHyphMinWordLength		= 0;

    nDataFilesChangedCheckValue = 0;

    //grammar options
    bIsGrammarAuto = sal_False,
    bIsGrammarInteractive = sal_False;

}


//////////////////////////////////////////////////////////////////////


class SvtLinguConfigItem : public utl::ConfigItem
{
    SvtLinguOptions     aOpt;

    // disallow copy-constructor and assignment-operator for now
    SvtLinguConfigItem( const SvtLinguConfigItem & );
    SvtLinguConfigItem & operator = ( const SvtLinguConfigItem & );

    static sal_Bool GetHdlByName( sal_Int32 &rnHdl, const OUString &rPropertyName, sal_Bool bFullPropName = sal_False );
    static const uno::Sequence< OUString > & GetPropertyNames();
    sal_Bool                LoadOptions( const uno::Sequence< OUString > &rProperyNames );
    sal_Bool                SaveOptions( const uno::Sequence< OUString > &rProperyNames );

public:
    SvtLinguConfigItem();
    virtual ~SvtLinguConfigItem();

    // utl::ConfigItem
    virtual void    Notify( const com::sun::star::uno::Sequence< rtl::OUString > &rPropertyNames );
    virtual void    Commit();

    // make some protected functions of utl::ConfigItem public
    using utl::ConfigItem::GetNodeNames;
    using utl::ConfigItem::GetProperties;
    //using utl::ConfigItem::PutProperties;
    //using utl::ConfigItem::SetSetProperties;
    using utl::ConfigItem::ReplaceSetProperties;
    //using utl::ConfigItem::GetReadOnlyStates;


    com::sun::star::uno::Any
            GetProperty( const rtl::OUString &rPropertyName ) const;
    com::sun::star::uno::Any
            GetProperty( sal_Int32 nPropertyHandle ) const;

    sal_Bool    SetProperty( const rtl::OUString &rPropertyName,
                         const com::sun::star::uno::Any &rValue );
    sal_Bool    SetProperty( sal_Int32 nPropertyHandle,
                         const com::sun::star::uno::Any &rValue );

    sal_Bool    GetOptions( SvtLinguOptions &rOptions ) const;
    sal_Bool    SetOptions( const SvtLinguOptions &rOptions );

    sal_Bool    IsReadOnly( const rtl::OUString &rPropertyName ) const;
    sal_Bool    IsReadOnly( sal_Int32 nPropertyHandle ) const;
};


SvtLinguConfigItem::SvtLinguConfigItem() :
    utl::ConfigItem( String::CreateFromAscii( "Office.Linguistic" ) )
{
    LoadOptions( GetPropertyNames() );
    ClearModified();

    // request notify events when properties change
    EnableNotification( GetPropertyNames() );
}


SvtLinguConfigItem::~SvtLinguConfigItem()
{
    //! Commit (SaveOptions) will be called by the d-tor of the base called !
}


void SvtLinguConfigItem::Notify( const uno::Sequence< OUString > &rPropertyNames )
{
    LoadOptions( rPropertyNames );
	NotifyListeners(0);
}


void SvtLinguConfigItem::Commit()
{
    SaveOptions( GetPropertyNames() );
}


static struct NamesToHdl
{
    const char   *pFullPropName;      // full qualified name as used in configuration
    const char   *pPropName;          // property name only (atom) of above
    sal_Int32   nHdl;               // numeric handle representing the property
}aNamesToHdl[] =
{
{/*  0 */    "General/DefaultLocale",                         UPN_DEFAULT_LOCALE,                    UPH_DEFAULT_LOCALE},
{/*  1 */    "General/DictionaryList/ActiveDictionaries",     UPN_ACTIVE_DICTIONARIES,               UPH_ACTIVE_DICTIONARIES},
{/*  2 */    "General/DictionaryList/IsUseDictionaryList",    UPN_IS_USE_DICTIONARY_LIST,            UPH_IS_USE_DICTIONARY_LIST},
{/*  3 */    "General/IsIgnoreControlCharacters",             UPN_IS_IGNORE_CONTROL_CHARACTERS,      UPH_IS_IGNORE_CONTROL_CHARACTERS},
{/*  5 */    "General/DefaultLocale_CJK",                     UPN_DEFAULT_LOCALE_CJK,                UPH_DEFAULT_LOCALE_CJK},
{/*  6 */    "General/DefaultLocale_CTL",                     UPN_DEFAULT_LOCALE_CTL,                UPH_DEFAULT_LOCALE_CTL},

{/*  7 */    "SpellChecking/IsSpellUpperCase",                UPN_IS_SPELL_UPPER_CASE,               UPH_IS_SPELL_UPPER_CASE},
{/*  8 */    "SpellChecking/IsSpellWithDigits",               UPN_IS_SPELL_WITH_DIGITS,              UPH_IS_SPELL_WITH_DIGITS},
{/*  9 */    "SpellChecking/IsSpellCapitalization",           UPN_IS_SPELL_CAPITALIZATION,           UPH_IS_SPELL_CAPITALIZATION},
{/* 10 */    "SpellChecking/IsSpellAuto",                     UPN_IS_SPELL_AUTO,                     UPH_IS_SPELL_AUTO},
{/* 11 */    "SpellChecking/IsSpellSpecial",                  UPN_IS_SPELL_SPECIAL,                  UPH_IS_SPELL_SPECIAL},
{/* 14 */    "SpellChecking/IsReverseDirection",              UPN_IS_WRAP_REVERSE,                   UPH_IS_WRAP_REVERSE},

{/* 15 */    "Hyphenation/MinLeading",                        UPN_HYPH_MIN_LEADING,                  UPH_HYPH_MIN_LEADING},
{/* 16 */    "Hyphenation/MinTrailing",                       UPN_HYPH_MIN_TRAILING,                 UPH_HYPH_MIN_TRAILING},
{/* 17 */    "Hyphenation/MinWordLength",                     UPN_HYPH_MIN_WORD_LENGTH,              UPH_HYPH_MIN_WORD_LENGTH},
{/* 18 */    "Hyphenation/IsHyphSpecial",                     UPN_IS_HYPH_SPECIAL,                   UPH_IS_HYPH_SPECIAL},
{/* 19 */    "Hyphenation/IsHyphAuto",                        UPN_IS_HYPH_AUTO,                      UPH_IS_HYPH_AUTO},

{/* 20 */    "TextConversion/ActiveConversionDictionaries",   UPN_ACTIVE_CONVERSION_DICTIONARIES,        UPH_ACTIVE_CONVERSION_DICTIONARIES},
{/* 21 */    "TextConversion/IsIgnorePostPositionalWord",     UPN_IS_IGNORE_POST_POSITIONAL_WORD,        UPH_IS_IGNORE_POST_POSITIONAL_WORD},
{/* 22 */    "TextConversion/IsAutoCloseDialog",              UPN_IS_AUTO_CLOSE_DIALOG,                  UPH_IS_AUTO_CLOSE_DIALOG},
{/* 23 */    "TextConversion/IsShowEntriesRecentlyUsedFirst", UPN_IS_SHOW_ENTRIES_RECENTLY_USED_FIRST,   UPH_IS_SHOW_ENTRIES_RECENTLY_USED_FIRST},
{/* 24 */    "TextConversion/IsAutoReplaceUniqueEntries",     UPN_IS_AUTO_REPLACE_UNIQUE_ENTRIES,        UPH_IS_AUTO_REPLACE_UNIQUE_ENTRIES},
{/* 25 */    "TextConversion/IsDirectionToSimplified",        UPN_IS_DIRECTION_TO_SIMPLIFIED,            UPH_IS_DIRECTION_TO_SIMPLIFIED},
{/* 26 */    "TextConversion/IsUseCharacterVariants",         UPN_IS_USE_CHARACTER_VARIANTS,             UPH_IS_USE_CHARACTER_VARIANTS},
{/* 27 */    "TextConversion/IsTranslateCommonTerms",         UPN_IS_TRANSLATE_COMMON_TERMS,             UPH_IS_TRANSLATE_COMMON_TERMS},
{/* 28 */    "TextConversion/IsReverseMapping",               UPN_IS_REVERSE_MAPPING,                    UPH_IS_REVERSE_MAPPING},

{/* 29 */    "ServiceManager/DataFilesChangedCheckValue",     UPN_DATA_FILES_CHANGED_CHECK_VALUE,        UPH_DATA_FILES_CHANGED_CHECK_VALUE},

{/* 30 */    "GrammarChecking/IsAutoCheck",                   UPN_IS_GRAMMAR_AUTO,                      UPH_IS_GRAMMAR_AUTO},
{/* 31 */    "GrammarChecking/IsInteractiveCheck",            UPN_IS_GRAMMAR_INTERACTIVE,               UPH_IS_GRAMMAR_INTERACTIVE},

            /* similar to entry 0 (thus no own configuration entry) but with different property name and type */
{            NULL,											 UPN_DEFAULT_LANGUAGE,                      UPH_DEFAULT_LANGUAGE},

{            NULL,                                            NULL,                                      -1}
};


const uno::Sequence< OUString > & SvtLinguConfigItem::GetPropertyNames()
{
    static uno::Sequence< OUString > aNames;
	static sal_Bool bInitialized = sal_False;

	if (!bInitialized)
	{
		sal_Int32 nMax = sizeof(aNamesToHdl) / sizeof(aNamesToHdl[0]);

		aNames.realloc( nMax );
		OUString *pNames = aNames.getArray();
		sal_Int32 nIdx = 0;
		for (sal_Int32 i = 0; i < nMax;  ++i)
		{
			const sal_Char *pFullPropName = aNamesToHdl[i].pFullPropName;
			if (pFullPropName)
				pNames[ nIdx++ ] = A2OU( pFullPropName );
		}
		aNames.realloc( nIdx );
		bInitialized = sal_True;
	}
	return aNames;
}


sal_Bool SvtLinguConfigItem::GetHdlByName(
	sal_Int32 &rnHdl,
	const OUString &rPropertyName,
	sal_Bool bFullPropName )
{
    NamesToHdl *pEntry = &aNamesToHdl[0];

	if (bFullPropName)
	{
		while (pEntry && pEntry->pFullPropName != NULL)
		{
			if (0 == rPropertyName.compareToAscii( pEntry->pFullPropName ))
			{
				rnHdl = pEntry->nHdl;
				break;
			}
			++pEntry;
		}
		return pEntry && pEntry->pFullPropName != NULL;
	}
	else
	{
		while (pEntry && pEntry->pPropName != NULL)
		{
			if (0 == rPropertyName.compareToAscii( pEntry->pPropName ))
			{
				rnHdl = pEntry->nHdl;
				break;
			}
			++pEntry;
		}
		return pEntry && pEntry->pPropName != NULL;
	}
}


uno::Any SvtLinguConfigItem::GetProperty( const OUString &rPropertyName ) const
{
    osl::MutexGuard aGuard( GetOwnMutex() );

    sal_Int32 nHdl;
    return GetHdlByName( nHdl, rPropertyName ) ? GetProperty( nHdl ) : uno::Any();
}


uno::Any SvtLinguConfigItem::GetProperty( sal_Int32 nPropertyHandle ) const
{
    osl::MutexGuard aGuard( GetOwnMutex() );

    uno::Any aRes;

    const sal_Int16 *pnVal = 0;
    const sal_Bool  *pbVal = 0;
    const sal_Int32 *pnInt32Val = 0;

    const SvtLinguOptions &rOpt = const_cast< SvtLinguConfigItem * >(this)->aOpt;
    switch (nPropertyHandle)
    {
        case UPH_IS_USE_DICTIONARY_LIST :   pbVal = &rOpt.bIsUseDictionaryList; break;
        case UPH_IS_IGNORE_CONTROL_CHARACTERS : pbVal = &rOpt.bIsIgnoreControlCharacters;   break;
        case UPH_IS_HYPH_AUTO :             pbVal = &rOpt.bIsHyphAuto;  break;
        case UPH_IS_HYPH_SPECIAL :          pbVal = &rOpt.bIsHyphSpecial;   break;
        case UPH_IS_SPELL_AUTO :            pbVal = &rOpt.bIsSpellAuto; break;
        case UPH_IS_SPELL_SPECIAL :         pbVal = &rOpt.bIsSpellSpecial;  break;
        case UPH_IS_WRAP_REVERSE :          pbVal = &rOpt.bIsSpellReverse;  break;
        case UPH_DEFAULT_LANGUAGE :         pnVal = &rOpt.nDefaultLanguage; break;
        case UPH_IS_SPELL_CAPITALIZATION :  pbVal = &rOpt.bIsSpellCapitalization;       break;
        case UPH_IS_SPELL_WITH_DIGITS :     pbVal = &rOpt.bIsSpellWithDigits;   break;
        case UPH_IS_SPELL_UPPER_CASE :      pbVal = &rOpt.bIsSpellUpperCase;        break;
        case UPH_HYPH_MIN_LEADING :         pnVal = &rOpt.nHyphMinLeading;      break;
        case UPH_HYPH_MIN_TRAILING :        pnVal = &rOpt.nHyphMinTrailing; break;
        case UPH_HYPH_MIN_WORD_LENGTH :     pnVal = &rOpt.nHyphMinWordLength;   break;
        case UPH_ACTIVE_DICTIONARIES :
        {
            aRes <<= rOpt.aActiveDics;
            break;
        }
        case UPH_ACTIVE_CONVERSION_DICTIONARIES :
        {
            aRes <<= rOpt.aActiveConvDics;
            break;
        }
        case UPH_DEFAULT_LOCALE :
        {
            lang::Locale aLocale( MsLangId::convertLanguageToLocale( rOpt.nDefaultLanguage, false ) );
            aRes.setValue( &aLocale, ::getCppuType((lang::Locale*)0 ));
            break;
        }
        case UPH_DEFAULT_LOCALE_CJK :
        {
            lang::Locale aLocale( MsLangId::convertLanguageToLocale( rOpt.nDefaultLanguage_CJK, false ) );
            aRes.setValue( &aLocale, ::getCppuType((lang::Locale*)0 ));
            break;
        }
        case UPH_DEFAULT_LOCALE_CTL :
        {
            lang::Locale aLocale( MsLangId::convertLanguageToLocale( rOpt.nDefaultLanguage_CTL, false ) );
            aRes.setValue( &aLocale, ::getCppuType((lang::Locale*)0 ));
            break;
        }
        case UPH_IS_IGNORE_POST_POSITIONAL_WORD :       pbVal = &rOpt.bIsIgnorePostPositionalWord; break;
        case UPH_IS_AUTO_CLOSE_DIALOG :                 pbVal = &rOpt.bIsAutoCloseDialog; break;
        case UPH_IS_SHOW_ENTRIES_RECENTLY_USED_FIRST :  pbVal = &rOpt.bIsShowEntriesRecentlyUsedFirst; break;
        case UPH_IS_AUTO_REPLACE_UNIQUE_ENTRIES :       pbVal = &rOpt.bIsAutoReplaceUniqueEntries; break;

        case UPH_IS_DIRECTION_TO_SIMPLIFIED:            pbVal = &rOpt.bIsDirectionToSimplified; break;
        case UPH_IS_USE_CHARACTER_VARIANTS :            pbVal = &rOpt.bIsUseCharacterVariants; break;
        case UPH_IS_TRANSLATE_COMMON_TERMS :            pbVal = &rOpt.bIsTranslateCommonTerms; break;
        case UPH_IS_REVERSE_MAPPING :                   pbVal = &rOpt.bIsReverseMapping; break;

        case UPH_DATA_FILES_CHANGED_CHECK_VALUE :       pnInt32Val = &rOpt.nDataFilesChangedCheckValue; break;
        case UPH_IS_GRAMMAR_AUTO:                       pbVal = &rOpt.bIsGrammarAuto; break;
        case UPH_IS_GRAMMAR_INTERACTIVE:                pbVal = &rOpt.bIsGrammarInteractive; break;
        default :
            DBG_ASSERT( 0, "unexpected property handle" );
    }

    if (pbVal)
        aRes <<= *pbVal;
    else if (pnVal)
        aRes <<= *pnVal;
    else if (pnInt32Val)
        aRes <<= *pnInt32Val;

    return aRes;
}


sal_Bool SvtLinguConfigItem::SetProperty( const OUString &rPropertyName, const uno::Any &rValue )
{
    osl::MutexGuard aGuard( GetOwnMutex() );

    sal_Bool bSucc = sal_False;
    sal_Int32 nHdl;
    if (GetHdlByName( nHdl, rPropertyName ))
        bSucc = SetProperty( nHdl, rValue );
    return bSucc;
}


sal_Bool SvtLinguConfigItem::SetProperty( sal_Int32 nPropertyHandle, const uno::Any &rValue )
{
    osl::MutexGuard aGuard( GetOwnMutex() );

    sal_Bool bSucc = sal_False;
    if (!rValue.hasValue())
        return bSucc;

    sal_Bool bMod = sal_False;

    sal_Int16 *pnVal = 0;
    sal_Bool  *pbVal = 0;
    sal_Int32 *pnInt32Val = 0;

    SvtLinguOptions &rOpt = aOpt;
    switch (nPropertyHandle)
    {
        case UPH_IS_USE_DICTIONARY_LIST :   pbVal = &rOpt.bIsUseDictionaryList;    break;
        case UPH_IS_IGNORE_CONTROL_CHARACTERS : pbVal = &rOpt.bIsIgnoreControlCharacters;  break;
        case UPH_IS_HYPH_AUTO :             pbVal = &rOpt.bIsHyphAuto; break;
        case UPH_IS_HYPH_SPECIAL :          pbVal = &rOpt.bIsHyphSpecial;  break;
        case UPH_IS_SPELL_AUTO :            pbVal = &rOpt.bIsSpellAuto;    break;
        case UPH_IS_SPELL_SPECIAL :         pbVal = &rOpt.bIsSpellSpecial; break;
        case UPH_IS_WRAP_REVERSE :          pbVal = &rOpt.bIsSpellReverse; break;
        case UPH_DEFAULT_LANGUAGE :         pnVal = &rOpt.nDefaultLanguage;    break;
        case UPH_IS_SPELL_CAPITALIZATION :  pbVal = &rOpt.bIsSpellCapitalization;      break;
        case UPH_IS_SPELL_WITH_DIGITS :     pbVal = &rOpt.bIsSpellWithDigits;  break;
        case UPH_IS_SPELL_UPPER_CASE :      pbVal = &rOpt.bIsSpellUpperCase;       break;
        case UPH_HYPH_MIN_LEADING :         pnVal = &rOpt.nHyphMinLeading;     break;
        case UPH_HYPH_MIN_TRAILING :        pnVal = &rOpt.nHyphMinTrailing;    break;
        case UPH_HYPH_MIN_WORD_LENGTH :     pnVal = &rOpt.nHyphMinWordLength;  break;
        case UPH_ACTIVE_DICTIONARIES :
        {
            rValue >>= rOpt.aActiveDics;
            bMod = sal_True;
            break;
        }
        case UPH_ACTIVE_CONVERSION_DICTIONARIES :
        {
            rValue >>= rOpt.aActiveConvDics;
            bMod = sal_True;
            break;
        }
        case UPH_DEFAULT_LOCALE :
        {
            bSucc = lcl_SetLocale( rOpt.nDefaultLanguage, rValue );
            bMod = bSucc;
            break;
        }
        case UPH_DEFAULT_LOCALE_CJK :
        {
            bSucc = lcl_SetLocale( rOpt.nDefaultLanguage_CJK, rValue );
            bMod = bSucc;
            break;
        }
        case UPH_DEFAULT_LOCALE_CTL :
        {
            bSucc = lcl_SetLocale( rOpt.nDefaultLanguage_CTL, rValue );
            bMod = bSucc;
            break;
        }
        case UPH_IS_IGNORE_POST_POSITIONAL_WORD :       pbVal = &rOpt.bIsIgnorePostPositionalWord; break;
        case UPH_IS_AUTO_CLOSE_DIALOG :                 pbVal = &rOpt.bIsAutoCloseDialog; break;
        case UPH_IS_SHOW_ENTRIES_RECENTLY_USED_FIRST :  pbVal = &rOpt.bIsShowEntriesRecentlyUsedFirst; break;
        case UPH_IS_AUTO_REPLACE_UNIQUE_ENTRIES :       pbVal = &rOpt.bIsAutoReplaceUniqueEntries; break;

        case UPH_IS_DIRECTION_TO_SIMPLIFIED :           pbVal = &rOpt.bIsDirectionToSimplified; break;
        case UPH_IS_USE_CHARACTER_VARIANTS :            pbVal = &rOpt.bIsUseCharacterVariants; break;
        case UPH_IS_TRANSLATE_COMMON_TERMS :            pbVal = &rOpt.bIsTranslateCommonTerms; break;
        case UPH_IS_REVERSE_MAPPING :                   pbVal = &rOpt.bIsReverseMapping; break;

        case UPH_DATA_FILES_CHANGED_CHECK_VALUE :       pnInt32Val = &rOpt.nDataFilesChangedCheckValue; break;
        case UPH_IS_GRAMMAR_AUTO:                       pbVal = &rOpt.bIsGrammarAuto; break;
        case UPH_IS_GRAMMAR_INTERACTIVE:                pbVal = &rOpt.bIsGrammarInteractive; break;
        default :
            DBG_ASSERT( 0, "unexpected property handle" );
    }

    if (pbVal)
    {
        sal_Bool bNew = sal_Bool();
        if (rValue >>= bNew)
        {
            if (bNew != *pbVal)
            {
                *pbVal = bNew;
                bMod = sal_True;
            }
            bSucc = sal_True;
        }
    }
    else if (pnVal)
    {
        sal_Int16 nNew = sal_Int16();
        if (rValue >>= nNew)
        {
            if (nNew != *pnVal)
            {
                *pnVal = nNew;
                bMod = sal_True;
            }
            bSucc = sal_True;
        }
    }
    else if (pnInt32Val)
    {
        sal_Int32 nNew = sal_Int32();
        if (rValue >>= nNew)
        {
            if (nNew != *pnInt32Val)
            {
                *pnInt32Val = nNew;
                bMod = sal_True;
            }
            bSucc = sal_True;
        }
    }

    if (bMod)
        SetModified();

	NotifyListeners(0);
    return bSucc;
}


sal_Bool SvtLinguConfigItem::GetOptions( SvtLinguOptions &rOptions ) const
{
    osl::MutexGuard aGuard( GetOwnMutex() );

    rOptions = aOpt;
    return sal_True;
}


sal_Bool SvtLinguConfigItem::SetOptions( const SvtLinguOptions &rOptions )
{
    osl::MutexGuard aGuard( GetOwnMutex() );

    aOpt = rOptions;
    SetModified();
	NotifyListeners(0);
    return sal_True;
}


sal_Bool SvtLinguConfigItem::LoadOptions( const uno::Sequence< OUString > &rProperyNames )
{
    osl::MutexGuard aGuard( GetOwnMutex() );

    sal_Bool bRes = sal_False;

    const OUString *pProperyNames = rProperyNames.getConstArray();
    sal_Int32 nProps = rProperyNames.getLength();

    const uno::Sequence< uno::Any > aValues = GetProperties( rProperyNames );
    const uno::Sequence< sal_Bool > aROStates = GetReadOnlyStates( rProperyNames );

    if (nProps  &&  aValues.getLength() == nProps &&  aROStates.getLength() == nProps)
    {
        SvtLinguOptions &rOpt = aOpt;

        const uno::Any *pValue = aValues.getConstArray();
        const sal_Bool *pROStates = aROStates.getConstArray();
        for (sal_Int32 i = 0;  i < nProps;  ++i)
        {
            const uno::Any &rVal = pValue[i];
            sal_Int32 nPropertyHandle;
            GetHdlByName( nPropertyHandle, pProperyNames[i], sal_True );
            switch ( nPropertyHandle )
            {
                case UPH_DEFAULT_LOCALE :
                    { rOpt.bRODefaultLanguage = pROStates[i]; rOpt.nDefaultLanguage = lcl_CfgAnyToLanguage( rVal ); } break;
                case UPH_ACTIVE_DICTIONARIES :
                    { rOpt.bROActiveDics = pROStates[i]; rVal >>= rOpt.aActiveDics;   } break;
                case UPH_IS_USE_DICTIONARY_LIST :
                    { rOpt.bROIsUseDictionaryList = pROStates[i]; rVal >>= rOpt.bIsUseDictionaryList;  } break;
                case UPH_IS_IGNORE_CONTROL_CHARACTERS :
                    { rOpt.bROIsIgnoreControlCharacters = pROStates[i]; rVal >>= rOpt.bIsIgnoreControlCharacters;    } break;
                case UPH_DEFAULT_LOCALE_CJK :
                    { rOpt.bRODefaultLanguage_CJK = pROStates[i]; rOpt.nDefaultLanguage_CJK = lcl_CfgAnyToLanguage( rVal );    } break;
                case UPH_DEFAULT_LOCALE_CTL :
                    { rOpt.bRODefaultLanguage_CTL = pROStates[i]; rOpt.nDefaultLanguage_CTL = lcl_CfgAnyToLanguage( rVal );    } break;

                case UPH_IS_SPELL_UPPER_CASE :
                    { rOpt.bROIsSpellUpperCase = pROStates[i]; rVal >>= rOpt.bIsSpellUpperCase; } break;
                case UPH_IS_SPELL_WITH_DIGITS :
                    { rOpt.bROIsSpellWithDigits = pROStates[i]; rVal >>= rOpt.bIsSpellWithDigits;    } break;
                case UPH_IS_SPELL_CAPITALIZATION :
                    { rOpt.bROIsSpellCapitalization = pROStates[i]; rVal >>= rOpt.bIsSpellCapitalization;    } break;
                case UPH_IS_SPELL_AUTO :
                    { rOpt.bROIsSpellAuto = pROStates[i]; rVal >>= rOpt.bIsSpellAuto;  } break;
                case UPH_IS_SPELL_SPECIAL :
                    { rOpt.bROIsSpellSpecial = pROStates[i]; rVal >>= rOpt.bIsSpellSpecial;   } break;
                case UPH_IS_WRAP_REVERSE :
                    { rOpt.bROIsSpellReverse = pROStates[i]; rVal >>= rOpt.bIsSpellReverse;   } break;

                case UPH_HYPH_MIN_LEADING :
                    { rOpt.bROHyphMinLeading = pROStates[i]; rVal >>= rOpt.nHyphMinLeading;   } break;
                case UPH_HYPH_MIN_TRAILING :
                    { rOpt.bROHyphMinTrailing = pROStates[i]; rVal >>= rOpt.nHyphMinTrailing;  } break;
                case UPH_HYPH_MIN_WORD_LENGTH :
                    { rOpt.bROHyphMinWordLength = pROStates[i]; rVal >>= rOpt.nHyphMinWordLength;    } break;
                case UPH_IS_HYPH_SPECIAL :
                    { rOpt.bROIsHyphSpecial = pROStates[i]; rVal >>= rOpt.bIsHyphSpecial;    } break;
                case UPH_IS_HYPH_AUTO :
                    { rOpt.bROIsHyphAuto = pROStates[i]; rVal >>= rOpt.bIsHyphAuto;   } break;

                case UPH_ACTIVE_CONVERSION_DICTIONARIES : { rOpt.bROActiveConvDics = pROStates[i]; rVal >>= rOpt.aActiveConvDics;   } break;

                case UPH_IS_IGNORE_POST_POSITIONAL_WORD :
                    { rOpt.bROIsIgnorePostPositionalWord = pROStates[i]; rVal >>= rOpt.bIsIgnorePostPositionalWord;  } break;
                case UPH_IS_AUTO_CLOSE_DIALOG :
                    { rOpt.bROIsAutoCloseDialog = pROStates[i]; rVal >>= rOpt.bIsAutoCloseDialog;  } break;
                case UPH_IS_SHOW_ENTRIES_RECENTLY_USED_FIRST :
                    { rOpt.bROIsShowEntriesRecentlyUsedFirst = pROStates[i]; rVal >>= rOpt.bIsShowEntriesRecentlyUsedFirst;  } break;
                case UPH_IS_AUTO_REPLACE_UNIQUE_ENTRIES :
                    { rOpt.bROIsAutoReplaceUniqueEntries = pROStates[i]; rVal >>= rOpt.bIsAutoReplaceUniqueEntries;  } break;

                case UPH_IS_DIRECTION_TO_SIMPLIFIED :
                    { rOpt.bROIsDirectionToSimplified = pROStates[i];
                            if( ! (rVal >>= rOpt.bIsDirectionToSimplified) )
                            {
                                //default is locale dependent:
                                if(  rOpt.nDefaultLanguage_CJK == LANGUAGE_CHINESE_HONGKONG
                                  || rOpt.nDefaultLanguage_CJK == LANGUAGE_CHINESE_MACAU
                                  || rOpt.nDefaultLanguage_CJK == LANGUAGE_CHINESE_TRADITIONAL )
                                {
                                    rOpt.bIsDirectionToSimplified = sal_False;
                                }
                                else
                                {
                                    rOpt.bIsDirectionToSimplified = sal_True;
                                }
                            }
                    } break;
                case UPH_IS_USE_CHARACTER_VARIANTS :
                    { rOpt.bROIsUseCharacterVariants = pROStates[i]; rVal >>= rOpt.bIsUseCharacterVariants;  } break;
                case UPH_IS_TRANSLATE_COMMON_TERMS :
                    { rOpt.bROIsTranslateCommonTerms = pROStates[i]; rVal >>= rOpt.bIsTranslateCommonTerms;  } break;
                case UPH_IS_REVERSE_MAPPING :
                    { rOpt.bROIsReverseMapping = pROStates[i]; rVal >>= rOpt.bIsReverseMapping;  } break;

                case UPH_DATA_FILES_CHANGED_CHECK_VALUE :
                    { rOpt.bRODataFilesChangedCheckValue = pROStates[i]; rVal >>= rOpt.nDataFilesChangedCheckValue;  } break;

                case UPH_IS_GRAMMAR_AUTO:
                    { rOpt.bROIsGrammarAuto = pROStates[i]; rVal >>= rOpt.bIsGrammarAuto; }
                break;
                case UPH_IS_GRAMMAR_INTERACTIVE:
                    { rOpt.bROIsGrammarInteractive = pROStates[i]; rVal >>= rOpt.bIsGrammarInteractive; }
                break;

                default:
                    DBG_ASSERT( 0, "unexpected case" );
            }
        }

        bRes = sal_True;
    }
    DBG_ASSERT( bRes, "LoadOptions failed" );

    return bRes;
}


sal_Bool SvtLinguConfigItem::SaveOptions( const uno::Sequence< OUString > &rProperyNames )
{
    if (!IsModified())
        return sal_True;

    osl::MutexGuard aGuard( GetOwnMutex() );

    sal_Bool bRet = sal_False;
    const uno::Type &rBOOL     = ::getBooleanCppuType();
    const uno::Type &rINT16    = ::getCppuType( (sal_Int16 *) NULL );
    const uno::Type &rINT32    = ::getCppuType( (sal_Int32 *) NULL );

    sal_Int32 nProps = rProperyNames.getLength();
    uno::Sequence< uno::Any > aValues( nProps );
    uno::Any *pValue = aValues.getArray();

    if (nProps  &&  aValues.getLength() == nProps)
    {
        const SvtLinguOptions &rOpt = aOpt;

        OUString aTmp( lcl_LanguageToCfgLocaleStr( rOpt.nDefaultLanguage ) );
        *pValue++ = uno::makeAny( aTmp );                               //   0
        *pValue++ = uno::makeAny( rOpt.aActiveDics );                   //   1
        pValue++->setValue( &rOpt.bIsUseDictionaryList, rBOOL );        //   2
        pValue++->setValue( &rOpt.bIsIgnoreControlCharacters, rBOOL );  //   3
        aTmp = lcl_LanguageToCfgLocaleStr( rOpt.nDefaultLanguage_CJK );
        *pValue++ = uno::makeAny( aTmp );                               //   5
        aTmp = lcl_LanguageToCfgLocaleStr( rOpt.nDefaultLanguage_CTL );
        *pValue++ = uno::makeAny( aTmp );                               //   6

        pValue++->setValue( &rOpt.bIsSpellUpperCase, rBOOL );          //   7
        pValue++->setValue( &rOpt.bIsSpellWithDigits, rBOOL );         //   8
        pValue++->setValue( &rOpt.bIsSpellCapitalization, rBOOL );     //   9
        pValue++->setValue( &rOpt.bIsSpellAuto, rBOOL );               //  10
        pValue++->setValue( &rOpt.bIsSpellSpecial, rBOOL );            //  11
        pValue++->setValue( &rOpt.bIsSpellReverse, rBOOL );            //  14

        pValue++->setValue( &rOpt.nHyphMinLeading, rINT16 );           //  15
        pValue++->setValue( &rOpt.nHyphMinTrailing, rINT16 );          //  16
        pValue++->setValue( &rOpt.nHyphMinWordLength, rINT16 );        //  17
        pValue++->setValue( &rOpt.bIsHyphSpecial, rBOOL );             //  18
        pValue++->setValue( &rOpt.bIsHyphAuto, rBOOL );                //  19

        *pValue++ = uno::makeAny( rOpt.aActiveConvDics );               //   20

        pValue++->setValue( &rOpt.bIsIgnorePostPositionalWord, rBOOL ); //  21
        pValue++->setValue( &rOpt.bIsAutoCloseDialog, rBOOL );          //  22
        pValue++->setValue( &rOpt.bIsShowEntriesRecentlyUsedFirst, rBOOL ); //  23
        pValue++->setValue( &rOpt.bIsAutoReplaceUniqueEntries, rBOOL ); //  24

        pValue++->setValue( &rOpt.bIsDirectionToSimplified, rBOOL ); //  25
        pValue++->setValue( &rOpt.bIsUseCharacterVariants, rBOOL ); //  26
        pValue++->setValue( &rOpt.bIsTranslateCommonTerms, rBOOL ); //  27
        pValue++->setValue( &rOpt.bIsReverseMapping, rBOOL ); //  28

        pValue++->setValue( &rOpt.nDataFilesChangedCheckValue, rINT32 ); //  29
        pValue++->setValue( &rOpt.bIsGrammarAuto, rBOOL ); //  30
        pValue++->setValue( &rOpt.bIsGrammarInteractive, rBOOL ); // 31

        bRet |= PutProperties( rProperyNames, aValues );
    }

    if (bRet)
        ClearModified();

    return bRet;
}

sal_Bool SvtLinguConfigItem::IsReadOnly( const rtl::OUString &rPropertyName ) const
{
    osl::MutexGuard aGuard( GetOwnMutex() );

    sal_Bool bReadOnly = sal_False;
    sal_Int32 nHdl;
    if (GetHdlByName( nHdl, rPropertyName ))
        bReadOnly = IsReadOnly( nHdl );
    return bReadOnly;
}

sal_Bool SvtLinguConfigItem::IsReadOnly( sal_Int32 nPropertyHandle ) const
{
    osl::MutexGuard aGuard( GetOwnMutex() );

    sal_Bool bReadOnly = sal_False;

    const SvtLinguOptions &rOpt = const_cast< SvtLinguConfigItem * >(this)->aOpt;
    switch(nPropertyHandle)
    {
        case UPH_IS_USE_DICTIONARY_LIST         : bReadOnly = rOpt.bROIsUseDictionaryList      ; break;
        case UPH_IS_IGNORE_CONTROL_CHARACTERS   : bReadOnly = rOpt.bROIsIgnoreControlCharacters; break;
        case UPH_IS_HYPH_AUTO                   : bReadOnly = rOpt.bROIsHyphAuto               ; break;
        case UPH_IS_HYPH_SPECIAL                : bReadOnly = rOpt.bROIsHyphSpecial            ; break;
        case UPH_IS_SPELL_AUTO                  : bReadOnly = rOpt.bROIsSpellAuto              ; break;
        case UPH_IS_SPELL_SPECIAL               : bReadOnly = rOpt.bROIsSpellSpecial           ; break;
        case UPH_IS_WRAP_REVERSE                : bReadOnly = rOpt.bROIsSpellReverse           ; break;
        case UPH_DEFAULT_LANGUAGE               : bReadOnly = rOpt.bRODefaultLanguage          ; break;
        case UPH_IS_SPELL_CAPITALIZATION        : bReadOnly = rOpt.bROIsSpellCapitalization    ; break;
        case UPH_IS_SPELL_WITH_DIGITS           : bReadOnly = rOpt.bROIsSpellWithDigits        ; break;
        case UPH_IS_SPELL_UPPER_CASE            : bReadOnly = rOpt.bROIsSpellUpperCase         ; break;
        case UPH_HYPH_MIN_LEADING               : bReadOnly = rOpt.bROHyphMinLeading           ; break;
        case UPH_HYPH_MIN_TRAILING              : bReadOnly = rOpt.bROHyphMinTrailing          ; break;
        case UPH_HYPH_MIN_WORD_LENGTH           : bReadOnly = rOpt.bROHyphMinWordLength        ; break;
        case UPH_ACTIVE_DICTIONARIES            : bReadOnly = rOpt.bROActiveDics               ; break;
        case UPH_ACTIVE_CONVERSION_DICTIONARIES : bReadOnly = rOpt.bROActiveConvDics           ; break;
        case UPH_DEFAULT_LOCALE                 : bReadOnly = rOpt.bRODefaultLanguage          ; break;
        case UPH_DEFAULT_LOCALE_CJK             : bReadOnly = rOpt.bRODefaultLanguage_CJK      ; break;
        case UPH_DEFAULT_LOCALE_CTL             : bReadOnly = rOpt.bRODefaultLanguage_CTL      ; break;
        case UPH_IS_IGNORE_POST_POSITIONAL_WORD :       bReadOnly = rOpt.bROIsIgnorePostPositionalWord; break;
        case UPH_IS_AUTO_CLOSE_DIALOG :                 bReadOnly = rOpt.bROIsAutoCloseDialog; break;
        case UPH_IS_SHOW_ENTRIES_RECENTLY_USED_FIRST :  bReadOnly = rOpt.bROIsShowEntriesRecentlyUsedFirst; break;
        case UPH_IS_AUTO_REPLACE_UNIQUE_ENTRIES :       bReadOnly = rOpt.bROIsAutoReplaceUniqueEntries; break;
        case UPH_IS_DIRECTION_TO_SIMPLIFIED : bReadOnly = rOpt.bROIsDirectionToSimplified; break;
        case UPH_IS_USE_CHARACTER_VARIANTS : bReadOnly = rOpt.bROIsUseCharacterVariants; break;
        case UPH_IS_TRANSLATE_COMMON_TERMS : bReadOnly = rOpt.bROIsTranslateCommonTerms; break;
        case UPH_IS_REVERSE_MAPPING :        bReadOnly = rOpt.bROIsReverseMapping; break;
        case UPH_DATA_FILES_CHANGED_CHECK_VALUE :       bReadOnly = rOpt.bRODataFilesChangedCheckValue; break;
        case UPH_IS_GRAMMAR_AUTO:                       bReadOnly = rOpt.bROIsGrammarAuto; break;
        case UPH_IS_GRAMMAR_INTERACTIVE:                bReadOnly = rOpt.bROIsGrammarInteractive; break;
        default :
            DBG_ASSERT( 0, "unexpected property handle" );
    }
    return bReadOnly;
}

//////////////////////////////////////////////////////////////////////

static SvtLinguConfigItem *pCfgItem = 0;
static sal_Int32           nCfgItemRefCount = 0;

static const rtl::OUString aG_SupportedDictionaryFormats( A2OU("SupportedDictionaryFormats") );
static const rtl::OUString aG_Dictionaries( A2OU("Dictionaries") );
static const rtl::OUString aG_Locations( A2OU("Locations") );
static const rtl::OUString aG_Format( A2OU("Format") );
static const rtl::OUString aG_Locales( A2OU("Locales") );
static const rtl::OUString aG_DisabledDictionaries( A2OU("DisabledDictionaries") );
static const rtl::OUString aG_LastActiveDictionaries( A2OU("LastActiveDictionaries") );

SvtLinguConfig::SvtLinguConfig()
{
    // Global access, must be guarded (multithreading)
    osl::MutexGuard aGuard( GetOwnMutex() );
    ++nCfgItemRefCount;
}


SvtLinguConfig::~SvtLinguConfig()
{
    osl::MutexGuard aGuard( GetOwnMutex() );

    if (pCfgItem && pCfgItem->IsModified())
        pCfgItem->Commit();

    if (--nCfgItemRefCount <= 0)
    {
        if (pCfgItem)
            delete pCfgItem;
        pCfgItem = 0;
    }
}


SvtLinguConfigItem & SvtLinguConfig::GetConfigItem()
{
    // Global access, must be guarded (multithreading)
    osl::MutexGuard aGuard( GetOwnMutex() );
    if (!pCfgItem)
    {
        pCfgItem = new SvtLinguConfigItem;
        ItemHolder1::holdConfigItem(E_LINGUCFG);
    }
    return *pCfgItem;
}


uno::Sequence< OUString > SvtLinguConfig::GetNodeNames( const OUString &rNode )
{
    return GetConfigItem().GetNodeNames( rNode );
}


uno::Sequence< uno::Any > SvtLinguConfig::GetProperties( const uno::Sequence< OUString > &rNames )
{
    return GetConfigItem().GetProperties(rNames);
}


sal_Bool SvtLinguConfig::ReplaceSetProperties(
        const OUString &rNode, uno::Sequence< beans::PropertyValue > rValues )
{
    return GetConfigItem().ReplaceSetProperties( rNode, rValues );
}


uno::Any SvtLinguConfig::GetProperty( const OUString &rPropertyName ) const
{
    return GetConfigItem().GetProperty( rPropertyName );
}


uno::Any SvtLinguConfig::GetProperty( sal_Int32 nPropertyHandle ) const
{
    return GetConfigItem().GetProperty( nPropertyHandle );
}


sal_Bool SvtLinguConfig::SetProperty( const OUString &rPropertyName, const uno::Any &rValue )
{
    return GetConfigItem().SetProperty( rPropertyName, rValue );
}


sal_Bool SvtLinguConfig::SetProperty( sal_Int32 nPropertyHandle, const uno::Any &rValue )
{
    return GetConfigItem().SetProperty( nPropertyHandle, rValue );
}


sal_Bool SvtLinguConfig::GetOptions( SvtLinguOptions &rOptions ) const
{
    return GetConfigItem().GetOptions( rOptions );
}


sal_Bool SvtLinguConfig::SetOptions( const SvtLinguOptions &rOptions )
{
    return GetConfigItem().SetOptions( rOptions );
}


sal_Bool SvtLinguConfig::IsReadOnly( const rtl::OUString &rPropertyName ) const
{
    return GetConfigItem().IsReadOnly( rPropertyName );
}

sal_Bool SvtLinguConfig::IsReadOnly( sal_Int32 nPropertyHandle ) const
{
    return GetConfigItem().IsReadOnly( nPropertyHandle );
}

sal_Bool SvtLinguConfig::GetElementNamesFor(
     const rtl::OUString &rNodeName,
     uno::Sequence< rtl::OUString > &rElementNames ) const
{
    bool bSuccess = false;
    try
    {
        uno::Reference< container::XNameAccess > xNA( GetMainUpdateAccess(), uno::UNO_QUERY_THROW );
        xNA.set( xNA->getByName( A2OU("ServiceManager") ), uno::UNO_QUERY_THROW );
        xNA.set( xNA->getByName( rNodeName ), uno::UNO_QUERY_THROW );
        rElementNames = xNA->getElementNames();
        bSuccess = true;
    }
    catch (uno::Exception &)
    {
    }
    return bSuccess;
}

static uno::Reference< container::XNameAccess > GetOrCreateSetEntry_Impl(
    const uno::Reference< container::XNameAccess > &rxSetNameAccess,
    const rtl::OUString &rEntryName )
{
    uno::Reference< container::XNameAccess > xResult;
    try
    {
        if (!rxSetNameAccess->hasByName( rEntryName ))
        {
            uno::Reference< lang::XSingleServiceFactory > xFactory( rxSetNameAccess, uno::UNO_QUERY_THROW);
            uno::Reference< uno::XInterface > xNewEntry( xFactory->createInstance() );
            uno::Reference< container::XNameContainer > xNC( rxSetNameAccess, uno::UNO_QUERY_THROW );
            xNC->insertByName( rEntryName, makeAny( xNewEntry ) );
        }
        xResult.set( rxSetNameAccess->getByName( rEntryName ), uno::UNO_QUERY_THROW );
    }
    catch (uno::Exception &)
    {
    }
    return xResult;
}

sal_Bool SvtLinguConfig::GetSupportedDictionaryFormatsFor(
    const rtl::OUString &rSetName,
    const rtl::OUString &rSetEntry,
    uno::Sequence< rtl::OUString > &rFormatList ) const
{
    if (rSetName.getLength() == 0 || rSetEntry.getLength() == 0)
        return sal_False;
    bool bSuccess = false;
    try
    {
        uno::Reference< container::XNameAccess > xNA( GetMainUpdateAccess(), uno::UNO_QUERY_THROW );
        xNA.set( xNA->getByName( A2OU("ServiceManager") ), uno::UNO_QUERY_THROW );
        xNA.set( xNA->getByName( rSetName ), uno::UNO_QUERY_THROW );
        xNA.set( xNA->getByName( rSetEntry ), uno::UNO_QUERY_THROW );
        if (xNA->getByName( aG_SupportedDictionaryFormats ) >>= rFormatList)
            bSuccess = true;
        DBG_ASSERT( rFormatList.getLength(), "supported dictionary format list is empty" );
    }
    catch (uno::Exception &)
    {
    }
    return bSuccess;
}

void SvtLinguConfig::SetOrCreateSupportedDictionaryFormatsFor(
    const rtl::OUString &rSetName,
    const rtl::OUString &rSetEntry,
    const uno::Sequence< rtl::OUString > &rFormatList  ) const
{
    if (rSetName.getLength() == 0 || rSetEntry.getLength() == 0)
        return;
    try
    {
        DBG_ASSERT( rFormatList.getLength(), "applying empty format list. Really??" );

        uno::Reference< util::XChangesBatch > xUpdateAccess( GetMainUpdateAccess() );
        uno::Reference< container::XNameAccess > xNA( xUpdateAccess, uno::UNO_QUERY_THROW );
        xNA.set( xNA->getByName( A2OU("ServiceManager") ), uno::UNO_QUERY_THROW );
        xNA.set( xNA->getByName( rSetName ), uno::UNO_QUERY_THROW );
        xNA = GetOrCreateSetEntry_Impl( xNA, rSetEntry );

        uno::Reference< container::XNameReplace > xNR( xNA, uno::UNO_QUERY_THROW );
        xNR->replaceByName( aG_SupportedDictionaryFormats, uno::makeAny( rFormatList ) );

        xUpdateAccess->commitChanges();
    }
    catch (uno::Exception &)
    {
    }
}


static uno::WeakReference< util::XMacroExpander > aG_xMacroExpander;

static uno::Reference< util::XMacroExpander > lcl_GetMacroExpander()
{
    uno::Reference< util::XMacroExpander > xMacroExpander( aG_xMacroExpander );
    if ( !xMacroExpander.is() )
    {
        if ( !xMacroExpander.is() )
        {
            uno::Reference< uno::XComponentContext > xContext;
            uno::Reference< beans::XPropertySet > xProps( ::comphelper::getProcessServiceFactory(), uno::UNO_QUERY );
            xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ))) >>= xContext;
            if ( xContext.is() )
            {
                aG_xMacroExpander =  uno::Reference< com::sun::star::util::XMacroExpander >( xContext->getValueByName(
                                        OUString( RTL_CONSTASCII_USTRINGPARAM( "/singletons/com.sun.star.util.theMacroExpander"))),
                                        uno::UNO_QUERY );
                xMacroExpander = aG_xMacroExpander;
            }
        }
    }

    return xMacroExpander;
}


static bool lcl_GetFileUrlFromOrigin(
    OUString /*out*/ &rFileUrl,
    const OUString &rOrigin,
    uno::Reference< util::XMacroExpander > &rxMacroExpander )
{
    bool bSuccess = false;
    if (rOrigin.getLength() > 0 && rxMacroExpander.is())
    {
        rtl::OUString aURL( rOrigin );
        if (( aURL.compareToAscii( RTL_CONSTASCII_STRINGPARAM( EXPAND_PROTOCOL )) == 0 ) &&
            rxMacroExpander.is() )
        {
            // cut protocol
            OUString aMacro( aURL.copy( sizeof ( EXPAND_PROTOCOL ) -1 ) );
            // decode uric class chars
            aMacro = Uri::decode( aMacro, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8 );
            // expand macro string
            aURL = rxMacroExpander->expandMacros( aMacro );

            bool bIsFileUrl = aURL.compareToAscii( RTL_CONSTASCII_STRINGPARAM( FILE_PROTOCOL )) == 0;
            if (bIsFileUrl)
            {
                rFileUrl = aURL;
                bSuccess = true;
            }
            else
            {
                DBG_ASSERT( bIsFileUrl, "not a file URL");
            }
        }
        else
        {
            DBG_ASSERT( 0, "failed to get file URL" );
        }
    }
    return bSuccess;
}


sal_Bool SvtLinguConfig::GetDictionaryEntry(
    const rtl::OUString &rNodeName,
    SvtLinguConfigDictionaryEntry &rDicEntry ) const
{
    if (rNodeName.getLength() == 0)
        return sal_False;
    bool bSuccess = false;
    try
    {
        uno::Reference< container::XNameAccess > xNA( GetMainUpdateAccess(), uno::UNO_QUERY_THROW );
        xNA.set( xNA->getByName( A2OU("ServiceManager") ), uno::UNO_QUERY_THROW );
        xNA.set( xNA->getByName( aG_Dictionaries ), uno::UNO_QUERY_THROW );
        xNA.set( xNA->getByName( rNodeName ), uno::UNO_QUERY_THROW );

        // read group data...
        uno::Sequence< rtl::OUString >	aLocations;
        rtl::OUString					aFormatName;
		uno::Sequence< rtl::OUString >  aLocaleNames;
        bSuccess =  (xNA->getByName( aG_Locations ) >>= aLocations)  &&
                    (xNA->getByName( aG_Format )    >>= aFormatName) &&
                    (xNA->getByName( aG_Locales )   >>= aLocaleNames);
        DBG_ASSERT( aLocations.getLength(), "Dictionary locations not set" );
        DBG_ASSERT( aFormatName.getLength(), "Dictionary format name not set" );
        DBG_ASSERT( aLocaleNames.getLength(), "No locales set for the dictionary" );

        // if sucessful continue
        if (bSuccess)
        {
            // get file URL's for the locations
            uno::Reference< util::XMacroExpander > xMacroExpander( lcl_GetMacroExpander() );
            for (sal_Int32 i = 0;  i < aLocations.getLength();  ++i)
            {
                rtl::OUString &rLocation = aLocations[i];
                if (!lcl_GetFileUrlFromOrigin( rLocation, rLocation, xMacroExpander ))
                    bSuccess = false;
            }

            // if everything was fine return the result
            if (bSuccess)
            {
                rDicEntry.aLocations    = aLocations;
                rDicEntry.aFormatName   = aFormatName;
                rDicEntry.aLocaleNames  = aLocaleNames;
            }
        }
    }
    catch (uno::Exception &)
    {
    }
    return bSuccess;
}

void SvtLinguConfig::SetOrCreateDictionaryEntry(
    const rtl::OUString &rNodeName,
    const SvtLinguConfigDictionaryEntry &rDicEntry ) const
{
    if (rNodeName.getLength() == 0)
        return;
    try
    {
        uno::Reference< util::XChangesBatch > xUpdateAccess( GetMainUpdateAccess() );
        uno::Reference< container::XNameAccess > xNA( xUpdateAccess, uno::UNO_QUERY_THROW );
        xNA.set( xNA->getByName( A2OU("ServiceManager") ), uno::UNO_QUERY_THROW );
        xNA.set( xNA->getByName( aG_Dictionaries ), uno::UNO_QUERY_THROW );
        xNA = GetOrCreateSetEntry_Impl( xNA, rNodeName );

        DBG_ASSERT( rDicEntry.aLocations.getLength(), "Applying empty dictionary locations. Really correct??" );
        DBG_ASSERT( rDicEntry.aFormatName.getLength(), "Applying empty dictionary format name. Really correct??" );
        DBG_ASSERT( rDicEntry.aLocaleNames.getLength(), "Applying empty list of locales for the dictionary. Really correct??" );

        uno::Reference< container::XNameReplace > xNR( xNA, uno::UNO_QUERY_THROW );
        xNR->replaceByName( aG_Locations, uno::makeAny( rDicEntry.aLocations ) );
        xNR->replaceByName( aG_Format,    uno::makeAny( rDicEntry.aFormatName ) );
        xNR->replaceByName( aG_Locales,   uno::makeAny( rDicEntry.aLocaleNames ) );

        xUpdateAccess->commitChanges();
    }
    catch (uno::Exception &)
    {
    }
}

uno::Sequence< rtl::OUString > SvtLinguConfig::GetDisabledDictionaries() const
{
    uno::Sequence< rtl::OUString > aResult;
    try
    {
        uno::Reference< container::XNameAccess > xNA( GetMainUpdateAccess(), uno::UNO_QUERY_THROW );
        xNA.set( xNA->getByName( A2OU("ServiceManager") ), uno::UNO_QUERY_THROW );
        xNA->getByName( aG_DisabledDictionaries ) >>= aResult;
    }
    catch (uno::Exception &)
    {
    }
    return aResult;
}

void SvtLinguConfig::SetDisabledDictionaries(
    const uno::Sequence< rtl::OUString > &rDictionaries ) const
{
    try
    {
        uno::Reference< util::XChangesBatch > xUpdateAccess( GetMainUpdateAccess() );
        uno::Reference< container::XNameAccess > xNA( xUpdateAccess, uno::UNO_QUERY_THROW );
        xNA.set( xNA->getByName( A2OU("ServiceManager") ), uno::UNO_QUERY_THROW );
        if (xNA->hasByName( aG_DisabledDictionaries ))
        {
            uno::Reference< container::XNameReplace > xNR( xNA, uno::UNO_QUERY_THROW );
            xNR->replaceByName( aG_DisabledDictionaries, makeAny( rDictionaries ) );
        }
        else
        {
            uno::Reference< container::XNameContainer > xNC( xNA, uno::UNO_QUERY_THROW );
            xNC->insertByName( aG_DisabledDictionaries, makeAny( rDictionaries ) );
        }

        xUpdateAccess->commitChanges();
    }
    catch (uno::Exception &)
    {
    }
}

std::vector< SvtLinguConfigDictionaryEntry > SvtLinguConfig::GetActiveDictionariesByFormat(
    const rtl::OUString &rFormatName )
{
    std::vector< SvtLinguConfigDictionaryEntry > aRes;
    if (rFormatName.getLength() == 0)
        return aRes;

    try
    {
        uno::Sequence< rtl::OUString > aElementNames;
        GetElementNamesFor( aG_Dictionaries, aElementNames );
        sal_Int32 nLen = aElementNames.getLength();
        const rtl::OUString *pElementNames = aElementNames.getConstArray();

        SvtLinguConfigDictionaryEntry aDicEntry;
        for (sal_Int32 i = 0;  i < nLen;  ++i)
        {
            // does dictionary match the format we are looking for?
            if (GetDictionaryEntry( pElementNames[i], aDicEntry ) &&
                aDicEntry.aFormatName == rFormatName)
            {
                // check if it is active or not
                bool bDicIsActive = true;
                const uno::Sequence< rtl::OUString > aDisabledDics( GetDisabledDictionaries() );
                for (sal_Int32 k = 0;  bDicIsActive && k < aDisabledDics.getLength();  ++k)
                {
                    if (aDisabledDics[k] == pElementNames[i])
                        bDicIsActive = false;
                }

                if (bDicIsActive)
                {
                    DBG_ASSERT( aDicEntry.aFormatName.getLength(),
                            "FormatName not set" );
                    DBG_ASSERT( aDicEntry.aLocations.getLength(),
                            "Locations not set" );
                    DBG_ASSERT( aDicEntry.aLocaleNames.getLength(),
                            "Locales not set" );
                    aRes.push_back( aDicEntry );
                }
            }
        }
    }
    catch (uno::Exception &)
    {
    }

    return aRes;
}


uno::Reference< util::XChangesBatch > SvtLinguConfig::GetMainUpdateAccess() const
{
    if (!m_xMainUpdateAccess.is())
    {
        try
        {
            // get configuration provider
            uno::Reference< lang::XMultiServiceFactory > xConfigurationProvider;
            uno::Reference< lang::XMultiServiceFactory > xMgr = comphelper::getProcessServiceFactory();
            if (xMgr.is())
            {
                xConfigurationProvider = uno::Reference< lang::XMultiServiceFactory > (
                        xMgr->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM(
                            "com.sun.star.configuration.ConfigurationProvider" ) ) ),
                        uno::UNO_QUERY_THROW ) ;
            }

            // get configuration update access
            beans::PropertyValue aValue;
            aValue.Name  = A2OU( "nodepath" );
            aValue.Value = uno::makeAny( A2OU("org.openoffice.Office.Linguistic") );
            uno::Sequence< uno::Any > aProps(1);
            aProps[0] <<= aValue;
            m_xMainUpdateAccess = uno::Reference< util::XChangesBatch >(
                    xConfigurationProvider->createInstanceWithArguments(
                        A2OU( "com.sun.star.configuration.ConfigurationUpdateAccess" ), aProps ),
                        uno::UNO_QUERY_THROW );
        }
        catch (uno::Exception &)
        {
        }
    }

    return m_xMainUpdateAccess;
}


rtl::OUString SvtLinguConfig::GetVendorImageUrl_Impl(
    const rtl::OUString &rServiceImplName,
    const rtl::OUString &rImageName ) const
{
    rtl::OUString aRes;
    try
    {
        uno::Reference< container::XNameAccess > xImagesNA( GetMainUpdateAccess(), uno::UNO_QUERY_THROW );
        xImagesNA.set( xImagesNA->getByName( A2OU("Images") ), uno::UNO_QUERY_THROW );

        uno::Reference< container::XNameAccess > xNA( xImagesNA->getByName( A2OU("ServiceNameEntries") ), uno::UNO_QUERY_THROW );
        xNA.set( xNA->getByName( rServiceImplName ), uno::UNO_QUERY_THROW );
        uno::Any aAny( xNA->getByName( A2OU("VendorImagesNode") ) );
        rtl::OUString aVendorImagesNode;
        if (aAny >>= aVendorImagesNode)
        {
            xNA = xImagesNA;
            xNA.set( xNA->getByName( A2OU("VendorImages") ), uno::UNO_QUERY_THROW );
            xNA.set( xNA->getByName( aVendorImagesNode ), uno::UNO_QUERY_THROW );
            aAny = xNA->getByName( rImageName );
            rtl::OUString aTmp;
            if (aAny >>= aTmp)
            {
                uno::Reference< util::XMacroExpander > xMacroExpander( lcl_GetMacroExpander() );
                if (lcl_GetFileUrlFromOrigin( aTmp, aTmp, xMacroExpander ))
                    aRes = aTmp;
            }
        }
    }
    catch (uno::Exception &)
    {
        DBG_ASSERT( 0, "exception caught. GetVendorImageUrl_Impl failed" );
    }
    return aRes;
}


rtl::OUString SvtLinguConfig::GetSpellAndGrammarDialogImage(
    const rtl::OUString &rServiceImplName,
    bool bHighContrast ) const
{
    rtl::OUString   aRes;
    if (rServiceImplName.getLength() > 0)
    {
        rtl::OUString aImageName( A2OU( bHighContrast ? "SpellAndGrammarDialogImage_HC" : "SpellAndGrammarDialogImage" ));
        rtl::OUString aPath( GetVendorImageUrl_Impl( rServiceImplName, aImageName ) );
        aRes = aPath;
    }
    return aRes;
}


rtl::OUString SvtLinguConfig::GetSpellAndGrammarContextSuggestionImage(
    const rtl::OUString &rServiceImplName,
    bool bHighContrast ) const
{
    rtl::OUString   aRes;
    if (rServiceImplName.getLength() > 0)
    {
        rtl::OUString aImageName( A2OU( bHighContrast ? "SpellAndGrammarContextMenuSuggestionImage_HC" : "SpellAndGrammarContextMenuSuggestionImage" ));
        rtl::OUString aPath( GetVendorImageUrl_Impl( rServiceImplName, aImageName ) );
        aRes = aPath;
    }
    return aRes;
}


rtl::OUString SvtLinguConfig::GetSpellAndGrammarContextDictionaryImage(
    const rtl::OUString &rServiceImplName,
    bool bHighContrast ) const
{
    rtl::OUString   aRes;
    if (rServiceImplName.getLength() > 0)
    {
        rtl::OUString aImageName( A2OU( bHighContrast ? "SpellAndGrammarContextMenuDictionaryImage_HC" : "SpellAndGrammarContextMenuDictionaryImage" ));
        rtl::OUString aPath( GetVendorImageUrl_Impl( rServiceImplName, aImageName ) );
        aRes = aPath;
    }
    return aRes;
}


::rtl::OUString SvtLinguConfig::GetThesaurusDialogImage( 
    const ::rtl::OUString &rServiceImplName, 
    bool bHighContrast ) const
{
    rtl::OUString   aRes;
    if (rServiceImplName.getLength() > 0)
    {
        rtl::OUString aImageName( A2OU( bHighContrast ? "ThesaurusDialogImage_HC" : "ThesaurusDialogImage" ));
        rtl::OUString aPath( GetVendorImageUrl_Impl( rServiceImplName, aImageName ) );
        aRes = aPath;
    }
    return aRes;
}

    
::rtl::OUString SvtLinguConfig::GetSynonymsContextImage( 
    const ::rtl::OUString &rServiceImplName, 
    bool bHighContrast ) const
{
    rtl::OUString   aRes;
    if (rServiceImplName.getLength() > 0)
    {
        rtl::OUString aImageName( A2OU( bHighContrast ? "SynonymsContextMenuImage_HC" : "SynonymsContextMenuImage" ));
        rtl::OUString aPath( GetVendorImageUrl_Impl( rServiceImplName, aImageName ) );
        aRes = aPath;
    }
    return aRes;
}
    

bool SvtLinguConfig::HasVendorImages( const char *pImageName ) const
{
    bool bRes = false;
    if (pImageName)
    {
        try
        {
            uno::Reference< container::XNameAccess > xNA( GetMainUpdateAccess(), uno::UNO_QUERY_THROW );
            xNA.set( xNA->getByName( A2OU("Images") ), uno::UNO_QUERY_THROW );
            xNA.set( xNA->getByName( A2OU("VendorImages") ), uno::UNO_QUERY_THROW );

            uno::Sequence< rtl::OUString > aElementNames( xNA->getElementNames() );
            sal_Int32 nVendors = aElementNames.getLength();
            const rtl::OUString *pVendor = aElementNames.getConstArray();
            for (sal_Int32 i = 0;  i < nVendors;  ++i)
            {
                uno::Reference< container::XNameAccess > xNA2( xNA->getByName( pVendor[i] ), uno::UNO_QUERY_THROW  );
                uno::Sequence< rtl::OUString > aPropNames( xNA2->getElementNames() );
                sal_Int32 nProps = aPropNames.getLength();
                const rtl::OUString *pPropNames = aPropNames.getConstArray();
                for (sal_Int32 k = 0;  k < nProps;  ++k)
                {
                    // for a quicker check we ignore the HC image names here
                    const OUString &rName = pPropNames[k];
                    if (rName.equalsAscii( pImageName ))
                    {
                        bRes = true;
                        break;
                    }    
                }    
            }    
        }
        catch (uno::Exception &)
        {
            DBG_ASSERT( 0, "exception caught. HasVendorImages failed" );
        }
    }
    return bRes;
}
    

bool SvtLinguConfig::HasGrammarChecker() const
{
	bool bRes = false;

	try
	{
        uno::Reference< container::XNameAccess > xNA( GetMainUpdateAccess(), uno::UNO_QUERY_THROW );
        xNA.set( xNA->getByName( A2OU("ServiceManager") ), uno::UNO_QUERY_THROW );
		xNA.set( xNA->getByName( A2OU("GrammarCheckerList") ), uno::UNO_QUERY_THROW );

		uno::Sequence< rtl::OUString > aElementNames( xNA->getElementNames() );
		bRes = aElementNames.getLength() > 0;
	}
	catch (uno::Exception &)
	{
	}

    return bRes;
}

//////////////////////////////////////////////////////////////////////

