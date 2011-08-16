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
#include "precompiled_fpicker.hxx"

#include "commonpicker.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <vcl/svapp.hxx>
#include <vos/mutex.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <comphelper/weakeventlistener.hxx>
#include <comphelper/types.hxx>
#include <vcl/msgbox.hxx>
#include "iodlg.hxx"

//.........................................................................
namespace svt
{
//.........................................................................

#define PROPERTY_ID_HELPURL		1
#define PROPERTY_ID_WINDOW		2

	// using --------------------------------------------------------------

	using namespace     ::com::sun::star::lang;
	using namespace     ::com::sun::star::ui::dialogs;
	using namespace     ::com::sun::star::uno;
	using namespace     ::com::sun::star::beans;
	using namespace     ::comphelper;

	//---------------------------------------------------------------------
	OCommonPicker::OCommonPicker( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory )
		:OCommonPicker_Base( m_aMutex )
		,OPropertyContainer( GetBroadcastHelper() )
		,m_xORB( _rxFactory )
		,m_pDlg( NULL )
		,m_nCancelEvent( 0 )
		,m_bExecuting( sal_False )
	{
		// the two properties we have
		registerProperty(
			::rtl::OUString::createFromAscii( "HelpURL" ), PROPERTY_ID_HELPURL,
			PropertyAttribute::TRANSIENT,
			&m_sHelpURL, ::getCppuType( &m_sHelpURL )
		);

		registerProperty(
			::rtl::OUString::createFromAscii( "Window" ), PROPERTY_ID_WINDOW,
			PropertyAttribute::TRANSIENT | PropertyAttribute::READONLY,
			&m_xWindow, ::getCppuType( &m_xWindow )
		);
	}

	//---------------------------------------------------------------------
	OCommonPicker::~OCommonPicker()
	{
		if ( !GetBroadcastHelper().bDisposed )
		{
			acquire();
			dispose();
		}
	}

	//---------------------------------------------------------------------
	// disambiguate XInterface
	//---------------------------------------------------------------------
	IMPLEMENT_FORWARD_XINTERFACE2( OCommonPicker, OCommonPicker_Base, OPropertyContainer )

	//---------------------------------------------------------------------
	// disambiguate XTypeProvider
	//---------------------------------------------------------------------
	IMPLEMENT_FORWARD_XTYPEPROVIDER2( OCommonPicker, OCommonPicker_Base, OPropertyContainer )

	//---------------------------------------------------------------------
	// XComponent related methods
	//---------------------------------------------------------------------
	void OCommonPicker::checkAlive() const SAL_THROW( (DisposedException) )
	{
		if ( GetBroadcastHelper().bInDispose || GetBroadcastHelper().bDisposed )
			throw DisposedException();
	}

    void OCommonPicker::prepareDialog()
    {
        if ( !getDialog() )
            createPicker();

        // set the title
        if ( m_aTitle.getLength() > 0 )
            getDialog()->SetText( m_aTitle );
    }

	//---------------------------------------------------------------------
	void SAL_CALL OCommonPicker::disposing()
	{
		::vos::OGuard aGuard( Application::GetSolarMutex() );

		stopWindowListening();

		if ( m_nCancelEvent )
			Application::RemoveUserEvent( m_nCancelEvent );

		{
			::osl::MutexGuard aOwnGuard( m_aMutex );
			if ( m_bExecuting && m_pDlg )
				m_pDlg->EndDialog( RET_CANCEL );
		}

		delete m_pDlg;
		m_pDlg = NULL;
		m_xWindow = NULL;
		m_xDialogParent = NULL;
	}

	//---------------------------------------------------------------------
	void OCommonPicker::stopWindowListening()
	{
		disposeComponent( m_xWindowListenerAdapter );
		disposeComponent( m_xParentListenerAdapter );
	}

	//---------------------------------------------------------------------
	// XEventListener
	//---------------------------------------------------------------------
	void SAL_CALL OCommonPicker::disposing( const EventObject& _rSource ) throw (RuntimeException)
	{
		::vos::OGuard aGuard( Application::GetSolarMutex() );
		sal_Bool bDialogDying = _rSource.Source == m_xWindow;
		sal_Bool bParentDying = _rSource.Source == m_xDialogParent;

		if ( bDialogDying || bParentDying )
		{
			stopWindowListening();

			if ( !bDialogDying )	// it's the parent which is dying -> delete the dialog
				delete m_pDlg;

			m_pDlg = NULL;
			m_xWindow = NULL;
			m_xDialogParent = NULL;
		}
		else
		{
			DBG_ERROR( "OCommonPicker::disposing: where did this come from?" );
        }
	}

