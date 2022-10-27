#include <fltKernel.h>
#include <dontuse.h>
#include <suppress.h>
#include <ntifs.h>
#include "scanuk.h"
#include "scanner.h"

#pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")
SCANNER_DATA ScannerData;

const UNICODE_STRING ScannerExtensionsToScan[] =
    { //RTL_CONSTANT_STRING(L"hwp"),
      //RTL_CONSTANT_STRING(L"hwpx"),
      //RTL_CONSTANT_STRING(L"pdf"),
      RTL_CONSTANT_STRING(L"doc"),
      RTL_CONSTANT_STRING(L"docx"),
      RTL_CONSTANT_STRING(L"xls"),
      RTL_CONSTANT_STRING(L"xlsx"),
      RTL_CONSTANT_STRING(L"pptx"),
      RTL_CONSTANT_STRING(L"ppsx"),
      //RTL_CONSTANT_STRING(L"crdownload"),       //webbrowser
      RTL_CONSTANT_STRING(L"download"),             //jandi
      RTL_CONSTANT_STRING(L"part"),                 //kakaotalk
      {0, 0, NULL}
    };

NTSTATUS ScannerPortConnect (__in PFLT_PORT ClientPort, __in_opt PVOID ServerPortCookie, __in_bcount_opt(SizeOfContext) PVOID ConnectionContext, __in ULONG SizeOfContext, __deref_out_opt PVOID *ConnectionCookie);
VOID ScannerPortDisconnect ( __in_opt PVOID ConnectionCookie);
NTSTATUS ScannerpScanFileInUserMode ( __in PWCHAR pwFilePath, __in ULONG cbFilePath, __in PFLT_INSTANCE Instance, __in PFILE_OBJECT FileObject, __in ULONG pid, __in ULONG mode_num, __out PBOOLEAN SafeToOpen);
BOOLEANScannerpCheckExtension ( __in PUNICODE_STRING Extension);

#ifdef ALLOC_PRAGMA
    #pragma alloc_text(INIT, DriverEntry)
    #pragma alloc_text(PAGE, ScannerInstanceSetup)
    #pragma alloc_text(PAGE, ScannerPreCreate)
    #pragma alloc_text(PAGE, ScannerPortConnect)
    #pragma alloc_text(PAGE, ScannerPortDisconnect)
#endif

const FLT_OPERATION_REGISTRATION Callbacks[] = {
    { IRP_MJ_CREATE, 0, ScannerPreCreate, ScannerPostCreate},
    { IRP_MJ_SET_INFORMATION, 0, ScannerPreInformation, NULL},
    //{ IRP_MJ_CLEANUP, 0, ScannerPreClean, ScannerPostClean},
    { IRP_MJ_OPERATION_END}
};

const FLT_CONTEXT_REGISTRATION ContextRegistration[] = {
    { FLT_STREAMHANDLE_CONTEXT, 0, NULL, sizeof(SCANNER_STREAM_HANDLE_CONTEXT), 'chBS' },
    { FLT_CONTEXT_END }
};

const FLT_REGISTRATION FilterRegistration = {

    sizeof( FLT_REGISTRATION ),         //  Size
    FLT_REGISTRATION_VERSION,           //  Version
    0,                                  //  Flags
    ContextRegistration,                //  Context Registration.
    Callbacks,                          //  Operation callbacks
    ScannerUnload,                      //  FilterUnload
    ScannerInstanceSetup,               //  InstanceSetup
    ScannerQueryTeardown,               //  InstanceQueryTeardown
    NULL,                               //  InstanceTeardownStart
    NULL,                               //  InstanceTeardownComplete
    NULL,                               //  GenerateFileName
    NULL,                               //  GenerateDestinationFileName
    NULL                                //  NormalizeNameComponent
};

