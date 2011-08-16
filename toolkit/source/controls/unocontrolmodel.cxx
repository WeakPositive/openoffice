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
#include "precompiled_toolkit.hxx"
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/FontWidth.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/MouseWheelBehavior.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/io/XMarkableStream.hpp>
#include <toolkit/controls/unocontrolmodel.hxx>
#include <toolkit/helper/macros.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <rtl/memory.h>
#include <rtl/uuid.h>
#include <tools/diagnose_ex.h>
#include <tools/string.hxx>
#include <tools/table.hxx>
#include <tools/date.hxx>
#include <tools/time.hxx>
#include <tools/urlobj.hxx>
#include <tools/debug.hxx>
#include <toolkit/helper/property.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/helper/emptyfontdescriptor.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <unotools/localedatawrapper.hxx>
#include <unotools/configmgr.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/extract.hxx>
#include <vcl/svapp.hxx>
#include <uno/data.h>

#include <memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::i18n;
using ::com::sun::star::awt::FontDescriptor;

struct ImplControlProperty
{
private:
	sal_uInt16					nId;
	::com::sun::star::uno::Any	aValue;

public:
	ImplControlProperty( const ImplControlProperty& rProp ) : aValue( rProp.aValue )
	{
		nId = rProp.nId;
	}

	ImplControlProperty( sal_uInt16 nT )
	{
		nId = nT;
	}

	ImplControlProperty( sal_uInt16 nT, const ::com::sun::star::uno::Any& rValue ) : aValue( rValue )
	{
		nId = nT;
	}

	sal_uInt16							GetId() const 											{ return nId; }
	const ::com::sun::star::uno::Any&	GetValue() const 										{ return aValue; }
	void								SetValue( const ::com::sun::star::uno::Any& rValue )	{ aValue = rValue; }
};

DECLARE_TABLE( ImplPropertyTable, ImplControlProperty* )

#define UNOCONTROL_STREAMVERSION	(short)2

static void lcl_ImplMergeFontProperty( FontDescriptor& rFD, sal_uInt16 nPropId, const Any& rValue )
{
	// some props are defined with other types than the matching FontDescriptor members have
	// (e.g. FontWidth, FontSlant)
	// 78474 - 09/19/2000 - FS
	float		nExtractFloat = 0;
	sal_Int16	nExtractShort = 0;

	switch ( nPropId )
	{
		case BASEPROPERTY_FONTDESCRIPTORPART_NAME: 			rValue >>= rFD.Name;
															break;
		case BASEPROPERTY_FONTDESCRIPTORPART_STYLENAME:		rValue >>= rFD.StyleName;
															break;
		case BASEPROPERTY_FONTDESCRIPTORPART_FAMILY: 		rValue >>= rFD.Family;
															break;
		case BASEPROPERTY_FONTDESCRIPTORPART_CHARSET: 		rValue >>= rFD.CharSet;
															break;
		case BASEPROPERTY_FONTDESCRIPTORPART_HEIGHT: 		rValue >>= nExtractFloat; rFD.Height = (sal_Int16)nExtractFloat;
															break;
		case BASEPROPERTY_FONTDESCRIPTORPART_WEIGHT: 		rValue >>= rFD.Weight;
															break;
		case BASEPROPERTY_FONTDESCRIPTORPART_SLANT: 		if ( rValue >>= nExtractShort )
																rFD.Slant = (::com::sun::star::awt::FontSlant)nExtractShort;
															else
																rValue >>= rFD.Slant;
															break;
		case BASEPROPERTY_FONTDESCRIPTORPART_UNDERLINE:		rValue >>= rFD.Underline;
															break;
		case BASEPROPERTY_FONTDESCRIPTORPART_STRIKEOUT:		rValue >>= rFD.Strikeout;
															break;
		case BASEPROPERTY_FONTDESCRIPTORPART_WIDTH:			rValue >>= rFD.Width;
															break;
		case BASEPROPERTY_FONTDESCRIPTORPART_PITCH:			rValue >>= rFD.Pitch;
															break;
		case BASEPROPERTY_FONTDESCRIPTORPART_CHARWIDTH:		rValue >>= rFD.CharacterWidth;
															break;
		case BASEPROPERTY_FONTDESCRIPTORPART_ORIENTATION: 	rValue >>= rFD.Orientation;
															break;
		case BASEPROPERTY_FONTDESCRIPTORPART_KERNING: 		rValue >>= rFD.Kerning;
															break;
		case BASEPROPERTY_FONTDESCRIPTORPART_WORDLINEMODE: 	rValue >>= rFD.WordLineMode;
															break;
		case BASEPROPERTY_FONTDESCRIPTORPART_TYPE:			rValue >>= rFD.Type;
															break;
		default: 											DBG_ERROR( "FontProperty?!" );
	}
}

//	----------------------------------------------------
//	class UnoControlModel
//	----------------------------------------------------
UnoControlModel::UnoControlModel()
    :UnoControlModel_Base()
    ,MutexAndBroadcastHelper()
    ,OPropertySetHelper( BrdcstHelper )
    ,maDisposeListeners( *this )
    ,maContext( ::comphelper::getProcessServiceFactory() )
{
    OSL_ENSURE( false, "UnoControlModel::UnoControlModel: not implemented. Well, not really." );
    // just implemented to let the various FooImplInheritanceHelper compile, you should use the
    // version taking a service factory
	mpData = new ImplPropertyTable;
}

UnoControlModel::UnoControlModel( const Reference< XMultiServiceFactory >& i_factory )
    :UnoControlModel_Base()
    ,MutexAndBroadcastHelper()
    ,OPropertySetHelper( BrdcstHelper )
    ,maDisposeListeners( *this )
    ,maContext( i_factory )
{
	// Die Properties muessen vom Model in die Tabelle gestopft werden,
	// nur vorhandene Properties sind gueltige Properties, auch wenn VOID.
	mpData = new ImplPropertyTable;
}

UnoControlModel::UnoControlModel( const UnoControlModel& rModel )
	: UnoControlModel_Base()
    , MutexAndBroadcastHelper()
    , OPropertySetHelper( BrdcstHelper )
    , maDisposeListeners( *this )
    , maContext( rModel.maContext )
{
	mpData = new ImplPropertyTable;

	for ( sal_uInt32 n = rModel.mpData->Count(); n; )
	{
		ImplControlProperty* pProp = rModel.mpData->GetObject( --n );
		ImplControlProperty* pNew = new ImplControlProperty( *pProp );
		mpData->Insert( pNew->GetId(), pNew );
	}
}

UnoControlModel::~UnoControlModel()
{
	for ( sal_uInt32 n = mpData->Count(); n; )
		delete mpData->GetObject( --n );
	delete mpData;
}

UnoControlModel* UnoControlModel::Clone() const 
{
	DBG_ERROR( "UnoControlModel::Clone() ?!" );
	return NULL;
}

::com::sun::star::uno::Sequence<sal_Int32> UnoControlModel::ImplGetPropertyIds() const
{
	sal_uInt32 nIDs = mpData->Count();
	::com::sun::star::uno::Sequence<sal_Int32>	aIDs( nIDs );
	sal_Int32* pIDs = aIDs.getArray();
	for ( sal_uInt32 n = 0; n < nIDs; n++ )
		pIDs[n] = mpData->GetObjectKey( n );
	return aIDs;
}

sal_Bool UnoControlModel::ImplHasProperty( sal_uInt16 nPropId ) const
{
	if ( ( nPropId >= BASEPROPERTY_FONTDESCRIPTORPART_START ) && ( nPropId <= BASEPROPERTY_FONTDESCRIPTORPART_END ) )
		nPropId = BASEPROPERTY_FONTDESCRIPTOR;

	return mpData->Get( nPropId ) ? sal_True : sal_False;
}

