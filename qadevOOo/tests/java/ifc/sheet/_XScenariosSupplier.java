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

package ifc.sheet;

import lib.MultiMethodTest;

import com.sun.star.sheet.XScenarios;
import com.sun.star.sheet.XScenariosSupplier;

/**
* Testing <code>com.sun.star.sheet.XSheetAnnotationsSupplier</code>
* interface methods :
* <ul>
*  <li><code> getAnnotations()</code></li>
* </ul> <p>
* @see com.sun.star.sheet.XSheetAnnotationsSupplier
*/
public class _XScenariosSupplier extends MultiMethodTest {
    public XScenariosSupplier oObj = null;

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _getScenarios(){
        XScenarios oSc = oObj.getScenarios();
        tRes.tested("getScenarios()", oSc != null);
    }

} // EOC _XScenariosSupplier

