// MainFrm.cpp : implmentation of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "resource.h"
#include "Aboutdlg.h"
#include "MainFrm.h"
#include <ToolbarHelper.h>
#include "ProgramsView.h"
#include "MapsView.h"
#include "LinksView.h"
#include "PinsView.h"
#include "ObjectFileView.h"
#include <eBPF.h>

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg) {
	if (CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
		return TRUE;

	return m_Tabs.PreTranslateMessage(pMsg);
}

BOOL CMainFrame::OnIdle() {
	UIUpdateToolBar();
	return FALSE;
}

HFONT CMainFrame::GetMonoFont() const {
	return m_MonoFont.m_hFont;
}

void CMainFrame::InitMenu() {
	struct {
		UINT id, icon;
		HICON hIcon = nullptr;
	} cmds[] = {
		{ ID_VIEW_REFRESH, IDI_REFRESH },
		{ ID_FILE_OPEN, IDI_OPEN },
		{ ID_PROGRAM_LOAD, IDI_PROGRAM_LOAD },
		{ ID_BPF_PIN, IDI_PIN },
		{ ID_BPF_UNPIN, IDI_UNPIN },
	};

	for (auto& cmd : cmds) {
		if (cmd.icon)
			AddCommand(cmd.id, cmd.icon);
		else
			AddCommand(cmd.id, cmd.hIcon);
	}
}

LRESULT CMainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	ToolBarButtonInfo const buttons[] = {
		{ ID_VIEW_REFRESH, IDI_REFRESH },
		{ 0 },
		{ ID_FILE_OPEN, IDI_OPEN },
		{ 0 },
		{ ID_PROGRAM_LOAD, IDI_PROGRAM_LOAD },
		{ 0 },
		{ ID_BPF_PIN, IDI_PIN },
		{ ID_BPF_UNPIN, IDI_UNPIN },
	};
	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	auto tb = ToolbarHelper::CreateAndInitToolBar(m_hWnd, buttons, std::size(buttons));
	AddSimpleReBarBand(tb);
	UIAddToolBar(tb);

	CreateSimpleStatusBar();

	m_Tabs.m_bTabCloseButton = false;
	m_hWndClient = m_Tabs.Create(m_hWnd, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0);

	CImageList images;
	images.Create(16, 16, ILC_COLOR32 | ILC_MASK, 8, 4);
	UINT ids[] = {
		IDR_MAINFRAME, IDI_MAPS, IDI_LINK, IDI_PIN, IDI_OBJECT,
	};
	for (auto id : ids)
		images.AddIcon(AtlLoadIconImage(id, 0, 16, 16));
	m_Tabs.SetImageList(images);

	auto pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != nullptr);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	CMenuHandle menuMain = GetMenu();
	const UINT WindowMenuPosition = 6;
	m_Tabs.SetWindowMenu(menuMain.GetSubMenu(WindowMenuPosition));

	InitMenu();
	UIAddMenu(menuMain);
	AddMenu(menuMain);
	SetCheckIcon(IDI_CHECK);

	UISetCheck(ID_VIEW_STATUS_BAR, 1);

	m_MonoFont.CreatePointFont(100, L"Consolas");

	PostMessage(WM_COMMAND, ID_FILE_NEW);

	return 0;
}

LRESULT CMainFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	auto pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != nullptr);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);

	bHandled = FALSE;
	return 1;
}

LRESULT CMainFrame::OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	PostMessage(WM_CLOSE);
	return 0;
}

LRESULT CMainFrame::OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	{
		auto view = new CProgramsView(this);
		view->Create(m_Tabs, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0);
		m_Tabs.AddPage(view->m_hWnd, L"Programs", 0, view);
	}
	{
		auto view = new CMapsView(this);
		view->Create(m_Tabs, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0);
		m_Tabs.AddPage(view->m_hWnd, L"Maps", 1, view);
	}
	{
		auto view = new CLinksView(this);
		view->Create(m_Tabs, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0);
		m_Tabs.AddPage(view->m_hWnd, L"Links", 2, view);
	}
	{
		auto view = new CPinsView(this);
		view->Create(m_Tabs, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0);
		m_Tabs.AddPage(view->m_hWnd, L"Pins", 3, view);
	}

	m_Tabs.SetActivePage(0);

	return 0;
}

