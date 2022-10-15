#pragma once
class LIVEPROTECT;
class NOSPEAR;
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

private:
	CString filename;
	CString filepath;
	NOSPEAR* client = NULL;
	bool Has_ADS(CString filepath);
	void PrintFolder(CString folderpath);

public:
	afx_msg void OnBnClickedselectfile();
	afx_msg void OnBnClickeduploadfile();
	afx_msg void OnBnClickedactivelive();
	afx_msg void OnBnClickedinactivelive();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	NOSPEAR* GetClientPtr();
};
