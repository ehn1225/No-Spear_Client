#include "pch.h"
#include "NoSpear_Client.h"
#include "NoSpear_ClientDlg.h"
#include "afxdialogex.h"
#include "FILELISTVIEWER.h"
#include "NOSPEAR.h"
#include "afxwin.h"
#include "SQLITE.h"

using namespace std;
namespace fs = std::filesystem;
#define WM_TRAY_NOTIFYICACTION (WM_USER + 10)

IMPLEMENT_DYNAMIC(FILELISTVIEWER, CDialogEx)

FILELISTVIEWER::FILELISTVIEWER(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_FILELISTVIEWDIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	
}

FILELISTVIEWER::~FILELISTVIEWER(){
}

void FILELISTVIEWER::DoDataExchange(CDataExchange* pDX){
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FileListCtrl, filelistbox);
	DDX_Control(pDX, IDC_COMBO1, file_check_combo);
}

BOOL FILELISTVIEWER::OnInitDialog(){
	CDialogEx::OnInitDialog();
	nospear_ptr = ((CNoSpearClientDlg*)GetParent())->GetClientPtr();
	fileViewerDB = nospear_ptr->GetSQLitePtr();

	filelistbox.InsertColumn(0, L"파일명", LVCFMT_LEFT, 200, -1);
	filelistbox.InsertColumn(1, L"실행 권한", LVCFMT_CENTER, 70, -1);
	filelistbox.InsertColumn(2, L"외부파일", LVCFMT_CENTER, 60, -1);
	filelistbox.InsertColumn(3, L"Create Process", LVCFMT_CENTER, 110, -1);
	filelistbox.InsertColumn(4, L"검사 날짜", LVCFMT_LEFT, 100, -1);
	filelistbox.InsertColumn(5, L"SHA_256", LVCFMT_LEFT, 100, -1);
	filelistbox.InsertColumn(6, L"위험도", LVCFMT_CENTER, 70, -1);
	filelistbox.InsertColumn(7, L"생성 일자", LVCFMT_CENTER, 140, -1);
	filelistbox.InsertColumn(8, L"파일경로", LVCFMT_CENTER, 100, -1);
	filelistbox.SetExtendedStyle(LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_GRIDLINES);

	CRect rctComboBox, rctDropDown;
	file_check_combo.GetClientRect(&rctComboBox);
	file_check_combo.GetDroppedControlRect(&rctDropDown);
	file_check_combo.GetParent()->ScreenToClient(&rctDropDown);
	rctDropDown.bottom = rctDropDown.top + rctComboBox.Height() + 400;
	file_check_combo.MoveWindow(&rctDropDown);

	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	CRect rcWin;
	GetWindowRect(&rcWin);
	m_iDlgLimitMinWidth = rcWin.Width();
	m_iDlgLimitMinHeight = rcWin.Height();

	m_background.CreateSolidBrush(RGB(255,255,255));

	office_file_ext_list.insert(L".doc");
	office_file_ext_list.insert(L".docx");
	office_file_ext_list.insert(L".xls");
	office_file_ext_list.insert(L".xlsx");
	office_file_ext_list.insert(L".pptx");
	office_file_ext_list.insert(L".ppsx");
	office_file_ext_list.insert(L".hwp");
	office_file_ext_list.insert(L".hwpx");
	office_file_ext_list.insert(L".pdf");
	tooltip.Create(this);
	tooltip.AddTool(GetDlgItem(IDC_refreshDB), L"DB 새로고침");
	tooltip.AddTool(GetDlgItem(IDC_refreshlist), L"화면 새로고침");
	tooltip.AddTool(GetDlgItem(IDC_search1), L"한글 문서 선택");
	tooltip.AddTool(GetDlgItem(IDC_search2), L"PDF 문서 선택");
	tooltip.AddTool(GetDlgItem(IDC_search3), L"PPT 문서 선택");
	tooltip.AddTool(GetDlgItem(IDC_search4), L"WORD 문서 선택");
	tooltip.AddTool(GetDlgItem(IDC_search5), L"EXCEL 문서 선택");
	tooltip.AddTool(GetDlgItem(btn_diagnose), L"체크한 문서를 검사합니다.");

	if (fileViewerDB->DatabaseOpen(L"NOSPEAR")) {
		AfxTrace(TEXT("[LIVEPROTECT::LIVEPROTECT] Can't Create NOSPEAR_HISTORY DataBase.\n"));
		DB_status = false;
	}
	else{
		DB_status = true;
		fileViewerDB->ExecuteSqlite(L"CREATE TABLE IF NOT EXISTS NOSPEAR_LocalFileList(FilePath TEXT NOT NULL PRIMARY KEY, ZoneIdentifier INTEGER, ProcessName TEXT, NOSPEAR INTEGER, DiagnoseDate TEXT, Hash TEXT, Serverity INTEGER, FileType TEXT, TimeStamp TEXT not null DEFAULT (datetime('now', 'localtime')));");
	}
	ext_filter.insert(make_pair(".doc", true));
	ext_filter.insert(make_pair(".docx", true));
	ext_filter.insert(make_pair(".xls", true));
	ext_filter.insert(make_pair(".xlsx", true));
	ext_filter.insert(make_pair(".pptx", true));
	ext_filter.insert(make_pair(".ppsx", true));
	ext_filter.insert(make_pair(".hwp", true));
	ext_filter.insert(make_pair(".hwpx", true));
	ext_filter.insert(make_pair(".pdf", true));
	((CButton*)GetDlgItem(IDC_CHECK1))->SetCheck(true);
	((CButton*)GetDlgItem(IDC_CHECK2))->SetCheck(true);
	((CButton*)GetDlgItem(IDC_CHECK3))->SetCheck(true);
	((CButton*)GetDlgItem(IDC_CHECK4))->SetCheck(true);
	((CButton*)GetDlgItem(IDC_CHECK5))->SetCheck(true);

	OnStnClickedrefreshlist();
	return 0;
}

