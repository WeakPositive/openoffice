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
#include "precompiled_svl.hxx"
#include <tools/stream.hxx>

#ifndef NUMTYPE

#define NUMTYPE sal_uInt16
#define SfxXRangeItem SfxRangeItem
#define SfxXRangesItem SfxUShortRangesItem
#include <svl/rngitem.hxx>
#include "rngitem_inc.cxx"

#define NUMTYPE	sal_uInt32
#define SfxXRangeItem SfxULongRangeItem
#define SfxXRangesItem SfxULongRangesItem
#include <svl/rngitem.hxx>
#include "rngitem_inc.cxx"

#else

// We leave this condition just in case NUMTYPE has been defined externally to this
// file and we are supposed to define the SfxXRangeItem based on that.

#include "rngitem_inc.cxx"

#endif

