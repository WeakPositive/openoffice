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

#include <tools/prewin.h>
#if defined _MSC_VER
#pragma warning(push, 1)
#pragma warning(disable: 4917)
#endif
#include <windows.h>
#include <objbase.h>
#include <strmif.h>
#include <control.h>
#include <uuids.h>
#include <evcode.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#include <tools/postwin.h>

#include "player.hxx"
#include "framegrabber.hxx"
#include "window.hxx"

#define AVMEDIA_WIN_PLAYER_IMPLEMENTATIONNAME "com.sun.star.comp.avmedia.Player_DirectX"
#define AVMEDIA_WIN_PLAYER_SERVICENAME "com.sun.star.media.Player_DirectX"

using namespace ::com::sun::star;

namespace avmedia { namespace win {

LRESULT CALLBACK MediaPlayerWndProc_2( HWND hWnd,UINT nMsg, WPARAM nPar1, LPARAM nPar2 )
{
    Player* pPlayer = (Player*) ::GetWindowLong( hWnd, 0 );
    bool    bProcessed = true;

    if( pPlayer )
    {
        switch( nMsg )
        {
            case( WM_GRAPHNOTIFY ):
                pPlayer->processEvent();
            break;
            default:
                bProcessed = false;
            break;
        }
    }
    else
        bProcessed = false;

    return( bProcessed ? 0 : DefWindowProc( hWnd, nMsg, nPar1, nPar2 ) );
}


bool isWindowsVistaOrHigher()
{
    // POST: return true if we are at least on Windows Vista
    OSVERSIONINFO osvi;
    ZeroMemory(&osvi, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionEx(&osvi);
    return osvi.dwMajorVersion >= 6;
}

// ----------------
// - Player -
// ----------------

Player::Player( const uno::Reference< lang::XMultiServiceFactory >& rxMgr ) :
    Player_BASE(m_aMutex),
    mxMgr( rxMgr ),
    mpGB( NULL ),
    mpOMF( NULL ),
    mpMC( NULL ),
    mpME( NULL ),
    mpMS( NULL ),
    mpMP( NULL ),
    mpBA( NULL ),
    mpBV( NULL ),
    mpVW( NULL ),
    mpEV( NULL ),
    mnUnmutedVolume( 0 ),
	mnFrameWnd( 0 ),
    mbMuted( false ),
    mbLooping( false ),
	mbAddWindow(sal_True)
{
    ::CoInitialize( NULL );
}

// ------------------------------------------------------------------------------

Player::~Player()
{
	if( mnFrameWnd )
        ::DestroyWindow( (HWND) mnFrameWnd );

    ::CoUninitialize();
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    stop();
    if( mpBA )
        mpBA->Release();

    if( mpBV )
        mpBV->Release();

    if( mpVW )
        mpVW->Release();

    if( mpMP )
        mpMP->Release();
        
    if( mpMS )
        mpMS->Release();
    
    if( mpME )
	{
		mpME->SetNotifyWindow( 0, WM_GRAPHNOTIFY, 0);
        mpME->Release();
	}

        
    if( mpMC )
        mpMC->Release();
    
    if( mpEV )
        mpEV->Release();
        
    if( mpOMF )
        mpOMF->Release();
    
    if( mpGB )
        mpGB->Release();
}
// ------------------------------------------------------------------------------
bool Player::create( const ::rtl::OUString& rURL )
{
    HRESULT hR;
    bool    bRet = false;

    if( SUCCEEDED( hR = CoCreateInstance( CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**) &mpGB ) ) )
    {
        // Don't use the overlay mixer on Windows Vista
		// It disables the desktop composition as soon as RenderFile is called
		// also causes some other problems: video rendering is not reliable
		if( !isWindowsVistaOrHigher() && SUCCEEDED( CoCreateInstance( CLSID_OverlayMixer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**) &mpOMF ) ) )
		{
			mpGB->AddFilter( mpOMF, L"com_sun_star_media_OverlayMixerFilter" );
	            
			if( !SUCCEEDED( mpOMF->QueryInterface( IID_IDDrawExclModeVideo, (void**) &mpEV ) ) )
				mpEV = NULL;
		}
        
        if( SUCCEEDED( hR = mpGB->RenderFile( reinterpret_cast<LPCWSTR>(rURL.getStr()), NULL ) ) &&
            SUCCEEDED( hR = mpGB->QueryInterface( IID_IMediaControl, (void**) &mpMC ) ) &&
            SUCCEEDED( hR = mpGB->QueryInterface( IID_IMediaEventEx, (void**) &mpME ) ) &&
            SUCCEEDED( hR = mpGB->QueryInterface( IID_IMediaSeeking, (void**) &mpMS ) ) &&
            SUCCEEDED( hR = mpGB->QueryInterface( IID_IMediaPosition, (void**) &mpMP ) ) )
        {
            // Video interfaces
            mpGB->QueryInterface( IID_IVideoWindow, (void**) &mpVW );
            mpGB->QueryInterface( IID_IBasicVideo, (void**) &mpBV );

            // Audio interface
            mpGB->QueryInterface( IID_IBasicAudio, (void**) &mpBA );

            if( mpBA )
                mpBA->put_Volume( mnUnmutedVolume );

            bRet = true;
        }
    }
    
