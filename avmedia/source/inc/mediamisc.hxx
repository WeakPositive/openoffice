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

class ResMgr;

#define AVMEDIA_RESID( nId ) ResId( nId, * ::avmedia::GetResMgr() )

#ifdef WNT 

#define AVMEDIA_MANAGER_SERVICE_NAME                    "com.sun.star.comp.avmedia.Manager_DirectX"
#define AVMEDIA_MANAGER_SERVICE_IS_JAVABASED            sal_False

#define AVMEDIA_MANAGER_SERVICE_NAME_FALLBACK1          ""
#define AVMEDIA_MANAGER_SERVICE_IS_JAVABASED_FALLBACK1  sal_False

#else
#ifdef QUARTZ

#define AVMEDIA_MANAGER_SERVICE_NAME                    "com.sun.star.comp.avmedia.Manager_QuickTime"
#define AVMEDIA_MANAGER_SERVICE_IS_JAVABASED            sal_False

#define AVMEDIA_MANAGER_SERVICE_NAME_FALLBACK1           ""
#define AVMEDIA_MANAGER_SERVICE_IS_JAVABASED_FALLBACK1  sal_False

#else

#define AVMEDIA_MANAGER_SERVICE_NAME                    "com.sun.star.comp.avmedia.Manager_GStreamer"
#define AVMEDIA_MANAGER_SERVICE_IS_JAVABASED            sal_False

#define AVMEDIA_MANAGER_SERVICE_NAME_FALLBACK1          "com.sun.star.comp.avmedia.Manager_Java"
#define AVMEDIA_MANAGER_SERVICE_IS_JAVABASED_FALLBACK1  sal_True

#endif
#endif
 
namespace avmedia
{
    ResMgr* GetResMgr();
}