::com::sun::star::uno::Any UnoControlModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
	::com::sun::star::uno::Any aDefault;

    if (
        (nPropId == BASEPROPERTY_FONTDESCRIPTOR) ||
        (
         (nPropId >= BASEPROPERTY_FONTDESCRIPTORPART_START) &&
         (nPropId <= BASEPROPERTY_FONTDESCRIPTORPART_END)
        )
       )
	{
		EmptyFontDescriptor aFD;
		switch ( nPropId )
		{
			case BASEPROPERTY_FONTDESCRIPTOR: 					aDefault <<= aFD;					break;
			case BASEPROPERTY_FONTDESCRIPTORPART_NAME: 			aDefault <<= aFD.Name;				break;
			case BASEPROPERTY_FONTDESCRIPTORPART_STYLENAME:		aDefault <<= aFD.StyleName;			break;
			case BASEPROPERTY_FONTDESCRIPTORPART_FAMILY: 		aDefault <<= aFD.Family;			break;
			case BASEPROPERTY_FONTDESCRIPTORPART_CHARSET: 		aDefault <<= aFD.CharSet;			break;
			case BASEPROPERTY_FONTDESCRIPTORPART_HEIGHT: 		aDefault <<= (float)aFD.Height;		break;
			case BASEPROPERTY_FONTDESCRIPTORPART_WEIGHT: 		aDefault <<= aFD.Weight;			break;
			case BASEPROPERTY_FONTDESCRIPTORPART_SLANT: 		aDefault <<= (sal_Int16)aFD.Slant;	break;
			case BASEPROPERTY_FONTDESCRIPTORPART_UNDERLINE:		aDefault <<= aFD.Underline;			break;
			case BASEPROPERTY_FONTDESCRIPTORPART_STRIKEOUT:		aDefault <<= aFD.Strikeout;			break;
			case BASEPROPERTY_FONTDESCRIPTORPART_WIDTH:			aDefault <<= aFD.Width;				break;
			case BASEPROPERTY_FONTDESCRIPTORPART_PITCH:			aDefault <<= aFD.Pitch;				break;
			case BASEPROPERTY_FONTDESCRIPTORPART_CHARWIDTH:		aDefault <<= aFD.CharacterWidth;	break;
			case BASEPROPERTY_FONTDESCRIPTORPART_ORIENTATION:	aDefault <<= aFD.Orientation;		break;
			case BASEPROPERTY_FONTDESCRIPTORPART_KERNING:		aDefault <<= aFD.Kerning;			break;
			case BASEPROPERTY_FONTDESCRIPTORPART_WORDLINEMODE:	aDefault <<= aFD.WordLineMode;		break;
			case BASEPROPERTY_FONTDESCRIPTORPART_TYPE:			aDefault <<= aFD.Type;				break;
			default: DBG_ERROR( "FontProperty?!" );
		}
	}
    else
    {
        switch ( nPropId )
        {
			case BASEPROPERTY_GRAPHIC:
                aDefault <<= Reference< graphic::XGraphic >();
                break;

            case BASEPROPERTY_REFERENCE_DEVICE:
                aDefault <<= Reference< awt::XDevice >();
                break;

            case BASEPROPERTY_ITEM_SEPARATOR_POS:
            case BASEPROPERTY_VERTICALALIGN:
            case BASEPROPERTY_BORDERCOLOR:
            case BASEPROPERTY_SYMBOL_COLOR:
            case BASEPROPERTY_TABSTOP:
            case BASEPROPERTY_TEXTCOLOR:
            case BASEPROPERTY_TEXTLINECOLOR:
            case BASEPROPERTY_DATE:
            case BASEPROPERTY_DATESHOWCENTURY:
            case BASEPROPERTY_TIME:
            case BASEPROPERTY_VALUE_DOUBLE:
            case BASEPROPERTY_PROGRESSVALUE: 
            case BASEPROPERTY_SCROLLVALUE: 
            case BASEPROPERTY_VISIBLESIZE: 
            case BASEPROPERTY_BACKGROUNDCOLOR:
            case BASEPROPERTY_FILLCOLOR:            break;  // Void

            case BASEPROPERTY_FONTRELIEF:
            case BASEPROPERTY_FONTEMPHASISMARK:
            case BASEPROPERTY_MAXTEXTLEN:
            case BASEPROPERTY_STATE:
            case BASEPROPERTY_EXTDATEFORMAT:
            case BASEPROPERTY_EXTTIMEFORMAT:
            case BASEPROPERTY_ECHOCHAR:             aDefault <<= (sal_Int16) 0; break;
            case BASEPROPERTY_BORDER:               aDefault <<= (sal_Int16) 1; break;
            case BASEPROPERTY_DECIMALACCURACY:      aDefault <<= (sal_Int16) 2; break;
            case BASEPROPERTY_LINECOUNT:            aDefault <<= (sal_Int16) 5; break;
            case BASEPROPERTY_ALIGN:                aDefault <<= (sal_Int16) PROPERTY_ALIGN_LEFT; break;
            case BASEPROPERTY_IMAGEALIGN:           aDefault <<= (sal_Int16) 1 /*ImageAlign::TOP*/; break;
            case BASEPROPERTY_IMAGEPOSITION:        aDefault <<= (sal_Int16) 12 /*ImagePosition::Centered*/; break;
            case BASEPROPERTY_PUSHBUTTONTYPE:       aDefault <<= (sal_Int16) 0 /*PushButtonType::STANDARD*/; break;
            case BASEPROPERTY_MOUSE_WHEEL_BEHAVIOUR:aDefault <<= (sal_Int16) awt::MouseWheelBehavior::SCROLL_FOCUS_ONLY; break;

            case BASEPROPERTY_DATEMAX:              aDefault <<= (sal_Int32) Date( 31, 12, 2200 ).GetDate();    break;
            case BASEPROPERTY_DATEMIN:              aDefault <<= (sal_Int32) Date( 1, 1, 1900 ).GetDate();  break;
            case BASEPROPERTY_TIMEMAX:              aDefault <<= (sal_Int32) Time( 23, 59 ).GetTime();  break;
            case BASEPROPERTY_TIMEMIN:              aDefault <<= (sal_Int32) 0;     break;
            case BASEPROPERTY_VALUEMAX_DOUBLE:      aDefault <<= (double) 1000000;  break;
            case BASEPROPERTY_VALUEMIN_DOUBLE:      aDefault <<= (double) -1000000; break;
            case BASEPROPERTY_VALUESTEP_DOUBLE:     aDefault <<= (double ) 1;       break; 
            case BASEPROPERTY_PROGRESSVALUE_MAX:    aDefault <<= (sal_Int32) 100;   break;
            case BASEPROPERTY_PROGRESSVALUE_MIN:    aDefault <<= (sal_Int32)   0;   break;
            case BASEPROPERTY_SCROLLVALUE_MAX:		aDefault <<= (sal_Int32) 100;	break;
            case BASEPROPERTY_SCROLLVALUE_MIN:		aDefault <<= (sal_Int32)   0;	break;
            case BASEPROPERTY_LINEINCREMENT:		aDefault <<= (sal_Int32)   1;	break;
            case BASEPROPERTY_BLOCKINCREMENT:		aDefault <<= (sal_Int32)  10;	break;
            case BASEPROPERTY_ORIENTATION:			aDefault <<= (sal_Int32)   0;	break;
            case BASEPROPERTY_SPINVALUE:            aDefault <<= (sal_Int32)   0;   break;
            case BASEPROPERTY_SPININCREMENT:        aDefault <<= (sal_Int32)   1;   break;
            case BASEPROPERTY_SPINVALUE_MIN:        aDefault <<= (sal_Int32)   0;   break;
            case BASEPROPERTY_SPINVALUE_MAX:        aDefault <<= (sal_Int32) 100;   break;
            case BASEPROPERTY_REPEAT_DELAY:         aDefault <<= (sal_Int32)  50;   break;    // 50 milliseconds
            case BASEPROPERTY_DEFAULTCONTROL:       aDefault <<= ((UnoControlModel*)this)->getServiceName();    break;

            case BASEPROPERTY_AUTOHSCROLL:
            case BASEPROPERTY_AUTOVSCROLL:
            case BASEPROPERTY_MOVEABLE:
            case BASEPROPERTY_CLOSEABLE:
            case BASEPROPERTY_SIZEABLE:
            case BASEPROPERTY_HSCROLL:
            case BASEPROPERTY_DEFAULTBUTTON:
            case BASEPROPERTY_MULTILINE:
            case BASEPROPERTY_MULTISELECTION:
            case BASEPROPERTY_TRISTATE:
            case BASEPROPERTY_DROPDOWN:
            case BASEPROPERTY_SPIN:
            case BASEPROPERTY_READONLY:
            case BASEPROPERTY_VSCROLL:
            case BASEPROPERTY_NUMSHOWTHOUSANDSEP:
            case BASEPROPERTY_STRICTFORMAT:
            case BASEPROPERTY_REPEAT:
            case BASEPROPERTY_PAINTTRANSPARENT:
            case BASEPROPERTY_DESKTOP_AS_PARENT:
            case BASEPROPERTY_HARDLINEBREAKS:
            case BASEPROPERTY_NOLABEL:              aDefault <<= (sal_Bool) sal_False; break;

            case BASEPROPERTY_MULTISELECTION_SIMPLEMODE:
            case BASEPROPERTY_HIDEINACTIVESELECTION:
			case BASEPROPERTY_ENFORCE_FORMAT:
            case BASEPROPERTY_AUTOCOMPLETE:
            case BASEPROPERTY_SCALEIMAGE:
            case BASEPROPERTY_ENABLED:
            case BASEPROPERTY_PRINTABLE:
            case BASEPROPERTY_ENABLEVISIBLE:
            case BASEPROPERTY_DECORATION:           aDefault <<= (sal_Bool) sal_True; break;

            case BASEPROPERTY_HELPTEXT:
            case BASEPROPERTY_HELPURL:
	        case BASEPROPERTY_IMAGEURL:
	        case BASEPROPERTY_DIALOGSOURCEURL:				    
            case BASEPROPERTY_EDITMASK:
            case BASEPROPERTY_LITERALMASK:
            case BASEPROPERTY_LABEL:
            case BASEPROPERTY_TITLE:
            case BASEPROPERTY_TEXT:                 aDefault <<= ::rtl::OUString(); break;

            case BASEPROPERTY_WRITING_MODE:
            case BASEPROPERTY_CONTEXT_WRITING_MODE:
                aDefault <<= text::WritingMode2::CONTEXT;
                break;

            case BASEPROPERTY_STRINGITEMLIST:
            {
                ::com::sun::star::uno::Sequence< ::rtl::OUString> aStringSeq;
                aDefault <<= aStringSeq;

            }
            break;
            case BASEPROPERTY_SELECTEDITEMS:
            {
                ::com::sun::star::uno::Sequence<sal_Int16> aINT16Seq;
                aDefault <<= aINT16Seq;
            }
            break;
            case BASEPROPERTY_CURRENCYSYMBOL:
            {
				Any aDefaultCurrency = ::utl::ConfigManager::GetDirectConfigProperty(::utl::ConfigManager::DEFAULTCURRENCY);
				DBG_ASSERT( TypeClass_STRING == aDefaultCurrency.getValueTypeClass(), "UnoControlModel::ImplGetDefaultValue: invalid currency config value!" );

				::rtl::OUString sDefaultCurrency;
				aDefaultCurrency >>= sDefaultCurrency;

				// extract the bank symbol
				sal_Int32 nSepPos = sDefaultCurrency.indexOf( '-' );
				::rtl::OUString sBankSymbol;
				if ( nSepPos >= 0 )
				{
					sBankSymbol = sDefaultCurrency.copy( 0, nSepPos );
					sDefaultCurrency = sDefaultCurrency.copy( nSepPos + 1 );
				}

				// the remaming is the locale
				Locale aLocale;
				nSepPos = sDefaultCurrency.indexOf( '-' );
				if ( nSepPos >= 0 )
				{
					aLocale.Language = sDefaultCurrency.copy( 0, nSepPos );
					aLocale.Country = sDefaultCurrency.copy( nSepPos + 1 );
				}

				LocaleDataWrapper aLocaleInfo( maContext.getLegacyServiceFactory(), aLocale );
				if ( !sBankSymbol.getLength() )
					sBankSymbol = aLocaleInfo.getCurrBankSymbol();

				// look for the currency entry (for this language) which has the given bank symbol
				Sequence< Currency2 > aAllCurrencies = aLocaleInfo.getAllCurrencies();
				const Currency2* pAllCurrencies		=						aAllCurrencies.getConstArray();
				const Currency2* pAllCurrenciesEnd	=	pAllCurrencies	+	aAllCurrencies.getLength();

				::rtl::OUString sCurrencySymbol = aLocaleInfo.getCurrSymbol();
				if ( !sBankSymbol.getLength() )
				{
					DBG_ASSERT( pAllCurrencies != pAllCurrenciesEnd, "UnoControlModel::ImplGetDefaultValue: no currencies at all!" );
					if ( pAllCurrencies != pAllCurrenciesEnd )
					{
						sBankSymbol = pAllCurrencies->BankSymbol;
						sCurrencySymbol = pAllCurrencies->Symbol;
					}
				}

				if ( sBankSymbol.getLength() )
				{
                    bool bLegacy = false;
					for ( ;pAllCurrencies != pAllCurrenciesEnd; ++pAllCurrencies )
						if ( pAllCurrencies->BankSymbol == sBankSymbol )
						{
                            sCurrencySymbol = pAllCurrencies->Symbol;
                            if ( pAllCurrencies->LegacyOnly )
                                bLegacy = true;
                            else
                                break;
						}
					DBG_ASSERT( bLegacy || pAllCurrencies != pAllCurrenciesEnd, "UnoControlModel::ImplGetDefaultValue: did not find the given bank symbol!" );
				}

				aDefault <<= sCurrencySymbol;
            }
            break;

            default:    DBG_ERROR( "ImplGetDefaultValue - unknown Property" );
        }
    }

	return aDefault;
}

