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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_i18npool.hxx"
#include <stdio.h>
#include <string.h>
#include "LocaleNode.hxx"
//-----------------------------------------
// The document handler, which is needed for the saxparser
// The Documenthandler for reading sax
//-----------------------------------------
OFileWriter::OFileWriter(const char *pcFile, const char *locale ) {

	strncpy( m_pcFile , pcFile, sizeof(m_pcFile) );
    m_pcFile[sizeof(m_pcFile)-1] = 0;
	printf("file generated=%s\n", m_pcFile);
    m_f = fopen( m_pcFile , "w" );
    strncpy( theLocale, locale, sizeof(theLocale) );
    theLocale[sizeof(theLocale)-1] = 0;
}

OFileWriter::~OFileWriter() {
	if(m_f)
		fclose( m_f );
}

void OFileWriter::writeInt(sal_Int16 nb) const
{
	fprintf(m_f, "%d", nb);
}

void OFileWriter::writeAsciiString(const char* str) const
{
	fprintf(m_f, "%s", str);
}

void OFileWriter::writeStringCharacters(const ::rtl::OUString& str) const
{
	for(int i = 0; i < str.getLength(); i++)
	    fprintf(m_f, "0x%x, ", str[i]);
}

void OFileWriter::writeFunction(const char *func, const char *count, const char *array) const
{
	fprintf(m_f, "sal_Unicode **  SAL_CALL %s%s(sal_Int16& count)\n{\n", func, theLocale);
	fprintf(m_f, "\tcount = %s;\n", count);
	fprintf(m_f, "\treturn (sal_Unicode**)%s;\n}\n", array);
}

void OFileWriter::writeRefFunction(const char *func, const ::rtl::OUString& useLocale) const
{
    OString aRefLocale( OUStringToOString(useLocale, RTL_TEXTENCODING_ASCII_US) );
	const char* locale = aRefLocale.getStr();
	fprintf(m_f, "extern sal_Unicode **  SAL_CALL %s%s(sal_Int16& count);\n", func, locale);
	fprintf(m_f, "sal_Unicode **  SAL_CALL %s%s(sal_Int16& count)\n{\n", func, theLocale);
	fprintf(m_f, "\treturn %s%s(count);\n}\n", func, locale);
}

void OFileWriter::writeFunction(const char *func, const char *count, const char *array, const char *from, const char *to) const
{
	fprintf(m_f, "sal_Unicode **  SAL_CALL %s%s(sal_Int16& count, const sal_Unicode*& from, const sal_Unicode*& to)\n{\n", func, theLocale);
	fprintf(m_f, "\tcount = %s;\n", count);
	fprintf(m_f, "\tfrom = %s;\n", from);
	fprintf(m_f, "\tto = %s;\n", to);
	fprintf(m_f, "\treturn (sal_Unicode**)%s;\n}\n", array);
}

void OFileWriter::writeRefFunction(const char *func, const ::rtl::OUString& useLocale, const char *to) const
{
    OString aRefLocale( OUStringToOString(useLocale, RTL_TEXTENCODING_ASCII_US) );
	const char* locale = aRefLocale.getStr();
	fprintf(m_f, "extern sal_Unicode **  SAL_CALL %s%s(sal_Int16& count, const sal_Unicode*& from, const sal_Unicode*& to);\n", func, locale);
	fprintf(m_f, "sal_Unicode **  SAL_CALL %s%s(sal_Int16& count, const sal_Unicode*& from, const sal_Unicode*& to)\n{\n", func, theLocale);
	fprintf(m_f, "\tto = %s;\n", to);
	fprintf(m_f, "\tconst sal_Unicode* tmp;\n");
	fprintf(m_f, "\treturn %s%s(count, from, tmp);\n}\n", func, locale);
}

void OFileWriter::writeFunction2(const char *func, const char *style, const char* attr, const char *array) const
{
	fprintf(m_f, "const sal_Unicode ***  SAL_CALL %s%s( sal_Int16& nStyles, sal_Int16& nAttributes )\n{\n", func, theLocale);
	fprintf(m_f, "\tnStyles     = %s;\n", style);
	fprintf(m_f, "\tnAttributes = %s;\n", attr);
	fprintf(m_f, "\treturn %s;\n}\n", array);
}

void OFileWriter::writeRefFunction2(const char *func, const ::rtl::OUString& useLocale) const
{
    OString aRefLocale( OUStringToOString(useLocale, RTL_TEXTENCODING_ASCII_US) );
	const char* locale = aRefLocale.getStr();
	fprintf(m_f, "extern const sal_Unicode ***  SAL_CALL %s%s(sal_Int16& nStyles, sal_Int16& nAttributes);\n", func, locale);
	fprintf(m_f, "const sal_Unicode ***  SAL_CALL %s%s(sal_Int16& nStyles, sal_Int16& nAttributes)\n{\n", func, theLocale);
	fprintf(m_f, "\treturn %s%s(nStyles, nAttributes);\n}\n", func, locale);
}

