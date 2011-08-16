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
#ifndef _DOCSTYLE_HXX
#define _DOCSTYLE_HXX

#include <rtl/ref.hxx>

#ifndef _SVSTDARR_HXX
#define _SVSTDARR_STRINGSDTOR
#include <svl/svstdarr.hxx>
#endif
#include <svl/style.hxx>
#include <svl/itemset.hxx>
#include "swdllapi.h"

class SwDoc;
class SwDocStyleSheetPool;
class SwPageDesc;
class SwCharFmt;
class SwTxtFmtColl;
class SwFrmFmt;
class SwNumRule;

/*--------------------------------------------------------------------
	Beschreibung:	Lokale Hilfsklasse
 --------------------------------------------------------------------*/
class SwPoolFmtList : public SvStringsDtor
{
public:
	SwPoolFmtList() {}
	void Append( char cChar, const String& rStr );
	void Erase();
};


/*--------------------------------------------------------------------
	Beschreibung:	temp. StyleSheet
 --------------------------------------------------------------------*/
class SW_DLLPUBLIC SwDocStyleSheet : public SfxStyleSheetBase
{
	friend class SwDocStyleSheetPool;
	friend class SwStyleSheetIterator;

	SwCharFmt*			pCharFmt;
	SwTxtFmtColl* 		pColl;
	SwFrmFmt* 			pFrmFmt;
	const SwPageDesc* 	pDesc;
	const SwNumRule*	pNumRule;

	SwDoc&				rDoc;
	SfxItemSet			aCoreSet;

	sal_Bool 				bPhysical;


	// leere Huelse zum richtigen StyleSheet (Core) machen
	SW_DLLPRIVATE void				Create();

	// den StyleSheet mit Daten fuellen
	enum FillStyleType {
		FillOnlyName,
		FillAllInfo,
		FillPhysical
	};

	SW_DLLPRIVATE sal_Bool FillStyleSheet( FillStyleType eFType );

protected:
	virtual ~SwDocStyleSheet();

public:
	SwDocStyleSheet( SwDoc& 				rDoc,
					 const String& 			rName,
					 SwDocStyleSheetPool& 	rPool,
					 SfxStyleFamily 		eFam,
					 sal_uInt16 				nMask);

	SwDocStyleSheet( const SwDocStyleSheet& );

	void					Reset();

	void 					SetMask(sal_uInt16 nMsk)			{ nMask = nMsk;   	}
	void 					SetFamily(SfxStyleFamily eFam)	{ nFamily = eFam;	}

	sal_Bool		   			IsPhysical() const				{ return bPhysical; }
	void					SetPhysical(sal_Bool bPhys);

    // --> OD 2008-02-11 #newlistlevelattrs#
    // add optional parameter <bResetIndentAttrsAtParagraphStyle>, default value sal_False,
    // which indicates that the indent attributes at a paragraph style should
    // be reset in case that a list style is applied to the paragraph style and
    // no indent attributes are applied.
    void                    SetItemSet( const SfxItemSet& rSet,
                                        const bool bResetIndentAttrsAtParagraphStyle = false );
    // <--

	virtual SfxItemSet& 	GetItemSet();
    // --> OD 2008-02-12 #newlistlevelattrs#
    // new method for paragraph styles to merge indent attributes of applied list
    // style into the given item set, if the list style indent attributes are applicable.
    void MergeIndentAttrsOfListStyle( SfxItemSet& rSet );
    // <--
	virtual const String& 	GetParent() const;
	virtual const String& 	GetFollow() const;

	virtual sal_uLong GetHelpId( String& rFile );
	virtual void SetHelpId( const String& r, sal_uLong nId );

	// Vorbelegen der member ohne physikalischen Zugriff
	// wird vom StyleSheetPool benutzt
	//
	void			  		PresetName(const String& rName)  { aName   = rName; }
	void			  		PresetNameAndFamily(const String& rName);
	void					PresetParent(const String& rName){ aParent = rName; }
	void					PresetFollow(const String& rName){ aFollow = rName; }

