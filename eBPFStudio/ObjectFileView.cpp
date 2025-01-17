#include "pch.h"
#include "ObjectFileView.h"
#include "resource.h"
#include <SortHelper.h>
#include <ranges>

CString CObjectFileView::GetColumnTextStat(int row, int column) const {
	if (m_SelectedProgram == nullptr)
		return L"";

	auto& stat = m_Extra.find(m_SelectedProgram->Name)->second.Stats[row];
	switch (static_cast<ColumnType>(GetColumnManager(m_StatList)->GetColumnTag(column))) {
		case ColumnType::Name: return CString(stat.Key.c_str());
		case ColumnType::Value: return std::to_wstring(stat.Value).c_str();
		case ColumnType::Details: return std::format(L"0x{:X}", stat.Value).c_str();
	}
	return L"";
}

void CObjectFileView::UpdateProgramData(int row) {
	if (row < 0) {
		m_SelectedProgram = nullptr;
		m_HexView.ClearAll();
		m_DisassemblyView.SetWindowTextW(L"");
		m_StatList.SetItemCount(0);
		return;
	}
	auto& p = m_Programs[row];
	m_SelectedProgram = &p;
	m_HexView.InitData(p.Data.data(), p.Data.size());
	m_StatList.SetItemCount(0);

	if (auto it = m_Extra.find(p.Name); it != m_Extra.end()) {
		m_DisassemblyView.SetWindowTextW(it->second.Assembly.c_str());
	}
	else {
		auto text = BpfSystem::DisassembleProgram(p);
		CString textw(text.c_str());
		textw.Replace(L"\n", L"\r\n");
		ProgramExtra extra;
		extra.Stats = p.Stats;
		extra.Assembly = textw;
		m_Extra.insert({ p.Name, extra });
		m_DisassemblyView.SetWindowTextW(textw);
	}
	m_StatList.SetItemCount((int)p.Stats.size());
	Sort(m_StatList);
}

LRESULT CObjectFileView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	m_hWndClient = m_Splitter.Create(m_hWnd, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN);
	m_List.Create(m_Splitter, rcDefault, nullptr,
		WS_CHILD | WS_VISIBLE | LVS_OWNERDATA | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL);
	m_List.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_HEADERDRAGDROP);

	m_Tabs.m_bTabCloseButton = false;
	m_Tabs.Create(m_Splitter, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN);

	CImageList images;
	images.Create(16, 16, ILC_COLOR32 | ILC_MASK, 2, 2);
	UINT ids[] = { IDR_MAINFRAME, IDI_BINARY, IDI_CPU, IDI_STATS };
	for (auto id : ids)
		images.AddIcon(AtlLoadIconImage(id, 0, 16, 16));
	m_List.SetImageList(images, LVSIL_SMALL);
	m_Tabs.SetImageList(images);

	m_HexView.Create(m_Tabs, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN);
	m_HexView.SetReadOnly(true);
	m_DisassemblyView.Create(m_Tabs, rcDefault, L"", WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | ES_READONLY | ES_MULTILINE | ES_WANTRETURN | WS_VSCROLL | WS_HSCROLL);
	m_DisassemblyView.SetFont(Frame()->GetMonoFont());
	m_StatList.Create(m_Tabs, rcDefault, nullptr,
		WS_CHILD | WS_VISIBLE | LVS_OWNERDATA | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL);
	m_StatList.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);


	m_Tabs.AddPage(m_HexView, L"Bytes", 1, this);
	m_Tabs.AddPage(m_DisassemblyView, L"Assembly", 2, this);
	m_Tabs.AddPage(m_StatList, L"Stats", 3, this);

	m_Splitter.SetSplitterPosPct(25);
	m_Splitter.SetSplitterPanes(m_List, m_Tabs);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"Name", LVCFMT_LEFT, 140, ColumnType::Name);
	cm->AddColumn(L"Section", LVCFMT_LEFT, 140, ColumnType::Section);
	cm->AddColumn(L"Offset", LVCFMT_RIGHT, 80, ColumnType::Offset);
	cm->AddColumn(L"Attach Type", LVCFMT_LEFT, 100, ColumnType::Type);
	cm->AddColumn(L"Data Size", LVCFMT_RIGHT, 80, ColumnType::DataSize);

	cm = GetColumnManager(m_StatList);
	cm->AddColumn(L"Name", LVCFMT_LEFT, 180, ColumnType::Name);
	cm->AddColumn(L"Value", LVCFMT_RIGHT, 120, ColumnType::Value);
	cm->AddColumn(L"Details", LVCFMT_LEFT, 120, ColumnType::Details);

	m_List.SetItemCount((int)m_Programs.size());
	return 0;
}

