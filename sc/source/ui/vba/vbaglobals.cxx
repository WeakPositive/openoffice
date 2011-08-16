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
#include <vbahelper/helperdecl.hxx>
#include "vbaglobals.hxx"

#include <comphelper/unwrapargs.hxx>

#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <cppuhelper/component_context.hxx>

#include "vbaapplication.hxx"
#include "vbaworksheet.hxx"
#include "vbarange.hxx"
#include <cppuhelper/bootstrap.hxx>
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::ooo::vba;



// =============================================================================
// ScVbaGlobals
// =============================================================================

//ScVbaGlobals::ScVbaGlobals(  css::uno::Reference< css::uno::XComponentContext >const& rxContext,  ) : ScVbaGlobals_BASE( uno::Reference< XHelperInterface >(), rxContext )
rtl::OUString sDocCtxName( RTL_CONSTASCII_USTRINGPARAM("ExcelDocumentContext") );

ScVbaGlobals::ScVbaGlobals( uno::Sequence< uno::Any > const& aArgs, uno::Reference< uno::XComponentContext >const& rxContext ) : ScVbaGlobals_BASE( uno::Reference< XHelperInterface >(), rxContext, sDocCtxName )
{
	OSL_TRACE("ScVbaGlobals::ScVbaGlobals()");

        uno::Sequence< beans::PropertyValue > aInitArgs( 2 );
        aInitArgs[ 0 ].Name = rtl::OUString::createFromAscii("Application");
        aInitArgs[ 0 ].Value = uno::makeAny( getApplication() );
        aInitArgs[ 1 ].Name = sDocCtxName;
        aInitArgs[ 1 ].Value = uno::makeAny( getXSomethingFromArgs< frame::XModel >( aArgs, 0 ) );
        
        init( aInitArgs );
}

ScVbaGlobals::~ScVbaGlobals()
{
	OSL_TRACE("ScVbaGlobals::~ScVbaGlobals");
}

// =============================================================================
// XGlobals
// =============================================================================
uno::Reference<excel::XApplication >
ScVbaGlobals::getApplication() throw (uno::RuntimeException)
{
//	OSL_TRACE("In ScVbaGlobals::getApplication");	
        if ( !mxApplication.is() )
	    mxApplication.set( new ScVbaApplication( mxContext) );
   	return mxApplication; 
}


uno::Reference<excel::XApplication > SAL_CALL
ScVbaGlobals::getExcel() throw (uno::RuntimeException)
{
   	return getApplication(); 
}



uno::Reference< excel::XWorkbook > SAL_CALL 
ScVbaGlobals::getActiveWorkbook() throw (uno::RuntimeException)
{
//	OSL_TRACE("In ScVbaGlobals::getActiveWorkbook");	
    uno::Reference< excel::XWorkbook > xWorkbook( getApplication()->getActiveWorkbook(), uno::UNO_QUERY);
    if ( xWorkbook.is() )
    {
        return xWorkbook;    
    }
// FIXME check if this is correct/desired behavior
    throw uno::RuntimeException( rtl::OUString::createFromAscii(
        "No activeWorkbook available" ), Reference< uno::XInterface >() );
}


uno::Reference< excel::XWindow > SAL_CALL 
ScVbaGlobals::getActiveWindow() throw (uno::RuntimeException)
{
    return getApplication()->getActiveWindow();
}

uno::Reference< excel::XWorksheet > SAL_CALL 
ScVbaGlobals::getActiveSheet() throw (uno::RuntimeException)
{
    return getApplication()->getActiveSheet();
}

uno::Any SAL_CALL 
ScVbaGlobals::WorkBooks( const uno::Any& aIndex ) throw (uno::RuntimeException)
{
	return uno::Any( getApplication()->Workbooks(aIndex) );
}

uno::Any SAL_CALL
ScVbaGlobals::WorkSheets(const uno::Any& aIndex) throw (uno::RuntimeException)
{
	return getApplication()->Worksheets( aIndex );
}
uno::Any SAL_CALL
ScVbaGlobals::Sheets( const uno::Any& aIndex ) throw (uno::RuntimeException)
{
	return WorkSheets( aIndex );
}

uno::Any SAL_CALL 
ScVbaGlobals::Range( const uno::Any& Cell1, const uno::Any& Cell2 ) throw (uno::RuntimeException)
{
	return getApplication()->Range( Cell1, Cell2 );
}