    if( bRet )
        maURL = rURL;
    else
        maURL = ::rtl::OUString();

    return bRet;
}

// ------------------------------------------------------------------------------

const IVideoWindow* Player::getVideoWindow() const
{
    return mpVW;
}

// ------------------------------------------------------------------------------

void Player::setNotifyWnd( int nNotifyWnd )
{
	mbAddWindow = sal_False;
    if( mpME )
        mpME->SetNotifyWindow( (OAHWND) nNotifyWnd, WM_GRAPHNOTIFY, reinterpret_cast< LONG_PTR>( this ) );
}

// ------------------------------------------------------------------------------

void Player::setDDrawParams( IDirectDraw* pDDraw, IDirectDrawSurface* pDDrawSurface )
{
    if( mpEV && pDDraw && pDDrawSurface )
    {
        mpEV->SetDDrawObject( pDDraw );
        mpEV->SetDDrawSurface( pDDrawSurface );
    }
}

// ------------------------------------------------------------------------------

long Player::processEvent()
{   
    long nCode, nParam1, nParam2;
    
    while( mpME && SUCCEEDED( mpME->GetEvent( &nCode, &nParam1, &nParam2, 0 ) ) )
    {
        if( EC_COMPLETE == nCode )
        {
            if( mbLooping )
            { 
                setMediaTime( 0.0 );
                start();
            }
            else
            {
                setMediaTime( getDuration() );
                stop();
            }
        }
   
        mpME->FreeEventParams( nCode, nParam1, nParam2 );
    }
    
    return 0;
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::start(  )
    throw (uno::RuntimeException)
{
	::osl::MutexGuard aGuard(m_aMutex);
    if( mpMC )
	{
		if ( mbAddWindow )
		{
			static WNDCLASS* mpWndClass = NULL;
			if ( !mpWndClass )
			{
				mpWndClass = new WNDCLASS;

				memset( mpWndClass, 0, sizeof( *mpWndClass ) );
				mpWndClass->hInstance = GetModuleHandle( NULL );
				mpWndClass->cbWndExtra = sizeof( DWORD );
				mpWndClass->lpfnWndProc = MediaPlayerWndProc_2;
				mpWndClass->lpszClassName = "com_sun_star_media_Sound_Player";
				mpWndClass->hbrBackground = (HBRUSH) ::GetStockObject( BLACK_BRUSH );
				mpWndClass->hCursor = ::LoadCursor( NULL, IDC_ARROW );

				::RegisterClass( mpWndClass );
			}
			if ( !mnFrameWnd )
			{
				mnFrameWnd = (int) ::CreateWindow( mpWndClass->lpszClassName, NULL,
                                           0,
                                           0, 0, 0, 0,
                                           (HWND) NULL, NULL, mpWndClass->hInstance, 0 );
				if ( mnFrameWnd )
				{
					::ShowWindow((HWND) mnFrameWnd, SW_HIDE);
					::SetWindowLong( (HWND) mnFrameWnd, 0, (DWORD) this );
					// mpVW->put_Owner( (OAHWND) mnFrameWnd );
					setNotifyWnd( mnFrameWnd );
				}
			}
		}

        mpMC->Run();
	}
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::stop(  )
    throw (uno::RuntimeException)
{
	::osl::MutexGuard aGuard(m_aMutex);

    if( mpMC )
        mpMC->Stop();
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL Player::isPlaying()
    throw (uno::RuntimeException)
{
	::osl::MutexGuard aGuard(m_aMutex);

    OAFilterState   eFilterState;
    bool            bRet = false;

    if( mpMC && SUCCEEDED( mpMC->GetState( 10, &eFilterState ) ) )
        bRet = ( State_Running == eFilterState );

    return bRet;
}

// ------------------------------------------------------------------------------

double SAL_CALL Player::getDuration(  )
    throw (uno::RuntimeException)
{
	::osl::MutexGuard aGuard(m_aMutex);

    REFTIME aRefTime( 0.0 );

    if( mpMP  )
        mpMP->get_Duration( &aRefTime );
        
    return aRefTime;
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::setMediaTime( double fTime )
    throw (uno::RuntimeException)
{
	::osl::MutexGuard aGuard(m_aMutex);

    if( mpMP  )
    {
        const bool bPlaying = isPlaying();

        mpMP->put_CurrentPosition( fTime );

        if( !bPlaying && mpMC )
            mpMC->StopWhenReady();
    }
}

// ------------------------------------------------------------------------------

double SAL_CALL Player::getMediaTime(  )
    throw (uno::RuntimeException)
{
	::osl::MutexGuard aGuard(m_aMutex);

    REFTIME aRefTime( 0.0 );

    if( mpMP  )
        mpMP->get_CurrentPosition( &aRefTime );
    
    return aRefTime; 
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::setStopTime( double fTime )
    throw (uno::RuntimeException)
{
	::osl::MutexGuard aGuard(m_aMutex);

    if( mpMP  )
        mpMP->put_StopTime( fTime );
}

// ------------------------------------------------------------------------------

double SAL_CALL Player::getStopTime(  )
    throw (uno::RuntimeException)
{
	::osl::MutexGuard aGuard(m_aMutex);

    REFTIME aRefTime( 0.0 );

    if( mpMP  )
        mpMP->get_StopTime( &aRefTime );

    return aRefTime; 
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::setRate( double fRate )
    throw (uno::RuntimeException)
{
	::osl::MutexGuard aGuard(m_aMutex);

    if( mpMP  )
        mpMP->put_Rate( fRate );
}

// ------------------------------------------------------------------------------

double SAL_CALL Player::getRate(  )
    throw (uno::RuntimeException)
{
	::osl::MutexGuard aGuard(m_aMutex);

    double fRet( 0.0 );

    if( mpMP  )
        mpMP->get_Rate( &fRet );
    
    return fRet;
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::setPlaybackLoop( sal_Bool bSet )
    throw (uno::RuntimeException)
{
	::osl::MutexGuard aGuard(m_aMutex);

    mbLooping = bSet;
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL Player::isPlaybackLoop(  )
    throw (uno::RuntimeException)
{
	::osl::MutexGuard aGuard(m_aMutex);

    return mbLooping;
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::setMute( sal_Bool bSet )
    throw (uno::RuntimeException)
{
	::osl::MutexGuard aGuard(m_aMutex);

    if( mpBA && ( mbMuted != bSet ) )
    {
        mbMuted = bSet;
        mpBA->put_Volume( mbMuted ? -10000 : mnUnmutedVolume );
    }
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL Player::isMute(  )
    throw (uno::RuntimeException)
{
	::osl::MutexGuard aGuard(m_aMutex);

    return mbMuted;
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::setVolumeDB( sal_Int16 nVolumeDB ) 
    throw (uno::RuntimeException)
{
	::osl::MutexGuard aGuard(m_aMutex);

    mnUnmutedVolume = static_cast< long >( nVolumeDB ) * 100;

    if( !mbMuted && mpBA )
        mpBA->put_Volume( mnUnmutedVolume );
}

// ------------------------------------------------------------------------------
    
sal_Int16 SAL_CALL Player::getVolumeDB(  ) 
    throw (uno::RuntimeException)
{
	::osl::MutexGuard aGuard(m_aMutex);

    return( static_cast< sal_Int16 >( mnUnmutedVolume / 100 ) );
}

// ------------------------------------------------------------------------------

awt::Size SAL_CALL Player::getPreferredPlayerWindowSize(  )
    throw (uno::RuntimeException)
{
	::osl::MutexGuard aGuard(m_aMutex);

    awt::Size aSize( 0, 0 );
    
    if( mpBV )
    { 
        long nWidth = 0, nHeight = 0;

        mpBV->GetVideoSize( &nWidth, &nHeight );
        aSize.Width = nWidth;
        aSize.Height = nHeight;
    }

    return aSize;
}

// ------------------------------------------------------------------------------

uno::Reference< ::media::XPlayerWindow > SAL_CALL Player::createPlayerWindow( const uno::Sequence< uno::Any >& aArguments )
    throw (uno::RuntimeException)
{
	::osl::MutexGuard aGuard(m_aMutex);

    uno::Reference< ::media::XPlayerWindow >    xRet;
    awt::Size                                   aSize( getPreferredPlayerWindowSize() );

    if( mpVW && aSize.Width > 0 && aSize.Height > 0 )
    {
        ::avmedia::win::Window* pWindow = new ::avmedia::win::Window( mxMgr, *this );

        xRet = pWindow;

        if( !pWindow->create( aArguments ) )
            xRet = uno::Reference< ::media::XPlayerWindow >();
    }

    return xRet;
}

// ------------------------------------------------------------------------------

uno::Reference< media::XFrameGrabber > SAL_CALL Player::createFrameGrabber(  )
    throw (uno::RuntimeException)
{
	::osl::MutexGuard aGuard(m_aMutex);

    uno::Reference< media::XFrameGrabber > xRet;

    if( maURL.getLength() > 0 )
    {
        FrameGrabber* pGrabber = new FrameGrabber( mxMgr );
        
        xRet = pGrabber;
        
        if( !pGrabber->create( maURL ) )
            xRet.clear();
    }
    
    return xRet;
}

// ------------------------------------------------------------------------------

::rtl::OUString SAL_CALL Player::getImplementationName(  )
    throw (uno::RuntimeException)
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( AVMEDIA_WIN_PLAYER_IMPLEMENTATIONNAME ) );
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL Player::supportsService( const ::rtl::OUString& ServiceName )
    throw (uno::RuntimeException)
{
    return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( AVMEDIA_WIN_PLAYER_SERVICENAME ) );
}

// ------------------------------------------------------------------------------

uno::Sequence< ::rtl::OUString > SAL_CALL Player::getSupportedServiceNames(  )
    throw (uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aRet(1);
    aRet[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( AVMEDIA_WIN_PLAYER_SERVICENAME ) );

    return aRet;
}

} // namespace win
} // namespace avmedia
