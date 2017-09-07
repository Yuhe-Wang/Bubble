
// VisualBubbles.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "SplashThread.h"

// CVisualBubblesApp:
// See VisualBubbles.cpp for the implementation of this class
//

class CVisualBubblesApp : public CWinAppEx
{
public:
	CVisualBubblesApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation=

	CSplashThread* pSplashThread;
	BOOL createSplash();
	BOOL quitSplash();

	DECLARE_MESSAGE_MAP()
};

extern CVisualBubblesApp theApp;