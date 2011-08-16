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
#include "precompiled_shell.hxx"
#include "internal/config.hxx"
#include "internal/global.hxx"

#ifndef PROPSEETS_HXX_INCLUDED
#include "internal/propsheets.hxx"
#endif
#include "internal/utilities.hxx"
#include "internal/resource.h"
#include "listviewbuilder.hxx"

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <shellapi.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif 

#include <string>
#include <vector>
#include <utility>
#include <strsafe.h>


/*---------------------------------------------
	INFO - INFO - INFO - INFO - INFO - INFO

	See MSDN "Using Windows XP Visual Styles" 
	for hints how to enable the new common
	control library for our property sheet.

	INFO - INFO - INFO - INFO - INFO - INFO
----------------------------------------------*/

//-----------------------------
//
//-----------------------------

CPropertySheet::CPropertySheet(long RefCnt) : 
	m_RefCnt(RefCnt)
{
    OutputDebugStringFormat("CPropertySheet::CTor [%d], [%d]", m_RefCnt, g_DllRefCnt );
	InterlockedIncrement(&g_DllRefCnt);
}

//-----------------------------
//
//-----------------------------

CPropertySheet::~CPropertySheet()
{
    OutputDebugStringFormat("CPropertySheet::DTor [%d], [%d]", m_RefCnt, g_DllRefCnt );
	InterlockedDecrement(&g_DllRefCnt);
}

//-----------------------------
// IUnknown methods
//-----------------------------

HRESULT STDMETHODCALLTYPE CPropertySheet::QueryInterface(
	REFIID riid, void __RPC_FAR *__RPC_FAR *ppvObject)
{
	*ppvObject = 0;
	
	IUnknown* pUnk = 0;
	if (IID_IUnknown == riid || IID_IShellExtInit == riid)
	{
		pUnk = static_cast<IShellExtInit*>(this);
		pUnk->AddRef();
		*ppvObject = pUnk;
		return S_OK;
	}
	else if (IID_IShellPropSheetExt == riid)
	{
		pUnk = static_cast<IShellPropSheetExt*>(this);
		pUnk->AddRef();
		*ppvObject = pUnk;
		return S_OK;
	}

	return E_NOINTERFACE;
}
      
//-----------------------------
//
//-----------------------------
  
ULONG STDMETHODCALLTYPE CPropertySheet::AddRef(void)
{
    OutputDebugStringFormat("CPropertySheet::AddRef [%d]", m_RefCnt );
	return InterlockedIncrement(&m_RefCnt);
}

//-----------------------------
//
//-----------------------------
        
ULONG STDMETHODCALLTYPE CPropertySheet::Release(void)
{
    OutputDebugStringFormat("CPropertySheet::Release [%d]", m_RefCnt );
	long refcnt = InterlockedDecrement(&m_RefCnt);

	if (0 == refcnt)
		delete this;

	return refcnt;
}

//-----------------------------
// IShellExtInit
//-----------------------------
	
HRESULT STDMETHODCALLTYPE CPropertySheet::Initialize(
	LPCITEMIDLIST /*pidlFolder*/, LPDATAOBJECT lpdobj, HKEY /*hkeyProgID*/)
{	
	InitCommonControls();

	STGMEDIUM medium;
	FORMATETC fe = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
   
	HRESULT hr = lpdobj->GetData(&fe, &medium);
   
	// save the file name
    if (SUCCEEDED(hr) && 
		(1 == DragQueryFileA(
			reinterpret_cast<HDROP>(medium.hGlobal), 
			0xFFFFFFFF, 
			NULL, 
			0))) 
	{
	    UINT size = DragQueryFile( reinterpret_cast<HDROP>(medium.hGlobal), 0, 0, 0 );
	    if ( size != 0 )
	    {
	        TCHAR * buffer = new TCHAR[ size + 1 ];
	        UINT result_size = DragQueryFile( reinterpret_cast<HDROP>(medium.hGlobal),
	                                          0, buffer, size + 1 );
	        if ( result_size != 0 )
	        {
	            std::wstring fname = getShortPathName( buffer );
	            std::string fnameA = WStringToString( fname );
	            ZeroMemory( m_szFileName, sizeof( m_szFileName ) );
	            strncpy( m_szFileName, fnameA.c_str(), ( sizeof( m_szFileName ) - 1 ) );
	            hr = S_OK;
	        }
	        else
	            hr = E_INVALIDARG;
            delete [] buffer;
	    }
	    else
	        hr = E_INVALIDARG;
    }
    else
        hr = E_INVALIDARG;

	ReleaseStgMedium(&medium);

    return hr;	
}

