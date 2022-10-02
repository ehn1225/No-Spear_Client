
// NoSpear_ClientDlg.cpp: 구현 파일
#include "pch.h"
#include "framework.h"
#include "NoSpear_Client.h"
#include "NoSpear_ClientDlg.h"
#include "afxdialogex.h"
#include "NOSPEAR_FILE.h"
#include "NOSPEAR.h"
#include "LIVEPROTECT.h"
namespace fs = std::filesystem;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

NOSPEAR* client = NULL;

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
	DDX_Control(pDX, FileList, m_ctrlFileList);
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
	ON_BN_CLICKED(IDC_BUTTON2, &CNoSpearClientDlg::OnBnClickedButton2)
	ON_NOTIFY(HDN_ITEMCLICK, 0, &CNoSpearClientDlg::OnHdnItemclickFilelist)
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

	//Listview 열 초기화
	m_ctrlFileList.InsertColumn(0, L"파일명", LVCFMT_LEFT, 100, -1);
	m_ctrlFileList.InsertColumn(1, L"파일경로", LVCFMT_LEFT, 100, -1);
	m_ctrlFileList.InsertColumn(2, L"외부파일", LVCFMT_LEFT, 100, -1);
	m_ctrlFileList.InsertColumn(3, L"검사여부", LVCFMT_LEFT, 100, -1);
	m_ctrlFileList.InsertColumn(4, L"확장자", LVCFMT_LEFT, 100, -1);

	PrintFolder(L"C:\\Users");


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

void CNoSpearClientDlg::OnBnClickeduploadfile()
{
	// Manual Diagnose
	// 수동검사의 검사 버튼을 눌렀을 때 작동을 구현
	CFileFind pFind;
	UpdateData(TRUE);
	BOOL bRet = pFind.FindFile(filepath);

	if (!bRet) {
		AfxMessageBox(_T("파일을 찾을 수 없습니다"));
		AfxTrace(TEXT("[CNoSpearClientDlg::OnBnClickeduploadfile] 파일이 유효하지 않음\n"));
		return;
	}

	int returncode = 0;
	CString count;

	returncode = client->Fileupload(NOSPEAR_FILE(filepath));
	for (int i = 0; i < 3; i++) {
		if (returncode < 0) {
			AfxMessageBox(client->GetErrorMsg());
			return;
		}

		switch (returncode) {
			case TYPE_NORMAL:
				AfxMessageBox(_T("분석 결과 : 정상 파일"));
				break;
			case TYPE_MALWARE:
				AfxMessageBox(_T("분석 결과 : 악성 파일"));
				break;
			case TYPE_SUSPICIOUS:
				AfxMessageBox(_T("분석 결과 : 악성 의심 파일"));
				break;
			case TYPE_UNEXPECTED://엔진도 모르겠다 진짜 Unknown
				AfxMessageBox(_T("분석 결과 : 알 수 없는 파일"));
				break;
			case TYPE_NOFILE:
				AfxMessageBox(_T("분석 결과 : 문서 파일이 아님"));
				break;
			case TYPE_RESEND:
				count.Format(L"재시도 횟수 : %d회/%d회", i + 1, 3);
				if (IDYES == AfxMessageBox(L"파일을 업로드하는 중 오류가 발생하였습니다.\n다시 시도 하시겠습니까?\n" + count, MB_YESNO | MB_ICONWARNING)) {
					returncode = client->Fileupload(NOSPEAR_FILE(filepath));
					continue;
				}
				break;
			default:
				AfxMessageBox(_T("Unknown Response"));
				break;
		}
		break;
	}
}


void CNoSpearClientDlg::OnBnClickedactivelive(){
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	client->ActivateLiveProtect(TRUE);
	
}


void CNoSpearClientDlg::OnBnClickedinactivelive(){
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	client->ActivateLiveProtect(FALSE);
}


bool CNoSpearClientDlg::Has_ADS(CString filepath) {
	//filepath로 주어진 파일의 ADS를 확인해보고, Zone.Identifier가 있으면 true, 없으면 false 리턴
	WIN32_FIND_STREAM_DATA fsd;
	HANDLE hFind = NULL;

	try {
		hFind = ::FindFirstStreamW(filepath, FindStreamInfoStandard, &fsd, 0);
		if (hFind == INVALID_HANDLE_VALUE) throw ::GetLastError();

		for (;;) {
			CString tmp;
			tmp.Format(TEXT("%s"), fsd.cStreamName);
			if (tmp == L":Zone.Identifier:$DATA") {
				return true;
			}
			if (!::FindNextStreamW(hFind, &fsd)) {
				DWORD dr = ::GetLastError();
				if (dr != ERROR_HANDLE_EOF) throw dr;
				break;
			}
		}
	}
	catch (DWORD err) {
		AfxTrace(TEXT("Error! Windows error code: %u\n", err));
	}

	if (hFind != NULL) ::FindClose(hFind);
	
	return false;
}