void FILELISTVIEWER::OnPaint(){
	if (IsIconic())	{
		CPaintDC dc(this);
		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;
		dc.DrawIcon(x, y, m_hIcon);
	}
	else{
		CDialogEx::OnPaint();
	}
}

BEGIN_MESSAGE_MAP(FILELISTVIEWER, CDialogEx)
	ON_WM_GETMINMAXINFO()
	ON_CBN_SELCHANGE(IDC_COMBO1, &FILELISTVIEWER::OnCbnSelchangeCombo1)
	ON_BN_CLICKED(btn_diagnose, &FILELISTVIEWER::OnBnClickeddiagnose)
	ON_WM_CTLCOLOR()
	ON_STN_CLICKED(IDC_refreshlist, &FILELISTVIEWER::OnStnClickedrefreshlist)
	ON_COMMAND_RANGE(IDC_CHECK1, IDC_CHECK5, &FILELISTVIEWER::OnCheckBoxChange)
	ON_STN_CLICKED(IDC_refreshDB, &FILELISTVIEWER::OnStnClickedrefreshdb)
	ON_COMMAND(ID_MANU_1, &FILELISTVIEWER::OnManu1)
	ON_COMMAND(ID_MANU_2_0, &FILELISTVIEWER::OnManu2_0)
	ON_COMMAND(ID_MANU_2_3, &FILELISTVIEWER::OnManu2_3)
	ON_COMMAND(ID_MANU_3_0, &FILELISTVIEWER::OnManu3_0)
	ON_COMMAND(ID_MANU_3_1, &FILELISTVIEWER::OnManu3_1)
	ON_COMMAND(ID_MANU_3_2, &FILELISTVIEWER::OnManu3_2)
	ON_COMMAND(ID_MANU_4, &FILELISTVIEWER::OnManu4)
	ON_COMMAND(ID_MANU_5, &FILELISTVIEWER::OnManu5)
	ON_COMMAND(ID_MANU_6, &FILELISTVIEWER::OnManu6)
	ON_NOTIFY(HDN_ITEMCLICK, 0, &FILELISTVIEWER::OnHdnItemclickFilelistctrl)
	ON_NOTIFY(NM_DBLCLK, IDC_FileListCtrl, &FILELISTVIEWER::OnNMDblclkFilelistctrl)
	ON_NOTIFY(NM_RCLICK, IDC_FileListCtrl, &FILELISTVIEWER::OnNMRClickFilelistctrl)
	ON_NOTIFY(NM_CLICK, IDC_FileListCtrl, &FILELISTVIEWER::OnNMClickFilelistctrl)
	ON_STN_CLICKED(IDC_search1, &FILELISTVIEWER::OnStnClickedsearch1)
	ON_STN_CLICKED(IDC_search2, &FILELISTVIEWER::OnStnClickedsearch2)
	ON_STN_CLICKED(IDC_search3, &FILELISTVIEWER::OnStnClickedsearch3)
	ON_STN_CLICKED(IDC_search4, &FILELISTVIEWER::OnStnClickedsearch4)
	ON_STN_CLICKED(IDC_search5, &FILELISTVIEWER::OnStnClickedsearch5)
