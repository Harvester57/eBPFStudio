#include "pch.h"
#include "MapsView.h"
#include "StringHelper.h"
#include <format>
#include "resource.h"

CString CMapsView::GetColumnText(HWND hWnd, int row, int column) const {
	if (hWnd == m_MapDataList)
		return GetColumnTextMapData(row, column);

	auto& m = m_Maps[row];
	switch (static_cast<ColumnType>(GetColumnManager(m_MapList)->GetColumnTag(column))) {
		case ColumnType::Name: return CString(m.Name.c_str());
		case ColumnType::Id: return std::to_wstring(m.Id).c_str();
		case ColumnType::Type: return StringHelper::MapTypeToString(m.Type);
		case ColumnType::MaxEntries: return std::to_wstring(m.MaxEntries).c_str();
		case ColumnType::KeySize: return std::to_wstring(m.KeySize).c_str();
		case ColumnType::ValueSize: return std::to_wstring(m.ValueSize).c_str();
		case ColumnType::PinnedPathCount: return std::to_wstring(m.PinnedPathCount).c_str();
		case ColumnType::Flags: return std::format("0x{:X}", m.Flags).c_str();
	}
	return L"";
}

CString CMapsView::GetColumnTextMapData(int row, int column) const {
	auto& m = m_MapData[row];
	auto& map = m_Maps[m_MapList.GetSelectedIndex()];

	switch (static_cast<ColumnType>(GetColumnManager(m_MapDataList)->GetColumnTag(column))) {
		case ColumnType::Id: return std::to_wstring(m.Index + 1).c_str();
		case ColumnType::Key: return map.KeySize <= 8 ? StringHelper::FormatNumber(m.Key.get(), map.KeySize).c_str() : L"";
		case ColumnType::Value: return map.ValueSize <= 8 ? StringHelper::FormatNumber(m.Value.get(), map.ValueSize).c_str() : L"";
		case ColumnType::KeyHex: return StringHelper::BufferToHexString(m.Key.get(), map.KeySize).c_str();
		case ColumnType::ValueHex: return StringHelper::BufferToHexString(m.Value.get(), map.ValueSize).c_str();
	}
	return L"";
}

int CMapsView::GetRowImage(HWND hWnd, int row, int column) const {
	if (hWnd == m_MapDataList)
		return -1;
	return m_Maps[row].IsPerCpu() ? 0 : 1;
}

void CMapsView::OnStateChanged(HWND hWnd, int from, int to, UINT oldState, UINT newState) {
	if (hWnd == m_MapList) {
		if (newState & LVIS_SELECTED) {
			UpdateMapData(to);
		}
	}
}

void CMapsView::UpdateMapData(int row) {
	auto& map = m_Maps[row];

	m_MapData = BPF::GetMapData(map.Id);
	m_MapDataList.SetItemCount((int)m_MapData.size());
}

LRESULT CMapsView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	m_hWndClient = m_Splitter.Create(m_hWnd, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN);
	m_MapList.Create(m_Splitter, rcDefault, nullptr,
		WS_CHILD | WS_VISIBLE | LVS_OWNERDATA | LVS_REPORT | LVS_SHOWSELALWAYS);
	m_MapList.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_HEADERDRAGDROP);

	m_MapDataList.Create(m_Splitter, rcDefault, nullptr,
		WS_CHILD | WS_VISIBLE | LVS_OWNERDATA | LVS_REPORT | LVS_SHOWSELALWAYS);
	m_MapDataList.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_HEADERDRAGDROP);

	CImageList images;
	images.Create(16, 16, ILC_COLOR32 | ILC_MASK, 2, 2);
	UINT ids[] = { IDI_CPU, IDI_SYSTEM };
	for (auto id : ids)
		images.AddIcon(AtlLoadIconImage(id, 0, 16, 16));
	m_MapList.SetImageList(images, LVSIL_SMALL);

	m_Splitter.SetSplitterPosPct(25);
	m_Splitter.SetSplitterPanes(m_MapList, m_MapDataList);

	auto cm = GetColumnManager(m_MapList);
	cm->AddColumn(L"Name", LVCFMT_LEFT, 140, ColumnType::Name);
	cm->AddColumn(L"ID", LVCFMT_RIGHT, 60, ColumnType::Id);
	cm->AddColumn(L"Type", LVCFMT_LEFT, 100, ColumnType::Type);
	cm->AddColumn(L"Max Entries", LVCFMT_RIGHT, 80, ColumnType::MaxEntries);
	cm->AddColumn(L"Key Size", LVCFMT_RIGHT, 60, ColumnType::KeySize);
	cm->AddColumn(L"Value Size", LVCFMT_RIGHT, 60, ColumnType::ValueSize);
	cm->AddColumn(L"Pinned Paths", LVCFMT_RIGHT, 80, ColumnType::PinnedPathCount);
	cm->AddColumn(L"Flags", LVCFMT_RIGHT, 70, ColumnType::Flags);

	cm = GetColumnManager(m_MapDataList);
	cm->AddColumn(L"Dummy", 0, 0, 0);
	cm->AddColumn(L"#", LVCFMT_RIGHT, 60, ColumnType::Id);
	cm->AddColumn(L"Key", LVCFMT_RIGHT, 130, ColumnType::Key);
	cm->AddColumn(L"Key (Hex)", LVCFMT_LEFT, 200, ColumnType::KeyHex);
	cm->AddColumn(L"Value", LVCFMT_RIGHT, 130, ColumnType::Value);
	cm->AddColumn(L"Value (Hex)", LVCFMT_LEFT, 200, ColumnType::ValueHex);

	cm->DeleteColumn(0);

	Refresh();

	return 0;
}

LRESULT CMapsView::OnRefresh(WORD, WORD, HWND, BOOL&) {
	Refresh();
	return 0;
}

void CMapsView::Refresh() {
	m_Maps = BPF::EnumMaps();

	m_MapList.SetItemCount((int)m_Maps.size());
}
