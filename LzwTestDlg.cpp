//
// Copyright (c) 2011 Mark Nelson
//
// This software is licensed under the OSI MIT License, contained in
// the file license.txt included with this project.
//
// LzwTestDlg.cpp : implementation file
//
#include "stdafx.h"
#include "LzwTest.h"
#include "LzwTestDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define _ITERATOR_DEBUG_LEVEL 0
#include <fstream>
#include "lzw_streambase.h"
#include "lzw-d.h"
#include "lzw.h"

// CLzwTestDlg dialog

CLzwTestDlg::CLzwTestDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CLzwTestDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CLzwTestDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT1, m_DirectoryName);
    DDX_Control(pDX, IDC_LIST1, m_List);
    DDX_Control(pDX, IDC_MAX_CODE_COMBO, m_MaxCodeCombo);
    DDX_Control(pDX, IDC_RECURSE, m_RecurseCheckBox);
    DDX_Control(pDX, IDC_FILE_IN_PROGRESS, m_FileInProgress);
}

BEGIN_MESSAGE_MAP(CLzwTestDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_PICK_DIR_BUTTON, &CLzwTestDlg::OnBnClickedPickDirButton)
    ON_BN_CLICKED(IDC_COMPRESS_BUTTON, &CLzwTestDlg::OnBnClickedCompressButton)
    ON_BN_CLICKED(IDC_COPY_DATA, &CLzwTestDlg::OnBnClickedCopyData)
END_MESSAGE_MAP()


// CLzwTestDlg message handlers

BOOL CLzwTestDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
    CString dir = AfxGetApp()->GetProfileString( "SavedSettings", "Directory" );
    m_DirectoryName.SetWindowText( dir );
    CString max_code = AfxGetApp()->GetProfileString( "SavedSettings", "Maximum Code" );
    if ( max_code.GetLength() )
        m_MaxCodeCombo.SetWindowText( max_code );
    else
        m_MaxCodeCombo.SetCurSel( m_MaxCodeCombo.GetCount() - 1 );
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CLzwTestDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CLzwTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

static int CALLBACK BrowseCallbackProc(HWND hwnd,UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	// If the BFFM_INITIALIZED message is received
	// set the path to the start path.
	switch (uMsg)
	{
		case BFFM_INITIALIZED:
		{
			if (NULL != lpData)
			{
				SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
			}
		}
	}

	return 0; // The function should always return 0.
}

void CLzwTestDlg::OnBnClickedPickDirButton()
{
    CString current_name;
    m_DirectoryName.GetWindowText( current_name );
    BROWSEINFO   bi; 
    ZeroMemory(&bi,   sizeof(bi)); 
    char szDisplayName[MAX_PATH] = {0};
    bi.hwndOwner        =   NULL; 
    bi.pidlRoot         =   NULL; 
    bi.pszDisplayName   =   szDisplayName;
    bi.lpszTitle        =   "Please select a folder to scan";
    bi.ulFlags          =   BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    bi.iImage           =   0;  
    bi.lpfn             =   BrowseCallbackProc;
    bi.lParam           =   (LPARAM) (const char *) current_name;

    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    char szPathName[MAX_PATH]; 
    if ( NULL != pidl)
    {
        BOOL bRet = SHGetPathFromIDList(pidl,szPathName);
         if(FALSE == bRet)
              return;
         m_DirectoryName.SetWindowText(szPathName);
    }
}

