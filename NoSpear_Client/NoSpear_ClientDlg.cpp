#include "pch.h"
#include "framework.h"
#include "NoSpear_Client.h"
#include "NoSpear_ClientDlg.h"
#include "afxdialogex.h"
#include "NOSPEAR_FILE.h"
#include "LIVEPROTECT.h"
#include "NOSPEAR.h"
#include "FILELISTVIEWER.h"
#include "MainView.h"
#include "SettingView.h"
#include "QuarantineView.h"
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
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CNoSpearClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, logo_frame, logo_ctl);
}

BEGIN_MESSAGE_MAP(CNoSpearClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_MESSAGE(WM_TRAY_NOTIFYICACTION, OnTrayNotifyAction)
	ON_COMMAND(ID_TRAY_EXIT, &CNoSpearClientDlg::OnTrayExit)
	ON_WM_CTLCOLOR()
	ON_STN_CLICKED(IDC_fileviewer, &CNoSpearClientDlg::OnStnClickedfileviewer)
	ON_STN_CLICKED(IDC_home, &CNoSpearClientDlg::OnStnClickedhome)
	ON_STN_CLICKED(IDC_setting, &CNoSpearClientDlg::OnStnClickedsetting)
	ON_STN_CLICKED(IDC_quarantine, &CNoSpearClientDlg::OnStnClickedquarantine)
	ON_STN_CLICKED(logo_frame, &CNoSpearClientDlg::OnStnClickedframe)
END_MESSAGE_MAP()

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
	clientThreadStatus = true;

	if (bRet) {
		//서버 주소 설정 파일이 존재할 경우, 반영해서 NOSPEAR 객체를 생성함
		/*
			config.dat 파일 형식
			첫번째 줄에 서버 주소
			두번째 줄에 백신 서버 port
			세번째 줄에 업데이트 서버 port

			[example - config.dat]
			127.0.0.1
			12345
			12345
		*/
		AfxTrace(TEXT("[CNoSpearClientDlg::OnInitDialog] 설정 파일 존재\n"));

		std::string ip, port, port2;
		std::ifstream ifs;
		ifs.open("config.dat");
		if (ifs.is_open()) {
			std::getline(ifs, ip);
			std::getline(ifs, port);
			std::getline(ifs, port2);
		}
		client = new NOSPEAR(ip, atoi(port.c_str()), atoi(port2.c_str()));
		clientThread = AfxBeginThread(ClientThreadFunc, new STPARAM(client));

	}
	else {
		//설정 파일이 없는 경우 기본 설정으로 진행
		AfxTrace(TEXT("[CNoSpearClientDlg::OnInitDialog] 설정 파일 미존재\n"));
		client = new NOSPEAR();
		clientThread = AfxBeginThread(ClientThreadFunc, new STPARAM(client));

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

	AllocForms();

	tooltip.Create(this);
	tooltip.AddTool(GetDlgItem(IDC_home), L"수동검사");
	tooltip.AddTool(GetDlgItem(IDC_fileviewer), L"문서 탐색");
	tooltip.AddTool(GetDlgItem(IDC_quarantine), L"검역소");
	tooltip.AddTool(GetDlgItem(IDC_setting), L"업데이트 및 설정");
	tooltip.AddTool(GetDlgItem(logo_frame), L"No-Spear 홈페이지 이동");

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

LRESULT CNoSpearClientDlg::OnTrayNotifyAction(WPARAM wParam, LPARAM lParam){
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

void CNoSpearClientDlg::OnPaint(){
	if (IsIconic())	{
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
	else{
		CDialogEx::OnPaint();
	}
}

HCURSOR CNoSpearClientDlg::OnQueryDragIcon(){
	return static_cast<HCURSOR>(m_hIcon);
}
void CNoSpearClientDlg::OnTrayExit(){
	EndDialog(0);
}

void CNoSpearClientDlg::AllocForms(){
	CCreateContext context;
	ZeroMemory(&context, sizeof(context));

	CRect rectOfPanelArea;

	GetDlgItem(IDC_FormViewer)->GetWindowRect(&rectOfPanelArea);
	ScreenToClient(&rectOfPanelArea);

	m_pForm1 = new MainView();
	m_pForm1->Create(NULL, NULL, WS_CHILD | WS_VSCROLL | WS_HSCROLL, rectOfPanelArea, this, IDD_MainView, &context);
	m_pForm1->OnInitialUpdate();
	m_pForm1->ShowWindow(SW_SHOW);

	m_pForm2 = new SettingView();
	m_pForm2->Create(NULL, NULL, WS_CHILD | WS_VSCROLL | WS_HSCROLL, rectOfPanelArea, this, IDD_SettingView, &context);
	m_pForm2->OnInitialUpdate();
	m_pForm2->ShowWindow(SW_HIDE);

	m_pForm3 = new QuarantineView();
	m_pForm3->Create(NULL, NULL, WS_CHILD | WS_VSCROLL | WS_HSCROLL, rectOfPanelArea, this, IDD_SettingView, &context);
	m_pForm3->OnInitialUpdate();
	m_pForm3->ShowWindow(SW_HIDE);

	fileListViewer = new FILELISTVIEWER();
	fileListViewer->Create(IDD_FILELISTVIEWDIALOG);

	GetDlgItem(IDC_FormViewer)->DestroyWindow();
}

void CNoSpearClientDlg::OnDestroy(){
	CDialogEx::OnDestroy();

	if (m_pForm1 != NULL)	{
		m_pForm1->DestroyWindow();
	}
	if (m_pForm2 != NULL)	{
		m_pForm2->DestroyWindow();
	}	
	if (m_pForm3 != NULL)	{
		m_pForm3->DestroyWindow();
	}
	if (fileListViewer != NULL)	{
		fileListViewer->DestroyWindow();
	}

	ZeroMemory(&nid, sizeof(nid));
	nid.cbSize = sizeof(nid);
	nid.uID = 0;
	nid.hWnd = GetSafeHwnd();
	::Shell_NotifyIcon(NIM_DELETE, &nid);

	if (clientThread != NULL) {
		clientThreadStatus = false;
		clientThread->ExitInstance();
		delete clientThread;
	}
	
	if (client != NULL) delete(client);
}

NOSPEAR* CNoSpearClientDlg::GetClientPtr(){
	return client;
}

HBRUSH CNoSpearClientDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor){
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
	hbr = (HBRUSH)m_background;
	return hbr;
}

void CNoSpearClientDlg::OnStnClickedfileviewer(){
	fileListViewer->ShowWindow(SW_SHOW);
}

void CNoSpearClientDlg::OnStnClickedhome(){
	ShowForm(0);
}

void CNoSpearClientDlg::OnStnClickedsetting(){
	ShowForm(1);
}
void CNoSpearClientDlg::OnStnClickedquarantine() {
	ShowForm(2);
	m_pForm3->RefrestList();
}

void CNoSpearClientDlg::ShowForm(int idx){
	switch (idx){
	case 0:
		m_pForm1->ShowWindow(SW_SHOW);
		m_pForm2->ShowWindow(SW_HIDE);
		m_pForm3->ShowWindow(SW_HIDE);
		break;
	case 1:
		m_pForm1->ShowWindow(SW_HIDE);
		m_pForm2->ShowWindow(SW_SHOW);
		m_pForm3->ShowWindow(SW_HIDE);
		break;
	case 2:
		m_pForm1->ShowWindow(SW_HIDE);
		m_pForm2->ShowWindow(SW_HIDE);
		m_pForm3->ShowWindow(SW_SHOW);
		break;
	}
}

void CNoSpearClientDlg::OnStnClickedframe(){
	ShellExecute(this->m_hWnd, TEXT("open"), TEXT("IEXPLORE.EXE"), L"http://4nul.org/", NULL, SW_SHOW);
}

UINT CNoSpearClientDlg::ClientThreadFunc(LPVOID pParam) {
	STPARAM param = *(STPARAM*)pParam;
	NOSPEAR* nospear = param.nospear;
	const int REFRESH_DB_DELAY = 300; //5분 주기로 반복
	time_t last_update = 0;
	while (clientThreadStatus) {
		time_t now = time(NULL);
		if (now - last_update >= REFRESH_DB_DELAY) {
			//DB 동기화 및 정리
			nospear->ScanLocalFile(L"C:\\Users");
			nospear->ScanFileAvailability();
			//AfxTrace(TEXT("Tick\n"));
			last_update = now;
		}

		//Zone.Identifier 붙혀주는 친구
		nospear->AttachADSOther();
		
		//검사 요청 큐에 들어온 애들 검사 처리
		if (!nospear->IsQueueEmpty()) {
			nospear->AutoDiagnose();
		}

		AfxTrace(L"Thread Loop\n");
		Sleep(500);
	}
	return 0;
}

BOOL CNoSpearClientDlg::PreTranslateMessage(MSG* pMsg){
	tooltip.RelayEvent(pMsg);
	return CDialogEx::PreTranslateMessage(pMsg);
}
