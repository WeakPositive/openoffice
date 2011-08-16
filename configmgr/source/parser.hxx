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

#ifndef INCLUDED_CONFIGMGR_SOURCE_PARSER_HXX
#define INCLUDED_CONFIGMGR_SOURCE_PARSER_HXX

#include "sal/config.h"

#include <memory>

#include "salhelper/simplereferenceobject.hxx"
#include "xmlreader/xmlreader.hxx"

namespace xmlreader { struct Span; }

namespace configmgr {

class Parser: public salhelper::SimpleReferenceObject {
public:
    virtual xmlreader::XmlReader::Text getTextMode() = 0;

    virtual bool startElement(
        xmlreader::XmlReader & reader, int nsId, xmlreader::Span const & name)
        = 0;

    virtual void endElement(xmlreader::XmlReader const & reader) = 0;

    virtual void characters(xmlreader::Span const & text) = 0;

protected:
    Parser() {}

    virtual ~Parser() {}
};

}

#endif
