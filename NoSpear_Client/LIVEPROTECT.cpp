#include "pch.h"
#include "LIVEPROTECT.h"
#include "NoSpear_ClientDlg.h"
#pragma comment(lib,"fltLib.lib")

unsigned short inline LIVEPROTECT::ReadNospearADS(CString filepath) {
    //result values
    //-1 : ADS:NOSPEAR 부재
    //0 : ADS:NOSPEAR에서 차단하도록 지정되어있음.
    //1 : ADS:NOSPEAR에서 문서 프로그램을 제외하고 허용하도록 지정되어있음.
    //2 : ADS:NOSPEAR에서 허용하도록 지정되어있음.

    CStdioFile ads_stream;
    CFileException e;
    if (!ads_stream.Open(filepath + L":NOSPEAR", CFile::modeRead, &e)) {
        //NOSPEAR ADS를 열 수 없기에, 없다고 간주하고 차단
        //e.ReportError();
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
    if (!ads_stream.Open(filepath + L":NOSPEAR", CStdioFile::modeCreate | CStdioFile::modeWrite, &e)) {
        return false;
    }
    CString str;
    str.Format(TEXT("%d"), value);
    ads_stream.WriteString(str);
    return true;
}

bool LIVEPROTECT::WriteZoneIdentifierADS(CString filepath, unsigned long pid){
    //pid를 이용해서 ProcessName으로 ADS:Zone.Identifier 생성
    CStdioFile ads_stream;
    CFileException e;
    if (!ads_stream.Open(filepath + L":Zone.Identifier", CStdioFile::modeCreate | CStdioFile::modeWrite, &e)) {
        return false;
    }
    CString processName = GetProcessName(pid);
    //processName = PathFindFileName(processName);
    processName.Format(TEXT("ProcessName=%ws"), processName);
    ads_stream.WriteString(L"[ZoneTransfer]\n");
    ads_stream.WriteString(L"ZoneId=3\n");
    ads_stream.WriteString(L"ADS Appended By No-Spear Client\n");
    ads_stream.WriteString(processName);

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
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    CString processname = L"Unknown";
    DWORD error = 0;

    if (HANDLE hProc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid)) {
        wchar_t buf[512] = { 0, };
        DWORD bufLen = sizeof(buf);
        QueryFullProcessImageName(hProc, 0, buf, &bufLen);
        CloseHandle(hProc);
        processname = CString(buf);
    }
    //AfxMessageBox(processname);
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

        if (!isMailicious) {
            hr = HRESULT_FROM_WIN32(GetLastError());
            break;
        }

        notification = &message->Notification;
        unsigned long pid = notification->pid;
        
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
 /*           CString tmp;
            tmp.Format(TEXT("pid : %d, process name : %ws, FilePath : %s\n"), pid, GetProcessName(pid), wDosFilePath);
            AfxTrace(tmp);*/

            CString path = CString(wDosFilePath);

            //Alternate Data Stream 기반 파일 허용 여부
            unsigned short ads_result = 0;
            ads_result = ReadNospearADS(path);
            switch (ads_result){
                case 0:
                    //무조건 차단
                    isMailicious = true;
                    break;
                case 1:
                    //Office 프로그램이 아닐 경우 통과(Office 365, adobe reader)
                    isMailicious = IsOfficeProgram(pid);
                    break;
                case 2:
                    //모두 허용
                    isMailicious = false;
                    break;
                default:
                    //ADS:NOSPEAR가 없을 때
                    if (IsOfficeProgram(pid) == false) {
                        //Office Program이 아니라면 ADS:NOSPEAR = 1부착
                        //또한, CREATE일 때 ProcessName을 이용하여 ADS:Zone.Identifier 부착
                        //근데 CREATE일 때 ADS에 접근할 수 있을까?
                        WriteNospearADS(path, 1);

                        isMailicious = false;
                    }
                    else {
                        //CREATE일 때 ADS:NOSPEAR = 2부착 후 PASS


                        //OPEN이라면 검사 진행
                    
                    
                    }

                    break;
            }

            //실시간 검사를 종료한 상태에서는 항상 통과시킴
            if (threadstatus == false) isMailicious = false;
        }
        
        //최종결과를 History남기는 로직 구현 필요 : 클라이언트의 스레드로 넘기기

        replyMessage.ReplyHeader.Status = 0;
        replyMessage.ReplyHeader.MessageId = message->MessageHeader.MessageId;
        replyMessage.Reply.SafeToOpen = !isMailicious;

        AfxTrace(TEXT("Replying message\n"));
  
        hr = FilterReplyMessage(Context->Port, (PFILTER_REPLY_HEADER)&replyMessage, sizeof(FILTER_REPLY_HEADER) + 1);
        //원래  sizeof( replyMessage ) 이거였는데, 21로 나와서 오류남. 17이 나와야 정상

        if (SUCCEEDED(hr)) {
            AfxTrace(TEXT("Replied message\n"));
        }
        else {
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
