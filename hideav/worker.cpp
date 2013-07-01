#include "stdafx.h"
#include "..\sys\objchk_win7_x86\i386\KernelModule.h"
#include <Fltuser.h>
#include "..\inc\maopian.h"
#include "worker.h"

#pragma comment(lib,"fltlib.lib")

#define SYS_NAME "maopian"

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

bool ExtractSysFile(char *targetPath,UCHAR *lpszCode,ULONG ulSize)
{
	DWORD dwBytesWrite;
	HANDLE hFile = CreateFile(targetPath,GENERIC_WRITE,FILE_SHARE_WRITE,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	SetFilePointer(hFile,0,0,FILE_END);
	WriteFile(hFile,lpszCode,ulSize,&dwBytesWrite,NULL);
	CloseHandle(hFile);
	return true;
}

bool WINAPI EnableDebugPriv(LPCTSTR name)
{
	HANDLE hToken;
	LUID luid;
	if (!OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,&hToken))
	{
		return FALSE;
	}

	if (!LookupPrivilegeValue(NULL,name,&luid))
	{
		return FALSE;
	}

	TOKEN_PRIVILEGES tp;
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	tp.Privileges[0].Luid = luid;
	if (!AdjustTokenPrivileges(hToken,0,&tp,sizeof(TOKEN_PRIVILEGES),NULL,NULL))
	{
		return FALSE;
	}
	return TRUE;
}

bool LoadNTDriver(char *DriverName,char *DriverPath)
{
	char szDriverImagePath[MAX_PATH] = {0};
	GetFullPathName(DriverPath,MAX_PATH,szDriverImagePath,NULL);
	bool bRet = false;

	SC_HANDLE hServiceMgr = NULL;
	SC_HANDLE hServiceDDK = NULL;

	hServiceMgr = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
	if (hServiceMgr == NULL)
	{
		ShowERR("OpenSCManager failed:%d",GetLastError());
		goto _Leave;
	}

	hServiceDDK = CreateService(hServiceMgr,
								DriverName,
								DriverName,		// DisplayName
								SERVICE_ALL_ACCESS,
								SERVICE_KERNEL_DRIVER,
								SERVICE_DEMAND_START,
								SERVICE_ERROR_IGNORE,
								szDriverImagePath,
								NULL,NULL,NULL,NULL,NULL);
	DWORD dwRtn;
	if (hServiceDDK == NULL)
	{
		dwRtn = GetLastError();
		if (dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_EXISTS)
		{
			ShowERR("CreateService Failed:%d.",dwRtn);
			goto _Leave;
		}
		else
		{
			// already have
		}

		hServiceDDK = OpenService(hServiceMgr,DriverName,SERVICE_ALL_ACCESS);
		if (hServiceDDK == NULL)
		{
			dwRtn = GetLastError();
			ShowERR("OpenService Failed:%d",dwRtn);
			goto _Leave;
		}
	}

	char szTempStr[MAX_PATH] = {0};
	strcpy_s(szTempStr,"SYSTEM\\CurrentControlSet\\Services\\");
	strcat_s(szTempStr,DriverName);
	strcat_s(szTempStr,"\\Instances");

	HKEY hKey;
	DWORD dwData;
	if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
					   szTempStr,
					   0,
					   "",
					   TRUE,
					   KEY_ALL_ACCESS,
					   NULL,
					   &hKey,
					   (LPDWORD)&dwData) != ERROR_SUCCESS)
	{
		ShowERR("RegCreateKeyEx 1 failed:%d",GetLastError());
		return FALSE;
	}

	strcpy_s(szTempStr,DriverName);
	strcat_s(szTempStr," Instance");
	if (RegSetValueEx(hKey,
					  "DefaultInstance",
					  0,
					  REG_SZ,
					  (CONST BYTE*)szTempStr,
					  (DWORD)strlen(szTempStr)) != ERROR_SUCCESS)
	{
		ShowERR("RegSetValueEx 1 failed:%d",GetLastError());
		return FALSE;
	}
	RegFlushKey(hKey);
	RegCloseKey(hKey);

	strcpy_s(szTempStr,"SYSTEM\\CurrentControlSet\\Services\\");
	strcat_s(szTempStr,DriverName);
	strcat_s(szTempStr,"\\Instances\\");
	strcat_s(szTempStr,DriverName);
	strcat_s(szTempStr," Instance");
	if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
					   szTempStr,
					   0,
					   "",
					   TRUE,
					   KEY_ALL_ACCESS,
					   NULL,
					   &hKey,
					   (LPDWORD)&dwData) != ERROR_SUCCESS)
	{
		ShowERR("RegCreateKeyEx 2 failed:%d",GetLastError());
		return FALSE;
	}

	char lpszAltitude[32] = "370030";
	strcpy_s(szTempStr,lpszAltitude);
	if (RegSetValueEx(hKey,"Altitude",0,REG_SZ,(CONST BYTE*)szTempStr,(DWORD)strlen(szTempStr)) != ERROR_SUCCESS)
	{
		ShowERR("RegSetValueEx 2 failed:%d",GetLastError());
		return FALSE;
	}

	dwData = 0x0;
	if (RegSetValueEx(hKey,"Flags",0,REG_DWORD,(CONST BYTE*)&dwData,sizeof(DWORD)) != ERROR_SUCCESS)
	{
		ShowERR("RegSetValueEx 3 failed:%d",GetLastError());
		return FALSE;
	}

	RegFlushKey(hKey);
	RegCloseKey(hKey);

	bRet = !!StartService(hServiceDDK,NULL,NULL);
	if (!bRet)
	{
		DWORD dwRtn = GetLastError();
		if (dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_ALREADY_RUNNING)
		{
			bRet = false;
			ShowERR("StartService() Failed:%d",dwRtn);
			goto _Leave;
		}
		else
		{
			if (dwRtn == ERROR_IO_PENDING)
			{
				bRet = false;
				ShowERR("StartService() Failed ERROR_IO_PENDING");
				goto _Leave;
			}
			else
			{
				bRet = true;
				goto _Leave;
			}
		}
	}
