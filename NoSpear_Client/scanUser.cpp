#include "pch.h"
#include "scanuk.h"
#include "scanuser.h"
//
//  Default and Maximum number of threads.
//

#define SCANNER_DEFAULT_REQUEST_COUNT       5
#define SCANNER_DEFAULT_THREAD_COUNT        2
#define SCANNER_MAX_THREAD_COUNT            64
#pragma comment(lib,"fltLib.lib")

typedef struct _SCANNER_THREAD_CONTEXT {

    HANDLE Port;
    HANDLE Completion;

} SCANNER_THREAD_CONTEXT, *PSCANNER_THREAD_CONTEXT;


PWCHAR GetCharPointerW(PWCHAR pwStr, WCHAR wLetter, int Count){
	int i = 0, j = 0;
	int cchStr = (int)wcslen(pwStr);

	for(j = 0; j < Count; j++){
		while(pwStr[i] != wLetter && pwStr[i] != 0)
		{
			++i;
		}
		++i;
	}

	return (PWCHAR)&pwStr[--i];
}

DWORD
ScannerWorker(
    __in PSCANNER_THREAD_CONTEXT Context
)
/*++

Routine Description

    This is a worker thread that


Arguments

    Context  - This thread context has a pointer to the port handle we use to send/receive messages,
                and a completion port handle that was already associated with the comm. port by the caller

Return Value

    HRESULT indicating the status of thread exit.

--*/
{
    PSCANNER_NOTIFICATION notification;
    SCANNER_REPLY_MESSAGE replyMessage;
    PSCANNER_MESSAGE message;
    LPOVERLAPPED pOvlp;
    BOOL result;
    DWORD outSize;
    HRESULT hr;
    ULONG_PTR key;
    int i = 0;
    unsigned char array[1024] = { 0, };


#pragma warning(push)
#pragma warning(disable:4127) // conditional expression is constant

    while (TRUE) {

#pragma warning(pop)

        //
        //  Poll for messages from the filter component to scan.
        //

        result = GetQueuedCompletionStatus( Context->Completion, &outSize, &key, &pOvlp, INFINITE );

        //
        //  Obtain the message: note that the message we sent down via FltGetMessage() may NOT be
        //  the one dequeued off the completion queue: this is solely because there are multiple
        //  threads per single port handle. Any of the FilterGetMessage() issued messages can be
        //  completed in random order - and we will just dequeue a random one.
        //

        message = CONTAINING_RECORD( pOvlp, SCANNER_MESSAGE, Ovlp );

        if (!result) {

            hr = HRESULT_FROM_WIN32( GetLastError() );
            break;
        }

        AfxTrace(TEXT("Received message, size %d\n", pOvlp->InternalHigh));

        notification = &message->Notification;
        assert(notification->BytesToScan <= SCANNER_READ_BUFFER_SIZE);
        __analysis_assume(notification->BytesToScan <= SCANNER_READ_BUFFER_SIZE);

		{
			WCHAR wDosFilePath[512] = {0,};
			
			PWCHAR pwPtr = GetCharPointerW((PWCHAR)notification->Contents, L'\\', 3);
            //3번째 /부터 시작하는 notification->Contents 위치 반환
			if(!pwPtr)
			{
				hr = -1;
				break;
			}
			
			*pwPtr = L'\0';
            //3번째 / 기준으로 좌우 쪼갬
			
			hr = FilterGetDosName((PWCHAR)notification->Contents, wDosFilePath, 512);
			if(FAILED(hr))
				break;
            AfxTrace(TEXT("FilePath1 : %s\n", wDosFilePath));
			wcscat(wDosFilePath, L"\\");
			wcscat(wDosFilePath, ++pwPtr); //파일 경로
            //C 뒤에 \와, 일반적으로 아는 경로를 붙혀줌
			
            AfxTrace(TEXT("FilePath2 : %s\n", wDosFilePath));

            memcpy(array, notification->Contents, 1024);
			
            result = FALSE;
            //result == true -> 차단
            //result == false -> 통과
		}

        replyMessage.ReplyHeader.Status = 0;
        replyMessage.ReplyHeader.MessageId = message->MessageHeader.MessageId;

        //  Need to invert the boolean -- result is true if found

        replyMessage.Reply.SafeToOpen = !result;
        //foul이 있으면 true를 리턴하니까, false를 리턴해서 안전하지 않다고 알림

        AfxTrace(TEXT( "Replying message, SafeToOpen: %d\n", replyMessage.Reply.SafeToOpen));


        hr = FilterReplyMessage( Context->Port,
                                 (PFILTER_REPLY_HEADER) &replyMessage,
                                 sizeof( replyMessage ) );

        if (SUCCEEDED( hr )) {
            AfxTrace(TEXT( "Replied message\n"));
        } 
        else {
            AfxTrace(TEXT( "Scanner: Error replying message. Error = 0x%X\n", hr));
            break;
        }

        memset( &message->Ovlp, 0, sizeof( OVERLAPPED ) );

        hr = FilterGetMessage( Context->Port,
                               &message->MessageHeader,
                               FIELD_OFFSET( SCANNER_MESSAGE, Ovlp ),
                               &message->Ovlp );

        if (hr != HRESULT_FROM_WIN32( ERROR_IO_PENDING )) {
            break;
        }
    }

    if (!SUCCEEDED( hr )) {

        if (hr == HRESULT_FROM_WIN32( ERROR_INVALID_HANDLE )) {
            //  Scanner port disconncted.
            AfxTrace(TEXT("Scanner: Port is disconnected, probably due to scanner filter unloading.\n"));
        } 
        else {
            AfxTrace(TEXT("Scanner: Unknown error occured.Error = 0x % X\n", hr));
        }
    }

    free( message );

    return hr;
}

