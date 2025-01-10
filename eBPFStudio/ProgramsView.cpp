// View.cpp : implementation of the CView class
//
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "resource.h"
#include "ProgramsView.h"
#include "StringHelper.h"

BOOL CProgramsView::PreTranslateMessage(MSG* pMsg) {
	pMsg;
	return FALSE;
}

CString CProgramsView::GetColumnText(HWND hWnd, int row, int column) const {
	auto& p = m_Programs[row];
	switch (static_cast<ColumnType>(GetColumnManager(m_List)->GetColumnTag(column))) {
		case ColumnType::Name: return CString(p.Name.c_str());
		case ColumnType::Id: return std::to_wstring(p.Id).c_str();
		case ColumnType::Type: return StringHelper::ProgramTypeToString(p.Type);
		case ColumnType::MapCount: return std::to_wstring(p.MapCount).c_str();
		case ColumnType::LinkCount: return std::to_wstring(p.LinkCount).c_str();
		case ColumnType::PinnedPathCount: return std::to_wstring(p.PinnedPathCount).c_str();
	}

	return CString();
}

LRESULT CProgramsView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	m_hWndClient = m_List.Create(m_hWnd, rcDefault, nullptr,
		WS_CHILD | WS_VISIBLE | LVS_OWNERDATA | LVS_REPORT);
	m_List.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_HEADERDRAGDROP);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"Name", LVCFMT_LEFT, 140, ColumnType::Name);
	cm->AddColumn(L"ID", LVCFMT_RIGHT, 60, ColumnType::Id);
	cm->AddColumn(L"Type", LVCFMT_LEFT, 100, ColumnType::Type);
	cm->AddColumn(L"Maps", LVCFMT_RIGHT, 60, ColumnType::MapCount);
	cm->AddColumn(L"Links", LVCFMT_RIGHT, 60, ColumnType::LinkCount);
	cm->AddColumn(L"Pinned Paths", LVCFMT_RIGHT, 80, ColumnType::PinnedPathCount);

	Refresh();

	return 0;
}

LRESULT CProgramsView::OnRefresh(WORD, WORD, HWND, BOOL&) {
	Refresh();

	return 0;
}

void CProgramsView::Refresh() {
	m_Programs = BPF::EnumPrograms();
	m_List.SetItemCount((int)m_Programs.size());
}
