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

#ifndef SVX_DBREGISTERSETTING_HXX
#define SVX_DBREGISTERSETTING_HXX

#include <comphelper/stl_types.hxx>
#include <svl/poolitem.hxx>

//........................................................................
namespace svx
{
//........................................................................

    struct DatabaseRegistration
    {
        ::rtl::OUString sLocation;
        bool            bReadOnly;

        DatabaseRegistration()
            :sLocation()
            ,bReadOnly( true )
        {
        }

        DatabaseRegistration( const ::rtl::OUString& _rLocation, const sal_Bool _bReadOnly )
            :sLocation( _rLocation )
            ,bReadOnly( _bReadOnly )
        {
        }

        bool operator==( const DatabaseRegistration& _rhs ) const
        {
            return  ( sLocation == _rhs.sLocation );
                // do not take the read-only-ness into account, this is not maintained everywhere, but only
                // properly set when filling the struct from the XDatabaseRegistrations data
        }

        bool operator!=( const DatabaseRegistration& _rhs ) const
        {
            return !( this->operator==( _rhs ) );
        }
    };

    typedef ::std::map< ::rtl::OUString, DatabaseRegistration, ::comphelper::UStringLess >   DatabaseRegistrations;

	//====================================================================
	//= DatabaseMapItem
	//====================================================================
	class DatabaseMapItem : public SfxPoolItem
	{
	protected:		
		DatabaseRegistrations   m_aRegistrations;

	public:
		TYPEINFO();

		DatabaseMapItem( sal_uInt16 _nId, const DatabaseRegistrations& _rRegistrations );

		virtual int              operator==( const SfxPoolItem& ) const;
		virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;

		const DatabaseRegistrations&
                                getRegistrations() const { return m_aRegistrations; }
	};

//........................................................................
}	// namespace svx
//........................................................................
#endif // SVX_DBREGISTERSETTING_HXX

