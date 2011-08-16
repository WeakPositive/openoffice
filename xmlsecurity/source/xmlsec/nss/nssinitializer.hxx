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

#ifndef _NSSINITIALIZER_HXX
#define _NSSINITIALIZER_HXX

#include <com/sun/star/xml/crypto/XDigestContextSupplier.hpp>
#include <com/sun/star/xml/crypto/XCipherContextSupplier.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <cppuhelper/implbase3.hxx>

#define NSS_SERVICE_NAME "com.sun.star.xml.crypto.NSSInitializer"

class ONSSInitializer : public cppu::WeakImplHelper3
<
    ::com::sun::star::xml::crypto::XDigestContextSupplier,
    ::com::sun::star::xml::crypto::XCipherContextSupplier,
    ::com::sun::star::lang::XServiceInfo
>
{
protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > mxMSF;
 
    ONSSInitializer()
    {}

public:
    ONSSInitializer( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > &rxMSF );
    virtual ~ONSSInitializer();

    bool initNSS( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > &xMSF );

    /* XDigestContextSupplier */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XDigestContext > SAL_CALL getDigestContext( ::sal_Int32 nDigestID, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& aParams ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    /* XCipherContextSupplier */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XCipherContext > SAL_CALL getCipherContext( ::sal_Int32 nCipherID, const ::com::sun::star::uno::Sequence< ::sal_Int8 >& aKey, const ::com::sun::star::uno::Sequence< ::sal_Int8 >& aInitializationVector, ::sal_Bool bEncryption, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& aParams ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    /* XServiceInfo */
    virtual rtl::OUString SAL_CALL getImplementationName()
        throw (::com::sun::star::uno::RuntimeException);
        
    virtual sal_Bool SAL_CALL supportsService( const rtl::OUString& ServiceName )
        throw (::com::sun::star::uno::RuntimeException);
        
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames()
        throw (::com::sun::star::uno::RuntimeException);
};

rtl::OUString ONSSInitializer_getImplementationName()
    throw ( ::com::sun::star::uno::RuntimeException );

sal_Bool SAL_CALL ONSSInitializer_supportsService( const rtl::OUString& ServiceName )
    throw ( ::com::sun::star::uno::RuntimeException );

com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL ONSSInitializer_getSupportedServiceNames()
    throw ( ::com::sun::star::uno::RuntimeException );

com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
SAL_CALL ONSSInitializer_createInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & rSMgr )
    throw ( ::com::sun::star::uno::Exception );

#endif

