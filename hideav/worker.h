#pragma once
#include "stdafx.h"

#ifdef __cplusplus
extern "C" {
#endif

_Must_inspect_result_
	HRESULT
	WINAPI
	FilterConnectCommunicationPort(
	_In_ LPCWSTR lpPortName,
	_In_ DWORD dwOptions,
	_In_reads_bytes_opt_(wSizeOfContext) LPCVOID lpContext,
	_In_ WORD wSizeOfContext,
	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes ,
	_Outptr_ HANDLE *hPort
	);

#ifdef __cplusplus
};
#endif

typedef struct _HAV_CONTEXT {
	HANDLE Port;
	BOOLEAN CleaningUp;
	HANDLE ShutDown;
} HAV_CONTEXT,*PHAV_CONTEXT;