struct SORTPARAM {
	int iSortColumn;
	bool bSortDirect;
	CListCtrl* pList;
};
struct LOCALFILELISTDB {
	CString FilePath;
	CString ZoneIdentifier;
	CString ProcessName;
	CString NOSPEAR;
	CString DiagnoseDate;
	CString Serverity;
	CString FileType;
	CString TimeStamp;
};
class SQLITE;
class FILELISTVIEWER : public CDialogEx
{
	DECLARE_DYNAMIC(FILELISTVIEWER)

public:
	FILELISTVIEWER(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	~FILELISTVIEWER();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FILELISTVIEWDIALOG };
#endif

protected:
	NOSPEAR* nospear_ptr = NULL;
	SQLITE* fileViewerDB;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	HICON m_hIcon;
	int m_iDlgLimitMinWidth;
	int m_iDlgLimitMinHeight;
	std::set<CString> office_file_ext_list;
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	bool HasZoneIdentifierADS(CString filepath);
	bool IsOfficeFile(CString ext);
	static int CALLBACK CompareItem(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	CListCtrl filelistbox;
	NOTIFYICONDATA nid;
	bool bAscending;
	CBrush   m_background;
	CComboBox file_check_combo;
	CToolTipCtrl tooltip;
	bool DB_status;
	std::vector<LOCALFILELISTDB> filelist;
	std::map<CString, bool> ext_filter;
	void ScanLocalFile(CString rootPath);
	unsigned short ReadNospearADS(CString filepath);

public:
	afx_msg void OnHdnItemclickFilelistctrl(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnHdnItemdblclickFilelistctrl(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMDblclkFilelistctrl(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCbnSelchangeCombo1();
	afx_msg void OnBnClickeddiagnose();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnStnClickedrefreshlist();
	afx_msg void OnCheckBoxChange(UINT nID);
	afx_msg void OnStnClickedrefreshdb();
};
