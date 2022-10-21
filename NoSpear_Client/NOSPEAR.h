struct DIAGNOSE_RESULT {
	short result_code = 0;
	CString result_msg;
};
class NOSPEAR_FILE;
class NOSPEAR {
	std::string SERVER_IP = "15.164.98.211";
	unsigned short SERVER_PORT = 42524;
	static const short FILE_BUFFER_SIZE = 4096;
	void Deletefile(NOSPEAR_FILE file);
	LIVEPROTECT* liveprotect = NULL;
	bool live_protect_status = false;
	DIAGNOSE_RESULT FileUpload(CString file);
	void GetMsgFromCode(DIAGNOSE_RESULT& result);
	//클라이언트 스레드에서 아래 큐에서 가져온 것들을 실시간으로 검사 진행함
	//드라이버 스레드에서 이 큐에 값을 넣음.
	std::queue<CString> request_diagnose_queue;

public:
	NOSPEAR();
	NOSPEAR(std::string ip, unsigned short port);
	void ActivateLiveProtect(bool status);
	DIAGNOSE_RESULT SingleDiagnose(CString file);
	std::vector<DIAGNOSE_RESULT> MultipleDiagnose(std::vector<CString> files);

};

