#include <fltKernel.h>
#include <dontuse.h>
#include <suppress.h>
#include "../inc/maopian.h"

#pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")


//PFLT_FILTER filterHandle;
PWCHAR prefixName = L"2";//要隐藏的文件夹名字

typedef struct _HAV_DATA
{
	PDRIVER_OBJECT DriverObject;

	// The filter handle that results from a call to FltRegisterFilter
	PFLT_FILTER Filter;
	PFLT_PORT ServerPort;
	PEPROCESS UserProcess;
	PFLT_PORT ClientPort;
} HAV_DATA,*PHAV_DATA;

HAV_DATA HavData;

/*************************************************************************
    Prototypes
*************************************************************************/

NTSTATUS DriverEntry ( __in PDRIVER_OBJECT DriverObject, __in PUNICODE_STRING RegistryPath );
NTSTATUS PtUnload ( __in FLT_FILTER_UNLOAD_FLAGS Flags );
                                                             
FLT_POSTOP_CALLBACK_STATUS 
HideFilePostDirCtrl ( 
__inout PFLT_CALLBACK_DATA Data,
__in PCFLT_RELATED_OBJECTS FltObjects,
__in_opt PVOID CompletionContext,
__in FLT_POST_OPERATION_FLAGS Flags );
    
#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, PtUnload)
#endif

CONST FLT_OPERATION_REGISTRATION Callbacks[] =
{
    { IRP_MJ_DIRECTORY_CONTROL,
	  0,
      NULL,
      HideFilePostDirCtrl },

    { IRP_MJ_OPERATION_END }
};

CONST FLT_REGISTRATION FilterRegistration =
{
    sizeof( FLT_REGISTRATION ),         //  Size
    FLT_REGISTRATION_VERSION,           //  Version
	0,                                  //  Flags
    NULL,                               //  Context
    Callbacks,                          //  Operation callbacks
    PtUnload,                           //  MiniFilterUnload
    NULL,                               //  InstanceSetup
    NULL,                               //  InstanceQueryTeardown
    NULL,                               //  InstanceTeardownStart
    NULL,                               //  InstanceTeardownComplete
    NULL,                               //  GenerateFileName
    NULL,                               //  GenerateDestinationFileName
    NULL                                //  NormalizeNameComponent
};

NTSTATUS HavConnect(__in PFLT_PORT ClientPort,
					__in_opt PVOID ServerPortCookie,
					__in_bcount_opt(SizeOfContext) PVOID ConnectionContext,
					__in ULONG SizeOfContext,
					__deref_out_opt PVOID *ConnectionCookie)
{
	PAGED_CODE();

	UNREFERENCED_PARAMETER(ServerPortCookie);
	UNREFERENCED_PARAMETER(ConnectionContext);
	UNREFERENCED_PARAMETER(SizeOfContext);
	UNREFERENCED_PARAMETER(ConnectionCookie);

	ASSERT(HavData.ClientPort == NULL);
	ASSERT(HavData.UserProcess == NULL);

	// set the user process and port
	HavData.UserProcess = PsGetCurrentProcess();
	HavData.ClientPort = ClientPort;

	KdPrint(("!!!maopian.sys --- connected,port = 0x%p\n",ClientPort));
	return STATUS_SUCCESS;
}

VOID HavDisconnect(__in_opt PVOID ConnectionCookie)
{
	UNREFERENCED_PARAMETER(ConnectionCookie);
	PAGED_CODE();
	KdPrint(("!!! maopian.sys --- disconnected,port = 0x%p\n",HavData.ClientPort));

	FltCloseClientPort(HavData.Filter,&HavData.ClientPort);

	HavData.UserProcess = NULL;
}

