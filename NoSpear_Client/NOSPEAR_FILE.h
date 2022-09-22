class NOSPEAR_FILE {
	static const short FILE_BUFFER_SIZE = 4096;
	static const unsigned int FILE_UPLOAD_MAX_SIZE = 20971520; //20MB
	CString filename;
	CString filepath;
	char filehash[65] = { 0, }; //sha256 is 64byte

public:
	NOSPEAR_FILE(CString filepath);
	CString Getfilename();
	CString Getfilepath();
	char* Getfilehash();
	bool Checkvalidation();
};