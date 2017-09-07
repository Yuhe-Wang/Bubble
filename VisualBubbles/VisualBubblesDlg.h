
// VisualBubblesDlg.h : header file
//

#pragma once

#include "vector"
#include "PanelDlg.h"
#include "afxwin.h"
using std::vector;

struct CBubble
{
	float x,y,z,R;
};

struct CSnapshot
{
	size_t step,N_bubble;
	vector<CBubble> bubbles;
	vector<size_t> statistic; //its length and value depends on m_dr;
	float max_r,ave_r,delta_r;
	size_t max_f,min_f;
	vector<float> fit; //the fitting curve data
};

struct CContainer
{
	float width,height;
	float max_R;
	size_t max_F,min_F;
	int N_snapshot;
	vector<CSnapshot> status;
	float max_F_prob,min_F_prob;
};

struct CGLWorld
{
	float x,y,z;
};

// CVisualBubblesDlg dialog
class CVisualBubblesDlg : public CDialog
{
// Construction
public:
	CVisualBubblesDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_VISUALBUBBLES_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();

	

	

	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	//data
	CDC *m_pDC;
	double m_theta,m_phi,m_R; //determine the observation point
	CContainer m_c;
	int m_is; //index of snapshot
	CPanelDlg m_panel; //the control panel
	float m_ws; //shorter width in the world coordinates
	BOOL m_timer; //whether show the animation
	int m_gllist_id;
	int m_glview_id;
	CGLWorld m_glworld; 
	UINT m_time_min,m_time_max;
	BOOL m_data_type;
	
	//function
	void InitGL(); //initialize the openGL render engine

	void setGLPorjection( CRect& rect );

	BOOL SetupPixelFormat(); //choose pixel format for openGL
	
	void updateInfo();
	void GLTextOut(float x,float y,float z,const char * textstring);
	void GLArrow(float x1,float y1,float z1,float x2,float y2,float z2);
	void GLSolidBox(float dx, float dy, float dz);
	void draw3DView();
	void drawStatisticView();
	void draw3DStatisticView();
	float fit_func(double x, size_t);
	BOOL loadFile(const char* filename);
	void outputStatistic();



public:
	afx_msg void OnClose();
protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	void renderNext();
	void renderPrevious();
	BOOL renderAnimation();
	void setAnimationSpeed();
	void switchView();
	void setTimeRange(int min,int max);
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	void doStatistic();
	void renderScene();

	CStatic m_glView;
	afx_msg void OnDropFiles(HDROP hDropInfo);
};
