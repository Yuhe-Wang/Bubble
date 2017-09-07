
// VisualBubblesDlg.cpp : implementation file
//

#include "stdafx.h"

#include "gl/GL.h"
#include "gl/GLU.h"
#include "VisualBubbles.h"
#include "VisualBubblesDlg.h"
#include "SplashScreen.h"
#include "glMath.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define PI 3.1415926
#define K_SPHERE (4.0/3.0*PI)
#define m_dr m_panel.m_dR
GLUquadricObj *quadObj=gluNewQuadric();
CVisualBubblesDlg *g_p; //global pointer making the dialog accessed easily 


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CVisualBubblesDlg dialog




CVisualBubblesDlg::CVisualBubblesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CVisualBubblesDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_theta=75;
	m_phi=60;
	m_R=80;
	m_is=0;
	m_timer=FALSE;
	m_glview_id=0; //default view: 3D view
	m_time_min=1;
	m_data_type=0;
}

void CVisualBubblesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, ID_GL_DRAW, m_glView);
}

BEGIN_MESSAGE_MAP(CVisualBubblesDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_DROPFILES()
END_MESSAGE_MAP()


// CVisualBubblesDlg message handlers

BOOL CVisualBubblesDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	//ShowWindow(SW_MINIMIZE);

	// TODO: Add extra initialization here
	g_p=this;

	//decide the layout
	m_panel.Create(IDD_DIALOG_PANEL,this);
	CRect rect_dlg,rect_panel,rect_gl;
	GetClientRect(&rect_dlg);
	m_panel.GetClientRect(&rect_panel);
	int gl_width=rect_dlg.Width()-rect_panel.Width();
	rect_panel.left+=gl_width;
	rect_panel.right+=gl_width;
	m_panel.MoveWindow(&rect_panel);
	m_panel.ShowWindow(SW_SHOW);
	rect_gl.left=0;
	rect_gl.right=gl_width;
	rect_gl.top=0;
	rect_gl.bottom=rect_dlg.Height();
	m_glView.MoveWindow(&rect_gl);
	
	//read the status.txt file
	if(!loadFile("status.txt")) exit(0); //must load a default file

	InitGL();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CVisualBubblesDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CVisualBubblesDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
	renderScene();
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CVisualBubblesDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CVisualBubblesDlg::InitGL() //Init the openGL render engine
{
	PIXELFORMATDESCRIPTOR pfd;
	int         n;
	HGLRC       hrc;
	CWnd* pWnd=GetDlgItem(ID_GL_DRAW);//get the drawing area
    m_pDC=pWnd->GetDC();
	 
	ASSERT(m_pDC != NULL);

	if (!SetupPixelFormat()) return;

	n = ::GetPixelFormat(m_pDC->GetSafeHdc());
	::DescribePixelFormat(m_pDC->GetSafeHdc(), n, sizeof(pfd), &pfd);

	hrc = wglCreateContext(m_pDC->GetSafeHdc());
	wglMakeCurrent(m_pDC->GetSafeHdc(), hrc);

	//////////////create openGL font//////////////////
	char fontname[]="Arial"; 
	HFONT hfont;
	LOGFONT logfont;
	logfont.lfHeight		= -20;
	logfont.lfWidth			= 0;
	logfont.lfEscapement	= 0;
	logfont.lfOrientation	= logfont.lfEscapement;
	logfont.lfWeight		= FW_NORMAL;
	logfont.lfItalic		= FALSE;
	logfont.lfUnderline		= FALSE;
	logfont.lfStrikeOut		= FALSE;
	logfont.lfCharSet		= ANSI_CHARSET;
	logfont.lfOutPrecision	= OUT_DEFAULT_PRECIS;
	logfont.lfClipPrecision	= CLIP_DEFAULT_PRECIS;
	logfont.lfQuality		= DEFAULT_QUALITY;
	logfont.lfPitchAndFamily = FF_DONTCARE|DEFAULT_PITCH;
	lstrcpy ( logfont.lfFaceName, fontname );
	hfont=CreateFontIndirect( &logfont );
	SelectObject(m_pDC->m_hDC,hfont);
	m_gllist_id=glGenLists(256);
	wglUseFontBitmaps( m_pDC->m_hDC, 0, 256, m_gllist_id );
	DeleteObject(hfont);//delete old font
	////////////////////////////////////////////////////////


	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);  //enable the depth test

	//set the projection parameters
	CRect rect;
	pWnd->GetClientRect(&rect);
	setGLPorjection(rect);

	/*
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	*/
	
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	//turn on the light
	
	GLfloat ambientLight[]={0.3f,0.3f,0.3f,1.0f};
	GLfloat diffuseLight[]={0.7f,0.7f,0.7f,1.0f};
	GLfloat lightPos[]={0.7f*m_c.width,0.7f*m_c.width,0.5f*m_c.height,1.0f};
	GLfloat specular[]={0.3f,0.3f,0.3f,1.0f};
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0,GL_AMBIENT,ambientLight);
	glLightfv(GL_LIGHT0,GL_DIFFUSE,diffuseLight);
	glLightfv(GL_LIGHT0,GL_SPECULAR,specular);
	glLightfv(GL_LIGHT0,GL_POSITION,lightPos);
	
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT,GL_AMBIENT_AND_DIFFUSE);
	glMaterialfv(GL_FRONT,GL_SPECULAR,specular);
	glMateriali(GL_FRONT,GL_SHININESS,100);
	
	//the background color
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	//glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	
	gluQuadricDrawStyle(quadObj, GLU_FILL);
	gluQuadricNormals(quadObj, GLU_SMOOTH);
	
}

