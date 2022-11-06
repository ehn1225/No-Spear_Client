#include "pch.h"
#include "NoSpear_Client.h"
#include "SettingView.h"

IMPLEMENT_DYNCREATE(SettingView, CFormView)

SettingView::SettingView()
	: CFormView(IDD_SettingView)
{
}

SettingView::~SettingView(){
}

void SettingView::DoDataExchange(CDataExchange* pDX){
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(SettingView, CFormView)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// SettingView 진단

#ifdef _DEBUG
void SettingView::AssertValid() const{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void SettingView::Dump(CDumpContext& dc) const{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG

BOOL SettingView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) {
	return CFormView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}


void SettingView::OnInitialUpdate() {
	CFormView::OnInitialUpdate();
}

HBRUSH SettingView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor){
	//HBRUSH hbr = CFormView::OnCtlColor(pDC, pWnd, nCtlColor);
	HBRUSH hbr = CreateSolidBrush(RGB(255, 255, 255));
	return hbr;
}
