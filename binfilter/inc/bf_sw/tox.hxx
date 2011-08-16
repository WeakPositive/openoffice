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
#ifndef _TOX_HXX
#define _TOX_HXX

#include <bf_svtools/bf_solar.h>



#ifndef _SFXPOOLITEM_HXX //autogen
#include <bf_svtools/poolitem.hxx>
#endif

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _TOXE_HXX
#include <toxe.hxx>
#endif
#ifndef _CALBCK_HXX
#include <calbck.hxx>
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif
namespace binfilter {

class SwTOXType;
class SwTOXMark;
class SwTxtTOXMark;
class SwDoc;

SV_DECL_PTRARR(SwTOXMarks, SwTOXMark*, 0, 10)

/*--------------------------------------------------------------------
     Description:  Entry of content index, alphabetical index or user defined index
 --------------------------------------------------------------------*/

#define IVER_TOXMARK_STRPOOL ((USHORT)1)
#define IVER_TOXMARK_NEWTOX ((USHORT)2)

class SwTOXMark : public SfxPoolItem, public SwClient
{
	friend void _InitCore();
	friend class SwTxtTOXMark;

	String aAltText; 	// Der Text des Eintrages ist unterschiedlich
	String aPrimaryKey, aSecondaryKey;

    // three more strings for phonetic sorting
    String aTextReading;
    String aPrimaryKeyReading;
    String aSecondaryKeyReading;

	SwTxtTOXMark* pTxtAttr;

	USHORT 	nLevel;
	BOOL	bAutoGenerated : 1;		// generated using a concordance file
	BOOL	bMainEntry : 1;			// main entry emphasized by character style


    SwTOXMark();                    // to create the dflt. atr. in _InitCore

public:
    TYPEINFO();   // rtti

	SwTOXMark( const SwTOXType* pTyp );
	SwTOXMark( const SwTOXMark& rCopy );
	virtual ~SwTOXMark();

    // "pure virtual methods" of SfxPoolItem
	virtual int 			operator==( const SfxPoolItem& ) const;
	virtual SfxPoolItem*	Clone( SfxItemPool* pPool = 0 ) const;
	virtual SfxPoolItem*	Create(SvStream &, USHORT nVer) const;
	virtual SvStream&		Store(SvStream &, USHORT nIVer) const;
	virtual USHORT			GetVersion( USHORT nFFVer ) const;

	SwTOXMark&				operator=( const SwTOXMark& rCopy );

	inline BOOL				IsAlternativeText() const;
	inline const String&	GetAlternativeText() const;

	inline void				SetAlternativeText( const String& rAlt );

    // content or user defined index
    inline void             SetLevel(USHORT nLevel);
	inline USHORT			GetLevel() const;

    // for alphabetical index only
	inline void				SetPrimaryKey(const String& rStr );
	inline void				SetSecondaryKey(const String& rStr);
    inline void             SetTextReading(const String& rStr);
    inline void             SetPrimaryKeyReading(const String& rStr );
    inline void             SetSecondaryKeyReading(const String& rStr);

    inline const String&    GetPrimaryKey() const;
	inline const String&	GetSecondaryKey() const;
    inline const String&    GetTextReading() const;
    inline const String&    GetPrimaryKeyReading() const;
    inline const String&    GetSecondaryKeyReading() const;

	BOOL					IsAutoGenerated() const {return bAutoGenerated;}
	void					SetAutoGenerated(BOOL bSet) {bAutoGenerated = bSet;}

	BOOL					IsMainEntry() const {return bMainEntry;}
	void					SetMainEntry(BOOL bSet) { bMainEntry = bSet;}

	inline const SwTOXType*    GetTOXType() const;

	const SwTxtTOXMark* GetTxtTOXMark() const	{ return pTxtAttr; }
		  SwTxtTOXMark* GetTxtTOXMark() 		{ return pTxtAttr; }
};

/*--------------------------------------------------------------------
     Description:  index types
 --------------------------------------------------------------------*/

class SwTOXType : public SwModify
{
public:
	SwTOXType(TOXTypes eTyp, const String& aName);
	SwTOXType(const SwTOXType& rCopy);