UINT _cdecl CLzwTestDlg::CompressorThread( LPVOID pParam )
{
    CTime start = CTime::GetCurrentTime();
    CLzwTestDlg *pDlg = (CLzwTestDlg *) pParam;
    CString dir;
    pDlg->m_DirectoryName.GetWindowText( dir );
    CHeaderCtrl* pHeaderCtrl = pDlg->m_List.GetHeaderCtrl();
    int n = pDlg->m_List.GetItemCount();
    for ( int i = 0 ; i < n ; i++ )
    {
        CString name = pDlg->m_List.GetItemText( i, 6 );
        OutputDebugString( name + "\n" );
        char temp_path[ MAX_PATH ];
        GetTempPath( MAX_PATH, temp_path );
        char temp_name_lzw[ MAX_PATH ];
        GetTempFileName( temp_path, "lzw_", 0, temp_name_lzw );
        char temp_name_out[ MAX_PATH ];
        GetTempFileName( temp_path, "lzw_", 0, temp_name_out );
        std::ifstream in( name, std::ios_base::binary );
        std::ofstream lzw_out( temp_name_lzw, std::ios_base::binary );
        CString temp = "Compressing " + name;
        pDlg->m_FileInProgress.SetWindowText(temp);
        lzw::compress( (std::istream &) in, (std::ostream&) lzw_out, pDlg->m_MaxCodeSize );
        long lzw_size = (long) lzw_out.tellp();
        in.clear();
        long input_size = (long) in.tellg();
        lzw_out.close();
        std::ifstream lzw_in( temp_name_lzw, std::ios_base::binary );
        std::fstream out( temp_name_out, std::fstream::in | std::fstream::out | std::fstream::binary );
        temp = "Decompressing " + name;
        pDlg->m_FileInProgress.SetWindowText(temp);
        lzw::decompress( (std::istream &) lzw_in, (std::ostream&) out, pDlg->m_MaxCodeSize );
        pDlg->m_FileInProgress.SetWindowText("");
        CString pass = "passed";
        out.flush();
        long output_size = (long) out.tellp();
        if ( output_size != input_size ) 
            pass = "Size mismatch";
        else {
            in.seekg( 0 );
            out.seekg( 0 );
            char c1;
            char c2;
            while ( in.get(c1) && out.get(c2) ) {
                if ( c1 != c2 ) {
                    pass = "Compare fail";
                    break;
                }
            }
        }
        in.close();
        out.close();
        lzw_in.close();
        DeleteFile( temp_name_lzw );
        DeleteFile( temp_name_out );
        CString text;
        text.Format( "%ld", lzw_size );
        pDlg->m_List.SetItemText( i, 2, text);
        if ( input_size == 0 ) 
            text = "???";
        else {
            int ratio = (lzw_size * 100 ) / input_size;
            text.Format( "%d%%", ratio );
        }
        pDlg->m_List.SetItemText( i, 3, text);
        if ( input_size == 0 ) 
            text = "???";
        else {
            double bpb = (lzw_size * 8.0 ) / input_size;
            text.Format( "%5.2f", bpb );
        }
        pDlg->m_List.SetItemText( i, 4, text);
        pDlg->m_List.SetItemText( i, 5, pass );
        pDlg->RedrawList();
    }
    CTime finish = CTime::GetCurrentTime();
    CTimeSpan diff = finish - start;
    pDlg->m_List.InsertItem( n, "Elapsed time: " );
    pDlg->m_List.SetItemText( n, 1, diff.Format( "%H:%M:%S" ) );
    pDlg->RedrawList();
    return 0;
}

void CLzwTestDlg::OnBnClickedCompressButton()
{
    CString max_code_string;
    m_MaxCodeCombo.GetWindowText( max_code_string );
    errno = 0;
    m_MaxCodeSize = atoi( max_code_string );
    if ( m_MaxCodeSize == 0 || errno != 0 ) {
        AfxMessageBox( "Invalid value for code size" );
        return;
    }

    m_List.DeleteAllItems();
    static bool first_time = true;
    if ( first_time ) {
        first_time = false;
        m_List.InsertColumn( 0, "File" );
        m_List.InsertColumn( 1, "Size" );
        m_List.InsertColumn( 2, "Comp. Size" );
        m_List.InsertColumn( 3, "Ratio" );
        m_List.InsertColumn( 4, "Bits/Byte" );
        m_List.InsertColumn( 5, "Pass/Fail" );
        m_List.InsertColumn( 6, "Full Name" );
    }
    CString base;
    m_DirectoryName.GetWindowText( base );
    PopulateList( base );
    RedrawList();
    AfxBeginThread( CompressorThread, this );
}

