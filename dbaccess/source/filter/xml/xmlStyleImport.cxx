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
#include "precompiled_dbaccess.hxx"

#ifndef DBA_XMLSTYLEIMPORT_HXX
#include "xmlStyleImport.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_XMLIMPPR_HXX
#include <xmloff/xmlimppr.hxx>
#endif
#ifndef _XMLOFF_FAMILIES_HXX_
#include <xmloff/families.hxx>
#endif
#ifndef _XMLOFF_XMLNUMFI_HXX
#include <xmloff/xmlnumfi.hxx>
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLEFAMILIESSUPPLIER_HPP_
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_BORDERLINE_HPP_
#include <com/sun/star/table/BorderLine.hpp>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _XMLOFF_XMLPROPERTYSETCONTEXT_HXX
#include <xmloff/xmlprcon.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef DBA_XMLFILTER_HXX
#include "xmlfilter.hxx"
#endif
#ifndef DBA_XMLHELPER_HXX
#include "xmlHelper.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif


#define XML_LINE_LEFT 0
#define XML_LINE_RIGHT 1
#define XML_LINE_TOP 2
#define XML_LINE_BOTTOM 3

namespace dbaxml
{

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace xmloff::token;

// -----------------------------------------------------------------------------
TYPEINIT1( OTableStyleContext, XMLPropStyleContext );
TYPEINIT1( OTableStylesContext, SvXMLStylesContext );
DBG_NAME(OTableStyleContext)

OTableStyleContext::OTableStyleContext( ODBFilter& rImport,
		sal_uInt16 nPrfx, const ::rtl::OUString& rLName,
		const Reference< XAttributeList > & xAttrList,
		SvXMLStylesContext& rStyles, sal_uInt16 nFamily, sal_Bool bDefaultStyle )
    :XMLPropStyleContext( rImport, nPrfx, rLName, xAttrList, rStyles, nFamily, bDefaultStyle )
    ,sNumberFormat(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NumberFormat")))
    ,pStyles(&rStyles)
    ,m_nNumberFormat(-1)
    ,bConditionalFormatCreated(sal_False)
    ,bParentSet(sal_False)
{
    DBG_CTOR(OTableStyleContext,NULL);

}
// -----------------------------------------------------------------------------

OTableStyleContext::~OTableStyleContext()
{

    DBG_DTOR(OTableStyleContext,NULL);
}
// -----------------------------------------------------------------------------

void OTableStyleContext::FillPropertySet(
			const Reference< XPropertySet > & rPropSet )
{
	if ( !IsDefaultStyle() )
	{
		if ( GetFamily() == XML_STYLE_FAMILY_TABLE_TABLE )
		{
			if ( sPageStyle.getLength() )
			{
				uno::Any aAny;
				aAny <<= sPageStyle;
				AddProperty(CTF_DB_MASTERPAGENAME, aAny);
			}
		}
		else if ( GetFamily() == XML_STYLE_FAMILY_TABLE_COLUMN )
		{
			if ((m_nNumberFormat == -1) && m_sDataStyleName.getLength())
			{
				SvXMLNumFormatContext* pStyle = PTR_CAST(SvXMLNumFormatContext,pStyles->FindStyleChildContext(
					XML_STYLE_FAMILY_DATA_STYLE, m_sDataStyleName, sal_True));
				if ( !pStyle )
				{
					OTableStylesContext* pMyStyles = PTR_CAST(OTableStylesContext,GetOwnImport().GetAutoStyles());
					if ( pMyStyles )
						pStyle = PTR_CAST(SvXMLNumFormatContext,pMyStyles->
							FindStyleChildContext(XML_STYLE_FAMILY_DATA_STYLE, m_sDataStyleName, sal_True));
					else {
						DBG_ERROR("not possible to get style");
                    }
				}
				if ( pStyle )
				{
					uno::Any aNumberFormat;
					m_nNumberFormat = pStyle->GetKey();
					aNumberFormat <<= m_nNumberFormat;
					//rPropSet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_NUMBERFORMAT)), aNumberFormat);
					AddProperty(CTF_DB_NUMBERFORMAT, aNumberFormat);
				}
			}
		}
	}
	XMLPropStyleContext::FillPropertySet(rPropSet);
}
// -----------------------------------------------------------------------------

void OTableStyleContext::SetDefaults()
{
}
// -----------------------------------------------------------------------------

void OTableStyleContext::AddProperty(const sal_Int16 nContextID, const uno::Any& rValue)
{
	sal_Int32 nIndex(static_cast<OTableStylesContext *>(pStyles)->GetIndex(nContextID));
	DBG_ASSERT(nIndex != -1, "Property not found in Map");
	XMLPropertyState aPropState(nIndex, rValue);
	GetProperties().push_back(aPropState); // has to be insertes in a sort order later
}
// -----------------------------------------------------------------------------
void OTableStyleContext::SetAttribute( sal_uInt16 nPrefixKey,
										const ::rtl::OUString& rLocalName,
										const ::rtl::OUString& rValue )
{
	// TODO: use a map here
	if( IsXMLToken(rLocalName, XML_DATA_STYLE_NAME ) )
		m_sDataStyleName = rValue;
	else if ( IsXMLToken(rLocalName, XML_MASTER_PAGE_NAME ) )
		sPageStyle = rValue;
	else
		XMLPropStyleContext::SetAttribute( nPrefixKey, rLocalName, rValue );
}
// -----------------------------------------------------------------------------
ODBFilter& OTableStyleContext::GetOwnImport()
{
	return static_cast<ODBFilter&>(GetImport());
}
// -----------------------------------------------------------------------------
DBG_NAME(OTableStylesContext)

OTableStylesContext::OTableStylesContext( SvXMLImport& rImport,
		sal_uInt16 nPrfx ,
		const ::rtl::OUString& rLName ,
		const Reference< XAttributeList > & xAttrList,
		const sal_Bool bTempAutoStyles ) :
	SvXMLStylesContext( rImport, nPrfx, rLName, xAttrList ),
	sTableStyleServiceName( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( XML_STYLE_FAMILY_TABLE_TABLE_STYLES_NAME ))),
    sColumnStyleServiceName( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( XML_STYLE_FAMILY_TABLE_COLUMN_STYLES_NAME ))),
    sCellStyleServiceName( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( XML_STYLE_FAMILY_TABLE_CELL_STYLES_NAME ))),
	m_nNumberFormatIndex(-1),
	bAutoStyles(bTempAutoStyles)
{
    DBG_CTOR(OTableStylesContext,NULL);

}
// -----------------------------------------------------------------------------

