#pragma once

#include <FrameView.h>
#include <VirtualListView.h>
#include "IMainFrame.h"
#include <eBPF.h>
#include <CustomSplitterWindow.h>
#include <CustomTabView.h>
#include <HexControl.h>

class CObjectFileView :
	public CFrameView<CObjectFileView, IMainFrame>,
	public CVirtualListView<CObjectFileView> {
public:
	using CFrameView::CFrameView;

	bool Open(PCWSTR path = nullptr);

	DECLARE_WND_CLASS(nullptr)

	CString GetColumnText(HWND hWnd, int row, int column) const;
	int GetRowImage(HWND hWnd, int row, int column) const;
	void OnStateChanged(HWND hWnd, int from, int to, UINT oldState, UINT newState);
	void DoSort(SortInfo const* si);

	BEGIN_MSG_MAP(CObjectFileView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		CHAIN_MSG_MAP(CVirtualListView<CObjectFileView>)
		CHAIN_MSG_MAP(BaseFrame)
		ALT_MSG_MAP(1)
		COMMAND_ID_HANDLER(ID_VIEW_REFRESH, OnRefresh)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

protected:
	void UpdateProgramData(int row);

	enum class ColumnType {
		Name, Id, Section, Type, DataSize, Size, AttachType, Offset,
	};

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	void Refresh();

private:
	struct ProgramExtra {
		std::wstring Assembly;
	};

	CCustomHorSplitterWindow m_Splitter;
	CListViewCtrl m_List;
	CCustomTabView m_Tabs;
	CHexControl m_HexView;
	CEdit m_DisassemblyView;
	std::vector<BpfProgramEx> m_Programs;
	std::unordered_map<std::string, ProgramExtra> m_Extra;

	CString m_Path;
};
