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
#include "precompiled_framework.hxx"

//_________________________________________________________________________________________________________________
//	my own includes
//_________________________________________________________________________________________________________________
#include <helper/uiconfigelementwrapperbase.hxx>
#include <general.h>
#include <properties.h>
#include <threadhelp/resetableguard.hxx>
#include <uielement/constitemcontainer.hxx>
#include <uielement/rootitemcontainer.hxx>

//_________________________________________________________________________________________________________________
//	interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/ui/XUIConfiguration.hpp>

//_________________________________________________________________________________________________________________
//	includes of other projects
//_________________________________________________________________________________________________________________
#include <vcl/svapp.hxx>
#include <rtl/logfile.hxx>

const int UIELEMENT_PROPHANDLE_CONFIGSOURCE     = 1;
const int UIELEMENT_PROPHANDLE_FRAME            = 2;
const int UIELEMENT_PROPHANDLE_PERSISTENT       = 3;
const int UIELEMENT_PROPHANDLE_RESOURCEURL      = 4;
const int UIELEMENT_PROPHANDLE_TYPE             = 5;
const int UIELEMENT_PROPHANDLE_XMENUBAR         = 6;
const int UIELEMENT_PROPHANDLE_CONFIGLISTENER   = 7;
const int UIELEMENT_PROPHANDLE_NOCLOSE          = 8;
const int UIELEMENT_PROPCOUNT                   = 8;
const rtl::OUString UIELEMENT_PROPNAME_CONFIGLISTENER( RTL_CONSTASCII_USTRINGPARAM( "ConfigListener" ));
const rtl::OUString UIELEMENT_PROPNAME_CONFIGSOURCE( RTL_CONSTASCII_USTRINGPARAM( "ConfigurationSource" ));
const rtl::OUString UIELEMENT_PROPNAME_FRAME( RTL_CONSTASCII_USTRINGPARAM( "Frame" ));
const rtl::OUString UIELEMENT_PROPNAME_PERSISTENT( RTL_CONSTASCII_USTRINGPARAM( "Persistent" ));
const rtl::OUString UIELEMENT_PROPNAME_RESOURCEURL( RTL_CONSTASCII_USTRINGPARAM( "ResourceURL" ));
const rtl::OUString UIELEMENT_PROPNAME_TYPE( RTL_CONSTASCII_USTRINGPARAM( "Type" ));
const rtl::OUString UIELEMENT_PROPNAME_XMENUBAR( RTL_CONSTASCII_USTRINGPARAM( "XMenuBar" ));
const rtl::OUString UIELEMENT_PROPNAME_NOCLOSE( RTL_CONSTASCII_USTRINGPARAM( "NoClose" ));

//using namespace rtl;
using namespace com::sun::star::beans;
using namespace com::sun::star::uno;
using namespace com::sun::star::frame;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace ::com::sun::star::ui;