void CNoSpearClientDlg::PrintFolder(CString folderpath) {
	//매개변수로 입력된 폴더 경로를 재귀 탐색하여 문서 파일을 Listview에 출력해줌.
	//filesystem test, https://stackoverflow.com/questions/62988629/c-stdfilesystemfilesystem-error-exception-trying-to-read-system-volume-inf

	string strfilepath = string(CT2CA(folderpath));
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

	for (; iter != end_iter; iter.increment(ec)) {
		if (ec) {
			continue;
		}
		string ext = iter->path().extension().string();
		//One-Drive상의 일부 폴더 탐색 안되는 문제 있음
		//*.hwp; *.hwpx; *.pdf; *.doc; *.docx; *.xls; *.xlsx;
		if (ext == ".hwp" || ext == ".hwpx" || ext == ".pdf" || ext == ".doc" || ext == ".docx" || ext == ".xls" || ext == ".xlsx") {
			CString path = CString(iter->path().string().c_str());
			AfxTrace(path + "\n");
			//추후 Listview 입력부는 따로 분리해야 함.
			//DB와 연동해야하기 때문임
			m_ctrlFileList.InsertItem(count, iter->path().filename().c_str());
			m_ctrlFileList.SetItem(count, 1, LVIF_TEXT, path, 0, 0, 0, NULL);
			m_ctrlFileList.SetItem(count, 2, LVIF_TEXT, (bNTFS) ? ((Has_ADS(path) == true) ? L"외부" : L"내부") : L"Not NTFS", 0, 0, 0, NULL);
			m_ctrlFileList.SetItem(count, 3, LVIF_TEXT, L"검사안됨", 0, 0, 0, NULL);
			m_ctrlFileList.SetItem(count, 4, LVIF_TEXT, CString((ext.substr(1,4)).c_str()), 0, 0, 0, NULL);

			count++;
		}
	}
	CString tmp;
	tmp.Format(TEXT("Folder Search Complete. Find %d"), count);
	AfxTrace(tmp);

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

	//NTFS 파일시스템인지 확인(ADS 전제 조건)


	
}


void CNoSpearClientDlg::OnBnClickedButton2(){
	//폴더 선택 검사 기능
	CFolderPickerDialog Picker(_T("C:\\Users"), OFN_FILEMUSTEXIST, NULL, 0);
	if (Picker.DoModal() == IDOK){
		CString strFolderPath = Picker.GetPathName();
		//PC에서 다운로드한 파일을 USB에 복사할 경우, ADS는 복사가 안되기에 내부로 출력될 수 있음.
		PrintFolder(strFolderPath);
	}
}
int CNoSpearClientDlg::CompareItem(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort){
	CListCtrl* pList = ((SORTPARAM*)lParamSort)->pList;
	int iSortColumn = ((SORTPARAM*)lParamSort)->iSortColumn;
	bool bSortDirect = ((SORTPARAM*)lParamSort)->bSortDirect;
	CString strItem1 = pList->GetItemText(lParam1, iSortColumn);
	CString strItem2 = pList->GetItemText(lParam2, iSortColumn);
	return	bSortDirect ? strcmp(LPSTR(LPCTSTR(strItem1)), LPSTR(LPCTSTR(strItem2))) : -strcmp(LPSTR(LPCTSTR(strItem1)), LPSTR(LPCTSTR(strItem2)));
}

void CNoSpearClientDlg::OnHdnItemclickFilelist(NMHDR* pNMHDR, LRESULT* pResult){
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	int nColumn = pNMLV->iItem;

	for (int i = 0; i < (m_ctrlFileList.GetItemCount()); i++) {
		m_ctrlFileList.SetItemData(i, i);
	}

	bAscending = !bAscending;

	SORTPARAM sortparams;
	sortparams.pList = &m_ctrlFileList;
	sortparams.iSortColumn = nColumn;
	sortparams.bSortDirect = bAscending;

	m_ctrlFileList.SortItems(&CompareItem, (LPARAM)&sortparams);
	*pResult = 0;
}

