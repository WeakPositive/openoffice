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
#include "precompiled_dtrans.hxx"


//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------
#include <osl/diagnose.h>
#include "ImplHelper.hxx"
#include <rtl/tencinfo.h>
#include <rtl/memory.h>

#include <memory>
#if defined _MSC_VER
#pragma warning(push,1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#ifdef __MINGW32__
#include <excpt.h>
#endif

//------------------------------------------------------------------------
// defines
//------------------------------------------------------------------------

#define FORMATETC_EXACT_MATCH    1
#define FORMATETC_PARTIAL_MATCH -1
#define FORMATETC_NO_MATCH       0

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using ::rtl::OUString;
using ::rtl::OString;

//------------------------------------------------------------------------
// returns a windows codepage appropriate to the 
// given mime charset parameter value
//------------------------------------------------------------------------

sal_uInt32 SAL_CALL getWinCPFromMimeCharset( const OUString& charset )
{
	sal_uInt32 winCP = GetACP( );

	if ( charset.getLength( ) )
	{
		OString osCharset( 
			charset.getStr( ), charset.getLength( ), RTL_TEXTENCODING_ASCII_US );

		rtl_TextEncoding txtEnc = 
			rtl_getTextEncodingFromMimeCharset( osCharset.getStr( ) );

		sal_uInt32 winChrs = rtl_getBestWindowsCharsetFromTextEncoding( txtEnc );

		CHARSETINFO chrsInf;
		sal_Bool bRet = TranslateCharsetInfo( (DWORD*)winChrs, &chrsInf, TCI_SRCCHARSET ) ?
                        sal_True : sal_False;
	
		// if one of the above functions fails
		// we will return the current ANSI codepage
		// of this thread	
		if ( bRet )
			winCP = chrsInf.ciACP;
	}

	return winCP;
}

//--------------------------------------------------
// returns a windows codepage appropriate to the
// given locale and locale type
//--------------------------------------------------

OUString SAL_CALL getWinCPFromLocaleId( LCID lcid, LCTYPE lctype )
{
	OSL_ASSERT( IsValidLocale( lcid, LCID_SUPPORTED ) );
	
	// we set an default value 
	OUString winCP;
	
	// set an default value
	if ( LOCALE_IDEFAULTCODEPAGE == lctype )
	{
		winCP = OUString::valueOf( static_cast<sal_Int32>(GetOEMCP( )), 10 );
	}
	else if ( LOCALE_IDEFAULTANSICODEPAGE == lctype )
	{
		winCP = OUString::valueOf( static_cast<sal_Int32>(GetACP( )), 10 );
	}
	else
		OSL_ASSERT( sal_False );

	// we use the GetLocaleInfoA because don't want to provide
	// a unicode wrapper function for Win9x in sal/systools
	char buff[6];
	sal_Int32 nResult = GetLocaleInfoA( 
		lcid, lctype | LOCALE_USE_CP_ACP, buff, sizeof( buff ) );
	
	OSL_ASSERT( nResult );

	if ( nResult )
	{
		sal_Int32 len = MultiByteToWideChar( 
			CP_ACP, 0, buff, -1, NULL, 0 );

		OSL_ASSERT( len > 0 );

		std::auto_ptr< sal_Unicode > lpwchBuff( new sal_Unicode[len] );

		if ( NULL != lpwchBuff.get( ) )
		{
			len = MultiByteToWideChar(	
				CP_ACP, 0, buff, -1, reinterpret_cast<LPWSTR>(lpwchBuff.get( )), len );

			winCP = OUString( lpwchBuff.get( ), (len - 1) );
		}
	}

	return winCP;
}

//--------------------------------------------------
// returns a mime charset parameter value appropriate
// to the given codepage, optional a prefix can be 
// given, e.g. "windows-" or "cp"
//--------------------------------------------------

OUString SAL_CALL getMimeCharsetFromWinCP( sal_uInt32 cp, const OUString& aPrefix )
{
	return aPrefix + cptostr( cp );
}

//--------------------------------------------------
// returns a mime charset parameter value appropriate 
// to the given locale id and locale type, optional a
// prefix can be given, e.g. "windows-" or "cp"
//--------------------------------------------------

OUString SAL_CALL getMimeCharsetFromLocaleId( LCID lcid, LCTYPE lctype, const OUString& aPrefix  )
{
	OUString charset = getWinCPFromLocaleId( lcid, lctype );
	return aPrefix + charset;
}

//------------------------------------------------------------------------
// IsOEMCP
//------------------------------------------------------------------------

