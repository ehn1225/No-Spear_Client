#include "pch.h"
#include "LIVEPROTECT.h"
#include "NoSpear_ClientDlg.h"
#pragma comment(lib,"fltLib.lib")

BOOL inline LIVEPROTECT::IsMaliciousLocal(CString filepath, bool& rediagnose) {
    //파일이 위험하다고 판단되면 true, 아니면 false를 리턴합니다.
    //1. 캐시확인(whitelist)
    //2. While list 확인
    //3. 블랙리스트 확인

    //1. 캐시 확인. Create Time을 기준으로 검색해보고 동일한게 존재한다면 Pass
    struct __stat64 buffer;
    _wstat64(filepath, &buffer);
    time_t file_ctime = buffer.st_ctime;
    time_t now = time(nullptr);

    //vector<CACHE>::iterator it;
    //for (it = cache_table.begin(); it != cache_table.end(); it++) {
    //    if (it->ctime == file_ctime) {
    //        //Create Time을 비교해보고, 동일한게 있다면 통과
    //        it->last_use = now;
    //        return false;
    //    }
    //    if (now - it->last_use > 300) {
    //        //5분 동안 사용안할 경우 캐시에서 지움
    //        cache_table.erase(it);
    //    }
    //}

    //set을 쓰는 경우 원소 값 삭제불가능
    // find는 존재하지 않으면 set.end()를 리턴. cache_table에 존재한다면 바로 통과함
    if (cache_table.find(file_ctime) != cache_table.end()) {
        return false;
    }

    //2.Local 확인. SQLite를 이용해 DB 검사해봄
    //key=filepath, filename, create time, 서버 검사 여부

    //3.Local BlackList 확인. SQLite를 이용해 DB 검사해봄
    //key=hash, diagnose code(blacklist만 있으면 code는 없음(악성코드 여부)

    //여기 까지 왔으면 일단 블락 때림.
    //이제 서버에 물어봐야하는 단계
    //네트워크에 연결안된 상태라면 사용자 선택에 따라 진행
    rediagnose = true;
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
    BOOL result;
    DWORD outSize;
    HRESULT hr;
    ULONG_PTR key;

#pragma warning(push)
#pragma warning(disable:4127) // conditional expression is constant

    while (threadstatus) {

#pragma warning(pop)

        result = GetQueuedCompletionStatus(Context->Completion, &outSize, &key, &pOvlp, INFINITE);
        message = CONTAINING_RECORD(pOvlp, SCANNER_MESSAGE, Ovlp);

        if (!result) {
            hr = HRESULT_FROM_WIN32(GetLastError());
            break;
        }
        //CString tmp;
        //tmp.Format(TEXT("Received message, size %d\n"), pOvlp->InternalHigh);
        //AfxTrace(tmp);
        notification = &message->Notification;
        assert(notification->BytesToScan <= SCANNER_READ_BUFFER_SIZE);
        __analysis_assume(notification->BytesToScan <= SCANNER_READ_BUFFER_SIZE);
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
            wcscat(wDosFilePath, ++pwPtr); //파일경로
            //C 뒤에 \와, 일반적으로 아는 경로를 붙혀줌

            CString tmp;
            tmp.Format(TEXT("pid : %d, process name : %ws, FilePath : %s\n"), pid, GetProcessName(pid), wDosFilePath);
            AfxTrace(tmp);
            //AfxMessageBox(tmp);
            CString path = CString(wDosFilePath);
            //AfxMessageBox(CString(wDosFilePath));
            bool rediagnose = false;
            result = IsMaliciousLocal(path, rediagnose);
            //네트워크가 차단되거나, 서버에 업로드 처리 진행
            //true -> 차단, false -> 통과

            if (rediagnose == true) {
                //IsMalicious에서 판단할 수 없음.
                //서버에 질의를 하는 함수 호출
                //AfxMessageBox(_T("서버에 물어봐야 합니다"));
                //NOSPESR_FILE 객체 생성
            }

            //사용자와 최종 확인하는 부분(악성코드인데 삭제할래? 그래도 실행할래?, 정상이 아닐때만 출력됨)


            //최종 결과를cache에 저장해주기. 허용할 것만 추가
            //if (result == false) {
            // //vecort<CACHE> 용
            //    struct __stat64 buffer;
            //    _wstat64(path, &buffer);
            //    CACHE tmp;
            //    tmp.filepath = path;
            //    tmp.ctime = buffer.st_ctime;
            //    tmp.last_use = time(nullptr);
            //    cache_table.push_back(tmp);
            //}
            if (result == false) {
                //set용
                struct __stat64 buffer;
                _wstat64(path, &buffer);
                cache_table.insert(buffer.st_ctime);
            }

            if (threadstatus == false) {
                result = false;
                //실시간 검사를 종료한 상태에서는 항상 통과시킴
            }
        }
        
        result = false;
        //테스트용 바이패스

        replyMessage.ReplyHeader.Status = 0;
        replyMessage.ReplyHeader.MessageId = message->MessageHeader.MessageId;

        //  Need to invert the boolean -- result is true if found
        replyMessage.Reply.SafeToOpen = !result;

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
        cache_table.clear();
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
