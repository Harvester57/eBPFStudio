#include "pch.h"
#include "PinsView.h"
#include "StringHelper.h"
#include <SortHelper.h>
#include "resource.h"

CString CPinsView::GetColumnText(HWND hWnd, int row, int column) const {
	auto& pin = m_Pins[row];
	switch (static_cast<ColumnType>(GetColumnManager(m_List)->GetColumnTag(column))) {
		case ColumnType::Id: return std::to_wstring(pin.Id).c_str();
		case ColumnType::Type: return StringHelper::ObjectTypeToString(pin.ObjectType);
		case ColumnType::Path: return pin.Path.c_str();
	}
	return L"";
}

void CPinsView::DoSort(SortInfo const* si) {
	auto sort = [&](auto const& p1, auto const& p2) {
		switch (static_cast<ColumnType>(GetColumnManager(m_List)->GetColumnTag(si->SortColumn))) {
			case ColumnType::Id: return SortHelper::Sort(p1.Id, p2.Id, si->SortAscending);
			case ColumnType::Type: return SortHelper::Sort(StringHelper::ObjectTypeToString(p1.ObjectType), StringHelper::ObjectTypeToString(p2.ObjectType), si->SortAscending);
			case ColumnType::Path: return SortHelper::Sort(p1.Path, p2.Path, si->SortAscending);
		}
		return false;
		};
	std::ranges::sort(m_Pins, sort);
}

int CPinsView::GetRowImage(HWND, int row, int) const {
	return (int)m_Pins[row].ObjectType - 1;
}

LRESULT CPinsView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	m_hWndClient = m_List.Create(m_hWnd, rcDefault, nullptr,
		WS_CHILD | WS_VISIBLE | LVS_OWNERDATA | LVS_REPORT);
	m_List.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_HEADERDRAGDROP);

	CImageList images;
	images.Create(16, 16, ILC_COLOR32 | ILC_MASK, 2, 2);
	UINT ids[] = { IDI_MAPS, IDI_LINK, IDR_MAINFRAME };
	for (auto id : ids)
		images.AddIcon(AtlLoadIconImage(id, 0, 16, 16));
	m_List.SetImageList(images, LVSIL_SMALL);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"", 0, 0, 0);
	cm->AddColumn(L"ID", LVCFMT_RIGHT, 60, ColumnType::Id);
	cm->AddColumn(L"Type", LVCFMT_LEFT, 100, ColumnType::Type);
	cm->AddColumn(L"Path", LVCFMT_LEFT, 300, ColumnType::Path);
	cm->DeleteColumn(0);

	Refresh();

	return 0;
}

LRESULT CPinsView::OnRefresh(WORD, WORD, HWND, BOOL&) {
	Refresh();

	return 0;
}

void CPinsView::Refresh() {
	m_Pins = BpfSystem::EnumPins();
	m_List.SetItemCount((int)m_Pins.size());
}
