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
	CFont text;
	CString filepath;
	CString filename;
	CString result_filename;
	CString result_text;
	CString result_report;
	NOSPEAR* nospear_ptr = NULL;
public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	virtual void OnInitialUpdate();
	MainView();           // 동적 만들기에 사용되는 protected 생성자입니다.
	virtual ~MainView();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnStnClickedsearch();
	afx_msg void OnStnClickedmanualdiagnose();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnStnClickedresultreport();
};


