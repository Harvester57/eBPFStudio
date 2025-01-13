#include "pch.h"
#include "LinksView.h"
#include "StringHelper.h"
#include <SortHelper.h>
#include "resource.h"

CString CLinksView::GetColumnText(HWND hWnd, int row, int column) const {
	auto& link = m_Links[row];
	switch (static_cast<ColumnType>(GetColumnManager(m_List)->GetColumnTag(column))) {
		case ColumnType::Id: return std::to_wstring(link.Id).c_str();
		case ColumnType::ProgramId: return std::to_wstring(link.ProgramId).c_str();
		case ColumnType::Type: return StringHelper::LinkTypeToString(link.Type);
		case ColumnType::AttachType: return StringHelper::AttachTypeToString(link.AttachType);
		case ColumnType::Data: return std::format("0x{:X}", link.AttachData).c_str();
		case ColumnType::ProgramGuid: return StringHelper::GuidToString(link.ProgramTypeUuid);
		case ColumnType::AttachTypeGuid: return StringHelper::GuidToString(link.AttachTypeUuid);
	}
	return L"";
}

void CLinksView::DoSort(SortInfo const* si) {
	auto sort = [&](auto const& p1, auto const& p2) {
		switch (static_cast<ColumnType>(GetColumnManager(m_List)->GetColumnTag(si->SortColumn))) {
			case ColumnType::Id: return SortHelper::Sort(p1.Id, p2.Id, si->SortAscending);
			case ColumnType::ProgramId: return SortHelper::Sort(p1.ProgramId, p2.ProgramId, si->SortAscending);
			case ColumnType::Type: return SortHelper::Sort(StringHelper::LinkTypeToString(p1.Type), StringHelper::LinkTypeToString(p2.Type), si->SortAscending);
			case ColumnType::AttachType: return SortHelper::Sort(StringHelper::AttachTypeToString(p1.AttachType), StringHelper::AttachTypeToString(p2.AttachType), si->SortAscending);
		}
		return false;
		};
	std::ranges::sort(m_Links, sort);
}

int CLinksView::GetRowImage(HWND, int row, int) const {
	return 0;
}

LRESULT CLinksView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	m_hWndClient = m_List.Create(m_hWnd, rcDefault, nullptr,
		WS_CHILD | WS_VISIBLE | LVS_OWNERDATA | LVS_REPORT);
	m_List.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_HEADERDRAGDROP);

	CImageList images;
	images.Create(16, 16, ILC_COLOR32 | ILC_MASK, 2, 2);
	UINT ids[] = { IDI_LINK };
	for (auto id : ids)
		images.AddIcon(AtlLoadIconImage(id, 0, 16, 16));
	m_List.SetImageList(images, LVSIL_SMALL);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"", 0, 0, 0);
	cm->AddColumn(L"ID", LVCFMT_RIGHT, 60, ColumnType::Id);
	cm->AddColumn(L"Type", LVCFMT_LEFT, 100, ColumnType::Type);
	cm->AddColumn(L"Attach Type", LVCFMT_LEFT, 150, ColumnType::AttachType);
	cm->AddColumn(L"Program ID", LVCFMT_RIGHT, 70, ColumnType::ProgramId);
	cm->AddColumn(L"Data", LVCFMT_RIGHT, 100, ColumnType::Data);
	cm->AddColumn(L"Attach Type GUID", LVCFMT_LEFT, 200, ColumnType::AttachTypeGuid);
	cm->AddColumn(L"Program Type GUID", LVCFMT_LEFT, 200, ColumnType::ProgramGuid);
	cm->DeleteColumn(0);

	Refresh();

	return 0;
}

LRESULT CLinksView::OnRefresh(WORD, WORD, HWND, BOOL&) {
    Refresh();

    return 0;
}

void CLinksView::Refresh() {
	m_Links = BpfSystem::EnumLinks();
	m_List.SetItemCount((int)m_Links.size());
}
