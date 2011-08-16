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
package org.openoffice.netbeans.editor;

import java.io.*;
import java.awt.event.KeyEvent;
import java.awt.event.InputEvent;
import java.awt.event.ActionEvent;
import java.net.URL;
import java.text.MessageFormat;

import java.util.Map;
import java.util.List;
import java.util.ResourceBundle;
import java.util.MissingResourceException;
import javax.swing.KeyStroke;
import javax.swing.JEditorPane;
import javax.swing.JMenuItem;
import javax.swing.Action;
import javax.swing.text.Document;
import javax.swing.text.JTextComponent;
import javax.swing.text.TextAction;
import javax.swing.text.BadLocationException;
import org.netbeans.editor.*;
import org.netbeans.editor.ext.*;
import org.netbeans.editor.ext.java.*;

/**
* Java editor kit with appropriate document
*
* @author Miloslav Metelka
* @version 1.00
*/

/* This class is based on the JavaKit class in the demosrc directory of
 * the editor module of the NetBeans project: http://www.netbeans.org
 *
 * The class sets up an EditorKit for syntax highlighting and code completion
 * of Java syntax
 */

public class JavaKit extends ExtKit {

    public static final String JAVA_MIME_TYPE = "text/x-java"; // NOI18N

    static final long serialVersionUID =-5445829962533684922L;

    static {
        Settings.addInitializer( new JavaSettingsInitializer( JavaKit.class ) );
        Settings.reset();

        URL skeleton = null, body = null;
        skeleton = JavaKit.class.getResource("OOo.jcs");
        body     = JavaKit.class.getResource("OOo.jcb");

        if (skeleton != null && body != null) {
            DAFileProvider provider = new DAFileProvider(
                new URLAccessor(skeleton),
                new URLAccessor(body));

            JCBaseFinder finder = new JCBaseFinder();
            finder.append( provider );
            JavaCompletion.setFinder( finder );
        }
    }

    public String getContentType() {
        return JAVA_MIME_TYPE;
    }

    /** Create new instance of syntax coloring scanner
    * @param doc document to operate on. It can be null in the cases the syntax
    *   creation is not related to the particular document
    */
    public Syntax createSyntax(Document doc) {
        return new JavaSyntax();
    }

    /** Create syntax support */
    public SyntaxSupport createSyntaxSupport(BaseDocument doc) {
        return new JavaSyntaxSupport(doc);
    }

    public Completion createCompletion(ExtEditorUI extEditorUI) {
        return new JavaCompletion(extEditorUI);
    }

    /** Create the formatter appropriate for this kit */
    public Formatter createFormatter() {
        return new JavaFormatter(this.getClass());
    }
    
    protected EditorUI createEditorUI() {
        return new ExtEditorUI();
    }

    protected void initDocument(BaseDocument doc) {
        doc.addLayer(new JavaDrawLayerFactory.JavaLayer(),
                JavaDrawLayerFactory.JAVA_LAYER_VISIBILITY);
        doc.addDocumentListener(new JavaDrawLayerFactory.LParenWatcher());
    }

    /**
     *   DataAccessor for parser DB files via URL streams
     *
     *   @author  Petr Nejedly
     */
    public static class URLAccessor implements DataAccessor {

        URL url;
        InputStream stream;
        int streamOff;
        int actOff;

        public URLAccessor(URL url) {
            this.url = url;
        }

        /** Not implemented
         */
        public void append(byte[] buffer, int off, int len)
            throws IOException
        {
            throw new IllegalArgumentException("read only!");
        }

        /**
         * Reads exactly <code>len</code> bytes from this file resource
         * into the byte array, starting at the current file pointer.
         * This method reads repeatedly from the file until the requested
         * number of bytes are read. This method blocks until the requested
         * number of bytes are read, the end of the inputStream is detected,
         * or an exception is thrown.
         *
         * @param      buffer     the buffer into which the data is read.
         * @param      off        the start offset of the data.
         * @param      len        the number of bytes to read.
         */
        public void read(byte[] buffer, int off, int len) throws IOException {
            InputStream str = getStream(actOff);
            while (len > 0) {
                int count = str.read(buffer, off, len);
                streamOff += count;
                off += count;
                len -= count;
            }
        }

        /** Opens DataAccessor file resource 
         *  @param requestWrite if true, file is opened for read/write
         */
        public void open(boolean requestWrite) throws IOException {
            if(requestWrite)
                throw new IllegalArgumentException("read only!");
        }

        /** Closes DataAccessor file resource  */
        public void close() throws IOException {
            if (stream != null) {
                stream.close();
                stream = null;
            }
        }

        /**
         * Returns the current offset in this file. 
         *
         * @return     the offset from the beginning of the file, in bytes,
         *             at which the next read or write occurs.
         */
        public long getFilePointer() throws IOException {
           return actOff;
        }
    
        /** Clears the file and sets the offset to 0 */
        public void resetFile() throws IOException {
            throw new IllegalArgumentException("read only!");
        }
    
        /**
         * Sets the file-pointer offset, measured from the beginning of this
         * file, at which the next read or write occurs.
         */    
        public void seek(long pos) throws IOException {
            actOff = (int)pos;
        }

        /** Gets InputStream prepared for reading from <code>off</code>
         *  offset position
         */
        private InputStream getStream(int off) throws IOException {
            if (streamOff > off && stream != null) {
                stream.close();
                stream = null;
            }

            if(stream == null) {
                stream = url.openStream();
                streamOff = 0;
            }

            while (streamOff < off) {
                long len = stream.skip(off - streamOff);
                streamOff += (int)len;
                if (len == 0) throw new IOException("EOF");
            }

            return stream;
        }    

        public int getFileLength() {
            try {
                int l =  url.openConnection().getContentLength();
                return l;
            } catch (IOException e) {
                return 0;
            }
        }

        public String toString() {
            return url.toString();
        }
    }
}
