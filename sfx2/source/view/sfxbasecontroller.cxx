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
#include "precompiled_sfx2.hxx"

//________________________________________________________________________________________________________
//	my own includes
//________________________________________________________________________________________________________

#include <time.h>
#include <sfx2/sfxbasecontroller.hxx>

//________________________________________________________________________________________________________
//	include of other projects
//________________________________________________________________________________________________________
#include <com/sun/star/awt/KeyEvent.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>
#include <com/sun/star/awt/MouseEvent.hpp>
#include <com/sun/star/awt/MouseButton.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/util/XCloseBroadcaster.hpp>
#include <com/sun/star/util/XCloseListener.hpp>
#include <com/sun/star/util/CloseVetoException.hpp>
#include <com/sun/star/document/XViewDataSupplier.hpp>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/frame/FrameActionEvent.hpp>
#include <com/sun/star/frame/FrameAction.hpp>
#include <com/sun/star/frame/CommandGroup.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XBorderResizeListener.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/implbase1.hxx>
#include <basic/sbstar.hxx>
#include <uno/mapping.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/app.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/userinputinterception.hxx>

#include <viewimp.hxx>
#include <sfx2/unoctitm.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/sfxsids.hrc>
#include <workwin.hxx>
#include <sfx2/objface.hxx>

#include <vos/mutex.hxx>
#include <osl/mutex.hxx>
#include <tools/diagnose_ex.h>
#include <comphelper/sequence.hxx>
#include <rtl/ustrbuf.hxx>
#include <toolkit/helper/convert.hxx>
#include <framework/titlehelper.hxx>
#include <comphelper/processfactory.hxx>
#include <tools/diagnose_ex.h>

#include <hash_map>

#include <sfx2/event.hxx>
#include "sfx2/viewfac.hxx"

#define	OMULTITYPEINTERFACECONTAINERHELPER		::cppu::OMultiTypeInterfaceContainerHelper
#define OINTERFACECONTAINERHELPER               ::cppu::OInterfaceContainerHelper
#define	XFRAMEACTIONLISTENER					::com::sun::star::frame::XFrameActionListener
#define	XCLOSELISTENER							::com::sun::star::util::XCloseListener
#define	FRAMEACTIONEVENT						::com::sun::star::frame::FrameActionEvent
#define	EVENTOBJECT								::com::sun::star::lang::EventObject
#define OTYPECOLLECTION							::cppu::OTypeCollection
#define OIMPLEMENTATIONID						::cppu::OImplementationId
#define	MUTEXGUARD								::osl::MutexGuard
#define	UNOQUERY								::com::sun::star::uno::UNO_QUERY
#define	MAPPING									::com::sun::star::uno::Mapping
#define XSTATUSINDICATORSUPPLIER                ::com::sun::star::task::XStatusIndicatorSupplier
#define XCOMPONENT                              ::com::sun::star::lang::XComponent
#define	XINTERFACE								::com::sun::star::uno::XInterface
#define XKEYHANDLER								::com::sun::star::awt::XKeyHandler
#define XMOUSECLICKHANDLER						::com::sun::star::awt::XMouseClickHandler

#define TIMEOUT_START_RESCHEDULE	10L	/* 10th s */

using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::UNO_SET_THROW;
using ::com::sun::star::lang::DisposedException;
using ::com::sun::star::awt::XWindow;
using ::com::sun::star::frame::XController;
using ::com::sun::star::frame::XDispatchProvider;
using ::com::sun::star::document::XViewDataSupplier;
using ::com::sun::star::container::XIndexAccess;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::frame::XFrame;
using ::com::sun::star::frame::XFrameActionListener;
using ::com::sun::star::util::XCloseListener;
using ::com::sun::star::task::XStatusIndicator;
using ::com::sun::star::frame::XTitle;
namespace css = ::com::sun::star;

struct GroupIDToCommandGroup
{
    sal_Int16   nGroupID;
    sal_Int16   nCommandGroup;
};

// Please update when a new command group is added
const sal_Int16 MAX_COMMANDGROUP = com::sun::star::frame::CommandGroup::CONTROLS;

static sal_Bool                 bGroupIDMapInitialized = sal_False;
static GroupIDToCommandGroup    GroupIDCommandGroupMap[] =
{
    { GID_INTERN        ,   com::sun::star::frame::CommandGroup::INTERNAL       },
    { GID_APPLICATION   ,   com::sun::star::frame::CommandGroup::APPLICATION    },
    { GID_DOCUMENT      ,   com::sun::star::frame::CommandGroup::DOCUMENT       },
    { GID_VIEW		    ,   com::sun::star::frame::CommandGroup::VIEW           },
    { GID_EDIT          ,   com::sun::star::frame::CommandGroup::EDIT           },
    { GID_MACRO         ,   com::sun::star::frame::CommandGroup::MACRO          },
    { GID_OPTIONS       ,   com::sun::star::frame::CommandGroup::OPTIONS        },
    { GID_MATH  		,   com::sun::star::frame::CommandGroup::MATH           },
    { GID_NAVIGATOR	    ,   com::sun::star::frame::CommandGroup::NAVIGATOR      },
    { GID_INSERT		,   com::sun::star::frame::CommandGroup::INSERT         },
    { GID_FORMAT        ,   com::sun::star::frame::CommandGroup::FORMAT         },
    { GID_TEMPLATE      ,   com::sun::star::frame::CommandGroup::TEMPLATE       },
    { GID_TEXT          ,   com::sun::star::frame::CommandGroup::TEXT           },
    { GID_FRAME         ,   com::sun::star::frame::CommandGroup::FRAME          },
    { GID_GRAPHIC       ,   com::sun::star::frame::CommandGroup::GRAPHIC        },
    { GID_TABLE         ,   com::sun::star::frame::CommandGroup::TABLE          },
    { GID_ENUMERATION   ,   com::sun::star::frame::CommandGroup::ENUMERATION    },
    { GID_DATA          ,   com::sun::star::frame::CommandGroup::DATA           },
    { GID_SPECIAL       ,   com::sun::star::frame::CommandGroup::SPECIAL        },
    { GID_IMAGE         ,   com::sun::star::frame::CommandGroup::IMAGE          },
    { GID_CHART         ,   com::sun::star::frame::CommandGroup::CHART          },
    { GID_EXPLORER      ,   com::sun::star::frame::CommandGroup::EXPLORER       },
    { GID_CONNECTOR     ,   com::sun::star::frame::CommandGroup::CONNECTOR      },
    { GID_MODIFY        ,   com::sun::star::frame::CommandGroup::MODIFY         },
    { GID_DRAWING       ,   com::sun::star::frame::CommandGroup::DRAWING        },
    { GID_CONTROLS      ,   com::sun::star::frame::CommandGroup::CONTROLS       },
    { 0                 ,   0                                                   }
};

typedef std::hash_map< sal_Int16, sal_Int16 > GroupHashMap;


sal_Int16 MapGroupIDToCommandGroup( sal_Int16 nGroupID )
{
    static GroupHashMap mHashMap;

    if ( !bGroupIDMapInitialized )
    {
        sal_Int32 i = 0;
        while ( GroupIDCommandGroupMap[i].nGroupID != 0 )
        {
            mHashMap.insert( GroupHashMap::value_type(
                GroupIDCommandGroupMap[i].nGroupID,
                GroupIDCommandGroupMap[i].nCommandGroup ));
            ++i;
        }
    }

    GroupHashMap::const_iterator pIter = mHashMap.find( nGroupID );
    if ( pIter != mHashMap.end() )
        return pIter->second;
    else
        return com::sun::star::frame::CommandGroup::INTERNAL;
}

sal_Int16 MapCommandGroupToGroupID( sal_Int16 nCommandGroup )
{
    sal_Int32 i = 0;
    while ( GroupIDCommandGroupMap[i].nGroupID != 0 )
    {
        if ( GroupIDCommandGroupMap[i].nCommandGroup == nCommandGroup )
            return GroupIDCommandGroupMap[i].nGroupID;
        ++i;
    }

    return -1;
}

