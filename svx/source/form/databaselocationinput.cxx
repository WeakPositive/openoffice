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
#include "precompiled_svx.hxx"

#include "svx/databaselocationinput.hxx"
#include "svx/dialmgr.hxx"

#include "svx/fmresids.hrc"

/** === begin UNO includes === **/
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
/** === end UNO includes === **/

#include <comphelper/componentcontext.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <rtl/ustrbuf.hxx>
#include <sfx2/filedlghelper.hxx>
#include <svtools/urlcontrol.hxx>
#include <svl/filenotation.hxx>
#include <tools/diagnose_ex.h>
#include <unotools/confignode.hxx>
#include <unotools/ucbhelper.hxx>
#include <vcl/button.hxx>
#include <vcl/msgbox.hxx>

//........................................................................
namespace svx
{
//........................................................................

	/** === begin UNO using === **/
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::container::XNameAccess;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::Exception;
	/** === end UNO using === **/
    namespace TemplateDescription = ::com::sun::star::ui::dialogs::TemplateDescription;

	//====================================================================
	//= DatabaseLocationInputController_Impl
	//====================================================================
    class DatabaseLocationInputController_Impl
    {
    public:
        DatabaseLocationInputController_Impl(
            const ::comphelper::ComponentContext&   _rContext,
            ::svt::OFileURLControl&                 _rLocationInput,
            PushButton&                             _rBrowseButton
        );
        ~DatabaseLocationInputController_Impl();

        bool    prepareCommit();
        void    setURL( const String& _rURL );
        String  getURL() const;

    private:
        void    impl_initFilterProperties_nothrow();
        void    impl_onBrowseButtonClicked();
        void    impl_onLocationModified();
        String  impl_getCurrentURL() const;

        DECL_LINK( OnControlAction, VclWindowEvent* );

    private:
        const ::comphelper::ComponentContext    m_aContext;
        ::svt::OFileURLControl&                 m_rLocationInput;
        PushButton&                             m_rBrowseButton;
        Sequence< ::rtl::OUString >             m_aFilterExtensions;
        ::rtl::OUString                         m_sFilterUIName;
        bool                                    m_bNeedExistenceCheck;
    };

	//--------------------------------------------------------------------
    DatabaseLocationInputController_Impl::DatabaseLocationInputController_Impl( const ::comphelper::ComponentContext& _rContext,
            ::svt::OFileURLControl& _rLocationInput, PushButton& _rBrowseButton )
        :m_aContext( _rContext )
        ,m_rLocationInput( _rLocationInput )
        ,m_rBrowseButton( _rBrowseButton )
        ,m_aFilterExtensions()
        ,m_sFilterUIName()
        ,m_bNeedExistenceCheck( true )
    {
        impl_initFilterProperties_nothrow();

        // forward the allowed extensions to the input control
        ::rtl::OUStringBuffer aExtensionList;
        for (   const ::rtl::OUString* pExtension = m_aFilterExtensions.getConstArray();
                pExtension != m_aFilterExtensions.getConstArray() + m_aFilterExtensions.getLength();
                ++pExtension
            )
        {
            aExtensionList.append( *pExtension );
            aExtensionList.append( (sal_Unicode)';' );
        }
        m_rLocationInput.SetFilter( aExtensionList.makeStringAndClear() );

        m_rBrowseButton.AddEventListener( LINK( this, DatabaseLocationInputController_Impl, OnControlAction ) );
        m_rLocationInput.AddEventListener( LINK( this, DatabaseLocationInputController_Impl, OnControlAction ) );
    }

    //--------------------------------------------------------------------
    DatabaseLocationInputController_Impl::~DatabaseLocationInputController_Impl()
    {
        m_rBrowseButton.RemoveEventListener( LINK( this, DatabaseLocationInputController_Impl, OnControlAction ) );
        m_rLocationInput.RemoveEventListener( LINK( this, DatabaseLocationInputController_Impl, OnControlAction ) );
    }

	//--------------------------------------------------------------------
    bool DatabaseLocationInputController_Impl::prepareCommit()
    {
		::rtl::OUString sURL( impl_getCurrentURL() );
        if ( !sURL.getLength() )
            return false;

        // check if the name exists
		if ( m_bNeedExistenceCheck )
		{
			if ( ::utl::UCBContentHelper::Exists( sURL ) )
			{
				QueryBox aBox( m_rLocationInput.GetSystemWindow(), WB_YES_NO, SVX_RES( RID_STR_ALREADYEXISTOVERWRITE ) );
				if ( aBox.Execute() != RET_YES )
					return false;
			}
		}

        return true;
    }

	//--------------------------------------------------------------------
    void DatabaseLocationInputController_Impl::setURL( const String& _rURL )
    {
        ::svt::OFileNotation aTransformer( _rURL );
        m_rLocationInput.SetText( aTransformer.get( ::svt::OFileNotation::N_SYSTEM ) );
    }

	//--------------------------------------------------------------------
    String DatabaseLocationInputController_Impl::getURL() const
    {
        return impl_getCurrentURL();
    }

