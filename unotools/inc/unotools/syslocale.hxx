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

#ifndef INCLUDED_UNOTOOLS_SYSLOCALE_HXX
#define INCLUDED_UNOTOOLS_SYSLOCALE_HXX

#include "unotools/unotoolsdllapi.h"
#include <unotools/localedatawrapper.hxx>
#include <unotools/charclass.hxx>
#include <sal/types.h>
#include <i18npool/lang.h>
#include <rtl/textenc.h>

class SvtSysLocale_Impl;
class SvtSysLocaleOptions;

namespace osl { class Mutex; }
class LocaleDataWrapper;

/**
    SvtSysLocale provides a refcounted single instance of an application wide
    <type>LocaleDataWrapper</type> and <type>CharClass</type> which always
    follow the locale as it is currently configured by the user. You may use
    it anywhere to access the locale data elements like decimal separator and
    simple date formatting and so on. Contructing and destructing a
    SvtSysLocale is not expensive as long as there is at least one instance
    left.
 */
class UNOTOOLS_DLLPUBLIC SvtSysLocale
{
    friend class SvtSysLocale_Impl;     // access to mutex

    static  SvtSysLocale_Impl*  pImpl;
    static  sal_Int32           nRefCount;

    UNOTOOLS_DLLPRIVATE static  ::osl::Mutex&               GetMutex();

public:
                                        SvtSysLocale();
                                        ~SvtSysLocale();

            const LocaleDataWrapper&    GetLocaleData() const;
            const CharClass&            GetCharClass() const;

    /** It is safe to store the pointers locally and use them AS LONG AS THE
        INSTANCE OF SvtSysLocale LIVES!
        It is a faster access but be sure what you do!
     */
            const LocaleDataWrapper*    GetLocaleDataPtr() const;
            const CharClass*            GetCharClassPtr() const;
			SvtSysLocaleOptions&		GetOptions() const;
			com::sun::star::lang::Locale GetLocale() const;
			LanguageType				GetLanguage() const;
			com::sun::star::lang::Locale GetUILocale() const;
			LanguageType				GetUILanguage() const;

    /** Get the best MIME encoding matching the system locale, or if that isn't
        determinable one that matches the UI locale, or UTF8 if everything else
        fails.
     */
    static  rtl_TextEncoding    GetBestMimeEncoding();
};

#endif  // INCLUDED_SVTOOLS_SYSLOCALE_HXX
