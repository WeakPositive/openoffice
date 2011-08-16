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

//////////////////////////////////////////////////////////////////////////
// includes
//////////////////////////////////////////////////////////////////////////

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <cppuhelper/interfacecontainer.h>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ControlActions.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>

#include <svtools/svtools.hrc>

#include <vos/mutex.hxx>

#include <vcl/svapp.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/syswin.hxx>

#include "osl/file.h"

#include "KDE4FilePicker.hxx"
#include "FPServiceInfo.hxx"

/* ********* Hack, but needed because of conflicting types... */
#define Region QtXRegion

//kde has an enum that uses this...OO does too
#undef SETTINGS_MOUSE

#include <kfiledialog.h>
#include <kwindowsystem.h>
#include <kapplication.h>
#include <kfilefiltercombo.h>

#include <QWidget>
#include <QCheckBox>
#include <QGridLayout>

#undef Region

using namespace ::com::sun::star;

using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::ui::dialogs::TemplateDescription;

using namespace ::com::sun::star;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::ui::dialogs::TemplateDescription;
using namespace ::com::sun::star::ui::dialogs::ExtendedFilePickerElementIds;
using namespace ::com::sun::star::ui::dialogs::CommonFilePickerElementIds;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;

//////////////////////////////////////////////////////////////////////////
// helper functions
//////////////////////////////////////////////////////////////////////////

#include <QDebug>

namespace
{
    // controling event notifications    
    const bool STARTUP_SUSPENDED = true;
    const bool STARTUP_ALIVE     = false;

    uno::Sequence<rtl::OUString> SAL_CALL FilePicker_getSupportedServiceNames()
    {
        uno::Sequence<rtl::OUString> aRet(3);
        aRet[0] = rtl::OUString::createFromAscii("com.sun.star.ui.dialogs.FilePicker");
        aRet[1] = rtl::OUString::createFromAscii("com.sun.star.ui.dialogs.SystemFilePicker");
        aRet[2] = rtl::OUString::createFromAscii("com.sun.star.ui.dialogs.KDE4FilePicker");
        return aRet;
    }
}

rtl::OUString toOUString(const QString& s)
{
    // QString stores UTF16, just like OUString
	return rtl::OUString(reinterpret_cast<const sal_Unicode*>(s.data()), s.length());
}

QString toQString(const rtl::OUString& s)
{
	return QString::fromUtf16(s.getStr(), s.getLength());
}

//////////////////////////////////////////////////////////////////////////
// KDE4FilePicker
//////////////////////////////////////////////////////////////////////////

KDE4FilePicker::KDE4FilePicker( const uno::Reference<lang::XMultiServiceFactory>& xServiceMgr )
    : cppu::WeakComponentImplHelper8< 		
          XFilterManager, 
          XFilterGroupManager,
          XFilePickerControlAccess,
          XFilePickerNotifier,
// TODO   XFilePreview,
          lang::XInitialization,
          util::XCancellable,
          lang::XEventListener, 
          lang::XServiceInfo>( _helperMutex ),
          m_xServiceMgr( xServiceMgr ),
		  _resMgr( CREATEVERSIONRESMGR( fps_office ) )
{
	_extraControls = new QWidget();
	_layout = new QGridLayout(_extraControls);
	
	_dialog = new KFileDialog(KUrl("~"), QString(""), 0, _extraControls);
	_dialog->setMode(KFile::File | KFile::LocalOnly);
	
	//default mode
	_dialog->setOperationMode(KFileDialog::Opening);
}

KDE4FilePicker::~KDE4FilePicker()
{
	delete _resMgr;
	delete _dialog;
}

void SAL_CALL KDE4FilePicker::addFilePickerListener( const uno::Reference<XFilePickerListener>& xListener )
	throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    m_xListener = xListener;
}

void SAL_CALL KDE4FilePicker::removeFilePickerListener( const uno::Reference<XFilePickerListener>& )
	throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    m_xListener.clear();
}

void SAL_CALL KDE4FilePicker::setTitle( const rtl::OUString &title )
    throw( uno::RuntimeException )
{
	_dialog->setCaption(toQString(title));
}

