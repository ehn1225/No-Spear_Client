#pragma once
struct SORTPARAM {
	int iSortColumn;
	bool bSortDirect;
	CListCtrl* pList;
};
struct LOCALFILELISTDB {
	CString FilePath;
	int ZoneIdentifier;
	CString ProcessName;
	int NOSPEAR;
	CString DiagnoseDate;
	int Serverity;
	CString FileType;
	CString Hash;
	CString TimeStamp;
};
class SQLITE;
class FILELISTVIEWER : public CDialogEx{
	DECLARE_DYNAMIC(FILELISTVIEWER)
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FILELISTVIEWDIALOG };
#endif

protected:
	HICON m_hIcon;
	NOSPEAR* nospear_ptr = NULL;
	SQLITE* fileViewerDB;
	int m_iDlgLimitMinWidth;
	int m_iDlgLimitMinHeight;
	CListCtrl filelistbox;
	NOTIFYICONDATA nid;
	bool bAscending;
	CBrush m_background;
	CComboBox file_check_combo;
	CToolTipCtrl tooltip;
	int select_index;
	bool DB_status;
	std::vector<LOCALFILELISTDB> filelist;
	std::map<CString, bool> ext_filter;

	static int CALLBACK CompareItem(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.


public:
	FILELISTVIEWER(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	~FILELISTVIEWER();
	afx_msg void OnHdnItemclickFilelistctrl(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnHdnItemdblclickFilelistctrl(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMDblclkFilelistctrl(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCbnSelchangeCombo1();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnStnClickedrefreshlist();
	afx_msg void OnCheckBoxChange(UINT nID);
	afx_msg void OnStnClickedrefreshdb();
	afx_msg void OnNMRClickFilelistctrl(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnManu1();
	afx_msg void OnManu2_0();
	afx_msg void OnManu2_3();
	afx_msg void OnManu3_0();
	afx_msg void OnManu3_1();
	afx_msg void OnManu3_2();
	afx_msg void OnManu4();
	afx_msg void OnManu5();
	afx_msg void OnManu6();
	afx_msg void OnNMClickFilelistctrl(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnStnClickedsearch1();
	afx_msg void OnStnClickedsearch2();
	afx_msg void OnStnClickedsearch3();
	afx_msg void OnStnClickedsearch4();
	afx_msg void OnStnClickedsearch5();
	afx_msg void OnStnClickedselectdiagnose();
};