END_MESSAGE_MAP()

void FILELISTVIEWER::OnGetMinMaxInfo(MINMAXINFO* lpMMI){
	lpMMI->ptMinTrackSize.x = m_iDlgLimitMinWidth;
	lpMMI->ptMinTrackSize.y = m_iDlgLimitMinHeight;
	CDialogEx::OnGetMinMaxInfo(lpMMI);
}

bool FILELISTVIEWER::IsOfficeFile(CString ext) {
	set<CString>::iterator it = office_file_ext_list.find(ext);

	if (it != office_file_ext_list.end())
		return true;
	else
		return false;
}

void FILELISTVIEWER::ScanLocalFile(CString rootPath) {
	//매개변수로 입력된 폴더 경로를 재귀 탐색하여 문서 파일을 DB에 저장함
	//Host -> DB 방향 검사
	//filesystem test, https://stackoverflow.com/questions/62988629/c-stdfilesystemfilesystem-error-exception-trying-to-read-system-volume-inf

	string strfilepath = string(CT2CA(rootPath));
	fs::path rootdir(strfilepath);
	CString rootname = CString(rootdir.root_name().string().c_str()) + "\\";

	//NTFS 파일 시스템만 ADS지원함
	bool bNTFS = false;
	wchar_t szVolName[MAX_PATH], szFSName[MAX_PATH];
	DWORD dwSN, dwMaxLen, dwVolFlags;

	::GetVolumeInformation(rootname, szVolName, MAX_PATH, &dwSN, &dwMaxLen, &dwVolFlags, szFSName, MAX_PATH);
	if (CString(szFSName) == L"NTFS")
		bNTFS = true;

	auto iter = fs::recursive_directory_iterator(rootdir, fs::directory_options::skip_permission_denied);
	auto end_iter = fs::end(iter);
	auto ec = std::error_code();
	int count = 0;
	int rc = 0;

	for (; iter != end_iter; iter.increment(ec)) {
		if (ec) {
			continue;
		}

		CString ext(iter->path().extension().string().c_str());
		CString path(iter->path().string().c_str());

		if (ext == L".exe") {
			CString tmp = path;
			CString tmp_ext;
			while ((tmp_ext = PathFindExtension(tmp)).GetLength() != 0) {
				if (IsOfficeFile(tmp_ext)) {
					ZeroMemory(&nid, sizeof(nid));
					nid.cbSize = sizeof(nid);
					nid.dwInfoFlags = NIIF_WARNING;
					nid.uFlags = NIF_MESSAGE | NIF_INFO | NIF_ICON;
					nid.uTimeout = 2000;
					nid.hWnd = AfxGetApp()->m_pMainWnd->m_hWnd;
					nid.uCallbackMessage = WM_TRAY_NOTIFYICACTION;
					nid.hIcon = AfxGetApp()->LoadIconW(IDR_MAINFRAME);
					lstrcpy(nid.szInfoTitle, L"문서로 위장한 실행 파일을 발견하였습니다.");
					lstrcpy(nid.szInfo, path);
					::Shell_NotifyIcon(NIM_MODIFY, &nid);
					break;
				}
				AfxTrace(TEXT("Find Ext : %ws\n"), tmp_ext);
				tmp.Replace(tmp_ext, L"");
				tmp.Trim();
			}
		}
		if (IsOfficeFile(ext)) {
			CString strInsQuery;
			int nospear = 2, zoneid = 0, serverity = TYPE_LOCAL;
			if (bNTFS && nospear_ptr->HasZoneIdentifierADS(path)) {
				nospear = 1;
				zoneid = 3;
				serverity = TYPE_SUSPICIOUS;
			}
			strInsQuery.Format(TEXT("INSERT INTO NOSPEAR_LocalFileList(FilePath, ZoneIdentifier, ProcessName, NOSPEAR, DiagnoseDate, Hash, Serverity, FileType) VALUES ('%ws','%d','No-Spear Client','%d','-', '-', '%d','DOCUMENT');"), path, zoneid, nospear, serverity);
			int rc = fileViewerDB->ExecuteSqlite(strInsQuery);
			if (rc == 0)
				count++;
			AfxTrace(path + "\n");
		}
	}
	CString tmp;
	tmp.Format(TEXT("새로운 %d개의 문서가 LocalFileListDB에 추가되었습니다.\n"), count);
	AfxTrace(tmp);
}
int FILELISTVIEWER::CompareItem(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) {
	CListCtrl* pList = ((SORTPARAM*)lParamSort)->pList;
	int iSortColumn = ((SORTPARAM*)lParamSort)->iSortColumn;
	bool bSortDirect = ((SORTPARAM*)lParamSort)->bSortDirect;
	CString strItem1 = pList->GetItemText(lParam1, iSortColumn);
	CString strItem2 = pList->GetItemText(lParam2, iSortColumn);
	return	bSortDirect ? strcmp(LPSTR(LPCTSTR(strItem1)), LPSTR(LPCTSTR(strItem2))) : -strcmp(LPSTR(LPCTSTR(strItem1)), LPSTR(LPCTSTR(strItem2)));
}

