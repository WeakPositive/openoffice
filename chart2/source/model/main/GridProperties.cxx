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

#include "GridProperties.hxx"
#include "LineProperties.hxx"
#include "UserDefinedProperties.hxx"
#include "PropertyHelper.hxx"
#include "macros.hxx"
#include "ContainerHelper.hxx"
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/Sequence.hxx>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::Property;
using ::osl::MutexGuard;
using ::rtl::OUString;

// ____________________________________________________________

namespace
{

static const ::rtl::OUString lcl_aServiceName(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart2.GridProperties" ));

enum
{
    PROP_GRID_SHOW
};

void lcl_AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( C2U( "Show" ),
                  PROP_GRID_SHOW,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}

struct StaticGridDefaults_Initializer
{
    ::chart::tPropertyValueMap* operator()()
    {
        static ::chart::tPropertyValueMap aStaticDefaults;
        lcl_AddDefaultsToMap( aStaticDefaults );
        return &aStaticDefaults;
    }
private:
    void lcl_AddDefaultsToMap( ::chart::tPropertyValueMap & rOutMap )
    {
        ::chart::LineProperties::AddDefaultsToMap( rOutMap );

        ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_GRID_SHOW, false );

        // override other defaults
        ::chart::PropertyHelper::setPropertyValue< sal_Int32 >(
            rOutMap, ::chart::LineProperties::PROP_LINE_COLOR, 0xb3b3b3 );  // gray30
    }    
};

struct StaticGridDefaults : public rtl::StaticAggregate< ::chart::tPropertyValueMap, StaticGridDefaults_Initializer >
{
};

struct StaticGridInfoHelper_Initializer
{
    ::cppu::OPropertyArrayHelper* operator()()
    {
        static ::cppu::OPropertyArrayHelper aPropHelper( lcl_GetPropertySequence() );
        return &aPropHelper;
    }

private:
    Sequence< Property > lcl_GetPropertySequence()
    {
        ::std::vector< Property > aProperties;
        lcl_AddPropertiesToVector( aProperties );
        ::chart::LineProperties::AddPropertiesToVector( aProperties );
        ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );

        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        return ::chart::ContainerHelper::ContainerToSequence( aProperties );
    }


};

struct StaticGridInfoHelper : public rtl::StaticAggregate< ::cppu::OPropertyArrayHelper, StaticGridInfoHelper_Initializer >
{
};

struct StaticGridInfo_Initializer
{
    uno::Reference< beans::XPropertySetInfo >* operator()()
    {
        static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
            ::cppu::OPropertySetHelper::createPropertySetInfo(*StaticGridInfoHelper::get() ) );
        return &xPropertySetInfo;
    }
};

struct StaticGridInfo : public rtl::StaticAggregate< uno::Reference< beans::XPropertySetInfo >, StaticGridInfo_Initializer >
{
};

} // anonymous namespace

// ____________________________________________________________

namespace chart
{

GridProperties::GridProperties( Reference< uno::XComponentContext > const & /* xContext */ ) :
        ::property::OPropertySet( m_aMutex ),
    m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{
}

GridProperties::GridProperties() :
        ::property::OPropertySet( m_aMutex ),
    m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{}

GridProperties::GridProperties( const GridProperties & rOther ) :
        MutexContainer(),
        impl::GridProperties_Base(),
        ::property::OPropertySet( rOther, m_aMutex ),
    m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{
}

GridProperties::~GridProperties()
{}

// ____ OPropertySet ____
uno::Any GridProperties::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    const tPropertyValueMap& rStaticDefaults = *StaticGridDefaults::get();
    tPropertyValueMap::const_iterator aFound( rStaticDefaults.find( nHandle ) );
    if( aFound == rStaticDefaults.end() )
        return uno::Any();
    return (*aFound).second;
}

::cppu::IPropertyArrayHelper & SAL_CALL GridProperties::getInfoHelper()
{
    return *StaticGridInfoHelper::get();
}

// ____ XPropertySet ____
Reference< beans::XPropertySetInfo > SAL_CALL GridProperties::getPropertySetInfo()
    throw (uno::RuntimeException)
{
    return *StaticGridInfo::get();
}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL GridProperties::createClone()
    throw (uno::RuntimeException)
{
    return uno::Reference< util::XCloneable >( new GridProperties( *this ));
}

// ____ XModifyBroadcaster ____
void SAL_CALL GridProperties::addModifyListener( const Reference< util::XModifyListener >& aListener )
    throw (uno::RuntimeException)
{
    try
    {
        Reference< util::XModifyBroadcaster > xBroadcaster( m_xModifyEventForwarder, uno::UNO_QUERY_THROW );
        xBroadcaster->addModifyListener( aListener );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

void SAL_CALL GridProperties::removeModifyListener( const Reference< util::XModifyListener >& aListener )
    throw (uno::RuntimeException)
{
    try
    {
        Reference< util::XModifyBroadcaster > xBroadcaster( m_xModifyEventForwarder, uno::UNO_QUERY_THROW );
        xBroadcaster->removeModifyListener( aListener );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

// ____ XModifyListener ____
void SAL_CALL GridProperties::modified( const lang::EventObject& aEvent )
    throw (uno::RuntimeException)
{
    m_xModifyEventForwarder->modified( aEvent );
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL GridProperties::disposing( const lang::EventObject& /* Source */ )
    throw (uno::RuntimeException)
{
    // nothing
}

// ____ OPropertySet ____
void GridProperties::firePropertyChangeEvent()
{
    fireModifyEvent();
}

void GridProperties::fireModifyEvent()
{
    m_xModifyEventForwarder->modified( lang::EventObject( static_cast< uno::XWeak* >( this )));
}

// ================================================================================

Sequence< OUString > GridProperties::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 2 );
    aServices[ 0 ] = C2U( "com.sun.star.chart2.GridProperties" );
    aServices[ 1 ] = C2U( "com.sun.star.beans.PropertySet" );
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( GridProperties, lcl_aServiceName );

// needed by MSC compiler
using impl::GridProperties_Base;

IMPLEMENT_FORWARD_XINTERFACE2( GridProperties, GridProperties_Base, ::property::OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( GridProperties, GridProperties_Base, ::property::OPropertySet )

} //  namespace chart