uno::Any SAL_CALL
ScVbaGlobals::Names( const css::uno::Any& aIndex ) throw ( uno::RuntimeException )
{
	return getApplication()->Names( aIndex );
}

uno::Reference< excel::XRange > SAL_CALL 
ScVbaGlobals::getActiveCell() throw (uno::RuntimeException)
{
	return getApplication()->getActiveCell();
}

uno::Reference< XAssistant > SAL_CALL 
ScVbaGlobals::getAssistant() throw (uno::RuntimeException)
{
	return getApplication()->getAssistant();
}

uno::Any SAL_CALL 
ScVbaGlobals::getSelection() throw (uno::RuntimeException)
{
	return getApplication()->getSelection();
}

uno::Reference< excel::XWorkbook > SAL_CALL 
ScVbaGlobals::getThisWorkbook() throw (uno::RuntimeException)
{
	return getApplication()->getThisWorkbook();
}
void SAL_CALL 
ScVbaGlobals::Calculate()  throw (::com::sun::star::script::BasicErrorException, ::com::sun::star::uno::RuntimeException)
{
	return getApplication()->Calculate();
}

uno::Reference< excel::XRange > SAL_CALL 
ScVbaGlobals::Cells( const uno::Any& RowIndex, const uno::Any& ColumnIndex ) throw (uno::RuntimeException)
{
	return getApplication()->getActiveSheet()->Cells( RowIndex, ColumnIndex );
}
uno::Reference< excel::XRange > SAL_CALL 
ScVbaGlobals::Columns( const uno::Any& aIndex ) throw (uno::RuntimeException)
{
	return getApplication()->getActiveSheet()->Columns( aIndex );
}

uno::Any SAL_CALL 
ScVbaGlobals::CommandBars( const uno::Any& aIndex ) throw (uno::RuntimeException)
{
	uno::Reference< XApplicationBase > xBase( getApplication(), uno::UNO_QUERY_THROW );
	return xBase->CommandBars( aIndex );
}

css::uno::Reference< ov::excel::XRange > SAL_CALL 
ScVbaGlobals::Union( const css::uno::Reference< ov::excel::XRange >& Arg1, const css::uno::Reference< ov::excel::XRange >& Arg2, const css::uno::Any& Arg3, const css::uno::Any& Arg4, const css::uno::Any& Arg5, const css::uno::Any& Arg6, const css::uno::Any& Arg7, const css::uno::Any& Arg8, const css::uno::Any& Arg9, const css::uno::Any& Arg10, const css::uno::Any& Arg11, const css::uno::Any& Arg12, const css::uno::Any& Arg13, const css::uno::Any& Arg14, const css::uno::Any& Arg15, const css::uno::Any& Arg16, const css::uno::Any& Arg17, const css::uno::Any& Arg18, const css::uno::Any& Arg19, const css::uno::Any& Arg20, const css::uno::Any& Arg21, const css::uno::Any& Arg22, const css::uno::Any& Arg23, const css::uno::Any& Arg24, const css::uno::Any& Arg25, const css::uno::Any& Arg26, const css::uno::Any& Arg27, const css::uno::Any& Arg28, const css::uno::Any& Arg29, const css::uno::Any& Arg30 ) throw (css::script::BasicErrorException, css::uno::RuntimeException)
{
	return getApplication()->Union(  Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9, Arg10, Arg11, Arg12, Arg13, Arg14, Arg15, Arg16, Arg17, Arg18, Arg19, Arg20, Arg21, Arg22, Arg23, Arg24, Arg25, Arg26, Arg27, Arg28, Arg29, Arg30 );
}
css::uno::Reference< ov::excel::XRange > SAL_CALL 
ScVbaGlobals::Intersect( const css::uno::Reference< ov::excel::XRange >& Arg1, const css::uno::Reference< ov::excel::XRange >& Arg2, const css::uno::Any& Arg3, const css::uno::Any& Arg4, const css::uno::Any& Arg5, const css::uno::Any& Arg6, const css::uno::Any& Arg7, const css::uno::Any& Arg8, const css::uno::Any& Arg9, const css::uno::Any& Arg10, const css::uno::Any& Arg11, const css::uno::Any& Arg12, const css::uno::Any& Arg13, const css::uno::Any& Arg14, const css::uno::Any& Arg15, const css::uno::Any& Arg16, const css::uno::Any& Arg17, const css::uno::Any& Arg18, const css::uno::Any& Arg19, const css::uno::Any& Arg20, const css::uno::Any& Arg21, const css::uno::Any& Arg22, const css::uno::Any& Arg23, const css::uno::Any& Arg24, const css::uno::Any& Arg25, const css::uno::Any& Arg26, const css::uno::Any& Arg27, const css::uno::Any& Arg28, const css::uno::Any& Arg29, const css::uno::Any& Arg30 ) throw (css::script::BasicErrorException, css::uno::RuntimeException)
{
	return getApplication()->Intersect(  Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9, Arg10, Arg11, Arg12, Arg13, Arg14, Arg15, Arg16, Arg17, Arg18, Arg19, Arg20, Arg21, Arg22, Arg23, Arg24, Arg25, Arg26, Arg27, Arg28, Arg29, Arg30 );
}