void FILELISTVIEWER::OnHdnItemclickFilelistctrl(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	int nColumn = pNMLV->iItem;

	for (int i = 0; i < (filelistbox.GetItemCount()); i++) {
		filelistbox.SetItemData(i, i);
	}

	bAscending = !bAscending;

	SORTPARAM sortparams;
	sortparams.pList = &filelistbox;
	sortparams.iSortColumn = nColumn;
	sortparams.bSortDirect = bAscending;

	filelistbox.SortItems(&CompareItem, (LPARAM)&sortparams);
	*pResult = 0;
}

void FILELISTVIEWER::OnNMDblclkFilelistctrl(NMHDR* pNMHDR, LRESULT* pResult){
	//List Control을 더블클릭하면 파일탐색기로 파일 위치로 이동해줌
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int row = pNMItemActivate->iItem;
	if (row != -1) {
		CString filepath = filelistbox.GetItemText(row, 8);
		ShellExecute(NULL, _T("open"), _T("explorer"), _T("/select,") + filepath, NULL, SW_SHOW);
	}
	//메시지박스로 디테일하게 보여주는 것도 좋을 듯
	//아니면 없애도 됨.
	*pResult = 0;
}

void FILELISTVIEWER::OnCbnSelchangeCombo1(){
	int index = file_check_combo.GetCurSel();
	if (index != CB_ERR) {
		for (int i = 0; i < filelistbox.GetItemCount(); i++)
			filelistbox.SetCheck(i, FALSE);
		//CString str;
		//file_check_combo.GetLBText(index, str);
		switch (index){
			case 0:
				//전체 문서 선택
				for (int i = 0; i < filelistbox.GetItemCount(); i++)
					filelistbox.SetCheck(i, TRUE);
				break;
			case 1:
				//미검사 문서 선택 column 3
				for (int i = 0; i < filelistbox.GetItemCount(); i++) {
					if (filelistbox.GetItemText(i, 4) == L"-")
						filelistbox.SetCheck(i, TRUE);
				}
				break;
			case 2:
				//외부 문서 선택 column 5
				for (int i = 0; i < filelistbox.GetItemCount(); i++) {
					if (filelistbox.GetItemText(i, 2) == L"O")
						filelistbox.SetCheck(i, TRUE);
				}
				break;
			case 3:
				//전체 선택 해제
				for (int i = 0; i < filelistbox.GetItemCount(); i++)
					filelistbox.SetCheck(i, FALSE);
				break;
		default:
			//여긴 올리가 없음
			break;
		}
	}
	
}

