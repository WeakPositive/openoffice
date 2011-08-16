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
PRJ=..$/..$/..$/..

PRJNAME=officecfg
TARGET=data_oopenoffice
PACKAGE=org.openoffice

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Targets ------------------------------------------------------

XCUFILES= \
    Inet.xcu \
    Setup.xcu \
    System.xcu \
    VCL.xcu \
    FirstStartWizard.xcu \
    UserProfile.xcu \
    Interaction.xcu

MODULEFILES= \
    Inet-macosx.xcu \
    Inet-unixdesktop.xcu \
    Inet-wnt.xcu \
    Setup-brand.xcu \
    Setup-writer.xcu   \
    Setup-calc.xcu   \
    Setup-draw.xcu   \
    Setup-impress.xcu   \
    Setup-base.xcu   \
    Setup-math.xcu \
    Setup-report.xcu \
    Setup-start.xcu \
    UserProfile-gconflockdown.xcu \
    VCL-gconflockdown.xcu \
    VCL-unixdesktop.xcu

LOCALIZEDFILES=Setup.xcu

.INCLUDE :  target.mk

LANGUAGEPACKS=$(MISC)$/registry$/spool$/Langpack-{$(alllangiso)}.xcu

ALLTAR : $(LANGUAGEPACKS)
