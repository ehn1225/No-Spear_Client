#ifndef __SCANUK_H__
#define __SCANUK_H__

const PWSTR ScannerPortName = L"\\ScannerPort";

#define SCANNER_READ_BUFFER_SIZE   1024

typedef struct _SCANNER_NOTIFICATION {

    ULONG BytesToScan;
    ULONG Reserved;
    UCHAR Contents[SCANNER_READ_BUFFER_SIZE];
    ULONG pid;
    ULONG mode;
    
} SCANNER_NOTIFICATION, *PSCANNER_NOTIFICATION;

typedef struct _SCANNER_REPLY {

    BOOLEAN SafeToOpen;
    
} SCANNER_REPLY, *PSCANNER_REPLY;

#endif