sal_Bool SupportsCommandGroup( sal_Int16 nCommandGroup )
{
    if (( nCommandGroup >= 0 ) && ( nCommandGroup <= MAX_COMMANDGROUP ))
        return sal_True;
    else
        return sal_False;
}

sal_uInt32 Get10ThSec()
{
	sal_uInt32 n10Ticks = 10 * (sal_uInt32)clock();
	return n10Ticks / CLOCKS_PER_SEC;
}

sal_Int32 m_nInReschedule = 0;	///	static counter for rescheduling

void reschedule()
{
	if ( m_nInReschedule == 0 )
	{
		++m_nInReschedule;
		Application::Reschedule();
		--m_nInReschedule;
	}
}

class SfxStatusIndicator : public ::cppu::WeakImplHelper2< ::com::sun::star::task::XStatusIndicator, ::com::sun::star::lang::XEventListener >
{
friend class SfxBaseController;
    ::com::sun::star::uno::Reference < XController > xOwner;
    ::com::sun::star::uno::Reference < ::com::sun::star::task::XStatusIndicator > xProgress;
    SfxWorkWindow*          pWorkWindow;
    sal_Int32               _nRange;
    sal_Int32               _nValue;
	long					_nStartTime;
public:
                            SfxStatusIndicator(SfxBaseController* pController, SfxWorkWindow* pWork)
                                : xOwner( pController )
                                , pWorkWindow( pWork )
                            {
								++m_refCount;
								::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > xComponent(
									SAL_STATIC_CAST(::cppu::OWeakObject*, pController ), ::com::sun::star::uno::UNO_QUERY );
								if (xComponent.is())
									xComponent->addEventListener(this);
								--m_refCount;
							}

    virtual void SAL_CALL   start(const ::rtl::OUString& aText, sal_Int32 nRange) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   end(void) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setText(const ::rtl::OUString& aText) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setValue(sal_Int32 nValue) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   reset() throw(::com::sun::star::uno::RuntimeException);

	virtual void SAL_CALL	disposing( const com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);
};

void SAL_CALL SfxStatusIndicator::start(const ::rtl::OUString& aText, sal_Int32 nRange) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( xOwner.is() )
	{
		_nRange = nRange;
		_nValue = 0;

        if ( !xProgress.is() )
            xProgress = pWorkWindow->GetStatusIndicator();

        if ( xProgress.is() )
            xProgress->start( aText, nRange );

		_nStartTime = Get10ThSec();
    	reschedule();
	}
}

void SAL_CALL SfxStatusIndicator::end(void) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( xOwner.is() )
	{
        if ( !xProgress.is() )
            xProgress = pWorkWindow->GetStatusIndicator();

        if ( xProgress.is() )
            xProgress->end();

        reschedule();
	}
}

void SAL_CALL SfxStatusIndicator::setText(const ::rtl::OUString& aText) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( xOwner.is() )
	{
        if ( !xProgress.is() )
            xProgress = pWorkWindow->GetStatusIndicator();

        if ( xProgress.is() )
            xProgress->setText( aText );

    	reschedule();
	}
}

void SAL_CALL SfxStatusIndicator::setValue( sal_Int32 nValue ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( xOwner.is() )
	{
		_nValue = nValue;

        if ( !xProgress.is() )
            xProgress = pWorkWindow->GetStatusIndicator();

        if ( xProgress.is() )
            xProgress->setValue( nValue );

        sal_Bool bReschedule = (( Get10ThSec() - _nStartTime ) > TIMEOUT_START_RESCHEDULE );
		if ( bReschedule )
			reschedule();
	}
}

void SAL_CALL SfxStatusIndicator::reset() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( xOwner.is() )
	{
        if ( !xProgress.is() )
            xProgress = pWorkWindow->GetStatusIndicator();

        if ( xProgress.is() )
            xProgress->reset();

        reschedule();
	}
}

void SAL_CALL SfxStatusIndicator::disposing( const com::sun::star::lang::EventObject& /*Source*/ ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
	xOwner = 0;
    xProgress.clear();
}

//________________________________________________________________________________________________________
//________________________________________________________________________________________________________
//	declaration IMPL_SfxBaseController_ListenerHelper
//________________________________________________________________________________________________________

class IMPL_SfxBaseController_ListenerHelper : public ::cppu::WeakImplHelper1< ::com::sun::star::frame::XFrameActionListener >
{
public:
	IMPL_SfxBaseController_ListenerHelper(	MUTEX&				aMutex		,
											SfxBaseController*	pController	) ;
	virtual ~IMPL_SfxBaseController_ListenerHelper() ;
    virtual void SAL_CALL frameAction( const FRAMEACTIONEVENT& aEvent ) throw (RUNTIMEEXCEPTION) ;
	virtual void SAL_CALL disposing( const EVENTOBJECT& aEvent ) throw (RUNTIMEEXCEPTION) ;

private:

	MUTEX&					m_aMutex		;
	SfxBaseController*		m_pController	;

} ;	// class IMPL_SfxBaseController_ListenerContainer

class IMPL_SfxBaseController_CloseListenerHelper : public ::cppu::WeakImplHelper1< ::com::sun::star::util::XCloseListener >
{
public:
	IMPL_SfxBaseController_CloseListenerHelper(	MUTEX&				aMutex		,
											SfxBaseController*	pController	) ;
	virtual ~IMPL_SfxBaseController_CloseListenerHelper() ;
    virtual void SAL_CALL queryClosing( const EVENTOBJECT& aEvent, sal_Bool bDeliverOwnership )
        throw (RUNTIMEEXCEPTION, com::sun::star::util::CloseVetoException) ;
    virtual void SAL_CALL notifyClosing( const EVENTOBJECT& aEvent ) throw (RUNTIMEEXCEPTION) ;
	virtual void SAL_CALL disposing( const EVENTOBJECT& aEvent ) throw (RUNTIMEEXCEPTION) ;

private:

	MUTEX&					m_aMutex;
	SfxBaseController*		m_pController;

} ;	// class IMPL_SfxBaseController_ListenerContainer

IMPL_SfxBaseController_CloseListenerHelper::IMPL_SfxBaseController_CloseListenerHelper(	MUTEX&				aMutex		,
																				SfxBaseController*	pController	)
		: m_aMutex		( aMutex		)
		, m_pController	( pController	)
{
}

IMPL_SfxBaseController_CloseListenerHelper::~IMPL_SfxBaseController_CloseListenerHelper()
{
}

void SAL_CALL IMPL_SfxBaseController_CloseListenerHelper::disposing( const EVENTOBJECT& /*aEvent*/ ) throw( ::com::sun::star::uno::RuntimeException )
{
}

void SAL_CALL IMPL_SfxBaseController_CloseListenerHelper::queryClosing( const EVENTOBJECT& aEvent, sal_Bool bDeliverOwnership )
    throw (RUNTIMEEXCEPTION, com::sun::star::util::CloseVetoException)
{
	::vos::OGuard aGuard( Application::GetSolarMutex() );
    SfxViewShell* pShell = m_pController->GetViewShell_Impl();
    if  ( m_pController !=  NULL &&  pShell )
	{
        sal_Bool bCanClose = (sal_Bool) pShell->PrepareClose( sal_False );
		if ( !bCanClose )
		{
            if ( bDeliverOwnership && ( !pShell->GetWindow() || !pShell->GetWindow()->IsReallyVisible() ) )
            {
                // ignore OwnerShip in case of visible frame (will be closed by user)
                uno::Reference < frame::XModel > xModel( aEvent.Source, uno::UNO_QUERY );
                if ( xModel.is() )
                    pShell->TakeOwnerShip_Impl();
                else
                    pShell->TakeFrameOwnerShip_Impl();
            }

			throw com::sun::star::util::CloseVetoException(::rtl::OUString::createFromAscii("Controller disagree ..."),static_cast< ::cppu::OWeakObject*>(this));
		}
	}
}

void SAL_CALL IMPL_SfxBaseController_CloseListenerHelper::notifyClosing( const EVENTOBJECT& /*aEvent*/ ) throw (RUNTIMEEXCEPTION)
{
}

