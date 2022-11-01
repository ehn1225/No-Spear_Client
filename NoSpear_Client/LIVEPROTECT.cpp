#include "pch.h"
#include "NoSpear_ClientDlg.h"
#include "LIVEPROTECT.h"
#include "resource.h"
#define WM_TRAY_NOTIFYICACTION (WM_USER + 10)

#pragma comment(lib,"fltLib.lib")

enum MODE_TYPE {
	TYPE_OPEN,
	TYPE_CREATE,
	TYPE_DELETE
};
unsigned short inline LIVEPROTECT::ReadNospearADS(CString filepath) {
    //return values
    //-1 : ADS:NOSPEAR 부재
    //0 : ADS:NOSPEAR외 전체 차단
    //1 : ADS:NOSPEAR에서 문서 프로그램을 제외하고 허용
    //2 : ADS:NOSPEAR에서 전체 허용

    CStdioFile ads_stream;
    CFileException e;
    if (!ads_stream.Open(filepath + L":NOSPEAR", CFile::modeRead, &e)) {
        //NOSPEAR ADS가 없음.
        return -1;
    }

    CString str;
    ads_stream.ReadString(str);

    //NOSPEAR ADS Value에 따른 결과 반환
    if (str == L"0")
        return 0;
    else if (str == L"1")
        return 1;
    else if (str == L"2")
        return 2;
    else{
        //NOSPEAR ADS 잘못된 값이 들어왔을 경우 없다고 간주하고 차단
        return -1;
    }
}

bool LIVEPROTECT::WriteNospearADS(CString filepath, unsigned short value){
    //value 값으로 ADS:NOSPEAR 생성
    CStdioFile ads_stream;
    CFileException e;
    if (!ads_stream.Open(filepath + L":NOSPEAR", CStdioFile::modeCreate | CStdioFile::modeWrite, &e)){
        AfxTrace(TEXT("WriteNospearADS 부착 실패 %d\n"), value);
        return false;
    }
    CString str;
    str.Format(TEXT("%d"), value);
    ads_stream.WriteString(str);
    AfxTrace(TEXT("WriteNospearADS 부착 성공 %d\n"), value);
    return true;
}

bool LIVEPROTECT::WriteZoneIdentifierADS(CString filepath, CString processName){
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
    return true;
}
bool LIVEPROTECT::ReadZoneIdentifierADS(CString filepath) {
    CStdioFile ads_stream;
    CFileException e;
    if (!ads_stream.Open(filepath + L":Zone.Identifier", CFile::modeRead, &e)) {
        return false;
    }
    return true;
}


PWCHAR LIVEPROTECT::GetCharPointerW(PWCHAR pwStr, WCHAR wLetter, int Count) {
    //pwStr문자열에서 wLetter문자의 Count번째 위치를 리턴하는 함수

	int i = 0, j = 0;
	int cchStr = (int)wcslen(pwStr);

	for (j = 0; j < Count; j++) {
		while (pwStr[i] != wLetter && pwStr[i] != 0) {
			++i;
		}
		++i;
	}

	return (PWCHAR)&pwStr[--i];
}

CString LIVEPROTECT::GetProcessName(unsigned long pid){
    CString processname = L"Unknown";
    DWORD error = 0;

    if (HANDLE hProc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid)) {
        wchar_t buf[512] = { 0, };
        DWORD bufLen = sizeof(buf);
        QueryFullProcessImageName(hProc, 0, buf, &bufLen);
        CloseHandle(hProc);
        processname = CString(buf);
    }
    return processname;
}

bool LIVEPROTECT::IsOfficeProgram(unsigned long pid){
    //Lowercase 필요할까?
    CString programName = GetProcessName(pid);
    programName = PathFindFileName(programName);
    set<CString>::iterator it = office_program_list.find(programName);

    if (it != office_program_list.end())
        return true;
    else
        return false;
}
bool LIVEPROTECT::IsOfficeFile(CString ext) {
    set<CString>::iterator it = office_file_ext_list.find(ext);

    if (it != office_file_ext_list.end())
        return true;
    else
        return false;
}

