// MainFrm.cpp : implmentation of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "resource.h"

#include "aboutdlg.h"
#include "MainFrm.h"
#include <ToolbarHelper.h>
#include "ProgramsView.h"
#include "MapsView.h"
#include "LinksView.h"
#include "ObjectFileView.h"

#define WINDOW_MENU_POSITION	4

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
		{ ID_FILE_OPEN, IDI_OPEN },
		{ ID_VIEW_REFRESH, IDI_REFRESH },
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
		{ ID_FILE_OPEN, IDI_OPEN },
		{ 0 },
		{ ID_VIEW_REFRESH, IDI_REFRESH },
	};
	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	auto tb = ToolbarHelper::CreateAndInitToolBar(m_hWnd, buttons, _countof(buttons));
	AddSimpleReBarBand(tb);
	UIAddToolBar(tb);

	CreateSimpleStatusBar();

	m_Tabs.m_bTabCloseButton = false;
	m_hWndClient = m_Tabs.Create(m_hWnd, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0);

	CImageList images;
	images.Create(16, 16, ILC_COLOR32 | ILC_MASK, 4, 4);
	UINT ids[] = { 
		IDR_MAINFRAME, IDI_MAPS, IDI_LINK, IDI_OBJECT
	};
	for (auto id : ids)
		images.AddIcon(AtlLoadIconImage(id, 0, 16, 16));
	m_Tabs.SetImageList(images);

	UISetCheck(ID_VIEW_TOOLBAR, 1);
	UISetCheck(ID_VIEW_STATUS_BAR, 1);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != nullptr);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	CMenuHandle menuMain = GetMenu();
	m_Tabs.SetWindowMenu(menuMain.GetSubMenu(WINDOW_MENU_POSITION));

	InitMenu();
	UIAddMenu(menuMain);
	AddMenu(menuMain);

	m_MonoFont.CreatePointFont(100, L"Consolas");

	PostMessage(WM_COMMAND, ID_FILE_NEW);

	return 0;
}

LRESULT CMainFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	// unregister message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
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
		L"Object Files\0*.o\0Binary Files\0*dll;*.sys\0Source Files\0*.c\0All Files\0*.*\0", m_hWnd);
	ThemeHelper::Suspend();
	auto ok = IDOK == dlg.DoModal();
	ThemeHelper::Resume();
	if(ok) {
		auto view = new CObjectFileView(this);
		if(!view->Open(dlg.m_szFileName)) {
			AtlMessageBox(m_hWnd, L"No programs found in file.", IDR_MAINFRAME, MB_ICONWARNING);
			delete view;
			return 0;
		}
		view->Create(m_Tabs, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN);
		m_Tabs.AddPage(view->m_hWnd, dlg.m_szFileTitle, 3, view);
	}
	return 0;
}
