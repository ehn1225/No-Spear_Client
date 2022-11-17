#include "pch.h"
#include "NoSpear_Client.h"
#include "SettingView.h"
#include "NOSPEAR.h"
#include "NoSpear_ClientDlg.h"
#include "SQLITE.h"

IMPLEMENT_DYNCREATE(SettingView, CFormView)
#define UPDATECHECK_BROWSER_STRING _T("No-Spear Update")

SettingView::SettingView()
	: CFormView(IDD_SettingView)
	, strVersionNow(_T(""))
	, strVersionNew(_T(""))
	, strVersionPattern(_T(""))
{
}

SettingView::~SettingView(){
}

void SettingView::DoDataExchange(CDataExchange* pDX){
	CFormView::DoDataExchange(pDX);
	DDX_Text(pDX, version_now, strVersionNow);
	DDX_Text(pDX, version_new, strVersionNew);
	DDX_Text(pDX, version_pattern, strVersionPattern);
}

BEGIN_MESSAGE_MAP(SettingView, CFormView)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON1, &SettingView::OnBnClickedButton1)
	ON_STN_CLICKED(btn_update_pattren, &SettingView::OnStnClickedupdatepattren)
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

void SettingView::UpdatePatternInfo(){
	sqlite3_select p_selResult = settingDB->SelectSqlite(L"select TimeStamp from NOSPEAR_VersionInfo WHERE VersionName='BlackListDB'");
	CString timeStamp;
	if (p_selResult.pnRow != 0) {
		timeStamp = SQLITE::Utf8ToCString(p_selResult.pazResult[1]);
		strVersionPattern = L"현재 패턴 버전 : " + timeStamp;
	}
}

BOOL SettingView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) {
	return CFormView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}


void SettingView::OnInitialUpdate() {
	CFormView::OnInitialUpdate();
	//title.CreateFontW(30, 20, 0, 0, 700, 0, 0, 0, 0, OUT_DEFAULT_PRECIS, 0, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Malgun Gothic Semilight");
	title.CreatePointFont(120, L"Malgun Gothic Semilight");
	GetDlgItem(IDC_STATIC)->SetFont(&title);
	GetDlgItem(IDC_STATIC2)->SetFont(&title);
	nospear_ptr = ((CNoSpearClientDlg*)GetParent())->GetClientPtr();
	settingDB = nospear_ptr->GetSQLitePtr();

	tooltip.Create(this);
	tooltip.AddTool(GetDlgItem(btn_updateClient), L"클라이언트 프로그램을 최신 버전으로 업데이트 합니다");
	tooltip.AddTool(GetDlgItem(btn_update_pattren), L"클라이언트 백신 패턴을 업데이트 합니다");

	CString version_online = L"";
	CString version_offline = L"";
	HINTERNET hInet = InternetOpen(UPDATECHECK_BROWSER_STRING, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, NULL);
	HINTERNET hUrl = InternetOpenUrl(hInet, CString("http://4nul.org:3000/version"), NULL, -1L,
	//HINTERNET hUrl = InternetOpenUrl(hInet, CString("http://localhost"), NULL, -1L,
		INTERNET_FLAG_RELOAD | INTERNET_FLAG_PRAGMA_NOCACHE |
		INTERNET_FLAG_NO_CACHE_WRITE | WININET_API_FLAG_ASYNC, NULL);
	if (hUrl) {
		char szBuffer[512] = { 0, };
		DWORD dwRead;
		if (InternetReadFile(hUrl, szBuffer, sizeof(szBuffer), &dwRead) && dwRead > 0) {
			version_online = CString(szBuffer);
		}
		InternetCloseHandle(hUrl);
	}
	InternetCloseHandle(hInet);

	TCHAR szPath[MAX_PATH];
	if (GetModuleFileName(NULL, szPath, MAX_PATH)) {
		SG_Version ver;
		au.SG_GetVersion(szPath, &ver);
		version_offline.Format(TEXT("%d.%d.%d.%d"), ver.Major, ver.Minor, ver.Revision, ver.SubRevision);
		strVersionNow = L"현재 버전 : " + version_offline;
	}
	if (version_online == L"") {
		strVersionNew = L"업데이트 서버 연결 실패";
	}
	else {
		strVersionNew = L"최신 버전 : " + version_online;
	}

	if (version_offline != L"" && version_online != L"") {
		if (version_offline != version_online)
			GetDlgItem(btn_updateClient)->EnableWindow(true);
	}

	UpdatePatternInfo();
	UpdateData(false);
}

HBRUSH SettingView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor){
	//HBRUSH hbr = CFormView::OnCtlColor(pDC, pWnd, nCtlColor);
	HBRUSH hbr = CreateSolidBrush(RGB(255, 255, 255));
	return hbr;
}


void SettingView::OnDraw(CDC* /*pDC*/){
	CRect rcWin;
	GetWindowRect(&rcWin);
	CClientDC dc(this);
	dc.MoveTo(0, 0);
	dc.LineTo(rcWin.Width(), 0);
}

void SettingView::OnBnClickedButton1(){
	if (au.CheckForUpdates() == false) {
		AfxMessageBox(L"업데이트에 실패하였습니다.");
	}
}


BOOL SettingView::PreTranslateMessage(MSG* pMsg){
	tooltip.RelayEvent(pMsg);
	return CFormView::PreTranslateMessage(pMsg);
}


void SettingView::OnStnClickedupdatepattren(){
	nospear_ptr->UpdataBlackListDB();
	UpdatePatternInfo();
	UpdateData(false);
}