NTSTATUS
DriverEntry (__in PDRIVER_OBJECT DriverObject, __in PUNICODE_STRING RegistryPath){
    OBJECT_ATTRIBUTES oa;
    UNICODE_STRING uniString;
    PSECURITY_DESCRIPTOR sd;
    NTSTATUS status;

    UNREFERENCED_PARAMETER( RegistryPath );

    status = FltRegisterFilter( DriverObject, &FilterRegistration, &ScannerData.Filter );

    if (!NT_SUCCESS( status )) {
        return status;
    }

    RtlInitUnicodeString( &uniString, ScannerPortName );
    status = FltBuildDefaultSecurityDescriptor( &sd, FLT_PORT_ALL_ACCESS );

    if (NT_SUCCESS( status )) {
        InitializeObjectAttributes( &oa, &uniString, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, sd );

        status = FltCreateCommunicationPort( ScannerData.Filter, &ScannerData.ServerPort, &oa, NULL, ScannerPortConnect, ScannerPortDisconnect, NULL, 1 );

        FltFreeSecurityDescriptor( sd );

        if (NT_SUCCESS( status )) {
            status = FltStartFiltering( ScannerData.Filter );
            if (NT_SUCCESS( status )) {
                return STATUS_SUCCESS;
            }
            FltCloseCommunicationPort( ScannerData.ServerPort );
        }
    }

    FltUnregisterFilter( ScannerData.Filter );
    return status;
}


NTSTATUS
ScannerPortConnect (
    __in PFLT_PORT ClientPort,
    __in_opt PVOID ServerPortCookie,
    __in_bcount_opt(SizeOfContext) PVOID ConnectionContext,
    __in ULONG SizeOfContext,
    __deref_out_opt PVOID *ConnectionCookie
    )
{
    PAGED_CODE();

    UNREFERENCED_PARAMETER( ServerPortCookie );
    UNREFERENCED_PARAMETER( ConnectionContext );
    UNREFERENCED_PARAMETER( SizeOfContext);
    UNREFERENCED_PARAMETER( ConnectionCookie );

    ASSERT( ScannerData.ClientPort == NULL );
    ASSERT( ScannerData.UserProcess == NULL );

    ScannerData.UserProcess = PsGetCurrentProcess();
    ScannerData.ClientPort = ClientPort;

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "!!! scanner.sys --- connected, port=0x%p\n", ClientPort);

    return STATUS_SUCCESS;
}


VOID ScannerPortDisconnect(__in_opt PVOID ConnectionCookie){
    UNREFERENCED_PARAMETER( ConnectionCookie );
    PAGED_CODE();
    DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "!!! scanner.sys --- disconnected, port=0x%p\n", ScannerData.ClientPort);
    FltCloseClientPort( ScannerData.Filter, &ScannerData.ClientPort );
    ScannerData.UserProcess = NULL;
}


NTSTATUS ScannerUnload (__in FLT_FILTER_UNLOAD_FLAGS Flags){
    UNREFERENCED_PARAMETER( Flags );
    FltCloseCommunicationPort( ScannerData.ServerPort );
    FltUnregisterFilter( ScannerData.Filter );
    return STATUS_SUCCESS;
}

NTSTATUS ScannerInstanceSetup (
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __in FLT_INSTANCE_SETUP_FLAGS Flags,
    __in DEVICE_TYPE VolumeDeviceType,
    __in FLT_FILESYSTEM_TYPE VolumeFilesystemType
){
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( Flags );
    UNREFERENCED_PARAMETER( VolumeFilesystemType );

    PAGED_CODE();
    ASSERT( FltObjects->Filter == ScannerData.Filter );
    if (VolumeDeviceType == FILE_DEVICE_NETWORK_FILE_SYSTEM) {
       return STATUS_FLT_DO_NOT_ATTACH;
    }
    return STATUS_SUCCESS;
}

NTSTATUS
ScannerQueryTeardown (
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __in FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
){
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( Flags );

    return STATUS_SUCCESS;
}