void UnoControlModel::ImplRegisterProperty( sal_uInt16 nPropId, const ::com::sun::star::uno::Any& rDefault )
{
	ImplControlProperty* pProp = new ImplControlProperty( nPropId, rDefault );
	mpData->Insert( nPropId, pProp );
}

void UnoControlModel::ImplRegisterProperty( sal_uInt16 nPropId )
{
	ImplRegisterProperty( nPropId, ImplGetDefaultValue( nPropId ) );

    if ( nPropId == BASEPROPERTY_FONTDESCRIPTOR )
    {
        // some properties are not included in the FontDescriptor, but everytime
        // when we have a FontDescriptor we want to have these properties too.
        // => Easier to register the here, istead everywhere where I register the FontDescriptor...

        ImplRegisterProperty( BASEPROPERTY_TEXTCOLOR );
        ImplRegisterProperty( BASEPROPERTY_TEXTLINECOLOR );
        ImplRegisterProperty( BASEPROPERTY_FONTRELIEF );
        ImplRegisterProperty( BASEPROPERTY_FONTEMPHASISMARK );
    }
}

void UnoControlModel::ImplRegisterProperties( const std::list< sal_uInt16 > &rIds )
{
    std::list< sal_uInt16 >::const_iterator iter;
    for( iter = rIds.begin(); iter != rIds.end(); iter++) {
        if( !ImplHasProperty( *iter ) )
            ImplRegisterProperty( *iter, ImplGetDefaultValue( *iter ) );
    }
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any UnoControlModel::queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    Any aRet = UnoControlModel_Base::queryAggregation( rType );
    if ( !aRet.hasValue() )
        aRet = ::cppu::OPropertySetHelper::queryInterface( rType );
    return aRet;
}

