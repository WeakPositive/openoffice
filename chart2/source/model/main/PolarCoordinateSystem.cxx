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
#include "precompiled_chart2.hxx"
#include "PolarCoordinateSystem.hxx"
#include "macros.hxx"
#include "servicenames_coosystems.hxx"

using namespace ::com::sun::star;

using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

namespace
{

static const ::rtl::OUString lcl_aServiceNamePolar2d(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart2.PolarCoordinateSystem2d" ));
static const ::rtl::OUString lcl_aServiceNamePolar3d(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart2.PolarCoordinateSystem3d" ));

static const ::rtl::OUString lcl_aImplementationNamePolar2d(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart2.PolarCoordinateSystem2d" ));
static const ::rtl::OUString lcl_aImplementationNamePolar3d(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart2.PolarCoordinateSystem3d" ));
}

namespace chart
{

// explicit
PolarCoordinateSystem::PolarCoordinateSystem(
    const uno::Reference< uno::XComponentContext > & xContext,
    sal_Int32 nDimensionCount /* = 2 */,
    sal_Bool bSwapXAndYAxis /* = sal_False */ ) :
        BaseCoordinateSystem( xContext, nDimensionCount, bSwapXAndYAxis )
{}

PolarCoordinateSystem::PolarCoordinateSystem(
    const PolarCoordinateSystem & rSource ) :
        BaseCoordinateSystem( rSource )
{}

PolarCoordinateSystem::~PolarCoordinateSystem()
{}

// ____ XCoordinateSystem ____
::rtl::OUString SAL_CALL PolarCoordinateSystem::getCoordinateSystemType()
    throw (RuntimeException)
{
    return CHART2_COOSYSTEM_POLAR_SERVICE_NAME;
}

::rtl::OUString SAL_CALL PolarCoordinateSystem::getViewServiceName()
    throw (RuntimeException)
{
    return CHART2_COOSYSTEM_POLAR_VIEW_SERVICE_NAME;
}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL PolarCoordinateSystem::createClone()
    throw (RuntimeException)
{
    return Reference< util::XCloneable >( new PolarCoordinateSystem( *this ));
}

// ____ XServiceInfo ____
Sequence< OUString > PolarCoordinateSystem::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 1 );
    aServices[ 0 ] = CHART2_COOSYSTEM_POLAR_SERVICE_NAME;
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( PolarCoordinateSystem,
                             C2U( "com.sun.star.comp.chart.PolarCoordinateSystem" ))


// =================================
// ==== PolarCoordinateSystem2d ====
// =================================

PolarCoordinateSystem2d::PolarCoordinateSystem2d(
    const uno::Reference< uno::XComponentContext > & xContext ) :
        PolarCoordinateSystem( xContext, 2, sal_False )
{}

PolarCoordinateSystem2d::~PolarCoordinateSystem2d()
{}

// ____ XServiceInfo ____
Sequence< OUString > PolarCoordinateSystem2d::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 2 );
    aServices[ 0 ] = CHART2_COOSYSTEM_POLAR_SERVICE_NAME;
    aServices[ 1 ] = lcl_aServiceNamePolar2d;
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( PolarCoordinateSystem2d, lcl_aImplementationNamePolar2d )

// =================================
// ==== PolarCoordinateSystem3d ====
// =================================

PolarCoordinateSystem3d::PolarCoordinateSystem3d(
    const uno::Reference< uno::XComponentContext > & xContext ) :
        PolarCoordinateSystem( xContext, 3, sal_False )
{}

PolarCoordinateSystem3d::~PolarCoordinateSystem3d()
{}

// ____ XServiceInfo ____
Sequence< OUString > PolarCoordinateSystem3d::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 2 );
    aServices[ 0 ] = CHART2_COOSYSTEM_POLAR_SERVICE_NAME;
    aServices[ 1 ] = lcl_aServiceNamePolar3d;
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( PolarCoordinateSystem3d, lcl_aImplementationNamePolar3d )

}  // namespace chart