sal_Int16 SAL_CALL KDE4FilePicker::execute()
    throw( uno::RuntimeException )
{
	//get the window id of the main OO window to set it for the dialog as a parent
	Window *pParentWin = Application::GetDefDialogParent();
	if ( pParentWin )
	{
		const SystemEnvData* pSysData = ((SystemWindow *)pParentWin)->GetSystemData();
		if ( pSysData )
		{
			KWindowSystem::setMainWindow( _dialog, pSysData->aWindow); // unx only
		}
	}
	
	_dialog->clearFilter();
	_dialog->setFilter(_filter);
    _dialog->filterWidget()->setEditable(false);
	
	//block and wait for user input
	if (_dialog->exec() == KFileDialog::Accepted)
		return ExecutableDialogResults::OK;
	
	return ExecutableDialogResults::CANCEL;
}

void SAL_CALL KDE4FilePicker::setMultiSelectionMode( sal_Bool multiSelect )
    throw( uno::RuntimeException )
{
	if (multiSelect)
		_dialog->setMode(KFile::Files | KFile::LocalOnly);
	else
		_dialog->setMode(KFile::File | KFile::LocalOnly);
}

void SAL_CALL KDE4FilePicker::setDefaultName( const ::rtl::OUString &name )
    throw( uno::RuntimeException )
{
	const QString url = toQString(name);
	_dialog->setSelection(url);
}

void SAL_CALL KDE4FilePicker::setDisplayDirectory( const rtl::OUString &dir )
    throw( uno::RuntimeException )
{
	const QString url = toQString(dir);
	_dialog->setUrl(KUrl(url));
}

rtl::OUString SAL_CALL KDE4FilePicker::getDisplayDirectory()
    throw( uno::RuntimeException )
{
	QString dir = _dialog->baseUrl().url();
	return toOUString(dir);
}

uno::Sequence< ::rtl::OUString > SAL_CALL KDE4FilePicker::getFiles()
    throw( uno::RuntimeException )
{
	QStringList rawFiles = _dialog->selectedFiles();
	QStringList files;
	
	// check if we need to add an extension
	QString extension = "";
	if ( _dialog->operationMode() == KFileDialog::Saving )
	{
		QCheckBox *cb = dynamic_cast<QCheckBox*> (
			_customWidgets[ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION ]);
			
		if (cb && cb->isChecked())
		{
			extension = _dialog->currentFilter(); // assuming filter value is like this *.ext
			extension.replace("*","");
		}
	}
	
	// Workaround for the double click selection KDE4 bug 
	// kde file picker returns the file and directories for selectedFiles()
	// when a file is double clicked
	// make a true list of files
	const QString dir = KUrl(rawFiles[0]).directory();
	
	bool singleFile = true;
	if (rawFiles.size() > 1)
	{
		singleFile = false;
		//for multi file sequences, oo expects the first param to be the directory
		//can't treat all cases like multi file because in some instances (inserting image)
		//oo WANTS only one entry in the final list
		files.append(dir);
	}
	
	for (sal_uInt16 i = 0; i < rawFiles.size(); ++i)
	{
		// if the raw file is not the base directory (see above kde bug)
		// we add the file to list of avail files
		if ((dir + "/") != ( rawFiles[i]))
		{
			QString filename = KUrl(rawFiles[i]).fileName();
			
			if (singleFile)
				filename.prepend(dir + "/");
			
			//prevent extension append if we already have one
			if (filename.endsWith(extension))
				files.append(filename);
			else
				files.append(filename + extension);
		}
	}
	
	// add all files and leading directory to outgoing OO sequence
	uno::Sequence< ::rtl::OUString > seq(files.size());
	for (int i = 0; i < files.size(); ++i)
    {
        rtl::OUString aFile(toOUString(files[i])), aURL;
        osl_getFileURLFromSystemPath(aFile.pData, &aURL.pData );
		seq[i] = aURL;
    }
	
	return seq;
}

void SAL_CALL KDE4FilePicker::appendFilter( const ::rtl::OUString &title, const ::rtl::OUString &filter )
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
	QString t = toQString(title);
	QString f = toQString(filter);
	
	if (!_filter.isNull())
		_filter.append("\n");
	
	//add to hash map for reverse lookup in getCurrentFilter
	_filters.insert(f, t);
	
	// '/' meed to be escaped to else they are assumed to be mime types by kfiledialog
	//see the docs
	t.replace("/", "\\/");
	
	// openoffice gives us filters separated by ';' qt dialogs just want space separated
	f.replace(";", " ");
	
	_filter.append(QString("%1|%2").arg(f).arg(t));
}

