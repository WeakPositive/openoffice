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
#include <uielement/recentfilesmenucontroller.hxx>

//_________________________________________________________________________________________________________________
//	my own includes
//_________________________________________________________________________________________________________________
#include <threadhelp/resetableguard.hxx>
#include "services.h"

#ifndef __FRAMEWORK_CLASSES_RESOURCE_HRC_
#include <classes/resource.hrc>
#endif
#include <classes/fwkresid.hxx>

//_________________________________________________________________________________________________________________
//	interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/awt/MenuItemStyle.hpp>
#include <com/sun/star/util/XStringWidth.hpp>
//_________________________________________________________________________________________________________________
//	includes of other projects
//_________________________________________________________________________________________________________________

#ifndef _VCL_MENU_HXX_
#include <vcl/menu.hxx>
#endif
#include <vcl/svapp.hxx>
#include <vcl/i18nhelp.hxx>
#include <tools/urlobj.hxx>
#include <rtl/ustrbuf.hxx>
#include <unotools/historyoptions.hxx>
#include <cppuhelper/implbase1.hxx>
#include <osl/file.hxx>
//#include <tools/solar.hrc>
#include <dispatch/uieventloghelper.hxx>
#include <vos/mutex.hxx>

//_________________________________________________________________________________________________________________
//	Defines
//_________________________________________________________________________________________________________________
//

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;
using namespace com::sun::star::container;

static const char SFX_REFERER_USER[] = "private:user";

namespace framework
{

class RecentFilesStringLength : public ::cppu::WeakImplHelper1< ::com::sun::star::util::XStringWidth >
{
	public:
		RecentFilesStringLength() {}
		virtual ~RecentFilesStringLength() {}

		// XStringWidth
		sal_Int32 SAL_CALL queryStringWidth( const ::rtl::OUString& aString )
			throw (::com::sun::star::uno::RuntimeException)
		{
			return aString.getLength();
		}
};

DEFINE_XSERVICEINFO_MULTISERVICE        (   RecentFilesMenuController                   ,
                                            OWeakObject                                 ,
                                            SERVICENAME_POPUPMENUCONTROLLER		        ,
											IMPLEMENTATIONNAME_RECENTFILESMENUCONTROLLER
										)

DEFINE_INIT_SERVICE                     (   RecentFilesMenuController, {} )

RecentFilesMenuController::RecentFilesMenuController( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager ) :
	svt::PopupMenuControllerBase( xServiceManager ),
    m_bDisabled( sal_False )
{
}

RecentFilesMenuController::~RecentFilesMenuController()
{
}

// private function
void RecentFilesMenuController::fillPopupMenu( Reference< css::awt::XPopupMenu >& rPopupMenu )
{
    VCLXPopupMenu*                                     pPopupMenu        = (VCLXPopupMenu *)VCLXMenu::GetImplementation( rPopupMenu );
    PopupMenu*                                         pVCLPopupMenu     = 0;

    vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

    resetPopupMenu( rPopupMenu );
    if ( pPopupMenu )
        pVCLPopupMenu = (PopupMenu *)pPopupMenu->GetMenu();

    if ( pVCLPopupMenu )
    {
	    Sequence< Sequence< PropertyValue > > aHistoryList = SvtHistoryOptions().GetList( ePICKLIST );
	    Reference< XStringWidth > xStringLength( new RecentFilesStringLength );

	    int nPickListMenuItems = ( aHistoryList.getLength() > 99 ) ? 99 : aHistoryList.getLength();

        // New vnd.sun.star.popup: command URL to support direct dispatches
        const rtl::OUString aCmdPrefix( RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.popup:RecentFileList?entry=" ));

        m_aRecentFilesItems.clear();
        if (( nPickListMenuItems > 0 ) && !m_bDisabled )
        {
            for ( int i = 0; i < nPickListMenuItems; i++ )
	        {
		        Sequence< PropertyValue >& rPickListEntry = aHistoryList[i];
		        RecentFile aRecentFile;

		        for ( int j = 0; j < rPickListEntry.getLength(); j++ )
		        {
			        Any a = rPickListEntry[j].Value;

			        if ( rPickListEntry[j].Name == HISTORY_PROPERTYNAME_URL )
				        a >>= aRecentFile.aURL;
			        else if ( rPickListEntry[j].Name == HISTORY_PROPERTYNAME_FILTER )
				        a >>= aRecentFile.aFilter;
			        else if ( rPickListEntry[j].Name == HISTORY_PROPERTYNAME_TITLE )
				        a >>= aRecentFile.aTitle;
			        else if ( rPickListEntry[j].Name == HISTORY_PROPERTYNAME_PASSWORD )
				        a >>= aRecentFile.aPassword;
		        }

                m_aRecentFilesItems.push_back( aRecentFile );
	        }
        }

	    if ( !m_aRecentFilesItems.empty() )
	    {
		    URL aTargetURL;

            const sal_uInt32 nCount = m_aRecentFilesItems.size();
            for ( sal_uInt32 i = 0; i < nCount; i++ )
			{
				char menuShortCut[5] = "~n: ";

				::rtl::OUString aMenuShortCut;
				if ( i <= 9 )
				{
					if ( i == 9 )
						aMenuShortCut = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "1~0: " ));
					else
					{
						menuShortCut[1] = (char)( '1' + i );
						aMenuShortCut = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( menuShortCut ));
					}
				}
				else
				{
					aMenuShortCut = rtl::OUString::valueOf((sal_Int32)( i + 1 ));
					aMenuShortCut += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ": " ));
				}

