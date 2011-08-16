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

#ifndef _XMLOFF_MERGEELEMTCONTEXT_HXX
#define _XMLOFF_MERGEELEMTCONTEXT_HXX

#include <rtl/ref.hxx>
#include "FlatTContext.hxx"
#include <vector>

typedef ::std::vector< ::rtl::Reference< XMLPersTextContentTContext> >
		XMLPersTextContentTContextVector;

class XMLMergeElemTransformerContext : public XMLTransformerContext 
{
	::com::sun::star::uno::Reference< 
		::com::sun::star::xml::sax::XAttributeList > m_xAttrList;
	XMLPersTextContentTContextVector m_aChildContexts;
	sal_uInt16 m_nActionMap;
	sal_Bool m_bStartElementExported;

	void ExportStartElement();

public:
	TYPEINFO();

	XMLMergeElemTransformerContext( XMLTransformerBase& rTransformer, 
						   const ::rtl::OUString& rQName,
		   				   sal_uInt16 nActionMap );

	virtual ~XMLMergeElemTransformerContext();

	virtual XMLTransformerContext *CreateChildContext( sal_uInt16 nPrefix,
								   const ::rtl::OUString& rLocalName,
								   const ::rtl::OUString& rQName,
								   const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

	virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

	virtual void EndElement();

	virtual void Characters( const ::rtl::OUString& rChars );
};

#endif	//  _XMLOFF_MERGEELEMCONTEXT_HXX

