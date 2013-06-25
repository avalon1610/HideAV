// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "aboutdlg.h"
#include "MainDlg.h"

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
