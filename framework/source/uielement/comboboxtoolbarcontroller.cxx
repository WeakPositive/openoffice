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
#include "precompiled_framework.hxx"

#ifndef __FRAMEWORK_UIELEMENT_COMBOBOXTOOLBARCONTROLLER_HXX
#include "uielement/comboboxtoolbarcontroller.hxx"
#endif

//_________________________________________________________________________________________________________________
//	my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_TOOLBAR_HXX_
#include "uielement/toolbar.hxx"
#endif

//_________________________________________________________________________________________________________________
//	interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/status/ItemStatus.hpp>
#include <com/sun/star/frame/status/ItemState.hpp>
#include <com/sun/star/frame/status/Visibility.hpp>
#include <com/sun/star/frame/XControlNotificationListener.hpp>
#include <com/sun/star/util/Color.hpp>

//_________________________________________________________________________________________________________________
//	other includes
//_________________________________________________________________________________________________________________
#include <svtools/toolboxcontroller.hxx>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#ifndef _VCL_MNEMONIC_HXX_
#include <vcl/mnemonic.hxx>
#endif
#include <vcl/toolbox.hxx>
#include <vcl/combobox.hxx>
#include <tools/urlobj.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::frame::status;
using namespace ::com::sun::star::util;

namespace framework
{

// ------------------------------------------------------------------

// Wrapper class to notify controller about events from combobox.
// Unfortunaltly the events are notifed through virtual methods instead
// of Listeners.

class ComboBoxControl : public ComboBox
{
    public:
        ComboBoxControl( Window* pParent, WinBits nStyle, IComboBoxListener* pComboBoxListener );
        virtual ~ComboBoxControl();

        virtual void Select();
        virtual void DoubleClick();
        virtual void Modify();
        virtual void KeyInput( const ::KeyEvent& rKEvt );
	    virtual void GetFocus();
	    virtual void LoseFocus();
        virtual long PreNotify( NotifyEvent& rNEvt );

    private:
        IComboBoxListener* m_pComboBoxListener;
};

ComboBoxControl::ComboBoxControl( Window* pParent, WinBits nStyle, IComboBoxListener* pComboBoxListener ) :
    ComboBox( pParent, nStyle )
    , m_pComboBoxListener( pComboBoxListener )
{
}

ComboBoxControl::~ComboBoxControl()
{
    m_pComboBoxListener = 0;
}

void ComboBoxControl::Select()
{
    ComboBox::Select();
    if ( m_pComboBoxListener )
        m_pComboBoxListener->Select();
}

void ComboBoxControl::DoubleClick()
{
    ComboBox::DoubleClick();
    if ( m_pComboBoxListener )
        m_pComboBoxListener->DoubleClick();
}

void ComboBoxControl::Modify()
{
    ComboBox::Modify();
    if ( m_pComboBoxListener )
        m_pComboBoxListener->Modify();
}

void ComboBoxControl::KeyInput( const ::KeyEvent& rKEvt )
{
    ComboBox::KeyInput( rKEvt );
    if ( m_pComboBoxListener )
        m_pComboBoxListener->KeyInput( rKEvt );
}

void ComboBoxControl::GetFocus()
{
    ComboBox::GetFocus();
    if ( m_pComboBoxListener )
        m_pComboBoxListener->GetFocus();
}

void ComboBoxControl::LoseFocus()
{
    ComboBox::LoseFocus();
    if ( m_pComboBoxListener )
        m_pComboBoxListener->LoseFocus();
}

long ComboBoxControl::PreNotify( NotifyEvent& rNEvt )
{
    long nRet( 0 );
    if ( m_pComboBoxListener )
        nRet = m_pComboBoxListener->PreNotify( rNEvt );
    if ( nRet == 0 )
        nRet = ComboBox::PreNotify( rNEvt );

    return nRet;
}

// ------------------------------------------------------------------

ComboboxToolbarController::ComboboxToolbarController(
    const Reference< XMultiServiceFactory >& rServiceManager,
    const Reference< XFrame >&               rFrame,
    ToolBox*                                 pToolbar,
    sal_uInt16                                   nID,
    sal_Int32                                nWidth,
    const ::rtl::OUString&                          aCommand ) :
    ComplexToolbarController( rServiceManager, rFrame, pToolbar, nID, aCommand )
    ,   m_pComboBox( 0 )
{
    m_pComboBox = new ComboBoxControl( m_pToolbar, WB_DROPDOWN, this );
    if ( nWidth == 0 )
        nWidth = 100;

    // default dropdown size
    ::Size aLogicalSize( 8, 160 );
    ::Size aPixelSize = m_pComboBox->LogicToPixel( aLogicalSize, MAP_APPFONT );

    m_pComboBox->SetSizePixel( ::Size( nWidth, aPixelSize.Height() ));
    m_pToolbar->SetItemWindow( m_nID, m_pComboBox );
}

// ------------------------------------------------------------------

ComboboxToolbarController::~ComboboxToolbarController()
{
}

// ------------------------------------------------------------------

void SAL_CALL ComboboxToolbarController::dispose()
throw ( RuntimeException )
{
    vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

    m_pToolbar->SetItemWindow( m_nID, 0 );
    delete m_pComboBox;

    ComplexToolbarController::dispose();

    m_pComboBox = 0;
}

// ------------------------------------------------------------------
Sequence<PropertyValue> ComboboxToolbarController::getExecuteArgs(sal_Int16 KeyModifier) const
{
    Sequence<PropertyValue> aArgs( 2 );
    ::rtl::OUString aSelectedText = m_pComboBox->GetText();

    // Add key modifier to argument list
    aArgs[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "KeyModifier" ));
    aArgs[0].Value <<= KeyModifier;
    aArgs[1].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Text" ));
    aArgs[1].Value <<= aSelectedText;
    return aArgs;
}

