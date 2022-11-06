#ifndef __DOCUMENTVALIDATE_H__
#define __DOCUMENTVALIDATE_H__

class DocumentValidate
{
public:
    DocumentValidate();
    bool readSignature(std::string filePath); // 반환값 false는 문서파일이 아닌경우 true는 문서파일인 경우 / 인자값 해당 파일의 절대경로 지정

private:
    std::vector<std::vector<int>> headerFileSignatureHexV;
    std::vector<int> headerFileSignatureHex;
    std::vector<std::string> fileSignature;
};

#endif // DOCUMENTVALIDATE_H
