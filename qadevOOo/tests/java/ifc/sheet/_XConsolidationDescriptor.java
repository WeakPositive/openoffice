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
import util.ValueComparer;

import com.sun.star.sheet.GeneralFunction;
import com.sun.star.sheet.XConsolidationDescriptor;
import com.sun.star.table.CellAddress;
import com.sun.star.table.CellRangeAddress;

/**
* Testing <code>com.sun.star.sheet.XConsolidationDescriptor</code>
* interface methods :
* <ul>
*  <li><code> getFunction()</code></li>
*  <li><code> setFunction()</code></li>
*  <li><code> getSources()</code></li>
*  <li><code> setSources()</code></li>
*  <li><code> getStartOutputPosition()</code></li>
*  <li><code> setStartOutputPosition()</code></li>
*  <li><code> getUseColumnHeaders()</code></li>
*  <li><code> setUseColumnHeaders()</code></li>
*  <li><code> getUseRowHeaders()</code></li>
*  <li><code> setUseRowHeaders()</code></li>
*  <li><code> getInsertLinks()</code></li>
*  <li><code> setInsertLinks()</code></li>
* </ul> <p>
* @see com.sun.star.sheet.XConsolidationDescriptor
*/
public class _XConsolidationDescriptor extends MultiMethodTest {

    public XConsolidationDescriptor oObj = null;
    public GeneralFunction aFunction = null;
    public boolean insertLinks = false;
    public boolean useColumnHeaders = false;
    public boolean useRowHeaders = false;
    public CellRangeAddress[] CRaddr = null;
    public CellAddress addr = null;

    /**
    * Test calls the method, stores and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _getFunction() {
        aFunction = oObj.getFunction();
        tRes.tested("getFunction()", aFunction != null );
    }

    /**
    * Test calls the method and stores returned value. <p>
    * Has <b> OK </b> status if the method successfully returns. <p>
    */
    public void _getInsertLinks() {
        insertLinks = oObj.getInsertLinks();
        tRes.tested("getInsertLinks()", true);
    }

    /**
    * Test calls the method and stores returned value. <p>
    * Has <b> OK </b> status if the method successfully returns. <p>
    */
    public void _getSources() {
        CRaddr = oObj.getSources();
        int wiev = CRaddr.length;
        tRes.tested("getSources()", true);
    }

    /**
    * Test calls the method and stores returned value. <p>
    * Has <b> OK </b> status if the method successfully returns. <p>
    */
    public void _getStartOutputPosition() {
        addr = oObj.getStartOutputPosition();
        tRes.tested("getStartOutputPosition()", true);
    }

    /**
    * Test calls the method and stores returned value. <p>
    * Has <b> OK </b> status if the method successfully returns. <p>
    */
    public void _getUseColumnHeaders() {
        useColumnHeaders = oObj.getUseColumnHeaders();
        tRes.tested("getUseColumnHeaders()", true);
    }

    /**
    * Test calls the method and stores returned value. <p>
    * Has <b> OK </b> status if the method successfully returns. <p>
    */
    public void _getUseRowHeaders() {
        useRowHeaders = oObj.getUseRowHeaders();
        tRes.tested("getUseRowHeaders()", true);
    }

    /**
    * Test sets the new current function, gets current function
    * and compare function gotten by method <code>getFunction()</code> with
    * function that was set. <p>
    * Has <b> OK </b> status if the functions aren't equal. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getFunction() </code> : to have current value of
    *  the function by which the ranges are consolidated </li>
    * </ul>
    */
    public void _setFunction() {
        requiredMethod("getFunction()");
        GeneralFunction aFunction2 = null;
        if (aFunction.equals(GeneralFunction.SUM)) {
            aFunction2 = GeneralFunction.COUNT;
        }
        else {
            aFunction2 = GeneralFunction.SUM;
        }
        oObj.setFunction(aFunction2);
        aFunction2 = oObj.getFunction();
        tRes.tested("setFunction()", !aFunction.equals(aFunction2));
    }