LRESULT CMainFrame::OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	static BOOL bVisible = TRUE;	// initially visible
	bVisible = !bVisible;
	CReBarCtrl rebar = m_hWndToolBar;
	int nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST + 1);	// toolbar is 2nd added band
	rebar.ShowBand(nBandIndex, bVisible);
	UISetCheck(ID_VIEW_TOOLBAR, bVisible);
	UpdateLayout();
	return 0;
}

LRESULT CMainFrame::OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	BOOL bVisible = !::IsWindowVisible(m_hWndStatusBar);
	::ShowWindow(m_hWndStatusBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
	UISetCheck(ID_VIEW_STATUS_BAR, bVisible);
	UpdateLayout();
	return 0;
}

LRESULT CMainFrame::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	CAboutDlg dlg;
	dlg.DoModal();
	return 0;
}

LRESULT CMainFrame::OnWindowClose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int nActivePage = m_Tabs.GetActivePage();
	if (nActivePage != -1)
		m_Tabs.RemovePage(nActivePage);
	else
		::MessageBeep((UINT)-1);

	return 0;
}

LRESULT CMainFrame::OnWindowCloseAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	m_Tabs.RemoveAllPages();

	return 0;
}

LRESULT CMainFrame::OnWindowActivate(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int nPage = wID - ID_WINDOW_TABFIRST;
	m_Tabs.SetActivePage(nPage);

	return 0;
}

LRESULT CMainFrame::OnFileOpen(WORD, WORD, HWND, BOOL&) {
	CSimpleFileDialog dlg(TRUE, nullptr, nullptr, OFN_EXPLORER | OFN_ENABLESIZING,
		L"Object Files\0*.o\0Binary Files\0*.dll;*.sys\0Source Files\0*.c\0All Files\0*.*\0", m_hWnd);
	ThemeHelper::Suspend();
	auto ok = IDOK == dlg.DoModal();
	ThemeHelper::Resume();
	if (ok) {
		auto view = new CObjectFileView(this);
		if (!view->Open(dlg.m_szFileName)) {
			AtlMessageBox(m_hWnd, L"No programs found in file.", IDR_MAINFRAME, MB_ICONWARNING);
			delete view;
			return 0;
		}
		view->Create(m_Tabs, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN);
		m_Tabs.AddPage(view->m_hWnd, dlg.m_szFileTitle, 3, view);
	}
	return 0;
}

LRESULT CMainFrame::OnProgramLoad(WORD, WORD, HWND, BOOL&) {
	CSimpleFileDialog dlg(TRUE, nullptr, nullptr, OFN_EXPLORER | OFN_ENABLESIZING,
		L"Object Files\0*.o\0Binary Files\0*.sys\0\0All Files\0*.*\0", m_hWnd);
	ThemeHelper::Suspend();
	auto ok = IDOK == dlg.DoModal();
	ThemeHelper::Resume();
	if (ok) {
		auto count = BpfSystem::LoadProgramsFromFile(CStringA(dlg.m_szFileName));
		if (count == 0) {
			auto errText = BpfSystem::GetLastErrorText();
			AtlMessageBox(m_hWnd,
				PCWSTR(CString(L"Failed to load program(s)") + (errText.empty() ? CString(L"") : (L" - " + CString(errText.c_str())))),
				IDR_MAINFRAME, MB_ICONERROR);
		}
		else {
			AtlMessageBox(m_hWnd, std::format(L"{} program(s) loaded", count).c_str(),
				IDR_MAINFRAME, MB_ICONINFORMATION);
			LRESULT result;
			for (int i = 0; i < 3; i++) {
				ProcessWindowMessage(m_Tabs.GetPageHWND(i), WM_COMMAND, ID_VIEW_REFRESH, 0, result, 1);
			}
		}
	}
	return 0;
}

LRESULT CMainFrame::OnStartServices(WORD, WORD, HWND, BOOL&) {
	if (!BpfSystem::StartServices()) {
		AtlMessageBox(m_hWnd, L"Failed to start services", IDR_MAINFRAME, MB_ICONERROR);
	}
	return 0;
}

LRESULT CMainFrame::OnRestartServices(WORD, WORD, HWND, BOOL&) {
	if (!BpfSystem::RestartServices()) {
		AtlMessageBox(m_hWnd, L"Failed to restart services", IDR_MAINFRAME, MB_ICONERROR);
	}
	return 0;
}

LRESULT CMainFrame::OnStopServices(WORD, WORD, HWND, BOOL&) {
	if (!BpfSystem::StopServices()) {
		AtlMessageBox(m_hWnd, L"Failed to stop services", IDR_MAINFRAME, MB_ICONERROR);
	}
	return 0;
}