void SAL_CALL KDE4FilePicker::setCurrentFilter( const rtl::OUString &title )
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
	QString t = toQString(title);
	t.replace("/", "\\/");
	_dialog->filterWidget()->setCurrentFilter(t);
}

rtl::OUString SAL_CALL KDE4FilePicker::getCurrentFilter()
    throw( uno::RuntimeException )
{
	QString filter = _filters[_dialog->currentFilter()];
	
	//default if not found
	if (filter.isNull())
		filter = "ODF Text Document (.odt)";
	
	return toOUString(filter);
}

void SAL_CALL KDE4FilePicker::appendFilterGroup( const rtl::OUString& , const uno::Sequence<beans::StringPair>& filters)
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
	if (!_filter.isNull())
		_filter.append(QString("\n"));
	
	const sal_uInt16 length = filters.getLength();
	for (sal_uInt16 i = 0; i < length; ++i)
	{
		beans::StringPair aPair = filters[i];
		
		_filter.append(QString("%1|%2").arg(
			toQString(aPair.Second).replace(";", " ")).arg(
			toQString(aPair.First).replace("/","\\/")));
			
		if (i != length - 1)
			_filter.append('\n');
	}
}

void SAL_CALL KDE4FilePicker::setValue( sal_Int16 controlId, sal_Int16, const uno::Any &value )
    throw( uno::RuntimeException )
{
	QWidget* widget = _customWidgets[controlId];
	
	if (widget)
	{
		switch (controlId)
		{
			case ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION:
			case ExtendedFilePickerElementIds::CHECKBOX_PASSWORD:
			case ExtendedFilePickerElementIds::CHECKBOX_FILTEROPTIONS:
			case ExtendedFilePickerElementIds::CHECKBOX_READONLY:
			case ExtendedFilePickerElementIds::CHECKBOX_LINK:
			case ExtendedFilePickerElementIds::CHECKBOX_PREVIEW:
			case ExtendedFilePickerElementIds::CHECKBOX_SELECTION:
			{
				QCheckBox* cb = dynamic_cast<QCheckBox*>(widget);
				cb->setChecked(value.getValue());
				break;
			}
			case ExtendedFilePickerElementIds::PUSHBUTTON_PLAY:
			case ExtendedFilePickerElementIds::LISTBOX_VERSION:
			case ExtendedFilePickerElementIds::LISTBOX_TEMPLATE:
			case ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE:
			case ExtendedFilePickerElementIds::LISTBOX_VERSION_LABEL:
			case ExtendedFilePickerElementIds::LISTBOX_TEMPLATE_LABEL:
			case ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE_LABEL:
			case ExtendedFilePickerElementIds::LISTBOX_FILTER_SELECTOR:
				break;
		}
	}
}

uno::Any SAL_CALL KDE4FilePicker::getValue( sal_Int16 controlId, sal_Int16 )
    throw( uno::RuntimeException )
{
	uno::Any res(false);
	
	QWidget* widget = _customWidgets[controlId];
	
	if (widget)
	{
		switch (controlId)
		{
			case ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION:
			case ExtendedFilePickerElementIds::CHECKBOX_PASSWORD:
			case ExtendedFilePickerElementIds::CHECKBOX_FILTEROPTIONS:
			case ExtendedFilePickerElementIds::CHECKBOX_READONLY:
			case ExtendedFilePickerElementIds::CHECKBOX_LINK:
			case ExtendedFilePickerElementIds::CHECKBOX_PREVIEW:
			case ExtendedFilePickerElementIds::CHECKBOX_SELECTION:
			{
				QCheckBox* cb = dynamic_cast<QCheckBox*>(widget);
				res = uno::Any(cb->isChecked());
				break;
			}
			case ExtendedFilePickerElementIds::PUSHBUTTON_PLAY:
			case ExtendedFilePickerElementIds::LISTBOX_VERSION:
			case ExtendedFilePickerElementIds::LISTBOX_TEMPLATE:
			case ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE:
			case ExtendedFilePickerElementIds::LISTBOX_VERSION_LABEL:
			case ExtendedFilePickerElementIds::LISTBOX_TEMPLATE_LABEL:
			case ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE_LABEL:
			case ExtendedFilePickerElementIds::LISTBOX_FILTER_SELECTOR:
				break;
		}
	}
	
	return res;
}

