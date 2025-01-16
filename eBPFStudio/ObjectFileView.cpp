#include "pch.h"
#include "ObjectFileView.h"
#include "resource.h"

void CObjectFileView::UpdateProgramData(int row) {
	auto& p = m_Programs[row];
	m_HexView.InitData(p.Data.data(), p.Data.size());

	if (auto it = m_Extra.find(p.Name); it != m_Extra.end()) {
		m_DisassemblyView.SetWindowTextW(it->second.Assembly.c_str());
	}
	else {
		auto text = BpfSystem::DisassembleProgram(p);
		CString textw(text.c_str());
		textw.Replace(L"\n", L"\r\n");
		ProgramExtra extra;
		extra.Assembly = textw;
		m_Extra.insert({ p.Name, extra });
		m_DisassemblyView.SetWindowTextW(textw);
	}
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
	UINT ids[] = { IDR_MAINFRAME, IDI_BINARY, IDI_CPU };
	for (auto id : ids)
		images.AddIcon(AtlLoadIconImage(id, 0, 16, 16));
	m_List.SetImageList(images, LVSIL_SMALL);
	m_Tabs.SetImageList(images);

	m_HexView.Create(m_Tabs, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN);
	m_HexView.SetReadOnly(true);
	m_DisassemblyView.Create(m_Tabs, rcDefault, L"", WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | ES_READONLY | ES_MULTILINE | ES_WANTRETURN | WS_VSCROLL | WS_HSCROLL);
	m_DisassemblyView.SetFont(Frame()->GetMonoFont());

	m_Tabs.AddPage(m_HexView, L"Bytes", 1, this);
	m_Tabs.AddPage(m_DisassemblyView, L"Assembly", 2, this);

	m_Splitter.SetSplitterPosPct(25);
	m_Splitter.SetSplitterPanes(m_List, m_Tabs);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"Name", LVCFMT_LEFT, 140, ColumnType::Name);
	cm->AddColumn(L"Section", LVCFMT_LEFT, 140, ColumnType::Section);
	cm->AddColumn(L"Offset", LVCFMT_RIGHT, 80, ColumnType::Offset);
	cm->AddColumn(L"Attach Type", LVCFMT_LEFT, 100, ColumnType::Type);
	cm->AddColumn(L"Data Size", LVCFMT_RIGHT, 80, ColumnType::DataSize);

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
	auto& p = m_Programs[row];
	switch (static_cast<ColumnType>(GetColumnManager(m_List)->GetColumnTag(column))) {
		case ColumnType::Name: return CString(p.Name.c_str());
		case ColumnType::Section: return CString(p.SectionName.c_str());
		case ColumnType::Offset: return std::to_wstring(p.OffsetInSection).c_str();
		case ColumnType::DataSize: return std::to_wstring(p.Data.size()).c_str();
	};

	return L"";
}

int CObjectFileView::GetRowImage(HWND hWnd, int row, int column) const {
	return 0;
}

void CObjectFileView::OnStateChanged(HWND hWnd, int from, int to, UINT oldState, UINT newState) {
	if (newState & LVIS_SELECTED) {
		UpdateProgramData(to);
	}
}

void CObjectFileView::DoSort(SortInfo const* si) {
}