namespace framework
{

//*****************************************************************************************************************
//	XInterface, XTypeProvider
//*****************************************************************************************************************
DEFINE_XINTERFACE_10    (   UIConfigElementWrapperBase                                               ,
                            OWeakObject                                                              ,
                            DIRECT_INTERFACE( ::com::sun::star::lang::XTypeProvider                  ),
                            DIRECT_INTERFACE( ::com::sun::star::ui::XUIElement               ),
                            DIRECT_INTERFACE( ::com::sun::star::ui::XUIElementSettings       ),
                            DIRECT_INTERFACE( ::com::sun::star::beans::XMultiPropertySet		     ),
                            DIRECT_INTERFACE( ::com::sun::star::beans::XFastPropertySet		         ),
                            DIRECT_INTERFACE( ::com::sun::star::beans::XPropertySet				     ),
                            DIRECT_INTERFACE( ::com::sun::star::lang::XInitialization                ),
                            DIRECT_INTERFACE( ::com::sun::star::lang::XComponent                     ),
                            DIRECT_INTERFACE( ::com::sun::star::util::XUpdatable                     ),
                            DIRECT_INTERFACE( ::com::sun::star::ui::XUIConfigurationListener )
						)

DEFINE_XTYPEPROVIDER_10 (   UIConfigElementWrapperBase                              ,
                            ::com::sun::star::lang::XTypeProvider                   ,
                            ::com::sun::star::ui::XUIElement                ,
                            ::com::sun::star::ui::XUIElementSettings        ,
                            ::com::sun::star::beans::XMultiPropertySet              ,
                            ::com::sun::star::beans::XFastPropertySet               ,
                            ::com::sun::star::beans::XPropertySet                   ,
                            ::com::sun::star::lang::XInitialization                 ,
                            ::com::sun::star::lang::XComponent                      ,
                            ::com::sun::star::util::XUpdatable                      ,
                            ::com::sun::star::ui::XUIConfigurationListener
						)

UIConfigElementWrapperBase::UIConfigElementWrapperBase( sal_Int16 nType,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xServiceFactory )
    :   ThreadHelpBase              ( &Application::GetSolarMutex()                      )
    ,   ::cppu::OBroadcastHelperVar< ::cppu::OMultiTypeInterfaceContainerHelper, ::cppu::OMultiTypeInterfaceContainerHelper::keyType >( m_aLock.getShareableOslMutex() )
    ,   ::cppu::OPropertySetHelper  ( *(static_cast< ::cppu::OBroadcastHelper* >(this)) )
    ,   ::cppu::OWeakObject         (                                                   )
    ,   m_nType                     ( nType                                             )
    ,   m_bPersistent               ( sal_True                                          )
    ,   m_bInitialized              ( sal_False                                         )
    ,   m_bConfigListener           ( sal_False                                         )
    ,   m_bConfigListening          ( sal_False                                         )
    ,   m_bDisposed                 ( sal_False                                         )
    ,   m_bNoClose                  ( sal_False                                         )
    ,   m_xServiceFactory           ( _xServiceFactory                                  )
    ,   m_aListenerContainer        ( m_aLock.getShareableOslMutex()                    )
{
}

UIConfigElementWrapperBase::~UIConfigElementWrapperBase()
{
}

// XComponent
void SAL_CALL UIConfigElementWrapperBase::dispose() throw (::com::sun::star::uno::RuntimeException)
{
    // must be implemented by derived class
    ResetableGuard aLock( m_aLock );
    m_bDisposed = sal_True;
}

void SAL_CALL UIConfigElementWrapperBase::addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    m_aListenerContainer.addInterface( ::getCppuType( ( const css::uno::Reference< css::lang::XEventListener >* ) NULL ), xListener );
}

void SAL_CALL UIConfigElementWrapperBase::removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException)
{
    m_aListenerContainer.removeInterface( ::getCppuType( ( const css::uno::Reference< css::lang::XEventListener >* ) NULL ), aListener );
}

// XEventListener
void SAL_CALL UIConfigElementWrapperBase::disposing( const EventObject& )
throw( RuntimeException )
{
    ResetableGuard aLock( m_aLock );
    m_xConfigSource.clear();
}

void SAL_CALL UIConfigElementWrapperBase::initialize( const Sequence< Any >& aArguments )
throw ( Exception, RuntimeException )
{
    ResetableGuard aLock( m_aLock );

    if ( !m_bInitialized )
    {
        for ( sal_Int32 n = 0; n < aArguments.getLength(); n++ )
        {
            PropertyValue aPropValue;
            if ( aArguments[n] >>= aPropValue )
            {
                if ( aPropValue.Name.equals( UIELEMENT_PROPNAME_CONFIGSOURCE ))
                    setFastPropertyValue_NoBroadcast( UIELEMENT_PROPHANDLE_CONFIGSOURCE, aPropValue.Value );
                else if ( aPropValue.Name.equals( UIELEMENT_PROPNAME_FRAME ))
                    setFastPropertyValue_NoBroadcast( UIELEMENT_PROPHANDLE_FRAME, aPropValue.Value );
                else if ( aPropValue.Name.equals( UIELEMENT_PROPNAME_PERSISTENT ))
                    setFastPropertyValue_NoBroadcast( UIELEMENT_PROPHANDLE_PERSISTENT, aPropValue.Value );
                else if ( aPropValue.Name.equals( UIELEMENT_PROPNAME_RESOURCEURL ))
                    setFastPropertyValue_NoBroadcast( UIELEMENT_PROPHANDLE_RESOURCEURL, aPropValue.Value );
                else if ( aPropValue.Name.equals( UIELEMENT_PROPNAME_TYPE ))
                    setFastPropertyValue_NoBroadcast( UIELEMENT_PROPHANDLE_TYPE, aPropValue.Value );
                else if ( aPropValue.Name.equals( UIELEMENT_PROPNAME_CONFIGLISTENER ))
                    setFastPropertyValue_NoBroadcast( UIELEMENT_PROPHANDLE_CONFIGLISTENER, aPropValue.Value );
                else if ( aPropValue.Name.equals( UIELEMENT_PROPNAME_NOCLOSE ))
                    setFastPropertyValue_NoBroadcast( UIELEMENT_PROPHANDLE_NOCLOSE, aPropValue.Value );
            }
        }

        m_bInitialized = sal_True;
    }
}

