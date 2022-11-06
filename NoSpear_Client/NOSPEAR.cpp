#include "pch.h"
#include "NOSPEAR_FILE.h"
#include "LIVEPROTECT.h"
#include "NOSPEAR.h"
#include "resource.h"
#include "SQLITE.h"

#pragma warning(disable:4996)
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Crypt32.lib")
#pragma comment(lib,"Secur32.lib")
#pragma comment(lib,"wininet.lib")
#pragma comment(lib,"fltLib.lib")
#pragma comment(lib,"sqlite3.lib")
#define WM_TRAY_NOTIFYICACTION (WM_USER + 10)

SOCKET s;
sockaddr_in dA, aa;
int slen = sizeof(sockaddr_in);

void NOSPEAR::Deletefile(CString filepath){
	CFileFind pFind;
	BOOL bRet = pFind.FindFile(filepath);
	if (bRet == TRUE) {
		if (DeleteFile(filepath) == TRUE) {
			AfxMessageBox(_T("삭제 완료"));
		}
	}
}

bool NOSPEAR::FileUpload(NOSPEAR_FILE& file){
	AfxTrace(TEXT("[NOSPEAR::FileUpload] 파일 업로드 시작\n"));
	AfxTrace(TEXT("[NOSPEAR::FileUpload] name : " + file.Getfilename() + "\n"));
	AfxTrace(TEXT("[NOSPEAR::FileUpload] path : " + file.Getfilepath() + "\n"));
	AfxTrace(TEXT("[NOSPEAR::FileUpload] hash : " + CString(file.Getfilehash()) + "\n"));

	//validation 호출
	if (file.Checkvalidation() == false) {
		AfxTrace(TEXT("[NOSPEAR::FileUpload] 제약되는 파일으로 확인\n"));
		file.diag_result.result_code = -1;
		return false;
	}

	unsigned long inaddr;

	memset(&dA, 0, sizeof(dA));
	dA.sin_family = AF_INET;
	inaddr = inet_addr(SERVER_IP.c_str());
	if (inaddr != INADDR_NONE)
		memcpy(&dA.sin_addr, &inaddr, sizeof(inaddr));

	dA.sin_port = htons(SERVER_PORT);
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(s, (sockaddr*)&dA, slen) < 0){
		AfxTrace(TEXT("[NOSPEAR::FileUpload] 서버에 연결할 수 없음\n"));
		file.diag_result.result_code = -2;
		return false;
	}

	getpeername(s, (sockaddr*)&aa, &slen);

	//CString to UTF-8
	CString filename = file.Getfilename();
	std::string utf8_filename = CW2A(filename, CP_UTF8);

	//Send File Name Length
	unsigned int length = htonl(utf8_filename.size());
	send(s, (char*)&length, 4, 0);

	//Send File Name (UTF-8 String to char*) UTP-8로 변경한 후 서버에 전송
	send(s, utf8_filename.c_str(), (UINT)utf8_filename.size(), 0);

	//Send File Hash
	//CString to UTF-8
	CString filehash = file.Getfilehash();
	std::string utf8_filehash = CW2A(filehash, CP_UTF8);
	send(s, utf8_filehash.c_str(), 64, 0);

	unsigned int filesize = htonl(file.Getfilesize());
	send(s, (char*)&filesize, 4, 0);

	char file_buffer[NOSPEAR::FILE_BUFFER_SIZE];
	int read_size = 0;

	FILE* fp = _wfopen(file.Getfilepath(), L"rb");
	if (fp == NULL) {
		AfxTrace(TEXT("[NOSPEAR::FileUpload] 파일이 유효하지 않습니다.\n"));
		closesocket(s);
		file.diag_result.result_code = -3;
		return false;
	}

	while ((read_size = fread(file_buffer, 1, NOSPEAR::FILE_BUFFER_SIZE, fp)) != 0) {
		send(s, file_buffer, read_size, 0);
	}

	AfxTrace(TEXT("[NOSPEAR::FileUpload] 파일 업로드 완료\n"));

	//검사 결과를 리턴 받습니다. 동기 방식을 사용
	unsigned short diag_result = 0;
	recv(s, (char*)&diag_result, 2, 0);
	file.diag_result.result_code = ntohs(diag_result);

	fclose(fp);
	closesocket(s);

	return true;
}