void FILELISTVIEWER::OnBnClickeddiagnose(){
	std::vector<CString> files;
	for (int i = 0; i < filelistbox.GetItemCount(); i++) {
		if (filelistbox.GetCheck(i)) {
			files.push_back(filelistbox.GetItemText(i, 8));
		}
	}
	
	std::vector<CString>::iterator it;
	for (it = files.begin(); it != files.end(); it++) {
		AfxTrace(L"Request Diagnose " + * it + L"\n");
		nospear_ptr->request_diagnose_queue.push(*it);
	}
	CString tmp;
	tmp.Format(TEXT("%d개의 문서에 대한 검사를 요청하였습니다."), files.size());
	nospear_ptr->Notification(L"No-Spear 검사 요청", tmp);
	nospear_ptr->AutoDiagnose();
}

HBRUSH FILELISTVIEWER::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor){
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
	hbr = (HBRUSH)m_background;
	return hbr;
}

BOOL FILELISTVIEWER::PreTranslateMessage(MSG* pMsg){
	tooltip.RelayEvent(pMsg);
	return CDialogEx::PreTranslateMessage(pMsg);
}

void FILELISTVIEWER::OnStnClickedrefreshlist(){
	LOCALFILELISTDB row;
	filelist.clear();
	sqlite3_select p_selResult = fileViewerDB->SelectSqlite(L"select * from NOSPEAR_LocalFileList WHERE FileType='DOCUMENT';");
	if (p_selResult.pnRow != 0) {
		for (int i = 1; i <= p_selResult.pnRow; i++)		{
			int colCtr = 0;
			int nCol = 1;
			int cellPosition = (i * p_selResult.pnColumn) + colCtr;

			row.FilePath = SQLITE::Utf8ToCString(p_selResult.pazResult[cellPosition++]);
			row.ZoneIdentifier = stoi(p_selResult.pazResult[cellPosition++]);
			row.ProcessName = SQLITE::Utf8ToCString(p_selResult.pazResult[cellPosition++]);
			row.NOSPEAR = stoi(p_selResult.pazResult[cellPosition++]);
			row.DiagnoseDate = SQLITE::Utf8ToCString(p_selResult.pazResult[cellPosition++]);
			row.Hash = SQLITE::Utf8ToCString(p_selResult.pazResult[cellPosition++]);
			row.Serverity = stoi(p_selResult.pazResult[cellPosition++]);
			row.FileType = SQLITE::Utf8ToCString(p_selResult.pazResult[cellPosition++]);
			row.TimeStamp = SQLITE::Utf8ToCString(p_selResult.pazResult[cellPosition++]);
			filelist.push_back(row);
		}
	}
	OnCheckBoxChange(0);
}

