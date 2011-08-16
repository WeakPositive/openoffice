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
#include "precompiled_vcl.hxx"

#include <vcl/metaact.hxx>
#include <vcl/cvtgrf.hxx>

#include <salinst.hxx>
#include <svdata.hxx>

// --------------
// - Callback	-
// --------------

// --------------------
// - GraphicConverter -
// --------------------

GraphicConverter::GraphicConverter() :
	mpConvertData( NULL )
{
}

// ------------------------------------------------------------------------

GraphicConverter::~GraphicConverter()
{
}

// ------------------------------------------------------------------------

sal_uLong GraphicConverter::ImplConvert( sal_uLong nInFormat, void* pInBuffer, sal_uLong nInBufSize,
									 void** ppOutBuffer, sal_uLong nOutFormat )
{
	sal_uLong nRetBufSize = 0UL;

	if( ( nInFormat != nOutFormat ) && pInBuffer )
	{
		if( ( nInFormat == CVT_SVM ) || ( nInFormat == CVT_BMP ) )
		{
			SvMemoryStream	aIStm;
			Graphic			aGraphic;

			aIStm.SetBuffer( (char*) pInBuffer, nInBufSize, sal_False, nInBufSize );
			aIStm >> aGraphic;

			if( !aIStm.GetError() )
			{
				SvMemoryStream aOStm( 64535, 64535 );

				mpConvertData = new ConvertData( aGraphic, aOStm, nOutFormat );
				
				if( maFilterHdl.IsSet() && maFilterHdl.Call( mpConvertData ) )
				{
					nRetBufSize = aOStm.Seek( STREAM_SEEK_TO_END );
					*ppOutBuffer = (void*) aOStm.GetData();
					aOStm.ObjectOwnsMemory( sal_False );
				}

				delete mpConvertData;
				mpConvertData = NULL;
			}
		}
		else if( ( nOutFormat == CVT_SVM ) || ( nOutFormat == CVT_BMP ) )
		{
			SvMemoryStream	aIStm;

			aIStm.SetBuffer( (char*) pInBuffer, nInBufSize, sal_False, nInBufSize );
			mpConvertData = new ConvertData( Graphic(), aIStm, nInFormat );
				
			if( maFilterHdl.IsSet() && maFilterHdl.Call( mpConvertData ) )
			{
				SvMemoryStream	aOStm( 645535, 64535 );
				Graphic&		rGraphic = mpConvertData->maGraphic;

				if( ( rGraphic.GetType() == GRAPHIC_BITMAP ) && ( CVT_SVM == nOutFormat ) )
				{
					GDIMetaFile aMtf;

					aMtf.SetPrefSize( rGraphic.GetPrefSize() );
					aMtf.SetPrefMapMode( rGraphic.GetPrefMapMode() );
					aMtf.AddAction( new MetaBmpExScaleAction( Point(), aMtf.GetPrefSize(), rGraphic.GetBitmapEx() ) );
					rGraphic = aMtf;
				}
				else if( ( rGraphic.GetType() == GRAPHIC_GDIMETAFILE ) && ( CVT_BMP == nOutFormat ) )
					rGraphic = rGraphic.GetBitmapEx();

				aOStm << rGraphic;

				if( !aOStm.GetError() )
				{
					nRetBufSize = aOStm.Seek( STREAM_SEEK_TO_END );
					*ppOutBuffer = (void*) aOStm.GetData();
					aOStm.ObjectOwnsMemory( sal_False );
				}
			}

			delete mpConvertData;
			mpConvertData = NULL;
		}
	}

	return nRetBufSize;
}

// ------------------------------------------------------------------------

sal_uLong GraphicConverter::Import( SvStream& rIStm, Graphic& rGraphic, sal_uLong nFormat )
{
	GraphicConverter*	pCvt = ImplGetSVData()->maGDIData.mpGrfConverter;
	sal_uLong				nRet = ERRCODE_IO_GENERAL;

	if( pCvt && pCvt->GetFilterHdl().IsSet() )
	{
		ConvertData	aData( rGraphic, rIStm, nFormat );
		
		if( pCvt->GetFilterHdl().Call( &aData ) )
		{
			rGraphic = aData.maGraphic;
			nRet = ERRCODE_NONE;
		}
		else if( rIStm.GetError() )
			nRet = rIStm.GetError();
	}
	
	return nRet;
}

// ------------------------------------------------------------------------

sal_uLong GraphicConverter::Export( SvStream& rOStm, const Graphic& rGraphic, sal_uLong nFormat )
{
	GraphicConverter*	pCvt = ImplGetSVData()->maGDIData.mpGrfConverter;
	sal_uLong				nRet = ERRCODE_IO_GENERAL;

	if( pCvt && pCvt->GetFilterHdl().IsSet() )
	{
		ConvertData	aData( rGraphic, rOStm, nFormat );
		
		if( pCvt->GetFilterHdl().Call( &aData ) )
			nRet = ERRCODE_NONE;
		else if( rOStm.GetError() )
			nRet = rOStm.GetError();
	}
	
	return nRet;
}
