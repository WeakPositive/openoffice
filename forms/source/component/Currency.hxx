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

#ifndef _FORMS_CURRENCY_HXX_
#define _FORMS_CURRENCY_HXX_

#include "EditBase.hxx"

//.........................................................................
namespace frm
{
//.........................................................................

//==================================================================
//= OCurrencyModel
//==================================================================
class OCurrencyModel
				:public OEditBaseModel
{
	::com::sun::star::uno::Any			m_aSaveValue;

protected:
	virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> _getTypes();

public:
	DECLARE_DEFAULT_LEAF_XTOR( OCurrencyModel );

	// ::com::sun::star::lang::XServiceInfo
	IMPLEMENTATION_NAME(OCurrencyModel);
	virtual StringSequence SAL_CALL getSupportedServiceNames() throw();

	// ::com::sun::star::io::XPersistObject
	virtual ::rtl::OUString SAL_CALL getServiceName() throw ( ::com::sun ::star::uno::RuntimeException);

    // OControlModel's property handling
	virtual void describeFixedProperties(
		::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps
    ) const;

protected:
    // OBoundControlModel overridables
    virtual ::com::sun::star::uno::Any
                            translateDbColumnToControlValue( );
    virtual sal_Bool        commitControlValueToDbColumn( bool _bPostReset );

    virtual ::com::sun::star::uno::Any
                            getDefaultForReset() const;

	virtual void            resetNoBroadcast();

protected:
	DECLARE_XCLONEABLE();

	void implConstruct();
};

//==================================================================
//= OCurrencyControl
//==================================================================
class OCurrencyControl: public OBoundControl
{
protected:
	virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> _getTypes();

public:
	OCurrencyControl(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);
	// ::com::sun::star::lang::XServiceInfo
	IMPLEMENTATION_NAME(OCurrencyControl);
	virtual StringSequence SAL_CALL getSupportedServiceNames() throw();
};

//.........................................................................
}	// namespace frm
//.........................................................................

#endif // _FORMS_CURRENCY_HXX_

