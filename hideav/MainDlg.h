// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "DropFileHandler.h"

class CMainDlg : public CDialogImpl<CMainDlg>, public CUpdateUI<CMainDlg>,
	public CMessageFilter, public CIdleHandler, public CDropFilesHandler<CMainDlg>
{
public:
	enum { IDD = IDD_MAINDLG };

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();

	BEGIN_UPDATE_UI_MAP(CMainDlg)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_CLOSE,OnClose);
		COMMAND_ID_HANDLER(IDC_HIDE, OnHide)
		COMMAND_ID_HANDLER(IDC_RESTORE, OnRestore)
		// Send WM_DROPFILES to its handler...
		CHAIN_MSG_MAP(CDropFilesHandler<CMainDlg>)
	END_MSG_MAP()

	CListBox m_ListBox;
	BOOL IsReadyForDrop(void) { m_ListBox.ResetContent(); return TRUE; }
	BOOL HandleDroppedFile(LPCSTR szBuff);
	void EndDropFiles(void);
// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnHide(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnRestore(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	
	void CloseDialog(int nVal);
};