void SAL_CALL KDE4FilePicker::enableControl( sal_Int16 controlId, sal_Bool enable )
    throw( uno::RuntimeException )
{
	QWidget* widget = _customWidgets[controlId];
	
	if (widget)
	{
		widget->setEnabled(enable);
	}
}

void SAL_CALL KDE4FilePicker::setLabel( sal_Int16 controlId, const ::rtl::OUString &label )
    throw( uno::RuntimeException )
{
	QWidget* widget = _customWidgets[controlId];
	
	if (widget)
	{
		switch (controlId)
		{
			case ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION:
			case ExtendedFilePickerElementIds::CHECKBOX_PASSWORD:
			case ExtendedFilePickerElementIds::CHECKBOX_FILTEROPTIONS:
			case ExtendedFilePickerElementIds::CHECKBOX_READONLY:
			case ExtendedFilePickerElementIds::CHECKBOX_LINK:
			case ExtendedFilePickerElementIds::CHECKBOX_PREVIEW:
			case ExtendedFilePickerElementIds::CHECKBOX_SELECTION:
			{
				QCheckBox* cb = dynamic_cast<QCheckBox*>(widget);
				cb->setText(toQString(label));
				break;
			}
			case ExtendedFilePickerElementIds::PUSHBUTTON_PLAY:
			case ExtendedFilePickerElementIds::LISTBOX_VERSION:
			case ExtendedFilePickerElementIds::LISTBOX_TEMPLATE:
			case ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE:
			case ExtendedFilePickerElementIds::LISTBOX_VERSION_LABEL:
			case ExtendedFilePickerElementIds::LISTBOX_TEMPLATE_LABEL:
			case ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE_LABEL:
			case ExtendedFilePickerElementIds::LISTBOX_FILTER_SELECTOR:
				break;
		}
	}
}

rtl::OUString SAL_CALL KDE4FilePicker::getLabel(sal_Int16 controlId) 
    throw ( uno::RuntimeException )
{
	QWidget* widget = _customWidgets[controlId];
	QString label;
	
	if (widget)
	{
		switch (controlId)
		{
			case ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION:
			case ExtendedFilePickerElementIds::CHECKBOX_PASSWORD:
			case ExtendedFilePickerElementIds::CHECKBOX_FILTEROPTIONS:
			case ExtendedFilePickerElementIds::CHECKBOX_READONLY:
			case ExtendedFilePickerElementIds::CHECKBOX_LINK:
			case ExtendedFilePickerElementIds::CHECKBOX_PREVIEW:
			case ExtendedFilePickerElementIds::CHECKBOX_SELECTION:
			{
				QCheckBox* cb = dynamic_cast<QCheckBox*>(widget);
				label = cb->text();
				break;
			}
			case ExtendedFilePickerElementIds::PUSHBUTTON_PLAY:
			case ExtendedFilePickerElementIds::LISTBOX_VERSION:
			case ExtendedFilePickerElementIds::LISTBOX_TEMPLATE:
			case ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE:
			case ExtendedFilePickerElementIds::LISTBOX_VERSION_LABEL:
			case ExtendedFilePickerElementIds::LISTBOX_TEMPLATE_LABEL:
			case ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE_LABEL:
			case ExtendedFilePickerElementIds::LISTBOX_FILTER_SELECTOR:
				break;
		}
	}
	return toOUString(label);
}