NTSTATUS HavMessage(__in PVOID ConnectionCookie,
					__in_bcount_opt(InputBufferSize) PVOID InputBuffer,
					__in ULONG InputBufferSize,
					__out_bcount_part_opt(OutputBufferSize,*ReturnOutputBufferLength) PVOID OutputBuffer,
					__in ULONG OutputBufferSize,
					__out PULONG ReturnOutputBufferLength)
{
	HAV_COMMAND command;
	NTSTATUS status;
	PAGED_CODE();

	UNREFERENCED_PARAMETER(ConnectionCookie);
	if ((InputBuffer != NULL) && (InputBufferSize >= (FIELD_OFFSET(COMMAND_MESSAGE,Command)+sizeof(HAV_COMMAND))))
	{
		try
		{
			// Probe and capture input message: the message is raw user mode
			// buffer, so need to protect with exception handler
			command = ((PCOMMAND_MESSAGE)InputBuffer)->Command;
		}
		except (EXCEPTION_EXECUTE_HANDLER)
		{
			return GetExceptionCode();
		}
		switch (command)
		{
		case SetDir:
			break;
		case GetVersion:
			if ((OutputBufferSize < sizeof(HAVVER) || OutputBuffer == NULL))
			{
				status = STATUS_INVALID_PARAMETER;
				break;
			}	

			// Validate Buffer alignment. If a minifilter cares about
			// the alignment value of the buffer pointer they must do
			// this check themselves. Note that a try/except will
			// not capture alignment faults.
			if (!IS_ALIGNED(OutputBuffer,sizeof(ULONG)))
			{
				status = STATUS_DATATYPE_MISALIGNMENT;
				break;
			}

			// Protect access to raw user-mode output buffer with
			// an exception handler
			try 
			{
				((PHAVVER)OutputBuffer)->Major = MAJ_VERSION;
				((PHAVVER)OutputBuffer)->Minor = MIN_VERSION;
			}
			except(EXCEPTION_EXECUTE_HANDLER)
			{
				return GetExceptionCode();
			}

			*ReturnOutputBufferLength = sizeof(HAVVER);
			status = STATUS_SUCCESS;
			break;
		default:
			status = STATUS_INVALID_PARAMETER;
			break;
		}
	}
	else
		status = STATUS_INVALID_PARAMETER;

	return status;
}

NTSTATUS DriverEntry ( __in PDRIVER_OBJECT DriverObject, __in PUNICODE_STRING RegistryPath )
{
    NTSTATUS status;
	UNICODE_STRING uniString;
	PSECURITY_DESCRIPTOR sd;
	OBJECT_ATTRIBUTES oa;

    UNREFERENCED_PARAMETER( RegistryPath );

    status = FltRegisterFilter( DriverObject, &FilterRegistration, &HavData.Filter );
	if (!NT_SUCCESS(status))
		return status;

	RtlInitUnicodeString(&uniString,HAV_PORT_NAME);
	status = FltBuildDefaultSecurityDescriptor(&sd,FLT_PORT_ALL_ACCESS);
	if (NT_SUCCESS(status))
	{
		InitializeObjectAttributes(&oa,
								   &uniString,
								   OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
								   NULL,
								   sd);
		status = FltCreateCommunicationPort(HavData.Filter,
											&HavData.ServerPort,
											&oa,
											NULL,
											HavConnect,
											HavDisconnect,
											HavMessage,1);

		// Free the security descriptor in all cases. It is not needed once
		// the call to FltCreateCommunicationPort() is made.
		FltFreeSecurityDescriptor(sd);
		if (NT_SUCCESS( status ))
		{
			status = FltStartFiltering( HavData.Filter );

			if (NT_SUCCESS( status ))
				return STATUS_SUCCESS;

			FltCloseCommunicationPort(HavData.ServerPort);	
		}

		FltUnregisterFilter(HavData.Filter);
		return status;
	}



    return status;
}

NTSTATUS PtUnload ( __in FLT_FILTER_UNLOAD_FLAGS Flags )
{
    UNREFERENCED_PARAMETER( Flags );
    PAGED_CODE();

	FltCloseCommunicationPort(HavData.ServerPort);

    FltUnregisterFilter( HavData.Filter );

    return STATUS_SUCCESS;
}

