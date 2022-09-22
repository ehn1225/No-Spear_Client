#include "pch.h"
#include "NOSPEAR_FILE.h"
#include "NOSPEAR.h"
#include "ssl.h"
SOCKET s;
SSL_SOCKET* sx = 0;
sockaddr_in dA, aa;
int slen = sizeof(sockaddr_in);

#pragma warning(disable:4996)
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Crypt32.lib")
#pragma comment(lib,"Secur32.lib")
#pragma comment(lib, "wininet.lib")

void NOSPEAR::Deletefile(NOSPEAR_FILE file){
	CFileFind pFind;
	CString filepath = file.Getfilepath();
	BOOL bRet = pFind.FindFile(filepath);
	if (bRet == TRUE) {
		if (DeleteFile(filepath) == TRUE) {
			AfxMessageBox(_T("삭제 완료"));
		}
	}
}

void NOSPEAR::Fileupload(NOSPEAR_FILE file){
	//SSL Socket Send
	//https://www.codeproject.com/Articles/24379/SSL-Convert-your-Plain-Sockets-to-SSL-Sockets-in-a

	CString filename = file.Getfilename();
	//validation 호출
	if (file.Checkvalidation() == false) {
		AfxTrace(TEXT("[NOSPEAR::Fileupload] 제약되는 파일으로 확인\n"));
		return;
	}

	unsigned long inaddr;

	memset(&dA, 0, sizeof(dA));
	dA.sin_family = AF_INET;
	inaddr = inet_addr(SERVER_IP);
	if (inaddr != INADDR_NONE)
		memcpy(&dA.sin_addr, &inaddr, sizeof(inaddr));

	dA.sin_port = htons(SERVER_PORT);
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(s, (sockaddr*)&dA, slen) < 0)
	{
		AfxTrace(TEXT("[NOSPEAR::Fileupload] 서버에 연결할 수 없음\n"));
		return;
	}

	getpeername(s, (sockaddr*)&aa, &slen);

	//sx = new SSL_SOCKET(s, 0);
	//sx->ClientInit();

	std::string utf8_filename = CW2A(filename, CP_UTF8);

	//Send File Name Length
	unsigned int length = htonl(utf8_filename.size());
	//sx->s_ssend((char*)namelength, 4);
	send(s, (char*)&length, 4, 0);

	//Send File Name (CString to char*)
	//UTP-8로 변경한 후 서버에 전송
	//sx->s_ssend((LPSTR)(LPCTSTR)filename, namelength * 2);
	send(s, utf8_filename.c_str(), utf8_filename.size(), 0);

	//Send File Hash
	//sx->s_ssend(file.Getfilehash(), 64);
	send(s, file.Getfilehash(), 64, 0);

	char file_buffer[NOSPEAR::FILE_BUFFER_SIZE];
	int read_size = 0;
	FILE* fp = _wfopen(file.Getfilepath(), L"rb");
	if (fp == NULL) {
		AfxTrace(TEXT("[NOSPEAR::Fileupload] 파일이 유효하지 않습니다.\n"));
		closesocket(s);
		//delete(sx);
		return;
	}

	while ((read_size = fread(file_buffer, 1, NOSPEAR::FILE_BUFFER_SIZE, fp)) != 0) {
		//sx->s_ssend(file_buffer, read_size);
		send(s, file_buffer, read_size, 0);
	}

	fclose(fp);
	closesocket(s);
	//delete(sx);
}