DWORD LIVEPROTECT::ScannerWorker(PSCANNER_THREAD_CONTEXT Context) {
    PSCANNER_NOTIFICATION notification;
    SCANNER_REPLY_MESSAGE replyMessage;
    PSCANNER_MESSAGE message = NULL;
    LPOVERLAPPED pOvlp;
    BOOL isMailicious;
    DWORD outSize;
    HRESULT hr;
    ULONG_PTR key;

#pragma warning(push)
#pragma warning(disable:4127) // conditional expression is constant

    while (threadstatus) {
#pragma warning(pop)

        isMailicious = GetQueuedCompletionStatus(Context->Completion, &outSize, &key, &pOvlp, INFINITE);
        message = CONTAINING_RECORD(pOvlp, SCANNER_MESSAGE, Ovlp);
        notification = &message->Notification;

        if (!isMailicious) {
            hr = HRESULT_FROM_WIN32(GetLastError());
            break;
        }

        {
            WCHAR wDosFilePath[512] = { 0, };

            PWCHAR pwPtr = GetCharPointerW((PWCHAR)notification->Contents, L'\\', 3);
            if (!pwPtr) {
                hr = -1;
                break;
            }

            *pwPtr = L'\0';

            hr = FilterGetDosName((PWCHAR)notification->Contents, wDosFilePath, 512);
            if (FAILED(hr))
                break;  

            wcscat(wDosFilePath, L"\\");
            wcscat(wDosFilePath, ++pwPtr);
            //CString tmp;
            //tmp.Format(TEXT("pid : %d, process name : %ws, FilePath : %s\n"), pid, GetProcessName(pid), wDosFilePath);
            //AfxTrace(tmp);

            unsigned long pid = notification->pid;
            unsigned long mode = notification->mode;
            CString path = CString(wDosFilePath);
            CString processname = GetProcessName(pid);
            CString ext = PathFindExtension(path);
            int passcode = 0;
            unsigned short ads_result = 0;

            if (mode == TYPE_CREATE || ext == L".part" || ext == L".download") {
                isMailicious = false;
                CString strInsQuery;
                CString type = L"DOCUMENT";
                int zoneid = 3;
                int nospear = 1;
                passcode = 14;
                if (IsOfficeProgram(pid)) {
                    safeDocList.insert(path);
                    zoneid = 0;
                    nospear = 2;
                    passcode = 11;
                }
                else {
                    if (ext == L".part" || ext == L".download") {
                        CString path2 = path;
                        path2.Replace(ext, L"");
                        path = path2;
                        if (!IsOfficeFile(PathFindExtension(path2))) {
                            type = "OTHER";
                            passcode = 12;
                        }
                        else {
                            passcode = 13;
                        }
                    }
                }
                strInsQuery.Format(TEXT("REPLACE INTO NOSPEAR_LocalFileList(FilePath, ZoneIdentifier, ProcessName, NOSPEAR, DiagnoseDate, Serverity, FileType) VALUES ('%ws','%d','%ws','%d','-','0','%ws');"), path, zoneid, processname, nospear, type);
                liveProtectDB.ExecuteSqlite(strInsQuery);
            }
            else {
                ads_result = ReadNospearADS(path);
                switch (ads_result) {
                    case 0:
                        isMailicious = true;
                        passcode = 3;
                        break;
                    case 1:
                        isMailicious = IsOfficeProgram(pid);
                        if (isMailicious) {
                            //ADS:NOSPEAR = 1인 파일을 사용자가 Office Program으로 실행할 때
                            ZeroMemory(&nid, sizeof(nid));
                            nid.cbSize = sizeof(nid);
                            nid.dwInfoFlags = NIIF_WARNING;
                            nid.uFlags = NIF_MESSAGE | NIF_INFO | NIF_ICON;
                            nid.uTimeout = 2000;
                            nid.hWnd = AfxGetApp()->m_pMainWnd->m_hWnd;
                            nid.uCallbackMessage = WM_TRAY_NOTIFYICACTION;
                            nid.hIcon = AfxGetApp()->LoadIconW(IDR_MAINFRAME);
                            CString filename = PathFindFileName(path);
                            lstrcpy(nid.szInfoTitle, L"악성 의심 파일의 실행을 차단하였습니다");
                            lstrcpy(nid.szInfo, (filename + L"은 사용자 컴퓨터에서 실행이 허용되지 않은 파일이므로 차단되었습니다."));
                            ::Shell_NotifyIcon(NIM_MODIFY, &nid);

                            CString errMsg;
                            errMsg.Format(TEXT("새로운 문서 파일이 실행되는 것을 탐지하였습니다.\n파일명 : %ws\n 검사를 진행하시겠습니까?\n검사를 진행하려면 \"예\"를, 검사 없이 문서를 여시려면 \"아니요\", 문서 실행을 취소하려면 \"취소\"를 누르세요."), filename);
                            switch (AfxMessageBox(errMsg, MB_YESNOCANCEL | MB_ICONWARNING)) {
                                case IDYES:
                                    AfxMessageBox(L"검사를 진행합니다.");
                                case IDCANCEL:
                                default:
                                    passcode = 81;
                                    break;
                                case IDNO:
                                    AfxMessageBox(L"검사없이 파일을 실행합니다.");
                                    WriteNospearADS(path, 2);
                                    ads_result = 2;
                                    isMailicious = false;
                                    passcode = 82;
                                    break;
                            }
                        }
                        passcode = 4;
                        break;
                    case 2:
                        isMailicious = false;
                        passcode = 5;
                        break;
                    default:
                        isMailicious = false;
                        std::set<CString>::iterator it = safeDocList.find(path);
                        if (it != safeDocList.end()) {
                            //로컬 문서 프로그램으로 생성한 이력이 있는 경우(캐시)
                            bool temp = WriteNospearADS(path, 2);
                            passcode = 6;
                        }
                        else {
                            //Local File DB 확인. DB에 있지만, 모종의 이유로 ADS가 사라진 경우
                            //isOfficeProgram?으로 그냥 1번 붙히고 통과시켜 주는 것을 먼저할 수도 있지만, 한번에 제대로 하기 위해서 뒤로 배치
                            //passcode = 6;
                            sqlite3_select p_selResult = liveProtectDB.SelectSqlite(L"select NOSPEAR, ZoneIdentifier, ProcessName from NOSPEAR_LocalFileList WHERE FilePath='" + path + L"' LIMIT 1;");
                            if (p_selResult.pnRow != 0) {
                                int nospear = stoi(p_selResult.pazResult[3]);
                                int zone = stoi(p_selResult.pazResult[4]);
                                CString ProcessName(p_selResult.pazResult[5]);
                                AfxTrace(TEXT("FIND Local DB ADS : %d, Zone : %d, ProcessName : %s\n"), nospear, zone, ProcessName);
                                WriteNospearADS(path, nospear);
                                if (zone != 0)
                                    WriteZoneIdentifierADS(path, ProcessName);
                                if (nospear == 1 && IsOfficeProgram(pid))
                                    isMailicious = true;
                                passcode = 71;
                                break;
                            }
                            else {
                                //LocalFileDB에 없으면서 ADS:NOSPEAR도 없을 때
                                //Office 프로그램이 생성한 파일은 CREATE로그가 존재하기에 여기에 안걸림
                                //실시간 검사가 없을 때 생긴 파일이므로 DB에 추가함
                                //악성문서일 가능성으로 인해 차단함
                                WriteNospearADS(path, 1);
                                CString strInsQuery;
                                int zoneid = ReadZoneIdentifierADS(path) ? 3 : 0;
                                strInsQuery.Format(TEXT("INSERT INTO NOSPEAR_LocalFileList(FilePath, ZoneIdentifier, ProcessName, NOSPEAR, DiagnoseDate, Serverity, FileType) VALUES ('%ws','%d','%ws','1','-','0','DOCUMENT');"), path, zoneid, processname);
                                liveProtectDB.ExecuteSqlite(strInsQuery);
                                //Office 프로그램 한정 차단
                                if (IsOfficeProgram(pid)) {
                                    isMailicious = true;
                                }
                                passcode = 72;
                                break;
                            }                           
                        }
                        
                        break;
                }
               

            }

            CString str_mode[] = { L"OPEN",  L"CREATE", L"DELETE" };
            //최종결과를 NOSPEAR:NOSPEAR_HISTORY 테이블에 저장 
            CString strInsQuery;
            strInsQuery.Format(TEXT("INSERT INTO NOSPEAR_HISTORY(FilePath, ProcessName, Operation, NOSPEAR, Permission) VALUES ('%ws','%ws','%ws','%d','%ws');"), path, processname, str_mode[mode], ads_result, ((isMailicious) ? L"BLOCKED" : L"ALLOWED"));
            liveProtectDB.ExecuteSqlite(strInsQuery);
            AfxTrace(strInsQuery);

        }
       
        replyMessage.ReplyHeader.Status = 0;
        replyMessage.ReplyHeader.MessageId = message->MessageHeader.MessageId;
        replyMessage.Reply.SafeToOpen = !isMailicious;

  
        hr = FilterReplyMessage(Context->Port, (PFILTER_REPLY_HEADER)&replyMessage, sizeof(FILTER_REPLY_HEADER) + 1);
        //원래  sizeof( replyMessage ) 이거였는데, 21로 나와서 오류남. 17이 나와야 정상

        if (!SUCCEEDED(hr)) {
            AfxTrace(TEXT("Scanner: Error replying message. Error = 0x%X\n", hr));
            break;
        }

        memset(&message->Ovlp, 0, sizeof(OVERLAPPED));

        hr = FilterGetMessage(Context->Port, &message->MessageHeader, FIELD_OFFSET(SCANNER_MESSAGE, Ovlp), &message->Ovlp);

        if (hr != HRESULT_FROM_WIN32(ERROR_IO_PENDING)) {
            break;;
        }
    }

    if (!SUCCEEDED(hr)) {
        if (hr == HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE)) {
            AfxTrace(TEXT("Scanner: Port is disconnected, probably due to scanner filter unloading.\n"));
        }
        else {
            AfxTrace(TEXT("Scanner: Unknown error occured.Error = 0x % X\n", hr));
        }
    }

    if(message != NULL) free(message);

    return hr;
}

