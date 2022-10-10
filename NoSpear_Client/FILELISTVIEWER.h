#pragma once
#include "afxdialogex.h"
#include "FlexibleDialog.h"

// FILELISTVIEWER 대화 상자

class FILELISTVIEWER : public CFlexibleDialog
{
	DECLARE_DYNAMIC(FILELISTVIEWER)

public:
	FILELISTVIEWER(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~FILELISTVIEWER();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FILELISTVIEWDIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	HICON m_hIcon;

public:

	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	bool Has_ADS(CString filepath);
	void PrintFolder(CString folderpath);
	static int CALLBACK CompareItem(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	CListCtrl filelistbox;
	int nSortColumn;
	bool bAscending;
	struct SORTPARAM {
		int iSortColumn;
		bool bSortDirect;
		CListCtrl* pList;
	};
	afx_msg void OnHdnItemclickFilelistctrl(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedSelectfolder();
	afx_msg void OnHdnItemdblclickFilelistctrl(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMDblclkFilelistctrl(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCbnSelchangeCombo1();
	CComboBox file_check_combo;
	afx_msg void OnBnClickedButton2();
};
