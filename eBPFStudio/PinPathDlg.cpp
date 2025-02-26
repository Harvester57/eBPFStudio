#include "pch.h"
#include "resource.h"
#include "PinPathDlg.h"


CPinPathDlg::CPinPathDlg(PCWSTR path) : m_Path(path) {
}

CString const& CPinPathDlg::GetPath() const {
	return m_Path;
}

LRESULT CPinPathDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	CenterWindow(GetParent());

	SetDlgItemText(IDC_PATH, m_Path);

	return TRUE;
}

LRESULT CPinPathDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if (wID == IDOK) {
		GetDlgItemText(IDC_PATH, m_Path);
	}
	EndDialog(wID);
	return 0;
}
