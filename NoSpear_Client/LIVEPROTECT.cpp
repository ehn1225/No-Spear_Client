#include "pch.h"
#include "NoSpear_ClientDlg.h"
#include "LIVEPROTECT.h"

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
    //processName.Format(TEXT("ProcessName=%ws"), processName);
    ads_stream.WriteString(L"[ZoneTransfer]\n");
    ads_stream.WriteString(L"ZoneId=3\n");
    ads_stream.WriteString(L"ADS Appended By No-Spear Client\n");
    ads_stream.WriteString(L"ProcessName=" + processName);
    AfxTrace(L"WriteZoneIdentifierADS 부착 성공\n");
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
    set<CString> list;
    list.insert(L"POWERPNT.EXE");       //MS Office PowerPoint
    list.insert(L"WINWORD.EXE");        //MS Office Word
    list.insert(L"EXCEL.EXE");          //MS Office Excel
    list.insert(L"HwpViewer.exe");      //Hancom hangule Viewer
    list.insert(L"scalc.exe");          //LibreOffice Excel
    list.insert(L"swriter.exe");        //LibreOffice Word
    list.insert(L"simpress.exe");       //LibreOffice PowerPoint

    //Lowercase 필요할까?
    CString programName = GetProcessName(pid);
    programName = PathFindFileName(programName);
    set<CString>::iterator it = list.find(programName);

    if (it != list.end())
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
            int passcode = 0;
            CString ext = PathFindExtension(path);

            if (mode == TYPE_CREATE || ext == L".part" || ext == L".download") {
                isMailicious = false;
                if (ext == L".part" || ext == L".download") {
                    CString path2 = path;
                    path2.Replace(ext, L"");
                    adsZoneList.insert(make_pair(path2, processname));
                    passcode = 1;
                }
                else{
                    if (IsOfficeProgram(pid))
                        safeDocList.insert(path);
                    passcode = 2;
                }
            }
            else {
                unsigned short ads_result = 0;
                ads_result = ReadNospearADS(path);
                switch (ads_result) {
                    case 0:
                        isMailicious = true;
                        passcode = 3;
                        break;
                    case 1:
                        isMailicious = IsOfficeProgram(pid);
                        passcode = 4;
                        break;
                    case 2:
                        isMailicious = false;
                        passcode = 5;
                        break;
                    default:
                        isMailicious = false;
                        WriteNospearADS(path, 1);
                        std::set<CString>::iterator it = safeDocList.find(path);
                        if (it != safeDocList.end()) {
                            //로컬 문서 프로그램으로 생성한 이력이 있는 경우
                            bool temp = WriteNospearADS(path, 2);
                            passcode = 6;
                        }
                        else {
                            if (!IsOfficeProgram(pid)) {
                                break;
                            }
                            //로컬에서 생성한 적이 없는 새로운 문서를 실행할 때
                            //Local File DB 확인
                            //passcode = 6;
                            //없으면 Black List DB 확인
                            //passcode = 7;
                            //없으면 업로드
                            CString errMsg;
                            errMsg.Format(TEXT("확인되지 않은 문서 파일이 실행되는 것을 탐지하였습니다.\n파일명 : %ws\n 검사를 진행하시겠습니까? "), PathFindFileName(path));
                            if (AfxMessageBox(errMsg, MB_YESNO | MB_ICONWARNING) == IDYES) {
                                AfxMessageBox(L"검사를 진행합니다.");
                                isMailicious = true;
                                passcode = 81;
                            }
                            else {
                                WriteNospearADS(path, 2);
                                isMailicious = false;
                                passcode = 82;
                            }
                        }
                        
                        break;
                }

                //Zone.Identifier 붙혀줌. NOSPEAR.cpp로 이동 예정(클라이언트 스레드)
                std::map<CString, CString>::iterator it = adsZoneList.begin();
                while (it != adsZoneList.end()) {
                    if (WriteZoneIdentifierADS(it->first, it->second) && processname != it->second) {
                        AfxTrace(L"PeeKabuu\n");
                        adsZoneList.erase(it++);
                    }
                    else
                        ++it;

                }
            }

            

            CString tmp;
            tmp.Format(TEXT("process name : %ws, FilePath : %s, MODE : %d, Permit : %ws(%d)\n"), processname, wDosFilePath, mode, (isMailicious) ? L"BLOCKED" : L"ALLOWED", passcode);
            AfxTrace(tmp);

            CString str_mode[] = {L"OPEN",  L"CREATE", L"DELETE"};
            //최종결과를 NOSPEAR:NOSPEAR_HISTORY 테이블에 저장 
            CString strInsQuery = _T("INSERT INTO NOSPEAR_HISTORY(FilePath, ProcessName, Operation, Permission) VALUES ('" + path + "','" + processname +"','" + str_mode[mode] + "','" + ((isMailicious) ? L"BLOCKED" : L"ALLOWED") + "');");
            liveProtectDB.ExecuteSqlite(strInsQuery);
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
    liveProtectDB.ExecuteSqlite(L"CREATE TABLE IF NOT EXISTS NOSPEAR_HISTORY(SEQ INTEGER PRIMARY KEY AUTOINCREMENT, TimeStamp TEXT not null DEFAULT (datetime('now', 'localtime')), FilePath TEXT NOT NULL, ProcessName TEXT, Operation TEXT, Permission TEXT);");
    liveProtectDB.ExecuteSqlite(L"CREATE TABLE IF NOT EXISTS NOSPEAR_LocalFileList(SEQ INTEGER PRIMARY KEY AUTOINCREMENT, TimeStamp TEXT not null DEFAULT (datetime('now', 'localtime')), FilePath TEXT NOT NULL, ZoneIdentifier INTEGER, ProcessName TEXT, DiagnoseDate TEXT, Serverity INTEGER);");
    
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
