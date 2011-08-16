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

#ifndef _GOODIES_GRAPHICTRANSFORMER_HXX 
#define _GOODIES_GRAPHICTRANSFORMER_HXX 

#include <cppuhelper/implbase1.hxx> 
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/graphic/XGraphicTransformer.hpp>

using namespace com::sun::star;

namespace unographic {

// ----------------------
// - GraphicTransformer -
// ----------------------

typedef ::cppu::WeakAggImplHelper1<
    ::com::sun::star::graphic::XGraphicTransformer
	> GraphicTransformer_UnoImplHelper1;
class GraphicTransformer : public GraphicTransformer_UnoImplHelper1
{
	public:

	GraphicTransformer();
	~GraphicTransformer();
		
	// XGraphicTransformer
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > SAL_CALL colorChange( 
		const ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic >& rGraphic,
			sal_Int32 nColorFrom, sal_Int8 nTolerance, sal_Int32 nColorTo, sal_Int8 nAlphaTo )
				throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
	
};

}

#endif 