	//--------------------------------------------------------------------
    void DatabaseLocationInputController_Impl::impl_initFilterProperties_nothrow()
    {
        try
        {
            // get the name of the default filter for database documents
            ::utl::OConfigurationTreeRoot aConfig(
                ::utl::OConfigurationTreeRoot::createWithServiceFactory(
                    m_aContext.getLegacyServiceFactory(),
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/org.openoffice.Setup/Office/Factories/com.sun.star.sdb.OfficeDatabaseDocument" ) )
            ) );
            ::rtl::OUString sDatabaseFilter;
            OSL_VERIFY( aConfig.getNodeValue( "ooSetupFactoryActualFilter" ) >>= sDatabaseFilter );

            // get the type this filter is responsible for
            Reference< XNameAccess > xFilterFactory(
                m_aContext.createComponent( "com.sun.star.document.FilterFactory" ),
                UNO_QUERY_THROW );
            ::comphelper::NamedValueCollection aFilterProperties( xFilterFactory->getByName( sDatabaseFilter ) );
            ::rtl::OUString sDocumentType = aFilterProperties.getOrDefault( "Type", ::rtl::OUString() );

            // get the extension(s) for this type
            Reference< XNameAccess > xTypeDetection(
                m_aContext.createComponent( "com.sun.star.document.TypeDetection" ),
                UNO_QUERY_THROW );

            ::comphelper::NamedValueCollection aTypeProperties( xTypeDetection->getByName( sDocumentType ) );
            m_aFilterExtensions = aTypeProperties.getOrDefault( "Extensions", m_aFilterExtensions );
            m_sFilterUIName = aTypeProperties.getOrDefault( "UIName", m_sFilterUIName );
        }
        catch( const Exception& )
        {
        	DBG_UNHANDLED_EXCEPTION();
        }

        // ensure we have at least one extension
        OSL_ENSURE( m_aFilterExtensions.getLength(),
            "DatabaseLocationInputController_Impl::impl_initFilterProperties_nothrow: unable to determine the file extension(s)!" );
        if ( m_aFilterExtensions.getLength() == 0 )
        {
            m_aFilterExtensions.realloc(1);
            m_aFilterExtensions[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "*.odb" ) );
        }
    }

    // -----------------------------------------------------------------------------
    IMPL_LINK( DatabaseLocationInputController_Impl, OnControlAction, VclWindowEvent*, _pEvent )
    {
        if  (   ( _pEvent->GetWindow() == &m_rBrowseButton )
            &&  ( _pEvent->GetId() == VCLEVENT_BUTTON_CLICK )
            )
        {
            impl_onBrowseButtonClicked();
        }

        if  (   ( _pEvent->GetWindow() == &m_rLocationInput )
            &&  ( _pEvent->GetId() == VCLEVENT_EDIT_MODIFY )
            )
        {
            impl_onLocationModified();
        }

        return 0L;
    }

    // -----------------------------------------------------------------------------
    String DatabaseLocationInputController_Impl::impl_getCurrentURL() const
    {
        String sCurrentFile( m_rLocationInput.GetText() );
        if ( sCurrentFile.Len() )
        {
            ::svt::OFileNotation aCurrentFile( sCurrentFile );
            sCurrentFile = aCurrentFile.get( ::svt::OFileNotation::N_URL );
        }
        return sCurrentFile;
    }

    // -----------------------------------------------------------------------------
    void DatabaseLocationInputController_Impl::impl_onBrowseButtonClicked()
    {
        ::sfx2::FileDialogHelper aFileDlg(
            TemplateDescription::FILESAVE_AUTOEXTENSION,
            WB_STDMODAL | WB_SAVEAS,
            m_rLocationInput.GetSystemWindow()
        );
        aFileDlg.SetDisplayDirectory( impl_getCurrentURL() );

        aFileDlg.AddFilter( m_sFilterUIName, ::rtl::OUStringBuffer().appendAscii( "*." ).append( m_aFilterExtensions[0] ).makeStringAndClear() );
        aFileDlg.SetCurrentFilter( m_sFilterUIName );

        if ( aFileDlg.Execute() == ERRCODE_NONE )
        {
            INetURLObject aURL( aFileDlg.GetPath() );
            if( aURL.GetProtocol() != INET_PROT_NOT_VALID )
            {
                ::svt::OFileNotation aFileNotation( aURL.GetMainURL( INetURLObject::NO_DECODE ) );
                m_rLocationInput.SetText( aFileNotation.get( ::svt::OFileNotation::N_SYSTEM ) );
                m_rLocationInput.GetModifyHdl().Call( &m_rLocationInput );
                // the dialog already checked for the file's existence, so we don't need to, again
                m_bNeedExistenceCheck = false;
            }
        }
    }

    // -----------------------------------------------------------------------------
    void DatabaseLocationInputController_Impl::impl_onLocationModified()
    {
        m_bNeedExistenceCheck = true;
    }

    //====================================================================
	//= DatabaseLocationInputController
	//====================================================================
	//--------------------------------------------------------------------
    DatabaseLocationInputController::DatabaseLocationInputController( const ::comphelper::ComponentContext& _rContext,
            ::svt::OFileURLControl& _rLocationInput, PushButton& _rBrowseButton )
        :m_pImpl( new DatabaseLocationInputController_Impl( _rContext, _rLocationInput, _rBrowseButton ) )
    {
    }

	//--------------------------------------------------------------------
    DatabaseLocationInputController::~DatabaseLocationInputController()
    {
    }

	//--------------------------------------------------------------------
    bool DatabaseLocationInputController::prepareCommit()
    {
        return m_pImpl->prepareCommit();
    }

	//--------------------------------------------------------------------
    void DatabaseLocationInputController::setURL( const String& _rURL )
    {
        m_pImpl->setURL( _rURL );
    }

	//--------------------------------------------------------------------
    String DatabaseLocationInputController::getURL() const
    {
        return m_pImpl->getURL();
    }

//........................................................................
} // namespace svx
//........................................................................