	inline	const String&	GetTypeName() const;
	void					SetTypeName(const String& rName);
	inline TOXTypes			GetType() const;

private:
	String			aName;
	TOXTypes		eType;
};

/*--------------------------------------------------------------------
     Description:  Structure of the index lines
 --------------------------------------------------------------------*/

#define FORM_TITLE 				0
#define FORM_ALPHA_DELIMITTER 	1
#define FORM_PRIMARY_KEY		2
#define FORM_SECONDARY_KEY		3
#define FORM_ENTRY				4

/*
 Pattern structure

 <E#> - entry number  					<E# CharStyleName,PoolId>
 <ET> - entry text      				<ET CharStyleName,PoolId>
 <E>  - entry text and number           <E CharStyleName,PoolId>
 <T>  - tab stop                        <T,,Position,Adjust>
 <C>  - chapter info n = {0, 1, 2, 3, 4 } values of SwChapterFormat <C CharStyleName,PoolId>
 <TX> - text token						<X CharStyleName,PoolId, TOX_STYLE_DELIMITERTextContentTOX_STYLE_DELIMITER>
 <#>  - Page number                     <# CharStyleName,PoolId>
 <LS> - Link start                      <LS>
 <LE> - Link end                        <LE>
 <A00> - Authority entry field			<A02 CharStyleName, PoolId>
 */

// These enum values are stored and must not be changed!
enum FormTokenType
{
	TOKEN_ENTRY_NO,
	TOKEN_ENTRY_TEXT,
	TOKEN_ENTRY,
	TOKEN_TAB_STOP,
	TOKEN_TEXT,
	TOKEN_PAGE_NUMS,
	TOKEN_CHAPTER_INFO,
	TOKEN_LINK_START,
	TOKEN_LINK_END,
	TOKEN_AUTHORITY,
	TOKEN_END
};

struct SwFormToken
{
	String			sText;
	String			sCharStyleName;
	SwTwips			nTabStopPosition;
	FormTokenType 	eTokenType;
	USHORT			nPoolId;
	USHORT			eTabAlign;
	USHORT			nChapterFormat;		//SwChapterFormat;
	USHORT			nAuthorityField;	//enum ToxAuthorityField
	sal_Unicode 	cTabFillChar;

	SwFormToken(FormTokenType eType ) :
        nTabStopPosition(0),
		eTokenType(eType),
		nPoolId(USHRT_MAX),
		eTabAlign(0 /*SVX_TAB_ADJUST_LEFT*/),
		nChapterFormat(0 /*CF_NUMBER*/),
		nAuthorityField(0 /*AUTH_FIELD_IDENTIFIER*/),
		cTabFillChar(' ') {}

	String GetString() const;
};

class SwFormTokenEnumerator
{
	String sPattern;
	xub_StrLen nCurPatternPos;
	xub_StrLen nCurPatternLen;

	FormTokenType _SearchNextToken( xub_StrLen nStt, xub_StrLen& rEnd,
									xub_StrLen* pTokenLen = 0 ) const;
	SwFormToken BuildToken( FormTokenType, xub_StrLen ) const;

public:
	SwFormTokenEnumerator( const String& rPattern );
	BOOL			HasNextToken() const
		{ return nCurPatternPos + nCurPatternLen < sPattern.Len(); }

	SwFormToken     GetNextToken();

	const String&	GetPattern() const		{ return sPattern; }
};

class SwForm
{
	String	aPattern[ AUTH_TYPE_END + 1 ];
	String	aTemplate[ AUTH_TYPE_END + 1 ];
	USHORT 	nType, nFormMaxLevel;
	//USHORT	nFirstTabPos; -> Value in tab token
//	BOOL 	bHasFirstTabPos : 1;
	BOOL 	bGenerateTabPos : 1;
	BOOL 	bIsRelTabPos : 1;
	BOOL	bCommaSeparated : 1;

public:
	SwForm( USHORT nType = TOX_CONTENT );
	SwForm( const SwForm& rForm );

	SwForm&	operator=( const SwForm& rForm );

	inline void				SetTemplate(USHORT nLevel, const String& rName);
	inline const String&	GetTemplate(USHORT nLevel) const;

