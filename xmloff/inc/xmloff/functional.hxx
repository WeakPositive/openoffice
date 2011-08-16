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

#ifndef _XMLOFF_FUNCTIONAL_HXX
#define _XMLOFF_FUNCTIONAL_HXX

#include <rtl/ustring.hxx>

/* THIS HEADER IS DEPRECATED. USE comphelper/stl_types.hxx INSTEAD!!! */

/** @#file
 *
 * re-implement STL functors as needed
 *
 * The standard comparison operators from the STL cause warnings with
 * several compilers about our sal_Bool (=unsigned char) being
 * converted to bool (C++ bool). We wish to avoid that.
 */

struct less_functor
{
	bool operator()(const ::rtl::OUString& x, 
					const ::rtl::OUString& y) const
	{
		return 0 != (x<y);
	}
};


#endif