BOOL CVisualBubblesDlg::SetupPixelFormat()
{
	static PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),  // size of this pfd
		1,                              // version number
		PFD_DRAW_TO_WINDOW |            // support window
		PFD_SUPPORT_OPENGL |          // support OpenGL
		PFD_DOUBLEBUFFER,             // double buffered
		PFD_TYPE_RGBA,                  // RGBA type
		24,                             // 24-bit color depth
		0, 0, 0, 0, 0, 0,               // color bits ignored
		0,                              // no alpha buffer
		0,                              // shift bit ignored
		0,                              // no accumulation buffer
		0, 0, 0, 0,                     // accum bits ignored
		32,                             // 32-bit z-buffer
		0,                              // no stencil buffer
		0,                              // no auxiliary buffer
		PFD_MAIN_PLANE,                 // main layer
		0,                              // reserved
		0, 0, 0                         // layer masks ignored
	};
	int pixelformat;
	if ( (pixelformat = ChoosePixelFormat(m_pDC->GetSafeHdc(), &pfd)) == 0 )
	{
		MessageBox("ChoosePixelFormat failed");
		return FALSE;
	}
	if (SetPixelFormat(m_pDC->GetSafeHdc(), pixelformat, &pfd) == FALSE)
	{
		MessageBox("SetPixelFormat failed");
		return FALSE;
	}
	return TRUE;
}

void CVisualBubblesDlg::GLTextOut(float x,float y,float z,const char *textstring) //output the text in the openGL window
{
	glRasterPos3f(x,y,z);
	glListBase(m_gllist_id);
	glCallLists(strlen( textstring ), GL_UNSIGNED_BYTE, (const GLvoid*)textstring ); 
}

void CVisualBubblesDlg::GLArrow(float x1,float y1,float z1,float x2,float y2,float z2)
{
	GLMath::Vector v1,v2,v;
	v1.x=0.0f;
	v1.y=0.0f;
	v1.z=1.0f;
	v2.x=x2-x1;
	v2.y=y2-y1;
	v2.z=z2-z1;
	GLMath::crossProduct(v1,v2,v);
	float r=GLMath::sqrt(v2);
	GLfloat theta=asin(GLMath::sqrt(v)/r)*180.0f/PI;
	if(theta<0) theta+=180.0f;
	glPushMatrix();
	glRotatef(theta,v.x,v.y,v.z);
	
	glBegin(GL_LINES);
	glVertex3f(0.0f,0.0f,0.0f);
	glVertex3f(0.0f,0.0f,r);
	glEnd();
	glPushMatrix();
	glTranslatef(0.0f,0.0f,r);
	gluCylinder(quadObj,1.0f,0.0f,3.0f,5,1); //5 edges
	glPopMatrix();
	glPopMatrix();

}

void CVisualBubblesDlg::GLSolidBox(float x, float y, float z)
{
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0,-1,0);
	glVertex3f(0.0f,0.0f,0.0f);
	glVertex3f(x,0.0f,0.0f);
	glVertex3f(x,0.0f,z);
	glVertex3f(0.0f,0.0f,z);	
	glEnd();

	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(1,0,0);
	glVertex3f(x,0.0f,0.0f);
	glVertex3f(x,y,0.0f);
	glVertex3f(x,y,z);
	glVertex3f(x,0.0f,z);	
	glEnd();

	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0,1,0);
	glVertex3f(x,y,0.0f);
	glVertex3f(0.0f,y,0.0f);
	glVertex3f(0.0f,y,z);
	glVertex3f(x,y,z);	
	glEnd();

	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(-1,0,0);
	glVertex3f(0.0f,y,0.0f);
	glVertex3f(0.0f,0.0f,0.0f);
	glVertex3f(0.0f,0.0f,z);
	glVertex3f(0.0f,y,z);	
	glEnd();

	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0,0,-1);
	glVertex3f(0.0f,0.0f,0.0f);
	glVertex3f(0.0f,y,0.0f);
	glVertex3f(x,y,0.0f);
	glVertex3f(x,0.0f,0.0f);	
	glEnd();


	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0,0,1);
	glVertex3f(0.0f,0.0f,z);
	glVertex3f(x,0.0f,z);
	glVertex3f(x,y,z);
	glVertex3f(0.0f,y,z);	
	glEnd();
}
void CVisualBubblesDlg::OnClose()
{
	gluDeleteQuadric(quadObj);
	HGLRC   hrc;	
	hrc = ::wglGetCurrentContext();	
	::wglMakeCurrent(NULL,  NULL);	
	if (hrc) ::wglDeleteContext(hrc);

	CDialog::OnClose();
}

