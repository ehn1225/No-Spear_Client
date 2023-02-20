#include "AutoUpdate.h"
class NOSPEAR;
class SQLITE;
class SettingView : public CFormView{
	DECLARE_DYNCREATE(SettingView)

public:
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SettingView };
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
	NOSPEAR* nospear_ptr = NULL;
	AutoUpdate au;
	CString strVersionNow;
	CString strVersionNew;
	CString strVersionPattern;
	SQLITE* settingDB;
	CToolTipCtrl tooltip;
	CString strVersionURL = L"http://localhost/version"; //최신 버전 정보를 받을 수 있는 홈페이지("2.4.6.0"과 같이 버전 문자열이 있으면 됨.)
	void UpdatePatternInfo();

public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	virtual void OnInitialUpdate();
	SettingView();           // 동적 만들기에 사용되는 protected 생성자입니다.
	virtual ~SettingView();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual void OnDraw(CDC* /*pDC*/);
	afx_msg void OnBnClickedButton1();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnStnClickedupdatepattren();
};


