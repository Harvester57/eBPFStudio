#pragma once

const UINT WM_UPDATEUI = WM_USER + 55;

class CMainFrame;

struct IMainFrame abstract {
	virtual HFONT GetMonoFont() const = 0;
	virtual CAutoUpdateUI<CMainFrame>& UI() = 0;
};