//-----------------------------
// IShellPropSheetExt
//-----------------------------

HRESULT STDMETHODCALLTYPE CPropertySheet::AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam)
{
    // Get OS version (we don't need the summary page on Windows Vista or later)
    OSVERSIONINFO sInfoOS;

    ZeroMemory( &sInfoOS, sizeof(OSVERSIONINFO) );
    sInfoOS.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
    GetVersionEx( &sInfoOS );
    bool bIsVistaOrLater = (sInfoOS.dwMajorVersion >= 6);

	std::wstring proppage_header;

	PROPSHEETPAGE psp;
	ZeroMemory(&psp, sizeof(PROPSHEETPAGEA));

	// add the summary property page
    psp.dwSize      = sizeof(PROPSHEETPAGE);
    psp.dwFlags     = PSP_DEFAULT | PSP_USETITLE | PSP_USECALLBACK;
    psp.hInstance   = GetModuleHandle(MODULE_NAME);
    psp.lParam      = reinterpret_cast<LPARAM>(this);
	psp.pfnCallback = reinterpret_cast<LPFNPSPCALLBACK>(CPropertySheet::PropPageSummaryCallback);

    HPROPSHEETPAGE hPage = NULL;

	if ( !bIsVistaOrLater )
	{
	    proppage_header = GetResString(IDS_PROPPAGE_SUMMARY_TITLE);

        psp.pszTemplate = MAKEINTRESOURCE(IDD_PROPPAGE_SUMMARY);
        psp.pszTitle    = proppage_header.c_str();
        psp.pfnDlgProc  = reinterpret_cast<DLGPROC>(CPropertySheet::PropPageSummaryProc);	

        hPage = CreatePropertySheetPage(&psp);

        // keep this instance alive, will be released when the
        // the page is about to be destroyed in the callback function

        if (hPage)
        {
            if (lpfnAddPage(hPage, lParam))
                AddRef();
            else
                DestroyPropertySheetPage(hPage);
        }
	}

	// add the statistics property page
	proppage_header = GetResString(IDS_PROPPAGE_STATISTICS_TITLE);

    psp.pszTemplate = MAKEINTRESOURCE(IDD_PROPPAGE_STATISTICS);
    psp.pszTitle    = proppage_header.c_str();
    psp.pfnDlgProc  = reinterpret_cast<DLGPROC>(CPropertySheet::PropPageStatisticsProc);	

    hPage = CreatePropertySheetPage(&psp);
    
    if (hPage)		
	{
		if (lpfnAddPage(hPage, lParam))		
			AddRef();
		else
			DestroyPropertySheetPage(hPage);
	}

	// always return success else
	// no property sheet will be 
	// displayed at all
	return NOERROR;
}

//-----------------------------
//
//-----------------------------

HRESULT STDMETHODCALLTYPE CPropertySheet::ReplacePage(
	UINT /*uPageID*/, LPFNADDPROPSHEETPAGE /*lpfnReplaceWith*/, LPARAM /*lParam*/)
{
	return E_NOTIMPL;
}

//-----------------------------
//
//-----------------------------

