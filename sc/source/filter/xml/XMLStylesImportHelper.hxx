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

#ifndef SC_XMLSTYLESIMPORTHELPER_HXX
#define SC_XMLSTYLESIMPORTHELPER_HXX

#include "rangelst.hxx"
#include <rtl/ustring.hxx>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/table/CellAddress.hpp>

#include <set>
#include <vector>

class ScXMLImport;

struct ScMyStyleNumberFormat
{
	rtl::OUString		sStyleName;
	sal_Int32			nNumberFormat;

	ScMyStyleNumberFormat() : nNumberFormat(-1) {}
	ScMyStyleNumberFormat(const rtl::OUString& rStyleName) :
		sStyleName(rStyleName), nNumberFormat(-1) {}
	ScMyStyleNumberFormat(const rtl::OUString& rStyleName, const sal_Int32 nFormat) :
		sStyleName(rStyleName), nNumberFormat(nFormat) {}
};

struct LessStyleNumberFormat
{
	sal_Bool operator() (const ScMyStyleNumberFormat& rValue1, const ScMyStyleNumberFormat& rValue2) const
	{
		return rValue1.sStyleName < rValue2.sStyleName;
	}
};

typedef std::set< ScMyStyleNumberFormat, LessStyleNumberFormat >	ScMyStyleNumberFormatSet;

class ScMyStyleNumberFormats
{
	ScMyStyleNumberFormatSet	aSet;

public:
	void AddStyleNumberFormat(const rtl::OUString& rStyleName, const sal_Int32 nNumberFormat);
	sal_Int32 GetStyleNumberFormat(const rtl::OUString& rStyleName);
};

struct ScMyCurrencyStyle
{
	rtl::OUString		sCurrency;
	ScRangeListRef		xRanges;

	ScMyCurrencyStyle() : xRanges(new ScRangeList()) {}
	~ScMyCurrencyStyle() {}
};

struct LessCurrencyStyle
{
	sal_Bool operator() (const ScMyCurrencyStyle& rValue1, const ScMyCurrencyStyle& rValue2) const
	{
		return rValue1.sCurrency < rValue2.sCurrency;
	}
};

typedef std::set<ScMyCurrencyStyle, LessCurrencyStyle>	ScMyCurrencyStylesSet;

class ScMyStyleRanges : public SvRefBase
{
	ScRangeList*			pTextList;
	ScRangeList*			pNumberList;
	ScRangeList*			pTimeList;
	ScRangeList*			pDateTimeList;
	ScRangeList*			pPercentList;
	ScRangeList*			pLogicalList;
	ScRangeList*			pUndefinedList;
	ScMyCurrencyStylesSet*	pCurrencyList;

	void AddRange(const ScRange& rRange, ScRangeList* pList,
		const rtl::OUString* pStyleName, const sal_Int16 nType,
		ScXMLImport& rImport, const sal_uInt32 nMaxRanges);
	void AddCurrencyRange(const ScRange& rRange, ScRangeListRef xList,
		const rtl::OUString* pStyleName, const rtl::OUString* pCurrency,
		ScXMLImport& rImport, const sal_uInt32 nMaxRanges);
	void InsertColRow(const ScRange& rRange, const SCsCOL nDx, const SCsROW nDy,
		const SCsTAB nDz, ScDocument* pDoc);
	void SetStylesToRanges(ScRangeList* pList,
		const rtl::OUString* pStyleName, const sal_Int16 nCellType,
		const rtl::OUString* pCurrency, ScXMLImport& rImport);
	void SetStylesToRanges(ScRangeListRef xList,
		const rtl::OUString* pStyleName, const sal_Int16 nCellType,
		const rtl::OUString* pCurrency, ScXMLImport& rImport);
public:
	ScMyStyleRanges();
	~ScMyStyleRanges();
	void AddRange(const ScRange& rRange,
		const rtl::OUString* pStyleName, const sal_Int16 nType,
		ScXMLImport& rImport, const sal_uInt32 nMaxRanges);
	void AddCurrencyRange(const ScRange& rRange,
		const rtl::OUString* pStyleName, const rtl::OUString* pCurrency,
		ScXMLImport& rImport, const sal_uInt32 nMaxRanges);
	void InsertRow(const sal_Int32 nRow, const sal_Int32 nTab, ScDocument* pDoc);
	void InsertCol(const sal_Int32 nCol, const sal_Int32 nTab, ScDocument* pDoc);
	void SetStylesToRanges(const rtl::OUString* pStyleName, ScXMLImport& rImport);
};
SV_DECL_IMPL_REF( ScMyStyleRanges );

struct ScMyStyle
{
	rtl::OUString		sStyleName;
	ScMyStyleRangesRef	xRanges;

	ScMyStyle() : xRanges(new ScMyStyleRanges()) {}
	~ScMyStyle() {}
};

struct LessStyle
{
	sal_Bool operator() (const ScMyStyle& rValue1, const ScMyStyle& rValue2) const
	{
		return rValue1.sStyleName < rValue2.sStyleName;
	}
};

typedef std::set<ScMyStyle, LessStyle>	ScMyStylesSet;
typedef std::vector<ScMyStylesSet::iterator> ScMyStyles;

class ScMyStylesImportHelper
{
	ScMyStylesSet		aCellStyles;
	ScMyStyles			aColDefaultStyles;
	ScMyStylesSet::iterator	aRowDefaultStyle;
	ScXMLImport&		rImport;
	rtl::OUString*		pStyleName;
	rtl::OUString*		pPrevStyleName;
	rtl::OUString*		pCurrency;
	rtl::OUString*		pPrevCurrency;
	ScRange				aPrevRange;
	sal_uInt32			nMaxRanges;
	sal_Int16			nCellType;
	sal_Int16			nPrevCellType;
	sal_Bool			bPrevRangeAdded;

	void ResetAttributes();
	ScMyStylesSet::iterator GetIterator(const rtl::OUString* pStyleName);
	void AddDefaultRange(const ScRange& rRange);
	void AddSingleRange(const ScRange& rRange);
	void AddRange();
	sal_Bool IsEqual(const rtl::OUString* pFirst, const rtl::OUString* pSecond)
	{
		return ((pFirst && pSecond && pFirst->equals(*pSecond)) ||
				(!pFirst && !pSecond) ||
				(!pFirst && pSecond && !pSecond->getLength()) ||
				(!pSecond &&  pFirst && !pFirst->getLength()));
	}
public:
	ScMyStylesImportHelper(ScXMLImport& rImport);
	~ScMyStylesImportHelper();
	void AddColumnStyle(const rtl::OUString& rStyleName, const sal_Int32 nColumn, const sal_Int32 nRepeat);
	void SetRowStyle(const rtl::OUString& rStyleName);
	void SetAttributes(rtl::OUString* pStyleName,
		rtl::OUString* pCurrency, const sal_Int16 nCellType);
	void AddRange(const ScRange& rRange);
	void AddCell(const com::sun::star::table::CellAddress& rAddress);
	void InsertRow(const sal_Int32 nRow, const sal_Int32 nTab, ScDocument* pDoc); // a row is inserted before nRow
	void InsertCol(const sal_Int32 nCol, const sal_Int32 nTab, ScDocument* pDoc); // a col is inserted before nCol
	void EndTable();
	void SetStylesToRanges();
};

#endif

