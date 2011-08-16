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

#ifndef _XMLOFF_XMLINDEXTITLETEMPLATECONTEXT_HXX_
#define _XMLOFF_XMLINDEXTITLETEMPLATECONTEXT_HXX_

#include <xmloff/xmlictxt.hxx>
#include <com/sun/star/uno/Reference.h>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>


namespace com { namespace sun { namespace star {
	namespace beans { class XPropertySet; }
	namespace xml { namespace sax { class XAttributeList; } }
} } }


/**
 * Import index title templates
 */
class XMLIndexTitleTemplateContext : public SvXMLImportContext
{

	const ::rtl::OUString sTitle;
	const ::rtl::OUString sParaStyleHeading;

	// paragraph style
	::rtl::OUString sStyleName;
	sal_Bool bStyleNameOK;

	// content
	::rtl::OUStringBuffer sContent;

	// TOC property set
	::com::sun::star::uno::Reference< 
		::com::sun::star::beans::XPropertySet> & rTOCPropertySet;

public:

	TYPEINFO();

	 XMLIndexTitleTemplateContext(
		SvXMLImport& rImport, 
		::com::sun::star::uno::Reference< 
			::com::sun::star::beans::XPropertySet> & rPropSet,
		sal_uInt16 nPrfx,
		const ::rtl::OUString& rLocalName);

	~XMLIndexTitleTemplateContext();

protected:

	/** process parameters */
	virtual void StartElement(
		const ::com::sun::star::uno::Reference< 
			::com::sun::star::xml::sax::XAttributeList> & xAttrList);

	/** set values */
	virtual void EndElement();

	/** pick up title characters */
	virtual void Characters(const ::rtl::OUString& sString);
};

#endif
