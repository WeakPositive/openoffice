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
#ifndef _CHART2_VPOLAR_COORDINATESYSTEM_HXX
#define _CHART2_VPOLAR_COORDINATESYSTEM_HXX

#include "VCoordinateSystem.hxx"

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/
class VPolarCoordinateSystem : public VCoordinateSystem
{
public:
    VPolarCoordinateSystem( const ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XCoordinateSystem >& xCooSys );
    virtual ~VPolarCoordinateSystem();

    //better performance for big data
    virtual ::com::sun::star::uno::Sequence< sal_Int32 > getCoordinateSystemResolution( const ::com::sun::star::awt::Size& rPageSize
                                    , const ::com::sun::star::awt::Size& rPageResolution );

    virtual void createVAxisList(
            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier > & xNumberFormatsSupplier
            , const ::com::sun::star::awt::Size& rFontReferenceSize
            , const ::com::sun::star::awt::Rectangle& rMaximumSpaceForLabels );

    virtual void initVAxisInList();
    virtual void updateScalesAndIncrementsOnAxes();

    virtual void createGridShapes();

private:
    VPolarCoordinateSystem();
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
