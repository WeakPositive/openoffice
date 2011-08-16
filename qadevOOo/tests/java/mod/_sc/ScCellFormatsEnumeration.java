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

package mod._sc;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.container.XEnumerationAccess;
import com.sun.star.container.XIndexAccess;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XCellFormatRangesSupplier;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.sheet.CellFormatRangesEnumeration</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::container::XEnumeration</code></li>
* </ul>
* @see com.sun.star.sheet.CellFormatRangesEnumeration
* @see com.sun.star.container.XEnumeration
* @see ifc.container._XEnumeration
*/
public class ScCellFormatsEnumeration extends TestCase {
    static XSpreadsheetDocument xSheetDoc = null;

    /**
    * Creates Spreadsheet document.
    */
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF());

        try {
            log.println( "creating a sheetdocument" );
            xSheetDoc = SOF.createCalcDoc(null);;
        } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e);
        }
    }

    /**
    * Disposes Spreadsheet document.
    */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xSheetDoc " );
        XComponent oComp = (XComponent)
            UnoRuntime.queryInterface (XComponent.class, xSheetDoc);
        util.DesktopTools.closeDoc(oComp);
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Retrieves a collection of spreadsheets from a document,
    * and takes one of them. Then retrieves a collection of cell format range
    * using the interface <code>XCellFormatRangesSupplier</code>, creates the
    * enumeration of this collection using interface <code>XEnumerationAccess</code>.
    * This enumeration is the instance of the service
    * <code>com.sun.star.sheet.CellFormatRangesEnumeration</code>.
    * Object relations created :
    * <ul>
    *  <li> <code>'ENUM'</code> for
    *      {@link ifc.container._XEnumeration} (type of
    *      <code>XEnumerationAccess</code> that was queried from the collection
    *      of cell format range)</li>
    * </ul>
    * @see com.sun.star.sheet.CellFormatRangesEnumeration
    * @see com.sun.star.sheet.XCellFormatRangesSupplier
    * @see com.sun.star.container.XEnumerationAccess
    */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XInterface oObj = null;

        log.println("getting sheets");
        XSpreadsheets xSpreadsheets = (XSpreadsheets)xSheetDoc.getSheets();

        log.println("getting a sheet");
        XSpreadsheet oSheet = null;
        XIndexAccess oIndexAccess = (XIndexAccess)
            UnoRuntime.queryInterface(XIndexAccess.class, xSpreadsheets);

        try {
            oSheet = (XSpreadsheet) AnyConverter.toObject(
                    new Type(XSpreadsheet.class),oIndexAccess.getByIndex(0));
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException( "Couldn't get a spreadsheet", e);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            throw new StatusException( "Couldn't get a spreadsheet", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException( "Couldn't get a spreadsheet", e);
        }

        log.println("getting CellFormats");

        XCellFormatRangesSupplier xCFRS = (XCellFormatRangesSupplier)
            UnoRuntime.queryInterface(XCellFormatRangesSupplier.class,oSheet);
        XIndexAccess formats = xCFRS.getCellFormatRanges();

        log.println("getting Enumeration");
        XEnumerationAccess oEnum = (XEnumerationAccess)
            UnoRuntime.queryInterface(XEnumerationAccess.class,formats);
        oObj = oEnum.createEnumeration();

        log.println("creating a new environment for object");
        TestEnvironment tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("ENUM", oEnum);

        return tEnv;
    }

}    // finish class ScCellFormatsEnumeration