// XUpdatable
void SAL_CALL UIConfigElementWrapperBase::update() throw (::com::sun::star::uno::RuntimeException)
{
    // can be implemented by derived class
}

void SAL_CALL UIConfigElementWrapperBase::elementInserted( const ::com::sun::star::ui::ConfigurationEvent& ) throw (::com::sun::star::uno::RuntimeException)
{
    // can be implemented by derived class
}

void SAL_CALL UIConfigElementWrapperBase::elementRemoved( const ::com::sun::star::ui::ConfigurationEvent& ) throw (::com::sun::star::uno::RuntimeException)
{
    // can be implemented by derived class
}

void SAL_CALL UIConfigElementWrapperBase::elementReplaced( const ::com::sun::star::ui::ConfigurationEvent& ) throw (::com::sun::star::uno::RuntimeException)
{
    // can be implemented by derived class
}

// XPropertySet helper
sal_Bool SAL_CALL UIConfigElementWrapperBase::convertFastPropertyValue( Any&       aConvertedValue ,
                                                                        Any&       aOldValue       ,
                                                                        sal_Int32  nHandle         ,
                                                                        const Any& aValue             ) throw( com::sun::star::lang::IllegalArgumentException )
{
	//	Initialize state with sal_False !!!
	//	(Handle can be invalid)
	sal_Bool bReturn = sal_False;

    switch( nHandle )
	{
        case UIELEMENT_PROPHANDLE_CONFIGLISTENER:
            bReturn = PropHelper::willPropertyBeChanged(
                        com::sun::star::uno::makeAny(m_bConfigListener),
                        aValue,
                        aOldValue,
                        aConvertedValue);
            break;

        case UIELEMENT_PROPHANDLE_CONFIGSOURCE:
            bReturn = PropHelper::willPropertyBeChanged(
                        com::sun::star::uno::makeAny(m_xConfigSource),
                        aValue,
                        aOldValue,
                        aConvertedValue);
            break;

        case UIELEMENT_PROPHANDLE_FRAME:
        {
            Reference< XFrame > xFrame( m_xWeakFrame );
            bReturn = PropHelper::willPropertyBeChanged(
                        com::sun::star::uno::makeAny(xFrame),
                        aValue,
                        aOldValue,
                        aConvertedValue);
        }
        break;

        case UIELEMENT_PROPHANDLE_PERSISTENT:
            bReturn = PropHelper::willPropertyBeChanged(
                        com::sun::star::uno::makeAny(m_bPersistent),
                        aValue,
                        aOldValue,
                        aConvertedValue);
            break;

        case UIELEMENT_PROPHANDLE_RESOURCEURL:
            bReturn = PropHelper::willPropertyBeChanged(
                        com::sun::star::uno::makeAny(m_aResourceURL),
                        aValue,
                        aOldValue,
                        aConvertedValue);
            break;

        case UIELEMENT_PROPHANDLE_TYPE :
            bReturn = PropHelper::willPropertyBeChanged(
                        com::sun::star::uno::makeAny(m_nType),
                        aValue,
                        aOldValue,
                        aConvertedValue);
                break;

        case UIELEMENT_PROPHANDLE_XMENUBAR :
            bReturn = PropHelper::willPropertyBeChanged(
                        com::sun::star::uno::makeAny(m_xMenuBar),
                        aValue,
                        aOldValue,
                        aConvertedValue);
                break;

        case UIELEMENT_PROPHANDLE_NOCLOSE:
            bReturn = PropHelper::willPropertyBeChanged(
                        com::sun::star::uno::makeAny(m_bNoClose),
                        aValue,
                        aOldValue,
                        aConvertedValue);
                break;
	}

	// Return state of operation.
	return bReturn ;
}

