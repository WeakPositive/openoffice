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

#ifndef _CHART2_CANDLESTICKCHART_HXX
#define _CHART2_CANDLESTICKCHART_HXX

#include "VSeriesPlotter.hxx"

//.............................................................................
namespace chart
{
//.............................................................................
class BarPositionHelper;

class CandleStickChart : public VSeriesPlotter
{
	//-------------------------------------------------------------------------
	// public methods
	//-------------------------------------------------------------------------
public:
	CandleStickChart( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::chart2::XChartType >& xChartTypeModel
                      , sal_Int32 nDimensionCount );
	virtual ~CandleStickChart();

	virtual void createShapes();
    virtual void addSeries( VDataSeries* pSeries, sal_Int32 zSlot = -1, sal_Int32 xSlot = -1,sal_Int32 ySlot = -1 );

    virtual ::com::sun::star::drawing::Direction3D  getPreferredDiagramAspectRatio() const;

    //-------------------------------------------------------------------------
	// MinimumAndMaximumSupplier
	//-------------------------------------------------------------------------
    virtual bool isSeperateStackingForDifferentSigns( sal_Int32 nDimensionIndex );

    //-------------------------------------------------------------------------

    virtual LegendSymbolStyle getLegendSymbolStyle();

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------

private: //methods
	//no default constructor
	CandleStickChart();

private: //member
    BarPositionHelper*                   m_pMainPosHelper;
};
//.............................................................................
} //namespace chart
//.............................................................................
#endif