void KDE4FilePicker::addCustomControl(sal_Int16 controlId)
{
	QWidget* widget = 0;
	sal_Int32 resId = -1;
	
	switch (controlId)
	{
		case ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION:
			resId = STR_SVT_FILEPICKER_AUTO_EXTENSION;
			break;
		case ExtendedFilePickerElementIds::CHECKBOX_PASSWORD:
			resId = STR_SVT_FILEPICKER_PASSWORD;
			break;
		case ExtendedFilePickerElementIds::CHECKBOX_FILTEROPTIONS:
			resId = STR_SVT_FILEPICKER_FILTER_OPTIONS;
			break;
		case ExtendedFilePickerElementIds::CHECKBOX_READONLY:
			resId = STR_SVT_FILEPICKER_READONLY;
			break;
		case ExtendedFilePickerElementIds::CHECKBOX_LINK:
			resId = STR_SVT_FILEPICKER_INSERT_AS_LINK;
			break;
		case ExtendedFilePickerElementIds::CHECKBOX_PREVIEW:
			resId = STR_SVT_FILEPICKER_SHOW_PREVIEW;
			break;
		case ExtendedFilePickerElementIds::CHECKBOX_SELECTION:
			resId = STR_SVT_FILEPICKER_SELECTION;
			break;
		case ExtendedFilePickerElementIds::PUSHBUTTON_PLAY:
			resId = STR_SVT_FILEPICKER_PLAY;
			break;
		case ExtendedFilePickerElementIds::LISTBOX_VERSION:
			resId = STR_SVT_FILEPICKER_VERSION;
			break;
		case ExtendedFilePickerElementIds::LISTBOX_TEMPLATE:
			resId = STR_SVT_FILEPICKER_TEMPLATES;
			break;
		case ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE:
			resId = STR_SVT_FILEPICKER_IMAGE_TEMPLATE;
			break;
		case ExtendedFilePickerElementIds::LISTBOX_VERSION_LABEL:
		case ExtendedFilePickerElementIds::LISTBOX_TEMPLATE_LABEL:
		case ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE_LABEL:
		case ExtendedFilePickerElementIds::LISTBOX_FILTER_SELECTOR:
			break;
	}
	
	switch (controlId)
	{
		case ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION:
		case ExtendedFilePickerElementIds::CHECKBOX_PASSWORD:
		case ExtendedFilePickerElementIds::CHECKBOX_FILTEROPTIONS:
		case ExtendedFilePickerElementIds::CHECKBOX_READONLY:
		case ExtendedFilePickerElementIds::CHECKBOX_LINK:
		case ExtendedFilePickerElementIds::CHECKBOX_PREVIEW:
		case ExtendedFilePickerElementIds::CHECKBOX_SELECTION:
		{
			QString label;
			
			if (_resMgr && resId != -1)
			{
				rtl::OUString s = String(ResId( resId, *_resMgr ));
				label = toQString(s);
				label.replace("~", "&");
			}
			
			widget = new QCheckBox(label, _extraControls);
			
			break;
		}
		case ExtendedFilePickerElementIds::PUSHBUTTON_PLAY:
		case ExtendedFilePickerElementIds::LISTBOX_VERSION:
		case ExtendedFilePickerElementIds::LISTBOX_TEMPLATE:
		case ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE:
		case ExtendedFilePickerElementIds::LISTBOX_VERSION_LABEL:
		case ExtendedFilePickerElementIds::LISTBOX_TEMPLATE_LABEL:
		case ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE_LABEL:
		case ExtendedFilePickerElementIds::LISTBOX_FILTER_SELECTOR:
			break;
	}
	
	if (widget)
	{
		_layout->addWidget(widget);
		_customWidgets.insert(controlId, widget);
	}
}