//________________________________________________________________________________________________________
//	declaration IMPL_SfxBaseController_DataContainer
//________________________________________________________________________________________________________

struct IMPL_SfxBaseController_DataContainer
{
    Reference< XFrame >                     m_xFrame                ;
    Reference< XFrameActionListener >       m_xListener             ;
    Reference< XCloseListener >             m_xCloseListener        ;
    ::sfx2::UserInputInterception           m_aUserInputInterception;
	OMULTITYPEINTERFACECONTAINERHELPER		m_aListenerContainer	;
    OINTERFACECONTAINERHELPER               m_aInterceptorContainer ;
    Reference< XStatusIndicator >           m_xIndicator            ;
	SfxViewShell*							m_pViewShell			;
	SfxBaseController*						m_pController			;
	sal_Bool								m_bDisposing			;
	sal_Bool                                m_bSuspendState         ;
    Reference< XTitle >                     m_xTitleHelper          ;
    Sequence< PropertyValue >               m_aCreationArgs         ;

	IMPL_SfxBaseController_DataContainer(	MUTEX&				aMutex		,
											SfxViewShell*		pViewShell	,
											SfxBaseController*	pController	)
            :   m_xListener                     ( new IMPL_SfxBaseController_ListenerHelper( aMutex, pController ) )
            ,   m_xCloseListener                ( new IMPL_SfxBaseController_CloseListenerHelper( aMutex, pController ) )
            ,   m_aUserInputInterception        ( *pController, aMutex                                  )
            ,   m_aListenerContainer            ( aMutex												)
            ,   m_aInterceptorContainer         ( aMutex                                                )
            ,   m_pViewShell			( pViewShell											)
            ,   m_pController			( pController											)
            ,   m_bDisposing			( sal_False												)
            ,   m_bSuspendState                 ( sal_False                                              )
	{
	}

} ;	// struct IMPL_SfxBaseController_DataContainer

//________________________________________________________________________________________________________
//	IMPL_SfxBaseController_ListenerHelper constructor
//________________________________________________________________________________________________________

IMPL_SfxBaseController_ListenerHelper::IMPL_SfxBaseController_ListenerHelper(	MUTEX&				aMutex		,
																				SfxBaseController*	pController	)
		: m_aMutex		( aMutex		)
		, m_pController	( pController	)
{
}

//________________________________________________________________________________________________________
//	IMPL_SfxBaseController_ListenerHelper destructor
//________________________________________________________________________________________________________

IMPL_SfxBaseController_ListenerHelper::~IMPL_SfxBaseController_ListenerHelper()
{
}

void SAL_CALL IMPL_SfxBaseController_ListenerHelper::frameAction( const FRAMEACTIONEVENT& aEvent ) throw( RUNTIMEEXCEPTION )
{
	::vos::OGuard aGuard( Application::GetSolarMutex() );
	if	(
			( m_pController	!=	NULL ) &&
			( aEvent.Frame	==	m_pController->getFrame() )	&&
            ( m_pController->GetViewShell_Impl() && m_pController->GetViewShell_Impl()->GetWindow() !=  NULL                                                    )
		)
	{
        if ( aEvent.Action == ::com::sun::star::frame::FrameAction_FRAME_UI_ACTIVATED )
		{
            if ( !m_pController->GetViewShell_Impl()->GetUIActiveIPClient_Impl() )
                m_pController->GetViewShell_Impl()->GetViewFrame()->MakeActive_Impl( sal_False );
		}
        else if ( aEvent.Action == ::com::sun::star::frame::FrameAction_CONTEXT_CHANGED )
		{
			m_pController->GetViewShell_Impl()->GetViewFrame()->GetBindings().ContextChanged_Impl();
		}
	}
}

//________________________________________________________________________________________________________
//	IMPL_SfxBaseController_ListenerHelper -> XEventListener
//________________________________________________________________________________________________________

void SAL_CALL IMPL_SfxBaseController_ListenerHelper::disposing( const EVENTOBJECT& /*aEvent*/ ) throw( ::com::sun::star::uno::RuntimeException )
{
	::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( m_pController && m_pController->getFrame().is() )
		m_pController->getFrame()->removeFrameActionListener( this ) ;
}

//________________________________________________________________________________________________________
//	SfxBaseController -> constructor
//________________________________________________________________________________________________________
DBG_NAME(sfx2_SfxBaseController)
SfxBaseController::SfxBaseController( SfxViewShell* pViewShell )
    :   m_pData ( new IMPL_SfxBaseController_DataContainer( m_aMutex, pViewShell, this ))
{
    DBG_CTOR(sfx2_SfxBaseController,NULL);    
    m_pData->m_pViewShell->SetController( this );
}

//________________________________________________________________________________________________________
//	SfxBaseController -> destructor
//________________________________________________________________________________________________________

SfxBaseController::~SfxBaseController()
{
    DBG_DTOR(sfx2_SfxBaseController,NULL);    
	delete m_pData;
}

//________________________________________________________________________________________________________
//	SfxBaseController -> XController2
//________________________________________________________________________________________________________

Reference< XWindow > SAL_CALL SfxBaseController::getComponentWindow() throw (RuntimeException)
{
	::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( !m_pData->m_pViewShell )
        throw DisposedException();

    return Reference< XWindow >( GetViewFrame_Impl().GetFrame().GetWindow().GetComponentInterface(), UNO_QUERY_THROW );
}

::rtl::OUString SAL_CALL SfxBaseController::getViewControllerName() throw (RuntimeException)
{
	::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( !m_pData->m_pViewShell || !m_pData->m_pViewShell->GetObjectShell() )
        throw DisposedException();

    const SfxObjectFactory& rDocFac( m_pData->m_pViewShell->GetObjectShell()->GetFactory() );
    sal_uInt16 nViewNo = rDocFac.GetViewNo_Impl( GetViewFrame_Impl().GetCurViewId(), rDocFac.GetViewFactoryCount() );
    OSL_ENSURE( nViewNo < rDocFac.GetViewFactoryCount(), "SfxBaseController::getViewControllerName: view ID not found in view factories!" );

    ::rtl::OUString sViewName;
    if ( nViewNo < rDocFac.GetViewFactoryCount() )
        sViewName = rDocFac.GetViewFactory( nViewNo ).GetAPIViewName();

    return sViewName;
}

Sequence< PropertyValue > SAL_CALL SfxBaseController::getCreationArguments() throw (RuntimeException)
{
	::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( !m_pData->m_pViewShell || !m_pData->m_pViewShell->GetObjectShell() )
        throw DisposedException();

    return m_pData->m_aCreationArgs;
}

void SfxBaseController::SetCreationArguments_Impl( const Sequence< PropertyValue >& i_rCreationArgs )
{
    OSL_ENSURE( m_pData->m_aCreationArgs.getLength() == 0, "SfxBaseController::SetCreationArguments_Impl: not intended to be called twice!" );
    m_pData->m_aCreationArgs = i_rCreationArgs;
}

SfxViewFrame& SfxBaseController::GetViewFrame_Impl() const
{
    ENSURE_OR_THROW( m_pData->m_pViewShell, "not to be called without a view shell" );
    SfxViewFrame* pActFrame = m_pData->m_pViewShell->GetFrame();
    ENSURE_OR_THROW( pActFrame, "a view shell without a view frame is pretty pathological" );
    return *pActFrame;
}