// ::com::sun::star::lang::XUnoTunnel
IMPL_XUNOTUNNEL( UnoControlModel )

// XInterface
IMPLEMENT_FORWARD_REFCOUNT( UnoControlModel, UnoControlModel_Base )

// ::com::sun::star::lang::XTypeProvider
IMPLEMENT_FORWARD_XTYPEPROVIDER2( UnoControlModel, UnoControlModel_Base, ::cppu::OPropertySetHelper )


uno::Reference< util::XCloneable > UnoControlModel::createClone() throw(::com::sun::star::uno::RuntimeException)
{
	UnoControlModel* pClone = Clone();
	uno::Reference< util::XCloneable > xClone( (::cppu::OWeakObject*) pClone, uno::UNO_QUERY );
	return xClone;
}

// ::com::sun::star::lang::XComponent
void UnoControlModel::dispose(  ) throw(::com::sun::star::uno::RuntimeException)
{
	::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

	::com::sun::star::lang::EventObject aEvt;
	aEvt.Source = (::com::sun::star::uno::XAggregation*)(::cppu::OWeakAggObject*)this;
	maDisposeListeners.disposeAndClear( aEvt );

	BrdcstHelper.aLC.disposeAndClear( aEvt );

	// let the property set helper notify our property listeners
	OPropertySetHelper::disposing();
}

void UnoControlModel::addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
	::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

	maDisposeListeners.addInterface( rxListener );
}

void UnoControlModel::removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
	::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

	maDisposeListeners.removeInterface( rxListener );
}


// ::com::sun::star::beans::XPropertyState
::com::sun::star::beans::PropertyState UnoControlModel::getPropertyState( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)
{
	::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

	sal_uInt16 nPropId = GetPropertyId( PropertyName );

	::com::sun::star::uno::Any aValue = getPropertyValue( PropertyName );
	::com::sun::star::uno::Any aDefault = ImplGetDefaultValue( nPropId );

	return CompareProperties( aValue, aDefault ) ? ::com::sun::star::beans::PropertyState_DEFAULT_VALUE : ::com::sun::star::beans::PropertyState_DIRECT_VALUE;
}

::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > UnoControlModel::getPropertyStates( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& PropertyNames ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)
{
	::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

	sal_uInt32 nNames = PropertyNames.getLength();
	const ::rtl::OUString* pNames = PropertyNames.getConstArray();

	::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > aStates( nNames );
	::com::sun::star::beans::PropertyState* pStates = aStates.getArray();

	for ( sal_uInt32 n = 0; n < nNames; n++ )
		pStates[n] = getPropertyState( pNames[n] );

	return aStates;
}

void UnoControlModel::setPropertyToDefault( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)
{
    Any aDefaultValue;
    {
	    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
        aDefaultValue = ImplGetDefaultValue( GetPropertyId( PropertyName ) );
    }
	setPropertyValue( PropertyName, aDefaultValue );
}

::com::sun::star::uno::Any UnoControlModel::getPropertyDefault( const ::rtl::OUString& rPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
	::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

	return ImplGetDefaultValue( GetPropertyId( rPropertyName ) );
}


// ::com::sun::star::io::XPersistObjec
::rtl::OUString UnoControlModel::getServiceName(  ) throw(::com::sun::star::uno::RuntimeException)
{
	::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

	DBG_ERROR( "ServiceName von UnoControlModel ?!" );
	return ::rtl::OUString();
}

