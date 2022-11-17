#include "pch.h"
#include "NoSpear_Client.h"
#include "QuarantineView.h"
#include "NoSpear_ClientDlg.h"
#include "NOSPEAR.h"
#include "SQLITE.h"

IMPLEMENT_DYNCREATE(QuarantineView, CFormView)

QuarantineView::QuarantineView()
	: CFormView(IDD_QuarantineView)
{

}

QuarantineView::~QuarantineView()
{
}

void QuarantineView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, quarantine_list, quarantine_list_ctrl);
}

BEGIN_MESSAGE_MAP(QuarantineView, CFormView)
	ON_WM_CTLCOLOR()
	ON_NOTIFY(NM_RCLICK, quarantine_list, &QuarantineView::OnNMRClicklist)
	ON_COMMAND(ID_QUARANTINE_delete, &QuarantineView::OnDelete)
	ON_COMMAND(ID_QUARANTINE_recover, &QuarantineView::OnRecover)
	ON_COMMAND(ID_QUARANTINE_report, &QuarantineView::OnReport)
END_MESSAGE_MAP()


// QuarantineView 진단

#ifdef _DEBUG
void QuarantineView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void QuarantineView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG

BOOL QuarantineView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) {
	return CFormView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}

void QuarantineView::OnInitialUpdate() {
	CFormView::OnInitialUpdate();
	title.CreatePointFont(120, L"Malgun Gothic Semilight");
	GetDlgItem(IDC_STATIC)->SetFont(&title);
	nospear_ptr = ((CNoSpearClientDlg*)GetParent())->GetClientPtr();
	quarantineDB = nospear_ptr->GetSQLitePtr();
	quarantine_list_ctrl.InsertColumn(0, L"파일명", LVCFMT_LEFT, 200, -1);
	quarantine_list_ctrl.InsertColumn(1, L"SHA-256", LVCFMT_LEFT, 200, -1);
	quarantine_list_ctrl.InsertColumn(2, L"격리일자", LVCFMT_CENTER, 120, -1);
	quarantine_list_ctrl.InsertColumn(3, L"파일경로", LVCFMT_LEFT, 60, -1);
	quarantine_list_ctrl.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_GRIDLINES);
	RefrestList();
}

HBRUSH QuarantineView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
	//HBRUSH hbr = CFormView::OnCtlColor(pDC, pWnd, nCtlColor);
	HBRUSH hbr = CreateSolidBrush(RGB(255, 255, 255));
	return hbr;
}
void QuarantineView::OnDraw(CDC* /*pDC*/) {
	CRect rcWin;
	GetWindowRect(&rcWin);
	CClientDC dc(this);
	dc.MoveTo(0, 0);
	dc.LineTo(rcWin.Width(), 0);
}
void QuarantineView::RefrestList() {
	quarantine_list_ctrl.DeleteAllItems();
	sqlite3_select p_selResult = quarantineDB->SelectSqlite(L"select * from NOSPEAR_Quarantine;");
	if (p_selResult.pnRow != 0) {
		for (int i = 1; i <= p_selResult.pnRow; i++) {
			int colCtr = 0;
			int nCol = 1;
			int cellPosition = (i * p_selResult.pnColumn) + colCtr;

			CString filepath = SQLITE::Utf8ToCString(p_selResult.pazResult[cellPosition++]);
			quarantine_list_ctrl.InsertItem(i - 1, PathFindFileName(filepath));
			quarantine_list_ctrl.SetItem(i - 1, 1,LVIF_TEXT, CString(p_selResult.pazResult[cellPosition++]), 0, 0, 0, NULL);
			quarantine_list_ctrl.SetItem(i - 1, 2,LVIF_TEXT, CString(p_selResult.pazResult[cellPosition++]), 0, 0, 0, NULL);
			quarantine_list_ctrl.SetItem(i - 1, 3,LVIF_TEXT, filepath, 0, 0, 0, NULL);
		}
	}
}

void QuarantineView::OnNMRClicklist(NMHDR* pNMHDR, LRESULT* pResult){
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	UNREFERENCED_PARAMETER(pNMItemActivate);

	CPoint CurrentPosition;
	::GetCursorPos(&CurrentPosition);

	quarantine_list_ctrl.ScreenToClient(&CurrentPosition);
	select_index = quarantine_list_ctrl.HitTest(CurrentPosition);

	if (-1 == select_index) {
		// 아이템 영역이 아닌 곳에서 마우스 오른쪽 버튼을 선택한 경우
	}
	else {
		::GetCursorPos(&CurrentPosition);
		CMenu MenuTemp;
		MenuTemp.LoadMenu(IDR_Quarantine);
		CMenu* pContextMenu = MenuTemp.GetSubMenu(0);
		pContextMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, CurrentPosition.x, CurrentPosition.y, this);
	}	*pResult = 0;
}
void QuarantineView::OnDelete() {
	CString hash = quarantine_list_ctrl.GetItemText(select_index, 1);
	CString filepath = L".\\Quarantine\\" + hash;
	DeleteFile(filepath);
	quarantineDB->ExecuteSqlite(L"DELETE FROM NOSPEAR_Quarantine WHERE FileHash='" + hash + L"';");
	RefrestList();
}
void QuarantineView::OnRecover() {
	nospear_ptr->InQuarantine(quarantine_list_ctrl.GetItemText(select_index, 3));
	RefrestList();
}
void QuarantineView::OnReport() {
	CString hash = quarantine_list_ctrl.GetItemText(select_index, 1);
	if (hash != L"-") {
		CString url = L"4nul.org/result?hash=" + hash;
		ShellExecute(this->m_hWnd, TEXT("open"), TEXT("IEXPLORE.EXE"), url, NULL, SW_SHOW);
	}
}