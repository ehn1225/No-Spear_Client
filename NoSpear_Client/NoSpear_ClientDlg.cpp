#include "pch.h"
#include "framework.h"
#include "NoSpear_Client.h"
#include "NoSpear_ClientDlg.h"
#include "afxdialogex.h"
#include "NOSPEAR_FILE.h"
#include "LIVEPROTECT.h"
#include "NOSPEAR.h"
#include "FILELISTVIEWER.h"
#define WM_TRAY_NOTIFYICACTION (WM_USER + 10)

namespace fs = std::filesystem;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

struct ST_WSA_INITIALIZER
{
	ST_WSA_INITIALIZER(void)
	{
		InitCommonControls();
		OleInitialize(0);
		WSAData wsaData;
		WSAStartup(MAKEWORD(2, 2), &wsaData);
	}
	~ST_WSA_INITIALIZER(void)
	{
		WSACleanup();
	}
};

static ST_WSA_INITIALIZER g_WsaInitializer;

// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

CNoSpearClientDlg::CNoSpearClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_NOSPEAR_CLIENT_DIALOG, pParent)
	, filename(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CNoSpearClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, label_filename, filename);
}

BEGIN_MESSAGE_MAP(CNoSpearClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(btn_selectfile, &CNoSpearClientDlg::OnBnClickedselectfile)
	ON_BN_CLICKED(btn_uploadfile, &CNoSpearClientDlg::OnBnClickeduploadfile)
	ON_BN_CLICKED(btn_activelive, &CNoSpearClientDlg::OnBnClickedactivelive)
	ON_BN_CLICKED(btn_inactivelive, &CNoSpearClientDlg::OnBnClickedinactivelive)
	ON_BN_CLICKED(IDC_BUTTON1, &CNoSpearClientDlg::OnBnClickedButton1)
	ON_WM_DROPFILES()
	ON_BN_CLICKED(IDC_BUTTON2, &CNoSpearClientDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CNoSpearClientDlg::OnBnClickedButton3)
	ON_MESSAGE(WM_TRAY_NOTIFYICACTION, OnTrayNotifyAction)
	ON_COMMAND(ID_TRAY_EXIT, &CNoSpearClientDlg::OnTrayExit)
	ON_WM_CTLCOLOR()
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CNoSpearClientDlg 메시지 처리기

BOOL CNoSpearClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.
	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	ChangeWindowMessageFilter(0x0049, MSGFLT_ADD);
	ChangeWindowMessageFilter(WM_DROPFILES, MSGFLT_ADD);
	DragAcceptFiles();

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	CFileFind pFind;
	BOOL bRet = pFind.FindFile(L"config.dat");

	if (bRet) {
		//서버 주소 설정 파일이 존재할 경우, 반영해서 NOSPEAR 객체를 생성함
		/*
			config.dat 파일 형식
			첫번째 줄에 서버 주소
			두번째 줄에 port

			[example - config.dat]
			127.0.0.1
			12345
		*/
		AfxTrace(TEXT("[CNoSpearClientDlg::OnInitDialog] 설정 파일 존재\n"));

		std::string ip, port;
		std::ifstream ifs;
		ifs.open("config.dat");
		if (ifs.is_open()) {
			std::getline(ifs, ip);
			std::getline(ifs, port);
		}
		client = new NOSPEAR(ip, atoi(port.c_str()));
	}
	else {
		//설정 파일이 없는 경우 기본 설정으로 진행
		AfxTrace(TEXT("[CNoSpearClientDlg::OnInitDialog] 설정 파일 미존재\n"));
		client = new NOSPEAR();
	}

	ZeroMemory(&nid, sizeof(nid));
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.uID = 0;    // 트레이 구조체 아이디.
	nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
	nid.hWnd = m_hWnd;
	nid.hIcon = AfxGetApp()->LoadIconW(IDR_MAINFRAME);
	nid.uCallbackMessage = WM_TRAY_NOTIFYICACTION;
	lstrcpy(nid.szTip, _T("No-Spear"));
	::Shell_NotifyIcon(NIM_ADD, &nid);
	m_background.CreateSolidBrush(RGB(255, 255, 255));

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

LRESULT CNoSpearClientDlg::OnTrayNotifyAction(WPARAM wParam, LPARAM lParam)
{
	switch (lParam)	{
	case WM_RBUTTONDOWN:
	{
		CPoint ptMouse;
		::GetCursorPos(&ptMouse);

		CMenu menu;
		menu.LoadMenu(IDR_TRAY_MENU);

		CMenu* pMenu = menu.GetSubMenu(0);
		pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, ptMouse.x, ptMouse.y, AfxGetMainWnd());
	}
	break;

	case WM_LBUTTONDBLCLK:
		ShowWindow(SW_SHOW);
	break;
	}

	return 1;
}
void CNoSpearClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX){
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else if (nID == SC_CLOSE) {
		ShowWindow(SW_HIDE);
		ZeroMemory(&nid, sizeof(nid));
		nid.cbSize = sizeof(nid);
		nid.dwInfoFlags = NIIF_INFO;
		nid.uFlags = NIF_MESSAGE | NIF_INFO | NIF_ICON;
		nid.uTimeout = 1000;
		nid.hWnd = AfxGetApp()->m_pMainWnd->m_hWnd;
		nid.uCallbackMessage = WM_TRAY_NOTIFYICACTION;
		nid.hIcon = AfxGetApp()->LoadIconW(IDR_MAINFRAME);
		lstrcpy(nid.szInfoTitle, L"No-Spear 프로그램이 백그라운드 모드로 전환되었습니다.");
		lstrcpy(nid.szInfo, L"프로그램을 종료하려면 작업 표시줄 아이콘을 마우스 오른쪽 버튼으로 클릭하세요.");
		::Shell_NotifyIcon(NIM_MODIFY, &nid);
	}
	else{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CNoSpearClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CNoSpearClientDlg::OnQueryDragIcon(){
	return static_cast<HCURSOR>(m_hIcon);
}
void CNoSpearClientDlg::OnTrayExit(){
	ZeroMemory(&nid, sizeof(nid));
	nid.cbSize = sizeof(nid);
	nid.uID = 0;
	nid.hWnd = GetSafeHwnd();
	::Shell_NotifyIcon(NIM_DELETE, &nid);
	if (fileListViewer != NULL) {
		fileListViewer->EndDialog(0);
		delete(fileListViewer);
	}
	if (client != NULL) delete(client);

	EndDialog(0);
}
void CNoSpearClientDlg::OnBnClickedselectfile(){
	// Manual Diagnose
	// 수동검사의 파일선택 버튼을 눌렀을 때 작동을 구현
	//hwp, hwpx, pdf, doc, docx, xls, xlsx
	CString szFilter = _T("문서 파일 (*.hwp, *.hwpx, *.pdf, *.doc, *.docx, *.xls, *.xlsx) | *.hwp; *.hwpx; *.pdf; *.doc; *.docx; *.xls; *.xlsx|");
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, szFilter);

	if (IDOK == dlg.DoModal())	{
		filepath = dlg.GetPathName();
		filename = dlg.GetFileName();
		UpdateData(FALSE);
	}
}

void CNoSpearClientDlg::OnBnClickeduploadfile(){
	// Manual Diagnose
	// 수동검사의 검사 버튼을 눌렀을 때 작동을 구현
	NOSPEAR_FILE file(filepath);
	client->Diagnose(file);
}


void CNoSpearClientDlg::OnBnClickedactivelive(){
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	client->ActivateLiveProtect(TRUE);
	
}


void CNoSpearClientDlg::OnBnClickedinactivelive(){
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	client->ActivateLiveProtect(FALSE);
}

void CNoSpearClientDlg::OnBnClickedButton1(){
	fileListViewer = new FILELISTVIEWER();
	fileListViewer->DoModal();
}

void CNoSpearClientDlg::OnDropFiles(HDROP hDropInfo){
	wchar_t buffer[512] = {0, };
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	//UINT count = DragQueryFile(hDropInfo, 0xFFFFFFFF, buffer, 512);
	DragQueryFile(hDropInfo, 0, buffer, 512);
	CString tmp = CString(buffer);
	filepath = tmp;
	filename = PathFindFileName(tmp);
	UpdateData(FALSE);
	DragFinish(hDropInfo);
	CDialogEx::OnDropFiles(hDropInfo);
}

NOSPEAR* CNoSpearClientDlg::GetClientPtr(){
	return client;
}
#define UPDATECHECK_BROWSER_STRING _T("No-Spear Update")

void CNoSpearClientDlg::OnBnClickedButton2(){
	CWaitCursor wait;
	HINTERNET hInet = InternetOpen(UPDATECHECK_BROWSER_STRING, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, NULL);
	HINTERNET hUrl = InternetOpenUrl(hInet, CString("http://localhost/"), NULL, -1L,
		INTERNET_FLAG_RELOAD | INTERNET_FLAG_PRAGMA_NOCACHE |
		INTERNET_FLAG_NO_CACHE_WRITE | WININET_API_FLAG_ASYNC, NULL);
	if (hUrl){
		AfxTrace(L"in\n");
		char szBuffer[512] = {0, };
		DWORD dwRead;
		if (InternetReadFile(hUrl, szBuffer, sizeof(szBuffer), &dwRead) && dwRead > 0)
		{

				CString fileversion(szBuffer);
				AfxTrace(fileversion);

		}
		InternetCloseHandle(hUrl);
	}
	else {
		AfxTrace(L"nope\n");
	}
	InternetCloseHandle(hInet);


}

void CNoSpearClientDlg::OnBnClickedButton3(){
	//CString strInsQuery;
	//CString type = L"DOCUMENT";
	//int zoneid = 3;
	//int nospear = 1;

	//strInsQuery.Format(TEXT("REPLACE INTO NOSPEAR_LocalFileList(FilePath, ZoneIdentifier, ProcessName, NOSPEAR, DiagnoseDate, Serverity, FileType) VALUES ('%ws','%d','%ws','%d','-','0','%ws');"), filepath, zoneid, L"sfjldsjflsjfsdldjs.exe", nospear, type);

	//AfxMessageBox(strInsQuery);

	//SQLITE temp;
	//if (temp.DatabaseOpen(L"NOSPEAR")) {
	//	AfxTrace(TEXT("[LIVEPROTECT::LIVEPROTECT] Can't Create NOSPEAR_HISTORY DataBase.\n"));
	//	return;
	//}
	//
	//sqlite3_select p_selResult = temp.SelectSqlite(L"select NOSPEAR, ZoneIdentifier, ProcessName from NOSPEAR_LocalFileList WHERE FilePath='" + filepath + L"' LIMIT 1;");
	//if (p_selResult.pnRow != 0) {
	//	//std::string sel1 = p_selResult.pazResult[3];
	//	//std::string sel2 = p_selResult.pazResult[4];
	//	//std::string sel3 = p_selResult.pazResult[5];
	//	int nospear = stoi(p_selResult.pazResult[3]);
	//	int zone = stoi(p_selResult.pazResult[4]);
	//	CString ProcessName(p_selResult.pazResult[5]);
	//	AfxTrace(TEXT("FIND Local DB ADS : %d, Zone : %d\, ProcessName : %s\n"), nospear, zone, ProcessName);
	//	if (zone != 0)
	//		AfxTrace(TEXT("Attatch Zone.Identifier Zone : %d\, ProcessName : %s\n"), zone, ProcessName);
	//}




	////CString strInsQuery = _T("Insert into helloworld VALUES( NULL,'" + filepath + "');");
	////int rc = database.ExecuteSqlite(strInsQuery);

	//sqlite3_select p_selResult = database.SelectSqlite(L"select * from NOSPEAR_HISTORY ;");
	//if (p_selResult.pnRow != 0) {
	//	for (int i = 0; i <= p_selResult.pnRow; ++i)
	//	{
	//		int colCtr = 0;
	//		int nCol = 1;
	//		int cellPosition = (i * p_selResult.pnColumn) + colCtr;

	//		std::string sel1 = p_selResult.pazResult[cellPosition++];
	//		CString zSeq1(sel1.c_str());
	//		std::string sel2 = p_selResult.pazResult[cellPosition++];
	//		CString zSeq2(sel2.c_str());
	//		std::string sel3 = p_selResult.pazResult[cellPosition++];
	//		CString zSeq3(sel3.c_str());
	//		std::string sel4 = p_selResult.pazResult[cellPosition++];
	//		CString zSeq4(sel4.c_str());
	//		AfxTrace(zSeq1 + L", " + zSeq2 + L", " + zSeq3 + L", " + zSeq4 + L"\n");

	//		if (i == 0) {
	//			continue;
	//		}

	//	}

	//}
	// 
	//exe한정 실행
	DeleteFile(filepath + L":Zone.Identifier");


}

HBRUSH CNoSpearClientDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor){
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
	hbr = (HBRUSH)m_background;
	return hbr;
}

void CNoSpearClientDlg::OnClose(){
	//이제 필요 없을 듯
	ZeroMemory(&nid, sizeof(nid));
	nid.cbSize = sizeof(nid);
	nid.uID = 0;
	nid.hWnd = GetSafeHwnd();
	::Shell_NotifyIcon(NIM_DELETE, &nid);
	CDialogEx::OnClose();
}