void UnoControlModel::write( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream >& OutStream ) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
	::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

	::com::sun::star::uno::Reference< ::com::sun::star::io::XMarkableStream > xMark( OutStream, ::com::sun::star::uno::UNO_QUERY );
	DBG_ASSERT( xMark.is(), "write: no ::com::sun::star::io::XMarkableStream!" );

	OutStream->writeShort( UNOCONTROL_STREAMVERSION );

	ImplPropertyTable aProps;
	sal_uInt32 i;
	for ( i = mpData->Count(); i; )
	{
		ImplControlProperty* pProp = mpData->GetObject( --i );
		if ( ( ( GetPropertyAttribs( pProp->GetId() ) & ::com::sun::star::beans::PropertyAttribute::TRANSIENT ) == 0 )
			&& ( getPropertyState( GetPropertyName( pProp->GetId() ) ) != ::com::sun::star::beans::PropertyState_DEFAULT_VALUE ) )
		{
			aProps.Insert( pProp->GetId(), pProp );
		}
	}

	sal_uInt32 nProps = aProps.Count();

	// FontProperty wegen fehlender Unterscheidung zwischen 5.0 / 5.1
	// immer im alten Format mitspeichern.
	OutStream->writeLong( (long) aProps.IsKeyValid( BASEPROPERTY_FONTDESCRIPTOR ) ? ( nProps + 3 ) : nProps );
	for ( i = 0; i < nProps; i++ )
	{
		sal_Int32 nPropDataBeginMark = xMark->createMark();
		OutStream->writeLong( 0L );	// DataLen

		ImplControlProperty* pProp = aProps.GetObject( i );
		OutStream->writeShort( pProp->GetId() );

		sal_Bool bVoid = pProp->GetValue().getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_VOID;

		OutStream->writeBoolean( bVoid );

		if ( !bVoid )
		{
			const ::com::sun::star::uno::Any& rValue = pProp->GetValue();
			const ::com::sun::star::uno::Type& rType = rValue.getValueType();

			if ( rType == ::getBooleanCppuType() )
			{
				sal_Bool b = false;
				rValue >>= b;
				OutStream->writeBoolean( b );
			}
			else if ( rType == ::getCppuType((const ::rtl::OUString*)0) )
			{
				::rtl::OUString aUString;
				rValue >>= aUString;
				OutStream->writeUTF( aUString );
			}
			else if ( rType == ::getCppuType((const sal_uInt16*)0) )
			{
				sal_uInt16 n = 0;
				rValue >>= n;
				OutStream->writeShort( n );
			}
			else if ( rType == ::getCppuType((const sal_Int16*)0) )
			{
				sal_Int16 n = 0;
				rValue >>= n;
				OutStream->writeShort( n );
			}
			else if ( rType == ::getCppuType((const sal_uInt32*)0) )
			{
				sal_uInt32 n = 0;
				rValue >>= n;
				OutStream->writeLong( n );
			}
			else if ( rType == ::getCppuType((const sal_Int32*)0) )
			{
				sal_Int32 n = 0;
				rValue >>= n;
				OutStream->writeLong( n );
			}
			else if ( rType == ::getCppuType((const double*)0) )
			{
				double n = 0;
				rValue >>= n;
				OutStream->writeDouble( n );
			}
			else if ( rType == ::getCppuType((const ::com::sun::star::awt::FontDescriptor*)0) )
			{
				::com::sun::star::awt::FontDescriptor aFD;
				rValue >>= aFD;
				OutStream->writeUTF( aFD.Name );
				OutStream->writeShort( aFD.Height );
				OutStream->writeShort( aFD.Width );
				OutStream->writeUTF( aFD.StyleName );
				OutStream->writeShort( aFD.Family );
				OutStream->writeShort( aFD.CharSet );
				OutStream->writeShort( aFD.Pitch );
				OutStream->writeDouble( aFD.CharacterWidth );
				OutStream->writeDouble( aFD.Weight );
				OutStream->writeShort(
                    sal::static_int_cast< sal_Int16 >(aFD.Slant) );
				OutStream->writeShort( aFD.Underline );
				OutStream->writeShort( aFD.Strikeout );
				OutStream->writeDouble( aFD.Orientation );
				OutStream->writeBoolean( aFD.Kerning );
				OutStream->writeBoolean( aFD.WordLineMode );
				OutStream->writeShort( aFD.Type );
			}
			else if ( rType == ::getCppuType((const ::com::sun::star::uno::Sequence< ::rtl::OUString>*)0 ) )
			{
				::com::sun::star::uno::Sequence< ::rtl::OUString> aSeq;
				rValue >>= aSeq;
				long nEntries = aSeq.getLength();
				OutStream->writeLong( nEntries );
				for ( long n = 0; n < nEntries; n++ )
					OutStream->writeUTF( aSeq.getConstArray()[n] );
			}
			else if ( rType == ::getCppuType((const ::com::sun::star::uno::Sequence<sal_uInt16>*)0 ) )
			{
				::com::sun::star::uno::Sequence<sal_uInt16> aSeq;
				rValue >>= aSeq;
				long nEntries = aSeq.getLength();
				OutStream->writeLong( nEntries );
				for ( long n = 0; n < nEntries; n++ )
					OutStream->writeShort( aSeq.getConstArray()[n] );
			}
			else if ( rType == ::getCppuType((const ::com::sun::star::uno::Sequence<sal_Int16>*)0 ) )
			{
				::com::sun::star::uno::Sequence<sal_Int16> aSeq;
				rValue >>= aSeq;
				long nEntries = aSeq.getLength();
				OutStream->writeLong( nEntries );
				for ( long n = 0; n < nEntries; n++ )
					OutStream->writeShort( aSeq.getConstArray()[n] );
			}
			else if ( rType.getTypeClass() == TypeClass_ENUM )
            {
                sal_Int32 nAsInt = 0;
                ::cppu::enum2int( nAsInt, rValue );
				OutStream->writeLong( nAsInt );
            }
#if OSL_DEBUG_LEVEL > 0
            else
			{
                ::rtl::OString sMessage( "UnoControlModel::write: don't know how to handle a property of type '" );
                ::rtl::OUString sTypeName( rType.getTypeName() );
                sMessage += ::rtl::OString( sTypeName.getStr(), sTypeName.getLength(), RTL_TEXTENCODING_ASCII_US );
                sMessage += "'.\n(Currently handling property '";
                ::rtl::OUString sPropertyName( GetPropertyName( pProp->GetId() ) );
                sMessage += ::rtl::OString( sPropertyName.getStr(), sPropertyName.getLength(), osl_getThreadTextEncoding() );
                sMessage += "'.)";
				DBG_ERROR( sMessage );
			}
#endif
		}

		sal_Int32 nPropDataLen = xMark->offsetToMark( nPropDataBeginMark );
		xMark->jumpToMark( nPropDataBeginMark );
		OutStream->writeLong( nPropDataLen );
		xMark->jumpToFurthest();
		xMark->deleteMark(nPropDataBeginMark);
	}

	ImplControlProperty* pProp = aProps.Get( BASEPROPERTY_FONTDESCRIPTOR );
	if ( pProp )
	{
		// Solange wir keinen 5.0-Export haben, muss das alte
		// Format mit rausgeschrieben werden...
		::com::sun::star::awt::FontDescriptor aFD;
		pProp->GetValue() >>= aFD;

		for ( sal_uInt16 n = BASEPROPERTY_FONT_TYPE; n <= BASEPROPERTY_FONT_ATTRIBS; n++ )
		{
			sal_Int32 nPropDataBeginMark = xMark->createMark();
			OutStream->writeLong( 0L );	// DataLen
			OutStream->writeShort( n );	// PropId
			OutStream->writeBoolean( sal_False );	// Void

			if ( n == BASEPROPERTY_FONT_TYPE )
			{
				OutStream->writeUTF( aFD.Name );
				OutStream->writeUTF( aFD.StyleName );
				OutStream->writeShort( aFD.Family );
				OutStream->writeShort( aFD.CharSet );
				OutStream->writeShort( aFD.Pitch );
			}
			else if ( n == BASEPROPERTY_FONT_SIZE )
			{
				OutStream->writeLong( aFD.Width );
				OutStream->writeLong( aFD.Height );
				OutStream->writeShort(
                    sal::static_int_cast< sal_Int16 >(
                        VCLUnoHelper::ConvertFontWidth( aFD.CharacterWidth )) );
			}
			else if ( n == BASEPROPERTY_FONT_ATTRIBS )
			{
				OutStream->writeShort(
                    sal::static_int_cast< sal_Int16 >(
                        VCLUnoHelper::ConvertFontWeight( aFD.Weight )) );
				OutStream->writeShort(
                    sal::static_int_cast< sal_Int16 >(aFD.Slant) );
				OutStream->writeShort( aFD.Underline );
				OutStream->writeShort( aFD.Strikeout );
				OutStream->writeShort( (short)(aFD.Orientation * 10) );
				OutStream->writeBoolean( aFD.Kerning );
				OutStream->writeBoolean( aFD.WordLineMode );
			}
			else
			{
				DBG_ERROR( "Property?!" );
			}

			sal_Int32 nPropDataLen = xMark->offsetToMark( nPropDataBeginMark );
			xMark->jumpToMark( nPropDataBeginMark );
			OutStream->writeLong( nPropDataLen );
			xMark->jumpToFurthest();
			xMark->deleteMark(nPropDataBeginMark);
		}
	}
}