//________________________________________________________________________________________________________
//	SfxBaseController -> XController2 -> XController
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseController::attachFrame( const REFERENCE< XFRAME >& xFrame ) throw( ::com::sun::star::uno::RuntimeException )
{
    REFERENCE< XFRAME > xTemp( getFrame() ) ;

	::vos::OGuard aGuard( Application::GetSolarMutex() );
	if ( xTemp.is() )
	{
        xTemp->removeFrameActionListener( m_pData->m_xListener ) ;
		REFERENCE < ::com::sun::star::util::XCloseBroadcaster > xCloseable( xTemp, com::sun::star::uno::UNO_QUERY );
		if ( xCloseable.is() )
			xCloseable->removeCloseListener( m_pData->m_xCloseListener );
	}

    m_pData->m_xFrame = xFrame;

	if ( xFrame.is() )
	{
        xFrame->addFrameActionListener( m_pData->m_xListener ) ;
		REFERENCE < ::com::sun::star::util::XCloseBroadcaster > xCloseable( xFrame, com::sun::star::uno::UNO_QUERY );
		if ( xCloseable.is() )
			xCloseable->addCloseListener( m_pData->m_xCloseListener );

		if ( m_pData->m_pViewShell )
		{
            ConnectSfxFrame_Impl( E_CONNECT );

            // attaching the frame to the controller is the last step in the creation of a new view, so notify this
            SfxViewEventHint aHint( SFX_EVENT_VIEWCREATED, GlobalEventConfig::GetEventName( STR_EVENT_VIEWCREATED ), m_pData->m_pViewShell->GetObjectShell(), uno::Reference< frame::XController2 >( this ) );
            SFX_APP()->NotifyEvent( aHint );
		}
	}
}

//________________________________________________________________________________________________________
//	SfxBaseController -> XController
//________________________________________________________________________________________________________

sal_Bool SAL_CALL SfxBaseController::attachModel( const REFERENCE< XMODEL >& xModel ) throw( ::com::sun::star::uno::RuntimeException )
{
	if ( m_pData->m_pViewShell && xModel.is() && xModel != m_pData->m_pViewShell->GetObjectShell()->GetModel() )
	{
		// don't allow to reattach a model!
		DBG_ERROR("Can't reattach model!");
		return sal_False;
	}

	REFERENCE < ::com::sun::star::util::XCloseBroadcaster > xCloseable( xModel, com::sun::star::uno::UNO_QUERY );
	if ( xCloseable.is() )
		xCloseable->addCloseListener( m_pData->m_xCloseListener );
	return sal_True;
}

//________________________________________________________________________________________________________
//	SfxBaseController -> XController
//________________________________________________________________________________________________________

sal_Bool SAL_CALL SfxBaseController::suspend( sal_Bool bSuspend ) throw( ::com::sun::star::uno::RuntimeException )
{
	::vos::OGuard aGuard( Application::GetSolarMutex() );

	// ignore dublicate calls, which doesnt change anything real
	if (bSuspend == m_pData->m_bSuspendState)
	   return sal_True;

	if ( bSuspend == sal_True )
	{
        if ( !m_pData->m_pViewShell )
        {
            m_pData->m_bSuspendState = sal_True;
            return sal_True;
        }

		if ( !m_pData->m_pViewShell->PrepareClose() )
            return sal_False;

        if ( getFrame().is() )
            getFrame()->removeFrameActionListener( m_pData->m_xListener ) ;
        SfxViewFrame* pActFrame = m_pData->m_pViewShell->GetFrame() ;

        // weitere View auf dasselbe Doc?
        SfxObjectShell*	pDocShell	=	m_pData->m_pViewShell->GetObjectShell()	;
        sal_Bool		bOther		=	sal_False								;

        for ( const SfxViewFrame* pFrame = SfxViewFrame::GetFirst( pDocShell );	!bOther && pFrame; pFrame = SfxViewFrame::GetNext( *pFrame, pDocShell ) )
            bOther = (pFrame != pActFrame);

        sal_Bool bRet = bOther || pDocShell->PrepareClose();
        if ( bRet )
        {
            ConnectSfxFrame_Impl( E_DISCONNECT );
            m_pData->m_bSuspendState = sal_True;
        }

        return bRet;
	}
	else
	{
		if ( getFrame().is() )
        	getFrame()->addFrameActionListener( m_pData->m_xListener ) ;

		if ( m_pData->m_pViewShell )
		{
            ConnectSfxFrame_Impl( E_RECONNECT );
		}

        m_pData->m_bSuspendState = sal_False;
		return sal_True ;
	}
}

//________________________________________________________________________________________________________
//	SfxBaseController -> XController
//________________________________________________________________________________________________________

ANY SfxBaseController::getViewData() throw( ::com::sun::star::uno::RuntimeException )
{
    ANY         aAny;
	String		sData1;
	::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( m_pData->m_pViewShell )
    {
        m_pData->m_pViewShell->WriteUserData( sData1 ) ;
        ::rtl::OUString    sData( sData1 );
        aAny <<= sData ;
    }

	return aAny ;
}

//________________________________________________________________________________________________________
//	SfxBaseController -> XController
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseController::restoreViewData( const ANY& aValue ) throw( ::com::sun::star::uno::RuntimeException )
{
	::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( m_pData->m_pViewShell )
    {
        ::rtl::OUString sData;
        aValue >>= sData ;
        m_pData->m_pViewShell->ReadUserData( sData ) ;
    }
}

//________________________________________________________________________________________________________
//	SfxBaseController -> XController
//________________________________________________________________________________________________________

REFERENCE< XFRAME > SAL_CALL SfxBaseController::getFrame() throw( ::com::sun::star::uno::RuntimeException )
{
	::vos::OGuard aGuard( Application::GetSolarMutex() );
    return m_pData->m_xFrame;
}

//________________________________________________________________________________________________________
//	SfxBaseController -> XController
//________________________________________________________________________________________________________

REFERENCE< XMODEL > SAL_CALL SfxBaseController::getModel() throw( ::com::sun::star::uno::RuntimeException )
{
	::vos::OGuard aGuard( Application::GetSolarMutex() );
    return m_pData->m_pViewShell ? m_pData->m_pViewShell->GetObjectShell()->GetModel() : REFERENCE < XMODEL > () ;
}

//________________________________________________________________________________________________________
//	SfxBaseController -> XDispatchProvider
//________________________________________________________________________________________________________