void SAL_CALL UIConfigElementWrapperBase::setFastPropertyValue_NoBroadcast(   sal_Int32               nHandle ,
                                                                        const com::sun::star::uno::Any&    aValue  ) throw( com::sun::star::uno::Exception )
{
    switch( nHandle )
	{
        case UIELEMENT_PROPHANDLE_CONFIGLISTENER:
        {
            bool bBool( m_bConfigListener );
            aValue >>= bBool;
            if ( m_bConfigListener != bBool )
            {
                if ( m_bConfigListening )
                {
                    if ( m_xConfigSource.is() && !bBool )
                    {
                        try
                        {
                            Reference< XUIConfiguration > xUIConfig( m_xConfigSource, UNO_QUERY );
                            if ( xUIConfig.is() )
                            {
                                xUIConfig->removeConfigurationListener( Reference< XUIConfigurationListener >( static_cast< OWeakObject* >( this ), UNO_QUERY ));
                                m_bConfigListening = sal_False;
                            }
                        }
                        catch ( Exception& )
                        {
                        }
                    }
                }
                else
                {
                    if ( m_xConfigSource.is() && bBool )
                    {
                        try
                        {
                            Reference< XUIConfiguration > xUIConfig( m_xConfigSource, UNO_QUERY );
                            if ( xUIConfig.is() )
                            {
                                xUIConfig->addConfigurationListener( Reference< XUIConfigurationListener >( static_cast< OWeakObject* >( this ), UNO_QUERY ));
                                m_bConfigListening = sal_True;
                            }
                        }
                        catch ( Exception& )
                        {
                        }
                    }
                }

                m_bConfigListener = bBool;
            }
        }
        break;
        case UIELEMENT_PROPHANDLE_CONFIGSOURCE:
            aValue >>= m_xConfigSource;
            break;
        case UIELEMENT_PROPHANDLE_FRAME:
        {
            Reference< XFrame > xFrame;

            aValue >>= xFrame;
            m_xWeakFrame = xFrame;
            break;
        }
        case UIELEMENT_PROPHANDLE_PERSISTENT:
        {
            sal_Bool bBool( m_bPersistent );
            aValue >>= bBool;
            m_bPersistent = bBool;
            break;
        }
        case UIELEMENT_PROPHANDLE_RESOURCEURL:
            aValue >>= m_aResourceURL;
            break;
        case UIELEMENT_PROPHANDLE_TYPE:
            aValue >>= m_nType;
            break;
        case UIELEMENT_PROPHANDLE_XMENUBAR:
            aValue >>= m_xMenuBar;
            break;
        case UIELEMENT_PROPHANDLE_NOCLOSE:
        {
            sal_Bool bBool( m_bNoClose );
            aValue >>= bBool;
            m_bNoClose = bBool;
            break;
        }
	}
}

void SAL_CALL UIConfigElementWrapperBase::getFastPropertyValue( com::sun::star::uno::Any& aValue  ,
                                                                sal_Int32                 nHandle   ) const
{
    switch( nHandle )
	{
        case UIELEMENT_PROPHANDLE_CONFIGLISTENER:
            aValue <<= m_bConfigListener;
            break;
        case UIELEMENT_PROPHANDLE_CONFIGSOURCE:
            aValue <<= m_xConfigSource;
            break;
        case UIELEMENT_PROPHANDLE_FRAME:
        {
            Reference< XFrame > xFrame( m_xWeakFrame );
            aValue <<= xFrame;
            break;
        }
        case UIELEMENT_PROPHANDLE_PERSISTENT:
            aValue <<= m_bPersistent;
            break;
        case UIELEMENT_PROPHANDLE_RESOURCEURL:
            aValue <<= m_aResourceURL;
            break;
        case UIELEMENT_PROPHANDLE_TYPE:
            aValue <<= m_nType;
            break;
        case UIELEMENT_PROPHANDLE_XMENUBAR:
            aValue <<= m_xMenuBar;
            break;
        case UIELEMENT_PROPHANDLE_NOCLOSE:
            aValue <<= m_bNoClose;
            break;
	}
}