void UnoControlModel::read( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream >& InStream ) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
	::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

	::com::sun::star::uno::Reference< ::com::sun::star::io::XMarkableStream > xMark( InStream, ::com::sun::star::uno::UNO_QUERY );
	DBG_ASSERT( xMark.is(), "read: no ::com::sun::star::io::XMarkableStream!" );

	short nVersion = InStream->readShort();
	sal_uInt32 nProps = (sal_uInt32)InStream->readLong();
	::com::sun::star::uno::Sequence< ::rtl::OUString> aProps( nProps );
	::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any> aValues( nProps );
	sal_Bool bInvalidEntries = sal_False;

	// Dummerweise kein Mark fuer den gesamten Block, es koennen also
	// nur Properties geaendert werden, es koennen aber nicht spaeter mal Daten
	// fuer das Model hinter den Properties geschrieben werden.

	// Fuer den Import der alten ::com::sun::star::awt::FontDescriptor-Teile
	::com::sun::star::awt::FontDescriptor* pFD = NULL;

	sal_uInt32 i;
	for ( i = 0; i < nProps; i++ )
	{
		sal_Int32 nPropDataBeginMark = xMark->createMark();
		sal_Int32 nPropDataLen = InStream->readLong();

		sal_uInt16 nPropId = (sal_uInt16)InStream->readShort();

		::com::sun::star::uno::Any aValue;
		sal_Bool bIsVoid = InStream->readBoolean();
		if ( !bIsVoid )
		{
			const ::com::sun::star::uno::Type* pType = mpData->Get( nPropId ) ? GetPropertyType( nPropId ) : NULL;
			if ( pType )
			{
				if ( *pType == ::getBooleanCppuType() )
				{
					sal_Bool b = InStream->readBoolean();
					aValue <<= b;
				}
				else if ( *pType == ::getCppuType((const ::rtl::OUString*)0) )
				{
					::rtl::OUString aUTF = InStream->readUTF();
					aValue <<= aUTF;
				}
				else if ( *pType == ::getCppuType((const sal_uInt16*)0) )
				{
					sal_uInt16 n = InStream->readShort();
					aValue <<= n;
				}
				else if ( *pType == ::getCppuType((const sal_Int16*)0) )
				{
					sal_Int16 n = InStream->readShort();
					aValue <<= n;
				}
				else if ( *pType == ::getCppuType((const sal_uInt32*)0) )
				{
					sal_uInt32 n = InStream->readLong();
					aValue <<= n;
				}
				else if ( *pType == ::getCppuType((const sal_Int32*)0) )
				{
					sal_Int32 n = InStream->readLong();
					aValue <<= n;
				}
				else if ( *pType == ::getCppuType((const double*)0) )
				{
					double n = InStream->readDouble();
					aValue <<= n;
				}
				else if ( *pType == ::getCppuType((const ::com::sun::star::awt::FontDescriptor*)0) )
				{
					::com::sun::star::awt::FontDescriptor aFD;
					aFD.Name = InStream->readUTF();
					aFD.Height = InStream->readShort();
					aFD.Width = InStream->readShort();
					aFD.StyleName = InStream->readUTF();
					aFD.Family = InStream->readShort();
					aFD.CharSet = InStream->readShort();
					aFD.Pitch = InStream->readShort();
					aFD.CharacterWidth = (float)InStream->readDouble();
					aFD.Weight = (float)InStream->readDouble();
					aFD.Slant =  (::com::sun::star::awt::FontSlant)InStream->readShort();
					aFD.Underline = InStream->readShort();
					aFD.Strikeout = InStream->readShort();
					aFD.Orientation = (float)InStream->readDouble();
					aFD.Kerning = InStream->readBoolean();
					aFD.WordLineMode = InStream->readBoolean();
					aFD.Type = InStream->readShort();
					aValue <<= aFD;
				}
				else if ( *pType == ::getCppuType((const ::com::sun::star::uno::Sequence< ::rtl::OUString>*)0 ) )
				{
					long nEntries = InStream->readLong();
					::com::sun::star::uno::Sequence< ::rtl::OUString> aSeq( nEntries );
					for ( long n = 0; n < nEntries; n++ ) 
						aSeq.getArray()[n] = InStream->readUTF();
					aValue <<= aSeq;

				}
				else if ( *pType == ::getCppuType((const ::com::sun::star::uno::Sequence<sal_uInt16>*)0 ) )

				{
					long nEntries = InStream->readLong();
					::com::sun::star::uno::Sequence<sal_uInt16> aSeq( nEntries );
					for ( long n = 0; n < nEntries; n++ ) 
						aSeq.getArray()[n] = (sal_uInt16)InStream->readShort();
					aValue <<= aSeq;
				}
				else if ( *pType == ::getCppuType((const ::com::sun::star::uno::Sequence<sal_Int16>*)0 ) )
				{
					long nEntries = InStream->readLong();
					::com::sun::star::uno::Sequence<sal_Int16> aSeq( nEntries );
					for ( long n = 0; n < nEntries; n++ ) 
						aSeq.getArray()[n] = (sal_Int16)InStream->readShort();
					aValue <<= aSeq;
				}
                else if ( pType->getTypeClass() == TypeClass_ENUM )
                {
                    sal_Int32 nAsInt = InStream->readLong();
                    aValue = ::cppu::int2enum( nAsInt, *pType );
                }
                else
                {
                    ::rtl::OString sMessage( "UnoControlModel::read: don't know how to handle a property of type '" );
                    ::rtl::OUString sTypeName( pType->getTypeName() );
                    sMessage += ::rtl::OString( sTypeName.getStr(), sTypeName.getLength(), RTL_TEXTENCODING_ASCII_US );
                    sMessage += "'.\n(Currently handling property '";
                    ::rtl::OUString sPropertyName( GetPropertyName( nPropId ) );
                    sMessage += ::rtl::OString( sPropertyName.getStr(), sPropertyName.getLength(), osl_getThreadTextEncoding() );
                    sMessage += "'.)";
				    DBG_ERROR( sMessage );
                }
			}
			else
			{
				// Altes Geraffel aus 5.0
				if ( nPropId == BASEPROPERTY_FONT_TYPE )
				{
					// Sonst ist es nur die redundante Info fuer alte Versionen
					// Daten werden durch MarkableStream geskippt.
					if ( nVersion < 2 ) 
					{
						if ( !pFD )
						{
							pFD = new ::com::sun::star::awt::FontDescriptor;
							ImplControlProperty* pProp = mpData->Get( BASEPROPERTY_FONTDESCRIPTOR );
							if ( pProp ) // wegen den Defaults...
								pProp->GetValue() >>= *pFD;
						}
						pFD->Name = InStream->readUTF();
						pFD->StyleName = InStream->readUTF();
						pFD->Family = InStream->readShort();
						pFD->CharSet = InStream->readShort();
						pFD->Pitch = InStream->readShort();
					}
				}
				else if ( nPropId == BASEPROPERTY_FONT_SIZE )
				{
					if ( nVersion < 2 )
					{
						if ( !pFD )
						{
							pFD = new ::com::sun::star::awt::FontDescriptor;
							ImplControlProperty* pProp = mpData->Get( BASEPROPERTY_FONTDESCRIPTOR );
							if ( pProp ) // wegen den Defaults...
								pProp->GetValue() >>= *pFD;
						}
						pFD->Width = (sal_Int16)InStream->readLong();
						pFD->Height = (sal_Int16)InStream->readLong();
						InStream->readShort();	// ::com::sun::star::awt::FontWidth ignorieren - wurde mal falsch geschrieben und wird nicht gebraucht.
						pFD->CharacterWidth = ::com::sun::star::awt::FontWidth::DONTKNOW;
					}
				}
				else if ( nPropId == BASEPROPERTY_FONT_ATTRIBS )
				{
					if ( nVersion < 2 )
					{
 						if ( !pFD )	
						{
							pFD = new ::com::sun::star::awt::FontDescriptor;
							ImplControlProperty* pProp = mpData->Get( BASEPROPERTY_FONTDESCRIPTOR );
							if ( pProp ) // wegen den Defaults...
								pProp->GetValue() >>= *pFD;
						}
						pFD->Weight = VCLUnoHelper::ConvertFontWeight( (FontWeight) InStream->readShort() );
						pFD->Slant =  (::com::sun::star::awt::FontSlant)InStream->readShort();
						pFD->Underline = InStream->readShort();
						pFD->Strikeout = InStream->readShort();
						pFD->Orientation = ( (float)(double)InStream->readShort() ) / 10;
						pFD->Kerning = InStream->readBoolean();
						pFD->WordLineMode = InStream->readBoolean();
					}
				}
				else
				{
					DBG_ERROR( "read: unknown Property!" );
				}
			}
		}
		else // bVoid
		{
			if ( nPropId == BASEPROPERTY_FONTDESCRIPTOR )
			{
				EmptyFontDescriptor aFD;
				aValue <<= aFD;
			}
		}

		if ( mpData->Get( nPropId ) ) 
		{
			aProps.getArray()[i] = GetPropertyName( nPropId );
			aValues.getArray()[i] = aValue;
		}
		else
		{
			bInvalidEntries = sal_True;
		}

		// Falls bereits mehr drinsteht als diese Version kennt:
		xMark->jumpToMark( nPropDataBeginMark );
		InStream->skipBytes( nPropDataLen );
		xMark->deleteMark(nPropDataBeginMark);		
	}
	if ( bInvalidEntries ) 
	{
		for ( i = 0; i < (sal_uInt32)aProps.getLength(); i++ ) 
		{
			if ( !aProps.getConstArray()[i].getLength() ) 
			{
				::comphelper::removeElementAt( aProps, i );
				::comphelper::removeElementAt( aValues, i );
				i--;
			}
		}
	}
	
    try
    {
        setPropertyValues( aProps, aValues );
    }
    catch ( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

	if ( pFD ) 
	{
		::com::sun::star::uno::Any aValue;
		aValue <<= *pFD;
		setPropertyValue( GetPropertyName( BASEPROPERTY_FONTDESCRIPTOR ), aValue );
		delete pFD;
	}
}


// ::com::sun::star::lang::XServiceInfo
::rtl::OUString UnoControlModel::getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException)
{
	DBG_ERROR( "This method should be overloaded!" );
	return ::rtl::OUString();
	
}

sal_Bool UnoControlModel::supportsService( const ::rtl::OUString& rServiceName ) throw(::com::sun::star::uno::RuntimeException)
{
	::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
	
	::com::sun::star::uno::Sequence< ::rtl::OUString > aSNL = getSupportedServiceNames();
	const ::rtl::OUString * pArray = aSNL.getConstArray();
	for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
		if( pArray[i] == rServiceName )
			return sal_True;
	return sal_False;
}