	inline void				SetPattern(USHORT nLevel, const String& rName);
	inline const String&	GetPattern(USHORT nLevel) const;

	//convert pattern string from old to new format or vice versa
	static String			ConvertPatternTo51(const String& rSource);
	static String			ConvertPatternFrom51(const String& rSource, TOXTypes eType);

	// fill tab stop positions from template to pattern
	void					AdjustTabStops(SwDoc& rDoc);

	SwFormTokenEnumerator	CreateTokenEnumerator(USHORT nLevel) const
		{return SwFormTokenEnumerator(GetPattern(nLevel));}

	inline USHORT	GetTOXType() const;
	inline USHORT	GetFormMax() const;

	USHORT GetFirstTabPos() const  ;	//{ return nFirstTabPos; }
    void SetFirstTabPos( USHORT n );    //{ nFirstTabPos = n; }

	BOOL IsFirstTabPosFlag() const 	;	//{ return bHasFirstTabPos; }

	BOOL IsGenerateTabPos() const 		{ return bGenerateTabPos; }
	void SetGenerateTabPos( BOOL b ) 	{ bGenerateTabPos = b; }

	BOOL IsRelTabPos() const 	{ 	return bIsRelTabPos; }
	void SetRelTabPos( BOOL b ) { 	bIsRelTabPos = b;		}

	BOOL IsCommaSeparated() const 		{ return bCommaSeparated;}
	void SetCommaSeparated( BOOL b)		{ bCommaSeparated = b;}

	static USHORT GetFormMaxLevel( USHORT nType );

	static const sal_Char*	aFormEntry;				// <E>
    static BYTE nFormEntryLen;                      // 3 characters
	static const sal_Char*	aFormTab;				// <T>
    static BYTE nFormTabLen;                        // 3 characters
	static const sal_Char*	aFormPageNums;			// <P>
    static BYTE nFormPageNumsLen;                   // 3 characters
	static const sal_Char* aFormLinkStt;			// <LS>
    static BYTE nFormLinkSttLen;                    // 4 characters
	static const sal_Char* aFormLinkEnd;			// <LE>
    static BYTE nFormLinkEndLen;                    // 4 characters
	static const sal_Char*	aFormEntryNum;			// <E#>
    static BYTE nFormEntryNumLen;                   // 4 characters
	static const sal_Char*	aFormEntryTxt;			// <ET>
    static BYTE nFormEntryTxtLen;                   // 4 characters
	static const sal_Char*	aFormChapterMark;		// <C>
    static BYTE nFormChapterMarkLen;                // 3 characters
	static const sal_Char*	aFormText;				// <TX>
    static BYTE nFormTextLen;                       // 4 characters
	static const sal_Char*	aFormAuth;				// <Axx> xx - decimal enum value
    static BYTE nFormAuthLen;                       // 3 characters
};

/*--------------------------------------------------------------------
     Description: Content to create indexes of
 --------------------------------------------------------------------*/

enum SwTOXElement
{
	TOX_MARK 			= 1,
	TOX_OUTLINELEVEL	= 2,
	TOX_TEMPLATE		= 4,
	TOX_OLE				= 8,
	TOX_TABLE			= 16,
	TOX_GRAPHIC			= 32,
	TOX_FRAME			= 64,
	TOX_SEQUENCE		= 128
};

enum SwTOIOptions
{
	TOI_SAME_ENTRY 			= 1,
	TOI_FF		  			= 2,
	TOI_CASE_SENSITIVE		= 4,
	TOI_KEY_AS_ENTRY		= 8,
	TOI_ALPHA_DELIMITTER	= 16,
	TOI_DASH				= 32,
	TOI_INITIAL_CAPS		= 64
};

//which part of the caption is to be displayed
enum SwCaptionDisplay
{
	CAPTION_COMPLETE,
	CAPTION_NUMBER,
	CAPTION_TEXT
};

enum SwTOOElements
{
	TOO_MATH		= 0x01,
	TOO_CHART		= 0x02,
	TOO_CALC		= 0x08,
	TOO_DRAW_IMPRESS= 0x10,
//	TOO_IMPRESS		= 0x20,