// ------------------------------------------------------------------

void ComboboxToolbarController::Select()
{
    if ( m_pComboBox->GetEntryCount() > 0 )
    {
        Window::PointerState aState = m_pComboBox->GetPointerState();

        sal_uInt16 nKeyModifier = sal_uInt16( aState.mnState & KEY_MODTYPE );
        execute( nKeyModifier );
    }
}

void ComboboxToolbarController::DoubleClick()
{
}

void ComboboxToolbarController::Modify()
{
    notifyTextChanged( m_pComboBox->GetText() );
}

void ComboboxToolbarController::KeyInput( const ::KeyEvent& )
{
}

void ComboboxToolbarController::GetFocus()
{
    notifyFocusGet();
}

void ComboboxToolbarController::LoseFocus()
{
    notifyFocusLost();
}

long ComboboxToolbarController::PreNotify( NotifyEvent& rNEvt )
{
    switch ( rNEvt.GetType() )
    {
        case EVENT_KEYINPUT :
            {
                const ::KeyEvent* pKeyEvent = rNEvt.GetKeyEvent();
                const KeyCode& rKeyCode = pKeyEvent->GetKeyCode();
                if(( rKeyCode.GetModifier() | rKeyCode.GetCode()) == KEY_RETURN )
                {
                    // Call execute only with non-empty text
                    if ( m_pComboBox->GetText().Len() > 0 )
                        execute( rKeyCode.GetModifier() );
                    return 1;
                }
            }
            break;
        case EVENT_GETFOCUS :
            notifyFocusGet();
            break;
        case EVENT_LOSEFOCUS :
            notifyFocusLost();
            break;
        default :
            break;
    }
    return 0;
}

// --------------------------------------------------------