	//---------------------------------------------------------------------
	// property set related methods
	//---------------------------------------------------------------------
	::cppu::IPropertyArrayHelper* OCommonPicker::createArrayHelper( ) const
	{
		Sequence< Property > aProps;
		describeProperties( aProps );
		return new cppu::OPropertyArrayHelper( aProps );
	}

	//---------------------------------------------------------------------
	::cppu::IPropertyArrayHelper& SAL_CALL OCommonPicker::getInfoHelper()
	{
		return *const_cast< OCommonPicker* >( this )->getArrayHelper();
	}

	//---------------------------------------------------------------------
	Reference< XPropertySetInfo > SAL_CALL OCommonPicker::getPropertySetInfo(  ) throw(RuntimeException)
	{
		return ::cppu::OPropertySetHelper::createPropertySetInfo( getInfoHelper() );
	}

	//---------------------------------------------------------------------
	void SAL_CALL OCommonPicker::setFastPropertyValue_NoBroadcast( sal_Int32 _nHandle, const Any& _rValue ) throw (Exception)
	{
		OPropertyContainer::setFastPropertyValue_NoBroadcast( _nHandle, _rValue );

		// if the HelpURL changed, forward this to the dialog
		if ( PROPERTY_ID_HELPURL == _nHandle )
			if ( m_pDlg )
				OControlAccess::setHelpURL( m_pDlg, m_sHelpURL, sal_False );
	}


	//---------------------------------------------------------------------
	sal_Bool OCommonPicker::createPicker()
	{
		::vos::OGuard aGuard( Application::GetSolarMutex() );

		if ( !m_pDlg )
		{
			m_pDlg = implCreateDialog( VCLUnoHelper::GetWindow( m_xDialogParent ) );
			DBG_ASSERT( m_pDlg, "OCommonPicker::createPicker: invalid dialog returned!" );

			if ( m_pDlg )
			{
				// synchronize the help id of the dialog with out help URL property
				if ( m_sHelpURL.getLength() )
				{	// somebody already set the help URL while we had no dialog yet
					OControlAccess::setHelpURL( m_pDlg, m_sHelpURL, sal_False );
				}
				else
				{
					m_sHelpURL = OControlAccess::getHelpURL( m_pDlg, sal_False );
				}

				m_xWindow = VCLUnoHelper::GetInterface( m_pDlg );

				// add as event listener to the window
				Reference< XComponent > xWindowComp( m_xWindow, UNO_QUERY );
				OSL_ENSURE( xWindowComp.is(), "OCommonPicker::createFileDialog: invalid window component!" );
				if ( xWindowComp.is() )
				{
					m_xWindowListenerAdapter = new OWeakEventListenerAdapter( this, xWindowComp );
						// the adapter will add itself as listener, and forward notifications
				}

				// _and_ add as event listener to the parent - in case the parent is destroyed
				// before we are disposed, our disposal would access dead VCL windows then ....
				m_xDialogParent = VCLUnoHelper::GetInterface( m_pDlg->GetParent() );
				xWindowComp = xWindowComp.query( m_xDialogParent );
				OSL_ENSURE( xWindowComp.is() || !m_pDlg->GetParent(), "OCommonPicker::createFileDialog: invalid window component (the parent this time)!" );
				if ( xWindowComp.is() )
				{
					m_xParentListenerAdapter = new OWeakEventListenerAdapter( this, xWindowComp );
						// the adapter will add itself as listener, and forward notifications
				}
			}
		}

		return NULL != m_pDlg;
	}

	//---------------------------------------------------------------------
	// XControlAccess functions
	//---------------------------------------------------------------------
	void SAL_CALL OCommonPicker::setControlProperty( const ::rtl::OUString& aControlName, const ::rtl::OUString& aControlProperty, const Any& aValue ) throw (IllegalArgumentException, RuntimeException)
	{
		checkAlive();

		::vos::OGuard aGuard( Application::GetSolarMutex() );
		if ( createPicker() )
		{
			::svt::OControlAccess aAccess( m_pDlg, m_pDlg->GetView() );
			aAccess.setControlProperty( aControlName, aControlProperty, aValue );
		}
	}