OTableStylesContext::~OTableStylesContext()
{

    DBG_DTOR(OTableStylesContext,NULL);
}
// -----------------------------------------------------------------------------

void OTableStylesContext::EndElement()
{
	SvXMLStylesContext::EndElement();
	if (bAutoStyles)
		GetImport().GetTextImport()->SetAutoStyles( this );
	else
		GetImport().GetStyles()->CopyStylesToDoc(sal_True);
}
// -----------------------------------------------------------------------------

UniReference < SvXMLImportPropertyMapper >
	OTableStylesContext::GetImportPropertyMapper(
					sal_uInt16 nFamily ) const
{
	UniReference < SvXMLImportPropertyMapper > xMapper = SvXMLStylesContext::GetImportPropertyMapper(nFamily);

	if (!xMapper.is())
	{
		switch( nFamily )
		{
			case XML_STYLE_FAMILY_TABLE_TABLE:
			{
				if ( !m_xTableImpPropMapper.is() )
					m_xTableImpPropMapper = new SvXMLImportPropertyMapper( const_cast<OTableStylesContext*>(this)->GetOwnImport().GetTableStylesPropertySetMapper(), const_cast<SvXMLImport&>(GetImport()) );
				xMapper = m_xTableImpPropMapper;
			}
	 		break;
            case XML_STYLE_FAMILY_TABLE_COLUMN:
			{
				if ( !m_xColumnImpPropMapper.is() )
					m_xColumnImpPropMapper = new SvXMLImportPropertyMapper( const_cast<OTableStylesContext*>(this)->GetOwnImport().GetColumnStylesPropertySetMapper(), const_cast<SvXMLImport&>(GetImport()) );
				xMapper = m_xColumnImpPropMapper;
			}
	 		break;
            case XML_STYLE_FAMILY_TABLE_CELL:
			{
				if ( !m_xCellImpPropMapper.is() )
					m_xCellImpPropMapper = new SvXMLImportPropertyMapper( const_cast<OTableStylesContext*>(this)->GetOwnImport().GetCellStylesPropertySetMapper(), const_cast<SvXMLImport&>(GetImport()) );
				xMapper = m_xCellImpPropMapper;
			}
	 		break;
		}
	}

	return xMapper;
}
// ----------------------------------------------------------------------------
SvXMLStyleContext *OTableStylesContext::CreateStyleStyleChildContext(
		sal_uInt16 nFamily, sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName,
		const Reference< xml::sax::XAttributeList > & xAttrList )
{
	SvXMLStyleContext *pStyle = SvXMLStylesContext::CreateStyleStyleChildContext( nFamily, nPrefix,
															rLocalName,
															xAttrList );
	if (!pStyle)
	{
		switch( nFamily )
		{
		case XML_STYLE_FAMILY_TABLE_TABLE:
        case XML_STYLE_FAMILY_TABLE_COLUMN:
        case XML_STYLE_FAMILY_TABLE_CELL:
			pStyle = new OTableStyleContext( GetOwnImport(), nPrefix, rLocalName,
			   								xAttrList, *this, nFamily );
			break;
		}
	}

	return pStyle;
}
// -----------------------------------------------------------------------------
Reference < XNameContainer >
		OTableStylesContext::GetStylesContainer( sal_uInt16 nFamily ) const
{
	Reference < XNameContainer > xStyles = SvXMLStylesContext::GetStylesContainer(nFamily);
	return xStyles;
}
// -----------------------------------------------------------------------------

