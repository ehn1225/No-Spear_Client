#define _CRT_SECURE_NO_WARNINGS
#include "pch.h"
#include "NOSPEAR_FILE.h"
#include "documentValidate.h"
#include "sha256.h"

NOSPEAR_FILE::NOSPEAR_FILE(CString filepath) {
	this->filepath = filepath;
	this->filename = PathFindFileName(filepath);

	//SHA256 계산부
	unsigned int readsize = 0;
	unsigned char filebuffer[FILE_BUFFER_SIZE] = {0, };
	unsigned char digest[SHA256::DIGEST_SIZE] = {0, };
	char temp[65] = { 0, }; //sha256 is 64byte

	SHA256 ctx = SHA256();
	FILE* fp = _wfopen(filepath, L"rb");
	ctx.init();
	while ((readsize = fread(filebuffer, 1, FILE_BUFFER_SIZE, fp)) != 0) {
		ctx.update((unsigned char*)filebuffer, readsize);
	}
	ctx.final(digest);

	for (int i = 0; i < SHA256::DIGEST_SIZE; i++)
		sprintf(temp + i * 2, "%02x", digest[i]);
	//취약 포인트. sprintf에서 sprintf_s로 변경해야 함. or snprintf
	filehash = CString(temp);

	//Get file size
	fseek(fp, 0, SEEK_END);
	this->filesize = ftell(fp);
	
	fclose(fp);
}

CString NOSPEAR_FILE::Getfilename() {
	return filename;
}

CString NOSPEAR_FILE::Getfilepath() {
	return filepath;
}

CString NOSPEAR_FILE::Getfilehash() {
	return filehash;
}

bool NOSPEAR_FILE::Checkvalidation(){
	FILE* fp = _wfopen(filepath, L"rb");
	if (fp == NULL) {
		AfxTrace(TEXT("[NOSPEAR_FILE::Checkvalidation] 파일이 유효하지 않음\n"));
		return false;
	}

	fseek(fp, 0, SEEK_END);
	filesize = ftell(fp);
	fclose(fp);

	if (filesize > FILE_UPLOAD_MAX_SIZE) {
		AfxTrace(TEXT("[NOSPEAR_FILE::Checkvalidation] 업로드 허용 용량 초과\n"));
		return false;
	}

	DocumentValidate document_validation;
	if (document_validation.readSignature(std::string(CT2CA(filepath))) == false) {
		AfxTrace(TEXT("[NOSPEAR_FILE::Checkvalidation] 허용된 문서 타입이 아님\n"));
		return false;
	}
	
	return true;
}

unsigned int NOSPEAR_FILE::Getfilesize(){
	return filesize;
}
bool NOSPEAR_FILE::Quarantine() {
	FILE* fpread = _wfopen(filepath, L"rb");
	FILE* fpwrite = _wfopen(L".\\Quarantine\\" + filehash, L"wb");
	if (fpread == NULL || fpwrite == NULL)
		return false;

	unsigned char filebuffer[FILE_BUFFER_SIZE] = { 0, };
	int readsize = 0;

	while ((readsize = fread(filebuffer, 1, FILE_BUFFER_SIZE, fpread)) != 0) {
		for (int i = 0; i < readsize; i++)
			filebuffer[i] = filebuffer[i] ^ 'N';
		fwrite(filebuffer, 1, readsize, fpwrite);
	}

	fclose(fpread);
	fclose(fpwrite);
	return true;
}

bool NOSPEAR_FILE::InQuarantine(CString filepath) {
	FILE* fpread = _wfopen(this->filepath, L"rb");
	FILE* fpwrite = _wfopen(filepath, L"wb");
	if (fpread == NULL || fpwrite == NULL)
		return false;

	unsigned char filebuffer[FILE_BUFFER_SIZE] = { 0, };
	int readsize = 0;

	while ((readsize = fread(filebuffer, 1, FILE_BUFFER_SIZE, fpread)) != 0) {
		for (int i = 0; i < readsize; i++)
			filebuffer[i] = filebuffer[i] ^ 'N';
		fwrite(filebuffer, 1, readsize, fpwrite);
	}

	fclose(fpread);
	fclose(fpwrite);
	return true;
}
bool NOSPEAR_FILE::BackUp() {
	FILE* fpread = _wfopen(filepath, L"rb");
	FILE* fpwrite = _wfopen(L".\\Backup\\" + filehash, L"wb");
	if (fpread == NULL || fpwrite == NULL)
		return false;

	unsigned char filebuffer[FILE_BUFFER_SIZE] = { 0, };
	int readsize = 0;

	while ((readsize = fread(filebuffer, 1, FILE_BUFFER_SIZE, fpread)) != 0) {
		for (int i = 0; i < readsize; i++)
			filebuffer[i] = filebuffer[i] ^ 'N';
		fwrite(filebuffer, 1, readsize, fpwrite);
	}

	fclose(fpread);
	fclose(fpwrite);
	return true;
}
bool NOSPEAR_FILE::Recovery(CString filepath) {
	FILE* fpread = _wfopen(this->filepath, L"rb");
	FILE* fpwrite = _wfopen(filepath, L"wb");
	if (fpread == NULL || fpwrite == NULL)
		return false;

	unsigned char filebuffer[FILE_BUFFER_SIZE] = { 0, };
	int readsize = 0;

	while ((readsize = fread(filebuffer, 1, FILE_BUFFER_SIZE, fpread)) != 0) {
		for (int i = 0; i < readsize; i++)
			filebuffer[i] = filebuffer[i] ^ 'N';
		fwrite(filebuffer, 1, readsize, fpwrite);
	}

	fclose(fpread);
	fclose(fpwrite);
	return true;
	return false;
}

CString NOSPEAR_FILE::GetfileRegNumber() {
	struct tm* clock;
	struct _stat attrib;
	_wstat(filepath, &attrib);

	clock = localtime(&(attrib.st_ctime));
	CString result;
	result.Format(TEXT("%d%02d%02d-%02d%02d%02d"), clock->tm_year + 1900, clock->tm_mon + 1, clock->tm_mday, clock->tm_hour, clock->tm_min, clock->tm_sec);

	return result;
}