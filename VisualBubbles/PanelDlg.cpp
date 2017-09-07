// PanelDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "VisualBubbles.h"
#include "VisualBubblesDlg.h"
#include "PanelDlg.h"

extern CVisualBubblesDlg *g_p;
// CPanelDlg 对话框

IMPLEMENT_DYNAMIC(CPanelDlg, CDialog)

CPanelDlg::CPanelDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPanelDlg::IDD, pParent)
	, m_dR(0.02)
	, m_time_min(1)
	, m_time_max(0)
	, m_max_r(10)
	, m_same_coordinates(TRUE)
	, m_auto_r(TRUE)
	, m_timer_speed(0)
	, m_f_axis(0)
	, m_f_prob(FALSE)
	, m_show_fitting(FALSE)
{

}

CPanelDlg::~CPanelDlg()
{
}

void CPanelDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_DR, m_dR);
	DDV_MinMaxFloat(pDX, m_dR, 0.01, 10);
	DDX_Text(pDX, IDC_EDIT_TIME_MIN, m_time_min);
	DDX_Text(pDX, IDC_EDIT_TIME_MAX, m_time_max);
	DDX_Text(pDX, IDC_EDIT_MAX_R, m_max_r);
	DDX_Check(pDX, IDC_CHECK_SAME_COORDINATES, m_same_coordinates);
	DDX_Check(pDX, IDC_CHECK_AUTO_R, m_auto_r);
	DDX_Slider(pDX, IDC_SLIDER_SPEED, m_timer_speed);
}


BEGIN_MESSAGE_MAP(CPanelDlg, CDialog)
	ON_BN_CLICKED(IDC_NEXT, &CPanelDlg::OnBnClickedNext)
	ON_BN_CLICKED(IDC_ANIMATE, &CPanelDlg::OnBnClickedAnimate)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_PREVIOUS, &CPanelDlg::OnBnClickedPrevious)
	ON_BN_CLICKED(IDC_SWITCH_VIEW, &CPanelDlg::OnBnClickedSwitchView)
	ON_BN_CLICKED(IDC_CHECK_SAME_COORDINATES, &CPanelDlg::OnBnClickedCheckSameCoordinates)
	ON_BN_CLICKED(IDC_BUTTON_SET_DR, &CPanelDlg::OnBnClickedButtonSetDr)
	ON_BN_CLICKED(IDC_BUTTON_SET_TIME_RANGE, &CPanelDlg::OnBnClickedButtonSetTimeRange)
	ON_BN_CLICKED(IDC_BUTTON_SET_MAX_R, &CPanelDlg::OnBnClickedButtonSetMaxR)
	ON_BN_CLICKED(IDC_CHECK_AUTO_R, &CPanelDlg::OnBnClickedCheckAutoR)
	ON_BN_CLICKED(IDC_RADIO_LINEAR, &CPanelDlg::OnBnClickedRadioLinear)
	ON_BN_CLICKED(IDC_RADIO_LOG, &CPanelDlg::OnBnClickedRadioLog)
	ON_BN_CLICKED(IDC_CHECK_F_PROBABILITY, &CPanelDlg::OnBnClickedCheckFProbability)
	ON_BN_CLICKED(IDC_CHECK_SHOW_FITTING, &CPanelDlg::OnBnClickedCheckShowFitting)
END_MESSAGE_MAP()


// CPanelDlg 消息处理程序

void CPanelDlg::OnBnClickedNext()
{
	g_p->renderNext();
}

void CPanelDlg::OnBnClickedAnimate()
{
	UpdateData(TRUE);
	if(g_p->renderAnimation()) 
	{
		GetDlgItem(IDC_ANIMATE)->SetWindowText("Stop");
		GetDlgItem(IDC_NEXT)->EnableWindow(FALSE);
		GetDlgItem(IDC_PREVIOUS)->EnableWindow(FALSE);
	}
	else 
	{
		GetDlgItem(IDC_ANIMATE)->SetWindowText("Animate");
		GetDlgItem(IDC_NEXT)->EnableWindow(TRUE);
		GetDlgItem(IDC_PREVIOUS)->EnableWindow(TRUE);
	}
}

BOOL CPanelDlg::PreTranslateMessage(MSG* pMsg)
{
	/*if(pMsg->message==WM_KEYDOWN)
	{
		if(g_p->isHotKey(pMsg->wParam)&&!isEdit()) //send the main dialog 
		{
			g_p->PostMessage(pMsg->message,pMsg->wParam,pMsg->lParam);
			return TRUE;
		}
		if(pMsg->wParam == VK_RETURN||pMsg->wParam == VK_ESCAPE ) return TRUE; //hide the return and escape keys		                
	}	*/
	
	return FALSE;//return CDialog::PreTranslateMessage(pMsg);
}


void CPanelDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	//CSliderCtrl *pSlidCtrl=(CSliderCtrl*)GetDlgItem(IDC_SLIDER_SPEED);

	//g_p->setAnimationSpeed(pSlidCtrl->GetPos());
	UpdateData(TRUE);
	g_p->setAnimationSpeed();

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

BOOL CPanelDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	((CSliderCtrl*)GetDlgItem(IDC_SLIDER_SPEED))->SetRange(0,10);
	((CButton* )GetDlgItem(IDC_RADIO_LINEAR))->SetCheck(1);

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CPanelDlg::OnBnClickedPrevious()
{
	g_p->renderPrevious();
}

void CPanelDlg::OnBnClickedSwitchView()
{
	g_p->switchView();
}

void CPanelDlg::OnBnClickedCheckSameCoordinates()
{
	//g_p->unifyCoordinates();
	UpdateData(TRUE);
	g_p->renderScene();
}

void CPanelDlg::OnBnClickedButtonSetDr()
{
	UpdateData(TRUE);
	g_p->doStatistic();
	g_p->renderScene();
}

void CPanelDlg::OnBnClickedButtonSetTimeRange()
{
	UpdateData(TRUE);
	g_p->setTimeRange(m_time_min,m_time_max);
}


void CPanelDlg::OnBnClickedButtonSetMaxR()
{
	UpdateData(TRUE);
	g_p->renderScene();
}

void CPanelDlg::OnBnClickedCheckAutoR()
{
	UpdateData(TRUE);
	g_p->renderScene();
}

void CPanelDlg::OnBnClickedRadioLinear()
{
	m_f_axis=0;
	g_p->renderScene();
}

void CPanelDlg::OnBnClickedRadioLog()
{
	m_f_axis=1;
	g_p->renderScene();
}

void CPanelDlg::OnBnClickedCheckFProbability()
{
	m_f_prob=!m_f_prob;
	g_p->renderScene();
}

void CPanelDlg::OnBnClickedCheckShowFitting()
{
	m_show_fitting=!m_show_fitting;
	g_p->renderScene();
}