LRESULT CObjectFileView::OnRefresh(WORD, WORD, HWND, BOOL&) {
	Refresh();

	return 0;
}

void CObjectFileView::Refresh() {
	Open();
	m_List.SetItemCount((int)m_Programs.size());
}

bool CObjectFileView::Open(PCWSTR path) {
	if (path == nullptr)
		path = m_Path;
	else
		m_Path = path;
	m_Programs = BpfSystem::EnumProgramsInFile(CStringA(path));
	return !m_Programs.empty();
}

CString CObjectFileView::GetColumnText(HWND hWnd, int row, int column) const {
	if (hWnd != m_List)
		return GetColumnTextStat(row, column);

	auto& p = m_Programs[row];
	switch (static_cast<ColumnType>(GetColumnManager(m_List)->GetColumnTag(column))) {
		case ColumnType::Name: return CString(p.Name.c_str());
		case ColumnType::Section: return CString(p.SectionName.c_str());
		case ColumnType::Offset: return std::to_wstring(p.OffsetInSection).c_str();
		case ColumnType::DataSize: return std::to_wstring(p.Data.size()).c_str();
		case ColumnType::Type: return BpfSystem::GetProgramTypeName(p.Type);
		case ColumnType::AttachType: return BpfSystem::GetAttachTypeName(p.ExpectedAttachType);
	};

	return L"";
}

int CObjectFileView::GetRowImage(HWND hWnd, int row, int column) const {
	return 0;
}

void CObjectFileView::OnStateChanged(HWND hWnd, int from, int to, UINT oldState, UINT newState) {
	if (hWnd != m_List)
		return;

	if (newState & LVIS_SELECTED) {
		UpdateProgramData(to);
	}
	else if (m_List.GetSelectedCount() == 0) {
		UpdateProgramData(-1);
	}
}

void CObjectFileView::DoSort(SortInfo const* si) {
	if (si->hWnd == m_List) {
		auto sort = [&](auto const& p1, auto const& p2) {
			switch (static_cast<ColumnType>(GetColumnManager(m_List)->GetColumnTag(si->SortColumn))) {
				case ColumnType::Name: return SortHelper::Sort(p1.Name, p2.Name, si->SortAscending);
				case ColumnType::DataSize: return SortHelper::Sort(p1.Data.size(), p2.Data.size(), si->SortAscending);
				case ColumnType::Section: return SortHelper::Sort(p1.SectionName, p2.SectionName, si->SortAscending);
				case ColumnType::AttachType: return SortHelper::Sort(BpfSystem::GetAttachTypeName(p1.ExpectedAttachType), BpfSystem::GetAttachTypeName(p1.ExpectedAttachType), si->SortAscending);
				case ColumnType::Type: return SortHelper::Sort(BpfSystem::GetProgramTypeName(p1.Type), BpfSystem::GetProgramTypeName(p2.Type), si->SortAscending);
			}
			return false;
			};
		std::ranges::sort(m_Programs, sort);
	}
	else {
		auto sort = [&](auto const& p1, auto const& p2) {
			switch (static_cast<ColumnType>(GetColumnManager(m_StatList)->GetColumnTag(si->SortColumn))) {
				case ColumnType::Name: return SortHelper::Sort(p1.Key, p2.Key, si->SortAscending);
				case ColumnType::Value: 
				case ColumnType::Details:
					return SortHelper::Sort(p1.Value, p2.Value, si->SortAscending);
			}
			return false;
			};
		auto& stat = m_Extra.find(m_SelectedProgram->Name)->second.Stats;
		std::ranges::sort(stat, sort);
	}
}