REFERENCE< XDISPATCH > SAL_CALL SfxBaseController::queryDispatch(	const	UNOURL&				aURL			,
																	const	::rtl::OUString&			sTargetFrameName,
																			sal_Int32			eSearchFlags	) throw( RUNTIMEEXCEPTION )
{
	::vos::OGuard aGuard( Application::GetSolarMutex() );
    REFERENCE< XDISPATCH >  xDisp;
    if ( m_pData->m_pViewShell )
    {
        SfxViewFrame*           pAct    = m_pData->m_pViewShell->GetViewFrame() ;
        if ( !m_pData->m_bDisposing )
        {
            if ( sTargetFrameName.compareToAscii( "_beamer" ) == COMPARE_EQUAL )
            {
                SfxViewFrame *pFrame = m_pData->m_pViewShell->GetViewFrame();
                if ( eSearchFlags & ( ::com::sun::star::frame::FrameSearchFlag::CREATE ))
                    pFrame->SetChildWindow( SID_BROWSER, sal_True );
                SfxChildWindow* pChildWin = pFrame->GetChildWindow( SID_BROWSER );
                REFERENCE < XFRAME > xFrame;
                if ( pChildWin )
                    xFrame = ( pChildWin->GetFrame() );
                if ( xFrame.is() )
                    xFrame->setName( sTargetFrameName );

                Reference< XDispatchProvider > xProv( xFrame, ::com::sun::star::uno::UNO_QUERY );
                if ( xProv.is() )
                    return xProv->queryDispatch( aURL, sTargetFrameName, ::com::sun::star::frame::FrameSearchFlag::SELF );
            }

            if ( aURL.Protocol.compareToAscii( ".uno:" ) == COMPARE_EQUAL )
            {
                rtl::OUString aMasterCommand = SfxOfficeDispatch::GetMasterUnoCommand( aURL );
                sal_Bool      bMasterCommand( aMasterCommand.getLength() > 0 );

                pAct = m_pData->m_pViewShell->GetViewFrame() ;
                SfxSlotPool& rSlotPool = SfxSlotPool::GetSlotPool( pAct );

                const SfxSlot* pSlot( 0 );
                if ( bMasterCommand )
                    pSlot = rSlotPool.GetUnoSlot( aMasterCommand );
                else
                    pSlot = rSlotPool.GetUnoSlot( aURL.Path );
                if ( pSlot && ( !pAct->GetFrame().IsInPlace() || !pSlot->IsMode( SFX_SLOT_CONTAINER ) ) )
                    return pAct->GetBindings().GetDispatch( pSlot, aURL, bMasterCommand );
				else
				{
					// try to find parent SfxViewFrame
					uno::Reference< frame::XFrame > xParentFrame;
					uno::Reference< frame::XFrame > xOwnFrame = pAct->GetFrame().GetFrameInterface();
					if ( xOwnFrame.is() )
						xParentFrame = uno::Reference< frame::XFrame >( xOwnFrame->getCreator(), uno::UNO_QUERY );

					if ( xParentFrame.is() )
					{
						// TODO/LATER: in future probably SfxViewFrame hirarchy should be the same as XFrame hirarchy
						// SfxViewFrame* pParentFrame = pAct->GetParentViewFrame();

						// search the related SfxViewFrame
						SfxViewFrame* pParentFrame = NULL;
						for ( SfxViewFrame* pFrame = SfxViewFrame::GetFirst();
								pFrame;
								pFrame = SfxViewFrame::GetNext( *pFrame ) )
						{
							if ( pFrame->GetFrame().GetFrameInterface() == xParentFrame )
							{
								pParentFrame = pFrame;
								break;
							}
						}

						if ( pParentFrame )
						{
                            SfxSlotPool& rFrameSlotPool = SfxSlotPool::GetSlotPool( pParentFrame );
                            const SfxSlot* pSlot2( 0 );
                            if ( bMasterCommand )
                                pSlot2 = rFrameSlotPool.GetUnoSlot( aMasterCommand );
                            else
                                pSlot2 = rFrameSlotPool.GetUnoSlot( aURL.Path );

                            if ( pSlot2 )
                                return pParentFrame->GetBindings().GetDispatch( pSlot2, aURL, bMasterCommand );
						}
					}
				}
            }
            else if ( aURL.Protocol.compareToAscii( "slot:" ) == COMPARE_EQUAL )
            {
                sal_uInt16 nId = (sal_uInt16) aURL.Path.toInt32();

                pAct = m_pData->m_pViewShell->GetViewFrame() ;
                if (nId >= SID_VERB_START && nId <= SID_VERB_END)
                {
                    const SfxSlot* pSlot = m_pData->m_pViewShell->GetVerbSlot_Impl(nId);
                    if ( pSlot )
                        return pAct->GetBindings().GetDispatch( pSlot, aURL, sal_False );
                }

                SfxSlotPool& rSlotPool = SfxSlotPool::GetSlotPool( pAct );
                const SfxSlot* pSlot = rSlotPool.GetSlot( nId );
                if ( pSlot && ( !pAct->GetFrame().IsInPlace() || !pSlot->IsMode( SFX_SLOT_CONTAINER ) ) )
                    return pAct->GetBindings().GetDispatch( pSlot, aURL, sal_False );
				else
				{
					// try to find parent SfxViewFrame
					uno::Reference< frame::XFrame > xParentFrame;
					uno::Reference< frame::XFrame > xOwnFrame = pAct->GetFrame().GetFrameInterface();
					if ( xOwnFrame.is() )
						xParentFrame = uno::Reference< frame::XFrame >( xOwnFrame->getCreator(), uno::UNO_QUERY );

					if ( xParentFrame.is() )
					{
						// TODO/LATER: in future probably SfxViewFrame hirarchy should be the same as XFrame hirarchy
						// SfxViewFrame* pParentFrame = pAct->GetParentViewFrame();

						// search the related SfxViewFrame
						SfxViewFrame* pParentFrame = NULL;
						for ( SfxViewFrame* pFrame = SfxViewFrame::GetFirst();
								pFrame;
								pFrame = SfxViewFrame::GetNext( *pFrame ) )
						{
							if ( pFrame->GetFrame().GetFrameInterface() == xParentFrame )
							{
								pParentFrame = pFrame;
								break;
							}
						}

						if ( pParentFrame )
						{
                            SfxSlotPool& rSlotPool2 = SfxSlotPool::GetSlotPool( pParentFrame );
                			const SfxSlot* pSlot2 = rSlotPool2.GetUnoSlot( aURL.Path );
                			if ( pSlot2 )
                                return pParentFrame->GetBindings().GetDispatch( pSlot2, aURL, sal_False );
						}
					}
				}
            }
            else if( sTargetFrameName.compareToAscii( "_self" )==COMPARE_EQUAL || sTargetFrameName.getLength()==0 )
            {
                // check for already loaded URL ... but with additional jumpmark!
                REFERENCE< XMODEL > xModel = getModel();
                if( xModel.is() && aURL.Mark.getLength() )
                {
                    SfxSlotPool& rSlotPool = SfxSlotPool::GetSlotPool( pAct );
                    const SfxSlot* pSlot = rSlotPool.GetSlot( SID_JUMPTOMARK );
                    if( aURL.Main.getLength() && aURL.Main == xModel->getURL() && pSlot )
                        return REFERENCE< XDISPATCH >( new SfxOfficeDispatch( pAct->GetBindings(), pAct->GetDispatcher(), pSlot, aURL) );
                }
            }
        }
    }

	return xDisp;
}

//________________________________________________________________________________________________________
//	SfxBaseController -> XDispatchProvider
//________________________________________________________________________________________________________

uno::Sequence< REFERENCE< XDISPATCH > > SAL_CALL SfxBaseController::queryDispatches( const uno::Sequence< DISPATCHDESCRIPTOR >& seqDescripts ) throw( ::com::sun::star::uno::RuntimeException )
{
    // Create return list - which must have same size then the given descriptor
    // It's not allowed to pack it!
    sal_Int32 nCount = seqDescripts.getLength();
    uno::Sequence< REFERENCE< XDISPATCH > > lDispatcher( nCount );

    for( sal_Int32 i=0; i<nCount; ++i )
    {
        lDispatcher[i] = queryDispatch( seqDescripts[i].FeatureURL  ,
                                        seqDescripts[i].FrameName   ,
                                        seqDescripts[i].SearchFlags );
    }

    return lDispatcher;
}

//________________________________________________________________________________________________________
//	SfxBaseController -> XControllerBorder
//________________________________________________________________________________________________________

frame::BorderWidths SAL_CALL SfxBaseController::getBorder()
	throw ( uno::RuntimeException )
{
	frame::BorderWidths aResult;

	::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( m_pData->m_pViewShell )
	{
		SvBorder aBorder = m_pData->m_pViewShell->GetBorderPixel();
		aResult.Left = aBorder.Left();
		aResult.Top = aBorder.Top();
		aResult.Right = aBorder.Right();
		aResult.Bottom = aBorder.Bottom();
	}

	return aResult;
}

void SAL_CALL SfxBaseController::addBorderResizeListener( const uno::Reference< frame::XBorderResizeListener >& xListener )
	throw ( uno::RuntimeException )
{
	m_pData->m_aListenerContainer.addInterface( ::getCppuType((const uno::Reference< frame::XBorderResizeListener >*)0),
												xListener );
}

void SAL_CALL SfxBaseController::removeBorderResizeListener( const uno::Reference< frame::XBorderResizeListener >& xListener )
	throw ( uno::RuntimeException )
{
	m_pData->m_aListenerContainer.removeInterface( ::getCppuType((const uno::Reference< frame::XBorderResizeListener >*)0),
												xListener );
}

awt::Rectangle SAL_CALL SfxBaseController::queryBorderedArea( const awt::Rectangle& aPreliminaryRectangle )
	throw ( uno::RuntimeException )
{
	::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( m_pData->m_pViewShell )
	{
		Rectangle aTmpRect = VCLRectangle( aPreliminaryRectangle );
		m_pData->m_pViewShell->QueryObjAreaPixel( aTmpRect );
		return AWTRectangle( aTmpRect );
	}

	return aPreliminaryRectangle;
}

