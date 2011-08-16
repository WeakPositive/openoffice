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

#ifndef __FRAMEWORK_XML_XMLNAMESPACES_HXX_
#define __FRAMEWORK_XML_XMLNAMESPACES_HXX_

#include <com/sun/star/xml/sax/SAXException.hpp>

#include <map>
#include <framework/fwedllapi.h>

namespace framework
{

class FWE_DLLPUBLIC XMLNamespaces
{
    public:
        XMLNamespaces();
        XMLNamespaces( const XMLNamespaces& );
        virtual ~XMLNamespaces();

        void addNamespace( const ::rtl::OUString& aName, const ::rtl::OUString& aValue )
            throw(  ::com::sun::star::xml::sax::SAXException );

        ::rtl::OUString applyNSToAttributeName( const ::rtl::OUString& ) const
            throw(  ::com::sun::star::xml::sax::SAXException );
        ::rtl::OUString applyNSToElementName( const ::rtl::OUString& ) const
            throw(  ::com::sun::star::xml::sax::SAXException );

    private:
        typedef ::std::map< ::rtl::OUString, ::rtl::OUString > NamespaceMap;

        ::rtl::OUString getNamespaceValue( const ::rtl::OUString& aNamespace ) const
            throw( ::com::sun::star::xml::sax::SAXException );

        ::rtl::OUString m_aDefaultNamespace;
        NamespaceMap    m_aNamespaceMap;
};

}

#endif // __FRAMEWORK_XML_XMLNAMESPACES_HXX_
