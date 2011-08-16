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

#ifndef _SDR_CONTACT_VIEWOBJECTCONTACTOFSDROBJ_HXX
#define _SDR_CONTACT_VIEWOBJECTCONTACTOFSDROBJ_HXX

#include <svx/sdr/contact/viewobjectcontact.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations
class SdrObject;

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
	namespace contact
	{
		class SVX_DLLPUBLIC ViewObjectContactOfSdrObj : public ViewObjectContact
		{
		protected:
			const SdrObject& getSdrObject() const;

		public:
			ViewObjectContactOfSdrObj(ObjectContact& rObjectContact, ViewContact& rViewContact);
			virtual ~ViewObjectContactOfSdrObj();

			virtual bool isPrimitiveVisible(const DisplayInfo& rDisplayInfo) const;
		};
	} // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_CONTACT_VIEWOBJECTCONTACTOFSDROBJ_HXX

// eof
