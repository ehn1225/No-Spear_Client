#define _CRT_SECURE_NO_WARNINGS

#include "pch.h"
#include "NOSPEAR_FILE.h"
#include "sha256.h"
#include <string.h>

NOSPEAR_FILE::NOSPEAR_FILE(CString filepath) {
	this->filepath = filepath;
	this->filename = PathFindFileName(filepath);

	//SHA256 계산부
	int readsize = 0;
	unsigned char filebuffer[FILE_BUFFER_SIZE] = {0, };
	unsigned char digest[SHA256::DIGEST_SIZE] = {0, };

	SHA256 ctx = SHA256();
	FILE* fp = _wfopen(filepath, L"rb");
	ctx.init();
	while ((readsize = fread(filebuffer, 1, FILE_BUFFER_SIZE, fp)) != 0) {
		ctx.update((unsigned char*)filebuffer, readsize);
	}
	ctx.final(digest);

	for (int i = 0; i < SHA256::DIGEST_SIZE; i++)
		sprintf(filehash + i * 2, "%02x", digest[i]);
	//취약 포인트. sprintf에서 sprintf_s로 변경해야 함. or snprintf
	
	fclose(fp);
}

CString NOSPEAR_FILE::Getfilename() {
	return filename;
}

CString NOSPEAR_FILE::Getfilepath() {
	return filepath;
}

char * NOSPEAR_FILE::Getfilehash() {
	return filehash;
}