::cppu::IPropertyArrayHelper& SAL_CALL UIConfigElementWrapperBase::getInfoHelper()
{
	// Optimize this method !
	// We initialize a static variable only one time. And we don't must use a mutex at every call!
	// For the first call; pInfoHelper is NULL - for the second call pInfoHelper is different from NULL!
    static ::cppu::OPropertyArrayHelper* pInfoHelper = NULL;

    if( pInfoHelper == NULL )
	{
		// Ready for multithreading
        osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;

		// Control this pointer again, another instance can be faster then these!
        if( pInfoHelper == NULL )
		{
			// Define static member to give structure of properties to baseclass "OPropertySetHelper".
			// "impl_getStaticPropertyDescriptor" is a non exported and static funtion, who will define a static propertytable.
			// "sal_True" say: Table is sorted by name.
            static ::cppu::OPropertyArrayHelper aInfoHelper( impl_getStaticPropertyDescriptor(), sal_True );
			pInfoHelper = &aInfoHelper;
		}
	}

    return(*pInfoHelper);
}

com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo > SAL_CALL UIConfigElementWrapperBase::getPropertySetInfo() throw (::com::sun::star::uno::RuntimeException)
{
	// Optimize this method !
	// We initialize a static variable only one time. And we don't must use a mutex at every call!
	// For the first call; pInfo is NULL - for the second call pInfo is different from NULL!
    static com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo >* pInfo = NULL;

    if( pInfo == NULL )
	{
		// Ready for multithreading
		osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;
		// Control this pointer again, another instance can be faster then these!
        if( pInfo == NULL )
		{
			// Create structure of propertysetinfo for baseclass "OPropertySetHelper".
			// (Use method "getInfoHelper()".)
            static com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
			pInfo = &xInfo;
		}
	}

	return (*pInfo);
}