_Leave:

	if (hServiceDDK)
	{
		CloseServiceHandle(hServiceDDK);
	}
	if (hServiceMgr)
	{
		CloseServiceHandle(hServiceMgr);
	}

	return bRet;
}

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

void LoadAndRun()
{
	char filePath[MAX_PATH] = {0};
	char LoadDriverPath[MAX_PATH] = {0};
	char ServicesName[MAX_PATH] = {0};
	GetCurrentDirectory(sizeof(filePath),filePath);
	strcat_s(filePath,"\\");
	strcat_s(filePath,SYS_NAME);
	strcat_s(filePath,".sys");
	sprintf_s(ServicesName,MAX_PATH,"SYSTEM\\CurrentControlSet\\Services\\%s",SYS_NAME);
	SHDeleteKey(HKEY_LOCAL_MACHINE,ServicesName);

	ExtractSysFile(filePath,KernelModule,sizeof(KernelModule));
	if (GetFileAttributes(filePath) == INVALID_FILE_ATTRIBUTES)
	{
		ShowERR("释放文件失败.");
		ExitProcess(0);
	}

	sprintf_s(LoadDriverPath,MAX_PATH,"\\??\\%s",filePath);
	if (!EnableDebugPriv(SE_LOAD_DRIVER_NAME))
	{
		ShowERR("权限不够.");
		ExitProcess(0);
	}

	if (!LoadNTDriver(SYS_NAME,filePath))
	{
		DeleteFile(filePath);
		SHDeleteKey(HKEY_LOCAL_MACHINE,ServicesName);
		ShowERR("加载文件失败.");
		ExitProcess(0);
	}

	DeleteFile(filePath);

	Init();
}

