class NOSPEAR {
	std::string SERVER_IP = "15.165.158.76";
	unsigned short SERVER_PORT = 42524;
	static const short FILE_BUFFER_SIZE = 4096;
	void Deletefile(NOSPEAR_FILE file);
	CString errormsg;
	LIVEPROTECT* liveprotect = NULL;
	bool live_protect_status = false;

public:
	NOSPEAR();
	NOSPEAR(std::string ip, unsigned short port);
	int Fileupload(NOSPEAR_FILE file);
	void ActivateLiveProtect(bool status);
	CString GetErrorMsg();

	//inode 기반 질의하는 함수 필요
	//로컬 SQLite를 확인해보고, 없을 경우 서버에 질의함
};