				// Abbreviate URL
                rtl::OUString	aURLString( aCmdPrefix + rtl::OUString::valueOf( sal_Int32( i )));
				rtl::OUString	aTipHelpText;
				rtl::OUString	aMenuTitle;
				INetURLObject	aURL( m_aRecentFilesItems[i].aURL );

				if ( aURL.GetProtocol() == INET_PROT_FILE )
				{
					// Do handle file URL differently => convert it to a system
					// path and abbreviate it with a special function:
					String aFileSystemPath( aURL.getFSysPath( INetURLObject::FSYS_DETECT ) );

					::rtl::OUString	aSystemPath( aFileSystemPath );
					::rtl::OUString	aCompactedSystemPath;

					aTipHelpText = aSystemPath;
					oslFileError nError = osl_abbreviateSystemPath( aSystemPath.pData, &aCompactedSystemPath.pData, 46, NULL );
					if ( !nError )
						aMenuTitle = String( aCompactedSystemPath );
					else
						aMenuTitle = aSystemPath;
				}
				else
				{
					// Use INetURLObject to abbreviate all other URLs
					String	aShortURL;
					aShortURL = aURL.getAbbreviated( xStringLength, 46, INetURLObject::DECODE_UNAMBIGUOUS );
					aMenuTitle += aShortURL;
					aTipHelpText = aURLString;
				}

				::rtl::OUString aTitle( aMenuShortCut + aMenuTitle );

				pVCLPopupMenu->InsertItem( sal_uInt16( i+1 ), aTitle );
				pVCLPopupMenu->SetTipHelpText( sal_uInt16( i+1 ), aTipHelpText );
                pVCLPopupMenu->SetItemCommand( sal_uInt16( i+1 ), aURLString );
			}
		}
        else
        {
            // No recent documents => insert "no document" string
            String aNoDocumentStr = String( FwkResId( STR_NODOCUMENT ));
            pVCLPopupMenu->InsertItem( 1, aNoDocumentStr );
            pVCLPopupMenu->EnableItem( 1, sal_False );
        }
	}
}

