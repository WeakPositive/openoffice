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

#include "DialogModel.hxx"
#include "RangeSelectionHelper.hxx"
#include "PropertyHelper.hxx"
#include "DataSeriesHelper.hxx"
#include "DataSourceHelper.hxx"
#include "DiagramHelper.hxx"
#include "macros.hxx"
#include "Strings.hrc"
#include "ResId.hxx"
#include "ContainerHelper.hxx"
#include "CommonFunctors.hxx"
#include "ControllerLockGuard.hxx"
#include "ChartTypeHelper.hxx"
#include "ThreeDHelper.hxx"

#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/chart2/XTitled.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/data/XDataSink.hpp>

#include <tools/string.hxx>

#include <utility>
#include <algorithm>
#include <iterator>
#include <functional>
#include <numeric>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using namespace ::chart::ContainerHelper;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

// ----------------------------------------

namespace
{
const OUString lcl_aLabelRole( RTL_CONSTASCII_USTRINGPARAM( "label" ));

struct lcl_ChartTypeToSeriesCnt : ::std::unary_function<
        Reference< XChartType >, Reference< XDataSeriesContainer > >
{
    Reference< XDataSeriesContainer > operator() (
        const Reference< XChartType > & xChartType )
    {
        return Reference< XDataSeriesContainer >::query( xChartType );
    }
};

OUString lcl_ConvertRole( const OUString & rRoleString, bool bFromInternalToUI )
{
    OUString aResult( rRoleString );

    typedef ::std::map< OUString, OUString > tTranslationMap;
    static tTranslationMap aTranslationMap;

    if( aTranslationMap.size() == 0 )
    {
        aTranslationMap[ C2U( "categories" ) ] =   OUString( String( ::chart::SchResId( STR_DATA_ROLE_CATEGORIES )));
        aTranslationMap[ C2U( "error-bars-x" ) ] = OUString( String( ::chart::SchResId( STR_DATA_ROLE_X_ERROR )));
        aTranslationMap[ C2U( "error-bars-x-positive" ) ] = OUString( String( ::chart::SchResId( STR_DATA_ROLE_X_ERROR_POSITIVE )));
        aTranslationMap[ C2U( "error-bars-x-negative" ) ] = OUString( String( ::chart::SchResId( STR_DATA_ROLE_X_ERROR_NEGATIVE )));
        aTranslationMap[ C2U( "error-bars-y" ) ] = OUString( String( ::chart::SchResId( STR_DATA_ROLE_Y_ERROR )));
        aTranslationMap[ C2U( "error-bars-y-positive" ) ] = OUString( String( ::chart::SchResId( STR_DATA_ROLE_Y_ERROR_POSITIVE )));
        aTranslationMap[ C2U( "error-bars-y-negative" ) ] = OUString( String( ::chart::SchResId( STR_DATA_ROLE_Y_ERROR_NEGATIVE )));
        aTranslationMap[ C2U( "label" ) ] =        OUString( String( ::chart::SchResId( STR_DATA_ROLE_LABEL )));
        aTranslationMap[ C2U( "values-first" ) ] = OUString( String( ::chart::SchResId( STR_DATA_ROLE_FIRST )));
        aTranslationMap[ C2U( "values-last" ) ] =  OUString( String( ::chart::SchResId( STR_DATA_ROLE_LAST )));
        aTranslationMap[ C2U( "values-max" ) ] =   OUString( String( ::chart::SchResId( STR_DATA_ROLE_MAX )));
        aTranslationMap[ C2U( "values-min" ) ] =   OUString( String( ::chart::SchResId( STR_DATA_ROLE_MIN )));
        aTranslationMap[ C2U( "values-x" ) ] =     OUString( String( ::chart::SchResId( STR_DATA_ROLE_X )));
        aTranslationMap[ C2U( "values-y" ) ] =     OUString( String( ::chart::SchResId( STR_DATA_ROLE_Y )));
        aTranslationMap[ C2U( "values-size" ) ] =  OUString( String( ::chart::SchResId( STR_DATA_ROLE_SIZE )));
    }

    if( bFromInternalToUI )
    {
        tTranslationMap::const_iterator aIt( aTranslationMap.find( rRoleString ));
        if( aIt != aTranslationMap.end())
        {
            aResult = (*aIt).second;
        }
    }
    else
    {
        tTranslationMap::const_iterator aIt(
            ::std::find_if( aTranslationMap.begin(), aTranslationMap.end(),
                            ::std::compose1( ::std::bind2nd(
                                                 ::std::equal_to< tTranslationMap::mapped_type >(),
                                                 rRoleString ),
                                             ::std::select2nd< tTranslationMap::value_type >())));

        if( aIt != aTranslationMap.end())
            aResult = (*aIt).first;
    }

    return aResult;
}

typedef ::std::map< ::rtl::OUString, sal_Int32 > lcl_tRoleIndexMap;

void lcl_createRoleIndexMap( lcl_tRoleIndexMap & rOutMap )
{
    rOutMap.clear();
    sal_Int32 nIndex = 0;

    rOutMap[ C2U( "label" ) ] =                 ++nIndex;
    rOutMap[ C2U( "categories" ) ] =            ++nIndex;
    rOutMap[ C2U( "values-x" ) ] =              ++nIndex;
    rOutMap[ C2U( "values-y" ) ] =              ++nIndex;
    rOutMap[ C2U( "error-bars-x" ) ] =          ++nIndex;
    rOutMap[ C2U( "error-bars-x-positive" ) ] = ++nIndex;
    rOutMap[ C2U( "error-bars-x-negative" ) ] = ++nIndex;
    rOutMap[ C2U( "error-bars-y" ) ] =          ++nIndex;
    rOutMap[ C2U( "error-bars-y-positive" ) ] = ++nIndex;
    rOutMap[ C2U( "error-bars-y-negative" ) ] = ++nIndex;
    rOutMap[ C2U( "values-first" ) ] =          ++nIndex;
    rOutMap[ C2U( "values-min" ) ] =            ++nIndex;
    rOutMap[ C2U( "values-max" ) ] =            ++nIndex;
    rOutMap[ C2U( "values-last" ) ] =           ++nIndex;
    rOutMap[ C2U( "values-size" ) ] =           ++nIndex;
}

struct lcl_DataSeriesContainerAppend : public
    ::std::iterator< ::std::output_iterator_tag, Reference< XDataSeriesContainer > >
{
    typedef ::std::vector< ::chart::DialogModel::tSeriesWithChartTypeByName > tContainerType;

    explicit lcl_DataSeriesContainerAppend( tContainerType & rCnt )
            : m_rDestCnt( rCnt )
    {}

    lcl_DataSeriesContainerAppend & operator= ( const value_type & xVal )
    {
        try
        {
            if( xVal.is())
            {
                Sequence< Reference< XDataSeries > > aSeq( xVal->getDataSeries());
                OUString aRole( RTL_CONSTASCII_USTRINGPARAM("values-y"));
                Reference< XChartType > xCT( xVal, uno::UNO_QUERY );
                if( xCT.is())
                    aRole = xCT->getRoleOfSequenceForSeriesLabel();
                for( sal_Int32 nI = 0; nI < aSeq.getLength(); ++ nI )
                {
                    m_rDestCnt.push_back(
                        ::chart::DialogModel::tSeriesWithChartTypeByName(
                            ::chart::DataSeriesHelper::getDataSeriesLabel( aSeq[nI], aRole ),
                            ::std::make_pair( aSeq[nI], xCT )));
                }
            }
        }
        catch( uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
        return *this;
    }

    lcl_DataSeriesContainerAppend & operator* ()     { return *this; }
    lcl_DataSeriesContainerAppend & operator++ ()    { return *this; }
    lcl_DataSeriesContainerAppend & operator++ (int) { return *this; }

private:
    tContainerType & m_rDestCnt;
};

struct lcl_RolesWithRangeAppend : public
    ::std::iterator< ::std::output_iterator_tag, Reference< data::XLabeledDataSequence > >
{
    typedef ::chart::DialogModel::tRolesWithRanges tContainerType;

    explicit lcl_RolesWithRangeAppend( tContainerType & rCnt,
                                       const ::rtl::OUString & aLabelRole )
            : m_rDestCnt( rCnt ),
              m_aRoleForLabelSeq( aLabelRole )
    {}

    lcl_RolesWithRangeAppend & operator= ( const value_type & xVal )
    {
        try
        {
            if( xVal.is())
            {
                // data sequence
                Reference< data::XDataSequence > xSeq( xVal->getValues());
                if( xSeq.is())
                {
                    OUString aRole;
                    Reference< beans::XPropertySet > xProp( xSeq, uno::UNO_QUERY_THROW );
                    if( xProp->getPropertyValue( C2U("Role")) >>= aRole )
                    {
                        m_rDestCnt.insert(
                            tContainerType::value_type(
                                aRole, xSeq->getSourceRangeRepresentation()));
                        // label
                        if( aRole.equals( m_aRoleForLabelSeq ))
                        {
                            Reference< data::XDataSequence > xLabelSeq( xVal->getLabel());
                            if( xLabelSeq.is())
                            {
                                m_rDestCnt.insert(
                                    tContainerType::value_type(
                                        lcl_aLabelRole, xLabelSeq->getSourceRangeRepresentation()));
                            }
                        }
                    }
                }
            }
        }
        catch( uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
        return *this;
    }

    lcl_RolesWithRangeAppend & operator* ()     { return *this; }
    lcl_RolesWithRangeAppend & operator++ ()    { return *this; }
    lcl_RolesWithRangeAppend & operator++ (int) { return *this; }

private:
    tContainerType & m_rDestCnt;
    OUString m_aRoleForLabelSeq;
};

void lcl_SetSequenceRole(
    const Reference< data::XDataSequence > & xSeq,
    const OUString & rRole )
{
    Reference< beans::XPropertySet > xProp( xSeq, uno::UNO_QUERY );
    if( xProp.is())
        xProp->setPropertyValue( C2U("Role"), uno::makeAny( rRole ));
}

Reference< XDataSeries > lcl_CreateNewSeries(
    const Reference< uno::XComponentContext > & xContext,
    const Reference< XChartType > & xChartType,
    sal_Int32 nNewSeriesIndex,
    sal_Int32 nTotalNumberOfSeriesInCTGroup,
    const Reference< XDiagram > & xDiagram,
    const Reference< XChartTypeTemplate > & xTemplate,
    bool bCreateDataCachedSequences )
{
    // create plain series
    Reference< XDataSeries > xResult(
        xContext->getServiceManager()->createInstanceWithContext(
            C2U( "com.sun.star.chart2.DataSeries" ),
            xContext ), uno::UNO_QUERY );
    if( xTemplate.is())
    {
        Reference< beans::XPropertySet > xResultProp( xResult, uno::UNO_QUERY );
        if( xResultProp.is())
        {
            // @deprecated: correct default color should be found by view
            // without setting it as hard attribute
            Reference< XColorScheme > xColorScheme( xDiagram->getDefaultColorScheme());
            if( xColorScheme.is())
                xResultProp->setPropertyValue(
                    C2U("Color"), uno::makeAny( xColorScheme->getColorByIndex( nNewSeriesIndex )));
        }
        sal_Int32 nGroupIndex=0;
        if( xChartType.is())
        {
            Sequence< Reference< XChartType > > aCTs(
                ::chart::DiagramHelper::getChartTypesFromDiagram( xDiagram ));
            for( ; nGroupIndex<aCTs.getLength(); ++nGroupIndex)
                if( aCTs[nGroupIndex] == xChartType )
                    break;
            if( nGroupIndex == aCTs.getLength())
                nGroupIndex = 0;
        }
        xTemplate->applyStyle( xResult, nGroupIndex, nNewSeriesIndex, nTotalNumberOfSeriesInCTGroup );
    }

    if( bCreateDataCachedSequences )
    {
        // set chart type specific roles
        Reference< data::XDataSink > xSink( xResult, uno::UNO_QUERY );
        if( xChartType.is() && xSink.is())
        {
            ::std::vector< Reference< data::XLabeledDataSequence > > aNewSequences;
            const OUString aRoleOfSeqForSeriesLabel = xChartType->getRoleOfSequenceForSeriesLabel();
            const OUString aLabel( String( ::chart::SchResId( STR_DATA_UNNAMED_SERIES )));
            const Sequence< OUString > aRoles( xChartType->getSupportedMandatoryRoles());
            const Sequence< OUString > aOptRoles( xChartType->getSupportedOptionalRoles());
            sal_Int32 nI = 0;

            for(nI=0; nI<aRoles.getLength(); ++nI)
            {
                if( aRoles[nI].equals( lcl_aLabelRole ))
                    continue;
                Reference< data::XDataSequence > xSeq( ::chart::DataSourceHelper::createCachedDataSequence() );
                lcl_SetSequenceRole( xSeq, aRoles[nI] );
                // assert that aRoleOfSeqForSeriesLabel is part of the mandatory roles
                if( aRoles[nI].equals( aRoleOfSeqForSeriesLabel ))
                {
                    Reference< data::XDataSequence > xLabel( ::chart::DataSourceHelper::createCachedDataSequence( aLabel ));
                    lcl_SetSequenceRole( xLabel, lcl_aLabelRole );
                    aNewSequences.push_back( ::chart::DataSourceHelper::createLabeledDataSequence( xSeq, xLabel ));
                }
                else
                    aNewSequences.push_back( ::chart::DataSourceHelper::createLabeledDataSequence( xSeq ));
            }

            for(nI=0; nI<aOptRoles.getLength(); ++nI)
            {
                if( aOptRoles[nI].equals( lcl_aLabelRole ))
                    continue;
                Reference< data::XDataSequence > xSeq( ::chart::DataSourceHelper::createCachedDataSequence());
                lcl_SetSequenceRole( xSeq, aOptRoles[nI] );
                aNewSequences.push_back( ::chart::DataSourceHelper::createLabeledDataSequence( xSeq ));
            }

            xSink->setData( ContainerToSequence( aNewSequences ));
        }
    }

    return xResult;
}

struct lcl_addSeriesNumber : public ::std::binary_function<
        sal_Int32, Reference< XDataSeriesContainer >, sal_Int32 >
{
    sal_Int32 operator() ( sal_Int32 nCurrentNumber, const Reference< XDataSeriesContainer > & xCnt ) const
    {
        if( xCnt.is())
            return nCurrentNumber + (xCnt->getDataSeries().getLength());
        return nCurrentNumber;
    }
};

} // anonymous namespace

// ----------------------------------------


namespace chart
{

DialogModel::DialogModel(
    const Reference< XChartDocument > & xChartDocument,
    const Reference< uno::XComponentContext > & xContext ) :
        m_xChartDocument( xChartDocument ),
        m_xContext( xContext ),
        m_aTimerTriggeredControllerLock( uno::Reference< frame::XModel >( m_xChartDocument, uno::UNO_QUERY ) )
{
}

DialogModel::~DialogModel()
{}

void DialogModel::setTemplate(
    const Reference< XChartTypeTemplate > & xTemplate )
{
    m_xTemplate = xTemplate;
}

::boost::shared_ptr< RangeSelectionHelper >
    DialogModel::getRangeSelectionHelper() const
{
    if( ! m_spRangeSelectionHelper.get())
        m_spRangeSelectionHelper.reset(
            new RangeSelectionHelper( m_xChartDocument ));

    return m_spRangeSelectionHelper;
}

Reference< frame::XModel > DialogModel::getChartModel() const
{
    Reference< frame::XModel > xResult( m_xChartDocument, uno::UNO_QUERY );
    return xResult;
}

Reference< data::XDataProvider > DialogModel::getDataProvider() const
{
    Reference< data::XDataProvider > xResult;
    if( m_xChartDocument.is())
        xResult.set( m_xChartDocument->getDataProvider());
    return xResult;
}

::std::vector< Reference< XDataSeriesContainer > >
    DialogModel::getAllDataSeriesContainers() const
{
    ::std::vector< Reference< XDataSeriesContainer > > aResult;

    try
    {
        Reference< XDiagram > xDiagram;
        if( m_xChartDocument.is())
            xDiagram.set( m_xChartDocument->getFirstDiagram());
        if( xDiagram.is())
        {
            Reference< XCoordinateSystemContainer > xCooSysCnt(
                xDiagram, uno::UNO_QUERY_THROW );
            Sequence< Reference< XCoordinateSystem > > aCooSysSeq(
                xCooSysCnt->getCoordinateSystems());
            for( sal_Int32 i=0; i<aCooSysSeq.getLength(); ++i )
            {
                Reference< XChartTypeContainer > xCTCnt( aCooSysSeq[i], uno::UNO_QUERY_THROW );
                Sequence< Reference< XChartType > > aChartTypeSeq( xCTCnt->getChartTypes());
                ::std::transform(
                    aChartTypeSeq.getConstArray(), aChartTypeSeq.getConstArray() + aChartTypeSeq.getLength(),
                    ::std::back_inserter( aResult ),
                    lcl_ChartTypeToSeriesCnt() );
            }
        }
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return aResult;
}

::std::vector< DialogModel::tSeriesWithChartTypeByName >
    DialogModel::getAllDataSeriesWithLabel() const
{
    ::std::vector< tSeriesWithChartTypeByName > aResult;
    ::std::vector< Reference< XDataSeriesContainer > > aContainers(
        getAllDataSeriesContainers());

    ::std::copy( aContainers.begin(), aContainers.end(),
                 lcl_DataSeriesContainerAppend( aResult ));
    return aResult;
}

DialogModel::tRolesWithRanges DialogModel::getRolesWithRanges(
    const Reference< XDataSeries > & xSeries,
    const ::rtl::OUString & aRoleOfSequenceForLabel,
    const Reference< chart2::XChartType > & xChartType ) const
{
    DialogModel::tRolesWithRanges aResult;
    try
    {
        Reference< data::XDataSource > xSource( xSeries, uno::UNO_QUERY_THROW );
        const Sequence< Reference< data::XLabeledDataSequence > > aSeq( xSource->getDataSequences());
        ::std::copy( aSeq.getConstArray(), aSeq.getConstArray() + aSeq.getLength(),
                     lcl_RolesWithRangeAppend( aResult, aRoleOfSequenceForLabel ));
        if( xChartType.is())
        {
            // add missing mandatory roles
            Sequence< OUString > aRoles( xChartType->getSupportedMandatoryRoles());
            OUString aEmptyString;
            sal_Int32 nI = 0;
            for( nI=0; nI < aRoles.getLength(); ++nI )
            {
                if( aResult.find( aRoles[nI] ) == aResult.end() )
                    aResult.insert( DialogModel::tRolesWithRanges::value_type( aRoles[nI], aEmptyString ));
            }

            // add missing optional roles
            aRoles = xChartType->getSupportedOptionalRoles();
            for( nI=0; nI < aRoles.getLength(); ++nI )
            {
                if( aResult.find( aRoles[nI] ) == aResult.end() )
                    aResult.insert( DialogModel::tRolesWithRanges::value_type( aRoles[nI], aEmptyString ));
            }
        }
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
    return aResult;
}

void DialogModel::moveSeries(
    const Reference< XDataSeries > & xSeries,
    eMoveDirection eDirection )
{
    m_aTimerTriggeredControllerLock.startTimer();
    ControllerLockGuard aLockedControllers( Reference< frame::XModel >( m_xChartDocument, uno::UNO_QUERY ) );

    Reference< XDiagram > xDiagram( m_xChartDocument->getFirstDiagram());
    DiagramHelper::moveSeries( xDiagram, xSeries, eDirection==MOVE_UP );
}

Reference< chart2::XDataSeries > DialogModel::insertSeriesAfter(
    const Reference< XDataSeries > & xSeries,
    const Reference< XChartType > & xChartType,
    bool bCreateDataCachedSequences /* = false */ )
{
    m_aTimerTriggeredControllerLock.startTimer();
    ControllerLockGuard aLockedControllers( Reference< frame::XModel >( m_xChartDocument, uno::UNO_QUERY ) );
    Reference< XDataSeries > xNewSeries;

    try
    {
        Reference< chart2::XDiagram > xDiagram( m_xChartDocument->getFirstDiagram() );
        ThreeDLookScheme e3DScheme = ThreeDHelper::detectScheme( xDiagram );

        sal_Int32 nSeriesInChartType = 0;
        const sal_Int32 nTotalSeries = countSeries();
        if( xChartType.is())
        {
            Reference< XDataSeriesContainer > xCnt( xChartType, uno::UNO_QUERY_THROW );
            nSeriesInChartType = xCnt->getDataSeries().getLength();
        }

        // create new series
        xNewSeries.set(
            lcl_CreateNewSeries(
                m_xContext,
                xChartType,
                nTotalSeries, // new series' index
                nSeriesInChartType,
                xDiagram,
                m_xTemplate,
                bCreateDataCachedSequences ));

        // add new series to container
        if( xNewSeries.is())
        {
            Reference< XDataSeriesContainer > xSeriesCnt( xChartType, uno::UNO_QUERY_THROW );
            ::std::vector< Reference< XDataSeries > > aSeries(
                SequenceToVector( xSeriesCnt->getDataSeries()));
            ::std::vector< Reference< XDataSeries > >::iterator aIt =
                  ::std::find( aSeries.begin(), aSeries.end(), xSeries );
            if( aIt == aSeries.end())
                // if we have no series we insert at the first position.
                aIt = aSeries.begin();
            else
                // vector::insert inserts before, so we have to advance
                ++aIt;
            aSeries.insert( aIt, xNewSeries );
            xSeriesCnt->setDataSeries( ContainerToSequence( aSeries ));
        }

        ThreeDHelper::setScheme( xDiagram, e3DScheme );
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
    return xNewSeries;
}

void DialogModel::deleteSeries(
    const Reference< XDataSeries > & xSeries,
    const Reference< XChartType > & xChartType )
{
    m_aTimerTriggeredControllerLock.startTimer();
    ControllerLockGuard aLockedControllers( Reference< frame::XModel >( m_xChartDocument, uno::UNO_QUERY ) );

    DataSeriesHelper::deleteSeries( xSeries, xChartType );
}

Reference< data::XLabeledDataSequence > DialogModel::getCategories() const
{
    Reference< data::XLabeledDataSequence > xResult;
    try
    {
        if( m_xChartDocument.is())
        {
            Reference< chart2::XDiagram > xDiagram( m_xChartDocument->getFirstDiagram());
            xResult.set( DiagramHelper::getCategoriesFromDiagram( xDiagram ));
        }
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
    return xResult;
}

void DialogModel::setCategories( const Reference< chart2::data::XLabeledDataSequence > & xCategories )
{
    if( m_xChartDocument.is())
    {
        Reference< chart2::XDiagram > xDiagram( m_xChartDocument->getFirstDiagram());
        if( xDiagram.is())
        {
            // categories
            bool bSupportsCategories = true;

            Reference< XChartType > xFirstChartType( DiagramHelper::getChartTypeByIndex( xDiagram, 0 ) );
            if( xFirstChartType.is() )
            {
                sal_Int32 nAxisType = ChartTypeHelper::getAxisType( xFirstChartType, 0 ); // x-axis
                bSupportsCategories = (nAxisType == AxisType::CATEGORY);
            }
            DiagramHelper::setCategoriesToDiagram( xCategories, xDiagram, true, bSupportsCategories );
        }
    }
}

OUString DialogModel::getCategoriesRange() const
{
    Reference< data::XLabeledDataSequence > xLSeq( getCategories());
    OUString aRange;
    if( xLSeq.is())
    {
        Reference< data::XDataSequence > xSeq( xLSeq->getValues());
        if( xSeq.is())
            aRange = xSeq->getSourceRangeRepresentation();
    }
    return aRange;
}

bool DialogModel::isCategoryDiagram() const
{
    bool bRet = false;
    if( m_xChartDocument.is())
        bRet = DiagramHelper::isCategoryDiagram( m_xChartDocument->getFirstDiagram() );
    return bRet;
}

void DialogModel::detectArguments(
    OUString & rOutRangeString,
    bool & rOutUseColumns,
    bool & rOutFirstCellAsLabel,
    bool & rOutHasCategories ) const
{
    try
    {
        uno::Sequence< sal_Int32 > aSequenceMapping;//todo YYYX

        // Note: unused data is currently not supported in being passed to detectRangeSegmentation
        if( m_xChartDocument.is())
            DataSourceHelper::detectRangeSegmentation(
                Reference< frame::XModel >( m_xChartDocument, uno::UNO_QUERY_THROW ),
                rOutRangeString, aSequenceMapping, rOutUseColumns, rOutFirstCellAsLabel, rOutHasCategories );
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

bool DialogModel::allArgumentsForRectRangeDetected() const
{
    return DataSourceHelper::allArgumentsForRectRangeDetected( m_xChartDocument );
}

void DialogModel::startControllerLockTimer()
{
    m_aTimerTriggeredControllerLock.startTimer();
}

bool DialogModel::setData(
    const Sequence< beans::PropertyValue > & rArguments )
{
    m_aTimerTriggeredControllerLock.startTimer();
    ControllerLockGuard aLockedControllers( Reference< frame::XModel >( m_xChartDocument, uno::UNO_QUERY ) );

    Reference< data::XDataProvider > xDataProvider( getDataProvider());
    if( ! xDataProvider.is() ||
        ! m_xTemplate.is() )
    {
        OSL_ENSURE( false, "Model objects missing" );
        return false;
    }

    try
    {
        Reference< chart2::data::XDataSource > xDataSource(
            xDataProvider->createDataSource( rArguments ) );

        Reference< chart2::XDataInterpreter > xInterpreter(
            m_xTemplate->getDataInterpreter());
        if( xInterpreter.is())
        {
            Reference< chart2::XDiagram > xDiagram( m_xChartDocument->getFirstDiagram() );
            ThreeDLookScheme e3DScheme = ThreeDHelper::detectScheme( xDiagram );

            ::std::vector< Reference< XDataSeries > > aSeriesToReUse(
                DiagramHelper::getDataSeriesFromDiagram( xDiagram ));
            applyInterpretedData(
                xInterpreter->interpretDataSource(
                    xDataSource, rArguments,
                    ContainerToSequence( aSeriesToReUse )),
                aSeriesToReUse,
                true /* bSetStyles */);

            ThreeDHelper::setScheme( xDiagram, e3DScheme );
        }
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
        return false;
    }

    return true;
}

OUString DialogModel::ConvertRoleFromInternalToUI( const OUString & rRoleString )
{
    return lcl_ConvertRole( rRoleString, true );
}

OUString DialogModel::GetRoleDataLabel()
{
    return OUString( String( ::chart::SchResId( STR_OBJECT_DATALABELS )));
}

sal_Int32 DialogModel::GetRoleIndexForSorting( const ::rtl::OUString & rInternalRoleString )
{
    static lcl_tRoleIndexMap aRoleIndexMap;

    if( aRoleIndexMap.empty())
        lcl_createRoleIndexMap( aRoleIndexMap );

    lcl_tRoleIndexMap::const_iterator aIt( aRoleIndexMap.find( rInternalRoleString ));
    if( aIt != aRoleIndexMap.end())
        return aIt->second;

    return 0;
}

// private methods

void DialogModel::applyInterpretedData(
    const InterpretedData & rNewData,
    const ::std::vector< Reference< XDataSeries > > & rSeriesToReUse,
    bool bSetStyles )
{
    if( ! m_xChartDocument.is())
        return;

    m_aTimerTriggeredControllerLock.startTimer();
    Reference< XDiagram > xDiagram( m_xChartDocument->getFirstDiagram());
    if( xDiagram.is())
    {
        // styles
        if( bSetStyles && m_xTemplate.is() )
        {
            sal_Int32 nGroup = 0;
            sal_Int32 nSeriesCounter = 0;
            sal_Int32 nNewSeriesIndex = static_cast< sal_Int32 >( rSeriesToReUse.size());
            const sal_Int32 nOuterSize=rNewData.Series.getLength();

            for(; nGroup < nOuterSize; ++nGroup)
            {
                Sequence< Reference< XDataSeries > > aSeries( rNewData.Series[ nGroup ] );
                const sal_Int32 nSeriesInGroup = aSeries.getLength();
                for( sal_Int32 nSeries=0; nSeries<nSeriesInGroup; ++nSeries, ++nSeriesCounter )
                {
                    if( ::std::find( rSeriesToReUse.begin(), rSeriesToReUse.end(), aSeries[nSeries] )
                        == rSeriesToReUse.end())
                    {
                        Reference< beans::XPropertySet > xSeriesProp( aSeries[nSeries], uno::UNO_QUERY );
                        if( xSeriesProp.is())
                        {
                            // @deprecated: correct default color should be found by view
                            // without setting it as hard attribute
                            Reference< XColorScheme > xColorScheme( xDiagram->getDefaultColorScheme());
                            if( xColorScheme.is())
                                xSeriesProp->setPropertyValue(
                                    C2U("Color"), uno::makeAny( xColorScheme->getColorByIndex( nSeriesCounter )));
                        }
                        m_xTemplate->applyStyle( aSeries[nSeries], nGroup, nNewSeriesIndex++, nSeriesInGroup );
                    }
                }
            }
        }

        // data series
        ::std::vector< Reference< XDataSeriesContainer > > aSeriesCnt( getAllDataSeriesContainers());
        ::std::vector< Sequence< Reference< XDataSeries > > > aNewSeries(
            SequenceToVector( rNewData.Series ));

        OSL_ASSERT( aSeriesCnt.size() == aNewSeries.size());

        ::std::vector< Sequence< Reference< XDataSeries > > >::const_iterator aSrcIt( aNewSeries.begin());
        ::std::vector< Reference< XDataSeriesContainer > >::iterator aDestIt( aSeriesCnt.begin());
        for(; aSrcIt != aNewSeries.end() && aDestIt != aSeriesCnt.end();
            ++aSrcIt, ++aDestIt )
        {
            try
            {
                OSL_ASSERT( (*aDestIt).is());
                (*aDestIt)->setDataSeries( *aSrcIt );
            }
            catch( uno::Exception & ex )
            {
                ASSERT_EXCEPTION( ex );
            }
        }

        DialogModel::setCategories(rNewData.Categories);
    }
}

sal_Int32 DialogModel::countSeries() const
{
    ::std::vector< Reference< XDataSeriesContainer > > aCnt( getAllDataSeriesContainers());
    return ::std::accumulate( aCnt.begin(), aCnt.end(), 0, lcl_addSeriesNumber());
}

} //  namespace chart
