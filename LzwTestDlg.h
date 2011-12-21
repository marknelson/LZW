//
// Copyright (c) 2011 Mark Nelson
//
// This software is licensed under the OSI MIT License, contained in
// the file license.txt included with this project.
//
// LzwTestDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// CLzwTestDlg dialog
class CLzwTestDlg : public CDialogEx
{
// Construction
public:
	CLzwTestDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_LZWTEST_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:

	// Generated message map functions
	virtual BOOL OnInitDialog();
    virtual void OnOK();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedPickDirButton();
    afx_msg void OnBnClickedCompressButton();
    afx_msg void OnBnClickedCopyData();
	DECLARE_MESSAGE_MAP()

    void RedrawList(void);
    void PopulateList(CString dir);

    static UINT _cdecl CompressorThread( LPVOID pParam );

    HICON m_hIcon;
    CEdit m_DirectoryName;
    CListCtrl m_List;
    CComboBox m_MaxCodeCombo;
    CButton m_RecurseCheckBox;
    CStatic m_FileInProgress;

    int m_MaxCodeSize;
public:
};
