#pragma once
class NOSPEAR;
class FILELISTVIEWER;
class MainView;
class SettingView;
class QuarantineView;
static bool clientThreadStatus;
static CWinThread* clientThread = NULL;
struct STPARAM {
	NOSPEAR *nospear;

	STPARAM(NOSPEAR* ptr)
		:nospear(ptr) {}
};
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
	CToolTipCtrl tooltip;
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	LRESULT OnTrayNotifyAction(WPARAM wParam, LPARAM lParam);
	bool Has_ADS(CString filepath);
	void PrintFolder(CString folderpath);
	NOTIFYICONDATA nid;
	CBrush m_background;
	NOSPEAR* client = NULL;
	FILELISTVIEWER* fileListViewer = NULL;
	MainView* m_pForm1;
	SettingView* m_pForm2;
	QuarantineView* m_pForm3;
	void AllocForms();
	void ShowForm(int idx);
	afx_msg void OnDestroy();
	CStatic logo_ctl;
	static UINT ClientThreadFunc(LPVOID param);


public:
	NOSPEAR* GetClientPtr();
	afx_msg void OnTrayExit();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnStnClickedfileviewer();
	afx_msg void OnStnClickedhome();
	afx_msg void OnStnClickedsetting();
	afx_msg void OnStnClickedquarantine();
	afx_msg void OnStnClickedframe();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