void SfxBaseController::BorderWidthsChanged_Impl()
{
   	::cppu::OInterfaceContainerHelper* pContainer = m_pData->m_aListenerContainer.getContainer(
						::getCppuType( ( const uno::Reference< frame::XBorderResizeListener >*) NULL ) );
    if ( pContainer )
	{
		frame::BorderWidths aBWidths = getBorder();
		uno::Reference< uno::XInterface > xThis( static_cast< ::cppu::OWeakObject* >(this), uno::UNO_QUERY );

	    ::cppu::OInterfaceIteratorHelper pIterator(*pContainer);
	    while (pIterator.hasMoreElements())
	    {
	        try
	        {
	            ((frame::XBorderResizeListener*)pIterator.next())->borderWidthsChanged( xThis, aBWidths );
	        }
	        catch( uno::RuntimeException& )
	        {
	            pIterator.remove();
	        }
	    }
	}
}

//________________________________________________________________________________________________________
//	SfxBaseController -> XComponent
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseController::dispose() throw( ::com::sun::star::uno::RuntimeException )
{
	::vos::OGuard aGuard( Application::GetSolarMutex() );
    Reference< XController > xTmp( this );
	m_pData->m_bDisposing = sal_True ;

	EVENTOBJECT aEventObject;
    aEventObject.Source = *this ;
	m_pData->m_aListenerContainer.disposeAndClear( aEventObject ) ;

    if ( m_pData->m_pController && m_pData->m_pController->getFrame().is() )
        m_pData->m_pController->getFrame()->removeFrameActionListener( m_pData->m_xListener ) ;

    if ( m_pData->m_pViewShell )
    {
        SfxViewFrame* pFrame = m_pData->m_pViewShell->GetViewFrame() ;
        if ( pFrame && pFrame->GetViewShell() == m_pData->m_pViewShell )
            pFrame->GetFrame().SetIsClosing_Impl();
        m_pData->m_pViewShell->DiscardClients_Impl();
        m_pData->m_pViewShell->pImp->m_bControllerSet = false;

        if ( pFrame )
        {
            EVENTOBJECT aObject;
            aObject.Source = *this ;

            SfxObjectShell* pDoc = pFrame->GetObjectShell() ;
            SfxViewFrame *pView = SfxViewFrame::GetFirst(pDoc);
            while( pView )
            {
                // if there is another ViewFrame or currently the ViewShell in my ViewFrame is switched (PagePreview)
                if ( pView != pFrame || pView->GetViewShell() != m_pData->m_pViewShell )
                    break;
                pView = SfxViewFrame::GetNext( *pView, pDoc );
            }

            SFX_APP()->NotifyEvent( SfxViewEventHint(SFX_EVENT_CLOSEVIEW, GlobalEventConfig::GetEventName( STR_EVENT_CLOSEVIEW ), pDoc, uno::Reference< frame::XController2 >( this ) ) );
            if ( !pView )
                SFX_APP()->NotifyEvent( SfxEventHint(SFX_EVENT_CLOSEDOC, GlobalEventConfig::GetEventName( STR_EVENT_CLOSEDOC ), pDoc) );

            REFERENCE< XMODEL > xModel = pDoc->GetModel();
			REFERENCE < ::com::sun::star::util::XCloseable > xCloseable( xModel, com::sun::star::uno::UNO_QUERY );
            if ( xModel.is() )
			{
                xModel->disconnectController( this );
				if ( xCloseable.is() )
					xCloseable->removeCloseListener( m_pData->m_xCloseListener );
			}

            REFERENCE < XFRAME > aXFrame;
			attachFrame( aXFrame );

            m_pData->m_xListener->disposing( aObject );
            SfxViewShell *pShell = m_pData->m_pViewShell;
            m_pData->m_pViewShell = NULL;
            if ( pFrame->GetViewShell() == pShell )
            {
                // Enter registrations only allowed if we are the owner!
                if ( pFrame->GetFrame().OwnsBindings_Impl() )
                    pFrame->GetBindings().ENTERREGISTRATIONS();
                pFrame->GetFrame().SetFrameInterface_Impl(  aXFrame );
                pFrame->GetFrame().DoClose_Impl();
            }
        }
    }
}

//________________________________________________________________________________________________________
//	SfxBaseController -> XComponent
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseController::addEventListener( const REFERENCE< XEVENTLISTENER >& aListener ) throw( ::com::sun::star::uno::RuntimeException )
{
	m_pData->m_aListenerContainer.addInterface( ::getCppuType((const REFERENCE< XEVENTLISTENER >*)0), aListener );
}

//________________________________________________________________________________________________________
//	SfxBaseController -> XComponent
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseController::removeEventListener( const REFERENCE< XEVENTLISTENER >& aListener ) throw( ::com::sun::star::uno::RuntimeException )
{
	m_pData->m_aListenerContainer.removeInterface( ::getCppuType((const REFERENCE< XEVENTLISTENER >*)0), aListener );
}

void SfxBaseController::ReleaseShell_Impl()
{
	::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( m_pData->m_pViewShell )
    {
        SfxObjectShell* pDoc = m_pData->m_pViewShell->GetObjectShell() ;
        REFERENCE< XMODEL > xModel = pDoc->GetModel();
        REFERENCE < ::com::sun::star::util::XCloseable > xCloseable( xModel, com::sun::star::uno::UNO_QUERY );
        if ( xModel.is() )
        {
            xModel->disconnectController( this );
            if ( xCloseable.is() )
                xCloseable->removeCloseListener( m_pData->m_xCloseListener );
        }
        m_pData->m_pViewShell = 0;

        REFERENCE < XFRAME > aXFrame;
		attachFrame( aXFrame );
    }
}

SfxViewShell* SfxBaseController::GetViewShell_Impl() const
{
    return m_pData->m_pViewShell;
}

::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator > SAL_CALL SfxBaseController::getStatusIndicator(  ) throw (::com::sun::star::uno::RuntimeException)
{
	::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( m_pData->m_pViewShell && !m_pData->m_xIndicator.is() )
        m_pData->m_xIndicator = new SfxStatusIndicator( this, m_pData->m_pViewShell->GetViewFrame()->GetFrame().GetWorkWindow_Impl() );
    return m_pData->m_xIndicator;
}

void SAL_CALL SfxBaseController::registerContextMenuInterceptor( const REFERENCE< XCONTEXTMENUINTERCEPTOR >& xInterceptor ) throw( RUNTIMEEXCEPTION )

{
    m_pData->m_aInterceptorContainer.addInterface( xInterceptor );

	::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( m_pData->m_pViewShell )
        m_pData->m_pViewShell->AddContextMenuInterceptor_Impl( xInterceptor );
}

void SAL_CALL SfxBaseController::releaseContextMenuInterceptor( const REFERENCE< XCONTEXTMENUINTERCEPTOR >& xInterceptor ) throw( RUNTIMEEXCEPTION )

{
    m_pData->m_aInterceptorContainer.removeInterface( xInterceptor );

	::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( m_pData->m_pViewShell )
        m_pData->m_pViewShell->RemoveContextMenuInterceptor_Impl( xInterceptor );
}

void SAL_CALL SfxBaseController::addKeyHandler( const ::com::sun::star::uno::Reference< XKEYHANDLER >& xHandler ) throw (::com::sun::star::uno::RuntimeException)
{
	::vos::OGuard aGuard( Application::GetSolarMutex() );
    m_pData->m_aUserInputInterception.addKeyHandler( xHandler );
}

void SAL_CALL SfxBaseController::removeKeyHandler( const ::com::sun::star::uno::Reference< XKEYHANDLER >& xHandler ) throw (::com::sun::star::uno::RuntimeException)
{
	::vos::OGuard aGuard( Application::GetSolarMutex() );
    m_pData->m_aUserInputInterception.removeKeyHandler( xHandler );
}

void SAL_CALL SfxBaseController::addMouseClickHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseClickHandler >& xHandler ) throw (::com::sun::star::uno::RuntimeException)
{
	::vos::OGuard aGuard( Application::GetSolarMutex() );
    m_pData->m_aUserInputInterception.addMouseClickHandler( xHandler );
}

void SAL_CALL SfxBaseController::removeMouseClickHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseClickHandler >& xHandler ) throw (::com::sun::star::uno::RuntimeException)
{
	::vos::OGuard aGuard( Application::GetSolarMutex() );
    m_pData->m_aUserInputInterception.removeMouseClickHandler( xHandler );
}