LIVEPROTECT::LIVEPROTECT() {
    if (liveProtectDB.DatabaseOpen(L"NOSPEAR")) {
        AfxTrace(TEXT("[LIVEPROTECT::LIVEPROTECT] Can't Create NOSPEAR_HISTORY DataBase.\n"));
        return;
    }

    liveProtectDB.ExecuteSqlite(L"CREATE TABLE IF NOT EXISTS NOSPEAR_HISTORY(SEQ INTEGER PRIMARY KEY AUTOINCREMENT, TimeStamp TEXT not null DEFAULT (datetime('now', 'localtime')), FilePath TEXT NOT NULL, ProcessName TEXT, Operation TEXT, NOSPEAR INTEGER, Permission TEXT);");
    liveProtectDB.ExecuteSqlite(L"CREATE TABLE IF NOT EXISTS NOSPEAR_LocalFileList(FilePath TEXT NOT NULL PRIMARY KEY, ZoneIdentifier INTEGER, ProcessName TEXT, NOSPEAR INTEGER, DiagnoseDate TEXT, Serverity INTEGER, FileType TEXT, TimeStamp TEXT not null DEFAULT (datetime('now', 'localtime')));");
    
    office_file_ext_list.insert(L".doc");
    office_file_ext_list.insert(L".docx");
    office_file_ext_list.insert(L".xls");
    office_file_ext_list.insert(L".xlsx");
    office_file_ext_list.insert(L".pptx");
    office_file_ext_list.insert(L".ppsx");
    office_file_ext_list.insert(L".hwp");
    office_file_ext_list.insert(L".hwpx");
    office_file_ext_list.insert(L".pdf");

     office_program_list.insert(L"POWERPNT.EXE");       //MS Office PowerPoint
    office_program_list.insert(L"WINWORD.EXE");        //MS Office Word
    office_program_list.insert(L"EXCEL.EXE");          //MS Office Excel
    office_program_list.insert(L"HwpViewer.exe");      //Hancom hangule Viewer
    office_program_list.insert(L"scalc.exe");          //LibreOffice Excel
    office_program_list.insert(L"swriter.exe");        //LibreOffice Word
    office_program_list.insert(L"simpress.exe");       //LibreOffice PowerPoint
}