BOOL CVisualBubblesDlg::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN;	
	return CDialog::PreCreateWindow(cs);
}

BOOL CVisualBubblesDlg::PreTranslateMessage(MSG* pMsg)
{
	switch(pMsg->wParam) //change the view point
	{			
	case VK_DOWN:		  
		m_theta-=5;
		if(m_theta<0) m_theta=0;			 
		UpdateData(FALSE);
		renderScene();
		break;
	case VK_UP:
		m_theta+=5;
		if(m_theta>180) m_theta=180;
		UpdateData(FALSE);
		renderScene();
		break;
	case VK_LEFT:
		m_phi-=5;
		if(m_phi<0) m_phi+=360;
		UpdateData(FALSE);
		renderScene();
		break;
	case VK_RIGHT:
		m_phi+=5;
		if(m_phi>360) m_phi-=360;
		UpdateData(FALSE);
		renderScene();
		break;
	case VK_PRIOR:
		m_R+=2;
		UpdateData(FALSE);
		renderScene();
		break;
	case VK_NEXT:
		m_R-=2;
		UpdateData(FALSE);
		renderScene();
		break;
	case VK_ESCAPE:
		return TRUE;
	case VK_RETURN:
		return TRUE;
	default:
		break;
	}	

	return CDialog::PreTranslateMessage(pMsg);
}

void CVisualBubblesDlg::renderScene() //draw the scene
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	if(m_glview_id==0) draw3DView();
	else if(m_glview_id==1) drawStatisticView();
	else if(m_glview_id==2) draw3DStatisticView();

	glFinish();
	SwapBuffers(wglGetCurrentDC());
}