::com::sun::star::uno::Sequence< sal_Int16 > SAL_CALL SfxBaseController::getSupportedCommandGroups()
throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    std::list< sal_Int16 > aGroupList;
    SfxViewFrame* pViewFrame( m_pData->m_pViewShell->GetFrame() );
    SfxSlotPool*  pPool = &SfxSlotPool::GetSlotPool( pViewFrame );

    SfxSlotPool* pSlotPool = pPool ? pPool : &SFX_SLOTPOOL();
    const sal_uIntPtr nMode( SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|SFX_SLOT_MENUCONFIG );

    // Gruppe anw"ahlen ( Gruppe 0 ist intern )
    for ( sal_uInt16 i=0; i<pSlotPool->GetGroupCount(); i++ )
	{
		String aName = pSlotPool->SeekGroup( i );
        const SfxSlot* pSfxSlot = pSlotPool->FirstSlot();
        while ( pSfxSlot )
		{
            if ( pSfxSlot->GetMode() & nMode )
			{
                sal_Int16 nCommandGroup = MapGroupIDToCommandGroup( pSfxSlot->GetGroupId() );
                aGroupList.push_back( nCommandGroup );
                break;
			}
			pSfxSlot = pSlotPool->NextSlot();
		}
    }

    ::com::sun::star::uno::Sequence< sal_Int16 > aSeq =
        comphelper::containerToSequence< sal_Int16 >( aGroupList );
    return aSeq;
}

::com::sun::star::uno::Sequence< ::com::sun::star::frame::DispatchInformation > SAL_CALL SfxBaseController::getConfigurableDispatchInformation( sal_Int16 nCmdGroup )
throw (::com::sun::star::uno::RuntimeException)
{
    std::list< ::com::sun::star::frame::DispatchInformation > aCmdList;

    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( m_pData->m_pViewShell )
    {
        const sal_uIntPtr nMode( SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|SFX_SLOT_MENUCONFIG );

        SfxViewFrame* pViewFrame( m_pData->m_pViewShell->GetFrame() );
        SfxSlotPool*  pPool( &SfxSlotPool::GetSlotPool( pViewFrame ));
        rtl::OUString aCmdPrefix( RTL_CONSTASCII_USTRINGPARAM( ".uno:" ));

        SfxSlotPool* pSlotPool = pPool ? pPool : &SFX_SLOTPOOL();
        for ( sal_uInt16 i=0; i<pSlotPool->GetGroupCount(); i++ )
		{
			String aName = pSlotPool->SeekGroup( i );
			const SfxSlot* pSfxSlot = pSlotPool->FirstSlot();
			if ( pSfxSlot )
			{
                sal_Int16 nCommandGroup = MapGroupIDToCommandGroup( pSfxSlot->GetGroupId() );
                if ( nCommandGroup == nCmdGroup )
                {
                    while ( pSfxSlot )
				    {
                        if ( pSfxSlot->GetMode() & nMode )
					    {
                            ::com::sun::star::frame::DispatchInformation aCmdInfo;
                            ::rtl::OUStringBuffer aBuf( aCmdPrefix );
                            aBuf.appendAscii( pSfxSlot->GetUnoName() );
                            aCmdInfo.Command = aBuf.makeStringAndClear();
                            aCmdInfo.GroupId = nCommandGroup;
                            aCmdList.push_back( aCmdInfo );
					    }
					    pSfxSlot = pSlotPool->NextSlot();
				    }
                }
		    }
        }
    }

    ::com::sun::star::uno::Sequence< ::com::sun::star::frame::DispatchInformation > aSeq =
        comphelper::containerToSequence< ::com::sun::star::frame::DispatchInformation, std::list< ::com::sun::star::frame::DispatchInformation > >( aCmdList );

    return aSeq;
}

sal_Bool SfxBaseController::HandleEvent_Impl( NotifyEvent& rEvent )
{
    return m_pData->m_aUserInputInterception.handleNotifyEvent( rEvent );
}

sal_Bool SfxBaseController::HasKeyListeners_Impl()
{
    return m_pData->m_aUserInputInterception.hasKeyHandlers();
}

sal_Bool SfxBaseController::HasMouseClickListeners_Impl()
{
    return m_pData->m_aUserInputInterception.hasMouseClickListeners();
}

