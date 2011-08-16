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

#ifndef TOOLKIT_FORMATTED_CONTROL_HXX
#define TOOLKIT_FORMATTED_CONTROL_HXX

#include <toolkit/controls/unocontrols.hxx>
#include <toolkit/controls/unocontrolmodel.hxx>
#include <toolkit/helper/servicenames.hxx>

#include <com/sun/star/util/XNumberFormatter.hpp>

//........................................................................
namespace toolkit
{
//........................................................................

	// ===================================================================
	// = UnoControlFormattedFieldModel
	// ===================================================================
	class UnoControlFormattedFieldModel : public UnoControlModel
	{
	protected:	
		::com::sun::star::uno::Any 		ImplGetDefaultValue( sal_uInt16 nPropId ) const;
		::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
        ::com::sun::star::uno::Any      m_aCachedFormat;
        bool                            m_bRevokedAsClient;
        bool                            m_bSettingValueAndText;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >
                                        m_xCachedFormatter;

	protected:
		sal_Bool SAL_CALL convertFastPropertyValue(
					::com::sun::star::uno::Any& rConvertedValue,
					::com::sun::star::uno::Any& rOldValue,
					sal_Int32 nPropId,
					const ::com::sun::star::uno::Any& rValue
				) throw (::com::sun::star::lang::IllegalArgumentException);

        void SAL_CALL setFastPropertyValue_NoBroadcast(
                    sal_Int32 nHandle,
                    const ::com::sun::star::uno::Any& rValue
                ) throw (::com::sun::star::uno::Exception);

	public:
							UnoControlFormattedFieldModel( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& i_factory );
							UnoControlFormattedFieldModel( const UnoControlFormattedFieldModel& rModel )
                                :UnoControlModel( rModel )
                            {
                            }
							
		UnoControlModel*	Clone() const { return new UnoControlFormattedFieldModel( *this ); }
							
		// ::com::sun::star::io::XPersistObject
		::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);

		// ::com::sun::star::beans::XMultiPropertySet
		::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);


		// ::com::sun::star::lang::XServiceInfo
		DECLIMPL_SERVICEINFO_DERIVED( UnoControlFormattedFieldModel, UnoControlModel, szServiceName2_UnoControlFormattedFieldModel )

    protected:
        ~UnoControlFormattedFieldModel();

        // XComponent
        void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException);

        // XPropertySet
        void SAL_CALL setPropertyValues( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& PropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Values ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        // UnoControlModel
	    virtual void ImplNormalizePropertySequence(
					    const sal_Int32					_nCount,		/// the number of entries in the arrays
					    sal_Int32*						_pHandles,		/// the handles of the properties to set
					    ::com::sun::star::uno::Any*		_pValues,		/// the values of the properties to set
					    sal_Int32*						_pValidHandles	/// pointer to the valid handles, allowed to be adjusted
				    )	const SAL_THROW(());
    private:
        void    impl_updateTextFromValue_nothrow();
        void    impl_updateCachedFormatter_nothrow();
        void    impl_updateCachedFormatKey_nothrow();
	};

	// ===================================================================
	// = UnoFormattedFieldControl
	// ===================================================================
	class UnoFormattedFieldControl : public UnoSpinFieldControl
	{
	public:
							UnoFormattedFieldControl( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& i_factory );
		::rtl::OUString		GetComponentServiceName();

		// ::com::sun::star::awt::XTextListener
		void SAL_CALL textChanged( const ::com::sun::star::awt::TextEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException);
		
		// ::com::sun::star::lang::XServiceInfo
		DECLIMPL_SERVICEINFO_DERIVED( UnoFormattedFieldControl, UnoEditControl, szServiceName2_UnoControlFormattedField )
	};

//........................................................................
}	// namespace toolkit
//........................................................................

#endif // TOOLKIT_FORMATTED_CONTROL_HXX
