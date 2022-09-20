class NOSPEAR_FILE {
	static const short FILE_BUFFER_SIZE = 4096;
	CString filename;
	CString filepath;
	char filehash[65] = { 0, }; //sha256 is 64byte

public:
	NOSPEAR_FILE(CString filepath);
	CString Getfilename();
	CString Getfilepath();
	char* Getfilehash();
};