void RecentFilesMenuController::executeEntry( sal_Int32 nIndex )
{
    static int NUM_OF_PICKLIST_ARGS = 3;

    Reference< css::awt::XPopupMenu > xPopupMenu;
    Reference< XDispatch >            xDispatch;
    Reference< XDispatchProvider >    xDispatchProvider;
    Reference< XMultiServiceFactory > xServiceManager;

    osl::ClearableMutexGuard aLock( m_aMutex );
    xPopupMenu          = m_xPopupMenu;
    xDispatchProvider   = Reference< XDispatchProvider >( m_xFrame, UNO_QUERY );
    xServiceManager     = m_xServiceManager;
    aLock.clear();

    css::util::URL            aTargetURL;
    Sequence< PropertyValue > aArgsList;

    if (( nIndex >= 0 ) &&
        ( nIndex < sal::static_int_cast<sal_Int32>( m_aRecentFilesItems.size() )))
    {
        const RecentFile& rRecentFile = m_aRecentFilesItems[ nIndex ];

        aTargetURL.Complete = rRecentFile.aURL;
        m_xURLTransformer->parseStrict( aTargetURL );

        aArgsList.realloc( NUM_OF_PICKLIST_ARGS );
        aArgsList[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Referer" ));
        aArgsList[0].Value = makeAny( ::rtl::OUString::createFromAscii( SFX_REFERER_USER ));

        // documents in the picklist will never be opened as templates
        aArgsList[1].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "AsTemplate" ));
        aArgsList[1].Value = makeAny( (sal_Bool) sal_False );

        ::rtl::OUString  aFilter( rRecentFile.aFilter );
        sal_Int32 nPos = aFilter.indexOf( '|' );
        if ( nPos >= 0 )
        {
	        ::rtl::OUString aFilterOptions;

	        if ( nPos < ( aFilter.getLength() - 1 ) )
		        aFilterOptions = aFilter.copy( nPos+1 );

	        aArgsList[2].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FilterOptions" ));
	        aArgsList[2].Value <<= aFilterOptions;

	        aFilter = aFilter.copy( 0, nPos-1 );
	        aArgsList.realloc( ++NUM_OF_PICKLIST_ARGS );
        }

        aArgsList[NUM_OF_PICKLIST_ARGS-1].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FilterName" ));
        aArgsList[NUM_OF_PICKLIST_ARGS-1].Value <<= aFilter;

        xDispatch = xDispatchProvider->queryDispatch( aTargetURL, ::rtl::OUString::createFromAscii("_default"), 0 );
    }

    if ( xDispatch.is() )
    {
        // Call dispatch asychronously as we can be destroyed while dispatch is
        // executed. VCL is not able to survive this as it wants to call listeners
        // after select!!!
        LoadRecentFile* pLoadRecentFile = new LoadRecentFile;
        pLoadRecentFile->xDispatch  = xDispatch;
        pLoadRecentFile->aTargetURL = aTargetURL;
        pLoadRecentFile->aArgSeq    = aArgsList;
        if(::comphelper::UiEventsLogger::isEnabled()) //#i88653#
            UiEventLogHelper(::rtl::OUString::createFromAscii("RecentFilesMenuController")).log(m_xServiceManager, m_xFrame, aTargetURL, aArgsList);
        Application::PostUserEvent( STATIC_LINK(0, RecentFilesMenuController, ExecuteHdl_Impl), pLoadRecentFile );
    }
}

// XEventListener
void SAL_CALL RecentFilesMenuController::disposing( const EventObject& ) throw ( RuntimeException )
{
    Reference< css::awt::XMenuListener > xHolder(( OWeakObject *)this, UNO_QUERY );

    osl::MutexGuard aLock( m_aMutex );
    m_xFrame.clear();
    m_xDispatch.clear();
    m_xServiceManager.clear();

    if ( m_xPopupMenu.is() )
        m_xPopupMenu->removeMenuListener( Reference< css::awt::XMenuListener >(( OWeakObject *)this, UNO_QUERY ));
    m_xPopupMenu.clear();
}

// XStatusListener
void SAL_CALL RecentFilesMenuController::statusChanged( const FeatureStateEvent& Event ) throw ( RuntimeException )
{
    osl::MutexGuard aLock( m_aMutex );
    m_bDisabled = !Event.IsEnabled;
}

void SAL_CALL RecentFilesMenuController::select( const css::awt::MenuEvent& rEvent ) throw (RuntimeException)
{
    Reference< css::awt::XPopupMenu > xPopupMenu;
    Reference< XDispatch >            xDispatch;
    Reference< XDispatchProvider >    xDispatchProvider;
    Reference< XMultiServiceFactory > xServiceManager;

    osl::ClearableMutexGuard aLock( m_aMutex );
    xPopupMenu          = m_xPopupMenu;
    xDispatchProvider   = Reference< XDispatchProvider >( m_xFrame, UNO_QUERY );
    xServiceManager     = m_xServiceManager;
    aLock.clear();

    css::util::URL aTargetURL;
    Sequence< PropertyValue > aArgsList;

    if ( xPopupMenu.is() && xDispatchProvider.is() )
    {
        VCLXPopupMenu* pPopupMenu = (VCLXPopupMenu *)VCLXPopupMenu::GetImplementation( xPopupMenu );
        if ( pPopupMenu )
            executeEntry( rEvent.MenuId-1 );
    }
}

void SAL_CALL RecentFilesMenuController::activate( const css::awt::MenuEvent& ) throw (RuntimeException)
{
    osl::MutexGuard aLock( m_aMutex );
    impl_setPopupMenu();
}

