#pragma once
class NOSPEAR;
class SQLITE;
class QuarantineView : public CFormView
{
	DECLARE_DYNCREATE(QuarantineView)

public:
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_QuarantineView };
#endif
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	CFont title;
	NOSPEAR* nospear_ptr = NULL;
	SQLITE* quarantineDB;
	DECLARE_MESSAGE_MAP()
	int select_index;

public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	virtual void OnInitialUpdate();
	QuarantineView();
	virtual ~QuarantineView();
	virtual void OnDraw(CDC* /*pDC*/);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	CListCtrl quarantine_list_ctrl;
	void RefrestList();
	afx_msg void OnNMRClicklist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDelete();
	afx_msg void OnRecover();
	afx_msg void OnReport();
};