CString NOSPEAR::GetMsgFromErrCode(short err_code){
	switch (err_code) {
		case -1:
			return L"업로드 제약 파일";
		case -2:
			return L"서버 연결 실패";
		case -3:
			return L"파일이 유효하지 않음";
		case TYPE_NORMAL:
			return L"정상";
		case TYPE_MALWARE:
			return L"악성";
		case TYPE_SUSPICIOUS:
			return L"악성 의심";
		case TYPE_UNEXPECTED:
			return L"알 수 없음";
		case TYPE_NOFILE:
			return L"문서 파일 아님";
		case TYPE_RESEND:
			return L"파일 업로드 오류";
		case TYPE_REJECT:
			return L"검사 거부";
		case TYPE_LOCAL:
			return L"로컬";
		default:
			return L"Error";
	}
}
CString NOSPEAR::GetMsgFromNospear(short nospear) {
	switch (nospear) {
		case 0:
			return L"모두 차단";
		case 1:
			return L"문서 차단";
		case 2:
			return L"전체 허용";
		default:
			return L"Error";
	}
}

void NOSPEAR::ActivateLiveProtect(bool status){
	//입력값과 현재 값이 같을 경우 함수 실행 취소
	if (live_protect_status == status)
		return;

	//LIVEPROTECT 객체 생성
	if (status) {
		if (liveprotect == NULL) {
			liveprotect = new LIVEPROTECT();
			//결과를 구조체로 넘겨서 메시지도 같이 출력해주면 좋을듯
			HRESULT result = liveprotect->ActivateLiveProtect();
			CString resultext;
			if (result == S_OK) {
				AfxMessageBox(_T("드라이버 연결 성공\n"));
				live_protect_status = true;
			}
			else {
				resultext.Format(_T("LIVEPROTECT::Init() return : %ld\n"), result);
				AfxMessageBox(resultext);
				delete(liveprotect);
				liveprotect = NULL;
			}
		}
	
	}
	else {
		liveprotect->InActivateLiveProtect();
		delete(liveprotect);
		liveprotect = NULL;
		AfxMessageBox(_T("드라이버 연결 종료\n"));
		live_protect_status = false;
	}
}

bool NOSPEAR::Diagnose(NOSPEAR_FILE& file){
	bool result = FileUpload(file);
	file.diag_result.result_msg = GetMsgFromErrCode(file.diag_result.result_code);
	if (result) {
		CString strInsQuery;
		strInsQuery.Format(TEXT("UPDATE NOSPEAR_LocalFileList SET DiagnoseDate=(datetime('now', 'localtime')), Hash='%ws', Serverity='%d' WHERE FilePath='%ws';"), file.Getfilehash(), file.diag_result.result_code, file.Getfilepath());
		nospearDB->ExecuteSqlite(strInsQuery);
	}
	return result;
}

void NOSPEAR::Notification(CString title, CString body) {
	ZeroMemory(&nid, sizeof(nid));
	nid.cbSize = sizeof(nid);
	nid.dwInfoFlags = NIIF_WARNING;
	nid.uFlags = NIF_MESSAGE | NIF_INFO | NIF_ICON;
	nid.uTimeout = 2000;
	nid.hWnd = AfxGetApp()->m_pMainWnd->m_hWnd;
	nid.uCallbackMessage = WM_TRAY_NOTIFYICACTION;
	nid.hIcon = AfxGetApp()->LoadIconW(IDR_MAINFRAME);
	lstrcpy(nid.szInfoTitle, title);
	lstrcpy(nid.szInfo, body);
	::Shell_NotifyIcon(NIM_MODIFY, &nid);
}

void NOSPEAR::AutoDiagnose() {
	int total = 0, success = 0;
	total = request_diagnose_queue.size();
	while (request_diagnose_queue.size() != 0) {
		CString filepath = request_diagnose_queue.front();
		CFileFind pFind;
		BOOL bRet = pFind.FindFile(filepath);
		if (bRet == FALSE) {
			request_diagnose_queue.pop();
			continue;
		}
		NOSPEAR_FILE file(filepath);
		bRet = Diagnose(file);
		if (!bRet && file.diag_result.result_code == -2) {
			Notification(L"No-Spear 서버 연결 실패", L"No-Spear 서버 연결에 실패하였습니다. 서버 주소와 포트를 확인하세요.");
			break;
		};
		if (bRet) {
			success++;
		}
		
		request_diagnose_queue.pop();
	}
	CString result;
	result.Format(TEXT("전체 %d개 중 %d개 검사가 완료되었습니다."), total, success);
	Notification(L"No-Spear 검사 결과", result);

}

