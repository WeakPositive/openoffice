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
#include "precompiled_svtools.hxx"
#include <com/sun/star/embed/XComponentSupplier.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/XVisualObject.hpp>
#include <com/sun/star/embed/XEmbedPersist.hpp>
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/embed/Aspects.hpp>

#include <svtools/embedtransfer.hxx>
#include <tools/mapunit.hxx>
#include <vcl/outdev.hxx>
#include <comphelper/storagehelper.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/tempfile.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include <svtools/embedhlp.hxx>

using namespace ::com::sun::star;

SvEmbedTransferHelper::SvEmbedTransferHelper( const uno::Reference< embed::XEmbeddedObject >& xObj,
												Graphic* pGraphic,
												sal_Int64 nAspect )
: m_xObj( xObj )
, m_pGraphic( pGraphic ? new Graphic( *pGraphic ) : NULL )
, m_nAspect( nAspect )
{
    if( xObj.is() )
    {
        TransferableObjectDescriptor aObjDesc;

        FillTransferableObjectDescriptor( aObjDesc, m_xObj, NULL, m_nAspect );
        PrepareOLE( aObjDesc );
    }
}

// -----------------------------------------------------------------------------

SvEmbedTransferHelper::~SvEmbedTransferHelper()
{
	if ( m_pGraphic )
	{
		delete m_pGraphic;
		m_pGraphic = NULL;
	}
}

// -----------------------------------------------------------------------------

void SvEmbedTransferHelper::AddSupportedFormats()
{
	AddFormat( SOT_FORMATSTR_ID_EMBED_SOURCE );
	AddFormat( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR );
	AddFormat( FORMAT_GDIMETAFILE );
}

// -----------------------------------------------------------------------------

sal_Bool SvEmbedTransferHelper::GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor )
{
	sal_Bool bRet = sal_False;

	if( m_xObj.is() )
	{
        try
        {
            sal_uInt32 nFormat = SotExchange::GetFormat( rFlavor );
            if( HasFormat( nFormat ) )
            {
                if( nFormat == SOT_FORMATSTR_ID_OBJECTDESCRIPTOR )
                {
                    TransferableObjectDescriptor aDesc;
                    FillTransferableObjectDescriptor( aDesc, m_xObj, m_pGraphic, m_nAspect );
                    bRet = SetTransferableObjectDescriptor( aDesc, rFlavor );
                }
                else if( nFormat == SOT_FORMATSTR_ID_EMBED_SOURCE )
                {
                    try
                    {
						// TODO/LATER: Propbably the graphic should be copied here as well
						// currently it is handled by the applications
                        utl::TempFile aTmp;
                        aTmp.EnableKillingFile( sal_True );
                        uno::Reference < embed::XEmbedPersist > xPers( m_xObj, uno::UNO_QUERY );
                        if ( xPers.is() )
                        {
                            uno::Reference < embed::XStorage > xStg = comphelper::OStorageHelper::GetTemporaryStorage();
                            ::rtl::OUString aName = ::rtl::OUString::createFromAscii("Dummy");
                            SvStream* pStream = NULL;
                            sal_Bool bDeleteStream = sal_False;
                            uno::Sequence < beans::PropertyValue > aEmpty;
                            xPers->storeToEntry( xStg, aName, aEmpty, aEmpty );
                            if ( xStg->isStreamElement( aName ) )
                            {
                                uno::Reference < io::XStream > xStm = xStg->cloneStreamElement( aName );
                                pStream = utl::UcbStreamHelper::CreateStream( xStm );
                                bDeleteStream = sal_True;
                            }
                            else
                            {
                                pStream = aTmp.GetStream( STREAM_STD_READWRITE );
                                uno::Reference < embed::XStorage > xStor = comphelper::OStorageHelper::GetStorageFromStream( new utl::OStreamWrapper( *pStream ) );
                                xStg->openStorageElement( aName, embed::ElementModes::READ )->copyToStorage( xStor );
                            }

                            ::com::sun::star::uno::Any                  aAny;
                            const sal_uInt32                            nLen = pStream->Seek( STREAM_SEEK_TO_END );
                            ::com::sun::star::uno::Sequence< sal_Int8 > aSeq( nLen );

                            pStream->Seek( STREAM_SEEK_TO_BEGIN );
                            pStream->Read( aSeq.getArray(),  nLen );
                            if ( bDeleteStream )
                                delete pStream;

                            if( ( bRet = ( aSeq.getLength() > 0 ) ) == sal_True )
                            {
                                aAny <<= aSeq;
                                SetAny( aAny, rFlavor );
                            }
                        }
                        else
                        {
                            //TODO/LATER: how to handle objects without persistance?!
                        }
                    }
                    catch ( uno::Exception& )
                    {
                    }
                }
                else if ( nFormat == FORMAT_GDIMETAFILE && m_pGraphic )
				{
					SvMemoryStream aMemStm( 65535, 65535 );
                    aMemStm.SetVersion( SOFFICE_FILEFORMAT_CURRENT );

					const GDIMetaFile& aMetaFile = m_pGraphic->GetGDIMetaFile();
					((GDIMetaFile*)(&aMetaFile))->Write( aMemStm );
					uno::Any aAny;
					aAny <<= uno::Sequence< sal_Int8 >( reinterpret_cast< const sal_Int8* >( aMemStm.GetData() ),
													aMemStm.Seek( STREAM_SEEK_TO_END ) );
					SetAny( aAny, rFlavor );
					bRet = sal_True;
				}
				else if ( m_xObj.is() && :: svt::EmbeddedObjectRef::TryRunningState( m_xObj ) )
                {
                    uno::Reference< datatransfer::XTransferable > xTransferable( m_xObj->getComponent(), uno::UNO_QUERY );
                    if ( xTransferable.is() )
                    {
                        uno::Any aAny = xTransferable->getTransferData( rFlavor );
                        SetAny( aAny, rFlavor );
                        bRet = sal_True;
                    }
                }
            }
		}
		catch( uno::Exception& )
		{
			// Error handling?
		}
	}

	return bRet;
}