void SAL_CALL KDE4FilePicker::initialize( const uno::Sequence<uno::Any> &args ) 
    throw( uno::Exception, uno::RuntimeException )
{	
	_filter.clear();
	_filters.clear();
	
    // parameter checking	    
    uno::Any arg;
    if (args.getLength() == 0)
	{
        throw lang::IllegalArgumentException(
                rtl::OUString::createFromAscii( "no arguments" ),
                static_cast< XFilePicker* >( this ), 1 );
	}

    arg = args[0];

    if (( arg.getValueType() != ::getCppuType((sal_Int16*)0)) && 
		( arg.getValueType() != ::getCppuType((sal_Int8*)0)))
	{
        throw lang::IllegalArgumentException(
                rtl::OUString::createFromAscii( "invalid argument type" ),
                static_cast< XFilePicker* >( this ), 1 );
	}

    sal_Int16 templateId = -1;
    arg >>= templateId;
	
	//default is opening
	KFileDialog::OperationMode operationMode = KFileDialog::Opening;
	
    switch ( templateId )
    {
        case FILEOPEN_SIMPLE:
            break;
			
        case FILESAVE_SIMPLE:
            operationMode = KFileDialog::Saving;
            break;
			
        case FILESAVE_AUTOEXTENSION:
            operationMode = KFileDialog::Saving;
			//addCustomControl( ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION );
            break;

        case FILESAVE_AUTOEXTENSION_PASSWORD:
		{
            operationMode = KFileDialog::Saving;
			//addCustomControl( ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION );
            addCustomControl( ExtendedFilePickerElementIds::CHECKBOX_PASSWORD );
            break;
		}
        case FILESAVE_AUTOEXTENSION_PASSWORD_FILTEROPTIONS:
		{
			operationMode = KFileDialog::Saving;
			addCustomControl( ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION );
            addCustomControl( ExtendedFilePickerElementIds::CHECKBOX_PASSWORD );
            addCustomControl( ExtendedFilePickerElementIds::CHECKBOX_FILTEROPTIONS );
            break;
		}
        case FILESAVE_AUTOEXTENSION_SELECTION:
            operationMode = KFileDialog::Saving;
            addCustomControl( ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION );
            addCustomControl( ExtendedFilePickerElementIds::CHECKBOX_SELECTION );
            break;

        case FILESAVE_AUTOEXTENSION_TEMPLATE:
            operationMode = KFileDialog::Saving;
            addCustomControl( ExtendedFilePickerElementIds::LISTBOX_TEMPLATE );
            break;

        case FILEOPEN_LINK_PREVIEW_IMAGE_TEMPLATE:
            addCustomControl( ExtendedFilePickerElementIds::CHECKBOX_LINK );
            addCustomControl( ExtendedFilePickerElementIds::CHECKBOX_PREVIEW );
            addCustomControl( ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE );
            break;

        case FILEOPEN_PLAY:        
            addCustomControl( ExtendedFilePickerElementIds::PUSHBUTTON_PLAY );
            break;

        case FILEOPEN_READONLY_VERSION:
            addCustomControl( ExtendedFilePickerElementIds::CHECKBOX_READONLY );
            addCustomControl( ExtendedFilePickerElementIds::LISTBOX_VERSION );
            break;

        case FILEOPEN_LINK_PREVIEW:
            addCustomControl( ExtendedFilePickerElementIds::CHECKBOX_LINK );
            addCustomControl( ExtendedFilePickerElementIds::CHECKBOX_PREVIEW );
            break;

        default:
            throw lang::IllegalArgumentException(
                    rtl::OUString::createFromAscii( "Unknown template" ),
                    static_cast< XFilePicker* >( this ),
                    1 );
    }
	
	_dialog->setOperationMode(operationMode);
    _dialog->setConfirmOverwrite(true);
}

void SAL_CALL KDE4FilePicker::cancel()
    throw ( uno::RuntimeException )
{
	
}

void SAL_CALL KDE4FilePicker::disposing( const lang::EventObject &rEvent )
    throw( uno::RuntimeException )
{
    uno::Reference<XFilePickerListener> xFilePickerListener( rEvent.Source, uno::UNO_QUERY );

    if ( xFilePickerListener.is() )
	{
        removeFilePickerListener( xFilePickerListener );
	}
}

rtl::OUString SAL_CALL KDE4FilePicker::getImplementationName() 
    throw( uno::RuntimeException )
{
    return rtl::OUString::createFromAscii( FILE_PICKER_IMPL_NAME );
}

sal_Bool SAL_CALL KDE4FilePicker::supportsService( const rtl::OUString& ServiceName ) 
    throw( uno::RuntimeException )
{
    uno::Sequence< ::rtl::OUString > SupportedServicesNames = FilePicker_getSupportedServiceNames();

    for ( sal_Int32 n = SupportedServicesNames.getLength(); n--; )
    {
        if ( SupportedServicesNames[n].compareTo( ServiceName ) == 0 )
            return sal_True;
    }

    return sal_False;
}

uno::Sequence< ::rtl::OUString > SAL_CALL KDE4FilePicker::getSupportedServiceNames() 
    throw( uno::RuntimeException )
{
    return FilePicker_getSupportedServiceNames();
}
