#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2011 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

$(eval $(call gb_Library_Library,fwi))

$(eval $(call gb_Library_add_package_headers,fwi,framework_inc))

$(eval $(call gb_Library_set_defs,fwi,\
	$$(DEFS) \
	-DFWI_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_set_include,fwi,\
	-I$(SRCDIR)/framework/inc/pch \
	-I$(SRCDIR)/framework/source/inc \
	-I$(SRCDIR)/framework/inc \
	-I$(WORKDIR)/inc/framework/ \
	$$(INCLUDE) \
	-I$(OUTDIR)/inc/framework \
	-I$(OUTDIR)/inc/offuh \
))

$(eval $(call gb_Library_add_linked_libs,fwi,\
	comphelper \
	cppu \
	cppuhelper \
	i18nisolang1 \
	sal \
	stl \
	svl \
	svt \
	tk \
	tl \
	utl \
	vcl \
	vos3 \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,fwi,\
	framework/source/fwi/classes/converter \
	framework/source/fwi/classes/propertysethelper \
	framework/source/fwi/classes/protocolhandlercache \
	framework/source/fwi/helper/mischelper \
	framework/source/fwi/helper/networkdomain \
	framework/source/fwi/helper/shareablemutex \
	framework/source/fwi/jobs/configaccess \
	framework/source/fwi/jobs/jobconst \
	framework/source/fwi/threadhelp/lockhelper \
	framework/source/fwi/threadhelp/transactionmanager \
	framework/source/fwi/uielement/constitemcontainer \
	framework/source/fwi/uielement/itemcontainer \
	framework/source/fwi/uielement/rootitemcontainer \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Library_add_linked_libs,fwi,\
	advapi32 \
))
endif

# vim: set noet sw=4 ts=4:
