// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "aboutdlg.h"
#include "MainDlg.h"

void OperaterFilter(char [][MAX_PATH]);
typedef BOOL (WINAPI *_ChangeWindowMessageFilter)(UINT,DWORD);
BOOL AllowMessageForVista(UINT uMessageID,BOOL bAllow)
{
	BOOL bResult = FALSE;
	HMODULE hUserMod = NULL;
	// vista and later
	hUserMod = LoadLibrary("user32.dll");
	if (NULL == hUserMod)
		return FALSE;
	_ChangeWindowMessageFilter pChangeWindowMessageFilter 
		= (_ChangeWindowMessageFilter)GetProcAddress(hUserMod,"ChangeWindowMessageFilter");
	if (NULL == pChangeWindowMessageFilter)
		return FALSE;
	bResult = pChangeWindowMessageFilter(uMessageID,bAllow ? 1 : 2); // MSGFLT_ADD:1,MSGFLT_REMOVE:2
	if (NULL != hUserMod)
		FreeLibrary(hUserMod);
	return bResult;
}

BOOL CMainDlg::PreTranslateMessage(MSG* pMsg)
{
	return CWindow::IsDialogMessage(pMsg);
}

BOOL CMainDlg::OnIdle()
{
	return FALSE;
}

LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// center the dialog on the screen
	CenterWindow();

	// set icons
	HICON hIcon = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON));
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON));
	SetIcon(hIconSmall, FALSE);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	UIAddChildWindowContainer(m_hWnd);
	
	m_ListBox.Attach(GetDlgItem(IDC_LIST));
	if (AllowMessageForVista(WM_DROPFILES,TRUE))
	{
		AllowMessageForVista(WM_COPYDATA,TRUE);
		AllowMessageForVista(0x0049,TRUE);
	}
	return TRUE;
}

LRESULT CMainDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// unregister message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);

	return 0;
}

LRESULT CMainDlg::OnClose(UINT , WPARAM , LPARAM , BOOL& )
{
	CloseDialog(0);
	return 0;
}

LRESULT CMainDlg::OnHide(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add validation code 
	char data[DIR_COUNT][MAX_PATH] = {0};
	CString temp;
	int count = m_ListBox.GetCount();
	for (int i = 0; i < count; i++)
	{
		m_ListBox.GetText(i,temp);
		if (temp)
			memcpy_s(data[i],MAX_PATH,temp.GetBuffer(0),temp.GetLength());
	}
	
	OperaterFilter(data);
	return 0;
}

LRESULT CMainDlg::OnRestore(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CloseDialog(wID);
	return 0;
}

void CMainDlg::CloseDialog(int nVal)
{
	DestroyWindow();
	::PostQuitMessage(nVal);
}

BOOL CMainDlg::HandleDroppedFile( LPCSTR szBuff )
{
	ATLTRACE("%s\n",szBuff);
	m_ListBox.AddString(szBuff);
	// Return TRUE unless you're done handling files (e.g., if you want 
	// to handle only the first relevant file, 
	// and you have already found it).
	return TRUE;
}

void CMainDlg::EndDropFiles( void )
{
	// Sometimes, if your file handling is not trivial,  
	// you might want to add all
	// file names to some container (std::list<CString> comes to mind), 
	// and do the 
	// handling afterwards, in a worker thread. 
	// If so, use this function to create your worker thread.
	CWindow wnd;
	wnd.Attach(GetDlgItem(IDC_COUNT));
	CHAR buff[30] = {0};
	sprintf_s(buff,"%d",m_ListBox.GetCount());
	wnd.SetWindowText(buff);
}
