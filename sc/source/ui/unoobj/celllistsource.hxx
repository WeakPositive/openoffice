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

#ifndef SC_CELLLISTSOURCE_HXX
#define SC_CELLLISTSOURCE_HXX

#include <com/sun/star/form/binding/XListEntrySource.hpp>
#include <cppuhelper/compbase4.hxx>
#include <comphelper/propertycontainer.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <comphelper/proparrhlp.hxx>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/util/XModifyListener.hpp>


//.........................................................................
namespace calc
{
//.........................................................................

    //=====================================================================
    //= OCellListSource
    //=====================================================================
    class OCellListSource;
    // the base for our interfaces
    typedef ::cppu::WeakAggComponentImplHelper4 <   ::com::sun::star::form::binding::XListEntrySource
                                                ,   ::com::sun::star::util::XModifyListener
                                                ,   ::com::sun::star::lang::XServiceInfo
                                                ,   ::com::sun::star::lang::XInitialization
                                                >   OCellListSource_Base;
    // the base for the property handling
    typedef ::comphelper::OPropertyContainer        OCellListSource_PBase;
    // the second base for property handling
    typedef ::comphelper::OPropertyArrayUsageHelper< OCellListSource >
                                                    OCellListSource_PABase;

    class OCellListSource :public ::comphelper::OBaseMutex
                            ,public OCellListSource_Base      // order matters! before OCellListSource_PBase, so rBHelper gets initialized
                            ,public OCellListSource_PBase
					        ,public OCellListSource_PABase
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheetDocument >
                    m_xDocument;            /// the document where our cell lives
        ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange >
                    m_xRange;               /// the range of cells we're bound to
    	::cppu::OInterfaceContainerHelper
                    m_aListEntryListeners;  /// our listeners
        sal_Bool    m_bInitialized;         /// has XInitialization::initialize been called?

    public:
        OCellListSource(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheetDocument >& _rxDocument
        );

        using OCellListSource_PBase::getFastPropertyValue;

    protected:
        ~OCellListSource( );

    protected:
        // XInterface
        DECLARE_XINTERFACE()

        // XTypeProvider
        DECLARE_XTYPEPROVIDER()

        // XListEntrySource
        virtual sal_Int32 SAL_CALL getListEntryCount(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getListEntry( sal_Int32 Position ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getAllListEntries(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addListEntryListener( const ::com::sun::star::uno::Reference< ::com::sun::star::form::binding::XListEntryListener >& Listener ) throw (::com::sun::star::lang::NullPointerException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeListEntryListener( const ::com::sun::star::uno::Reference< ::com::sun::star::form::binding::XListEntryListener >& Listener ) throw (::com::sun::star::lang::NullPointerException, ::com::sun::star::uno::RuntimeException);

        // OComponentHelper/XComponent
        virtual void SAL_CALL disposing();

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

        // XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

        // OPropertySetHelper
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
	    virtual void SAL_CALL getFastPropertyValue( ::com::sun::star::uno::Any& _rValue, sal_Int32 _nHandle ) const;

        // ::comphelper::OPropertyArrayUsageHelper
		virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

        // XModifyListener
        virtual void SAL_CALL modified( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

        // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    private:
        void    checkDisposed( ) const
                    SAL_THROW( ( ::com::sun::star::lang::DisposedException ) );
        void    checkInitialized()
                    SAL_THROW( ( ::com::sun::star::uno::RuntimeException ) );

        /** retrieves the actual address of our cell range
            @precond
                our m_xRange is not <NULL/>
        */
        ::com::sun::star::table::CellRangeAddress
                getRangeAddress( ) const;

        /** retrievs the text of a cell within our range
            @param _nRangeRelativeColumn
                the relative column index of the cell within our range
            @param _nRangeRelativeRow
                the relative row index of the cell within our range
            @precond
                our m_xRange is not <NULL/>
        */
        ::rtl::OUString
                getCellTextContent_noCheck(
                    sal_Int32 _nRangeRelativeColumn,
                    sal_Int32 _nRangeRelativeRow
                );

        void    notifyModified();

    private:
        OCellListSource();                                      // never implemented
        OCellListSource( const OCellListSource& );              // never implemented
        OCellListSource& operator=( const OCellListSource& );   // never implemented

#ifdef DBG_UTIL
    private:
        static  const char* checkConsistency_static( const void* _pThis );
                const char* checkConsistency( ) const;
#endif
    };

//.........................................................................
}   // namespace calc
//.........................................................................

#endif // SC_CELLLISTSOURCE_HXX