void CVisualBubblesDlg::draw3DView()
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_LIGHTING);
	//print the title
	glColor3f(1.0f,1.0f,1.0f);
	GLTextOut(-m_glworld.x*0.95f,m_glworld.y*0.9f,0.0f,"3D view of the bubbles");	

	double ks1=sin(m_theta*PI/180.0),ks2=sin(m_phi*PI/180.0),kc1=cos(m_theta*PI/180.0),kc2=cos(m_phi*PI/180.0);
	gluLookAt(m_R*ks1*kc2,m_R*ks1*ks2,m_R*kc1,0.0f,0.0f,0.0f,-kc1*kc2,-kc1*ks2,ks1); //determine the observation point

	glTranslatef(-m_c.width/2,-m_c.width/2,-m_c.height/2);
	//draw the container
	glColor3f(1.0,0.0,0.0); 
	glLineWidth(1.0f);
	glBegin(GL_LINE_LOOP);
	glVertex3f(0.0f,0.0f,0.0f);
	glVertex3f(m_c.width,0.0f,0.0f);
	glVertex3f(m_c.width,m_c.width,0.0f);
	glVertex3f(0.0f,m_c.width,0.0f);
	glEnd();

	glBegin(GL_LINE_LOOP);
	glVertex3f(0.0f,0.0f,m_c.height);
	glVertex3f(m_c.width,0.0f,m_c.height);
	glVertex3f(m_c.width,m_c.width,m_c.height);
	glVertex3f(0.0f,m_c.width,m_c.height);
	glEnd();

	glBegin(GL_LINES);
	glVertex3f(0.0f,0.0f,0.0f);
	glVertex3f(0.0f,0.0f,m_c.height);
	glEnd();

	glBegin(GL_LINES);
	glVertex3f(m_c.width,0.0f,0.0f);
	glVertex3f(m_c.width,0.0f,m_c.height);
	glEnd();

	glBegin(GL_LINES);
	glVertex3f(m_c.width,m_c.width,0.0f);
	glVertex3f(m_c.width,m_c.width,m_c.height);
	glEnd();

	glBegin(GL_LINES);
	glVertex3f(0.0f,m_c.width,0.0f);
	glVertex3f(0.0f,m_c.width,m_c.height);
	glEnd();

	//draw the axis
	glColor3f(0.0f,1.0f,1.0f);
	glLineWidth(4.0f);
	GLArrow(0.0f,0.0f,0.0f,1.1f*m_c.width,0.0f,0.0f);
	GLTextOut(1.2f*m_c.width,0.0f,0.1f*m_c.width,"x");
	GLArrow(0.0f,0.0f,0.0f,0.0f,1.1f*m_c.width,0.0f);
	GLTextOut(0.0f,1.2f*m_c.width,0.1f*m_c.width,"y");
	GLArrow(0.0f,0.0f,0.0f,0.0f,0.0f,1.1f*m_c.width);
	GLTextOut(0.0f,0.1f*m_c.width,1.2f*m_c.width,"z");

	glEnable(GL_LIGHTING);
	//draw the m_is th scene
	glColor3f(1.0,1.0,0.0);
	vector<CBubble> & bs=m_c.status[m_is].bubbles;
	for(size_t i=0; i< m_c.status[m_is].N_bubble; ++i) 
	{
		glPushMatrix();
		glTranslatef(bs[i].x,bs[i].y,bs[i].z);
		if(m_data_type==0)gluSphere(quadObj,bs[i].R,5,5);
		else gluSphere(quadObj,pow(bs[i].R/K_SPHERE,1.0/3.0),5,5); //the data is in fact volume
		glPopMatrix();
	}
}
void CVisualBubblesDlg::drawStatisticView()
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_LIGHTING);
	//print the title
	glColor3f(1.0f,1.0f,1.0f);
	GLTextOut(-m_glworld.x*0.95f,m_glworld.y*0.9f,0.0f,"Statistic view of the bubbles");
	vector<size_t>& stat=m_c.status[m_is].statistic;
	float k_f,k_r,min_f;
	const float k_view=0.8f;
	if(m_panel.m_same_coordinates)
	{
		
		if(m_panel.m_f_axis==0) //linear f
		{
			if(m_panel.m_f_prob) k_f=2*k_view*m_glworld.y/m_c.max_F_prob;
			else k_f=2*k_view*m_glworld.y/m_c.max_F;
		}
		else if(m_panel.m_f_axis==1) // draw log(f)
		{
			if(m_panel.m_f_prob)
			{
				min_f= m_c.min_F_prob==0 ? 1.0/m_c.status[0].N_bubble : m_c.min_F_prob;
				k_f=2*k_view*m_glworld.y/log(m_c.max_F_prob/min_f);	
			}
			else 
			{
				min_f=m_c.min_F==0?1:m_c.min_F;
				k_f=2*k_view*m_glworld.y/log(m_c.max_F/min_f);	
			}
		}

		if(m_panel.m_auto_r) k_r=2*k_view*m_glworld.x/m_c.max_R;
		else
		{
			k_r=2*k_view*m_glworld.x/m_panel.m_max_r;
		}
	}
	else //use different coordinates at each time
	{
		
		if(m_panel.m_f_axis==0) //linear f
		{
			if(m_panel.m_f_prob) k_f=2*k_view*m_glworld.y/m_c.status[m_is].max_f*m_c.status[m_is].N_bubble;
			else k_f=2*k_view*m_glworld.y/m_c.status[m_is].max_f;
		}
		else if (m_panel.m_f_axis==1) //log f
		{
			min_f=m_c.status[m_is].min_f==0?1:m_c.status[m_is].min_f;
			k_f=2*k_view*m_glworld.y/log(m_c.status[m_is].max_f/min_f);
			if(m_panel.m_f_prob) 
			{
				min_f/=m_c.status[m_is].N_bubble;
			}
		}	
		if(m_panel.m_auto_r) k_r=2*k_view*m_glworld.x/m_c.status[m_is].max_r;
		else 
		{
			 k_r=2*k_view*m_glworld.x/m_panel.m_max_r;
		}	
	}
	glTranslatef(-k_view*m_glworld.x,-k_view*m_glworld.y,0.0f);
	glColor3f(0.0f,1.0f,1.0f);
	GLArrow(0.0f,0.0f,0.0f,2*k_view*m_glworld.x,0.0f,0.0f);
	if(m_data_type==0) GLTextOut(2*k_view*m_glworld.x,-0.1f*k_view*m_glworld.y,0.0f,"R");
	else GLTextOut(2*k_view*m_glworld.x,-0.1f*k_view*m_glworld.y,0.0f,"V");
	GLArrow(0.0f,0.0f,0.0f,0.0f,2*k_view*m_glworld.y,0.0f);
	if(m_panel.m_f_axis==0) GLTextOut(0.1f*k_view*m_glworld.x,2*k_view*m_glworld.y,0.0f,"f");
	else if(m_panel.m_f_axis==1) GLTextOut(0.1f*k_view*m_glworld.x,2*k_view*m_glworld.y,0.0f,"log(f)");
	GLTextOut(-0.1f*k_view*m_glworld.x,-0.1f*k_view*m_glworld.y,0.0f,"O");
	glColor3f(1.0f,0.0f,0.0f);
	for(size_t i=0; i< stat.size(); ++i)
	{
		glBegin(GL_TRIANGLE_STRIP);
		glVertex2f(k_r*i*m_dr,0.0f);
		glVertex2f(k_r*(i+1)*m_dr,0.0f);
		if(m_panel.m_f_axis==0) //linear
		{
			if(m_panel.m_f_prob)
			{
				glVertex2f(k_r*i*m_dr,k_f*(float)stat[i]/m_c.status[m_is].N_bubble);
				glVertex2f(k_r*(i+1)*m_dr,k_f*(float)stat[i]/m_c.status[m_is].N_bubble);
			}
			else
			{
				glVertex2f(k_r*i*m_dr,k_f*stat[i]);
				glVertex2f(k_r*(i+1)*m_dr,k_f*stat[i]);
			}			
		}
		else if(m_panel.m_f_axis==1) //log
		{
			if(m_panel.m_f_prob)
			{
				glVertex2f(k_r*i*m_dr,k_f*log((double)(stat[i]?stat[i]:1)/m_c.status[m_is].N_bubble/min_f));
				glVertex2f(k_r*(i+1)*m_dr,k_f*log((double)(stat[i]?stat[i]:1)/m_c.status[m_is].N_bubble/min_f));
			}
			else
			{
				glVertex2f(k_r*i*m_dr,k_f*log(stat[i]?stat[i]:1/min_f));
				glVertex2f(k_r*(i+1)*m_dr,k_f*log(stat[i]?stat[i]:1/min_f));	
			}
		}	
		glEnd();
		
	}
	if(m_panel.m_show_fitting)
	{
		vector<float>& fit=m_c.status[m_is].fit;
		glColor3f(0.0f,0.0f,1.0f);
		glBegin(GL_LINE_STRIP);
		for(size_t i=0; i< stat.size(); ++i)
		{
			glVertex2f(k_r*(i+0.5)*m_dr,k_f*fit[i]);	
		}
		glEnd();
	}
	glColor3f(1.0f,0.0f,1.0f);
	//draw the ticks on x axis, default 6
	int tick=(m_panel.m_same_coordinates?m_c.max_R:m_c.status[m_is].max_r)/m_dr;
	int tick_interval=tick/6;
	if(tick_interval==0) tick_interval=1;
	CString st;
	for(size_t i=tick_interval; i< tick-tick_interval/2; i+=tick_interval)
	{
		
		glBegin(GL_LINES);
		glVertex2f(k_r*i*m_dr,0.0f);
		glVertex2f(k_r*i*m_dr,-m_glworld.y*0.03f);
		glEnd();
		st.Format("%0.1f",i*m_dr);
		GLTextOut(k_r*(i-0.5f)*m_dr,-m_glworld.y*0.1f,0.0f,(LPCTSTR)st);
	}
	//draw the ticks on y axis, default 6
	float max_f=m_panel.m_same_coordinates?m_c.max_F:m_c.status[m_is].max_f;
	float increase=max_f/6;
	for(float i=increase; i< max_f; i+=increase)
	{

		glBegin(GL_LINES);
		glVertex2f(-m_glworld.x*0.02f,k_f*i);
		glVertex2f(0.0f,k_f*i);
		glEnd();
		st.Format("%0.1f",i);
		GLTextOut(-m_glworld.x*0.1f, k_f*i, 0.0f, (LPCTSTR)st);
	}

}