uno::Any SAL_CALL 
ScVbaGlobals::Evaluate( const ::rtl::OUString& Name ) throw (uno::RuntimeException)
{
	return getApplication()->Evaluate( Name );
}

css::uno::Any SAL_CALL 
ScVbaGlobals::WorksheetFunction(  ) throw (css::uno::RuntimeException)
{
	return getApplication()->WorksheetFunction();
}

uno::Any SAL_CALL 
ScVbaGlobals::Windows( const uno::Any& aIndex ) throw (uno::RuntimeException)
{
	return getApplication()->Windows( aIndex );
}

uno::Reference< excel::XRange > SAL_CALL 
ScVbaGlobals::Rows( const uno::Any& aIndex ) throw (uno::RuntimeException)
{
	return getApplication()->getActiveSheet()->Rows( aIndex );
	
}


uno::Any SAL_CALL
ScVbaGlobals::getDebug() throw (uno::RuntimeException)
{
	try // return empty object on error
	{
		uno::Reference< lang::XMultiComponentFactory > xServiceManager( mxContext->getServiceManager(), uno::UNO_SET_THROW );
		uno::Reference< uno::XInterface > xVBADebug = xServiceManager->createInstanceWithContext(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ooo.vba.Debug" ) ), mxContext );
        return uno::Any( xVBADebug );
	}
    catch( uno::Exception& )
	{
	}
	return uno::Any();
}

uno::Sequence< ::rtl::OUString > SAL_CALL 
ScVbaGlobals::getAvailableServiceNames(  ) throw (uno::RuntimeException)
{
    static bool bInit = false;
    static uno::Sequence< rtl::OUString > serviceNames( ScVbaGlobals_BASE::getAvailableServiceNames() );
    if ( !bInit )
    {
         rtl::OUString names[] = { 
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "ooo.vba.excel.Range" ) ),
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "ooo.vba.excel.Workbook" ) ),
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "ooo.vba.excel.Window" ) ),
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "ooo.vba.excel.Worksheet" ) ),
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "ooo.vba.excel.Application" ) ),
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "ooo.vba.excel.Hyperlink" ) ),
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.script.vba.VBASpreadsheetEventProcessor" ) )
          };
        sal_Int32 nExcelServices = ( sizeof( names )/ sizeof( names[0] ) );
        sal_Int32 startIndex = serviceNames.getLength();
        serviceNames.realloc( serviceNames.getLength() + nExcelServices );
        for ( sal_Int32 index = 0; index < nExcelServices; ++index )
             serviceNames[ startIndex + index ] = names[ index ];
        bInit = true;
    }
    return serviceNames;
}

rtl::OUString&
ScVbaGlobals::getServiceImplName()
{
        static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaGlobals") );
        return sImplName;
}

uno::Sequence< rtl::OUString >
ScVbaGlobals::getServiceNames()
{
        static uno::Sequence< rtl::OUString > aServiceNames;
        if ( aServiceNames.getLength() == 0 )
        {
                aServiceNames.realloc( 1 );
                aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.excel.Globals" ) );
        }
        return aServiceNames;
}

namespace globals
{
namespace sdecl = comphelper::service_decl;
sdecl::vba_service_class_<ScVbaGlobals, sdecl::with_args<true> > serviceImpl;
extern sdecl::ServiceDecl const serviceDecl(
    serviceImpl,
    "ScVbaGlobals",
    "ooo.vba.excel.Globals" );
}