::com::sun::star::uno::Sequence< ::rtl::OUString > UnoControlModel::getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException)
{
	::rtl::OUString sName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlModel" ) );
	return Sequence< ::rtl::OUString >( &sName, 1 );
}

// ::cppu::OPropertySetHelper
::cppu::IPropertyArrayHelper& UnoControlModel::getInfoHelper()
{
	DBG_ERROR( "UnoControlModel::getInfoHelper() not possible!" );
	return *(::cppu::IPropertyArrayHelper*) NULL;
}

// ------------------------------------------------------------------
template <class TYPE>
sal_Bool convertType(Any& _rConvertedValue, const Any& _rNewValueTest, const TYPE* /* _pTypeDisambiguation */)
{
	TYPE tValue;
	if (_rNewValueTest >>= tValue)
	{
		_rConvertedValue <<= tValue;
		return sal_True;
	}
}

// ..................................................................
sal_Bool UnoControlModel::convertFastPropertyValue( Any & rConvertedValue, Any & rOldValue, sal_Int32 nPropId, const Any& rValue ) throw (IllegalArgumentException)
{
	::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

	sal_Bool bVoid = rValue.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_VOID;
	if ( bVoid )
	{
		rConvertedValue.clear();
	}
	else
	{
		const ::com::sun::star::uno::Type* pDestType = GetPropertyType( (sal_uInt16)nPropId );
		if ( pDestType->getTypeClass() == TypeClass_ANY )
		{
			rConvertedValue = rValue;
		}
		else
		{
			if ( pDestType->equals( rValue.getValueType() ) )
            {
                rConvertedValue = rValue;
            }
			else
			{
				sal_Bool bConverted = sal_False;
				// 13.03.2001 - 84923 - frank.schoenheit@germany.sun.com

				switch (pDestType->getTypeClass())
				{
					case TypeClass_DOUBLE:
					{
						// try as double
						double nAsDouble = 0;
                        bConverted = ( rValue >>= nAsDouble );
						if ( bConverted )
							rConvertedValue <<= nAsDouble;
						else
						{	// try as integer - 96136 - 2002-10-08 - fs@openoffice.org
							sal_Int32 nAsInteger = 0;
                            bConverted = ( rValue >>= nAsInteger );
							if ( bConverted )
								rConvertedValue <<= (double)nAsInteger;
						}
					}
					break;
					case TypeClass_SHORT:
                    {
                        sal_Int16 n;
                        bConverted = ( rValue >>= n );
                        if ( bConverted )
                            rConvertedValue <<= n;
                    }
					break;
					case TypeClass_UNSIGNED_SHORT:
                    {
                        sal_uInt16 n;
                        bConverted = ( rValue >>= n );
                        if ( bConverted )
                            rConvertedValue <<= n;
                    }
					break;
					case TypeClass_LONG:
                    {
                        sal_Int32 n;
                        bConverted = ( rValue >>= n );
                        if ( bConverted )
                            rConvertedValue <<= n;
                    }
					break;
					case TypeClass_UNSIGNED_LONG:
                    {
                        sal_uInt32 n;
                        bConverted = ( rValue >>= n );
                        if ( bConverted )
                            rConvertedValue <<= n;
                    }
					break;
					case TypeClass_INTERFACE:
					{
						if ( rValue.getValueType().getTypeClass() == TypeClass_INTERFACE )
						{
							Reference< XInterface > xPure( rValue, UNO_QUERY );
							if ( xPure.is() )
								rConvertedValue = xPure->queryInterface( *pDestType );
                            else
                                rConvertedValue.setValue( NULL, *pDestType );
							bConverted = sal_True;
						}
					}
					break;
					case TypeClass_ENUM:
					{
                        sal_Int32 nValue = 0;
                        bConverted = ( rValue >>= nValue );
                        if ( bConverted )
                            rConvertedValue = ::cppu::int2enum( nValue, *pDestType );
					}
					break;
					default: ; // avoid compiler warning
				}

				if (!bConverted)
                {
                    ::rtl::OUStringBuffer aErrorMessage;
                    aErrorMessage.appendAscii( "Unable to convert the given value for the property " );
                    aErrorMessage.append     ( GetPropertyName( (sal_uInt16)nPropId ) );
                    aErrorMessage.appendAscii( ".\n" );
                    aErrorMessage.appendAscii( "Expected type: " );
                    aErrorMessage.append     ( pDestType->getTypeName() );
                    aErrorMessage.appendAscii( "\n" );
                    aErrorMessage.appendAscii( "Found type: " );
                    aErrorMessage.append     ( rValue.getValueType().getTypeName() );
					throw ::com::sun::star::lang::IllegalArgumentException(
                        aErrorMessage.makeStringAndClear(),
						static_cast< ::com::sun::star::beans::XPropertySet* >(this),
						1);
                }
			}
		}
	}

	// the current value
	getFastPropertyValue( rOldValue, nPropId );
	return !CompareProperties( rConvertedValue, rOldValue );
}

void UnoControlModel::setFastPropertyValue_NoBroadcast( sal_Int32 nPropId, const ::com::sun::star::uno::Any& rValue ) throw (::com::sun::star::uno::Exception)
{
	// Fehlt: Die gefakten Einzelproperties des FontDescriptors...
		
	ImplControlProperty* pProp = mpData->Get( nPropId );
    ENSURE_OR_RETURN_VOID( pProp, "UnoControlModel::setFastPropertyValue_NoBroadcast: invalid property id!" );

    DBG_ASSERT( ( rValue.getValueType().getTypeClass() != ::com::sun::star::uno::TypeClass_VOID ) || ( GetPropertyAttribs( (sal_uInt16)nPropId ) & ::com::sun::star::beans::PropertyAttribute::MAYBEVOID ), "Property darf nicht VOID sein!" );
	pProp->SetValue( rValue );
}

void UnoControlModel::getFastPropertyValue( ::com::sun::star::uno::Any& rValue, sal_Int32 nPropId ) const
{
	::osl::Guard< ::osl::Mutex > aGuard( ((UnoControlModel*)this)->GetMutex() );
	
	ImplControlProperty* pProp = mpData->Get( nPropId );
	
	if ( pProp )
		rValue = pProp->GetValue();
	else if ( ( nPropId >= BASEPROPERTY_FONTDESCRIPTORPART_START ) && ( nPropId <= BASEPROPERTY_FONTDESCRIPTORPART_END ) )
	{
		pProp = mpData->Get( BASEPROPERTY_FONTDESCRIPTOR );
		::com::sun::star::awt::FontDescriptor aFD;
		pProp->GetValue() >>= aFD;
		switch ( nPropId ) 
		{
			case BASEPROPERTY_FONTDESCRIPTORPART_NAME: 			rValue <<= aFD.Name;
																break;
			case BASEPROPERTY_FONTDESCRIPTORPART_STYLENAME:		rValue <<= aFD.StyleName;
																break;
			case BASEPROPERTY_FONTDESCRIPTORPART_FAMILY: 		rValue <<= aFD.Family;
																break;
			case BASEPROPERTY_FONTDESCRIPTORPART_CHARSET: 		rValue <<= aFD.CharSet;
																break;
			case BASEPROPERTY_FONTDESCRIPTORPART_HEIGHT: 		rValue <<= (float)aFD.Height;
																break;
			case BASEPROPERTY_FONTDESCRIPTORPART_WEIGHT: 		rValue <<= aFD.Weight;
																break;
			case BASEPROPERTY_FONTDESCRIPTORPART_SLANT: 		rValue <<= (sal_Int16)aFD.Slant;
																break;
			case BASEPROPERTY_FONTDESCRIPTORPART_UNDERLINE:		rValue <<= aFD.Underline;
																break;
			case BASEPROPERTY_FONTDESCRIPTORPART_STRIKEOUT:		rValue <<= aFD.Strikeout;
																break;
			case BASEPROPERTY_FONTDESCRIPTORPART_WIDTH:			rValue <<= aFD.Width;
																break;
			case BASEPROPERTY_FONTDESCRIPTORPART_PITCH:			rValue <<= aFD.Pitch;
																break;
			case BASEPROPERTY_FONTDESCRIPTORPART_CHARWIDTH:		rValue <<= aFD.CharacterWidth;
																break;
			case BASEPROPERTY_FONTDESCRIPTORPART_ORIENTATION:	rValue <<= aFD.Orientation;
																break;
			case BASEPROPERTY_FONTDESCRIPTORPART_KERNING:		rValue <<= aFD.Kerning;
																break;
			case BASEPROPERTY_FONTDESCRIPTORPART_WORDLINEMODE:	rValue <<= aFD.WordLineMode;
																break;
			case BASEPROPERTY_FONTDESCRIPTORPART_TYPE:			rValue <<= aFD.Type;
																break;
			default: DBG_ERROR( "FontProperty?!" );
		}
	}
	else
	{
		DBG_ERROR( "getFastPropertyValue - invalid Property!" );
	}
}