void CVisualBubblesDlg::draw3DStatisticView()
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_LIGHTING);
	//print the title
	glColor3f(1.0f,1.0f,1.0f);
	GLTextOut(-m_glworld.x*0.95f,m_glworld.y*0.9f,0.0f,"3D statistic view");

	double ks1=sin(m_theta*PI/180.0),ks2=sin(m_phi*PI/180.0),kc1=cos(m_theta*PI/180.0),kc2=cos(m_phi*PI/180.0);
	gluLookAt(m_R*ks1*kc2,m_R*ks1*ks2,m_R*kc1,0.0f,0.0f,0.0f,-kc1*kc2,-kc1*ks2,ks1); //determine the observation point

	//calculate the length of the axises
	float k_h2w=m_glworld.y/m_glworld.x;
	float world_min=m_glworld.y<m_glworld.x?m_glworld.y:m_glworld.x;
	float dR=sqrt(4*world_min*world_min/(k_h2w*k_h2w+2));
	float df=k_h2w*dR;
	float k_r;
	if(m_panel.m_auto_r) k_r=dR/m_c.max_R;
	else k_r=dR/m_panel.m_max_r;
	float min_f=m_c.min_F==0?1:m_c.min_F;
	float k_f;
	if(m_panel.m_f_axis==0) 
	{
		if(m_panel.m_f_prob) k_f=df/m_c.max_F_prob;
		else k_f=df/m_c.max_F;
	}
	else if (m_panel.m_f_axis==1)  
	{
		if(m_panel.m_f_prob)
		{
			min_f= m_c.min_F_prob==0 ? 1.0/m_c.status[0].N_bubble : m_c.min_F_prob;
			k_f=df/log(m_c.max_F_prob/min_f);
		}
		else 
		{
			min_f=m_c.min_F==0?1:m_c.min_F;
			k_f=df/log(m_c.max_F/min_f);
		}
	}
	float k_t=dR/(m_time_max-m_time_min+1);
	glTranslatef(-dR/2,-dR/2,-df/2);

	//draw the axis
	glColor3f(0.0f,0.0f,0.0f);
	GLTextOut(-0.1f*dR,-0.1f*dR,0.0f,"O");
	GLArrow(0.0f,0.0f,0.0f,1.1f*dR,0.0f,0.0f);
	if(m_data_type==0) GLTextOut(1.2f*dR,0.0f,0.0f,"R");
	else GLTextOut(1.2f*dR,0.0f,0.0f,"V");
	GLArrow(0.0f,0.0f,0.0f,0.0f,1.1f*dR,0.0f);
	GLTextOut(0.0f,1.2f*dR,0.0f,"t");
	GLArrow(0.0f,0.0f,0.0f,0.0f,0.0f,1.1f*df);
	if(m_panel.m_f_axis==0) GLTextOut(0.0f,0.1f*dR,1.2f*df,"N");
	else if(m_panel.m_f_axis==1)  GLTextOut(0.0f,0.1f*dR,1.2f*df,"log(f)");
	glEnable(GL_LIGHTING);
	glColor3f(1.0,0.0,0.0);
	for(size_t j=0; j<=m_time_max-m_time_min; ++j)
	{
		vector<size_t>& stat=m_c.status[j+m_time_min-1].statistic;
		vector<float>& fit=m_c.status[j+m_time_min-1].fit;
		for(size_t i=0; i< stat.size(); ++i)
		{
			glColor3f(1.0f,0.0f,0.0f);
			glPushMatrix();
			glTranslatef(k_r*i*m_dr,k_t*j,0.0f);
			if(m_panel.m_f_axis==0) //linear
			{
				if(m_panel.m_f_prob) GLSolidBox(k_r*m_dr, k_t, k_f*stat[i]/(float)m_c.status[j+m_time_min-1].N_bubble);
				else GLSolidBox(k_r*m_dr, k_t, k_f*stat[i]);
			}
			else if (m_panel.m_f_axis==1) //log f
			{
				if(m_panel.m_f_prob) GLSolidBox(k_r*m_dr, k_t, k_f*log((double)(stat[i]?stat[i]:1)/m_c.status[j+j+m_time_min-1].N_bubble/min_f));
				else GLSolidBox(k_r*m_dr, k_t, k_f*log((stat[i]?stat[i]:1)/min_f));			
			}
			glPopMatrix();
		}


		if(m_panel.m_show_fitting) //draw fitting lines parallel to R/V axis
		{
			glColor3f(0.0f,0.0f,1.0f);
			glBegin(GL_LINE_STRIP);
			for(size_t i=0; i< fit.size(); ++i)
			{
				glVertex3f(k_r*(i+0.5)*m_dr,k_t*(j+0.5),k_f*fit[i]);	
			}
			glEnd();
		}		
	}

	if(m_panel.m_show_fitting) //draw fitting lines parallel to t axis
	{
		
	}

	//draw the ticks on R axis, default 6
	int tick_interval=(int)(m_c.max_R/m_dr)/6;
	if(tick_interval==0) tick_interval=1;
	CString st;
	for(size_t i=tick_interval; i*m_dr< m_c.max_R; i+=tick_interval)
	{
		glBegin(GL_LINES);
		glVertex2f(k_r*i*m_dr,0.0f);
		glVertex2f(k_r*i*m_dr,-dR*0.03f);
		glEnd();
		st.Format("%0.1f",i*m_dr);
		GLTextOut(k_r*(i-0.5f)*m_dr,-dR*0.1f,0.0f,(LPCTSTR)st);
	}
	//draw the ticks on t axis, default 6
	tick_interval=(m_time_max-m_time_min+1)/6;
	if(tick_interval==0) tick_interval=1;
	for(size_t i=tick_interval; i< m_time_max-m_time_min+2; i+=tick_interval)
	{

		glBegin(GL_LINES);
		glVertex3f(0.0f,k_t*i,0.0f);
		glVertex3f(-dR*0.03f,k_t*i,0.0f);
		glEnd();
		st.Format("%d",i+m_time_min-1);
		GLTextOut(-dR*0.1f,k_t*i,0.0f,(LPCTSTR)st);
	}
	//draw the ticks on f axis, default 6
	float increase=m_c.max_F/6;
	for(float i=increase; i< m_c.max_F; i+=increase)
	{
		glBegin(GL_LINES);
		glVertex3f(-dR*0.02f,0,k_f*i);
		glVertex3f(0.0f,0,k_f*i);
		glEnd();
		st.Format("%0.1f",i);
		GLTextOut(-dR*0.1f, -dR*0.1f, k_f*i, (LPCTSTR)st);
	}
	
}
void CVisualBubblesDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	if(nType==SIZE_MINIMIZED) return;
	CWnd *p_gl= GetDlgItem(ID_GL_DRAW);
	if(p_gl)
	{
		CRect rect_gl,rect_panel;
		m_panel.GetClientRect(&rect_panel);
		int interval=cx-rect_panel.Width();
		rect_panel.left+=interval;
		rect_panel.right+=interval;
		m_panel.MoveWindow(&rect_panel);
		rect_gl.left=0;
		rect_gl.top=0;
		rect_gl.right=interval;
		rect_gl.bottom=cy;
		m_glView.MoveWindow(&rect_gl);
		glViewport(0,0,rect_gl.Width(),rect_gl.Height());
		setGLPorjection(rect_gl);
	}	
}