	TOO_OTHER		= 0x80
};

#define TOX_STYLE_DELIMITER ((sal_Unicode)0x01)		//JP 19.07.00: use a control char

/*--------------------------------------------------------------------
     Description:  Class for all indexes
 --------------------------------------------------------------------*/

class SwTOXBase : public SwClient
{
	// not implemented
	SwTOXBase& 			operator=(const SwTOXBase& rSource);

    SwForm      aForm;              // description of the lines
	String		aName; 				// unique name
    String      aTitle;             // title

	String 		sMainEntryCharStyle; // name of the character style applied to main index entries

	String		aStyleNames[MAXLEVEL]; // (additional) style names TOX_CONTENT, TOX_USER
	String 		sSequenceName;		// FieldTypeName of a caption sequence

    LanguageType    eLanguage;
    String          sSortAlgorithm;

    union {
        USHORT      nLevel;             // consider outline levels
        USHORT      nOptions;           // options of alphabetical index
	} aData;

    USHORT      nCreateType;        // sources to create the index from
	USHORT		nOLEOptions;		// OLE sources
	SwCaptionDisplay eCaptionDisplay;	//
	BOOL 		bProtected : 1;			// index protected ?
	BOOL		bFromChapter : 1; 		// create from chapter or document
	BOOL 		bFromObjectNames : 1; 	// create a table or object index
									// from the names rather than the caption
	BOOL		bLevelFromChapter : 1; // User index: get the level from the source chapter
public:
	SwTOXBase( const SwTOXType* pTyp, const SwForm& rForm,
			   USHORT nCreaType, const String& rTitle );
	SwTOXBase( const SwTOXBase& rCopy, SwDoc* pDoc = 0 );
	virtual ~SwTOXBase();


	// a kind of CopyCtor - check if the TOXBase is at TOXType of the doc.
	// If not, so create it an copy all other used things. The return is this
	SwTOXBase& CopyTOXBase( SwDoc*, const SwTOXBase& );

	const SwTOXType*	GetTOXType() const;	//

    USHORT              GetCreateType() const;      // creation types

	const String&		GetTOXName() const {return aName;}
	void				SetTOXName(const String& rSet) {aName = rSet;}

    const String&       GetTitle() const;           // Title
    const String&       GetTypeName() const;        // Name
    const SwForm&       GetTOXForm() const;         // description of the lines

	void 				SetCreate(USHORT);
	void				SetTitle(const String& rTitle);
	void				SetTOXForm(const SwForm& rForm);

	TOXTypes			GetType() const;

	const String&		GetMainEntryCharStyle() const {return sMainEntryCharStyle;}
	void				SetMainEntryCharStyle(const String& rSet)  {sMainEntryCharStyle = rSet;}

    // content index only
    inline void             SetLevel(USHORT);                   // consider outline level
	inline USHORT	  		GetLevel() const;

    // alphabetical index only
    inline USHORT           GetOptions() const;                 // alphabetical index options
	inline void   			SetOptions(USHORT nOpt);

	// index of objects
	USHORT 		GetOLEOptions() const {return nOLEOptions;}
	void   		SetOLEOptions(USHORT nOpt) {nOLEOptions = nOpt;}

	// index of objects

    // user defined index only
	inline void				SetTemplateName(const String& rName); // Absatzlayout beachten
	inline String			GetTemplateName() const;

	const String&			GetStyleNames(USHORT nLevel) const
								{
								DBG_ASSERT( nLevel < MAXLEVEL, "Which level?");
								return aStyleNames[nLevel];
								}
	void					SetStyleNames(const String& rSet, USHORT nLevel)
								{
								DBG_ASSERT( nLevel < MAXLEVEL, "Which level?");
								aStyleNames[nLevel] = rSet;
								}

	BOOL					IsFromChapter() const { return bFromChapter;}
	void					SetFromChapter(BOOL bSet) { bFromChapter = bSet;}

	BOOL					IsFromObjectNames() const {return bFromObjectNames;}
	void					SetFromObjectNames(BOOL bSet) {bFromObjectNames = bSet;}

