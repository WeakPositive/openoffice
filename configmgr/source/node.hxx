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

#ifndef INCLUDED_CONFIGMGR_SOURCE_NODE_HXX
#define INCLUDED_CONFIGMGR_SOURCE_NODE_HXX

#include "sal/config.h"

#include "rtl/ref.hxx"
#include "salhelper/simplereferenceobject.hxx"

#include "nodemap.hxx"

namespace rtl { class OUString; }

namespace configmgr {

class Node: public salhelper::SimpleReferenceObject {
public:
    enum Kind {
        KIND_PROPERTY, KIND_LOCALIZED_PROPERTY, KIND_LOCALIZED_VALUE,
        KIND_GROUP, KIND_SET };

    virtual Kind kind() const = 0;

    virtual rtl::Reference< Node > clone(bool keepTemplateName) const = 0;

    virtual NodeMap & getMembers();

    virtual rtl::OUString getTemplateName() const;

    virtual void setMandatory(int layer);

    virtual int getMandatory() const;

    void setLayer(int layer);

    int getLayer() const;

    void setFinalized(int layer);

    int getFinalized() const;

    rtl::Reference< Node > getMember(rtl::OUString const & name);

protected:
    explicit Node(int layer);

    Node(const Node & other);

    virtual ~Node();

    virtual void clear();

    int layer_;
    int finalized_;
};

}

#endif