void FILELISTVIEWER::OnCheckBoxChange(UINT nID){
	//체크박스에서 변경사항이 있을 때 마다 여기로 옴
	bool hwp = ((CButton*)GetDlgItem(IDC_CHECK1))->GetCheck();
	ext_filter[L".hwp"] = ((CButton*)GetDlgItem(IDC_CHECK1))->GetCheck();
	ext_filter[L".hwpx"] = ((CButton*)GetDlgItem(IDC_CHECK1))->GetCheck();
	ext_filter[L".doc"] = ((CButton*)GetDlgItem(IDC_CHECK4))->GetCheck();
	ext_filter[L".docx"] = ((CButton*)GetDlgItem(IDC_CHECK4))->GetCheck();
	ext_filter[L".xls"] = ((CButton*)GetDlgItem(IDC_CHECK5))->GetCheck();
	ext_filter[L".xlsx"] = ((CButton*)GetDlgItem(IDC_CHECK5))->GetCheck();
	ext_filter[L".pptx"] = ((CButton*)GetDlgItem(IDC_CHECK3))->GetCheck();
	ext_filter[L".ppsx"] = ((CButton*)GetDlgItem(IDC_CHECK3))->GetCheck();
	ext_filter[L".pdf"] = ((CButton*)GetDlgItem(IDC_CHECK2))->GetCheck();
	filelistbox.DeleteAllItems();
	int count = 0;

	std::vector<LOCALFILELISTDB>::iterator it;
	for (it = filelist.begin(); it != filelist.end(); it++) {
		//체크박스 여부에 따른 필터 구현
		CString ext = PathFindExtension(it->FilePath);
		if (!ext_filter.at(ext))
			continue;
		
		filelistbox.InsertItem(count, PathFindFileName(it->FilePath));
		filelistbox.SetItem(count, 1, LVIF_TEXT, nospear_ptr->GetMsgFromNospear(it->NOSPEAR), 0, 0, 0, NULL);
		filelistbox.SetItem(count, 2, LVIF_TEXT, (it->ZoneIdentifier == 3) ? L"O" : L"X", 0, 0, 0, NULL);
		filelistbox.SetItem(count, 3, LVIF_TEXT, it->ProcessName, 0, 0, 0, NULL);
		filelistbox.SetItem(count, 4, LVIF_TEXT, it->DiagnoseDate, 0, 0, 0, NULL);
		filelistbox.SetItem(count, 5, LVIF_TEXT, it->Hash, 0, 0, 0, NULL);
		filelistbox.SetItem(count, 6, LVIF_TEXT, nospear_ptr->GetMsgFromErrCode(it->Serverity), 0, 0, 0, NULL);
		filelistbox.SetItem(count, 7, LVIF_TEXT, it->TimeStamp, 0, 0, 0, NULL);
		filelistbox.SetItem(count, 8, LVIF_TEXT, it->FilePath, 0, 0, 0, NULL);
	}
}


void FILELISTVIEWER::OnStnClickedrefreshdb() {
	LOCALFILELISTDB row;
	sqlite3_select p_selResult = fileViewerDB->SelectSqlite(L"select count(*) from NOSPEAR_LocalFileList Where FileType='DOCUMENT';");
	if (p_selResult.pnRow != 0) {
		int count = stoi(p_selResult.pazResult[1]);
		if (count == 0) {
			AfxMessageBox(L"프로그램의 최초 실행");
			ScanLocalFile(L"C:\\Users");
		}
		else {
			AfxMessageBox(L"처음은 아니네");
			ScanLocalFile(L"C:\\Users");
		}
	}
	OnStnClickedrefreshlist();
}

