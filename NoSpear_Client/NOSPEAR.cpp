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
namespace fs = std::filesystem;

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

	dA.sin_port = htons(SERVER_Diagnose_PORT);
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

void NOSPEAR::InitNospear(){
	nospearDB = new SQLITE();
	nospearDB->DatabaseOpen(L"NOSPEAR");
	
	nospearDB->ExecuteSqlite(L"CREATE TABLE IF NOT EXISTS NOSPEAR_HISTORY(SEQ INTEGER PRIMARY KEY AUTOINCREMENT, TimeStamp TEXT not null DEFAULT (datetime('now', 'localtime')), FilePath TEXT NOT NULL, ProcessName TEXT, Operation TEXT, NOSPEAR INTEGER, Permission TEXT);");
	nospearDB->ExecuteSqlite(L"CREATE TABLE IF NOT EXISTS NOSPEAR_LocalFileList(FilePath TEXT NOT NULL PRIMARY KEY, ZoneIdentifier INTEGER, ProcessName TEXT, NOSPEAR INTEGER, DiagnoseDate TEXT, Hash TEXT, Serverity INTEGER, FileType TEXT, TimeStamp TEXT not null DEFAULT (datetime('now', 'localtime')));");
	nospearDB->ExecuteSqlite(L"CREATE TABLE IF NOT EXISTS NOSPEAR_VersionInfo(VersionName TEXT NOT NULL PRIMARY KEY, TimeStamp TEXT not null DEFAULT (datetime('now', 'localtime')));");
	nospearDB->ExecuteSqlite(L"INSERT INTO NOSPEAR_VersionInfo(VersionName, TimeStamp) VALUES ('BlackListDB', '2022-09-01 00:00:00');");
	nospearDB->ExecuteSqlite(L"CREATE TABLE IF NOT EXISTS NOSPEAR_Quarantine(FilePath TEXT NOT NULL PRIMARY KEY, FileHash TEXT, TimeStamp TEXT not null DEFAULT (datetime('now', 'localtime')));");

	office_file_ext_list.insert(L".doc");
	office_file_ext_list.insert(L".docx");
	office_file_ext_list.insert(L".xls");
	office_file_ext_list.insert(L".xlsx");
	office_file_ext_list.insert(L".pptx");
	office_file_ext_list.insert(L".ppsx");
	office_file_ext_list.insert(L".hwp");
	office_file_ext_list.insert(L".hwpx");
	office_file_ext_list.insert(L".pdf");

	CreateDirectory(L"Quarantine", NULL);
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

bool NOSPEAR::ActivateLiveProtect(bool status){
	//입력값과 현재 값이 같을 경우 함수 실행 취소
	if (live_protect_status == status)
		return status;

	//LIVEPROTECT 객체 생성
	if (status) {
		if (liveprotect == NULL) {
			liveprotect = new LIVEPROTECT();
			//결과를 구조체로 넘겨서 메시지도 같이 출력해주면 좋을듯
			HRESULT result = liveprotect->ActivateLiveProtect();
			CString resultext;
			if (result == S_OK) {
				Notification(L"실시간 감시가 활성화되었습니다.", L"새로운 문서 파일 생성과 문서 파일 접근을 통제합니다.");
				//AfxMessageBox(_T("드라이버 연결 성공\n"));
				live_protect_status = true;
			}
			else {
				if (result == 2)
					AfxMessageBox(L"드라이버에 연결할 수 없습니다.\n드라이버가 실행 중인지 확인하세요.");
				else {
					resultext.Format(_T("LIVEPROTECT::Init() return : %ld\n"), result);
					AfxMessageBox(resultext);
				}
				
				delete(liveprotect);
				liveprotect = NULL;
			}
		}
	
	}
	else {
		Notification(L"실시간 감시가 비활성화되었습니다.", L"새로운 문서 파일 생성과 문서 파일 접근에 대한 통제를 종료합니다.");
		liveprotect->InActivateLiveProtect();
		delete(liveprotect);
		liveprotect = NULL;
		//AfxMessageBox(_T("드라이버 연결 종료\n"));
		live_protect_status = false;
	}
	return live_protect_status;
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
	while (!request_diagnose_queue.empty()) {
		CString filepath = request_diagnose_queue.front();
		request_diagnose_queue.pop();
		CFileFind pFind;
		BOOL bRet = pFind.FindFile(filepath);
		if (bRet == FALSE) {
			continue;
		}
		NOSPEAR_FILE file(filepath);
		bRet = Diagnose(file);
		if (!bRet && file.diag_result.result_code == -2) {
			Notification(L"No-Spear 서버 연결 실패", L"파일 검사를 위한 No-Spear 서버 연결에 실패하였습니다.");
			while (!request_diagnose_queue.empty()) request_diagnose_queue.pop();
		};
		if (bRet) {
			success++;
		}
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
	InitNospear();
}
NOSPEAR::NOSPEAR(std::string ip, unsigned short port1, unsigned short port2) {
	//config.dat 파일이 있을 때 사용되는 생성자
	SERVER_IP = ip;
	SERVER_Diagnose_PORT = port1;
	SERVER_Update_PORT = port2;
	InitNospear();
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

bool NOSPEAR::UpdataBlackListDB(){
	//패턴 업데이트
	unsigned long inaddr;
	memset(&dA, 0, sizeof(dA));
	dA.sin_family = AF_INET;
	inaddr = inet_addr(SERVER_IP.c_str());
	if (inaddr != INADDR_NONE)
		memcpy(&dA.sin_addr, &inaddr, sizeof(inaddr));

	dA.sin_port = htons(SERVER_Update_PORT);
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(s, (sockaddr*)&dA, slen) < 0) {
		AfxTrace(TEXT("[NOSPEAR::UpdataBlackListDB] 서버에 연결할 수 없음\n"));
		return false;
	}

	//NOSPEAR_VersionInfo 테이블에서 BlackListDB 마지막 업데이트 일자를 가져오고 서버로 전송함
	sqlite3_select p_selResult = nospearDB->SelectSqlite(L"select TimeStamp from NOSPEAR_VersionInfo WHERE VersionName='BlackListDB'");
	string timeStamp;
	if (p_selResult.pnRow != 0) {
		timeStamp = string(p_selResult.pazResult[1]);
		AfxTrace(TEXT("[NOSPEAR::UpdataBlackListDB] Update BlackListDB from %ws\n"), CString(p_selResult.pazResult[1]));
	}
	else {
		AfxTrace(TEXT("[NOSPEAR::UpdataBlackListDB] Can't access Last Pattern Update Data\n"));
		return false;
	}

	//업데이트를 요청한 시각으로 DB 최신화
	nospearDB->ExecuteSqlite(L"update NOSPEAR_VersionInfo set TimeStamp=(datetime('now', 'localtime')) WHERE VersionName='BlackListDB';");
	getpeername(s, (sockaddr*)&aa, &slen);

	//마지막 패턴 업데이트 일자를 서버로 전송
	send(s, timeStamp.c_str(), (UINT)timeStamp.size(), 0);

	//4바이트 json 전체 길이 수신
	unsigned int recv_size = 0;
	recv(s, (char*)&recv_size, 4, 0);
	recv_size = ntohl(recv_size);
	string json;

	//소켓의 내용을 배열에 저장
	if (recv_size != 0) {
		unsigned char* arr = (unsigned char*)malloc(recv_size + 1);
		recv(s, (char*)arr, recv_size, 0);
		json = string((char*)arr);
		ofstream output;
		output.open(L"temp.txt");
		output.write((char*)arr, recv_size);
		output.close();
	}

	closesocket(s);

	return true;
}

void NOSPEAR::AppendDiagnoseQueue(CString filepath) {
	request_diagnose_queue.push(filepath);
}

void NOSPEAR::ScanLocalFile(CString rootPath) {
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
					Notification(L"문서로 위장한 실행 파일을 발견하였습니다.", path);
					break;
				}
				AfxTrace(TEXT("Find Ext : %ws\n"), tmp_ext);
				tmp.Replace(tmp_ext, L"");
				tmp.Trim();
			}
		}
		if (IsOfficeFile(ext)) {
			CString strInsQuery;
			short nospear = 2, zoneid = 0, serverity = TYPE_LOCAL;
			if (bNTFS && HasZoneIdentifierADS(path)) {
				nospear = 1;
				zoneid = 3;
				serverity = TYPE_SUSPICIOUS;
			}
			//ADS:NOSPEAR값 다른지 확인하는 로직 필요
			strInsQuery.Format(TEXT("INSERT INTO NOSPEAR_LocalFileList(FilePath, ZoneIdentifier, ProcessName, NOSPEAR, DiagnoseDate, Hash, Serverity, FileType) VALUES ('%ws','%d','No-Spear Client','%d','-', '-', '%d','DOCUMENT');"), path, zoneid, nospear, serverity);
			int rc = nospearDB->ExecuteSqlite(strInsQuery);
			//rc가 19면 이미 들어가 있는 것.
			if (rc == 0) {
				count++;
			}
			else {
				nospear = ReadNospearADS(path);
				if(nospear > -1) {
					strInsQuery.Format(TEXT("UPDATE NOSPEAR_LocalFileList SET NOSPEAR='%d' WHERE FilePath='%ws';"), nospear, path);
					nospearDB->ExecuteSqlite(strInsQuery);
				}
			}
			AfxTrace(TEXT("path : %ws, rc : %d\n"), path, rc);
		}
	}
	CString tmp;
	tmp.Format(TEXT("새로운 %d개의 문서가 LocalFileListDB에 추가되었습니다.\n"), count);
	AfxTrace(tmp);
}

