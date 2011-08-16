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

#include <precomp.h>
#include <x_parse2.hxx>

// NOT FULLY DECLARED SERVICES

	enum E_Type
	{
		x_Any						= 0,
		x_InvalidChar,
		x_UnexpectedEOF
	};
void
X_AutodocParser::GetInfo( std::ostream &     	o_rOutputMedium ) const
{
	switch (eType)
    {
		case x_Any:
			o_rOutputMedium << "Unspecified parsing exception ." << Endl();
			break;
		case x_InvalidChar:
			o_rOutputMedium << "Unknown character during parsing." << Endl();
			break;
		case x_UnexpectedToken:
			o_rOutputMedium << "Unexpected token " << sName << " found." << Endl();
			break;
		case x_UnexpectedEOF:
			o_rOutputMedium << "Unexpected end of file found." << Endl();
			break;
		default:
			o_rOutputMedium << "Unknown exception during parsing." << Endl();
	}
}