	virtual sal_Bool 			SetName( const String& rStr);
	virtual sal_Bool 			SetParent( const String& rStr);
	virtual sal_Bool 			SetFollow( const String& rStr);

	virtual sal_Bool 			HasFollowSupport() const;
	virtual sal_Bool 			HasParentSupport() const;
	virtual sal_Bool 			HasClearParentSupport() const;
	virtual String 			GetDescription();
	virtual String 			GetDescription(SfxMapUnit eUnit);

	SwCharFmt*		   		GetCharFmt();
	SwTxtFmtColl* 			GetCollection();
	SwFrmFmt* 				GetFrmFmt();
	const SwPageDesc* 		GetPageDesc();
	const SwNumRule*		GetNumRule();
	void					SetNumRule(const SwNumRule& rRule);

	virtual sal_Bool 			IsUsed() const;
};

/*--------------------------------------------------------------------
	Beschreibung:	Iterator fuer den Pool
 --------------------------------------------------------------------*/

class SwStyleSheetIterator : public SfxStyleSheetIterator, public SfxListener
{
	rtl::Reference< SwDocStyleSheet > mxIterSheet;
	rtl::Reference< SwDocStyleSheet > mxStyleSheet;
	SwPoolFmtList 		aLst;
	sal_uInt16 				nLastPos;
	sal_Bool 				bFirstCalled;

	void				AppendStyleList(const SvStringsDtor& rLst,
										sal_Bool 	bUsed,
										sal_uInt16 	nSection,
										char	cType);

public:
	SwStyleSheetIterator( SwDocStyleSheetPool* pBase,
						  SfxStyleFamily eFam, sal_uInt16 n=0xFFFF );
	virtual ~SwStyleSheetIterator();

	virtual sal_uInt16 Count();
	virtual SfxStyleSheetBase *operator[](sal_uInt16 nIdx);
	virtual SfxStyleSheetBase* First();
	virtual SfxStyleSheetBase* Next();
	virtual SfxStyleSheetBase* Find(const UniString& rStr);

	virtual void Notify( SfxBroadcaster&, const SfxHint& );
};

/*--------------------------------------------------------------------
	Beschreibung:	Pool fuer
 --------------------------------------------------------------------*/

class SwDocStyleSheetPool : public SfxStyleSheetBasePool
{
	rtl::Reference< SwDocStyleSheet > mxStyleSheet;
	SwDoc&				rDoc;
	sal_Bool				bOrganizer : 1;		// sal_True: fuer den Organizer


	virtual SfxStyleSheetBase* Create( const String&, SfxStyleFamily, sal_uInt16 nMask);
	virtual SfxStyleSheetBase* Create( const SfxStyleSheetBase& );

    using SfxStyleSheetBasePool::Find;

public:
	SwDocStyleSheetPool( SwDoc&, sal_Bool bOrganizer = sal_False );

	virtual void Replace( SfxStyleSheetBase& rSource,
						  SfxStyleSheetBase& rTarget );
	virtual SfxStyleSheetBase& Make(const String&, SfxStyleFamily, sal_uInt16 nMask, sal_uInt16 nPos = 0xffff);

	virtual SfxStyleSheetBase* Find( const String&, SfxStyleFamily eFam,
									sal_uInt16 n=0xFFFF );

	virtual sal_Bool SetParent( SfxStyleFamily eFam, const String &rStyle,
							const String &rParent );

	virtual void Remove( SfxStyleSheetBase* pStyle);

	void	SetOrganizerMode( sal_Bool bMode )	{ bOrganizer = bMode; }
	sal_Bool 	IsOrganizerMode() const 		{ return bOrganizer; }

	virtual SfxStyleSheetIterator* CreateIterator( SfxStyleFamily,
													sal_uInt16 nMask );

	SwDoc& GetDoc() const { return rDoc; }

	void dispose();

    virtual void SAL_CALL acquire(  ) throw ();
    virtual void SAL_CALL release(  ) throw ();

protected:
	virtual ~SwDocStyleSheetPool();

	//Fuer die daemlicheren Compiler
private:
	SwDocStyleSheetPool( const SwDocStyleSheetPool& );
};


#endif