void ComboboxToolbarController::executeControlCommand( const ::com::sun::star::frame::ControlCommand& rControlCommand )
{
    if ( rControlCommand.Command.equalsAsciiL( "SetText", 7 ))
    {
        for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
        {
            if ( rControlCommand.Arguments[i].Name.equalsAsciiL( "Text", 4 ))
            {
                rtl::OUString aText;
                rControlCommand.Arguments[i].Value >>= aText;
                m_pComboBox->SetText( aText );

                // send notification
                notifyTextChanged( aText );
                break;
            }
        }
    }
    else if ( rControlCommand.Command.equalsAsciiL( "SetList", 7 ))
    {
        for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
        {
            if ( rControlCommand.Arguments[i].Name.equalsAsciiL( "List", 4 ))
            {
                Sequence< ::rtl::OUString > aList;
                m_pComboBox->Clear();

                rControlCommand.Arguments[i].Value >>= aList;
                for ( sal_Int32 j = 0; j < aList.getLength(); j++ )
                    m_pComboBox->InsertEntry( aList[j] );

                // send notification
                uno::Sequence< beans::NamedValue > aInfo( 1 );
                aInfo[0].Name  = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "List" ));
                aInfo[0].Value <<= aList;
                addNotifyInfo( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ListChanged" )),
                               getDispatchFromCommand( m_aCommandURL ),
                               aInfo );

                break;
            }
        }
    }
    else if ( rControlCommand.Command.equalsAsciiL( "AddEntry", 8 ))
    {
        sal_uInt16      nPos( COMBOBOX_APPEND );
        rtl::OUString   aText;
        for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
        {
            if ( rControlCommand.Arguments[i].Name.equalsAsciiL( "Text", 4 ))
            {
                if ( rControlCommand.Arguments[i].Value >>= aText )
                    m_pComboBox->InsertEntry( aText, nPos );
                break;
            }
        }
    }
    else if ( rControlCommand.Command.equalsAsciiL( "InsertEntry", 11 ))
    {
        sal_uInt16      nPos( COMBOBOX_APPEND );
        rtl::OUString   aText;
        for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
        {
            if ( rControlCommand.Arguments[i].Name.equalsAsciiL( "Pos", 3 ))
            {
                sal_Int32 nTmpPos = 0;
                if ( rControlCommand.Arguments[i].Value >>= nTmpPos )
                {
                    if (( nTmpPos >= 0 ) &&
                        ( nTmpPos < sal_Int32( m_pComboBox->GetEntryCount() )))
                        nPos = sal_uInt16( nTmpPos );
                }
            }
            else if ( rControlCommand.Arguments[i].Name.equalsAsciiL( "Text", 4 ))
                rControlCommand.Arguments[i].Value >>= aText;
        }

        m_pComboBox->InsertEntry( aText, nPos );
    }
    else if ( rControlCommand.Command.equalsAsciiL( "RemoveEntryPos", 14 ))
    {
        for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
        {
            if ( rControlCommand.Arguments[i].Name.equalsAsciiL( "Pos", 3 ))
            {
                sal_Int32 nPos( -1 );
                if ( rControlCommand.Arguments[i].Value >>= nPos )
                {
                    if ( nPos < sal_Int32( m_pComboBox->GetEntryCount() ))
                        m_pComboBox->RemoveEntry( sal_uInt16( nPos ));
                }
                break;
            }
        }
    }
    else if ( rControlCommand.Command.equalsAsciiL( "RemoveEntryText", 15 ))
    {
        for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
        {
            if ( rControlCommand.Arguments[i].Name.equalsAsciiL( "Text", 4 ))
            {
                rtl::OUString aText;
                if ( rControlCommand.Arguments[i].Value >>= aText )
                    m_pComboBox->RemoveEntry( aText );
                break;
            }
        }
    }
    else if ( rControlCommand.Command.equalsAsciiL( "SetDropDownLines", 16 ))
    {
        for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
        {
            if ( rControlCommand.Arguments[i].Name.equalsAsciiL( "Lines", 5 ))
            {
                sal_Int32 nValue( 5 );
                rControlCommand.Arguments[i].Value >>= nValue;
                m_pComboBox->SetDropDownLineCount( sal_uInt16( nValue ));
                break;
            }
        }
    }
    else if ( rControlCommand.Command.equalsAsciiL( "SetBackgroundColor", 18 ))
    {
        for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
        {
            if ( rControlCommand.Arguments[i].Name.equalsAsciiL( "Color", 5 ))
            {
                com::sun::star::util::Color aColor(0);
                if ( rControlCommand.Arguments[i].Value >>= aColor )
                {
                    ::Color aBackColor( static_cast< sal_uInt32 >( aColor ));
                    m_pComboBox->SetControlBackground( aBackColor );
                }
                break;
            }
        }
    }
    else if ( rControlCommand.Command.equalsAsciiL( "SetTextColor", 12 ))
    {
        for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
        {
            if ( rControlCommand.Arguments[i].Name.equalsAsciiL( "Color", 5 ))
            {
                com::sun::star::util::Color aColor(0);
                if ( rControlCommand.Arguments[i].Value >>= aColor )
                {
                    ::Color aForeColor( static_cast< sal_uInt32 >( aColor ));
                    m_pComboBox->SetControlForeground( aForeColor );
                }
                break;
            }
        }
    }
}

} // namespace