void OFileWriter::writeFunction3(const char *func, const char *style, const char* levels, const char* attr, const char *array) const
{
	fprintf(m_f, "const sal_Unicode ****  SAL_CALL %s%s( sal_Int16& nStyles, sal_Int16& nLevels, sal_Int16& nAttributes )\n{\n", func, theLocale);
	fprintf(m_f, "\tnStyles     = %s;\n", style);
	fprintf(m_f, "\tnLevels     = %s;\n", levels);
	fprintf(m_f, "\tnAttributes = %s;\n", attr);
	fprintf(m_f, "\treturn %s;\n}\n", array);
}

void OFileWriter::writeRefFunction3(const char *func, const ::rtl::OUString& useLocale) const
{
    OString aRefLocale( OUStringToOString(useLocale, RTL_TEXTENCODING_ASCII_US) );
	const char* locale = aRefLocale.getStr();
	fprintf(m_f, "extern const sal_Unicode ****  SAL_CALL %s%s(sal_Int16& nStyles, sal_Int16& nLevels, sal_Int16& nAttributes);\n", func, locale);
	fprintf(m_f, "const sal_Unicode ****  SAL_CALL %s%s(sal_Int16& nStyles, sal_Int16& nLevels, sal_Int16& nAttributes)\n{\n", func, theLocale);
	fprintf(m_f, "\treturn %s%s(nStyles, nLevels, nAttributes);\n}\n", func, locale);
}

void OFileWriter::writeIntParameter(const sal_Char* pAsciiStr, const sal_Int16 count, sal_Int16 val) const
{
	fprintf(m_f, "static const sal_Unicode %s%d[] = {%d};\n", pAsciiStr, count, val);
}

bool OFileWriter::writeDefaultParameter(const sal_Char* pAsciiStr, const ::rtl::OUString& str, sal_Int16 count) const
{
    bool bBool = (str.equalsAscii("true") ? 1 : 0);
	fprintf(m_f,"static const sal_Unicode default%s%d[] = {%d};\n", pAsciiStr, count, bBool);
    return bBool;
}

bool OFileWriter::writeDefaultParameter(const sal_Char* pAsciiStr, const ::rtl::OUString& str) const
{
    bool bBool = (str.equalsAscii("true") ? 1 : 0);
	fprintf(m_f,"static const sal_Unicode default%s[] = {%d};\n", pAsciiStr, bBool);
    return bBool;
}

void OFileWriter::writeParameter(const sal_Char* pAsciiStr, const ::rtl::OUString& aChars) const
{
	fprintf(m_f, "static const sal_Unicode %s[] = {", pAsciiStr);
	writeStringCharacters(aChars);
	fprintf(m_f, "0x0};\n");
}

void OFileWriter::writeParameter(const sal_Char* pAsciiStr, const ::rtl::OUString& aChars, sal_Int16 count) const
{
	fprintf(m_f, "static const sal_Unicode %s%d[] = {", pAsciiStr, count);
	writeStringCharacters(aChars);
	fprintf(m_f, "0x0};\n");
}

void OFileWriter::writeParameter(const sal_Char* pAsciiStr, const ::rtl::OUString& aChars, sal_Int16 count0, sal_Int16 count1) const
{
	fprintf(m_f, "static const sal_Unicode %s%d%d[] = {", pAsciiStr, count0, count1);
	writeStringCharacters(aChars);
	fprintf(m_f, "0x0};\n");
}

void OFileWriter::writeParameter(const sal_Char* pTagStr, const sal_Char* pAsciiStr, const ::rtl::OUString& aChars, const sal_Int16 count) const
{
	fprintf(m_f, "static const sal_Unicode %s%s%d[] = {", pTagStr, pAsciiStr, count);
	writeStringCharacters(aChars);
	fprintf(m_f, "0x0};\n");
}

void OFileWriter::writeParameter(const sal_Char* pTagStr, const sal_Char* pAsciiStr, const ::rtl::OUString& aChars) const
{
	fprintf(m_f, "static const sal_Unicode %s%s[] = {", pTagStr, pAsciiStr);
	writeStringCharacters(aChars);
	fprintf(m_f, "0x0};\n");
}

void OFileWriter::writeParameter(const sal_Char* pTagStr, const sal_Char* pAsciiStr, const ::rtl::OUString& aChars, sal_Int16 count0, sal_Int16 count1) const
{
	fprintf(m_f, "static const sal_Unicode %s%s%d%d[] = {", pTagStr, pAsciiStr, count0, count1);
	writeStringCharacters(aChars);
	fprintf(m_f, "0x0};\n");
}

void OFileWriter::flush(void) const
{
	fflush( m_f );
}

void OFileWriter::closeOutput(void) const
{
	if(m_f)
	{
		fclose( m_f );
		const_cast< OFileWriter * > ( this )->m_f = 0;
	}
}