void FILELISTVIEWER::OnNMRClickFilelistctrl(NMHDR* pNMHDR, LRESULT* pResult){
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	
	UNREFERENCED_PARAMETER(pNMItemActivate);

	CPoint CurrentPosition;
	::GetCursorPos(&CurrentPosition);


	filelistbox.ScreenToClient(&CurrentPosition);
	select_index = filelistbox.HitTest(CurrentPosition);

	if (-1 == select_index)	{
		// 아이템 영역이 아닌 곳에서 마우스 오른쪽 버튼을 선택한 경우
	}
	else{
		::GetCursorPos(&CurrentPosition);
		CMenu MenuTemp;
		MenuTemp.LoadMenu(IDR_FileViewRMENU);
		CMenu* pContextMenu = MenuTemp.GetSubMenu(0);
		pContextMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, CurrentPosition.x, CurrentPosition.y, this);
	}
	*pResult = 0;
}
void FILELISTVIEWER::OnManu1() {
	//검사요청
	CString filepath = filelistbox.GetItemText(select_index, 8);
	CString tmp;
	tmp.Format(TEXT("파일명 : %ws\n검사를 요청하였습니다."), filepath);
	nospear_ptr->request_diagnose_queue.push(filepath);
	nospear_ptr->Notification(L"No-Spear 검사 요청", tmp);
	nospear_ptr->AutoDiagnose();
}
void FILELISTVIEWER::OnManu2_0() {
	//Set ADS:Zone.Identifier 0
	CString filepath = filelistbox.GetItemText(select_index, 8);
	nospear_ptr->DeleteZoneIdentifierADS(filepath);
}
void FILELISTVIEWER::OnManu2_3() {
	//Set ADS:Zone.Identifier 3
	CString filepath = filelistbox.GetItemText(select_index, 8);
	nospear_ptr->WriteZoneIdentifierADS(filepath, L"No-Spear Client");
}
void FILELISTVIEWER::OnManu3_0() {
	//Set ADS:NOSPEAR 0
	CString filepath = filelistbox.GetItemText(select_index, 8);
	nospear_ptr->WriteNospearADS(filepath, 0);
}
void FILELISTVIEWER::OnManu3_1() {
	//Set ADS:NOSPEAR 1
	CString filepath = filelistbox.GetItemText(select_index, 8);
	nospear_ptr->WriteNospearADS(filepath, 1);
}
void FILELISTVIEWER::OnManu3_2() {
	//Set ADS:NOSPEAR 2
	CString filepath = filelistbox.GetItemText(select_index, 8);
	nospear_ptr->WriteNospearADS(filepath, 2);
}

void FILELISTVIEWER::OnManu4(){
	ShellExecute(NULL, _T("open"), _T("explorer"), _T("/select,") + filelistbox.GetItemText(select_index, 8), NULL, SW_SHOW);
}

void FILELISTVIEWER::OnManu5(){
	CString hash = filelistbox.GetItemText(select_index, 5);
	if (hash != L"-") {
		CString url = L"4nul.org/result?hash=" + hash;
		ShellExecute(this->m_hWnd, TEXT("open"), TEXT("IEXPLORE.EXE"), url, NULL, SW_SHOW);
	}
}
void FILELISTVIEWER::OnManu6() {
	AfxMessageBox(L"6");
}


void FILELISTVIEWER::OnNMClickFilelistctrl(NMHDR* pNMHDR, LRESULT* pResult){
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int row = pNMItemActivate->iItem;
	if (row != -1) {
		filelistbox.SetCheck(row, !filelistbox.GetCheck(row));
	}
	*pResult = 0;
}


void FILELISTVIEWER::OnStnClickedsearch1(){
	CButton* btn = (CButton*)GetDlgItem(IDC_CHECK1);
	btn->SetCheck(!btn->GetCheck());
	OnCheckBoxChange(0);
}

void FILELISTVIEWER::OnStnClickedsearch2(){
	CButton* btn = (CButton*)GetDlgItem(IDC_CHECK2);
	btn->SetCheck(!btn->GetCheck());
	OnCheckBoxChange(0);
}

void FILELISTVIEWER::OnStnClickedsearch3(){
	CButton* btn = (CButton*)GetDlgItem(IDC_CHECK3);
	btn->SetCheck(!btn->GetCheck());
	OnCheckBoxChange(0);
}

void FILELISTVIEWER::OnStnClickedsearch4(){
	CButton* btn = (CButton*)GetDlgItem(IDC_CHECK4);
	btn->SetCheck(!btn->GetCheck());
	OnCheckBoxChange(0);
}

void FILELISTVIEWER::OnStnClickedsearch5(){
	CButton* btn = (CButton*)GetDlgItem(IDC_CHECK5);
	btn->SetCheck(!btn->GetCheck());
	OnCheckBoxChange(0);
}
