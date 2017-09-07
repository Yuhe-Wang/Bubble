
// VisualBubbles.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "VisualBubbles.h"
#include "VisualBubblesDlg.h"
#include "SplashThread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CVisualBubblesApp

BEGIN_MESSAGE_MAP(CVisualBubblesApp, CWinAppEx)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CVisualBubblesApp construction

CVisualBubblesApp::CVisualBubblesApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	pSplashThread=NULL;
}


// The one and only CVisualBubblesApp object

CVisualBubblesApp theApp;


// CVisualBubblesApp initialization

BOOL CVisualBubblesApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	//create the splash screen
	createSplash();
	////////////////////////////

	CVisualBubblesDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

BOOL CVisualBubblesApp::createSplash()
{
	pSplashThread = (CSplashThread*)AfxBeginThread(RUNTIME_CLASS(CSplashThread),THREAD_PRIORITY_NORMAL,0);
	if(pSplashThread) return TRUE;
	else return FALSE;

}

BOOL CVisualBubblesApp::quitSplash()
{
	if(pSplashThread)
	{
		pSplashThread->m_pSplashDlg->PostMessage(WM_CLOSE);//close the splash screen
		pSplashThread=NULL;
		return TRUE;
	}
	else return FALSE;
	
}