::rtl::OUString OTableStylesContext::GetServiceName( sal_uInt16 nFamily ) const
{
	rtl::OUString sServiceName = SvXMLStylesContext::GetServiceName(nFamily);
	if (!sServiceName.getLength())
	{
		switch( nFamily )
		{
		case XML_STYLE_FAMILY_TABLE_TABLE:
			sServiceName = sTableStyleServiceName;
			break;
        case XML_STYLE_FAMILY_TABLE_COLUMN:
			sServiceName = sColumnStyleServiceName;
			break;
        case XML_STYLE_FAMILY_TABLE_CELL:
			sServiceName = sCellStyleServiceName;
			break;
            
		}
	}
	return sServiceName;
}
// -----------------------------------------------------------------------------

sal_Int32 OTableStylesContext::GetIndex(const sal_Int16 nContextID)
{
	if ( nContextID == CTF_DB_NUMBERFORMAT )
	{
		if (m_nNumberFormatIndex == -1)
			m_nNumberFormatIndex =
				GetImportPropertyMapper(XML_STYLE_FAMILY_TABLE_COLUMN)->getPropertySetMapper()->FindEntryIndex(nContextID);
		return m_nNumberFormatIndex;
	}
	else if ( nContextID == CTF_DB_MASTERPAGENAME )
	{
		if (nMasterPageNameIndex == -1)
			nMasterPageNameIndex =
				GetImportPropertyMapper(XML_STYLE_FAMILY_TABLE_TABLE)->getPropertySetMapper()->FindEntryIndex(nContextID);
		return nMasterPageNameIndex;
	}
	else if ( nContextID == CTF_DB_ISVISIBLE )
	{
		return -1;
	}
	else
		return -1;
}
// -----------------------------------------------------------------------------
ODBFilter& OTableStylesContext::GetOwnImport()
{
	return static_cast<ODBFilter&>(GetImport());
}
// -----------------------------------------------------------------------------
} // dbaxml
// -----------------------------------------------------------------------------
