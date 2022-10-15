#include "pch.h"
#include "LIVEPROTECT.h"
#include "NoSpear_ClientDlg.h"
#pragma comment(lib,"fltLib.lib")

BOOL inline LIVEPROTECT::IsMaliciousLocal(unsigned long pid, CString filepath) {
    //ADS:NOSPEAR 을 확인하고 파일이 위험하다고 판단되면 true, 아니면 false를 리턴합니다.
    //1. Process Name 확인
    //2. ADS 유무 확인
    //3. ADS Value 확인

    //1. ProcessName 확인
    //allow explorer.exe


    //2. ADS:NOSPEAR 유무 확인 후 없으면 차단
    bool HasADS = false;
    WIN32_FIND_STREAM_DATA fsd;
    HANDLE hFind = NULL;
    try {
        hFind = ::FindFirstStreamW(filepath, FindStreamInfoStandard, &fsd, 0);
        if (hFind == INVALID_HANDLE_VALUE) throw ::GetLastError();

        for (;;) {
            CString tmp;
            tmp.Format(TEXT("%s"), fsd.cStreamName);
            if (tmp == L":NOSPEAR:$DATA") {
                HasADS = true;
            }
            if (!::FindNextStreamW(hFind, &fsd)) {
                DWORD dr = ::GetLastError();
                if (dr != ERROR_HANDLE_EOF) throw dr;
                break;
            }
        }
    }
    catch (DWORD err) {
        AfxTrace(TEXT("[LIVEPROTECT::IsMaliciousLocal] Find ADS, Windows error code: %u\n", err));
    }
    if (hFind != NULL)
        ::FindClose(hFind);

    if (HasADS == false)
        return true;

    //2. ADS Value 확인
    CStdioFile ads_stream;
    CFileException e;
    if (!ads_stream.Open(filepath + L":Zone.Identifier", CFile::modeRead, &e)) {
        e.ReportError();
    }
    CString str;
    //ads_stream.ReadString(str);
    //string to integer and compare 0 or 1

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
            CString tmp;
            tmp.Format(TEXT("pid : %d, process name : %ws, FilePath : %s\n"), pid, GetProcessName(pid), wDosFilePath);
            AfxTrace(tmp);

            CString path = CString(wDosFilePath);

            //Process Name 검사 수행. Office 프로그램이 아닐 경우, 바로 넘어감.
            //office 프로그램이 아니라면 통과, 한글, Office 365, adobe reader, this.
            //Create Time으로 현재 시각과 동일하면 새로 생성한 파일이다.
            //새로 생성한 파일은 통과시킨다.

            //Alternate Data Stream 기반 파일 허용 여부
            isMailicious = IsMaliciousLocal(pid, path);
            //true -> 차단, false -> 통과

            if (threadstatus == false) {
                isMailicious = false;
                //실시간 검사를 종료한 상태에서는 항상 통과시킴
            }
        }
        
        isMailicious = false;
        //테스트용 바이패스

        //최종결과를 History남기는 로직 구현 필요 : 클라이언트의 스레드로 넘기기
        //해당 수행 결과를 클라이언트 스레드로 넘김.
        //클라이언트 스레드는 넘오오는 값을 통해 새로운 파일이 탐지되었음을 파악하고, 사용자에게 허용할지 물어봄.
        //커널 드라이버는 멈추지 않는다.
        //빠꾸 맥이고 허용 여부 물어본다음에 다시 반복하게끔.

        replyMessage.ReplyHeader.Status = 0;
        replyMessage.ReplyHeader.MessageId = message->MessageHeader.MessageId;

        //  Need to invert the boolean -- result is true if found
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

    //  Open a commuication channel to the filter
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
