﻿
// NoSpear_ClientDlg.cpp: 구현 파일
#include "pch.h"
#include "framework.h"
#include "NoSpear_Client.h"
#include "NoSpear_ClientDlg.h"
#include "afxdialogex.h"
#include <wininet.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <process.h>
#include <commctrl.h>
#include "ssl.h"

SOCKET s;
SSL_SOCKET* sx = 0;
sockaddr_in dA, aa;
int slen = sizeof(sockaddr_in);

#pragma warning(disable:4996)
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Crypt32.lib")
#pragma comment(lib,"Secur32.lib")
#pragma comment(lib, "wininet.lib")
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
	, manual_file_path(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CNoSpearClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, edit_filepath, manual_file_path);
}

BEGIN_MESSAGE_MAP(CNoSpearClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CNoSpearClientDlg::OnBnClickedOk)
	ON_BN_CLICKED(btn_selectfile, &CNoSpearClientDlg::OnBnClickedselectfile)
	ON_BN_CLICKED(btn_uploadfile, &CNoSpearClientDlg::OnBnClickeduploadfile)
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



void CNoSpearClientDlg::OnBnClickedOk()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CDialogEx::OnOK();
}

void CNoSpearClientDlg::OnBnClickedselectfile()
{
	// Manual Diagnose
	// 수동검사의 파일선택 버튼을 눌렀을 때 작동을 구현

	CString szFilter = _T("문서 파일 (*.doc, *.hwp, *.xls, *.pdf, *.ppt, *.txt) | *.doc; *.hwp; *.xls; *.pdf; *.ppt; *.txt|");
	CFileDialog dlg(TRUE, _T("*.dat"), NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, szFilter);

	if (IDOK == dlg.DoModal())	{
		CString pathName = dlg.GetPathName();
		UpdateData(TRUE);
		manual_file_path = pathName;
		UpdateData(FALSE);
	}
}

void CNoSpearClientDlg::OnBnClickeduploadfile()
{
	// Manual Diagnose
	// 수동검사의 검사 버튼을 눌렀을 때 작동을 구현

	CFileFind pFind;
	UpdateData(TRUE);
	BOOL bRet = pFind.FindFile(manual_file_path);

	if (!bRet) {
		AfxMessageBox(_T("파일을 찾을 수 없습니다"));
		return;
	}

	//SSL Socket Send
	//https://www.codeproject.com/Articles/24379/SSL-Convert-your-Plain-Sockets-to-SSL-Sockets-in-a

	hostent* hp;
	unsigned long inaddr;

	memset(&dA, 0, sizeof(dA));
	dA.sin_family = AF_INET;
	inaddr = inet_addr("127.0.0.1"); //Server IPv4 Address
	if (inaddr != INADDR_NONE)
		memcpy(&dA.sin_addr, &inaddr, sizeof(inaddr));
	else
	{
		hp = gethostbyname("localhost"); //Server Domain Name
		if (!hp)
		{
			AfxMessageBox(_T("Remote System not found!"));
			return;
		}
		memcpy(&dA.sin_addr, hp->h_addr, hp->h_length);
	}
	dA.sin_port = htons(42524); //Server Listen Port
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(s, (sockaddr*)&dA, slen) < 0)
	{
		AfxMessageBox(_T("Cannot connect"));
		return;
	}

	getpeername(s, (sockaddr*)&aa, &slen);

	sx = new SSL_SOCKET(s, 0);
	sx->ClientInit();

	FILE* fp = _wfopen(manual_file_path, L"rb");
	if (fp == NULL) {
		AfxMessageBox(_T("파일열기 실패"));
		return;
	}

	//Issue : 서버에서 바로 처음 세션이 끊기는 현상
	//이 이슈 CMD 말고 VS에서 돌리니까 발생 안함... 뭐지?
	char buffer[1024];
	int read_size = 0;
	while ((read_size = fread(buffer, 1, 1024, fp)) != 0) {
		sx->s_ssend(buffer, read_size);
	}
	fclose(fp);
	closesocket(s);
	delete(sx);
}