void CVisualBubblesDlg::renderNext()
{
	if(m_is<m_c.N_snapshot-1) ++m_is;
	else m_is=0;
	renderScene();
	updateInfo();

}

void CVisualBubblesDlg::renderPrevious()
{
	if(m_is>0) --m_is;
	else m_is=m_c.N_snapshot-1;
	renderScene();
	updateInfo();
}

BOOL CVisualBubblesDlg::renderAnimation()
{
	if(m_timer)	
	{
		KillTimer(1);
		m_timer=FALSE;
		return FALSE; //RETURN state after this operation
	}
	else 
	{
		SetTimer(1,pow(10.0,2+m_panel.m_timer_speed/5.0),NULL);
		m_timer=TRUE;
		return TRUE;
	}
}
void CVisualBubblesDlg::OnTimer(UINT_PTR nIDEvent)
{
	renderNext();
	CDialog::OnTimer(nIDEvent);
}

void CVisualBubblesDlg::updateInfo()
{
	CString str;
	str.Format("%d",m_c.status[m_is].step);
	m_panel.GetDlgItem(IDC_N_STEP)->SetWindowText(str);
	str.Format("%d",m_c.status[m_is].N_bubble);
	m_panel.GetDlgItem(IDC_N_BUBBLE)->SetWindowText(str);
	str.Format("%d",m_is+1);
	m_panel.GetDlgItem(IDC_N_PAGE)->SetWindowText(str);
}

