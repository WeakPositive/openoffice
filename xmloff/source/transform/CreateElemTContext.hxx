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

#ifndef _XMLOFF_CREATEELEMTCONTEXT_HXX
#define _XMLOFF_CREATEELEMTCONTEXT_HXX

#include <xmloff/xmltoken.hxx>
#include "TransformerContext.hxx"

class XMLMutableAttributeList;
class XMLTransformerActions;
class XMLCreateElemTransformerContext : public XMLTransformerContext 
{
	sal_uInt16 m_nActionMap;

protected:

	sal_uInt16 GetActionMap() const { return m_nActionMap; }

public:
	TYPEINFO();

	XMLCreateElemTransformerContext( XMLTransformerBase& rTransformer, 
						   const ::rtl::OUString& rQName,
		   				   sal_uInt16 nActionMap );

	virtual ~XMLCreateElemTransformerContext();

	virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
};

#endif	//  _XMLOFF_CREATEELEMCONTEXT_HXX

