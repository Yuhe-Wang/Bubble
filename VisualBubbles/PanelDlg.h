#pragma once
#include "afxcmn.h"

//class CVisualBubblesDlg;

// CPanelDlg 对话框

class CPanelDlg : public CDialog
{
	friend class CVisualBubblesDlg;

	DECLARE_DYNAMIC(CPanelDlg)

public:
	CPanelDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CPanelDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG_PANEL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedNext();
	afx_msg void OnBnClickedAnimate();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//CSliderCtrl m_speed;
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedPrevious();
	afx_msg void OnBnClickedSwitchView();
	afx_msg void OnBnClickedCheckSameCoordinates();
	float m_dR;
	afx_msg void OnBnClickedButtonSetDr();
	int m_time_min;
	int m_time_max;
	afx_msg void OnBnClickedButtonSetTimeRange();
	float m_max_r;
	afx_msg void OnBnClickedButtonSetMaxR();
	BOOL m_same_coordinates;
	BOOL m_auto_r;
	int m_f_axis; //ways to show the f axis
	BOOL m_f_prob;
	BOOL m_show_fitting;
	afx_msg void OnBnClickedCheckAutoR();
	int m_timer_speed;
	afx_msg void OnBnClickedRadioLinear();
	afx_msg void OnBnClickedRadioLog();
	afx_msg void OnBnClickedCheckFProbability();
	afx_msg void OnBnClickedCheckShowFitting();
};