/*
int _cdecl
main (
    __in int argc,
    __in_ecount(argc) char *argv[]
    )
{
    DWORD requestCount = SCANNER_DEFAULT_REQUEST_COUNT;
    DWORD threadCount = SCANNER_DEFAULT_THREAD_COUNT;
    HANDLE threads[SCANNER_MAX_THREAD_COUNT];
    SCANNER_THREAD_CONTEXT context;
    HANDLE port, completion;
    PSCANNER_MESSAGE msg;
    DWORD threadId;
    HRESULT hr;
    DWORD i, j;


    //
    //  Open a commuication channel to the filter
    //

    AfxTrace(TEXT( "Scanner: Connecting to the filter ...\n"));

    hr = FilterConnectCommunicationPort( ScannerPortName,
                                         0,
                                         NULL,
                                         0,
                                         NULL,
                                         &port );

    if (IS_ERROR( hr )) {

        AfxTrace(TEXT("ERROR: Connecting to filter port: 0x%08x\n", hr));
        return 2;
    }

    //
    //  Create a completion port to associate with this handle.
    //

    completion = CreateIoCompletionPort( port,
                                         NULL,
                                         0,
                                         threadCount );

    if (completion == NULL) {

        AfxTrace(TEXT("ERROR: Creating completion port: %d\n", GetLastError()));
        CloseHandle( port );
        return 3;
    }

    AfxTrace(TEXT("Scanner: Port = 0x%p Completion = 0x%p\n", port, completion));

    context.Port = port;
    context.Completion = completion;

    //
    //  Create specified number of threads.
    //

    for (i = 0; i < threadCount; i++) {

        threads[i] = CreateThread( NULL,
                                   0,
                                   (LPTHREAD_START_ROUTINE)ScannerWorker,
                                   &context,
                                   0,
                                   &threadId );

        if (threads[i] == NULL) {

            //
            //  Couldn't create thread.
            //

            hr = GetLastError();
            AfxTrace(TEXT("ERROR: Couldn't create thread: %d\n", hr));
            goto main_cleanup;
        }

        for (j = 0; j < requestCount; j++) {

            //
            //  Allocate the message.
            //

#pragma prefast(suppress:__WARNING_MEMORY_LEAK, "msg will not be leaked because it is freed in ScannerWorker")
            msg = (PSCANNER_MESSAGE) malloc( sizeof( SCANNER_MESSAGE ) );

            if (msg == NULL) {

                hr = ERROR_NOT_ENOUGH_MEMORY;
                goto main_cleanup;
            }

            memset( &msg->Ovlp, 0, sizeof( OVERLAPPED ) );

            //
            //  Request messages from the filter driver.
            //

            hr = FilterGetMessage( port,
                                   &msg->MessageHeader,
                                   FIELD_OFFSET( SCANNER_MESSAGE, Ovlp ),
                                   &msg->Ovlp );

            if (hr != HRESULT_FROM_WIN32( ERROR_IO_PENDING )) {

                free( msg );
                goto main_cleanup;
            }
        }
    }

    hr = S_OK;

    WaitForMultipleObjectsEx( i, threads, TRUE, INFINITE, FALSE );

main_cleanup:

    AfxTrace(TEXT("Scanner:  All done. Result = 0x%08x\n", hr));

    CloseHandle( port );
    CloseHandle( completion );

    return hr;
}

*/