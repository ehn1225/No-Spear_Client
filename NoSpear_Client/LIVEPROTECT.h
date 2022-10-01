#define SCANNER_DEFAULT_REQUEST_COUNT       5
#define SCANNER_DEFAULT_THREAD_COUNT        2
#define SCANNER_MAX_THREAD_COUNT            64
#include "scanuk.h"

using namespace std;

typedef struct _SCANNER_THREAD_CONTEXT {
    HANDLE Port;
    HANDLE Completion;
} SCANNER_THREAD_CONTEXT, * PSCANNER_THREAD_CONTEXT;

typedef struct _SCANNER_MESSAGE {
    FILTER_MESSAGE_HEADER MessageHeader;
    SCANNER_NOTIFICATION Notification;
    OVERLAPPED Ovlp;
} SCANNER_MESSAGE, * PSCANNER_MESSAGE;

typedef struct _SCANNER_REPLY_MESSAGE {
    FILTER_REPLY_HEADER ReplyHeader;
    SCANNER_REPLY Reply;
} SCANNER_REPLY_MESSAGE, * PSCANNER_REPLY_MESSAGE;

//typedef struct _CACHE {
//    CString filepath;       //file 경로
//    time_t ctime;         //file Create time
//    time_t last_use;     //캐시를 사용한 마지막 시간(time out용)
//}CACHE;

static bool threadstatus = false;
//static vector <CACHE> cache_table;
static set<time_t> cache_table;

class LIVEPROTECT{
    const DWORD requestCount = SCANNER_DEFAULT_REQUEST_COUNT;
    const DWORD threadCount = SCANNER_DEFAULT_THREAD_COUNT;
    HANDLE threads[SCANNER_MAX_THREAD_COUNT];
    SCANNER_THREAD_CONTEXT context;
    PSCANNER_MESSAGE msg;
    static BOOL IsMaliciousLocal(CString filepath, bool& rediagnose);
    static BOOL IsMaliciousOnline(CString filepath);
    static PWCHAR GetCharPointerW(PWCHAR pwStr, WCHAR wLetter, int Count);
    static DWORD ScannerWorker(PSCANNER_THREAD_CONTEXT Context);
    HANDLE port, completion;

public:
    LIVEPROTECT();
    ~LIVEPROTECT();
    int ActivateLiveProtect();
    int InActivateLiveProtect();
};