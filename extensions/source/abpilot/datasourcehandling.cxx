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
#include "precompiled_extensions.hxx"

#include "abpresid.hrc"
#include "abptypes.hxx"
#include "componentmodule.hxx"
#include "datasourcehandling.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#include <com/sun/star/sdb/XDatabaseRegistrations.hpp>
#include <com/sun/star/sdb/XDocumentDataSource.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/uno/XNamingService.hpp>

#include <comphelper/interaction.hxx>
#include <comphelper/componentcontext.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <unotools/confignode.hxx>
#include <unotools/sharedunocomponent.hxx>
#include <vcl/stdtext.hxx>

//.........................................................................
namespace abp
{
//.........................................................................

	using namespace ::utl;
	using namespace ::comphelper;
	using namespace ::com::sun::star::uno;
	using namespace ::com::sun::star::lang;
	using namespace ::com::sun::star::sdb;
	using namespace ::com::sun::star::sdbc;
	using namespace ::com::sun::star::task;
	using namespace ::com::sun::star::beans;
	using namespace ::com::sun::star::sdbcx;
	using namespace ::com::sun::star::container;
	using namespace ::com::sun::star::frame;

	//=====================================================================
	struct PackageAccessControl { };

	//=====================================================================
    //---------------------------------------------------------------------
	static Reference< XNameAccess > lcl_getDataSourceContext( const Reference< XMultiServiceFactory >& _rxORB ) SAL_THROW (( Exception ))
	{
		Reference< XNameAccess > xContext( _rxORB->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.sdb.DatabaseContext" ) ), UNO_QUERY );
		DBG_ASSERT(xContext.is(), "lcl_getDataSourceContext: could not access the data source context!");
		return xContext;
	}

	//---------------------------------------------------------------------
	/// creates a new data source and inserts it into the context
	static void lcl_implCreateAndInsert(
		const Reference< XMultiServiceFactory >& _rxORB, const ::rtl::OUString& _rName,
		Reference< XPropertySet >& /* [out] */ _rxNewDataSource ) SAL_THROW (( ::com::sun::star::uno::Exception ))
	{
		//.............................................................
		// get the data source context
		Reference< XNameAccess > xContext = lcl_getDataSourceContext( _rxORB );

		DBG_ASSERT( !xContext->hasByName( _rName ), "lcl_implCreateAndInsert: name already used!" );
        (void)_rName;

		//.............................................................
		// create a new data source
		Reference< XSingleServiceFactory > xFactory( xContext, UNO_QUERY );
		Reference< XPropertySet > xNewDataSource;
		if (xFactory.is())
			xNewDataSource = Reference< XPropertySet >( xFactory->createInstance(), UNO_QUERY );
		DBG_ASSERT( xNewDataSource.is(), "lcl_implCreateAndInsert: could not create a new data source!" );

		//.............................................................
		// insert the data source into the context
		Reference< XNamingService > xDynamicContext( xContext, UNO_QUERY );
		DBG_ASSERT( xDynamicContext.is(), "lcl_implCreateAndInsert: missing an interface on the context (XNamingService)!" );
		if (xDynamicContext.is())
		{
			//	xDynamicContext->registerObject( _rName, xNewDataSource );
			_rxNewDataSource = xNewDataSource;
		}
	}

	//---------------------------------------------------------------------
	/// creates and inserts a data source, and sets it's URL property to the string given
	static ODataSource lcl_implCreateAndSetURL(
		const Reference< XMultiServiceFactory >& _rxORB, const ::rtl::OUString& _rName,
		const sal_Char* _pInitialAsciiURL ) SAL_THROW (( ))
	{
		ODataSource aReturn( _rxORB );
		try
		{
			// create the new data source
			Reference< XPropertySet > xNewDataSource;
			lcl_implCreateAndInsert( _rxORB, _rName, xNewDataSource );

			//.............................................................
			// set the URL property
			if (xNewDataSource.is())
			{
				xNewDataSource->setPropertyValue(
					::rtl::OUString::createFromAscii( "URL" ),
					makeAny( ::rtl::OUString::createFromAscii( _pInitialAsciiURL ) )
				);
			}

			aReturn.setDataSource( xNewDataSource, _rName,PackageAccessControl() );
		}
		catch(const Exception&)
		{
			DBG_ERROR( "lcl_implCreateAndSetURL: caught an exception while creating the data source!" );
		}

		return aReturn;
	}
	//---------------------------------------------------------------------
	void lcl_registerDataSource(
		const Reference< XMultiServiceFactory >& _rxORB, const ::rtl::OUString& _sName,
		const ::rtl::OUString& _sURL ) SAL_THROW (( ::com::sun::star::uno::Exception ))
	{
        OSL_ENSURE( _sName.getLength(), "lcl_registerDataSource: invalid name!" );
        OSL_ENSURE( _sURL.getLength(), "lcl_registerDataSource: invalid URL!" );
        try
        {

            ::comphelper::ComponentContext aContext( _rxORB );
            Reference< XDatabaseRegistrations > xRegistrations(
                aContext.createComponent( "com.sun.star.sdb.DatabaseContext" ), UNO_QUERY_THROW );

		    if ( xRegistrations->hasRegisteredDatabase( _sName ) )
                xRegistrations->changeDatabaseLocation( _sName, _sURL );
            else
                xRegistrations->registerDatabaseLocation( _sName, _sURL );
        }
        catch( const Exception& )
        {
        	DBG_UNHANDLED_EXCEPTION();
        }
	}

	//=====================================================================
	//= ODataSourceContextImpl
	//=====================================================================
	struct ODataSourceContextImpl
	{
		Reference< XMultiServiceFactory >	xORB;
		Reference< XNameAccess >			xContext;			/// the UNO data source context
		StringBag							aDataSourceNames;	/// for quicker name checks (without the UNO overhead)

		ODataSourceContextImpl( const Reference< XMultiServiceFactory >& _rxORB ) : xORB( _rxORB ) { }
		ODataSourceContextImpl( const ODataSourceContextImpl& _rSource )
			:xORB		( _rSource.xORB )
			,xContext	( _rSource.xContext )
		{
		}
	};

	//=====================================================================
	//= ODataSourceContext
	//=====================================================================
	//---------------------------------------------------------------------
	ODataSourceContext::ODataSourceContext(const Reference< XMultiServiceFactory >& _rxORB)
		:m_pImpl( new ODataSourceContextImpl( _rxORB ) )
	{
		try
		{
			// create the UNO context
			m_pImpl->xContext = lcl_getDataSourceContext( _rxORB );

			if (m_pImpl->xContext.is())
			{
				// collect the data source names
				Sequence< ::rtl::OUString > aDSNames = m_pImpl->xContext->getElementNames();
				const ::rtl::OUString* pDSNames = aDSNames.getConstArray();
				const ::rtl::OUString* pDSNamesEnd = pDSNames + aDSNames.getLength();

				for ( ;pDSNames != pDSNamesEnd; ++pDSNames )
					m_pImpl->aDataSourceNames.insert( *pDSNames );
			}
		}
		catch( const Exception& )
		{
			DBG_ERROR( "ODataSourceContext::ODataSourceContext: caught an exception!" );
		}
	}

	//---------------------------------------------------------------------
	::rtl::OUString& ODataSourceContext::disambiguate(::rtl::OUString& _rDataSourceName)
	{
		::rtl::OUString sCheck( _rDataSourceName );
		ConstStringBagIterator aPos = m_pImpl->aDataSourceNames.find( sCheck );

		sal_Int32 nPostFix = 1;
		while ( ( m_pImpl->aDataSourceNames.end() != aPos ) && ( nPostFix < 65535 ) )
		{	// there already is a data source with this name
			sCheck = _rDataSourceName;
			sCheck += ::rtl::OUString::valueOf( nPostFix++ );

			aPos = m_pImpl->aDataSourceNames.find( sCheck );
		}

		_rDataSourceName = sCheck;
		return _rDataSourceName;
	}

	//---------------------------------------------------------------------
	void ODataSourceContext::getDataSourceNames( StringBag& _rNames ) const SAL_THROW (( ))
	{
		_rNames = m_pImpl->aDataSourceNames;
	}

	//---------------------------------------------------------------------
	ODataSource	ODataSourceContext::createNewLDAP( const ::rtl::OUString& _rName) SAL_THROW (( ))
	{
		return lcl_implCreateAndSetURL( m_pImpl->xORB, _rName, "sdbc:address:ldap:" );
	}

	//---------------------------------------------------------------------
	ODataSource	ODataSourceContext::createNewMORK( const ::rtl::OUString& _rName) SAL_THROW (( ))
	{
		return lcl_implCreateAndSetURL( m_pImpl->xORB, _rName, "sdbc:address:mozilla" );
	}

	//---------------------------------------------------------------------
	ODataSource	ODataSourceContext::createNewThunderbird( const ::rtl::OUString& _rName ) SAL_THROW (( ))
	{
		return lcl_implCreateAndSetURL( m_pImpl->xORB, _rName, "sdbc:address:thunderbird" );
	}

	//---------------------------------------------------------------------
	ODataSource	ODataSourceContext::createNewEvolutionLdap( const ::rtl::OUString& _rName) SAL_THROW (( ))
	{
		return lcl_implCreateAndSetURL( m_pImpl->xORB, _rName, "sdbc:address:evolution:ldap" );
	}
	//---------------------------------------------------------------------
	ODataSource	ODataSourceContext::createNewEvolutionGroupwise( const ::rtl::OUString& _rName) SAL_THROW (( ))
	{
		return lcl_implCreateAndSetURL( m_pImpl->xORB, _rName, "sdbc:address:evolution:groupwise" );
	}
	//---------------------------------------------------------------------
	ODataSource	ODataSourceContext::createNewEvolution( const ::rtl::OUString& _rName) SAL_THROW (( ))
	{
		return lcl_implCreateAndSetURL( m_pImpl->xORB, _rName, "sdbc:address:evolution:local" );
	}

	//---------------------------------------------------------------------
	ODataSource	ODataSourceContext::createNewKab( const ::rtl::OUString& _rName) SAL_THROW (( ))
	{
		return lcl_implCreateAndSetURL( m_pImpl->xORB, _rName, "sdbc:address:kab" );
	}

	//---------------------------------------------------------------------
	ODataSource	ODataSourceContext::createNewMacab( const ::rtl::OUString& _rName) SAL_THROW (( ))
	{
		return lcl_implCreateAndSetURL( m_pImpl->xORB, _rName, "sdbc:address:macab" );
	}

	//---------------------------------------------------------------------
	ODataSource	ODataSourceContext::createNewOutlook( const ::rtl::OUString& _rName) SAL_THROW (( ))
	{
		return lcl_implCreateAndSetURL( m_pImpl->xORB, _rName, "sdbc:address:outlook" );
	}

	//---------------------------------------------------------------------
	ODataSource	ODataSourceContext::createNewOE( const ::rtl::OUString& _rName) SAL_THROW (( ))
	{
		return lcl_implCreateAndSetURL( m_pImpl->xORB, _rName, "sdbc:address:outlookexp" );
	}

	//---------------------------------------------------------------------
	ODataSource	ODataSourceContext::createNewDBase( const ::rtl::OUString& _rName) SAL_THROW (( ))
	{
		return lcl_implCreateAndSetURL( m_pImpl->xORB, _rName, "sdbc:dbase:" );
	}

	//=====================================================================
	//= ODataSourceImpl
	//=====================================================================
	struct ODataSourceImpl
	{
	public:
		Reference< XMultiServiceFactory >		xORB;				/// the service factory
		Reference< XPropertySet >				xDataSource;		/// the UNO data source
        ::utl::SharedUNOComponent< XConnection >
                                                xConnection;
		StringBag								aTables;			// the cached table names
		::rtl::OUString							sName;
		sal_Bool								bTablesUpToDate;	// table name cache up-to-date?

		ODataSourceImpl( const Reference< XMultiServiceFactory >& _rxORB )
			:xORB( _rxORB )
			,bTablesUpToDate( sal_False )
		{
		}

		ODataSourceImpl( const ODataSourceImpl& _rSource );
	};

	//---------------------------------------------------------------------
	ODataSourceImpl::ODataSourceImpl( const ODataSourceImpl& _rSource )
		:xORB( _rSource.xORB )
		,xDataSource( _rSource.xDataSource )
		,xConnection( _rSource.xConnection )
		,aTables( _rSource.aTables )
		,sName( _rSource.sName )
		,bTablesUpToDate( _rSource.bTablesUpToDate )
	{
	}

	//=====================================================================
	//= ODataSource
	//=====================================================================
	//---------------------------------------------------------------------
	ODataSource::ODataSource( const ODataSource& _rSource )
		:m_pImpl( NULL )
	{
		*this = _rSource;
	}

	//---------------------------------------------------------------------
	ODataSource& ODataSource::operator=( const ODataSource& _rSource )
	{
		delete m_pImpl;
		m_pImpl = new ODataSourceImpl( *_rSource.m_pImpl );

		return *this;
	}

	//---------------------------------------------------------------------
	ODataSource::ODataSource( const Reference< XMultiServiceFactory >& _rxORB )
		:m_pImpl(new ODataSourceImpl(_rxORB))
	{
	}

	//---------------------------------------------------------------------
	ODataSource::~ODataSource( )
	{
		delete m_pImpl;
	}

	//---------------------------------------------------------------------
	void ODataSource::store() SAL_THROW (( ))
	{
		if (!isValid())
			// nothing to do
			return;
		try
		{
			Reference< XDocumentDataSource > xDocAccess( m_pImpl->xDataSource, UNO_QUERY );
            Reference< XStorable > xStorable;
            if ( xDocAccess.is() )
                xStorable = xStorable.query( xDocAccess->getDatabaseDocument() );
			OSL_ENSURE( xStorable.is(),"DataSource is no XStorable!" );
			if ( xStorable.is() )
				xStorable->storeAsURL(m_pImpl->sName,Sequence<PropertyValue>());
		}
		catch(const Exception&)
		{
			DBG_ERROR( "ODataSource::registerDataSource: caught an exception while creating the data source!" );
		}
	}
	//---------------------------------------------------------------------
	void ODataSource::registerDataSource( const ::rtl::OUString& _sRegisteredDataSourceName) SAL_THROW (( ))
	{
		if (!isValid())
			// nothing to do
			return;

		try
		{
			// invalidate ourself
			lcl_registerDataSource(m_pImpl->xORB,_sRegisteredDataSourceName,m_pImpl->sName);
		}
		catch(const Exception&)
		{
			DBG_ERROR( "ODataSource::registerDataSource: caught an exception while creating the data source!" );
		}		
	}

	//---------------------------------------------------------------------
	void ODataSource::setDataSource( const Reference< XPropertySet >& _rxDS,const ::rtl::OUString& _sName, PackageAccessControl )
	{
		if (m_pImpl->xDataSource.get() == _rxDS.get())
			// nothing to do
			return;

		if ( isConnected() )
			disconnect();

		m_pImpl->sName = _sName;
		m_pImpl->xDataSource = _rxDS;
	}

	//---------------------------------------------------------------------
	void ODataSource::remove() SAL_THROW (( ))
	{
		if (!isValid())
			// nothing to do
			return;

		try
		{
			// invalidate ourself
			m_pImpl->xDataSource.clear();
		}
		catch(const Exception&)
		{
			DBG_ERROR( "ODataSource::remove: caught an exception while creating the data source!" );
		}
	}

	//---------------------------------------------------------------------
	sal_Bool ODataSource::rename( const ::rtl::OUString& _rName ) SAL_THROW (( ))
	{
		if (!isValid())
			// nothing to do
			return sal_False;

		m_pImpl->sName = _rName;
		return sal_True;
	}

	//---------------------------------------------------------------------
	::rtl::OUString ODataSource::getName() const SAL_THROW (( ))
	{
		if ( !isValid() )
			return ::rtl::OUString();
		return m_pImpl->sName;
	}

	//---------------------------------------------------------------------
    bool ODataSource::hasTable( const ::rtl::OUString& _rTableName ) const
    {
        if ( !isConnected() )
            return false;

        const StringBag& aTables( getTableNames() );
        return aTables.find( _rTableName ) != aTables.end();
    }

	//---------------------------------------------------------------------
	const StringBag& ODataSource::getTableNames() const SAL_THROW (( ))
	{
		m_pImpl->aTables.clear();
		if ( !isConnected() )
		{
			DBG_ERROR( "ODataSource::getTableNames: not connected!" );
		}
		else
		{
			try
			{
				// get the tables container from the connection
				Reference< XTablesSupplier > xSuppTables( m_pImpl->xConnection.getTyped(), UNO_QUERY );
				Reference< XNameAccess > xTables;
				if ( xSuppTables.is( ) )
					xTables = xSuppTables->getTables();
				DBG_ASSERT( xTables.is(), "ODataSource::getTableNames: could not retrieve the tables container!" );

				// get the names
				Sequence< ::rtl::OUString > aTableNames;
				if ( xTables.is( ) )
					aTableNames = xTables->getElementNames( );

				// copy the names
				const ::rtl::OUString* pTableNames = aTableNames.getConstArray();
				const ::rtl::OUString* pTableNamesEnd = pTableNames + aTableNames.getLength();
				for (;pTableNames < pTableNamesEnd; ++pTableNames)
					m_pImpl->aTables.insert( *pTableNames );
			}
			catch(const Exception&)
			{
			}
		}

		// now the table cache is up-to-date
		m_pImpl->bTablesUpToDate = sal_True;
		return m_pImpl->aTables;
	}

	//---------------------------------------------------------------------
	sal_Bool ODataSource::connect( Window* _pMessageParent ) SAL_THROW (( ))
	{
		if ( isConnected( ) )
			// nothing to do
			return sal_True;

		// ................................................................
		// create the interaction handler (needed for authentication and error handling)
		static ::rtl::OUString s_sInteractionHandlerServiceName = ::rtl::OUString::createFromAscii("com.sun.star.task.InteractionHandler");
		Reference< XInteractionHandler > xInteractions;
		try
		{
			xInteractions = Reference< XInteractionHandler >(
				m_pImpl->xORB->createInstance( s_sInteractionHandlerServiceName ),
				UNO_QUERY
			);
		}
		catch(const Exception&)
		{
		}

		// ................................................................
		// failure to create the interaction handler is a serious issue ...
		if (!xInteractions.is())
		{
			if ( _pMessageParent )
				ShowServiceNotAvailableError( _pMessageParent, s_sInteractionHandlerServiceName, sal_True );
			return sal_False;
		}

		// ................................................................
		// open the connection
		Any aError;
		Reference< XConnection > xConnection;
		try
		{
			Reference< XCompletedConnection > xComplConn( m_pImpl->xDataSource, UNO_QUERY );
			DBG_ASSERT( xComplConn.is(), "ODataSource::connect: missing the XCompletedConnection interface on the data source!" );
			if ( xComplConn.is() )
				xConnection = xComplConn->connectWithCompletion( xInteractions );
		}
		catch( const SQLContext& e ) { aError <<= e; }
		catch( const SQLWarning& e ) { aError <<= e; }
		catch( const SQLException& e ) { aError <<= e; }
		catch( const Exception& )
		{
			DBG_ERROR( "ODataSource::connect: caught a generic exception!" );
		}

		// ................................................................
		// handle errors
		if ( aError.hasValue() && _pMessageParent )
		{
			try
			{
				SQLException aException;
  				aError >>= aException;
  				if ( !aException.Message.getLength() )
  				{
	    			// prepend some context info
					SQLContext aDetailedError;
					aDetailedError.Message = String( ModuleRes( RID_STR_NOCONNECTION ) );
					aDetailedError.Details = String( ModuleRes( RID_STR_PLEASECHECKSETTINGS ) );
					aDetailedError.NextException = aError;
					// handle (aka display) the new context info
					xInteractions->handle( new OInteractionRequest( makeAny( aDetailedError ) ) );
  				}
  				else
  				{
  					// handle (aka display) the original error
					xInteractions->handle( new OInteractionRequest( makeAny( aException ) ) );
				}
			}
			catch( const Exception& )
			{
				DBG_ERROR( "ODataSource::connect: caught an exception while trying to display the error!" );
			}
		}

		if ( !xConnection.is() )
			return sal_False;

		// ................................................................
		// success
		m_pImpl->xConnection.reset( xConnection );
		m_pImpl->aTables.clear();
		m_pImpl->bTablesUpToDate = sal_False;

		return sal_True;
	}

	//---------------------------------------------------------------------
	void ODataSource::disconnect( ) SAL_THROW (( ))
	{
		m_pImpl->xConnection.clear();
		m_pImpl->aTables.clear();
		m_pImpl->bTablesUpToDate = sal_False;
	}

	//---------------------------------------------------------------------
	sal_Bool ODataSource::isConnected( ) const SAL_THROW (( ))
	{
		return m_pImpl->xConnection.is();
	}

	//---------------------------------------------------------------------
	sal_Bool ODataSource::isValid() const SAL_THROW (( ))
	{
		return m_pImpl && m_pImpl->xDataSource.is();
	}
	//---------------------------------------------------------------------
	Reference< XPropertySet > ODataSource::getDataSource() const SAL_THROW (( ))
	{
		return m_pImpl ? m_pImpl->xDataSource : Reference< XPropertySet >();
	}

//.........................................................................
}	// namespace abp
//.........................................................................

