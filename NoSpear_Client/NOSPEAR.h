class NOSPEAR {
	const char* SERVER_IP = "127.0.0.1";
	const short SERVER_PORT = 42524;
	static const short FILE_BUFFER_SIZE = 4096;
	static const unsigned int FILE_UPLOAD_MAX_SIZE = 20971520; //20MB

	void Deletefile(NOSPEAR_FILE file);

public:
	void Fileupload(NOSPEAR_FILE file);
};

