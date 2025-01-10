#include "pch.h"
#include "MapsView.h"
#include "StringHelper.h"

CString CMapsView::GetColumnText(HWND hWnd, int row, int column) const {
	auto& m = m_Maps[row];
	switch (static_cast<ColumnType>(GetColumnManager(m_List)->GetColumnTag(column))) {
		case ColumnType::Name: return CString(m.Name.c_str());
		case ColumnType::Id: return std::to_wstring(m.Id).c_str();
		case ColumnType::Type: return StringHelper::MapTypeToString(m.Type);
		case ColumnType::MaxEntries: return std::to_wstring(m.MaxEntries).c_str();
		case ColumnType::KeySize: return std::to_wstring(m.KeySize).c_str();
		case ColumnType::ValueSize: return std::to_wstring(m.ValueSize).c_str();
		case ColumnType::PinnedPathCount: return std::to_wstring(m.PinnedPathCount).c_str();
	}
	return L"";
}

LRESULT CMapsView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	m_hWndClient = m_Splitter.Create(m_hWnd, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN);
	m_List.Create(m_Splitter, rcDefault, nullptr,
		WS_CHILD | WS_VISIBLE | LVS_OWNERDATA | LVS_REPORT);
	m_List.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_HEADERDRAGDROP);

	m_Splitter.SetSplitterPosPct(25);
	m_Splitter.SetSplitterPane(0, m_List);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"Name", LVCFMT_LEFT, 140, ColumnType::Name);
	cm->AddColumn(L"ID", LVCFMT_RIGHT, 60, ColumnType::Id);
	cm->AddColumn(L"Type", LVCFMT_LEFT, 100, ColumnType::Type);
	cm->AddColumn(L"Max Entries", LVCFMT_RIGHT, 80, ColumnType::MaxEntries);
	cm->AddColumn(L"Key Size", LVCFMT_RIGHT, 60, ColumnType::KeySize);
	cm->AddColumn(L"Value Size", LVCFMT_RIGHT, 60, ColumnType::ValueSize);
	cm->AddColumn(L"Pinned Paths", LVCFMT_RIGHT, 80, ColumnType::PinnedPathCount);

	Refresh();

	return 0;
}

LRESULT CMapsView::OnRefresh(WORD, WORD, HWND, BOOL&) {
	Refresh();
	return 0;
}

void CMapsView::Refresh() {
	m_Maps = BPF::EnumMaps();
	m_List.SetItemCount((int)m_Maps.size());
}