	BOOL					IsLevelFromChapter() const {return bLevelFromChapter;}
	void					SetLevelFromChapter(BOOL bSet) {bLevelFromChapter = bSet;}

	BOOL					IsProtected() const { return bProtected; }
	void					SetProtected(BOOL bSet) { bProtected = bSet; }

	const String&			GetSequenceName() const {return sSequenceName;}
	void					SetSequenceName(const String& rSet) {sSequenceName = rSet;}

	SwCaptionDisplay		GetCaptionDisplay() const { return eCaptionDisplay;}
	void					SetCaptionDisplay(SwCaptionDisplay eSet) {eCaptionDisplay = eSet;}

	static const String&	GetTOXName(TOXTypes eType);           // toxmgr.cxx

    LanguageType    GetLanguage() const {return eLanguage;}
    void            SetLanguage(LanguageType nLang)  {eLanguage = nLang;}

    const String&   GetSortAlgorithm()const {return sSortAlgorithm;}
    void            SetSortAlgorithm(const String& rSet) {sSortAlgorithm = rSet;}

};


/*--------------------------------------------------------------------
     Description:  Inlines
 --------------------------------------------------------------------*/

//
//SwTOXMark
//
inline const String& SwTOXMark::GetAlternativeText() const
	{	return aAltText;	}

inline const SwTOXType* SwTOXMark::GetTOXType() const
	{ return (SwTOXType*)GetRegisteredIn(); }

inline BOOL SwTOXMark::IsAlternativeText() const
	{ return aAltText.Len() > 0; }

inline void SwTOXMark::SetAlternativeText(const String& rAlt)
{
	aAltText = rAlt;
}

inline void SwTOXMark::SetLevel( USHORT nLvl )
{
	ASSERT( !GetTOXType() || GetTOXType()->GetType() != TOX_INDEX, "Falscher Feldtyp");
	nLevel = nLvl;
}

inline void SwTOXMark::SetPrimaryKey( const String& rKey )
{
	ASSERT(GetTOXType()->GetType() == TOX_INDEX, "Falscher Feldtyp");
	aPrimaryKey = rKey;
}

inline void SwTOXMark::SetSecondaryKey( const String& rKey )
{
	ASSERT(GetTOXType()->GetType() == TOX_INDEX, "Falscher Feldtyp");
	aSecondaryKey = rKey;
}

inline void SwTOXMark::SetTextReading( const String& rTxt )
{
	ASSERT(GetTOXType()->GetType() == TOX_INDEX, "Falscher Feldtyp");
    aTextReading = rTxt;
}

inline void SwTOXMark::SetPrimaryKeyReading( const String& rKey )
{
	ASSERT(GetTOXType()->GetType() == TOX_INDEX, "Falscher Feldtyp");
    aPrimaryKeyReading = rKey;
}

inline void SwTOXMark::SetSecondaryKeyReading( const String& rKey )
{
	ASSERT(GetTOXType()->GetType() == TOX_INDEX, "Falscher Feldtyp");
    aSecondaryKeyReading = rKey;
}

inline USHORT SwTOXMark::GetLevel() const
{
	ASSERT( !GetTOXType() || GetTOXType()->GetType() != TOX_INDEX, "Falscher Feldtyp");
	return nLevel;
}

inline const String& SwTOXMark::GetPrimaryKey() const
{
	ASSERT(GetTOXType()->GetType() == TOX_INDEX, "Falscher Feldtyp");
	return aPrimaryKey;
}

inline const String& SwTOXMark::GetSecondaryKey() const
{
	ASSERT(GetTOXType()->GetType() == TOX_INDEX, "Falscher Feldtyp");
	return aSecondaryKey;
}

inline const String& SwTOXMark::GetTextReading() const
{
	ASSERT(GetTOXType()->GetType() == TOX_INDEX, "Falscher Feldtyp");
    return aTextReading;
}

inline const String& SwTOXMark::GetPrimaryKeyReading() const
{
	ASSERT(GetTOXType()->GetType() == TOX_INDEX, "Falscher Feldtyp");
    return aPrimaryKeyReading;
}

inline const String& SwTOXMark::GetSecondaryKeyReading() const
{
	ASSERT(GetTOXType()->GetType() == TOX_INDEX, "Falscher Feldtyp");
    return aSecondaryKeyReading;
}

//
//SwForm
//
inline void SwForm::SetTemplate(USHORT nLevel, const String& rName)
{
	ASSERT(nLevel < GetFormMax(), "Index >= FORM_MAX");
	aTemplate[nLevel] = rName;
}

inline void SwForm::SetPattern(USHORT nLevel, const String& rName)
{
	ASSERT(nLevel < GetFormMax(), "Index >= FORM_MAX");
	aPattern[nLevel] = rName;
}

inline const String& SwForm::GetPattern(USHORT nLevel) const
{
	ASSERT(nLevel < GetFormMax(), "Index >= FORM_MAX");
	return aPattern[nLevel];
}

inline const String& SwForm::GetTemplate(USHORT nLevel) const
{
	ASSERT(nLevel < GetFormMax(), "Index >= FORM_MAX");
	return aTemplate[nLevel];
}

inline USHORT SwForm::GetTOXType() const
{
	return nType;
}

inline USHORT SwForm::GetFormMax() const
{
	return nFormMaxLevel;
}


//
//SwTOXType
//
inline const String& SwTOXType::GetTypeName() const
	{	return aName;	}

inline void SwTOXType::SetTypeName(const String& rName)
	{	aName = rName;	}

inline TOXTypes SwTOXType::GetType() const
	{	return eType;	}

//
// SwTOXBase
//
inline const SwTOXType* SwTOXBase::GetTOXType() const
	{ return (SwTOXType*)GetRegisteredIn(); }

inline USHORT SwTOXBase::GetCreateType() const
	{ return nCreateType; }

inline const String& SwTOXBase::GetTitle() const
	{ return aTitle; }

inline const String& SwTOXBase::GetTypeName() const
	{ return GetTOXType()->GetTypeName();  }

inline const SwForm& SwTOXBase::GetTOXForm() const
	{ return aForm;	}

inline void SwTOXBase::SetCreate(USHORT nCreate)
	{ nCreateType = nCreate; }

inline void SwTOXBase::SetTitle(const String& rTitle)
	{	aTitle = rTitle; }

inline void	SwTOXBase::SetTOXForm(const SwForm& rForm)
	{  aForm = rForm; }

inline TOXTypes SwTOXBase::GetType() const
	{ return GetTOXType()->GetType(); }

inline void SwTOXBase::SetLevel(USHORT nLev)
{
	ASSERT(GetTOXType()->GetType() != TOX_INDEX, "Falscher Feldtyp");
	aData.nLevel = nLev;
}

inline USHORT SwTOXBase::GetLevel() const
{
	ASSERT(GetTOXType()->GetType() != TOX_INDEX, "Falscher Feldtyp");
	return aData.nLevel;
}

inline void SwTOXBase::SetTemplateName(const String& rName)
{
//	ASSERT(GetTOXType()->GetType() == TOX_USER, "Falscher Feldtyp");
//	ASSERT(aData.pTemplateName, "pTemplateName == 0");
//	(*aData.pTemplateName) = rName;
	DBG_WARNING("SwTOXBase::SetTemplateName obsolete");
	aStyleNames[0] = rName;

}

inline String SwTOXBase::GetTemplateName() const
{
//	ASSERT(GetTOXType()->GetType() == TOX_USER, "Falscher Feldtyp");
//	return *aData.pTemplateName;
	DBG_WARNING("SwTOXBase::GetTemplateName obsolete");
	return aStyleNames[0].GetToken(0, TOX_STYLE_DELIMITER);
}

inline USHORT SwTOXBase::GetOptions() const
{
	ASSERT(GetTOXType()->GetType() == TOX_INDEX, "Falscher Feldtyp");
	return aData.nOptions;
}

inline void SwTOXBase::SetOptions(USHORT nOpt)
{
	ASSERT(GetTOXType()->GetType() == TOX_INDEX, "Falscher Feldtyp");
	aData.nOptions = nOpt;
}


} //namespace binfilter
#endif	// _TOX_HXX
