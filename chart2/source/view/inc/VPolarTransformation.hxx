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
#ifndef _CHART_VPOLARTRANSFORMATION_HXX
#define _CHART_VPOLARTRANSFORMATION_HXX

#include "PlottingPositionHelper.hxx"
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/chart2/XTransformation.hpp>

namespace chart
{

class VPolarTransformation : public ::cppu::WeakImplHelper1<
    ::com::sun::star::chart2::XTransformation
    >
{
public:
    VPolarTransformation( const PolarPlottingPositionHelper& rPositionHelper );
	virtual ~VPolarTransformation();

    // ____ XTransformation ____
    /// @see ::com::sun::star::chart2::XTransformation
    virtual ::com::sun::star::uno::Sequence< double > SAL_CALL transform(
        const ::com::sun::star::uno::Sequence< double >& rSourceValues )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);
    /// @see ::com::sun::star::chart2::XTransformation
    virtual sal_Int32 SAL_CALL getSourceDimension()
        throw (::com::sun::star::uno::RuntimeException);
    /// @see ::com::sun::star::chart2::XTransformation
    virtual sal_Int32 SAL_CALL getTargetDimension()
        throw (::com::sun::star::uno::RuntimeException);

private:
    PolarPlottingPositionHelper     m_aPositionHelper;
    ::basegfx::B3DHomMatrix         m_aUnitCartesianToScene;
};

}  // namespace chart

// _CHART_VPOLARTRANSFORMATION_HXX
#endif