// XPopupMenuController
void RecentFilesMenuController::impl_setPopupMenu()
{
    if ( m_xPopupMenu.is() )
        fillPopupMenu( m_xPopupMenu );
}

void SAL_CALL RecentFilesMenuController::updatePopupMenu() throw (RuntimeException)
{
    osl::ClearableMutexGuard aLock( m_aMutex );

    throwIfDisposed();

    Reference< XStatusListener > xStatusListener( static_cast< OWeakObject* >( this ), UNO_QUERY );
    Reference< XDispatch > xDispatch( m_xDispatch );
    com::sun::star::util::URL aTargetURL;
    aTargetURL.Complete = m_aCommandURL;
    m_xURLTransformer->parseStrict( aTargetURL );
    aLock.clear();

    // Add/remove status listener to get a status update once
    if ( xDispatch.is() )
    {
        xDispatch->addStatusListener( xStatusListener, aTargetURL );
        xDispatch->removeStatusListener( xStatusListener, aTargetURL );
    }
}

// XDispatchProvider
Reference< XDispatch > SAL_CALL RecentFilesMenuController::queryDispatch(
    const URL& aURL,
    const ::rtl::OUString& /*sTarget*/,
    sal_Int32 /*nFlags*/ )
throw( RuntimeException )
{
    osl::MutexGuard aLock( m_aMutex );

	throwIfDisposed();

    if ( aURL.Complete.indexOf( m_aBaseURL ) == 0 )
        return Reference< XDispatch >( static_cast< OWeakObject* >( this ), UNO_QUERY );
    else
        return Reference< XDispatch >();
}

// XDispatch
void SAL_CALL RecentFilesMenuController::dispatch(
    const URL& aURL,
    const Sequence< PropertyValue >& /*seqProperties*/ )
throw( RuntimeException )
{
    osl::MutexGuard aLock( m_aMutex );

	throwIfDisposed();

    if ( aURL.Complete.indexOf( m_aBaseURL ) == 0 )
    {
        // Parse URL to retrieve entry argument and its value
        sal_Int32 nQueryPart = aURL.Complete.indexOf( '?', m_aBaseURL.getLength() );
        if ( nQueryPart > 0 )
        {
            const rtl::OUString aEntryArgStr( RTL_CONSTASCII_USTRINGPARAM( "entry=" ));
            sal_Int32 nEntryArg = aURL.Complete.indexOf( aEntryArgStr, nQueryPart );
            sal_Int32 nEntryPos = nEntryArg + aEntryArgStr.getLength();
            if (( nEntryArg > 0 ) && ( nEntryPos < aURL.Complete.getLength() ))
            {
                sal_Int32 nAddArgs = aURL.Complete.indexOf( '&', nEntryPos );
                rtl::OUString aEntryArg;

                if ( nAddArgs < 0 )
                    aEntryArg = aURL.Complete.copy( nEntryPos );
                else
                    aEntryArg = aURL.Complete.copy( nEntryPos, nAddArgs-nEntryPos );

                sal_Int32 nEntry = aEntryArg.toInt32();
                executeEntry( nEntry );
            }
        }
    }
}

void SAL_CALL RecentFilesMenuController::addStatusListener(
    const Reference< XStatusListener >& xControl,
    const URL& aURL )
throw( RuntimeException )
{
    osl::MutexGuard aLock( m_aMutex );

	throwIfDisposed();

	svt::PopupMenuControllerBase::addStatusListener( xControl, aURL );
}

void SAL_CALL RecentFilesMenuController::removeStatusListener(
    const Reference< XStatusListener >& xControl,
    const URL& aURL )
throw( RuntimeException )
{
	svt::PopupMenuControllerBase::removeStatusListener( xControl, aURL );
}

IMPL_STATIC_LINK_NOINSTANCE( RecentFilesMenuController, ExecuteHdl_Impl, LoadRecentFile*, pLoadRecentFile )
{
    try
    {
        // Asynchronous execution as this can lead to our own destruction!
        // Framework can recycle our current frame and the layout manager disposes all user interface
        // elements if a component gets detached from its frame!
        pLoadRecentFile->xDispatch->dispatch( pLoadRecentFile->aTargetURL, pLoadRecentFile->aArgSeq );
    }
    catch ( Exception& )
    {
    }

    delete pLoadRecentFile;
    return 0;
}

}
