#pragma once
class NOSPEAR;
class MainView : public CFormView{
	DECLARE_DYNCREATE(MainView)

public:
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MainView };
#endif
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	DECLARE_MESSAGE_MAP()
	CFont title;
	CString filepath;
	CString filename;
	CString result_filename;
	CString result_text;
	CString result_report;
	CString report_url;
	CString diagnose_status;
	NOSPEAR* nospear_ptr = NULL;
	CToolTipCtrl tooltip;
public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	virtual void OnInitialUpdate();
	MainView();
	virtual ~MainView();
	virtual void OnDraw(CDC* /*pDC*/);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnStnClickedsearch();
	afx_msg void OnStnClickedmanualdiagnose();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnStnClickedresultreport();
	afx_msg void OnBnClickedactivatelive();
	afx_msg void OnBnClickedinactivatelive();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};