FLT_POSTOP_CALLBACK_STATUS 
HideFilePostDirCtrl ( 
__inout PFLT_CALLBACK_DATA Data,
__in PCFLT_RELATED_OBJECTS FltObjects,
__in_opt PVOID CompletionContext,
__in FLT_POST_OPERATION_FLAGS Flags )
{
    ULONG nextOffset = 0;
    int modified = 0;
    int removedAllEntries = 1;
    PVOID SafeBuffer;
    


    PFILE_BOTH_DIR_INFORMATION currentFileInfo = 0;
    PFILE_BOTH_DIR_INFORMATION nextFileInfo = 0;
    PFILE_BOTH_DIR_INFORMATION previousFileInfo = 0;

    
    UNICODE_STRING fileName;
    
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( CompletionContext );

    if( FlagOn( Flags, FLTFL_POST_OPERATION_DRAINING ) )
    {
        return FLT_POSTOP_FINISHED_PROCESSING;
    }
    
    //vista或win7返回的FileInformationClass结构不再是FileBothDirectoryInformation.
    //而是FileidBothDirectoryInformation
    if( Data->Iopb->MinorFunction == IRP_MN_QUERY_DIRECTORY && 
        (Data->Iopb->Parameters.DirectoryControl.QueryDirectory.FileInformationClass == FileBothDirectoryInformation ) &&
        Data->Iopb->Parameters.DirectoryControl.QueryDirectory.Length > 0 &&
        NT_SUCCESS(Data->IoStatus.Status))
    {
        if (Data->Iopb->Parameters.DirectoryControl.QueryDirectory.MdlAddress != NULL)
        {
            SafeBuffer=MmGetSystemAddressForMdlSafe( 
                         Data->Iopb->Parameters.DirectoryControl.QueryDirectory.MdlAddress,
                         NormalPagePriority );            
        }
        else
         {
             SafeBuffer=Data->Iopb->Parameters.DirectoryControl.QueryDirectory.DirectoryBuffer;             
         }     
        
        if(SafeBuffer==NULL)
        {
            return FLT_POSTOP_FINISHED_PROCESSING;
        }
        
        currentFileInfo = (PFILE_BOTH_DIR_INFORMATION)SafeBuffer;

        previousFileInfo = currentFileInfo;
            
        do
        {
            //Byte offset of the next FILE_BOTH_DIR_INFORMATION entry
            nextOffset = currentFileInfo->NextEntryOffset;

           
            nextFileInfo = (PFILE_BOTH_DIR_INFORMATION)((PCHAR)(currentFileInfo) + nextOffset);
			//	如果要隐藏的文件夹在FILE_BOTH_DIR_INFORMATION的第一个情况 需要特殊处理
			if((previousFileInfo == currentFileInfo) && 
				(_wcsnicmp(currentFileInfo->FileName,prefixName,wcslen(prefixName))==0 && 
				(currentFileInfo->FileNameLength == 2)))
			{
				RtlCopyMemory(currentFileInfo->FileName,L".",2);
				currentFileInfo->FileNameLength =0;
				FltSetCallbackDataDirty( Data );
				return FLT_POSTOP_FINISHED_PROCESSING;
			}
			
            //若满足条件，隐藏之 
            if(_wcsnicmp(currentFileInfo->FileName,prefixName,wcslen(prefixName))==0 && (currentFileInfo->FileNameLength == 2))
            {                
                if( nextOffset == 0 )
                {
                    previousFileInfo->NextEntryOffset = 0;
                }
                else
                {
                    previousFileInfo->NextEntryOffset = (ULONG)((PCHAR)currentFileInfo - (PCHAR)previousFileInfo) + nextOffset;
                }
                
                modified = 1;                
            }
            else
            {
                removedAllEntries = 0;
                //前驱结点指针后移 
                previousFileInfo = currentFileInfo;                
            }
            //当前指针后移 
            currentFileInfo = nextFileInfo;
        } while( nextOffset != 0 );

        if( modified )
        {
            if( removedAllEntries )
            {
                Data->IoStatus.Status = STATUS_NO_MORE_FILES;
            }
            else
            {
                FltSetCallbackDataDirty( Data );
            }
        }
    }
    
    return FLT_POSTOP_FINISHED_PROCESSING;
}