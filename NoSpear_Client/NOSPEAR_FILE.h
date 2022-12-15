#pragma once
struct DIAGNOSE_RESULT {
	CString filepath;
	short result_code = 0;
	CString result_msg;
};
class NOSPEAR_FILE {
	static const short FILE_BUFFER_SIZE = 4096;
	static const unsigned int FILE_UPLOAD_MAX_SIZE = 20971520; //20MB
	CString filename;
	CString filepath;
	CString filehash;
	unsigned int filesize = 0; //MAX of type : 4GB

public:
	NOSPEAR_FILE(CString filepath);
	CString Getfilename();
	CString Getfilepath();
	CString Getfilehash();
	bool Quarantine();
	bool BackUp();
	bool Recovery(CString filepath);
	bool InQuarantine(CString filepath);
	bool Checkvalidation();
	unsigned int Getfilesize();
	DIAGNOSE_RESULT diag_result;
	CString GetfileRegNumber();
};