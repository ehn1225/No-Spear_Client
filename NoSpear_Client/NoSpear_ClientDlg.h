class NOSPEAR;
class FILELISTVIEWER;
class CNoSpearClientDlg : public CDialogEx{
public:
	CNoSpearClientDlg(CWnd* pParent = nullptr);
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_NOSPEAR_CLIENT_DIALOG };
#endif
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	HICON m_hIcon;
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	LRESULT OnTrayNotifyAction(WPARAM wParam, LPARAM lParam);
	CString filename;
	CString filepath;
	bool Has_ADS(CString filepath);
	void PrintFolder(CString folderpath);
	NOTIFYICONDATA nid;
	CBrush m_background;
	NOSPEAR* client = NULL;
	FILELISTVIEWER* fileListViewer = NULL;

public:
	afx_msg void OnBnClickedselectfile();
	afx_msg void OnBnClickeduploadfile();
	afx_msg void OnBnClickedactivelive();
	afx_msg void OnBnClickedinactivelive();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	NOSPEAR* GetClientPtr();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnTrayExit();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnClose();

};
