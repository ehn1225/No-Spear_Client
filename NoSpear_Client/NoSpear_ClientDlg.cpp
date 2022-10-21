#include "pch.h"
#include "framework.h"
#include "NoSpear_Client.h"
#include "NoSpear_ClientDlg.h"
#include "afxdialogex.h"
#include "NOSPEAR_FILE.h"
#include "LIVEPROTECT.h"
#include "NOSPEAR.h"
#include "FILELISTVIEWER.h"

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


// CNoSpearClientDlg 대화 상자



CNoSpearClientDlg::CNoSpearClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_NOSPEAR_CLIENT_DIALOG, pParent)
	, filename(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON1);
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

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CNoSpearClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
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
HCURSOR CNoSpearClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
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
	UpdateData(TRUE);
	DIAGNOSE_RESULT diagnose_resut;
	diagnose_resut = client->SingleDiagnose(filepath);
	AfxMessageBox(diagnose_resut.result_msg);
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
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	//Create Time
	//UpdateData(TRUE);
	//struct __stat64 buffer;
	//_wstat64(filepath, &buffer);
	//struct tm* timeinfo = localtime(&buffer.st_ctime); // or gmtime() depending on what you want
	//CString tmp = CString(asctime(timeinfo));
	////AfxMessageBox(tmp);

	//time_t timeinfo1 = buffer.st_ctime; // or gmtime() depending on what you want
	//tmp.Format(TEXT("time : %ld"), timeinfo1);
	//AfxMessageBox(tmp);

	/*time_t result = time(nullptr);
	tmp.Format(TEXT("time : %ld"), result);
	AfxMessageBox(tmp);*/

	//SQLite 테스트

	FILELISTVIEWER dlg;
	dlg.DoModal();

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


void CNoSpearClientDlg::OnBnClickedButton2(){
	//bool HasADS = false;
	//WIN32_FIND_STREAM_DATA fsd;
	//HANDLE hFind = NULL;
	//try {
	//	hFind = ::FindFirstStreamW(filepath, FindStreamInfoStandard, &fsd, 0);
	//	if (hFind == INVALID_HANDLE_VALUE) throw ::GetLastError();

	//	for (;;) {
	//		CString tmp;
	//		tmp.Format(TEXT("%s"), fsd.cStreamName);
	//		if (tmp == L":NOSPEAR:$DATA") {
	//			HasADS = true;
	//		}
	//		if (!::FindNextStreamW(hFind, &fsd)) {
	//			DWORD dr = ::GetLastError();
	//			if (dr != ERROR_HANDLE_EOF) throw dr;
	//			break;
	//		}
	//	}
	//}
	//catch (DWORD err) {
	//	AfxTrace(TEXT("[LIVEPROTECT::IsMaliciousLocal] Find ADS, Windows error code: %u\n", err));
	//}
	//if (hFind != NULL)
	//	::FindClose(hFind);

	//if (HasADS == false)
	//	AfxMessageBox(L"없어용");
	//	return;

	////2. ADS Value 확인
	//CStdioFile ads_stream;
	//CFileException e;
	//if (!ads_stream.Open(filepath + L":NOSPEAR", CFile::modeRead, &e)) {
	//	e.ReportError();
	//}
	//CString str;
	//ads_stream.ReadString(str);
	//bool result = (str == L"1");
	CStdioFile ads_stream;
	CFileException e;
	if (!ads_stream.Open(filepath + L":Zone.Identifier", CStdioFile::modeCreate | CStdioFile::modeWrite, &e)) {
		return;
	}
	CString processName;
	CString tmp = L"Helloworld.exe";
	processName.Format(TEXT("ProcessName=%ws"), tmp);
	ads_stream.WriteString(L"[ZoneTransfer]\n");
	ads_stream.WriteString(L"ZoneId=3\n");
	ads_stream.WriteString(L"ADS Appended By No-Spear Client\n");
	ads_stream.WriteString(processName);

	return;
}