    /**
    * Test sets property to value that negative to the current value
    * and compares returned value with value that was set. <p>
    * Has <b> OK </b> status if returned value isn't equal to value that was
    * stored by method <code>getInsertLinks()</code>. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getInsertLinks() </code> : to have current value of
    *  this property </li>
    * </ul>
    */
    public void _setInsertLinks() {
        requiredMethod("getInsertLinks()");
        oObj.setInsertLinks( !insertLinks );
        boolean insertLinks2 = oObj.getInsertLinks();
        tRes.tested("setInsertLinks()", insertLinks != insertLinks2);
    }

    /**
    * Test sets new cell ranges and compares cell ranges gotten by method
    * <code>getSources()</code> with the cell range that was set. <p>
    * Has <b> OK </b> status if returned value isn't equal to value that was
    * stored by method <code>getSources()</code>. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getSources() </code> : to have the cell ranges which
    *  are consolidated </li>
    * </ul>
    */
    public void _setSources() {
        requiredMethod("getSources()");
        oObj.setSources(newCRaddr());
        CellRangeAddress[] CRaddr2 = oObj.getSources();
        tRes.tested("setSources()",!ValueComparer.equalValue(CRaddr, CRaddr2));
    }

    /**
    * Test sets new cell address for start output position and compares
    * cell address gotten by method <code>getStartOutputPosition()</code>
    * with the cell address that was set. <p>
    * Has <b> OK </b> status if returned value isn't equal to value that was
    * stored by method <code>getStartOutputPosition()</code>. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getStartOutputPosition() </code> : to have the cell address
    *  of start output position </li>
    * </ul>
    */
    public void _setStartOutputPosition() {
        requiredMethod("getStartOutputPosition()");
        CellAddress addr2 = new CellAddress();
        addr2.Column = addr.Column + 1;
        addr2.Row = addr.Row + 1;
        oObj.setStartOutputPosition(addr2);
        tRes.tested("setStartOutputPosition()",
            !ValueComparer.equalValue(addr, addr2));
    }

    /**
    * Test sets property to value that negative to the current value
    * and compares returned value with value that was set. <p>
    * Has <b> OK </b> status if returned value isn't equal to value that was
    * stored by method <code>getUseColumnHeaders()</code>. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getUseColumnHeaders() </code> : to have current value of
    *  this property </li>
    * </ul>
    */
    public void _setUseColumnHeaders() {
        requiredMethod("getUseColumnHeaders()");
        oObj.setUseColumnHeaders( !useColumnHeaders );
        boolean uCH = oObj.getUseColumnHeaders();
        tRes.tested("setUseColumnHeaders()", useColumnHeaders != uCH);
    }

    /**
    * Test sets property to value that negative to the current value
    * and compares returned value with value that was set. <p>
    * Has <b> OK </b> status if returned value isn't equal to value that was
    * stored by method <code>getUseRowHeaders()</code>. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getUseRowHeaders() </code> : to have current value of
    *  this property </li>
    * </ul>
    */
    public void _setUseRowHeaders() {
        requiredMethod("getUseRowHeaders()");
        oObj.setUseRowHeaders(!useRowHeaders);
        boolean uRH = oObj.getUseRowHeaders();
        tRes.tested("setUseRowHeaders()", useRowHeaders != uRH );
    }

    /**
    * Constructs new cell range addresses using old cell range addresses.
    * @param CRaddr old cell range addresses
    * @return new cell range addresses
    */
    public CellRangeAddress[] newCRaddr() {

        CellRangeAddress[] back = new CellRangeAddress[1];

        CellRangeAddress cra1 = new CellRangeAddress();
        cra1.EndColumn=5;
        cra1.EndRow=5;
        cra1.Sheet=(short)0;
        cra1.StartColumn=1;
        cra1.StartRow=1;
        back[0]=cra1;

        return back;
    }


}  // finish class _XConsolidationDescriptor


