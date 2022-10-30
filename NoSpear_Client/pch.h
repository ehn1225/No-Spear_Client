#ifndef PCH_H
#define PCH_H

// 여기에 미리 컴파일하려는 헤더 추가
#include "framework.h"
#include <fstream>
#include <wininet.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <process.h>
#include <commctrl.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <Windows.h>
#include <sys/stat.h>
#include <time.h>
#include <set>
#include <map>
#include <filesystem>
#include <queue>
#include <stdlib.h>
#include <winioctl.h>
#include <fltuser.h>
#include "scanuk.h"

enum E_MALWARE_TYPE {
	TYPE_NORMAL,
	TYPE_MALWARE,
	TYPE_SUSPICIOUS,
	TYPE_UNEXPECTED,
	TYPE_NOFILE,
	TYPE_RESEND,
	TYPE_REJECT
};

struct NOSPEAR_HISTORY {
	time_t eventtime;
	time_t createtime;
	time_t modifytime;
	time_t access_time;

};

//MFC XP 스타일 지정
//그래픽적으로 조금 느려진 것 같음
#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

#endif //PCH_H
