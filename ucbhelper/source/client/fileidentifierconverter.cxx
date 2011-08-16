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
#include "precompiled_ucbhelper.hxx"
#include <ucbhelper/fileidentifierconverter.hxx>
#include <com/sun/star/ucb/ContentProviderInfo.hpp>
#include <com/sun/star/ucb/XContentProviderManager.hpp>
#include <com/sun/star/ucb/XFileIdentifierConverter.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <osl/diagnose.h>
#include <rtl/ustring.hxx>
#include <sal/types.h>

using namespace com::sun::star;

namespace ucbhelper {

//============================================================================
//
//  getLocalFileURL
//
//============================================================================

rtl::OUString
getLocalFileURL(
	uno::Reference< ucb::XContentProviderManager > const &)
    SAL_THROW((uno::RuntimeException))
{
    // If there were more file systems than just "file:///" (e.g., the obsolete
    // "vnd.sun.star.wfs:///"), this code should query all relevant UCPs for
    // their com.sun.star.ucb.XFileIdentifierConverter.getFileProviderLocality
    // and return the most local one:
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("file:///"));
}

//============================================================================
//
//  getFileURLFromSystemPath
//
//============================================================================

rtl::OUString
getFileURLFromSystemPath(
	uno::Reference< ucb::XContentProviderManager > const & rManager,
	rtl::OUString const & rBaseURL,
	rtl::OUString const & rSystemPath)
    SAL_THROW((uno::RuntimeException))
{
	OSL_ASSERT(rManager.is());

	uno::Reference< ucb::XFileIdentifierConverter >
		xConverter(rManager->queryContentProvider(rBaseURL), uno::UNO_QUERY);
	if (xConverter.is())
		return xConverter->getFileURLFromSystemPath(rBaseURL, rSystemPath);
	else
		return rtl::OUString();
}

//============================================================================
//
//  getSystemPathFromFileURL
//
//============================================================================

rtl::OUString
getSystemPathFromFileURL(
	uno::Reference< ucb::XContentProviderManager > const & rManager,
	rtl::OUString const & rURL)
    SAL_THROW((uno::RuntimeException))
{
	OSL_ASSERT(rManager.is());

	uno::Reference< ucb::XFileIdentifierConverter >
		xConverter(rManager->queryContentProvider(rURL), uno::UNO_QUERY);
	if (xConverter.is())
		return xConverter->getSystemPathFromFileURL(rURL);
	else
		return rtl::OUString();
}

}
