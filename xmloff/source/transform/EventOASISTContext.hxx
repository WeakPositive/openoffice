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

#ifndef _XMLOFF_EVENTOASISTCONTEXT_HXX
#define _XMLOFF_EVENTOASISTCONTEXT_HXX

#include "RenameElemTContext.hxx"

class XMLTransformerOASISEventMap_Impl;

class XMLEventOASISTransformerContext : public XMLRenameElemTransformerContext
{
public:
	TYPEINFO();

	XMLEventOASISTransformerContext( XMLTransformerBase& rTransformer, 
						   const ::rtl::OUString& rQName );
	virtual ~XMLEventOASISTransformerContext();

	static XMLTransformerOASISEventMap_Impl *CreateFormEventMap();
	static XMLTransformerOASISEventMap_Impl *CreateEventMap();
	static void FlushEventMap( XMLTransformerOASISEventMap_Impl *p );
	static ::rtl::OUString GetEventName( sal_uInt16 nPrefix, 
							 const ::rtl::OUString& rName,
							 XMLTransformerOASISEventMap_Impl& rMap,
							 XMLTransformerOASISEventMap_Impl* pMap2	);

	virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
};

#endif	//  _XMLOFF_EVENTOASISTCONTEXT_HXX