void CLzwTestDlg::PopulateList(CString dir)
{
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;
    CString base = dir + "\\*.*";
    hFind = FindFirstFile( base, &FindFileData );
    if ( hFind )
        do {
            if ( (FindFileData.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_DEVICE|FILE_ATTRIBUTE_SYSTEM)) == 0  ) {
                int item = m_List.GetItemCount();
                m_List.InsertItem( item, FindFileData.cFileName );
                CString size;
                size.Format("%d", FindFileData.nFileSizeLow );
                m_List.SetItemText( item, 1, size );
                CString full_name = dir + "\\" + FindFileData.cFileName;
                m_List.SetItemText( item, 6, full_name );
                item++;
            } 
        } while ( FindNextFile( hFind, &FindFileData ) );
    FindClose( hFind );
    if ( m_RecurseCheckBox.GetCheck() == BST_UNCHECKED )
        return;
    //
    // Now recurse through all directories
    //
    hFind = FindFirstFile( base, &FindFileData );
    if ( hFind )
        do {
            if ( (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) && !( FindFileData.dwFileAttributes & (FILE_ATTRIBUTE_DEVICE|FILE_ATTRIBUTE_SYSTEM)) ) {
                if ( strcmp( FindFileData.cFileName, "." ) && strcmp( FindFileData.cFileName, ".." ) ) {
                    CString newdir = dir + "\\" + FindFileData.cFileName;
                    PopulateList( newdir );
                }
            } 
        } while ( FindNextFile( hFind, &FindFileData ) );
    FindClose( hFind );
}

void CLzwTestDlg::RedrawList(void)
{
    m_List.SetRedraw(FALSE);
    CHeaderCtrl* pHeaderCtrl = m_List.GetHeaderCtrl();
    int nColumnCount = pHeaderCtrl->GetItemCount();
    for (int i = 0; i < (nColumnCount-1); i++)
    {
        m_List.SetColumnWidth(i, LVSCW_AUTOSIZE);
        int nColumnWidth = m_List.GetColumnWidth(i);
        m_List.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
        int nHeaderWidth = m_List.GetColumnWidth(i); 
        m_List.SetColumnWidth(i, max(nColumnWidth, nHeaderWidth));
    }
    m_List.SetColumnWidth((nColumnCount-1), 0 );
    m_List.SetRedraw(TRUE);
}


void CLzwTestDlg::OnBnClickedCopyData()
{
    CString data = "Full Name\tBase Name\tUncompressed Size\tCompressed Size\tRatio\tBits per Byte\tPass/Fail\r\n";
    CHeaderCtrl* pHeaderCtrl = m_List.GetHeaderCtrl();
    int nColumnCount = pHeaderCtrl->GetItemCount();
    int n = m_List.GetItemCount();
    for ( int i = 0 ; i < n ; i++ ) {
        data += m_List.GetItemText( i, 6 ) + "\t";
        for ( int j = 0 ; j < (nColumnCount-1) ; j++ ) {
            data += m_List.GetItemText( i, j );
            if ( j == (nColumnCount-2) )
                data += "\r\n";
            else
                data += "\t";
        }
    }
    if ( OpenClipboard() ) {
        EmptyClipboard();
        HGLOBAL hClipboardData = GlobalAlloc(GMEM_DDESHARE, data.GetLength()+1);
        char * pchData = (char*) GlobalLock(hClipboardData);
        strcpy_s( pchData, data.GetLength()+1, data);
        GlobalUnlock(hClipboardData);
        SetClipboardData(CF_TEXT,hClipboardData);
        CloseClipboard();
    }
}


void CLzwTestDlg::OnOK()
{
    CString dir;
    m_DirectoryName.GetWindowText( dir );
    AfxGetApp()->WriteProfileString( "SavedSettings", "Directory", dir );
    CString max_code;
    m_MaxCodeCombo.GetWindowText( max_code );
    AfxGetApp()->WriteProfileString( "SavedSettings", "Maximum Code", max_code );
    CDialogEx::OnOK();
}