void SfxBaseController::ConnectSfxFrame_Impl( const ConnectSfxFrame i_eConnect )
{
    ENSURE_OR_THROW( m_pData->m_pViewShell, "not to be called without a view shell" );
    SfxViewFrame* pViewFrame = m_pData->m_pViewShell->GetFrame();
    ENSURE_OR_THROW( pViewFrame, "a view shell without a view frame is pretty pathological" );

    const bool bConnect = ( i_eConnect != E_DISCONNECT );

    // disable window and dispatcher
    pViewFrame->Enable( bConnect );
    pViewFrame->GetDispatcher()->Lock( !bConnect );

    if ( bConnect )
    {
        if ( i_eConnect == E_CONNECT )
        {
            if  (   ( m_pData->m_pViewShell->GetObjectShell() != NULL )
                &&  ( m_pData->m_pViewShell->GetObjectShell()->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
                )
            {
                SfxViewFrame* pViewFrm = m_pData->m_pViewShell->GetViewFrame();
                if ( !pViewFrm->GetFrame().IsInPlace() )
                {
                    // for outplace embedded objects, we want the layout manager to keep the content window
                    // size constant, if possible
                    try
                    {
                        uno::Reference< beans::XPropertySet > xFrameProps( m_pData->m_xFrame, uno::UNO_QUERY_THROW );
                        uno::Reference< beans::XPropertySet > xLayouterProps(
                            xFrameProps->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "LayoutManager" ) ) ), uno::UNO_QUERY_THROW );
                        xLayouterProps->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PreserveContentSize" ) ), uno::makeAny( sal_True ) );
                    }
                    catch( const uno::Exception& )
                    {
                	    DBG_UNHANDLED_EXCEPTION();
                    }
                }
            }
        }

        // upon DISCONNECT, we did *not* pop the shells from the stack (this is done elsewhere), so upon
        // RECONNECT, we're not allowed to push them
        if ( i_eConnect != E_RECONNECT )
        {
            pViewFrame->GetDispatcher()->Push( *m_pData->m_pViewShell );
            if ( m_pData->m_pViewShell->GetSubShell() )
                pViewFrame->GetDispatcher()->Push( *m_pData->m_pViewShell->GetSubShell() );
	        m_pData->m_pViewShell->PushSubShells_Impl();
            pViewFrame->GetDispatcher()->Flush();
        }

        Window* pEditWin = m_pData->m_pViewShell->GetWindow();
        if ( pEditWin && m_pData->m_pViewShell->IsShowView_Impl() )
            pEditWin->Show();

        if ( SfxViewFrame::Current() == pViewFrame )
            pViewFrame->GetDispatcher()->Update_Impl( sal_True );

        Window* pFrameWin = &pViewFrame->GetWindow();
        if ( pFrameWin != &pViewFrame->GetFrame().GetWindow() )
	        pFrameWin->Show();

        if ( i_eConnect == E_CONNECT )
        {
            ::comphelper::NamedValueCollection aDocumentArgs( getModel()->getArgs() );

            const sal_Int16 nPluginMode = aDocumentArgs.getOrDefault( "PluginMode", sal_Int16( 0 ) );
            const bool bHasPluginMode = ( nPluginMode != 0 );

            SfxFrame& rFrame = pViewFrame->GetFrame();
            SfxObjectShell& rDoc = *m_pData->m_pViewShell->GetObjectShell();
            if ( !rFrame.IsMarkedHidden_Impl() )
	        {
                if ( rDoc.IsHelpDocument() || ( nPluginMode == 2 ) )
                    pViewFrame->GetDispatcher()->HideUI( sal_True );
                else
                    pViewFrame->GetDispatcher()->HideUI( sal_False );

                if ( rFrame.IsInPlace() )
                    pViewFrame->LockAdjustPosSizePixel();

                if ( nPluginMode == 3 )
                    rFrame.GetWorkWindow_Impl()->SetInternalDockingAllowed( sal_False );

                if ( !rFrame.IsInPlace() )
			        pViewFrame->GetDispatcher()->Update_Impl();
		        pViewFrame->Show();
                rFrame.GetWindow().Show();
                if ( !rFrame.IsInPlace() || ( nPluginMode == 3 ) )
                    pViewFrame->MakeActive_Impl( rFrame.GetFrameInterface()->isActive() );

                if ( rFrame.IsInPlace() )
                {
                    pViewFrame->UnlockAdjustPosSizePixel();
                    // force resize for OLE server to fix layout problems of writer and math
                    // see i53651
                    if ( nPluginMode == 3 )
                        pViewFrame->Resize( sal_True );
                }
	        }
            else
            {
                DBG_ASSERT( !rFrame.IsInPlace() && !bHasPluginMode, "Special modes not compatible with hidden mode!" );
                rFrame.GetWindow().Show();
            }

	        // Jetzt UpdateTitle, hidden TopFrames haben sonst keinen Namen!
	        pViewFrame->UpdateTitle();

            if ( !rFrame.IsInPlace() )
                pViewFrame->Resize( sal_True );

            // if there's a JumpMark given, then, well, jump to it
            ::comphelper::NamedValueCollection aViewArgs( getCreationArguments() );
            const ::rtl::OUString sJumpMark = aViewArgs.getOrDefault( "JumpMark", ::rtl::OUString() );
            const bool bHasJumpMark = ( sJumpMark.getLength() > 0 );
            OSL_ENSURE( ( !m_pData->m_pViewShell->GetObjectShell()->IsLoading() )
                    ||  ( !sJumpMark.getLength() ),
                "SfxBaseController::ConnectSfxFrame_Impl: so this code wasn't dead?" );
                // Before CWS autorecovery, there was code which postponed jumping to the Mark to a later time
                // (SfxObjectShell::PositionView_Impl), but it seems this branch was never used, since this method
                // here is never called before the load process finished. At least not with a non-empty jump mark
            if ( sJumpMark.getLength() )
		        m_pData->m_pViewShell->JumpToMark( sJumpMark );

            // if no plugin mode and no jump mark was supplied, check whether the document itself can provide view data, and
            // if so, forward it to the view/shell.
            if ( !bHasPluginMode && !bHasJumpMark )
            {
                // Note that this might not be the ideal place here. Restoring view data should, IMO, be the
                // responsibility of the loader, not an implementation detail burried here deep within the controller's
                // implementation.
                // What I think should be done to replace the below code:
                // - change SfxBaseController::restoreViewData to also accept a PropertyValue[] (it currently accepts
                //   a string only), and forward it to its ViewShell's ReadUserDataSequence
                // - change the frame loader so that when a new document is loaded (as opposed to an existing
                //   document being loaded into a new frame), the model's view data is examine the very same
                //   way as below, and the proper view data is set via XController::restoreViewData
                // - extend SfxViewFrame::SwitchToViewShell_Impl. Currently, it cares for the case where a non-PrintPreview
                //   view is exchanged, and sets the old view's data at the model. It should also care for the other
                //   way, were the PrintPreview view is left: in this case, the new view should also be initialized
                //   with the model's view data
                try
                {
                    Reference< XViewDataSupplier > xViewDataSupplier( getModel(), UNO_QUERY_THROW );
                    Reference< XIndexAccess > xViewData( xViewDataSupplier->getViewData() );

                    // find the view data item whose ViewId matches the ID of the view we're just connecting to
                    const SfxObjectFactory& rDocFactory( rDoc.GetFactory() );
                    const sal_Int32 nCount = xViewData.is() ? xViewData->getCount() : 0;
                    sal_Int32 nViewDataIndex = 0;
                    for ( sal_Int32 i=0; i<nCount; ++i )
                    {
                        const ::comphelper::NamedValueCollection aViewData( xViewData->getByIndex(i) );
                        ::rtl::OUString sViewId( aViewData.getOrDefault( "ViewId", ::rtl::OUString() ) );
                        if ( sViewId.getLength() == 0 )
                            continue;

                        const SfxViewFactory* pViewFactory = rDocFactory.GetViewFactoryByViewName( sViewId );
                        if ( pViewFactory == NULL )
                            continue;

                        if ( pViewFactory->GetOrdinal() == pViewFrame->GetCurViewId() )
                        {
                            nViewDataIndex = i;
                            break;
                        }
                    }
                    if ( nViewDataIndex < nCount )
                    {
                        Sequence< PropertyValue > aViewData;
                        OSL_VERIFY( xViewData->getByIndex( nViewDataIndex ) >>= aViewData );
                        if ( aViewData.getLength() > 0 )
                            m_pData->m_pViewShell->ReadUserDataSequence( aViewData, sal_True );
                    }
                }
                catch( const Exception& )
                {
        	        DBG_UNHANDLED_EXCEPTION();
                }
            }
        }
    }

    // invalidate slot corresponding to the view shell
    const sal_uInt16 nViewNo = m_pData->m_pViewShell->GetObjectShell()->GetFactory().GetViewNo_Impl( pViewFrame->GetCurViewId(), USHRT_MAX );
    DBG_ASSERT( nViewNo != USHRT_MAX, "view shell id not found" );
    if ( nViewNo != USHRT_MAX )
        pViewFrame->GetBindings().Invalidate( nViewNo + SID_VIEWSHELL0 );
}

//=============================================================================
css::uno::Reference< css::frame::XTitle > SfxBaseController::impl_getTitleHelper ()
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    if ( ! m_pData->m_xTitleHelper.is ())
    {
        css::uno::Reference< css::frame::XModel >           xModel           = getModel ();
        css::uno::Reference< css::frame::XUntitledNumbers > xUntitledProvider(xModel                                       , css::uno::UNO_QUERY      );
        css::uno::Reference< css::frame::XController >      xThis            (static_cast< css::frame::XController* >(this), css::uno::UNO_QUERY_THROW);
    
        ::framework::TitleHelper* pHelper                 = new ::framework::TitleHelper(::comphelper::getProcessServiceFactory());
                                  m_pData->m_xTitleHelper = css::uno::Reference< css::frame::XTitle >(static_cast< ::cppu::OWeakObject* >(pHelper), css::uno::UNO_QUERY_THROW);
    
        pHelper->setOwner                   (xThis            );
        pHelper->connectWithUntitledNumbers (xUntitledProvider);
    }

    return m_pData->m_xTitleHelper;
}

//=============================================================================
// css::frame::XTitle
::rtl::OUString SAL_CALL SfxBaseController::getTitle()
    throw (css::uno::RuntimeException)
{
    return impl_getTitleHelper()->getTitle ();
}

//=============================================================================
// css::frame::XTitle
void SAL_CALL SfxBaseController::setTitle(const ::rtl::OUString& sTitle)
    throw (css::uno::RuntimeException)
{
    impl_getTitleHelper()->setTitle (sTitle);
}
    
//=============================================================================
// css::frame::XTitleChangeBroadcaster
void SAL_CALL SfxBaseController::addTitleChangeListener(const css::uno::Reference< css::frame::XTitleChangeListener >& xListener)
    throw (css::uno::RuntimeException)
{
    css::uno::Reference< css::frame::XTitleChangeBroadcaster > xBroadcaster(impl_getTitleHelper(), css::uno::UNO_QUERY);
    if (xBroadcaster.is ())
        xBroadcaster->addTitleChangeListener (xListener);
}

//=============================================================================
// css::frame::XTitleChangeBroadcaster
void SAL_CALL SfxBaseController::removeTitleChangeListener(const css::uno::Reference< css::frame::XTitleChangeListener >& xListener)
    throw (css::uno::RuntimeException)
{
    css::uno::Reference< css::frame::XTitleChangeBroadcaster > xBroadcaster(impl_getTitleHelper(), css::uno::UNO_QUERY);
    if (xBroadcaster.is ())
        xBroadcaster->removeTitleChangeListener (xListener);
}
