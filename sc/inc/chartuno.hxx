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

#ifndef SC_CHARTUNO_HXX
#define SC_CHARTUNO_HXX

#include "address.hxx"
#include <svl/lstner.hxx>
#include <tools/string.hxx>
#include <comphelper/proparrhlp.hxx>
#include <comphelper/propertycontainer.hxx>

#include <com/sun/star/table/XTableChart.hpp>
#include <com/sun/star/table/XTableCharts.hpp>
#include <com/sun/star/document/XEmbeddedObjectSupplier.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <cppuhelper/compbase4.hxx>
#include <cppuhelper/implbase4.hxx>


class ScDocShell;
class ScRangeListRef;
class ScChartObj;


class ScChartsObj : public cppu::WeakImplHelper4<
							com::sun::star::table::XTableCharts,
							com::sun::star::container::XEnumerationAccess,
							com::sun::star::container::XIndexAccess,
							com::sun::star::lang::XServiceInfo >,
						public SfxListener
{
private:
	ScDocShell*				pDocShell;
	SCTAB					nTab;			// Charts sind pro Sheet

	ScChartObj*				GetObjectByIndex_Impl(long nIndex) const;
	ScChartObj*				GetObjectByName_Impl(const ::rtl::OUString& aName) const;

public:
							ScChartsObj(ScDocShell* pDocSh, SCTAB nT);
	virtual					~ScChartsObj();

	virtual void			Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

							// XTableCharts
	virtual void SAL_CALL	addNewByName( const ::rtl::OUString& aName,
									const ::com::sun::star::awt::Rectangle& aRect,
									const ::com::sun::star::uno::Sequence<
										::com::sun::star::table::CellRangeAddress >& aRanges,
									sal_Bool bColumnHeaders, sal_Bool bRowHeaders )
										throw(::com::sun::star::uno::RuntimeException);
	virtual void SAL_CALL	removeByName( const ::rtl::OUString& aName )
										throw(::com::sun::star::uno::RuntimeException);

							// XNameAccess
	virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName )
								throw(::com::sun::star::container::NoSuchElementException,
									::com::sun::star::lang::WrappedTargetException,
									::com::sun::star::uno::RuntimeException);
	virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames()
								throw(::com::sun::star::uno::RuntimeException);
	virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
								throw(::com::sun::star::uno::RuntimeException);

							// XIndexAccess
	virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException);
	virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
								throw(::com::sun::star::lang::IndexOutOfBoundsException,
									::com::sun::star::lang::WrappedTargetException,
									::com::sun::star::uno::RuntimeException);

							// XEnumerationAccess
	virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
							createEnumeration() throw(::com::sun::star::uno::RuntimeException);

							// XElementAccess
	virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
								throw(::com::sun::star::uno::RuntimeException);
	virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

							// XServiceInfo
	virtual ::rtl::OUString SAL_CALL getImplementationName()
								throw(::com::sun::star::uno::RuntimeException);
	virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
								throw(::com::sun::star::uno::RuntimeException);
	virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
								throw(::com::sun::star::uno::RuntimeException);
};


typedef ::cppu::WeakComponentImplHelper4<
    ::com::sun::star::table::XTableChart,
    ::com::sun::star::document::XEmbeddedObjectSupplier,
    ::com::sun::star::container::XNamed,
    ::com::sun::star::lang::XServiceInfo > ScChartObj_Base;

typedef ::comphelper::OPropertyContainer ScChartObj_PBase;
typedef ::comphelper::OPropertyArrayUsageHelper< ScChartObj > ScChartObj_PABase;

class ScChartObj : public ::comphelper::OBaseMutex
                  ,public ScChartObj_Base
                  ,public ScChartObj_PBase
                  ,public ScChartObj_PABase
                  ,public SfxListener
{
private:
	ScDocShell*				pDocShell;
	SCTAB					nTab;			// Charts sind pro Sheet
	String					aChartName;

	void	Update_Impl( const ScRangeListRef& rRanges, bool bColHeaders, bool bRowHeaders );
	void	GetData_Impl( ScRangeListRef& rRanges, bool& rColHeaders, bool& rRowHeaders ) const;

protected:
    // ::comphelper::OPropertySetHelper
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
        throw (::com::sun::star::uno::Exception);
    using ::cppu::OPropertySetHelper::getFastPropertyValue;
    virtual void SAL_CALL getFastPropertyValue( ::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const;

    // ::comphelper::OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const;

public:
							ScChartObj(ScDocShell* pDocSh, SCTAB nT, const String& rN);
	virtual					~ScChartObj();

	virtual void			Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    // XInterface
    DECLARE_XINTERFACE()

    // XTypeProvider
    DECLARE_XTYPEPROVIDER()

    // XComponent
    virtual void SAL_CALL disposing();

							// XTableChart
	virtual sal_Bool SAL_CALL getHasColumnHeaders() throw(::com::sun::star::uno::RuntimeException);
	virtual void SAL_CALL	setHasColumnHeaders( sal_Bool bHasColumnHeaders )
								throw(::com::sun::star::uno::RuntimeException);
	virtual sal_Bool SAL_CALL getHasRowHeaders() throw(::com::sun::star::uno::RuntimeException);
	virtual void SAL_CALL	setHasRowHeaders( sal_Bool bHasRowHeaders )
								throw(::com::sun::star::uno::RuntimeException);
	virtual ::com::sun::star::uno::Sequence< ::com::sun::star::table::CellRangeAddress > SAL_CALL
							getRanges(  ) throw(::com::sun::star::uno::RuntimeException);
	virtual void SAL_CALL	setRanges( const ::com::sun::star::uno::Sequence<
									::com::sun::star::table::CellRangeAddress >& aRanges )
								throw(::com::sun::star::uno::RuntimeException);

							// XEmbeddedObjectSupplier
	virtual ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > SAL_CALL
							getEmbeddedObject() throw(::com::sun::star::uno::RuntimeException);

							// XNamed
	virtual ::rtl::OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);
	virtual void SAL_CALL	setName( const ::rtl::OUString& aName )
								throw(::com::sun::star::uno::RuntimeException);

							// XServiceInfo
	virtual ::rtl::OUString SAL_CALL getImplementationName()
								throw(::com::sun::star::uno::RuntimeException);
	virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
								throw(::com::sun::star::uno::RuntimeException);
	virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
								throw(::com::sun::star::uno::RuntimeException);

    // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo()
        throw (::com::sun::star::uno::RuntimeException);
};

#endif

