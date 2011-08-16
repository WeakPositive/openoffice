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
#ifndef _XUNBUFFERED_STREAM_HXX
#define _XUNBUFFERED_STREAM_HXX

#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/xml/crypto/XCipherContext.hpp>

#include <cppuhelper/implbase1.hxx>
#include <rtl/ref.hxx>
#include <Inflater.hxx>
#include <ZipEntry.hxx>
#include <CRC32.hxx>
#include <mutexholder.hxx>

#define UNBUFF_STREAM_DATA			0
#define UNBUFF_STREAM_RAW			1
#define UNBUFF_STREAM_WRAPPEDRAW	2

class EncryptionData;
class XUnbufferedStream : public cppu::WeakImplHelper1
<
	com::sun::star::io::XInputStream
>
{
protected:
    SotMutexHolderRef maMutexHolder;

	com::sun::star::uno::Reference < com::sun::star::io::XInputStream > mxZipStream;
	com::sun::star::uno::Reference < com::sun::star::io::XSeekable > mxZipSeek;
	com::sun::star::uno::Sequence < sal_Int8 > maCompBuffer, maHeader;
	ZipEntry maEntry;
	::rtl::Reference< EncryptionData > mxData;
    sal_Int32 mnBlockSize;
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XCipherContext > m_xCipherContext;
	Inflater maInflater;
	sal_Bool mbRawStream, mbWrappedRaw, mbFinished;
	sal_Int16 mnHeaderToRead;
	sal_Int64 mnZipCurrent, mnZipEnd, mnZipSize, mnMyCurrent;
	CRC32 maCRC;
	sal_Bool mbCheckCRC;

public:
	XUnbufferedStream(
                 const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory,
                 SotMutexHolderRef aMutexHolder,
                 ZipEntry & rEntry,
				 com::sun::star::uno::Reference < com::sun::star::io::XInputStream > xNewZipStream,
				 const ::rtl::Reference< EncryptionData >& rData, 
				 sal_Int8 nStreamMode,
				 sal_Bool bIsEncrypted,
				 const ::rtl::OUString& aMediaType,
				 sal_Bool bRecoveryMode );

	// allows to read package raw stream
	XUnbufferedStream(
                 const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory,
                 const com::sun::star::uno::Reference < com::sun::star::io::XInputStream >& xRawStream,
				 const ::rtl::Reference< EncryptionData >& rData );


	virtual ~XUnbufferedStream();

	// XInputStream
    virtual sal_Int32 SAL_CALL readBytes( ::com::sun::star::uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead ) 
		throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL readSomeBytes( ::com::sun::star::uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead ) 
		throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip ) 
		throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL available(  ) 
		throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL closeInput(  ) 
		throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
	// XSeekable
	/*
    virtual void SAL_CALL seek( sal_Int64 location ) 
		throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int64 SAL_CALL getPosition(  ) 
		throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int64 SAL_CALL getLength(  ) 
		throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
	*/
};
#endif
