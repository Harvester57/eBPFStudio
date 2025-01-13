#pragma once

#include <FrameView.h>
#include <VirtualListView.h>
#include "IMainFrame.h"
#include <eBPF.h>

class CProgramsView : 
	public CFrameView<CProgramsView, IMainFrame>,
	public CVirtualListView<CProgramsView> {
public:
	using CFrameView::CFrameView;

	DECLARE_WND_CLASS(nullptr)

	BOOL PreTranslateMessage(MSG* pMsg);

	CString GetColumnText(HWND hWnd, int row, int column) const;

	BEGIN_MSG_MAP(CProgramsView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		CHAIN_MSG_MAP(CVirtualListView<CProgramsView>)
		CHAIN_MSG_MAP(BaseFrame)
	ALT_MSG_MAP(1)
		COMMAND_ID_HANDLER(ID_VIEW_REFRESH, OnRefresh)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

protected:
	enum class ColumnType {
		Name, Id, LinkCount, MapCount, GuidType, Type, PinnedPathCount, ExeType, Section, FileName,
	};

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	void Refresh();

private:
	CListViewCtrl m_List;
	std::vector<BpfProgram> m_Programs;
};