	//---------------------------------------------------------------------
	Any SAL_CALL OCommonPicker::getControlProperty( const ::rtl::OUString& aControlName, const ::rtl::OUString& aControlProperty ) throw (IllegalArgumentException, RuntimeException)
	{
		checkAlive();

		::vos::OGuard aGuard( Application::GetSolarMutex() );
		if ( createPicker() )
		{
			::svt::OControlAccess aAccess( m_pDlg, m_pDlg->GetView() );
			return aAccess.getControlProperty( aControlName, aControlProperty );
		}

		return Any();
	}

	//---------------------------------------------------------------------
	// XControlInformation functions
	//---------------------------------------------------------------------
	Sequence< ::rtl::OUString > SAL_CALL OCommonPicker::getSupportedControls(  ) throw (RuntimeException)
	{
		checkAlive();

		::vos::OGuard aGuard( Application::GetSolarMutex() );
		if ( createPicker() )
		{
			::svt::OControlAccess aAccess( m_pDlg, m_pDlg->GetView() );
			return aAccess.getSupportedControls( );
		}

		return Sequence< ::rtl::OUString >();
	}

	//---------------------------------------------------------------------
	sal_Bool SAL_CALL OCommonPicker::isControlSupported( const ::rtl::OUString& aControlName ) throw (RuntimeException)
	{
		checkAlive();

		::vos::OGuard aGuard( Application::GetSolarMutex() );
		if ( createPicker() )
		{
			::svt::OControlAccess aAccess( m_pDlg, m_pDlg->GetView() );
			return aAccess.isControlSupported( aControlName );
		}

		return sal_False;
	}

	//---------------------------------------------------------------------
	Sequence< ::rtl::OUString > SAL_CALL OCommonPicker::getSupportedControlProperties( const ::rtl::OUString& aControlName ) throw (IllegalArgumentException, RuntimeException)
	{
		checkAlive();

		::vos::OGuard aGuard( Application::GetSolarMutex() );
		if ( createPicker() )
		{
			::svt::OControlAccess aAccess( m_pDlg, m_pDlg->GetView() );
			return aAccess.getSupportedControlProperties( aControlName );
		}

		return Sequence< ::rtl::OUString >();
	}

	//---------------------------------------------------------------------
	sal_Bool SAL_CALL OCommonPicker::isControlPropertySupported( const ::rtl::OUString& aControlName, const ::rtl::OUString& aControlProperty ) throw (IllegalArgumentException, RuntimeException)
	{
		checkAlive();

		::vos::OGuard aGuard( Application::GetSolarMutex() );
		if ( createPicker() )
		{
			::svt::OControlAccess aAccess( m_pDlg, m_pDlg->GetView() );
			return aAccess.isControlPropertySupported( aControlName, aControlProperty );
		}

		return sal_False;
	}

	//---------------------------------------------------------------------
	// XExecutableDialog functions
	//---------------------------------------------------------------------
	void SAL_CALL OCommonPicker::setTitle( const rtl::OUString& _rTitle ) throw( RuntimeException )
	{
		::vos::OGuard aGuard( Application::GetSolarMutex() );
		m_aTitle = _rTitle;
	}

	//---------------------------------------------------------------------
	sal_Int16 OCommonPicker::execute() throw (RuntimeException)
	{
		::vos::OGuard aGuard( Application::GetSolarMutex() );

        prepareDialog();

		{
			::osl::MutexGuard aOwnGuard( m_aMutex );
			m_bExecuting = sal_True;
		}
		sal_Int16 nResult = implExecutePicker();
		{
			::osl::MutexGuard aOwnGuard( m_aMutex );
			m_bExecuting = sal_False;
		}

		return nResult;
	}

