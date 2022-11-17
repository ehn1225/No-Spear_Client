#include "pch.h"
#include "NoSpear_Client.h"
#include "NOSPEAR_FILE.h"
#include "NoSpear_ClientDlg.h"
#include "NOSPEAR.h"
#include "MainView.h"

IMPLEMENT_DYNCREATE(MainView, CFormView)

MainView::MainView()
	: CFormView(IDD_MainView)
	, filename(L"파일명 : 선택한 파일이 없습니다.")
	, result_filename(L"검사한 파일이 없습니다.")
	, result_text(L"화면에 파일을 드래그 앤 드롭하거나 검사할 파일을 선택하세요")
	, result_report(L"이곳을 클릭하면 검사 결과 보고서를 웹브라우저로 확인할 수 있습니다.")
	, diagnose_status(L"진행 상태 : 검사 대기")
	, filepath(L"")
	, report_url(L"4nul.org")
{

}

MainView::~MainView(){
}

void MainView::DoDataExchange(CDataExchange* pDX){
	CFormView::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_filename, filename);
	DDX_Text(pDX, label_result_filename, result_filename);
	DDX_Text(pDX, label_result_text, result_text);
	DDX_Text(pDX, label_result_report, result_report);
	DDX_Text(pDX, IDC_diagnose_status, diagnose_status);
}

BEGIN_MESSAGE_MAP(MainView, CFormView)
	ON_WM_CTLCOLOR()
	ON_WM_DROPFILES()
	ON_STN_CLICKED(btn_search, &MainView::OnStnClickedsearch)
	ON_STN_CLICKED(btn_manualDiagnose, &MainView::OnStnClickedmanualdiagnose)
	ON_STN_CLICKED(label_result_report, &MainView::OnStnClickedresultreport)
	ON_BN_CLICKED(btn_activateLive, &MainView::OnBnClickedactivatelive)
	ON_BN_CLICKED(btn_inactivateLive, &MainView::OnBnClickedinactivatelive)
END_MESSAGE_MAP()

#ifdef _DEBUG
void MainView::AssertValid() const{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void MainView::Dump(CDumpContext& dc) const{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


BOOL MainView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext){
	return CFormView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}

void MainView::OnInitialUpdate(){
	CFormView::OnInitialUpdate();
	//title.CreateFontW(30, 20, 0, 0, 700, 0, 0, 0, 0, OUT_DEFAULT_PRECIS, 0, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Malgun Gothic Semilight");
	title.CreatePointFont(120, L"Malgun Gothic Semilight");
	GetDlgItem(IDC_STATIC)->SetFont(&title);
	GetDlgItem(IDC_STATIC2)->SetFont(&title);
	GetDlgItem(IDC_STATIC3)->SetFont(&title);
	nospear_ptr = ((CNoSpearClientDlg*)GetParent())->GetClientPtr();
	UpdateData(FALSE);
	tooltip.Create(this);
	tooltip.AddTool(GetDlgItem(btn_search), L"검사할 파일을 선택합니다");
	tooltip.AddTool(GetDlgItem(btn_manualDiagnose), L"선택한 파일을 검사합니다");
	tooltip.AddTool(GetDlgItem(label_result_report), L"여기를 클릭하면 결과 보고서를 웹페이지로 보여줍니다");
	tooltip.AddTool(GetDlgItem(btn_activateLive), L"실시간 검사를 시작합니다");
	tooltip.AddTool(GetDlgItem(btn_inactivateLive), L"실시간 검사를 종료합니다");
}

HBRUSH MainView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor){
	//HBRUSH hbr = CFormView::OnCtlColor(pDC, pWnd, nCtlColor);
	HBRUSH hbr = CreateSolidBrush(RGB(255, 255, 255));
	return hbr;
}

void MainView::OnStnClickedsearch(){
	// Manual Diagnose
	// 수동검사의 파일선택 버튼을 눌렀을 때 작동을 구현
	//hwp, hwpx, pdf, doc, docx, xls, xlsx
	CString szFilter = _T("문서 파일 (*.hwp, *.hwpx, *.pdf, *.doc, *.docx, *.xls, *.xlsx, *.ppt, *.pptx) | *.hwp; *.hwpx; *.pdf; *.doc; *.docx; *.xls; *.xlsx; *.ppt; *.pptx|");
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, szFilter);

	if (IDOK == dlg.DoModal()) {
		filepath = dlg.GetPathName();
		filename.Format(TEXT("파일명 : %ws"), dlg.GetFileName());
		UpdateData(FALSE);
	}

}

void MainView::OnStnClickedmanualdiagnose(){

	if (filepath.IsEmpty()) {
		AfxMessageBox(L"검사할 문서를 선택하세요");
		return;
	}

	diagnose_status = L"진행 상태 : 검사 진행 중";
	UpdateData(FALSE);
	NOSPEAR_FILE file = NOSPEAR_FILE(filepath);
	nospear_ptr->Diagnose(file);
	result_filename.Format(TEXT("파일명 : %ws"), file.Getfilename());
	result_text.Format(TEXT("검사 결과 : %ws"), file.diag_result.result_msg);
	report_url.Format(TEXT("4nul.org/result?hash=%ws"), file.Getfilehash());
	result_report = L"검사 보고서 : " + report_url;
	diagnose_status = L"진행 상태 : 검사 완료";
	UpdateData(FALSE);
}

void MainView::OnDropFiles(HDROP hDropInfo){
	wchar_t buffer[512] = { 0, };
	DragQueryFile(hDropInfo, 0, buffer, 512);
	CString tmp = CString(buffer);
	filepath = tmp;
	filename.Format(TEXT("파일명 : %ws"), PathFindFileName(tmp));
	UpdateData(FALSE);
	DragFinish(hDropInfo);
	CFormView::OnDropFiles(hDropInfo);
}

void MainView::OnStnClickedresultreport(){
	ShellExecute(this->m_hWnd, TEXT("open"), TEXT("IEXPLORE.EXE"), report_url, NULL, SW_SHOW);
}

void MainView::OnDraw(CDC* /*pDC*/){
	CRect rcWin;
	GetWindowRect(&rcWin);
	CClientDC dc(this);
	dc.MoveTo(0, 0);
	dc.LineTo(rcWin.Width(), 0);
	dc.MoveTo(0, 110);
	dc.LineTo(rcWin.Width(), 110);
	dc.MoveTo(0, 250);
	dc.LineTo(rcWin.Width(), 250);
}

void MainView::OnBnClickedactivatelive() {
	if (nospear_ptr->ActivateLiveProtect(true)) {
		GetDlgItem(btn_activateLive)->EnableWindow(false);
		GetDlgItem(btn_inactivateLive)->EnableWindow(true);
	}
}

void MainView::OnBnClickedinactivatelive(){

	if (!nospear_ptr->ActivateLiveProtect(false)) {
		GetDlgItem(btn_activateLive)->EnableWindow(true);
		GetDlgItem(btn_inactivateLive)->EnableWindow(false);
	}
}


BOOL MainView::PreTranslateMessage(MSG* pMsg){
	tooltip.RelayEvent(pMsg);
	return CFormView::PreTranslateMessage(pMsg);
}