FLT_PREOP_CALLBACK_STATUS
ScannerPreCreate (
    __inout PFLT_CALLBACK_DATA Data,
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __deref_out_opt PVOID *CompletionContext
){
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( CompletionContext );
    PAGED_CODE();

    if (IoThreadToProcess( Data->Thread ) == ScannerData.UserProcess) {
        //DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "!!! scanner.sys -- allowing create for trusted process \n");
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

BOOLEAN
ScannerpCheckExtension (__in PUNICODE_STRING Extension){
    const UNICODE_STRING *ext;

    if (Extension->Length == 0) {
        return FALSE;
    }

    ext = ScannerExtensionsToScan;

    while (ext->Buffer != NULL) {
        if (RtlCompareUnicodeString( Extension, ext, TRUE ) == 0) {
            return TRUE;
        }
        ext++;
    }

    return FALSE;
}


FLT_POSTOP_CALLBACK_STATUS
ScannerPostCreate (
    __inout PFLT_CALLBACK_DATA Data,
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __in_opt PVOID CompletionContext,
    __in FLT_POST_OPERATION_FLAGS Flags
){
    PSCANNER_STREAM_HANDLE_CONTEXT scannerContext;
    FLT_POSTOP_CALLBACK_STATUS returnStatus = FLT_POSTOP_FINISHED_PROCESSING;
    PFLT_FILE_NAME_INFORMATION nameInfo;
    NTSTATUS status;
    BOOLEAN safeToOpen, scanFile;
	WCHAR wFilePath[512] = {0,};
	ULONG cbFilePath = 0;
    ULONG pid = 0;
    ULONG mode_num = 0;
    WCHAR wStream[256] = { 0, };
    ULONG stream_len = 0;
    //Mode
    //open : 0
    //create : 1
    //delete : 2

    UNREFERENCED_PARAMETER( CompletionContext );
    UNREFERENCED_PARAMETER( Flags );

    if (!NT_SUCCESS( Data->IoStatus.Status ) || (STATUS_REPARSE == Data->IoStatus.Status)) {
        return FLT_POSTOP_FINISHED_PROCESSING;
    }

    status = FltGetFileNameInformation( Data,FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &nameInfo );

    if (!NT_SUCCESS( status )) {
        return FLT_POSTOP_FINISHED_PROCESSING;
    }

    FltParseFileNameInformation( nameInfo );

    scanFile = ScannerpCheckExtension( &nameInfo->Extension );

	if(scanFile){
        //if (nameInfo->Stream.Length != 0) {
        //    stream_len = min((512 - 1) * sizeof(WCHAR), nameInfo->Stream.Length);
        //    RtlCopyMemory(wStream, nameInfo->Stream.Buffer, stream_len);
        //    DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "PASS : Stream is non 0 % ws\n", wStream);
        //    FltReleaseFileNameInformation(nameInfo);
        //    return FLT_POSTOP_FINISHED_PROCESSING;
        //}

		cbFilePath = min((512-1)*sizeof(WCHAR), nameInfo->Name.Length);
		RtlCopyMemory(wFilePath, nameInfo->Name.Buffer, cbFilePath);
    }

    FltReleaseFileNameInformation( nameInfo );

    if (!scanFile || nameInfo->Stream.Length != 0) {
        //Stream이 다를 경우(ADS가 있을 경우, 파일에 대한 접근이 아니므로 통과)
        return FLT_POSTOP_FINISHED_PROCESSING;
    }

    pid = PtrToUint(PsGetCurrentProcessId());

    if (Data->IoStatus.Information == FILE_OPENED) {
        mode_num = 0;
    }
    else if (Data->IoStatus.Information == FILE_CREATED) {
        mode_num = 1;
    }
    else {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[scanner.sys] " __FUNCTION__ "[% u] ELSEE a file(% ws)\n", pid, wFilePath);
    }

    (VOID)ScannerpScanFileInUserMode(wFilePath, cbFilePath, FltObjects->Instance, FltObjects->FileObject, pid, mode_num, &safeToOpen);

    if (safeToOpen) {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[scanner.sys] " __FUNCTION__ "[% u] % ws a file(% ws)\n", pid, (mode_num == 0) ? L"OPEN" : L"CREATE", wFilePath);
    }

    if (!safeToOpen) {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[scanner.sys] " __FUNCTION__ "[% u] % ws a file is BLOCKED (% ws)\n", pid, (mode_num == 0) ? L"OPEN" : L"CREATE", wFilePath);
        FltCancelFileOpen( FltObjects->Instance, FltObjects->FileObject );
        Data->IoStatus.Status = STATUS_ACCESS_DENIED;
        Data->IoStatus.Information = 0;
        returnStatus = FLT_POSTOP_FINISHED_PROCESSING;
    }
    else if (FltObjects->FileObject->WriteAccess) {
        status = FltAllocateContext( ScannerData.Filter, FLT_STREAMHANDLE_CONTEXT, sizeof(SCANNER_STREAM_HANDLE_CONTEXT), PagedPool, &scannerContext );

        if (NT_SUCCESS(status)){
            scannerContext->RescanRequired = TRUE;
            (VOID) FltSetStreamHandleContext( FltObjects->Instance, FltObjects->FileObject, FLT_SET_CONTEXT_REPLACE_IF_EXISTS, scannerContext, NULL );
            FltReleaseContext( scannerContext );
        }
    }

    return returnStatus;
}

FLT_PREOP_CALLBACK_STATUS
ScannerPreInformation(
    __inout PFLT_CALLBACK_DATA Data,
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __deref_out_opt PVOID* CompletionContext
){
    PFLT_FILE_NAME_INFORMATION nameInfo;
    NTSTATUS status;
    WCHAR wFilePath[512] = { 0, };
    ULONG cbFilePath = 0;
    BOOLEAN safeToOpen, scanFile;
    ULONG pid = 0;

    if (Data->Iopb->Parameters.SetFileInformation.FileInformationClass == FileDispositionInformation) {
        if (((FILE_DISPOSITION_INFORMATION*) Data->Iopb->Parameters.SetFileInformation.InfoBuffer)->DeleteFile) {

            status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &nameInfo);

            if (!NT_SUCCESS(status)) {
                return FLT_PREOP_SUCCESS_NO_CALLBACK;
            }

            FltParseFileNameInformation(nameInfo);

            scanFile = ScannerpCheckExtension(&nameInfo->Extension);

            if (scanFile) {
                cbFilePath = min((512 - 1) * sizeof(WCHAR), nameInfo->Name.Length);
                RtlCopyMemory(wFilePath, nameInfo->Name.Buffer, cbFilePath);
            }

            FltReleaseFileNameInformation(nameInfo);

            if (!scanFile) {
                return FLT_PREOP_SUCCESS_NO_CALLBACK;
            }
            pid = PtrToUint(PsGetCurrentProcessId());
            DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[scanner.sys] " __FUNCTION__ "[% u] Deleted a file(% ws)\n", pid, wFilePath);

            (VOID)ScannerpScanFileInUserMode(wFilePath, cbFilePath, FltObjects->Instance, FltObjects->FileObject, pid, 2, &safeToOpen);

        }
    }

    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}

NTSTATUS
ScannerpScanFileInUserMode (
	__in PWCHAR pwFilePath,
	__in ULONG cbFilePath,
    __in PFLT_INSTANCE Instance,
    __in PFILE_OBJECT FileObject,
    __in ULONG pid,
    __in ULONG mode_num,
    __out PBOOLEAN SafeToOpen
){
    NTSTATUS status = STATUS_SUCCESS;
    PVOID buffer = NULL;
    ULONG bytesRead;
    PSCANNER_NOTIFICATION notification = NULL;
    FLT_VOLUME_PROPERTIES volumeProps;
    LARGE_INTEGER offset;
    ULONG replyLength, length;
    PFLT_VOLUME volume = NULL;

    *SafeToOpen = TRUE;

    if (ScannerData.ClientPort == NULL) {
        return STATUS_SUCCESS;
    }

    try {
        status = FltGetVolumeFromInstance( Instance, &volume );

        if (!NT_SUCCESS( status )) {
            leave;
        }

        status = FltGetVolumeProperties( volume, &volumeProps, sizeof( volumeProps ), &length );

        if (NT_ERROR( status )) {
            leave;
        }

        length = max( SCANNER_READ_BUFFER_SIZE, volumeProps.SectorSize );

        buffer = FltAllocatePoolAlignedWithTag( Instance, NonPagedPool, length, 'nacS' );

        if (NULL == buffer) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        }

        notification = ExAllocatePoolWithTag( NonPagedPool, sizeof( SCANNER_NOTIFICATION ), 'nacS' );

        if(NULL == notification) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        }
		
		RtlZeroMemory(notification, sizeof( SCANNER_NOTIFICATION ));

        offset.QuadPart = bytesRead = 0;
        status = FltReadFile( Instance, FileObject, &offset, length, buffer, FLTFL_IO_OPERATION_NON_CACHED | FLTFL_IO_OPERATION_DO_NOT_UPDATE_BYTE_OFFSET, &bytesRead, NULL, NULL );

        if (NT_SUCCESS( status ) && (0 != bytesRead)) {
            //notification->BytesToScan = (ULONG) bytesRead;
			notification->BytesToScan = (ULONG) cbFilePath;
            notification->pid = (ULONG)pid;
            notification->mode = (ULONG)mode_num;

            /*RtlCopyMemory( &notification->Contents,
                           buffer,
                           min( notification->BytesToScan, SCANNER_READ_BUFFER_SIZE ) );*/
			//유저에게 전송하는 영역	   
			RtlCopyMemory( &notification->Contents, pwFilePath, cbFilePath);


            replyLength = sizeof( SCANNER_REPLY );

            status = FltSendMessage( ScannerData.Filter, &ScannerData.ClientPort, notification, sizeof(SCANNER_NOTIFICATION), notification, &replyLength, NULL );

            if (STATUS_SUCCESS == status) {
                *SafeToOpen = ((PSCANNER_REPLY) notification)->SafeToOpen;
            }
            else {
                DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "!!! scanner.sys --- couldn't send message to user-mode to scan file, status 0x%X\n", status);
            }
        }

    } finally {

        if (NULL != buffer) {
            FltFreePoolAlignedWithTag( Instance, buffer, 'nacS' );
        }

        if (NULL != notification) {
            ExFreePoolWithTag( notification, 'nacS' );
        }

        if (NULL != volume) {
            FltObjectDereference( volume );
        }
    }

    return status;
}