// ::com::sun::star::beans::XPropertySet
void UnoControlModel::setPropertyValue( const ::rtl::OUString& rPropertyName, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    sal_Int32 nPropId = 0;
    {
	    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
	    nPropId = (sal_Int32) GetPropertyId( rPropertyName );
	    DBG_ASSERT( nPropId, "Invalid ID in UnoControlModel::setPropertyValue" );
    }
	if( nPropId )
		setFastPropertyValue( nPropId, rValue );
	else
		throw ::com::sun::star::beans::UnknownPropertyException();
}

// ::com::sun::star::beans::XFastPropertySet
void UnoControlModel::setFastPropertyValue( sal_Int32 nPropId, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
	if ( ( nPropId >= BASEPROPERTY_FONTDESCRIPTORPART_START ) && ( nPropId <= BASEPROPERTY_FONTDESCRIPTORPART_END ) )
	{
		::osl::ClearableMutexGuard aGuard( GetMutex() );
		
        Any aOldSingleValue;
        getFastPropertyValue( aOldSingleValue, BASEPROPERTY_FONTDESCRIPTORPART_START );

        ImplControlProperty* pProp = mpData->Get( BASEPROPERTY_FONTDESCRIPTOR );
		FontDescriptor aOldFontDescriptor;
		pProp->GetValue() >>= aOldFontDescriptor;

        FontDescriptor aNewFontDescriptor( aOldFontDescriptor );
		lcl_ImplMergeFontProperty( aNewFontDescriptor, (sal_uInt16)nPropId, rValue );

		Any aNewValue;
		aNewValue <<= aNewFontDescriptor;
        sal_Int32 nDescriptorId( BASEPROPERTY_FONTDESCRIPTOR );
		nDescriptorId = BASEPROPERTY_FONTDESCRIPTOR;

        // also, we need  fire a propertyChange event for the single property, since with
        // the above line, only an event for the FontDescriptor property will be fired
        Any aNewSingleValue;
        getFastPropertyValue( aNewSingleValue, BASEPROPERTY_FONTDESCRIPTORPART_START );

        aGuard.clear();
		setFastPropertyValues( 1, &nDescriptorId, &aNewValue, 1 );
        fire( &nPropId, &aNewSingleValue, &aOldSingleValue, 1, sal_False );
   	}
    else
		setFastPropertyValues( 1, &nPropId, &rValue, 1 );
}

// ::com::sun::star::beans::XMultiPropertySet
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > UnoControlModel::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
	DBG_ERROR( "UnoControlModel::getPropertySetInfo() not possible!" );
	return ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >();
}

void UnoControlModel::setPropertyValues( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& rPropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Values ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
	::osl::ClearableMutexGuard aGuard( GetMutex() );
	
	sal_Int32 nProps = rPropertyNames.getLength();

//	sal_Int32* pHandles = new sal_Int32[nProps];
        // don't do this - it leaks in case of an exception
    Sequence< sal_Int32 > aHandles( nProps );
	sal_Int32* pHandles = aHandles.getArray();

	// may need to change the order in the sequence, for this we need a non-const value sequence
	// 15.05.2002 - 99314 - fs@openoffice.org
	uno::Sequence< uno::Any > aValues( Values );
	uno::Any* pValues = aValues.getArray();

	sal_Int32 nValidHandles = getInfoHelper().fillHandles( pHandles, rPropertyNames );

	if ( nValidHandles ) 
	{
		// if somebody sets properties which are single aspects of a font descriptor,
        // remove them, and build a font descriptor instead
        ::std::auto_ptr< awt::FontDescriptor > pFD;
		for ( sal_uInt16 n = 0; n < nProps; ++n ) 
		{
			if ( ( pHandles[n] >= BASEPROPERTY_FONTDESCRIPTORPART_START ) && ( pHandles[n] <= BASEPROPERTY_FONTDESCRIPTORPART_END ) )
			{
				if ( !pFD.get() ) 
				{
					ImplControlProperty* pProp = mpData->Get( BASEPROPERTY_FONTDESCRIPTOR );
					pFD.reset( new awt::FontDescriptor );
					pProp->GetValue() >>= *pFD;
				}
				lcl_ImplMergeFontProperty( *pFD, (sal_uInt16)pHandles[n], pValues[n] );
				pHandles[n] = -1;
				nValidHandles--;
			}
		}

		if ( nValidHandles )
		{
			ImplNormalizePropertySequence( nProps, pHandles, pValues, &nValidHandles );
            aGuard.clear();
                // clear our guard before calling into setFastPropertyValues - this method
                // will implicitly call property listeners, and this should not happen with
                // our mutex locked
                // #i23451# - 2004-03-18 - fs@openoffice.org
 			setFastPropertyValues( nProps, pHandles, pValues, nValidHandles );
		}
        else
            aGuard.clear();
            // same as a few lines above
		
		// FD-Propertie nicht in das Array mergen, weil sortiert...
		if ( pFD.get() ) 
		{
			::com::sun::star::uno::Any aValue;
			aValue <<= *pFD;
			sal_Int32 nHandle = BASEPROPERTY_FONTDESCRIPTOR;
			setFastPropertyValues( 1, &nHandle, &aValue, 1 );
		}
	}
}



void UnoControlModel::ImplNormalizePropertySequence( const sal_Int32, sal_Int32*,
	uno::Any*, sal_Int32* ) const SAL_THROW(())
{
	// nothing to do here
}

void UnoControlModel::ImplEnsureHandleOrder( const sal_Int32 _nCount, sal_Int32* _pHandles,
        uno::Any* _pValues, sal_Int32 _nFirstHandle, sal_Int32 _nSecondHandle ) const
{
	for ( sal_Int32 i=0; i < _nCount; ++_pHandles, ++_pValues, ++i )
	{
		if ( _nSecondHandle  == *_pHandles )
		{
			sal_Int32* pLaterHandles = _pHandles + 1;
			uno::Any* pLaterValues = _pValues + 1;
			for ( sal_Int32 j = i + 1; j < _nCount; ++j, ++pLaterHandles, ++pLaterValues )
			{
				if ( _nFirstHandle == *pLaterHandles )
				{
					// indeed it is -> exchange the both places in the sequences
					sal_Int32 nHandle( *_pHandles );
					*_pHandles = *pLaterHandles;
					*pLaterHandles = nHandle;

					uno::Any aValue( *_pValues );
					*_pValues = *pLaterValues;
					*pLaterValues = aValue;

					break;
					// this will leave the inner loop, and continue with the outer loop.
					// Note that this means we will encounter the _nSecondHandle handle, again, once we reached
					// (in the outer loop) the place where we just put it.
				}
			}
		}
	}
}
