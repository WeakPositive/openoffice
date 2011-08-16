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

#ifndef __FRAMEWORK_HELPER_TITLEBARUPDATE_HXX_
#include <helper/titlebarupdate.hxx>
#endif

//_________________________________________________________________________________________________________________
//	my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_PATTERN_WINDOW_HXX_
#include <pattern/window.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_WRITEGUARD_HXX_
#include <threadhelp/writeguard.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_READGUARD_HXX_
#include <threadhelp/readguard.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#include <macros/generic.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_H_
#include <services.h>
#endif

#ifndef __FRAMEWORK_PROPETIES_H_
#include <properties.h>
#endif

//_________________________________________________________________________________________________________________
//	interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICXEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XMODULEMANAGER_HPP_
#include <com/sun/star/frame/XModuleManager.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XMATERIALHOLDER_HPP_
#include <com/sun/star/beans/XMaterialHolder.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XTITLECHANGEBROADCASTER_HPP_
#include <com/sun/star/frame/XTitleChangeBroadcaster.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif

//_________________________________________________________________________________________________________________
//	other includes
//_________________________________________________________________________________________________________________

#include <comphelper/sequenceashashmap.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/bootstrap.hxx>
#include <vcl/window.hxx>
#include <vcl/syswin.hxx>
#include <toolkit/unohlp.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <tools/diagnose_ex.h>

//_________________________________________________________________________________________________________________
//	namespace

