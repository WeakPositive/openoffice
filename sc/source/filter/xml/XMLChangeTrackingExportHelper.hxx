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

#ifndef SC_XMLCHANGETRACKINGEXPORTHELPER_HXX
#define SC_XMLCHANGETRACKINGEXPORTHELPER_HXX

#include <xmloff/xmltoken.hxx>
#include <list>
#include <com/sun/star/text/XText.hpp>
#include <rtl/ustrbuf.hxx>

class ScChangeAction;
class ScChangeTrack;
class ScXMLExport;
class ScBaseCell;
class ScChangeActionDel;
class ScBigRange;
class ScEditEngineTextObj;
class ScChangeActionTable;
class String;
class DateTime;

typedef std::list<ScChangeActionDel*> ScMyDeletionsList;

class ScChangeTrackingExportHelper
{
	ScXMLExport&	rExport;

	ScChangeTrack*	pChangeTrack;
	ScEditEngineTextObj* pEditTextObj;
	ScChangeActionTable* pDependings;
	rtl::OUString	sChangeIDPrefix;
	com::sun::star::uno::Reference<com::sun::star::text::XText>	xText;

	rtl::OUString GetChangeID(const sal_uInt32 nActionNumber);
	void GetAcceptanceState(const ScChangeAction* pAction);

	void WriteBigRange(const ScBigRange& rBigRange, xmloff::token::XMLTokenEnum aName);
	void WriteChangeInfo(const ScChangeAction* pAction);
	void WriteGenerated(const ScChangeAction* pDependAction);
	void WriteDeleted(const ScChangeAction* pDependAction);
	void WriteDepending(const ScChangeAction* pDependAction);
	void WriteDependings(ScChangeAction* pAction);

	void WriteEmptyCell();
	void SetValueAttributes(const double& fValue, const String& sValue);
	void WriteValueCell(const ScBaseCell* pCell, const String& sValue);
	void WriteStringCell(const ScBaseCell* pCell);
	void WriteEditCell(const ScBaseCell* pCell);
	void WriteFormulaCell(const ScBaseCell* pCell, const String& sValue);
	void WriteCell(const ScBaseCell* pCell, const String& sValue);

	void WriteContentChange(ScChangeAction* pAction);
	void AddInsertionAttributes(const ScChangeAction* pAction);
	void WriteInsertion(ScChangeAction* pAction);
	void AddDeletionAttributes(const ScChangeActionDel* pAction, const ScChangeActionDel* pLastAction);
	void WriteDeletionCells(ScChangeActionDel* pAction);
	void WriteCutOffs(const ScChangeActionDel* pAction);
	void WriteDeletion(ScChangeAction* pAction);
	void WriteMovement(ScChangeAction* pAction);
	void WriteRejection(ScChangeAction* pAction);

	void CollectCellAutoStyles(const ScBaseCell* pBaseCell);
	void CollectActionAutoStyles(ScChangeAction* pAction);
	void WorkWithChangeAction(ScChangeAction* pAction);
public:
	ScChangeTrackingExportHelper(ScXMLExport& rExport);
	~ScChangeTrackingExportHelper();

	void CollectAutoStyles();
	void CollectAndWriteChanges();
};

#endif
