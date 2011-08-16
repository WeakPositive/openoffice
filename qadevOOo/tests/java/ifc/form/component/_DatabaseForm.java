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

package ifc.form.component;

import lib.MultiPropertyTest;
import util.dbg;

import com.sun.star.form.TabulatorCycle;
import com.sun.star.uno.Enum;

/**
* Testing <code>com.sun.star.form.component.DatabaseForm</code>
* service properties:
* <ul>
*  <li><code> MasterFields</code></li>
*  <li><code> DetailFields</code></li>
*  <li><code> Cycle</code></li>
*  <li><code> NavigationBarMode</code></li>
*  <li><code> AllowInserts</code></li>
*  <li><code> AllowUpdates</code></li>
*  <li><code> AllowDeletes</code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.form.component.DatabaseForm
*/
public class _DatabaseForm extends MultiPropertyTest {

    /**
    * In this property test only debugging information output
    * is customized.
    */
    public void _NavigationBarMode() {
        dbg.printPropertyInfo(oObj, "NavigationBarMode");
        testProperty("NavigationBarMode", new PropertyTester() {
            public String toString(Object obj) {
                if (obj == null) {
                    return "null";
                } else {
                    return "(" + obj.getClass().toString() + ")"
                            + ((Enum)obj).getValue();
                }
            }
        });
    }

    /**
    * This property new value is always fixed and debugging
    * information output is customized.
    */
    public void _Cycle() {
        dbg.printPropertyInfo(oObj, "Cycle");
        testProperty("Cycle", new PropertyTester() {
            public Object getNewValue(String propName, Object oldValue) {
                return TabulatorCycle.CURRENT;
            }

            public String toString(Object obj) {
                if (obj == null) {
                    return "null";
                } else {
                    return "(" + obj.getClass().toString() + ")"
                            + ((Enum)obj).getValue();
                }
            }
        });
    }
}

