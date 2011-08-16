#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
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

PRJ=..$/..

PRJNAME=connectivity
TARGET=simpledbt

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/dbtools.pmk

# --- Files --------------------------------------------------------

EXCEPTIONSFILES=\
        $(SLO)$/charset_s.obj   \
        $(SLO)$/staticdbtools_s.obj \
        $(SLO)$/parsenode_s.obj     \
        $(SLO)$/parser_s.obj

SLOFILES=\
        $(EXCEPTIONSFILES)       \
		$(SLO)$/refbase.obj		\
		$(SLO)$/dbtfactory.obj	\

# --- Targets ------------------------------------------------------

.INCLUDE :      target.mk


