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
	, result_report(L"검사 후 이곳을 클릭하면 검사 결과 보고서를 웹브라우저로 출력합니다.")
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
}

BEGIN_MESSAGE_MAP(MainView, CFormView)
	ON_WM_CTLCOLOR()
	ON_STN_CLICKED(btn_search, &MainView::OnStnClickedsearch)
	ON_STN_CLICKED(btn_manualDiagnose, &MainView::OnStnClickedmanualdiagnose)
	ON_STN_CLICKED(label_result_report, &MainView::OnStnClickedresultreport)
	ON_WM_DROPFILES()
END_MESSAGE_MAP()


// MainForm 진단

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
	nospear_ptr = ((CNoSpearClientDlg*)GetParent())->GetClientPtr();
	UpdateData(FALSE);
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
	CString szFilter = _T("문서 파일 (*.hwp, *.hwpx, *.pdf, *.doc, *.docx, *.xls, *.xlsx) | *.hwp; *.hwpx; *.pdf; *.doc; *.docx; *.xls; *.xlsx|");
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, szFilter);

	if (IDOK == dlg.DoModal()) {
		filepath = dlg.GetPathName();
		filename.Format(TEXT("파일명 : %ws"), dlg.GetFileName());
		UpdateData(FALSE);
	}

}

void MainView::OnStnClickedmanualdiagnose(){
	NOSPEAR_FILE file = NOSPEAR_FILE(filepath);
	nospear_ptr->Diagnose(file);
	nospear_ptr->Diagnose(file);
	result_filename.Format(TEXT("파일명 : %ws"), file.Getfilename());
	result_text.Format(TEXT("검사결과 : %ws"), file.diag_result.result_msg);
	result_report.Format(TEXT("4nul.org/result?hash=%ws"), file.Getfilehash());
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
	ShellExecute(this->m_hWnd, TEXT("open"), TEXT("IEXPLORE.EXE"), result_report, NULL, SW_SHOW);
}
