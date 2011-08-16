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

PRJ     = ..$/..$/..
TARGET  = FCFGMerge
PRJNAME = l10ntools
PACKAGE = com$/sun$/star$/filter$/config$/tools$/merge

# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk

#----- compile .java files -----------------------------------------
#.IF "$(L10N_framework)"==""

OWNCOPY         =   \
                    $(MISC)$/$(TARGET)_copied.done

#JARFILES        =   \
#                    ridl.jar        \
#                    unoil.jar       \
#                    jurt.jar        \
#                    juh.jar         \
#                    java_uno.jar

CFGFILES        =   \
                    FCFGMerge.cfg

JAVACLASSFILES  =   \
                    $(CLASSDIR)$/$(PACKAGE)$/Merger.class     \
                    $(CLASSDIR)$/$(PACKAGE)$/FCFGMerge.class

CUSTOMMANIFESTFILE     =   \
                    Manifest.mf

MAXLINELENGTH   =   100000

#----- make a jar from compiled files ------------------------------

JARCLASSDIRS    =   \
                    com$/sun$/star$/filter$/config$/tools$/utils    \
                    com$/sun$/star$/filter$/config$/tools$/merge

JARTARGET       =   $(TARGET).jar

JARCOMPRESS     =   TRUE

# --- targets -----------------------------------------------------

.INCLUDE :  target.mk

.IF "$(SOLAR_JAVA)" != "" || "$(GUI)"=="OS2"
ALLTAR : $(OWNCOPY)

.IF "$(JARTARGETN)" != ""
$(JARTARGETN) : $(OWNCOPY)
.ENDIF

$(OWNCOPY) : $(CFGFILES)
    -$(MKDIRHIER) $(CLASSDIR)$/$(PACKAGE)
    $(COPY) $? $(CLASSDIR)$/$(PACKAGE) && $(TOUCH) $@

.ENDIF # "$(SOLAR_JAVA)" != ""

#.ELSE
#pseudo:

#.ENDIF
