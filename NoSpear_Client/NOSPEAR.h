#include "sqlite3.h"

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
	std::queue<CString> request_diagnose_queue;

public:
	NOSPEAR();
	NOSPEAR(std::string ip, unsigned short port);
	void ActivateLiveProtect(bool status);
	DIAGNOSE_RESULT SingleDiagnose(CString file);
	std::vector<DIAGNOSE_RESULT> MultipleDiagnose(std::vector<CString> files);

};