	//---------------------------------------------------------------------
	// XCancellable functions
	//---------------------------------------------------------------------
	void SAL_CALL OCommonPicker::cancel(  ) throw (RuntimeException)
	{
		{
			::osl::MutexGuard aGuard( m_aMutex );
			if ( m_nCancelEvent )
				// nothing to do - the event for cancelling the dialog is already on the way
				return;
		}

		// The thread which executes our dialog has locked the solar mutex for
		// sure. Cancelling the dialog should be done with a locked solar mutex, too.
		// Thus we post ourself a message for cancelling the dialog. This way, the message
		// is either handled in the thread which opened the dialog (which may even be
		// this thread here), or, if no dialog is open, in the thread doing scheduling
		// currently. Both is okay for us ....
		//
		// Note that we could do check if we are really executing the dialog currently.
		// but the information would be potentially obsolete at the moment our event
		// arrives, so we need to check it there, anyway ...
		m_nCancelEvent = Application::PostUserEvent( LINK( this, OCommonPicker, OnCancelPicker ) );
	}

	//---------------------------------------------------------------------
	IMPL_LINK( OCommonPicker, OnCancelPicker, void*, EMPTYARG )
	{
		// By definition, the solar mutex is locked when we arrive here. Note that this
		// is important, as for instance the consistency of m_pDlg depends on this mutex.
		::osl::MutexGuard aGuard( m_aMutex );
		m_nCancelEvent = 0;

		if ( !m_bExecuting )
			// nothing to do. This may be because the dialog was cancelled after our cancel method
			// posted this async event, or because somebody called cancel without the dialog
			// being executed at this time.
			return 0;

		OSL_ENSURE( getDialog(), "OCommonPicker::OnCancelPicker: executing, but no dialog!" );
		if ( getDialog() )
			getDialog()->EndDialog( RET_CANCEL );

		return 0L;
	}

	//------------------------------------------------------------------------------------
	// XInitialization functions
	//------------------------------------------------------------------------------------
	void SAL_CALL OCommonPicker::initialize( const Sequence< Any >& _rArguments )
		throw ( Exception, RuntimeException )
	{
		checkAlive();

		::rtl::OUString sSettingName;
		Any				aSettingValue;

		PropertyValue	aPropArg;
		NamedValue		aPairArg;


		const Any* pArguments		= _rArguments.getConstArray();
		const Any* pArgumentsEnd	= _rArguments.getConstArray() + _rArguments.getLength();
		for	(	const Any* pArgument = pArguments;
				pArgument != pArgumentsEnd;
				++pArgument
			)
		{
			if ( *pArgument >>= aPropArg )
			{
				if ( aPropArg.Name.getLength() <= 0)
					continue;

				sSettingName = aPropArg.Name;
				aSettingValue = aPropArg.Value;
			}
			else if ( *pArgument >>= aPairArg )
			{
				if ( aPairArg.Name.getLength() <= 0)
					continue;

				sSettingName = aPairArg.Name;
				aSettingValue = aPairArg.Value;

				
			}
			else
			{
				DBG_ERROR(
					(	::rtl::OString( "OCommonPicker::initialize: unknown argument type at position " )
					+=	::rtl::OString::valueOf( (sal_Int32)( pArguments - _rArguments.getConstArray() ) )
					).getStr()
				);
				continue;
			}

#ifdef DBG_UTIL
			sal_Bool bKnownSetting =
#endif
			implHandleInitializationArgument( sSettingName, aSettingValue );
			DBG_ASSERT( bKnownSetting,
				(	::rtl::OString( "OCommonPicker::initialize: unknown argument \"" )
				+=	::rtl::OString( sSettingName.getStr(), sSettingName.getLength(), osl_getThreadTextEncoding() )
				+=	::rtl::OString( "\"!" )
				).getStr()
			);
		}
	}

	//---------------------------------------------------------------------
	sal_Bool OCommonPicker::implHandleInitializationArgument( const ::rtl::OUString& _rName, const Any& _rValue ) SAL_THROW( ( Exception, RuntimeException ) )
	{
		sal_Bool bKnown = sal_True;
		if ( _rName.equalsAscii( "ParentWindow" ) )
		{
			m_xDialogParent.clear();
			OSL_VERIFY( _rValue >>= m_xDialogParent );
			OSL_ENSURE( VCLUnoHelper::GetWindow( m_xDialogParent ), "OCommonPicker::implHandleInitializationArgument: invalid parent window given!" );
		}
		else
			bKnown = sal_False;
		return bKnown;
	}

//.........................................................................
}	// namespace svt
//.........................................................................

