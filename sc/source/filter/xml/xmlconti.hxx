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
#ifndef SC_XMLCONTI_HXX
#define SC_XMLCONTI_HXX

#include <xmloff/xmlimp.hxx>
#include <rtl/ustrbuf.hxx>

class ScXMLImport;

class ScXMLContentContext : public SvXMLImportContext
{
	rtl::OUStringBuffer sOUText;
	rtl::OUStringBuffer& sValue;

	const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
	ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

	ScXMLContentContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
					   const ::rtl::OUString& rLName,
					   const ::com::sun::star::uno::Reference<
										::com::sun::star::xml::sax::XAttributeList>& xAttrList,
						rtl::OUStringBuffer& sValue);

	virtual ~ScXMLContentContext();

	virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
									 const ::rtl::OUString& rLocalName,
									 const ::com::sun::star::uno::Reference<
									  	::com::sun::star::xml::sax::XAttributeList>& xAttrList );

	virtual void Characters( const ::rtl::OUString& rChars );

	virtual void EndElement();
};

#endif