SQLITE* NOSPEAR::GetSQLitePtr(){
	return nospearDB;
}

NOSPEAR::NOSPEAR(){
	//기본생성자
	//일반 사용자 환경에서는 하드코딩된 서버 주소로 접속함
	nospearDB = new SQLITE();
}

NOSPEAR::~NOSPEAR(){
	if (nospearDB != NULL) {
		nospearDB->~SQLITE();
		delete(nospearDB);
	}
	if (liveprotect != NULL) {
		ActivateLiveProtect(false);
		delete(liveprotect);
	}
}

NOSPEAR::NOSPEAR(std::string ip, unsigned short port){
	//config.dat 파일이 있을 때 사용되는 생성자
	SERVER_IP = ip;
	SERVER_PORT = port;
	nospearDB = new SQLITE();
}
bool NOSPEAR::HasZoneIdentifierADS(CString filepath) {
	CStdioFile ads_stream;
	CFileException e;
	if (!ads_stream.Open(filepath + L":Zone.Identifier", CFile::modeRead, &e)) {
		return false;
	}
	return true;
}
unsigned short NOSPEAR::ReadNospearADS(CString filepath) {

	CStdioFile ads_stream;
	CFileException e;
	if (!ads_stream.Open(filepath + L":NOSPEAR", CFile::modeRead, &e)) {
		return -1;
	}

	CString str;
	ads_stream.ReadString(str);

	if (str == L"0")
		return 0;
	else if (str == L"1")
		return 1;
	else if (str == L"2")
		return 2;
	else {
		return -1;
	}
}
bool NOSPEAR::WriteNospearADS(CString filepath, unsigned short value) {
	//value 값으로 ADS:NOSPEAR 생성
	CString strInsQuery;
	strInsQuery.Format(TEXT("UPDATE NOSPEAR_LocalFileList SET NOSPEAR='%d' WHERE FilePath='%ws';"), value, filepath);
	nospearDB->ExecuteSqlite(strInsQuery);

	CStdioFile ads_stream;
	CFileException e;
	if (!ads_stream.Open(filepath + L":NOSPEAR", CStdioFile::modeCreate | CStdioFile::modeWrite, &e)) {
		AfxTrace(TEXT("WriteNospearADS 부착 실패 %d\n"), value);
		return false;
	}
	CString str;
	str.Format(TEXT("%d"), value);
	ads_stream.WriteString(str);
	AfxTrace(TEXT("WriteNospearADS 부착 성공 %d\n"), value);

	return true;
}

bool NOSPEAR::WriteZoneIdentifierADS(CString filepath, CString processName) {
	//pid를 이용해서 ProcessName으로 ADS:Zone.Identifier 생성
	CStdioFile ads_stream;
	CFileException e;
	if (!ads_stream.Open(filepath + L":Zone.Identifier", CStdioFile::modeCreate | CStdioFile::modeWrite, &e)) {
		AfxTrace(L"WriteZoneIdentifierADS 부착 실패\n");
		return false;
	}
	ads_stream.WriteString(L"[ZoneTransfer]\n");
	ads_stream.WriteString(L"ZoneId=3\n");
	ads_stream.WriteString(L"ADS Appended By No-Spear Client\n");
	ads_stream.WriteString(L"ProcessName=" + processName);
	AfxTrace(L"WriteZoneIdentifierADS 부착 성공\n");

	CString strInsQuery;
	strInsQuery.Format(TEXT("UPDATE NOSPEAR_LocalFileList SET ZoneIdentifier='3', ProcessName='%ws' WHERE FilePath='%ws';"), processName, filepath);
	nospearDB->ExecuteSqlite(strInsQuery);
	return true;
}

bool NOSPEAR::DeleteZoneIdentifierADS(CString filepath) {
	CString strInsQuery;
	strInsQuery.Format(TEXT("UPDATE NOSPEAR_LocalFileList SET ZoneIdentifier='0' WHERE FilePath='%ws';"), filepath);
	nospearDB->ExecuteSqlite(strInsQuery);
	return DeleteFile(filepath + L":Zone.Identifier");
}

void NOSPEAR::AppendDiagnoseQueue(CString filepath) {
	int queue_size = request_diagnose_queue.size();
	request_diagnose_queue.push(filepath);
	if (queue_size == 0){
		AutoDiagnose();
	}
}