const com::sun::star::uno::Sequence< com::sun::star::beans::Property > UIConfigElementWrapperBase::impl_getStaticPropertyDescriptor()
{
	// Create a new static property array to initialize sequence!
	// Table of all predefined properties of this class. Its used from OPropertySetHelper-class!
	// Don't forget to change the defines (see begin of this file), if you add, change or delete a property in this list!!!
	// It's necessary for methods of OPropertySetHelper.
	// ATTENTION:
    //      YOU MUST SORT FOLLOW TABLE BY NAME ALPHABETICAL !!!

    static const com::sun::star::beans::Property pProperties[] =
	{
        com::sun::star::beans::Property( UIELEMENT_PROPNAME_CONFIGLISTENER, UIELEMENT_PROPHANDLE_CONFIGLISTENER , ::getCppuType((const sal_Bool*)NULL), com::sun::star::beans::PropertyAttribute::TRANSIENT  ),
        com::sun::star::beans::Property( UIELEMENT_PROPNAME_CONFIGSOURCE  , UIELEMENT_PROPHANDLE_CONFIGSOURCE   , ::getCppuType((const Reference< ::com::sun::star::ui::XUIConfigurationManager >*)NULL), com::sun::star::beans::PropertyAttribute::TRANSIENT  ),
        com::sun::star::beans::Property( UIELEMENT_PROPNAME_FRAME         , UIELEMENT_PROPHANDLE_FRAME          , ::getCppuType((const Reference< com::sun::star::frame::XFrame >*)NULL), com::sun::star::beans::PropertyAttribute::TRANSIENT | com::sun::star::beans::PropertyAttribute::READONLY ),
        com::sun::star::beans::Property( UIELEMENT_PROPNAME_NOCLOSE       , UIELEMENT_PROPHANDLE_NOCLOSE        , ::getCppuType((const sal_Bool*)NULL), com::sun::star::beans::PropertyAttribute::TRANSIENT ),
        com::sun::star::beans::Property( UIELEMENT_PROPNAME_PERSISTENT    , UIELEMENT_PROPHANDLE_PERSISTENT     , ::getCppuType((const sal_Bool*)NULL), com::sun::star::beans::PropertyAttribute::TRANSIENT  ),
        com::sun::star::beans::Property( UIELEMENT_PROPNAME_RESOURCEURL   , UIELEMENT_PROPHANDLE_RESOURCEURL    , ::getCppuType((const ::rtl::OUString*)NULL), com::sun::star::beans::PropertyAttribute::TRANSIENT | com::sun::star::beans::PropertyAttribute::READONLY ),
        com::sun::star::beans::Property( UIELEMENT_PROPNAME_TYPE          , UIELEMENT_PROPHANDLE_TYPE           , ::getCppuType((const ::rtl::OUString*)NULL), com::sun::star::beans::PropertyAttribute::TRANSIENT | com::sun::star::beans::PropertyAttribute::READONLY ),
        com::sun::star::beans::Property( UIELEMENT_PROPNAME_XMENUBAR      , UIELEMENT_PROPHANDLE_XMENUBAR       , ::getCppuType((const Reference< com::sun::star::awt::XMenuBar >*)NULL), com::sun::star::beans::PropertyAttribute::TRANSIENT | com::sun::star::beans::PropertyAttribute::READONLY )
	};
	// Use it to initialize sequence!
    static const com::sun::star::uno::Sequence< com::sun::star::beans::Property > lPropertyDescriptor( pProperties, UIELEMENT_PROPCOUNT );
	// Return static "PropertyDescriptor"
    return lPropertyDescriptor;
}
void SAL_CALL UIConfigElementWrapperBase::setSettings( const Reference< XIndexAccess >& xSettings ) throw ( RuntimeException )
{
    ResetableGuard aLock( m_aLock );

    //if ( m_bDisposed )
    //    throw DisposedException();

    if ( xSettings.is() )
    {
        // Create a copy of the data if the container is not const
        Reference< XIndexReplace > xReplace( xSettings, UNO_QUERY );
        if ( xReplace.is() )
            m_xConfigData = Reference< XIndexAccess >( static_cast< OWeakObject * >( new ConstItemContainer( xSettings ) ), UNO_QUERY );
        else
            m_xConfigData = xSettings;

        if ( m_xConfigSource.is() && m_bPersistent )
        {
            ::rtl::OUString aResourceURL( m_aResourceURL );
            Reference< XUIConfigurationManager > xUICfgMgr( m_xConfigSource );

            aLock.unlock();

            try
            {
                xUICfgMgr->replaceSettings( aResourceURL, m_xConfigData );
            }
            catch( NoSuchElementException& )
            {
            }
        }
        else if ( !m_bPersistent )
        {
            // Transient menubar => Fill menubar with new data
            impl_fillNewData();
        }
    }
}
void UIConfigElementWrapperBase::impl_fillNewData()
{
}
Reference< XIndexAccess > SAL_CALL UIConfigElementWrapperBase::getSettings( sal_Bool bWriteable ) throw ( RuntimeException )
{
    ResetableGuard aLock( m_aLock );

    //if ( m_bDisposed )
    //    throw DisposedException();

    if ( bWriteable )
        return Reference< XIndexAccess >( static_cast< OWeakObject * >( new RootItemContainer( m_xConfigData ) ), UNO_QUERY );
    
    return m_xConfigData;
}

Reference< XFrame > SAL_CALL UIConfigElementWrapperBase::getFrame() throw (RuntimeException)
{
    ResetableGuard aLock( m_aLock );
    Reference< XFrame > xFrame( m_xWeakFrame );
    return xFrame;
}

::rtl::OUString SAL_CALL UIConfigElementWrapperBase::getResourceURL() throw (RuntimeException)
{
    ResetableGuard aLock( m_aLock );
    return m_aResourceURL;
}

::sal_Int16 SAL_CALL UIConfigElementWrapperBase::getType() throw (RuntimeException)
{
    ResetableGuard aLock( m_aLock );
    return m_nType;
}

}