FLT_PREOP_CALLBACK_STATUS
ScannerPreClean(
    __inout PFLT_CALLBACK_DATA Data,
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __deref_out_opt PVOID* CompletionContext
){
    PFLT_FILE_NAME_INFORMATION nameInfo;
    NTSTATUS status;
    BOOLEAN scanFile;
    WCHAR wFilePath[512] = { 0, };
    ULONG cbFilePath = 0;

    UNREFERENCED_PARAMETER(CompletionContext);

    if (!NT_SUCCESS(Data->IoStatus.Status) ||
        (STATUS_REPARSE == Data->IoStatus.Status)) {
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &nameInfo);


    if (!NT_SUCCESS(status)) {
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    FltParseFileNameInformation(nameInfo);

    scanFile = ScannerpCheckExtension(&nameInfo->Extension);

    if (scanFile) {
        cbFilePath = min((512 - 1) * sizeof(WCHAR), nameInfo->Name.Length);
        RtlCopyMemory(wFilePath, nameInfo->Name.Buffer, cbFilePath);
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[scanner.sys] " __FUNCTION__ "[% u] clean-up a file(% ws)\n", PtrToUint(PsGetCurrentProcessId()), wFilePath);
    }

    FltReleaseFileNameInformation(nameInfo);

    
    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}

FLT_POSTOP_CALLBACK_STATUS
ScannerPostClean(
    __inout PFLT_CALLBACK_DATA Data,
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __in_opt PVOID CompletionContext,
    __in FLT_POST_OPERATION_FLAGS Flags
) {
    UNREFERENCED_PARAMETER(Data);
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);
    UNREFERENCED_PARAMETER(Flags);
    DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[scanner.sys] " __FUNCTION__ "[% u])\n", PtrToUint(PsGetCurrentProcessId()));
    return FLT_POSTOP_FINISHED_PROCESSING;
}
