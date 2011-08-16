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
#include "precompiled_xmloff.hxx"
#include <tools/debug.hxx>
#include "XMLTextPropertySetContext.hxx"
#include "XMLTextColumnsContext.hxx"
#include "XMLBackgroundImageContext.hxx"
#include "XMLSectionFootnoteConfigImport.hxx"

#ifndef _XMLOFF_TXTPRMAP_HXX
#include <xmloff/txtprmap.hxx>
#endif
#include "xmltabi.hxx"
#ifndef _XMLOFF_TXTDROPI_HXX
#include "txtdropi.hxx"
#endif

using ::rtl::OUString;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;

XMLTextPropertySetContext::XMLTextPropertySetContext(
				 SvXMLImport& rImport, sal_uInt16 nPrfx,
				 const OUString& rLName,
				 const Reference< xml::sax::XAttributeList > & xAttrList,
				 sal_uInt32 nFamily,
				 ::std::vector< XMLPropertyState > &rProps,
				 const UniReference < SvXMLImportPropertyMapper > &rMap,
				 OUString& rDCTextStyleName ) :
	SvXMLPropertySetContext( rImport, nPrfx, rLName, xAttrList, nFamily,
							 rProps, rMap ),
	rDropCapTextStyleName( rDCTextStyleName )
{
}

XMLTextPropertySetContext::~XMLTextPropertySetContext()
{
}
	
SvXMLImportContext *XMLTextPropertySetContext::CreateChildContext(
				   sal_uInt16 nPrefix,
				   const OUString& rLocalName,
				   const Reference< xml::sax::XAttributeList > & xAttrList,
				   ::std::vector< XMLPropertyState > &rProperties,
				   const XMLPropertyState& rProp )
{
	SvXMLImportContext *pContext = 0;

	switch( mxMapper->getPropertySetMapper()
					->GetEntryContextId( rProp.mnIndex ) )
	{
	case CTF_TABSTOP:
		pContext = new SvxXMLTabStopImportContext( GetImport(), nPrefix,
												   rLocalName, rProp,
												   rProperties );
		break;
	case CTF_TEXTCOLUMNS:
#ifndef SVX_LIGHT
		pContext = new XMLTextColumnsContext( GetImport(), nPrefix,
												   rLocalName, xAttrList, rProp,
												   rProperties );
#else
		// create default context to skip content
		pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
#endif // #ifndef SVX_LIGHT
		break;

	case CTF_DROPCAPFORMAT:
		{
			DBG_ASSERT( rProp.mnIndex >= 2 &&
						CTF_DROPCAPWHOLEWORD  == mxMapper->getPropertySetMapper()
							->GetEntryContextId( rProp.mnIndex-2 ),
						"invalid property map!");
			XMLTextDropCapImportContext *pDCContext =
				new XMLTextDropCapImportContext( GetImport(), nPrefix,
														rLocalName, xAttrList,
														rProp,
														rProp.mnIndex-2,
														rProperties );
			rDropCapTextStyleName = pDCContext->GetStyleName();
		pContext = pDCContext;
		}
		break;

	case CTF_BACKGROUND_URL:
    {
		DBG_ASSERT( rProp.mnIndex >= 2 &&
					CTF_BACKGROUND_POS  == mxMapper->getPropertySetMapper()
						->GetEntryContextId( rProp.mnIndex-2 ) &&
					CTF_BACKGROUND_FILTER  == mxMapper->getPropertySetMapper()
						->GetEntryContextId( rProp.mnIndex-1 ),
					"invalid property map!");

        // #99657# Transparency might be there as well... but doesn't have
        // to. Thus, this is checked with an if, rather than with an assertion.
        sal_Int32 nTranspIndex = -1;
        if( (rProp.mnIndex >= 3) &&
            ( CTF_BACKGROUND_TRANSPARENCY == 
              mxMapper->getPropertySetMapper()->GetEntryContextId( 
                  rProp.mnIndex-3 ) ) )
            nTranspIndex = rProp.mnIndex-3;

		pContext = 
			new XMLBackgroundImageContext( GetImport(), nPrefix,
										   rLocalName, xAttrList,
										   rProp,
										   rProp.mnIndex-2,
										   rProp.mnIndex-1,
										   nTranspIndex,
										   rProperties );
    }
    break;
#ifndef SVX_LIGHT
	case CTF_SECTION_FOOTNOTE_END:
	case CTF_SECTION_ENDNOTE_END:
		pContext = new XMLSectionFootnoteConfigImport( 
			GetImport(), nPrefix, rLocalName, rProperties, 
			mxMapper->getPropertySetMapper());
		break;
#endif // #ifndef SVX_LIGHT
	}
	
	if( !pContext )
		pContext = SvXMLPropertySetContext::CreateChildContext( nPrefix, rLocalName,
															xAttrList, 
															rProperties, rProp );
	
	return pContext;
}