void CVisualBubblesDlg::setAnimationSpeed()
{
	if(m_timer)
	{
		SetTimer(1,pow(10.0,2+m_panel.m_timer_speed/5.0),NULL);
	}
}

void CVisualBubblesDlg::switchView()
{
	if(m_glview_id==2) m_glview_id=0;
	else m_glview_id++;
	renderScene();
}
void CVisualBubblesDlg::doStatistic()
{
	//do further statistic analysis 
	size_t max_f=0,max_F=0,min_f=10000,min_F=10000;
	float max_F_prob=0,min_F_prob=10000;
	CSnapshot* shot=NULL;
	for (int n=0;n<m_c.N_snapshot;++n)
	{
		shot=&m_c.status[n];
		max_f=0;
		min_f=10000;
		shot->statistic.clear();
		shot->statistic.resize(shot->max_r/m_dr+1,0);
		shot->fit.resize(m_c.max_R/m_dr+1,0);
		for(size_t i=0;i<shot->N_bubble;++i)
		{
			++shot->statistic[shot->bubbles[i].R/m_dr];
		}
		for(size_t i=0;i<shot->statistic.size();++i)
		{	
			max_f=shot->statistic[i]>max_f?shot->statistic[i]:max_f;
			min_f=shot->statistic[i]<min_f?shot->statistic[i]:min_f;
		}
		//calculate the fitting data
		for(size_t i=0;i< shot->fit.size();++i)
		{	
			shot->fit[i]=fit_func((i+0.5)*m_dr,n+1);
		}


		shot->max_f=max_f;
		shot->min_f=min_f;

		max_F=max_F>max_f?max_F:max_f;
		min_F=min_F<min_f?min_F:min_f;
		max_F_prob=max_F_prob > (float)max_f/shot->N_bubble ? max_F_prob : (float)max_f/shot->N_bubble;
		min_F_prob=min_F_prob < (float)min_f/shot->N_bubble ? min_F_prob : (float)min_f/shot->N_bubble;
	}
	m_c.max_F=max_F;
	m_c.min_F=min_F;
	m_c.max_F_prob=max_F_prob;
	m_c.min_F_prob=min_F_prob;

	outputStatistic(); //output the data for origin
}


void CVisualBubblesDlg::setTimeRange(int min,int max)
{
	if(min>0 && min<=m_c.N_snapshot && max>0 && max<=m_c.N_snapshot && min<max)
	{
		m_time_min=min;
		m_time_max=max;
		renderScene();
	}
	else MessageBox("The time range you input is not valid!");
}
float CVisualBubblesDlg::fit_func(double x, size_t t)
{
	//Gaussian curve assumption
	double k_ave=7.45238e-4;
	double b_ave=0.40589;
	double k_delta=8.65198e-4;
	double b_delta=0.09862;
	double ave=k_ave*t+b_ave;
	double delta=k_delta*t+b_delta;
	return exp(-(x-ave)*(x-ave)/delta/delta/2)/sqrt(2*PI)/delta*m_dr;
}

