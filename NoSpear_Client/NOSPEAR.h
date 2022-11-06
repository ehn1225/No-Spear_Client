#ifndef NOSPEAR_H
#define NOSPEAR_H
class LIVEPROTECT;
class NOSPEAR_FILE;
class DIAGNOSE_RESULT;
class SQLITE;
class NOSPEAR {
	NOTIFYICONDATA nid;
	std::string SERVER_IP = "15.164.98.211";
	unsigned short SERVER_PORT = 42524;
	static const short FILE_BUFFER_SIZE = 4096;
	CString clientUpdateUrl = L"http://4nul.org:3000/";
	//http://4nul.org:3000/version
	//http://4nul.org:3000/NoSpear_Client.2.2.3.4.exe
	void Deletefile(CString filepath);
	LIVEPROTECT* liveprotect = NULL;
	SQLITE* nospearDB;
	bool live_protect_status = false;
	bool FileUpload(NOSPEAR_FILE& file);
	std::queue<CString> request_diagnose_queue;

public:
	NOSPEAR();
	~NOSPEAR();
	NOSPEAR(std::string ip, unsigned short port);
	void ActivateLiveProtect(bool status);
	bool Diagnose(NOSPEAR_FILE& file);
	void AutoDiagnose();
	void Notification(CString title, CString body);
	SQLITE* GetSQLitePtr();
	CString GetMsgFromErrCode(short err_code);
	CString GetMsgFromNospear(short nospear);
	unsigned short ReadNospearADS(CString filepath);
	bool WriteNospearADS(CString filepath, unsigned short value);
	bool HasZoneIdentifierADS(CString filepath);
	bool WriteZoneIdentifierADS(CString filepath, CString processName);
	bool DeleteZoneIdentifierADS(CString filepath);
	void AppendDiagnoseQueue(CString filepath);
};
#endif