LIVEPROTECT::~LIVEPROTECT() {
    if (threadstatus) {
        InActivateLiveProtect();
    }
}

int LIVEPROTECT::ActivateLiveProtect(){
    DWORD threadId;
    HRESULT hr;
    DWORD i, j;

    //Open a commuication channel to the filter
    AfxTrace(TEXT("[LIVEPROTECT::ActivateLiveProtect] Connecting to the filter ...\n"));

    hr = FilterConnectCommunicationPort(ScannerPortName, 0, NULL, 0, NULL, &port);

    if (IS_ERROR(hr)) {
        AfxTrace(TEXT("[LIVEPROTECT::ActivateLiveProtect] ERROR: Connecting to filter port: 0x%08x\n", hr));
        return 2;
    }

    completion = CreateIoCompletionPort(port, NULL, 0, threadCount);

    if (completion == NULL) {
        AfxTrace(TEXT("[LIVEPROTECT::ActivateLiveProtect] ERROR: Creating completion port: %d\n", GetLastError()));
        CloseHandle(port);
        return 3;
    }

    AfxTrace(TEXT("[LIVEPROTECT::ActivateLiveProtect] Scanner: Port = 0x%p Completion = 0x%p\n", port, completion));

    context.Port = port;
    context.Completion = completion;
    threadstatus = true;

    for (i = 0; i < threadCount; i++) {
        threads[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ScannerWorker, &context, 0, &threadId);
        if (threads[i] == NULL) {
            hr = GetLastError();
            AfxTrace(TEXT("[LIVEPROTECT::ActivateLiveProtect] ERROR: Couldn't create thread: %d\n", hr));
            threadstatus = false;
            return 4;
        }

        for (j = 0; j < requestCount; j++) {
#pragma prefast(suppress:__WARNING_MEMORY_LEAK, "msg will not be leaked because it is freed in ScannerWorker")
            msg = (PSCANNER_MESSAGE)malloc(sizeof(SCANNER_MESSAGE));

            if (msg == NULL) {
                hr = ERROR_NOT_ENOUGH_MEMORY;
                threadstatus = false;
                return 5;
            }

            memset(&msg->Ovlp, 0, sizeof(OVERLAPPED));

            hr = FilterGetMessage(port,
                &msg->MessageHeader,
                FIELD_OFFSET(SCANNER_MESSAGE, Ovlp),
                &msg->Ovlp);

            if (hr != HRESULT_FROM_WIN32(ERROR_IO_PENDING)) {
                free(msg);
                threadstatus = false;
                return 6;
            }
        }
    }
    hr = S_OK;
    return hr;
}

int LIVEPROTECT::InActivateLiveProtect(){
    if (threadstatus) {
        threadstatus = false;
        WaitForMultipleObjectsEx(threadCount, threads, TRUE, INFINITE, FALSE);

        for (int i = 0; i < threadCount; i++) {
            CloseHandle(threads[i]);
        }
        AfxTrace(TEXT("[LIVEPROTECT::InActivateLiveProtect] InActivated!\n"));
        CloseHandle(port);
        CloseHandle(completion);
        return 0;
    }
    else {
        AfxTrace(TEXT("[LIVEPROTECT::InActivateLiveProtect] Not Activate!!\n"));
        return 1;
    }
}