namespace framework{

//_________________________________________________________________________________________________________________
//	const

static const ::sal_Int32 INVALID_ICON_ID = -1;
static const ::sal_Int32 DEFAULT_ICON_ID =  0;

//_________________________________________________________________________________________________________________
//	definitions

//*****************************************************************************************************************
//  XInterface, XTypeProvider

DEFINE_XINTERFACE_5(TitleBarUpdate                                                              ,
                    OWeakObject                                                                 ,
                    DIRECT_INTERFACE (css::lang::XTypeProvider                                  ),
                    DIRECT_INTERFACE (css::lang::XInitialization                                ),
                    DIRECT_INTERFACE (css::frame::XFrameActionListener                          ),
                    DIRECT_INTERFACE (css::frame::XTitleChangeListener                          ),
                    DERIVED_INTERFACE(css::lang::XEventListener,css::frame::XFrameActionListener))

DEFINE_XTYPEPROVIDER_5(TitleBarUpdate                  ,
                       css::lang::XTypeProvider        ,
                       css::lang::XInitialization      ,
                       css::frame::XFrameActionListener,
                       css::frame::XTitleChangeListener,
                       css::lang::XEventListener       )

//*****************************************************************************************************************
TitleBarUpdate::TitleBarUpdate(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
    : ThreadHelpBase          (&Application::GetSolarMutex())
    , m_xSMGR                 (xSMGR                        )
    , m_xFrame                (                             )
{
}

//*****************************************************************************************************************
TitleBarUpdate::~TitleBarUpdate()
{
}

//*****************************************************************************************************************
void SAL_CALL TitleBarUpdate::initialize(const css::uno::Sequence< css::uno::Any >& lArguments)
    throw(css::uno::Exception       ,
          css::uno::RuntimeException)
{
    // check arguments
    css::uno::Reference< css::frame::XFrame > xFrame;
    if (lArguments.getLength() < 1)
        throw css::lang::IllegalArgumentException(
                DECLARE_ASCII("Empty argument list!"),
                static_cast< ::cppu::OWeakObject* >(this),
                1);
                
    lArguments[0] >>= xFrame;
    if (!xFrame.is())
        throw css::lang::IllegalArgumentException(
                DECLARE_ASCII("No valid frame specified!"),
                static_cast< ::cppu::OWeakObject* >(this),
                1);
                
    // SYNCHRONIZED ->
    WriteGuard aWriteLock(m_aLock);
    // hold the frame as weak reference(!) so it can die everytimes :-)
    m_xFrame = xFrame;
    aWriteLock.unlock();
    // <- SYNCHRONIZED
    
    // start listening
    xFrame->addFrameActionListener(this);

    css::uno::Reference< css::frame::XTitleChangeBroadcaster > xBroadcaster(xFrame, css::uno::UNO_QUERY);
    if (xBroadcaster.is ())
        xBroadcaster->addTitleChangeListener (this);
}

//*****************************************************************************************************************
void SAL_CALL TitleBarUpdate::frameAction(const css::frame::FrameActionEvent& aEvent)
    throw(css::uno::RuntimeException)
{
    // we are interested on events only, which must trigger a title bar update
    // because component was changed.
    if (
        (aEvent.Action == css::frame::FrameAction_COMPONENT_ATTACHED  ) ||
        (aEvent.Action == css::frame::FrameAction_COMPONENT_REATTACHED) ||
        (aEvent.Action == css::frame::FrameAction_COMPONENT_DETACHING )
       )
    {
        impl_forceUpdate ();
    }
}

//*****************************************************************************************************************
void SAL_CALL TitleBarUpdate::titleChanged(const css::frame::TitleChangedEvent& /* aEvent */)
    throw (css::uno::RuntimeException)
{
    impl_forceUpdate ();
}

//*****************************************************************************************************************
void SAL_CALL TitleBarUpdate::disposing(const css::lang::EventObject&)
    throw(css::uno::RuntimeException)
{
    // nothing todo here - because we hold the frame as weak reference only
}

//*****************************************************************************************************************
::sal_Bool TitleBarUpdate::implst_getModuleInfo(const css::uno::Reference< css::frame::XFrame >& xFrame,
                                                      TModuleInfo&                               rInfo )
{
    if ( ! xFrame.is ())
        return sal_False;

    // SYNCHRONIZED ->
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = m_xSMGR;
    aReadLock.unlock();
    // <- SYNCHRONIZED
    
    try
    {
        css::uno::Reference< css::frame::XModuleManager > xModuleManager(
            xSMGR->createInstance(SERVICENAME_MODULEMANAGER),
            css::uno::UNO_QUERY_THROW);
    
        css::uno::Reference< css::container::XNameAccess > xConfig(
            xModuleManager,
            css::uno::UNO_QUERY_THROW);
    
                                        rInfo.sID = xModuleManager->identify(xFrame);
        ::comphelper::SequenceAsHashMap lProps    = xConfig->getByName (rInfo.sID);
        
        rInfo.sUIName = lProps.getUnpackedValueOrDefault (OFFICEFACTORY_PROPNAME_UINAME, ::rtl::OUString());
        rInfo.nIcon   = lProps.getUnpackedValueOrDefault (OFFICEFACTORY_PROPNAME_ICON  , INVALID_ICON_ID  );
    
		// Note: If we could retrieve a module id ... everything is OK.
		// UIName and Icon ID are optional values !
        ::sal_Bool bSuccess = (rInfo.sID.getLength () > 0);
        return bSuccess;
    }
    catch(const css::uno::Exception&)
        {}
    
    return sal_False;
}

//*****************************************************************************************************************
void TitleBarUpdate::impl_forceUpdate()
{
    // SYNCHRONIZED ->
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = m_xSMGR ;
    css::uno::Reference< css::frame::XFrame >              xFrame(m_xFrame.get(), css::uno::UNO_QUERY);
    aReadLock.unlock();
    // <- SYNCHRONIZED

    // frame already gone ? We hold it weak only ...
    if ( ! xFrame.is())
        return;
    
    // no window -> no chance to set/update title and icon
    css::uno::Reference< css::awt::XWindow > xWindow = xFrame->getContainerWindow();
    if ( ! xWindow.is())
        return;

    impl_updateIcon  (xFrame);
    impl_updateTitle (xFrame);
}

//*****************************************************************************************************************
void TitleBarUpdate::impl_updateIcon(const css::uno::Reference< css::frame::XFrame >& xFrame)
{
    css::uno::Reference< css::frame::XController > xController = xFrame->getController      ();
    css::uno::Reference< css::awt::XWindow >       xWindow     = xFrame->getContainerWindow ();

    if (
        ( ! xController.is() ) ||
        ( ! xWindow.is()     )
       )
        return;
    
    // a) set default value to an invalid one. So we can start further searches for right icon id, if
    //    first steps failed!
    sal_Int32 nIcon = INVALID_ICON_ID;

    // b) try to find information on controller property set directly
    //    Don't forget to catch possible exceptions - because these property is an optional one!
    css::uno::Reference< css::beans::XPropertySet > xSet( xController, css::uno::UNO_QUERY );
    if ( xSet.is() )
    {
        try
        {
            css::uno::Reference< css::beans::XPropertySetInfo > const xPSI( xSet->getPropertySetInfo(), css::uno::UNO_SET_THROW );
            if ( xPSI->hasPropertyByName( CONTROLLER_PROPNAME_ICONID ) )
                xSet->getPropertyValue( CONTROLLER_PROPNAME_ICONID ) >>= nIcon;
        }
        catch(const css::uno::Exception&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    // c) if b) failed ... identify the used module and retrieve set icon from module config.
    //    Tirck :-) Module was already specified outside and aInfo contains all needed informations.
    if ( nIcon == INVALID_ICON_ID )
	{
		TModuleInfo aInfo;
		if (implst_getModuleInfo(xFrame, aInfo))
			nIcon = aInfo.nIcon;
	}

    // d) if all steps failed - use fallback :-)
    //    ... means using the global staroffice icon
    if( nIcon == INVALID_ICON_ID )
        nIcon = DEFAULT_ICON_ID;

    // e) set icon on container window now
    //    Don't forget SolarMutex! We use vcl directly :-(
    //    Check window pointer for right WorkWindow class too!!!
    
    // VCL SYNCHRONIZED ->
    ::vos::OClearableGuard aSolarLock( Application::GetSolarMutex() );
    
    Window* pWindow = (VCLUnoHelper::GetWindow( xWindow ));
    if (
        ( pWindow                                 ) &&
        ( pWindow->GetType() == WINDOW_WORKWINDOW )
       )
    {
        WorkWindow* pWorkWindow = (WorkWindow*)pWindow;
        pWorkWindow->SetIcon( (sal_uInt16)nIcon );
        
        css::uno::Reference< css::frame::XModel > xModel = xController->getModel();
        rtl::OUString aURL;
        if( xModel.is() )
            aURL = xModel->getURL();
        pWorkWindow->SetRepresentedURL( aURL );
    }

    aSolarLock.clear();
    // <- VCL SYNCHRONIZED
}

//*****************************************************************************************************************
void TitleBarUpdate::impl_updateTitle(const css::uno::Reference< css::frame::XFrame >& xFrame)
{
	// no window ... no chance to set any title -> return
    css::uno::Reference< css::awt::XWindow > xWindow = xFrame->getContainerWindow ();
    if ( ! xWindow.is() )
        return;
    
	css::uno::Reference< css::frame::XTitle > xTitle(xFrame, css::uno::UNO_QUERY);
	if ( ! xTitle.is() )
		return;

	const ::rtl::OUString sTitle = xTitle->getTitle ();

    // VCL SYNCHRONIZED ->
    ::vos::OClearableGuard aSolarLock( Application::GetSolarMutex() );
    
    Window* pWindow = (VCLUnoHelper::GetWindow( xWindow ));
    if (
        ( pWindow                                 ) &&
        ( pWindow->GetType() == WINDOW_WORKWINDOW )
       )
    {
        WorkWindow* pWorkWindow = (WorkWindow*)pWindow;
        pWorkWindow->SetText( sTitle );
    }

    aSolarLock.clear();
    // <- VCL SYNCHRONIZED
}

} // namespace framework
