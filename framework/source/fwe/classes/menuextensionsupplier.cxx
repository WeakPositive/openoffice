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
#include "precompiled_framework.hxx"
#include <framework/menuextensionsupplier.hxx>
#include <osl/mutex.hxx>

static pfunc_setMenuExtensionSupplier pMenuExtensionSupplierFunc = NULL;

namespace framework
{

pfunc_setMenuExtensionSupplier SAL_CALL SetMenuExtensionSupplier( pfunc_setMenuExtensionSupplier pMenuExtensionSupplierFuncArg )
{
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    
    pfunc_setMenuExtensionSupplier pOldMenuExtensionSupplierFunc = pMenuExtensionSupplierFunc;
    pMenuExtensionSupplierFunc = pMenuExtensionSupplierFuncArg;
    return pOldMenuExtensionSupplierFunc;
}

MenuExtensionItem SAL_CALL GetMenuExtension()
{
    MenuExtensionItem aItem;
    
    pfunc_setMenuExtensionSupplier pLocalMenuExtensionSupplierFunc( 0 );

    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        pLocalMenuExtensionSupplierFunc = pMenuExtensionSupplierFunc;
    }

    if ( pLocalMenuExtensionSupplierFunc )
        return (*pLocalMenuExtensionSupplierFunc)();
    else
        return aItem;
}

}