UINT CALLBACK CPropertySheet::PropPageSummaryCallback(
	HWND /*hwnd*/, UINT uMsg, LPPROPSHEETPAGE ppsp)
{
	CPropertySheet* pImpl = 
		reinterpret_cast<CPropertySheet*>(ppsp->lParam);

	// release this instance, acquired 
	// in the AddPages method
	if (PSPCB_RELEASE == uMsg)
	{
		pImpl->Release();
	}

	return TRUE;
}


//-----------------------------
//
//-----------------------------

BOOL CALLBACK CPropertySheet::PropPageSummaryProc(HWND hwnd, UINT uiMsg, WPARAM /*wParam*/, LPARAM lParam)
{
	switch (uiMsg)
	{
	case WM_INITDIALOG:		
		{
			LPPROPSHEETPAGE psp = reinterpret_cast<LPPROPSHEETPAGE>(lParam);
			CPropertySheet* pImpl = reinterpret_cast<CPropertySheet*>(psp->lParam);
			pImpl->InitPropPageSummary(hwnd, psp);
			return TRUE;			
		}
	}

	return FALSE;
}

//-----------------------------
//
//-----------------------------

BOOL CALLBACK CPropertySheet::PropPageStatisticsProc(HWND hwnd, UINT uiMsg, WPARAM /*wParam*/, LPARAM lParam)
{
	switch (uiMsg)
	{
	case WM_INITDIALOG:	
		{
			LPPROPSHEETPAGE psp = reinterpret_cast<LPPROPSHEETPAGE>(lParam);
			CPropertySheet* pImpl = reinterpret_cast<CPropertySheet*>(psp->lParam);
			pImpl->InitPropPageStatistics(hwnd, psp);
			return TRUE;			
		}
	}

	return FALSE;
}

//##################################
void CPropertySheet::InitPropPageSummary(HWND hwnd, LPPROPSHEETPAGE /*lppsp*/)
{
    try
    {
        CMetaInfoReader metaInfo(m_szFileName);

        SetWindowText(GetDlgItem(hwnd,IDC_TITLE),    metaInfo.getTagData( META_INFO_TITLE ).c_str() );
        SetWindowText(GetDlgItem(hwnd,IDC_AUTHOR),   metaInfo.getTagData( META_INFO_AUTHOR ).c_str() );
        SetWindowText(GetDlgItem(hwnd,IDC_SUBJECT),  metaInfo.getTagData( META_INFO_SUBJECT ).c_str() );
        SetWindowText(GetDlgItem(hwnd,IDC_KEYWORDS), metaInfo.getTagData( META_INFO_KEYWORDS ).c_str() );

        // comments read from meta.xml use "\n" for return, but this will not displayable in Edit control, add
        // "\r" before "\n" to form "\r\n" in order to display return in Edit control.
        std::wstring tempStr = metaInfo.getTagData( META_INFO_DESCRIPTION ).c_str();
        std::wstring::size_type itor = tempStr.find ( L"\n" , 0 );
        while (itor != std::wstring::npos)
        {
            tempStr.insert(itor, L"\r");
            itor = tempStr.find(L"\n", itor + 2);
        }
        SetWindowText(GetDlgItem(hwnd,IDC_COMMENTS), tempStr.c_str());
    }
    catch (const std::exception&)
    {
    }
}

//---------------------------------
/**
*/
void CPropertySheet::InitPropPageStatistics(HWND hwnd, LPPROPSHEETPAGE /*lppsp*/)
{
    try
    {
        CMetaInfoReader metaInfo(m_szFileName);

        document_statistic_reader_ptr doc_stat_reader = create_document_statistic_reader(m_szFileName, &metaInfo);

        statistic_group_list_t sgl;
        doc_stat_reader->read(&sgl);

        list_view_builder_ptr lv_builder = create_list_view_builder(
            GetDlgItem(hwnd, IDC_STATISTICSLIST),
            GetResString(IDS_PROPERTY),
            GetResString(IDS_PROPERTY_VALUE));         

        lv_builder->build(sgl);
    }
    catch (const std::exception&)
    {
    }
}
