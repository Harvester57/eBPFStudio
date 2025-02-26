#pragma once

#include <FrameView.h>
#include <VirtualListView.h>
#include "IMainFrame.h"
#include <eBPF.h>
#include <CustomSplitterWindow.h>
#include "resource.h"

class CMapsView :
	public CFrameView<CMapsView, IMainFrame>,
	public CVirtualListView<CMapsView> {
public:
	using CFrameView::CFrameView;

	DECLARE_WND_CLASS(nullptr)

	CString GetColumnText(HWND hWnd, int row, int column) const;
	CString GetColumnTextMapData(int row, int column) const;
	int GetRowImage(HWND hWnd, int row, int column) const;
	void OnStateChanged(HWND hWnd, int from, int to, UINT oldState, UINT newState);
	void DoSort(SortInfo const* si);
	int GetSaveColumnRange(HWND, int&) const;

	BEGIN_MSG_MAP(CMapsView)
		MESSAGE_HANDLER(WM_UPDATEUI, OnUpdateUI)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		CHAIN_MSG_MAP(CVirtualListView<CMapsView>)
		CHAIN_MSG_MAP(BaseFrame)
	ALT_MSG_MAP(1)
		COMMAND_ID_HANDLER(ID_VIEW_REFRESH, OnRefresh)
		COMMAND_ID_HANDLER(ID_BPF_UNPIN, OnUnpin)
		COMMAND_ID_HANDLER(ID_BPF_PIN, OnPin)
		COMMAND_ID_HANDLER(ID_EBPF_PINWITHPATH, OnPinWithPath)
		COMMAND_ID_HANDLER(ID_EBPF_UNPINWITHPATH, OnUnpinWithPath)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

protected:
	void DoSortData(SortInfo const* si);
	void UpdateMapData(int row);

	enum class ColumnType {
		Name, Id, KeySize, ValueSize, Type, PinnedPathCount, MaxEntries, Flags,
		Key, KeyHex, Value, ValueHex, ValueChars,
	};

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPin(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnUnpin(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPinWithPath(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnUnpinWithPath(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnUpdateUI(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	void Refresh();

private:
	CCustomHorSplitterWindow m_Splitter;
	CListViewCtrl m_MapList;
	CListViewCtrl m_MapDataList;
	std::vector<BpfMap> m_Maps;
	BpfMap m_CurrentMap;
	std::vector<BpfMapItem> m_MapData;
};
