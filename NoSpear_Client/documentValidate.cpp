#include "pch.h"
#include "documentValidate.h"


DocumentValidate::DocumentValidate()
{

    headerFileSignatureHex.push_back(0x25); // PDF
    headerFileSignatureHex.push_back(0x50);
    headerFileSignatureHex.push_back(0x44);
    headerFileSignatureHex.push_back(0x46);
    headerFileSignatureHexV.push_back(headerFileSignatureHex);
    headerFileSignatureHex.clear();

    headerFileSignatureHex.push_back(0x0f); // PPT
    headerFileSignatureHex.push_back(0x00);
    headerFileSignatureHex.push_back(0xE8);
    headerFileSignatureHex.push_back(0x03);
    headerFileSignatureHexV.push_back(headerFileSignatureHex);
    headerFileSignatureHex.clear();

    headerFileSignatureHex.push_back(0x0D); //DOC
    headerFileSignatureHex.push_back(0x44);
    headerFileSignatureHex.push_back(0x4F);
    headerFileSignatureHex.push_back(0x43);
    headerFileSignatureHexV.push_back(headerFileSignatureHex);
    headerFileSignatureHex.clear();

    headerFileSignatureHex.push_back(0x50); // "XLSX or PPTX or DOCX or ZIP"
    headerFileSignatureHex.push_back(0x4B);
    headerFileSignatureHex.push_back(0x03);
    headerFileSignatureHex.push_back(0x04);
    headerFileSignatureHex.push_back(0x14);
    headerFileSignatureHexV.push_back(headerFileSignatureHex);
    headerFileSignatureHex.clear();


    headerFileSignatureHex.push_back(0xD0); // "HWP XLS PPT DOC etc.."
    headerFileSignatureHex.push_back(0xCF);
    headerFileSignatureHex.push_back(0x11);
    headerFileSignatureHex.push_back(0xE0);
    headerFileSignatureHex.push_back(0xA1);
    headerFileSignatureHex.push_back(0xB1);
    headerFileSignatureHex.push_back(0x1A);
    headerFileSignatureHex.push_back(0xE1);
    headerFileSignatureHexV.push_back(headerFileSignatureHex);
    headerFileSignatureHex.clear();


}

bool DocumentValidate::readSignature(std::string filePath)
{
    std::ifstream readFile;
    std::cout << filePath << std::endl;
    readFile.open(filePath, std::ios::binary);
    bool fileType = false;
    if (readFile.is_open())
    {

        int sw = 0;
        int binary = 0;

        while (!readFile.eof())
        {




            binary = readFile.get();
            if (sw == 0) {
                for (uint32_t i = 0; i < headerFileSignatureHexV.size(); i++) {
                    if (headerFileSignatureHexV[i][0] == binary) {
                        binary = readFile.get();

                        for (uint32_t x = 1; x < headerFileSignatureHexV[i].size(); x++) {

                            if (headerFileSignatureHexV[i][x] == binary) binary = readFile.get();
                            else break;

                            if (x == headerFileSignatureHexV[i].size() - 1) {
                                fileType = true;
                                sw = 1;
                            }

                        }

                    }
                }
            }

            if (sw == 1)break;
        }
    }

    readFile.close();
    return fileType;
}