bool NOSPEAR::IsOfficeFile(CString ext) {
	set<CString>::iterator it = office_file_ext_list.find(ext);

	if (it != office_file_ext_list.end())
		return true;
	else
		return false;
}

bool NOSPEAR::IsQueueEmpty() {
	return request_diagnose_queue.empty();
}

void NOSPEAR::ScanFileAvailability() {
	sqlite3_select p_selResult = nospearDB->SelectSqlite(L"select FilePath from NOSPEAR_LocalFileList;");
	if (p_selResult.pnRow != 0) {
		for (int i = 1; i <= p_selResult.pnRow; i++) {
			int colCtr = 0;
			int nCol = 1;
			int cellPosition = (i * p_selResult.pnColumn) + colCtr;
			CString FilePath = SQLITE::Utf8ToCString(p_selResult.pazResult[cellPosition++]);
			CFileFind pFind;
			BOOL bRet = pFind.FindFile(FilePath);
			if (bRet == FALSE) {
				nospearDB->ExecuteSqlite(L"DELETE FROM NOSPEAR_LocalFileList WHERE FilePath='" + FilePath + L"';");
				AfxTrace(FilePath+ L" 파일은 유효하지 않음.\n");
			}
		}
	}
}
void NOSPEAR::AttachADSOther() {
	sqlite3_select p_selResult = nospearDB->SelectSqlite(L"select FilePath, ZoneIdentifier from NOSPEAR_LocalFileList WHERE FileType='OTHER';");
	if (p_selResult.pnRow != 0) {
		for (int i = 1; i <= p_selResult.pnRow; i++) {
			int colCtr = 0;
			int nCol = 1;
			int cellPosition = (i * p_selResult.pnColumn) + colCtr;
			CString filePath = SQLITE::Utf8ToCString(p_selResult.pazResult[cellPosition++]);
			CString zoneId = SQLITE::Utf8ToCString(p_selResult.pazResult[cellPosition++]);
			CFileFind pFind;
			BOOL bRet = pFind.FindFile(filePath);
			if (bRet == FALSE) {
				nospearDB->ExecuteSqlite(L"DELETE FROM NOSPEAR_LocalFileList WHERE FilePath='" + filePath + L"';");
				AfxTrace(filePath + L" 파일은 유효하지 않음.\n");
			}
			else {
				if (HasZoneIdentifierADS(filePath)) {
					nospearDB->ExecuteSqlite(L"DELETE FROM NOSPEAR_LocalFileList WHERE FilePath='" + filePath + L"';");
					AfxTrace(filePath + L" ADS가 유지되었으므로 DB에서 삭제\n");
				}
				WriteZoneIdentifierADS(filePath, zoneId);
			}
		}
	}
}
void NOSPEAR::Quarantine(CString filepath){
	//정상 파일을 XOR 인코딩
	NOSPEAR_FILE file(filepath);
	if (file.Quarantine()) {
		nospearDB->ExecuteSqlite(L"INSERT INTO NOSPEAR_Quarantine(FilePath, FileHash) VALUES ('"+filepath+"', '"+ file.Getfilehash() + L"');");
		DeleteFile(filepath);
		nospearDB->ExecuteSqlite(L"DELETE FROM NOSPEAR_LocalFileList WHERE FilePath='" + filepath + L"';");
	}
}
void NOSPEAR::InQuarantine(CString filepath) {
	//정상 파일로 XOR 디코딩
	sqlite3_select p_selResult = nospearDB->SelectSqlite(L"select FileHash from NOSPEAR_Quarantine WHERE FilePath='"+ filepath + L"'");
	CString hash;
	if (p_selResult.pnRow != 0) {
		hash = CString(p_selResult.pazResult[1]);
		NOSPEAR_FILE file(L".\\Quarantine\\" + hash);
		file.InQuarantine(filepath);
		DeleteFile(file.Getfilepath());
		nospearDB->ExecuteSqlite(L"DELETE FROM NOSPEAR_Quarantine WHERE FilePath='" + filepath + L"';");
	}

	//NOSPEAR_FILE file(L".\\Quarantine\\" + hash);
	//sqlite3_select p_selResult = nospearDB->SelectSqlite(L"select FilePath from NOSPEAR_Quarantine WHERE FileHash='" + hash + L"'");
	//CString OriginalPath;
	//if (p_selResult.pnRow != 0) {
	//	OriginalPath = SQLITE::Utf8ToCString(p_selResult.pazResult[1]);
	//	file.InQuarantine(OriginalPath);
	//	DeleteFile(file.Getfilepath());
	//}
 }