BOOL CVisualBubblesDlg::loadFile(const char* filename)
{
	FILE *fp=fopen(filename,"r");
	if(fp==NULL)
	{
		((CVisualBubblesApp*)AfxGetApp())->quitSplash();//we started splash screen before, time to close it
		MessageBox("Cannot open the status file in current directory!\nPlease check whether the file is available");
		return FALSE;
	}
	m_c.status.clear(); //the data container must be empty first
	char str[200];
	fscanf(fp,"%s %f %s %f %s %d %s %d %s",str,&m_c.width, str,&m_c.height, str,&m_c.N_snapshot, str,&m_data_type, str);
	CSnapshot shot;
	CBubble b;
	float max_r=0,max_R=0,sum_r=0,sum_v=0,sum_r2=0,sum_v2=0;
	for (int n=0;n<m_c.N_snapshot;++n)
	{
		max_r=0;
		sum_r=0;
		sum_v=0;
		sum_r2=0;
		sum_v2=0;
		fscanf(fp,"%s %d %s %d",str,&shot.step, str,&shot.N_bubble);
		for(size_t i=0;i<shot.N_bubble;++i)
		{
			fscanf(fp,"%f %f %f %f",&b.x,&b.y,&b.z,&b.R);
			shot.bubbles.push_back(b);
			max_r=b.R>max_r?b.R:max_r;
			sum_r+=b.R;
			sum_v+=b.R*b.R*b.R;
			sum_r2+=b.R*b.R;
			sum_v2+=b.R*b.R*b.R*b.R*b.R*b.R;
		}
		shot.max_r=max_r;
		shot.ave_r=sum_r/shot.N_bubble;
		shot.delta_r=sqrt(sum_r2/shot.N_bubble - shot.ave_r*shot.ave_r);
		max_R=max_r>max_R?max_r:max_R;
		fscanf(fp,"%s",str); //read the "************" line
		m_c.status.push_back(shot);
		shot.bubbles.clear();//clear the container of each snapshot so it can be reused
	}
	m_c.max_R=max_R;
	m_ws=sqrt(m_c.width*m_c.width/2+m_c.height*m_c.height/4); //shorter width of the view box in the world coordinates
	m_time_max=m_c.N_snapshot;
	m_panel.m_time_max=m_time_max;
	fclose(fp);
	//print the ave_r file
	CString ave_file=filename;
	ave_file=ave_file.Mid(0,ave_file.GetLength()-4);
	ave_file+="_ave.txt";
	fp=fopen((LPCTSTR)ave_file,"w");
	for(int i=0; i<m_c.N_snapshot; ++i)	fprintf(fp,"%f\t%f\t%d\n",m_c.status[i].ave_r,m_c.status[i].delta_r,m_c.status[i].N_bubble);
	fclose(fp);
	doStatistic();
	updateInfo();//print information on the control panel

	((CVisualBubblesApp*)AfxGetApp())->quitSplash();//we started splash screen before, time to close it
	return TRUE;
}

void CVisualBubblesDlg::OnDropFiles(HDROP hDropInfo) //accept the dragged-in file
{
	UINT count;
	char filePath[200];
	count = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);
	if(count)
	{        
		int pathLen = DragQueryFile(hDropInfo, 0, filePath, sizeof(filePath));
		((CVisualBubblesApp*)AfxGetApp())->createSplash();
		loadFile(filePath);
		
		CRect rect;
		GetDlgItem(ID_GL_DRAW)->GetClientRect(&rect);
		setGLPorjection(rect);
		renderScene();
	}
	DragFinish(hDropInfo);

	CDialog::OnDropFiles(hDropInfo);
}

void CVisualBubblesDlg::setGLPorjection( CRect& rect )
{
	float k=rect.Height()/(float)rect.Width();
	if(k>1)
	{
		m_glworld.x=m_ws;
		m_glworld.y=k*m_ws;
	}
	else
	{
		m_glworld.x=m_ws/k;
		m_glworld.y=m_ws;
	}
	m_glworld.z=10*m_ws;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-m_glworld.x,m_glworld.x,-m_glworld.y,m_glworld.y,-m_glworld.z,m_glworld.z);
}

void CVisualBubblesDlg::outputStatistic()
{
	CString filename;
	FILE* fp=NULL;
	CreateDirectory("statistic",NULL);
	CreateDirectory("statistic/time_slice",NULL);
	for(int i=0; i<m_c.N_snapshot; ++i)
	{
		filename.Format("statistic/time_slice/%d.txt",i);
		fp=fopen((LPCTSTR)filename,"w");
		if(!fp)
		{
			MessageBox("Cannot open file: "+filename);
			exit(1);
		}
		vector<size_t>& st=m_c.status[i].statistic;
		for(int j=0; j< st.size(); ++j)
		{
			fprintf(fp,"%d\t%d\n",j,st[j]);
		}
		fclose(fp);
	}

	CreateDirectory("statistic/x_slice",NULL);
	size_t elem=0;
	for(int i=0; i< m_c.max_R/m_dr+1; ++i)
	{
		filename.Format("statistic/x_slice/%d.txt",i);
		fp=fopen((LPCTSTR)filename,"w");
		if(!fp)
		{
			MessageBox("Cannot open file: "+filename);
			exit(1);
		}
		for(int j=0; j< m_c.N_snapshot; ++j)
		{
			if(i+1>m_c.status[j].statistic.size()) elem=0;
			else elem=m_c.status[j].statistic[i];
			fprintf(fp,"%d\t%d\n",j,elem);
		}
		fclose(fp);
	}

	//output the whole 3D surface
	fp=fopen("statistic/3D.txt","w");
	if(!fp)
	{
		MessageBox("Error!Cannot open file: 3D.txt");
		exit(1);
	}
	for(int i=0; i<m_c.N_snapshot; ++i)
	{
		vector<size_t>& st=m_c.status[i].statistic;
		for(int j=0; j< st.size(); ++j)
		{
			fprintf(fp,"%d\t%f\t%d\n",i,j*m_dr,st[j]);
		}	
	}
	fclose(fp);
}
