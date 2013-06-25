#include "stdafx.h"
#include "..\Bin\KernelModule.h"
#include <Fltuser.h>
#include "..\inc\maopian.h"
#include "worker.h"

#pragma comment(lib,"fltlib")

#define SYS_NAME "maopian"

void LoadAndRun()
{
	char filePath[MAX_PATH] = {0};
	GetWindowsDirectoryA(filePath,sizeof(filePath));
	strcat_s(filePath,"\\");
	strcat_s(filePath,SYS_NAME);
	strcat_s(filePath,".sys");

	//ExtractSysFile(filePath,KernelModule,sizeof(KernelModule));
}


#pragma warning(push)
#pragma warning(disable:4996)
void ShowERR(TCHAR *format,...)
{
	TCHAR msg[1024] = {0};
	va_list args;
	va_start(args,format);
	_vstprintf(msg,format,args);
	va_end(args);
	MessageBox(NULL,msg,0,0);
}
#pragma warning(pop)

bool Init()
{
	HANDLE port = INVALID_HANDLE_VALUE;
	HAV_CONTEXT context;
	HRESULT hResult = S_OK;
	context.ShutDown = NULL;

	hResult = FilterConnectCommunicationPort(HAV_PORT_NAME,0,NULL,0,NULL,&port);
	if (IS_ERROR(hResult))
	{
		ShowERR("Could not connect to filter: 0x%08x\n",hResult);
		return false;
	}

	context.Port = port;
	context.ShutDown = CreateSemaphore(NULL,0,1,"Hav shut down");
	context.CleaningUp = FALSE;

	return true;
}