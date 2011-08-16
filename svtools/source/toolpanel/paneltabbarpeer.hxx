/*************************************************************************
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

#ifndef SVT_PANELTABBARPEER_HXX
#define SVT_PANELTABBARPEER_HXX

#include "svtaccessiblefactory.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <toolkit/awt/vclxwindow.hxx>

//......................................................................................................................
namespace svt
{
//......................................................................................................................

    class PanelTabBar;
	//====================================================================
	//= PanelTabBarPeer
	//====================================================================
	class PanelTabBarPeer : public VCLXWindow
	{
    public:
        PanelTabBarPeer( PanelTabBar& i_rTabBar );

    protected:
        ~PanelTabBarPeer();

        // VCLXWindow overridables
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > CreateAccessibleContext();

        // XComponent
        void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);

    private:
        AccessibleFactoryAccess m_aAccessibleFactory;
        PanelTabBar*            m_pTabBar;
	};

//........................................................................
} // namespace svt
//........................................................................

#endif // SVT_PANELTABBARPEER_HXX
