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
#ifndef _ZIP_PACKAGE_FOLDER_ENUMERATION_HXX
#define _ZIP_PACKAGE_FOLDER_ENUMERATION_HXX

#include <cppuhelper/implbase2.hxx> // helper for implementations
#include <com/sun/star/container/XEnumeration.hpp>
#ifndef _COM_SUN_STAR_LANG_XPSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _HASH_MAPS_HXX
#include <HashMaps.hxx>
#endif

class ZipPackageFolderEnumeration : public cppu::WeakImplHelper2
< 
	com::sun::star::container::XEnumeration,
	com::sun::star::lang::XServiceInfo
>
{
protected:
	ContentHash& rContents;
	ContentHash::const_iterator aIterator;
public:
	//ZipPackageFolderEnumeration (std::hash_map < rtl::OUString, com::sun::star::uno::Reference < com::sun::star::container::XNamed >, hashFunc, eqFunc > &rInput);
	ZipPackageFolderEnumeration (ContentHash &rInput);
	virtual ~ZipPackageFolderEnumeration( void );

	// XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements(  ) 
		throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL nextElement(  ) 
		throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

	// XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) 
		throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) 
		throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) 
		throw (::com::sun::star::uno::RuntimeException);
	
};
#endif
