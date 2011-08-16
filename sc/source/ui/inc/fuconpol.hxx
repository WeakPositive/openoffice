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

#ifndef SC_FUCONPOL_HXX
#define SC_FUCONPOL_HXX

#include "fuconstr.hxx"

/*************************************************************************
|*
|* Basisklasse fuer alle Funktionen
|*
\************************************************************************/

class FuConstPolygon : public FuConstruct
{
	Point aLastPos;

 public:
	FuConstPolygon(ScTabViewShell* pViewSh, Window* pWin, ScDrawView* pView,
				   SdrModel* pDoc, SfxRequest& rReq);

	virtual ~FuConstPolygon();
									   // Mouse- & Key-Events
	virtual sal_Bool KeyInput(const KeyEvent& rKEvt);
	virtual sal_Bool MouseMove(const MouseEvent& rMEvt);
	virtual sal_Bool MouseButtonUp(const MouseEvent& rMEvt);
	virtual sal_Bool MouseButtonDown(const MouseEvent& rMEvt);

	virtual void Activate();		   // Function aktivieren
	virtual void Deactivate();		   // Function deaktivieren

	// #98185# Create default drawing objects via keyboard
	virtual SdrObject* CreateDefaultObject(const sal_uInt16 nID, const Rectangle& rRectangle);
};


#endif		// _FUCONPOL_HXX