sal_Bool SAL_CALL IsOEMCP( sal_uInt32 codepage )
{
	OSL_ASSERT( IsValidCodePage( codepage ) );

	sal_uInt32 arrOEMCP[] = { 437, 708, 709, 710, 720, 737,
							  775, 850, 852, 855, 857, 860,
						      861, 862, 863, 864, 865, 866,
						      869, 874, 932, 936, 949, 950, 1361 };
	
	for ( sal_Int8 i = 0; i < ( sizeof( arrOEMCP )/sizeof( sal_uInt32 ) ); ++i )
		if ( arrOEMCP[i] == codepage )
			return sal_True;			

	return sal_False;
}

//------------------------------------------------------------------------
// converts a codepage into its string representation
//------------------------------------------------------------------------

OUString SAL_CALL cptostr( sal_uInt32 codepage )
{
	OSL_ASSERT( IsValidCodePage( codepage ) );

	return OUString::valueOf( static_cast<sal_Int64>( codepage ), 10 );
}

//-------------------------------------------------------------------------
// OleStdDeleteTargetDevice()
//
// Purpose:
//
// Parameters:
//
// Return Value:
//    SCODE  -  S_OK if successful
//-------------------------------------------------------------------------

void SAL_CALL DeleteTargetDevice( DVTARGETDEVICE* ptd )
{
#ifdef __MINGW32__
	jmp_buf jmpbuf;
	__SEHandler han;
	if (__builtin_setjmp(jmpbuf) == 0)
	{
		han.Set(jmpbuf, NULL, (__SEHandler::PF)EXCEPTION_EXECUTE_HANDLER);
#else
	__try
	{
#endif
		CoTaskMemFree( ptd );
	}
#ifdef __MINGW32__
	else
#else
	__except( EXCEPTION_EXECUTE_HANDLER )
#endif
	{
		OSL_ENSURE( sal_False, "Error DeleteTargetDevice" );
	}
#ifdef __MINGW32__
	han.Reset();
#endif
}



//-------------------------------------------------------------------------
// OleStdCopyTargetDevice()
//
// Purpose:
//  duplicate a TARGETDEVICE struct. this function allocates memory for
//  the copy. the caller MUST free the allocated copy when done with it
//  using the standard allocator returned from CoGetMalloc.
//  (OleStdFree can be used to free the copy).
//
// Parameters:
//  ptdSrc      pointer to source TARGETDEVICE
//
// Return Value:
//    pointer to allocated copy of ptdSrc
//    if ptdSrc==NULL then retuns NULL is returned.
//    if ptdSrc!=NULL and memory allocation fails, then NULL is returned
//-------------------------------------------------------------------------

DVTARGETDEVICE* SAL_CALL CopyTargetDevice( DVTARGETDEVICE* ptdSrc )
{
	DVTARGETDEVICE* ptdDest = NULL;

#ifdef __MINGW32__
	jmp_buf jmpbuf;
	__SEHandler han;
	if (__builtin_setjmp(jmpbuf) == 0)
	{
		han.Set(jmpbuf, NULL, (__SEHandler::PF)EXCEPTION_EXECUTE_HANDLER);
#else
	__try
	{
#endif
		if ( NULL != ptdSrc )
		{
			ptdDest = static_cast< DVTARGETDEVICE* >( CoTaskMemAlloc( ptdSrc->tdSize ) );
			rtl_copyMemory( ptdDest, ptdSrc, static_cast< size_t >( ptdSrc->tdSize ) );	
		}
	}
#ifdef __MINGW32__
	han.Reset();
#else
	__except( EXCEPTION_EXECUTE_HANDLER )
	{		
	}
#endif

	return ptdDest;
}


//-------------------------------------------------------------------------
// OleStdCopyFormatEtc()
//
// Purpose:
//  Copies the contents of a FORMATETC structure. this function takes
//  special care to copy correctly copying the pointer to the TARGETDEVICE
//  contained within the source FORMATETC structure.
//  if the source FORMATETC has a non-NULL TARGETDEVICE, then a copy
//  of the TARGETDEVICE will be allocated for the destination of the
//  FORMATETC (petcDest).
//
//  NOTE: the caller MUST free the allocated copy of the TARGETDEVICE
//  within the destination FORMATETC when done with it
//  using the standard allocator returned from CoGetMalloc.
//  (OleStdFree can be used to free the copy).
//
// Parameters:
//  petcDest      pointer to destination FORMATETC
//  petcSrc       pointer to source FORMATETC
//
// Return Value:
//  returns TRUE if copy was successful; 
//	retuns FALSE if not successful, e.g. one or both of the pointers
//	were invalid or the pointers were equal
//-------------------------------------------------------------------------

sal_Bool SAL_CALL CopyFormatEtc( LPFORMATETC petcDest, LPFORMATETC petcSrc )
{
	sal_Bool bRet = sal_False;

#ifdef __MINGW32__
	jmp_buf jmpbuf;
	__SEHandler han;
	if (__builtin_setjmp(jmpbuf) == 0)
	{
		han.Set(jmpbuf, NULL, (__SEHandler::PF)EXCEPTION_EXECUTE_HANDLER);
#else
	__try
	{
#endif
		if ( petcDest != petcSrc )
		{

		petcDest->cfFormat = petcSrc->cfFormat;
		
		petcDest->ptd      = NULL;
		if ( NULL != petcSrc->ptd )
			petcDest->ptd  = CopyTargetDevice(petcSrc->ptd);
		
		petcDest->dwAspect = petcSrc->dwAspect;
		petcDest->lindex   = petcSrc->lindex;
		petcDest->tymed    = petcSrc->tymed;
	
		bRet = sal_True;
		}
	}
#ifdef __MINGW32__
	else
#else
	__except( EXCEPTION_EXECUTE_HANDLER )
#endif
	{
		OSL_ENSURE( sal_False, "Error CopyFormatEtc" );
	}
#ifdef __MINGW32__
	han.Reset();
#endif

	return bRet;
}

//-------------------------------------------------------------------------
// returns:
//  1 for exact match, 
//  0 for no match, 
// -1 for partial match (which is defined to mean the left is a subset 
//    of the right: fewer aspects, null target device, fewer medium).
//-------------------------------------------------------------------------

sal_Int32 SAL_CALL CompareFormatEtc( const FORMATETC* pFetcLhs, const FORMATETC* pFetcRhs )
{
	sal_Int32 nMatch = FORMATETC_EXACT_MATCH;

#ifdef __MINGW32__
	jmp_buf jmpbuf;
	__SEHandler han;
	if (__builtin_setjmp(jmpbuf) == 0)
	{
		han.Set(jmpbuf, NULL, (__SEHandler::PF)EXCEPTION_EXECUTE_HANDLER);
#else
	__try
	{
#endif
		if ( pFetcLhs != pFetcRhs )

		if ( ( pFetcLhs->cfFormat != pFetcRhs->cfFormat ) || 
			 ( pFetcLhs->lindex   != pFetcRhs->lindex ) ||
			 !CompareTargetDevice( pFetcLhs->ptd, pFetcRhs->ptd ) )
		{
			nMatch = FORMATETC_NO_MATCH;
		}

		else if ( pFetcLhs->dwAspect == pFetcRhs->dwAspect )
			// same aspects; equal
			;
		else if ( ( pFetcLhs->dwAspect & ~pFetcRhs->dwAspect ) != 0 )
		{
			// left not subset of aspects of right; not equal
			nMatch = FORMATETC_NO_MATCH;
		}
		else
			// left subset of right
			nMatch = FORMATETC_PARTIAL_MATCH;

		if ( nMatch == FORMATETC_EXACT_MATCH || nMatch == FORMATETC_PARTIAL_MATCH )
		{
		if ( pFetcLhs->tymed == pFetcRhs->tymed )
			// same medium flags; equal
			;
		else if ( ( pFetcLhs->tymed & ~pFetcRhs->tymed ) != 0 )
		{
			// left not subset of medium flags of right; not equal
			nMatch = FORMATETC_NO_MATCH;
		}
		else
			// left subset of right
			nMatch = FORMATETC_PARTIAL_MATCH;
		}
	}
#ifdef __MINGW32__
	else
#else
	__except( EXCEPTION_EXECUTE_HANDLER )
#endif
	{
		OSL_ENSURE( sal_False, "Error CompareFormatEtc" );
		nMatch = FORMATETC_NO_MATCH;
	}
#ifdef __MINGW32__
	han.Reset();
#endif

    return nMatch;
}


//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------

sal_Bool SAL_CALL CompareTargetDevice( DVTARGETDEVICE* ptdLeft, DVTARGETDEVICE* ptdRight )
{
	sal_Bool bRet = sal_False;

#ifdef __MINGW32__
	jmp_buf jmpbuf;
	__SEHandler han;
	if (__builtin_setjmp(jmpbuf) == 0)
	{
		han.Set(jmpbuf, NULL, (__SEHandler::PF)EXCEPTION_EXECUTE_HANDLER);
#else
	__try
	{
#endif
		if ( ptdLeft == ptdRight )
		{
			// same address of td; must be same (handles NULL case)
			bRet = sal_True;
		}

		// one ot the two is NULL
		else if ( ( NULL != ptdRight ) && ( NULL != ptdLeft ) )
		
		if ( ptdLeft->tdSize == ptdRight->tdSize )		

		if ( rtl_compareMemory( ptdLeft, ptdRight, ptdLeft->tdSize ) == 0 )			
			bRet = sal_True;
	}
#ifdef __MINGW32__
	else
#else
	__except( EXCEPTION_EXECUTE_HANDLER )
#endif
	{
		OSL_ENSURE( sal_False, "Error CompareTargetDevice" );
		bRet = sal_False;
	}
#ifdef __MINGW32__
	han.Reset();
#endif

    return bRet;
}