// -----------------------------------------------------------------------------

void SvEmbedTransferHelper::ObjectReleased()
{
	m_xObj = uno::Reference< embed::XEmbeddedObject >();
}

void SvEmbedTransferHelper::FillTransferableObjectDescriptor( TransferableObjectDescriptor& rDesc,
    const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XEmbeddedObject >& xObj,
	Graphic* pGraphic,
	sal_Int64 nAspect )
{
    //TODO/LATER: need TypeName to fill it into the Descriptor (will be shown in listbox)
    ::com::sun::star::datatransfer::DataFlavor aFlavor;
    SotExchange::GetFormatDataFlavor( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR, aFlavor );

    rDesc.maClassName = SvGlobalName( xObj->getClassID() );
    rDesc.maTypeName = aFlavor.HumanPresentableName;

    //TODO/LATER: the aspect size in the descriptor is wrong, unfortunately the stream
	// representation of the descriptor allows only 4 bytes for the aspect
	// so for internal transport something different should be found
    rDesc.mnViewAspect = sal::static_int_cast<sal_uInt16>( nAspect );

    //TODO/LATER: status needs to become sal_Int64
    rDesc.mnOle2Misc = sal::static_int_cast<sal_Int32>(xObj->getStatus( rDesc.mnViewAspect ));

	Size aSize;
	MapMode aMapMode( MAP_100TH_MM );
	if ( nAspect == embed::Aspects::MSOLE_ICON )
	{
		if ( pGraphic )
		{
			aMapMode = pGraphic->GetPrefMapMode();
			aSize = pGraphic->GetPrefSize();
		}
		else
			aSize = Size( 2500, 2500 );
	}
	else
	{
		try
		{
    		awt::Size aSz;
			aSz = xObj->getVisualAreaSize( rDesc.mnViewAspect );
			aSize = Size( aSz.Width, aSz.Height );
		}
		catch( embed::NoVisualAreaSizeException& )
		{
			OSL_ENSURE( sal_False, "Can not get visual area size!\n" );
			aSize = Size( 5000, 5000 );
		}

    	// TODO/LEAN: getMapUnit can switch object to running state
    	aMapMode = MapMode( VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( rDesc.mnViewAspect ) ) );
	}

    rDesc.maSize = OutputDevice::LogicToLogic( aSize, aMapMode, MapMode( MAP_100TH_MM ) );
	rDesc.maDragStartPos = Point();
	rDesc.maDisplayName = String();
	rDesc.mbCanLink = sal_False;
}

