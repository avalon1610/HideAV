#include "stdafx.h"
#include "hideav.h"
#include <fltUser.h>
#include "..\inc\maopian.h"

#pragma comment(lib,"fltlib")

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
		ShowERR(L"Could not connect to filter: 0x%08x\n",hResult);
		return false;
	}

	context.Port = port;
	context.ShutDown = CreateSemaphore(NULL,0,1,L"Hav shut down");
	context.CleaningUp = FALSE;
	
	return true;
}