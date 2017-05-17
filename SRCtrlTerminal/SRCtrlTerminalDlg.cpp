// SRCtrlTerminalDlg.cpp : 实现文件
//
#include "stdafx.h"
#include "SRCtrlTerminal.h"
#include "SRCtrlTerminalDlg.h"
#include "afxdialogex.h"
#include <iostream>
#include <cmath>
#include <time.h>
#include <opencv2\opencv.hpp>
#include <opencv2\highgui\highgui.hpp>

#include "camerads.h"

using namespace cv;
using namespace std;

#pragma warning(disable : 4995)
#pragma warning(disable : 4018)
#pragma warning(disable : 4996)
#pragma warning(disable : 4244)
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
/*求角度余弦值的函数*/
static double angle(cv::Point pt1, cv::Point pt2, cv::Point pt0);
//用户自定义的变量
VideoCapture capture; //相机结构体
Mat frame;            //帧
//相机内参矩阵
Mat cameraMatrix = (Mat_<double>(3, 3) << 6.416428e+02, 0.1529, 2.746019e+02, 0, 6.384720e+02, 2.528256e+02, 0, 0, 1);
//相机畸变矩阵
Mat distCoeffs = (Mat_<double>(5, 1) << -0.4423, 0.2297, -0.0027, -0.00200, 0);
drawCircleData cirInfo; //画圆相关
drawTextData textInfo;  //显示文字相关
uartArr uartSendBuff;   //串口发送数据相关
//坐标信息
static int startFlag = 0; //程序开始标志，1：开始
static int threadFlag = 0;//多线程创建标志，1：创建
static int state_com = 0; //忽略端口复选框标志，1：忽略端口连接
static int comFlag = 0;   //端口连接状态，1：端口已连接
static int state_pix = 0; //显示像素坐标复选框标志，1：显示像素坐标
double Xp, Yp, Xc, Yc, Zc=2000; //坐标变量，Xp:像素坐标u；Yp:像素坐标v；【Xc,Yc,Zc】:相机坐标
//坐标变量，【CXw,CYw,CZw】：圆的世界坐标；【TXw,TYw,TZw】：三角形的世界坐标；【RXw,RYw,RZw】：矩形的世界坐标
double CXw, CYw, CZw, TXw, TYw, TZw, RXw, RYw, RZw;
double DX=1000, DY=800, DZ=2000;//平移矩阵参数，相机无旋转
double fdx = 6.416428e+02,fdy = 6.384720e+02;//fdx：f/dx；fdy：f/dy;
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();
// 对话框数据
	enum { IDD = IDD_ABOUTBOX };
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);  // DDX/DDV 支持
// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()
// CSRCtrlTerminalDlg 对话框
CSRCtrlTerminalDlg::CSRCtrlTerminalDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSRCtrlTerminalDlg::IDD, pParent)
	, m_ctrlCmd(_T(""))
	, m_datRec(_T(""))
	, m_MessageShow(_T(""))
	{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSRCtrlTerminalDlg::DoDataExchange(CDataExchange* pDX)
{
CDialogEx::DoDataExchange(pDX);
DDX_Text(pDX, IDC_EDITCMD, m_ctrlCmd);
DDX_Text(pDX, IDC_EDITDATREC, m_datRec);
DDX_Control(pDX, IDC_COMBO_COMLS, m_combLsCom);
DDX_Control(pDX, IDC_COMBO_BAUDRATE, m_combBaudrate);
DDX_Control(pDX, IDC_MSCOMM1, m_mscom);
DDX_Text(pDX, IDC_EDIT_MESSAGE, m_MessageShow);
DDX_Control(pDX, IDC_COMBO_CAMERA, m_camera);
}

BEGIN_MESSAGE_MAP(CSRCtrlTerminalDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_OPEN, &CSRCtrlTerminalDlg::OnBnClickedButtonOpen)
	ON_BN_CLICKED(IDC_BUTTON_SENDCMD, &CSRCtrlTerminalDlg::OnBnClickedButtonSendcmd)
	ON_BN_CLICKED(IDC_BUTTON_EXIT, &CSRCtrlTerminalDlg::OnBnClickedButtonExit)
	ON_BN_CLICKED(IDC_BUTTON_SCAN, &CSRCtrlTerminalDlg::OnBnClickedButtonScan)
	ON_BN_CLICKED(IDC_BUTTON_START, &CSRCtrlTerminalDlg::OnBnClickedButtonStart)
	ON_MESSAGE(WM_COMSEND, OnComSend)
	ON_MESSAGE(VM_DRAWBACKGROUND, OnDrawBackGround)
	ON_MESSAGE(VM_DRAWCIRCLE, OnDrawCircle)
	ON_MESSAGE(VM_DRAWTEXT, OnDrawText)
	ON_BN_CLICKED(IDC_CHECK_COM, &CSRCtrlTerminalDlg::OnBnClickedCheckCom)
	ON_BN_CLICKED(IDC_CHECK_PIX, &CSRCtrlTerminalDlg::OnBnClickedCheckPix)
END_MESSAGE_MAP()
// CSRCtrlTerminalDlg 消息处理程序
BOOL CSRCtrlTerminalDlg::OnInitDialog()
{
    /*以下两句用来在MFC中打开一个控制台*/
    AllocConsole();
	freopen("CONOUT$", "a+", stdout);
	CDialogEx::OnInitDialog();
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);
	CMenu* pSysMenu = GetSystemMenu(FALSE);
    pSysMenu->ModifyMenu(SC_CLOSE,MF_BYCOMMAND | MF_GRAYED );//禁用关闭按钮
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

	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标
	//在对话框初始化时，设置程序的默认选项
	CString str;
	int i;
	for(i=0;i<8;i++)
	{
		str.Format(_T("COM %d"),i+1);
		m_combLsCom.InsertString(i,str);
	}
	m_combLsCom.SetCurSel(0);//预置COM口
	//波特率选择组合框
	CString str1[]={_T("300"),_T("600"),_T("1200"),_T("2400"),_T("4800"),_T("9600"),
		            _T("19200"),_T("38400"),_T("43000"),_T("56000"),_T("57600"),_T("115200")};
	for(int i=0;i<12;i++)
	{
		int judge_tf=m_combBaudrate.AddString(str1[i]);
		if((judge_tf==CB_ERR)||(judge_tf==CB_ERRSPACE))
			MessageBox(_T("build baud error!"));
	}
	m_combBaudrate.SetCurSel(0);//预置波特率为"115200"
	//相机选择下拉框
	CString str2;
		int cai;
	for(cai = 0; cai<3; cai++)
		{
		str2.Format(_T("Camera %d"),cai);
		m_camera.InsertString(cai, str2);
		}
	m_camera.SetCurSel(1);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CSRCtrlTerminalDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
void CSRCtrlTerminalDlg::OnPaint()
{
	::SendMessage(::AfxGetMainWnd()->m_hWnd,VM_DRAWBACKGROUND,FALSE,NULL);
    CDialogEx::OnPaint();	
}
//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CSRCtrlTerminalDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
/*点击连接端口/断开端口按钮后执行的动作*/
void CSRCtrlTerminalDlg::OnBnClickedButtonOpen()
{
	CString str,str1,n;	 //定义字符串
	GetDlgItemText(	IDC_BUTTON_OPEN,str);
	CWnd *h1;
	h1=GetDlgItem(IDC_BUTTON_OPEN);	//指向控件的caption
	if(!m_mscom.get_PortOpen())
	{
	//取得所选的字符串，并存放在str1里面
	m_combBaudrate.GetLBText(m_combBaudrate.GetCurSel(),str1);
	str1=str1+','+'n'+','+'8'+','+'1'; //这句话很关键
		
	m_mscom.put_CommPort((m_combLsCom.GetCurSel()+1));	//选择串口
	m_mscom.put_InputMode(1);		//设置输入方式为二进制方式
	m_mscom.put_Settings(str1);		//波特率为（波特率组合框）无校验，8数据位，1个停止位
	m_mscom.put_InputLen(1024);		//设置当前接收区数据长度为1024
	m_mscom.put_RThreshold(1);		//缓冲区一个字符引发事件
	m_mscom.put_RTSEnable(1);	    //设置RT允许
			
	m_mscom.put_PortOpen(true);		//打开串口
	if(m_mscom.get_PortOpen())
	  {
	   comFlag = 1;
	   str=_T("断开端口");
	   UpdateData(true);
	   h1->SetWindowText(str);	   //改变按钮名称为"断开端口"
	  }		
	}	
	else 
	{
	m_mscom.put_PortOpen(false);
	if(str!=_T("连接端口"))
	  {
	   comFlag = 0;
	   str=_T("连接端口");
	   UpdateData(true);
	   h1->SetWindowText(str);	//改变按钮名称为"连接端口"
	  }
	}
}
/*点击发送命令按钮后执行的动作*/
void CSRCtrlTerminalDlg::OnBnClickedButtonSendcmd()
{
   UpdateData(true);						   //更新控件数据
   m_mscom.put_Output(COleVariant(m_ctrlCmd));//把发送编辑框的数据发送出去
}
/*点击退出程序按钮后执行的动作*/
void CSRCtrlTerminalDlg::OnBnClickedButtonExit()
{
   startFlag = 0; //用于结束线程
   capture.release(); //用于关闭摄像头
   destroyAllWindows(); //销毁所有的CV窗口
   if(m_mscom.get_PortOpen()) //关闭打开的端口
		m_mscom.put_PortOpen(false);
   CDialogEx::OnCancel();
}
BEGIN_EVENTSINK_MAP(CSRCtrlTerminalDlg, CDialogEx)
	ON_EVENT(CSRCtrlTerminalDlg, IDC_MSCOMM1, 1, CSRCtrlTerminalDlg::OnCommMscomm1, VTS_NONE)
END_EVENTSINK_MAP()
/*串口控件的事件处理函数*/
void CSRCtrlTerminalDlg::OnCommMscomm1()
{  
   if(m_mscom.get_CommEvent()==2)
	 {
	 char str[1024]={0};
	 long k;
	 VARIANT InputData=m_mscom.get_Input();	//读缓冲区
	 COleSafeArray fs;
	 fs=InputData;	//VARIANT型变量转换为COleSafeArray型变量
	 for(k=0;k<(fs.GetOneDimSize());k++)
	    fs.GetElement(&k,str+k);	//转换为BYTE型数组
	 cout<<"串口接收到的数据"<<sizeof(str)<<";"<<m_datRec<<endl;
	 m_datRec += str;      //接收到编辑框里面;
	 //SetTimer(1,10,NULL);		//延时10ms
	 UpdateData(false);
	}
}
/*按下扫描端口按钮后执行的动作*/
void CSRCtrlTerminalDlg::OnBnClickedButtonScan()
{
   CRegKey setreg;
   LPCTSTR ps = _T("HARDWARE\\DEVICEMAP\\SERIALCOMM\\");
   LONG lResult = setreg.Open(HKEY_LOCAL_MACHINE, ps, KEY_READ);
   if (ERROR_SUCCESS != lResult)
	  {
	   MessageShowOnDialog(CString("无可用端口!"), true);
	  }
	else
      {
	   TCHAR pszName[80]; BYTE data[80]; DWORD pnNameLength = 80;
	   DWORD lpcvaluename = 80; DWORD size =80; DWORD type = REG_SZ;
	   CStringArray CSAcomport; CString name; int index = 0;
	   lResult = RegEnumValue(setreg.m_hKey, index, pszName, &lpcvaluename,NULL,&type,
		data, &size );
	   if(lResult != ERROR_NO_MORE_ITEMS && lResult == ERROR_SUCCESS)
		  MessageShowOnDialog(CString("可用的端口有："), true);
	   else
		  MessageShowOnDialog(CString("无可用端口!"), true);
       while (lResult != ERROR_NO_MORE_ITEMS && lResult == ERROR_SUCCESS)
		     {
		     name.Format(_T("%s"), data);
		     CSAcomport.Add(name);
		     index ++; lpcvaluename = 80; size = 80;
		     lResult = RegEnumValue(setreg.m_hKey, index, pszName, &lpcvaluename, NULL, &type, data, &size );
		     }
       for(int si = 0; si< CSAcomport.GetUpperBound()+1; si++)
		  {
		   MessageShowOnDialog(CSAcomport.GetAt(si),false);
		   MessageShowOnDialog(CString(" "),false);
		  }
	   if(!CSAcomport.IsEmpty())
		  {
  	       MessageShowOnDialog(CString(""),true);
	       CSAcomport.RemoveAll();
		  }
	   setreg.Close(); //关闭注册表
      }
   /*以下部分用来扫描可用的视频设备*/
   int cam_count;
   cam_count = CCameraDS::CameraCount();
   if(cam_count == 0)
	  MessageShowOnDialog(CString("无可用的视频输入设备"),true);
   else
	  {
	  CString camNum;
	  CStringArray CSCamnum;
	  MessageShowOnDialog(CString("可用的摄像头有："),true);
	  for(int i = 0; i < cam_count; i++)
         {
          char camera_name[1024];  
          int retval = CCameraDS::CameraName(i, camera_name, sizeof(camera_name) );
          if(retval >0)
		    {
			 camNum.Format(_T("%d"), i);
		     CSCamnum.Add(CString("Camera #"));
		     CSCamnum.Add(camNum);
			 CSCamnum.Add(CString(" "));
			 CSCamnum.Add(CString(camera_name));
			 for(int ci = 0; ci< CSCamnum.GetUpperBound()+1; ci++)
		        {
		         MessageShowOnDialog(CSCamnum.GetAt(ci),false);
		        }
			 MessageShowOnDialog(CString(""),true);
		    }
          else
			{
			CSCamnum.RemoveAll();
			camNum.Format(_T("%d"), i);
			CSCamnum.Add(CString("无法获得摄像头#"));
			CSCamnum.Add(camNum);
			CSCamnum.Add(CString("的名称"));
			for(int ci = 0; ci< CSCamnum.GetUpperBound()+1; ci++)
		       {
		        MessageShowOnDialog(CSCamnum.GetAt(ci),false);
		       }
			MessageShowOnDialog(CString(""),true);
		    }
         CSCamnum.RemoveAll();
         }
      }
}
/*按下开始运行按钮后执行的动作*/
void CSRCtrlTerminalDlg::OnBnClickedButtonStart()
{
if(!threadFlag)
   {
   startFlag = 1;
   CString str3;
   m_camera.GetLBText(m_camera.GetCurSel(), str3);
   if (str3 == CString("Camera 0") )
	   capture.open(0);
   if (str3 == CString("Camera 1") )
	   capture.open(1);
   if (str3 == CString("Camera 2") )
	   capture.open(2);
   if(!capture.isOpened())
	   AfxMessageBox(_T("摄像头打开失败!"));
   capture>>frame; //USB摄像头无法读取到第一帧
   capture>>frame; //所以这里连续读两帧
   if(!frame.data &&startFlag == 1)
	 {
	  AfxMessageBox(_T("未获取到图像信息"));
	  startFlag = 0;
	 }
   else
	 {
	 //使用多线程运行主要算法
	 if(!comFlag && !state_com)
		 AfxMessageBox(_T("请先连接端口!"));
	 else
	       pThread = AfxBeginThread(ThreadFunc, NULL);
	 }
   }
}
/*将系统的重要消息通过文本框显示*/
void CSRCtrlTerminalDlg::MessageShowOnDialog(CString Message, bool clf)
{	
   if(m_MessageShow.GetLength()>200)
	  m_MessageShow = CString("");
   m_MessageShow += Message;
   if(clf)
	  m_MessageShow +="\r\n";
   UpdateData(false);
}
/*消息处理函数，绘制背景*/
LRESULT CSRCtrlTerminalDlg::OnDrawBackGround(WPARAM wParam, LPARAM lParam)
{
   CRect rect;
   CWnd *pWin = GetDlgItem(IDC_MAPAXES_STATIC);//获取该控件指针，就可以对该控件直接操作
   pWin->MoveWindow(10,12,640,480,true);//调整大小
   pWin->GetClientRect(rect);//把控件的长宽、坐标等信息保存在rect里
   CDC *pDc = pWin->GetDC();//获取该控件的画布,有了画布，下面可以自由的画图了
   pDc->Rectangle(rect);
   CBrush myBrush;
   myBrush.CreateSolidBrush(RGB(222,222,222));
   pDc->FillRect(rect,&myBrush);
   CFont m_Font;
   m_Font.CreatePointFont(100,_T("黑体"),pDc);
   pDc->SelectObject(&m_Font);
   pDc->SetTextColor(RGB(0,0,0));
   pDc->SetBkMode(TRANSPARENT);
   BOOL TextOk = pDc->TextOut(1,1,CString("FPS:"));
   m_Font.DeleteObject();
   myBrush.DeleteObject();//释放内存
   ReleaseDC(pDc);
   return 0;
}
/*消息处理函数，绘制实心圆*/
LRESULT CSRCtrlTerminalDlg::OnDrawCircle(WPARAM wParam, LPARAM lParam)
{
   drawCircleData* pA = (drawCircleData*)wParam; //消息映射
   CWnd *pWin = GetDlgItem(IDC_MAPAXES_STATIC);//获取该控件的指针，就可以对该控件直接操作了
   CDC *pDc = pWin->GetDC();//获取该控件的画布,有了画布，下面可以自由的画图了
   CBrush myBrush(RGB(pA->r, pA->g, pA->b)), *pOldBrush;
   pOldBrush = pDc->SelectObject(&myBrush);
   pDc->Ellipse(pA->pu, pA->pv, (pA->pu)+7, (pA->pv)+7);
   pDc->SelectObject(pOldBrush);
   myBrush.DeleteObject();
   ReleaseDC(pDc);
   return 0;
}
/*消息处理函数，绘制文字*/
LRESULT CSRCtrlTerminalDlg::OnDrawText(WPARAM wParam, LPARAM lParam)
{
   drawTextData* pA = (drawTextData*)wParam; //消息映射
   CWnd *pWin = GetDlgItem(IDC_MAPAXES_STATIC);//获取该控件的指针，就可以对该控件直接操作了
   CDC *pDc = pWin->GetDC();//获取该控件的画布
   CFont m_Font;
   m_Font.CreatePointFont(100,_T("黑体"),pDc);
   pDc->SelectObject(&m_Font);
   pDc->SetTextColor(RGB(105,105,105));
   pDc->SetBkMode(TRANSPARENT);
   BOOL TextOk = pDc->TextOut(pA->ptx,pA->pty,CString(pA->ptinfo));
   m_Font.DeleteObject();
   ReleaseDC(pDc);	
   return 0;	
}
/*消息处理函数，串口发送数据*/
LRESULT CSRCtrlTerminalDlg::OnComSend(WPARAM wParam, LPARAM lParam)
{
   uartArr* pA = (uartArr*)wParam; //消息映射
   CString uartData, t1, t2,t3;
   t1.Format(_T("%d"),pA->id); t2.Format(_T("%d"),pA->wx); t3.Format(_T("%d"),pA->wy); 
   uartData = CString("srs,")+t1+CString(",")+t2+CString(",")+t3+CString(",sre");
   m_mscom.put_Output(COleVariant(uartData));
   return 0;
}
/*线程函数，用于创建进程 ，工作线程，运行主要算法*/
UINT ThreadFunc(LPVOID lpParam)  
{ 
   threadFlag = 1;
   while(startFlag)
	    {
		capture>>frame;
		Mat frameCalibration;
        Mat map1, map2;
        Size imageSize;
		Mat dstImage, grayImage, edgeImage, cannyImage;
        vector<std::vector<Point> > contours; //角点
        vector<cv::Point> approx; //轮廓
        vector<cv::Point> approx1;
        double time0 = static_cast<double>(getTickCount());
		::SendMessage(::AfxGetMainWnd()->m_hWnd,VM_DRAWBACKGROUND,FALSE,NULL);
        imageSize = frame.size();
        initUndistortRectifyMap(cameraMatrix, distCoeffs, Mat(),getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imageSize, 1, imageSize, 0),imageSize, CV_16SC2, map1, map2);
        remap(frame, frameCalibration, map1, map2, INTER_LINEAR);
        cvtColor(frameCalibration, grayImage, CV_BGR2GRAY);//转换为灰度图像
        threshold(grayImage, dstImage, 245, 255, THRESH_BINARY);//二值化处理
        Canny(dstImage,cannyImage, 100, 300, 3); //Canny边缘检测
        findContours(cannyImage.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
   double sr = 0;
   for (int i = 0; i < contours.size(); i++)
       {
	    approxPolyDP(Mat(contours[i]), approx1, arcLength(Mat(contours[i]), true)*0.03, true);
		approxPolyDP(Mat(contours[i]), approx,  arcLength(Mat(contours[i]), true)*0.1, true);
		if (fabs(contourArea(contours[i])) < 100 || !isContourConvex(approx))
		   continue;
		if (approx1.size() >= 7)//是圆
			{
			cirInfo.r = 255; cirInfo.g = 255; cirInfo.b = 0;
			sr = contourArea(contours[i]);
			Point circle_center;
			Point text_point;
			Moments dstMoments = moments(approx1); 
			circle_center.x = dstMoments.m10/dstMoments.m00;
			circle_center.y = dstMoments.m01/dstMoments.m00;
			cirInfo.pu = circle_center.x; cirInfo.pv = circle_center.y;
			CXw = Zc * (circle_center.x-320)/fdx + DX;
			CYw = Zc * (circle_center.y-240)/fdy + DY;
			CZw = Zc - DZ;
			cout<<"圆形："<<circle_center.x<<", "<<circle_center.y<<"->"<<(int)CXw<<", "<<(int)CYw<<endl;
			if(!state_com && comFlag)
			  {
			  uartSendBuff.id = 2; uartSendBuff.wx = int(CXw); uartSendBuff.wy = int(CYw);
			  ::SendMessage(::AfxGetMainWnd()->m_hWnd,WM_COMSEND,(WPARAM)&uartSendBuff,NULL);
		      }
			textInfo.ptx = circle_center.x-60 >0?circle_center.x-60:circle_center.x;
			textInfo.pty = circle_center.y-20 >0?circle_center.y-8:circle_center.y+16;
			if(state_pix)
			    sprintf(textInfo.ptinfo, "(%d,%d)", circle_center.x, circle_center.y);
			else
				sprintf(textInfo.ptinfo, "(%d,%d)", int(CXw), int(CYw));
			::SendMessage(::AfxGetMainWnd()->m_hWnd,VM_DRAWCIRCLE,(WPARAM)&cirInfo,NULL);
			::SendMessage(::AfxGetMainWnd()->m_hWnd,VM_DRAWTEXT,(WPARAM)&textInfo,NULL);
			}	
		if (approx.size() == 3) //是三角形
		   {
		    cirInfo.r = 255; cirInfo.g = 0; cirInfo.b = 0;
			Point tri_center;
			Point text_point;
			Moments dstMoments = moments(approx); 
			tri_center.x = dstMoments.m10/dstMoments.m00;
			tri_center.y = dstMoments.m01/dstMoments.m00;
			cirInfo.pu = tri_center.x; cirInfo.pv = tri_center.y;
			TXw = Zc * (tri_center.x-320)/fdx + DX;
		    TYw = Zc * (tri_center.y-240)/fdy + DY;
			TZw = Zc - DZ;
			cout<<"三角形："<<tri_center.x<<", "<<tri_center.y<<"->"<<(int)TXw<<", "<<(int)TYw<<endl;
			if(!state_com && comFlag)
				{
			    uartSendBuff.id = 3; uartSendBuff.wx = int(TXw); uartSendBuff.wy = int(TYw);
			    ::SendMessage(::AfxGetMainWnd()->m_hWnd,WM_COMSEND,(WPARAM)&uartSendBuff,NULL);
				}
            textInfo.ptx = tri_center.x-60 >0?tri_center.x-60:tri_center.x;
			textInfo.pty = tri_center.y-20 >0?tri_center.y-8:tri_center.y+16;
			if(state_pix)
			  sprintf(textInfo.ptinfo, "(%d,%d)", tri_center.x, tri_center.y);
			else
			  sprintf(textInfo.ptinfo, "(%d,%d)", int(TXw), int(TYw));
			::SendMessage(::AfxGetMainWnd()->m_hWnd,VM_DRAWCIRCLE,(WPARAM)&cirInfo,NULL);
			::SendMessage(::AfxGetMainWnd()->m_hWnd,VM_DRAWTEXT,(WPARAM)&textInfo,NULL);
		   }
		   else if (approx.size() == 4)//是矩形
           {
			double st = 0;
			st = contourArea(contours[i]);
			if(sr == st)
			  {
			  //一个误检测到的圆
			  }
			else
			  {
			   int rf = 4;
			   cirInfo.r = 0; cirInfo.g = 255; cirInfo.b = 0;
			   Moments dstMoments = moments(approx);  
			   Point rect_center;
			   Point text_point;
			   rect_center.x = dstMoments.m10/dstMoments.m00;
			   rect_center.y = dstMoments.m01/dstMoments.m00;
			   cirInfo.pu = rect_center.x; cirInfo.pv = rect_center.y;
			   RXw = Zc * (rect_center.x-320)/fdx + DX;
			   RYw = Zc * (rect_center.y-240)/fdy + DY;
			   RZw = Zc - DZ;
			   cout<<"矩形："<<rect_center.x<<", "<<rect_center.y<<"->"<<(int)RXw<<", "<<(int)RYw<<endl;
			   if(!state_com && comFlag)
				 {
			     uartSendBuff.id = 4; uartSendBuff.wx = int(RXw); uartSendBuff.wy = int(RYw);
			     ::SendMessage(::AfxGetMainWnd()->m_hWnd,WM_COMSEND,(WPARAM)&uartSendBuff,NULL);
				 }
               textInfo.ptx = rect_center.x-60 >0?rect_center.x-60:rect_center.x;
			   textInfo.pty = rect_center.y-20 >0?rect_center.y-8:rect_center.y+8;
			   if(state_pix)
			     sprintf(textInfo.ptinfo, "(%d,%d)", rect_center.x, rect_center.y);
			   else
			     sprintf(textInfo.ptinfo, "(%d,%d)", int(RXw), int(RYw));
			   ::SendMessage(::AfxGetMainWnd()->m_hWnd,VM_DRAWCIRCLE,(WPARAM)&cirInfo,NULL);
			   ::SendMessage(::AfxGetMainWnd()->m_hWnd,VM_DRAWTEXT,(WPARAM)&textInfo,NULL);
			  }	
           }
		   else
			  {
			   //do nothing now
			  }
       } //for 2
	   sprintf(textInfo.ptinfo, "%d", int(getTickFrequency() / (getTickCount() - time0)));
	   textInfo.ptx = 28; textInfo.pty = 1;
	   ::SendMessage(::AfxGetMainWnd()->m_hWnd,VM_DRAWTEXT,(WPARAM)&textInfo,NULL);
	   contours.clear();
       approx.clear();
       approx1.clear();
	   Sleep(SLEEPTIME); //防止线程阻塞 
	  }
   threadFlag = 0;
   return 0;
}
/*用来禁用ALT+F4和ESC式程序关闭*/
BOOL CSRCtrlTerminalDlg::PreTranslateMessage(MSG* pMsg)
{
   if(pMsg->message == WM_KEYDOWN)
   if (pMsg->wParam==VK_ESCAPE ||pMsg->wParam==VK_RETURN) //屏蔽回车和ESC
      return TRUE;
   if (pMsg->message == WM_SYSKEYDOWN&&pMsg->wParam== VK_F4 ) //屏蔽ALT+F4
      return TRUE;
   return CDialog::PreTranslateMessage(pMsg);
}
/*复选框消息响应函数，忽略端口*/
void CSRCtrlTerminalDlg::OnBnClickedCheckCom()
{
   CButton* pBtn = (CButton*)GetDlgItem(IDC_CHECK_COM); 
   if(pBtn->GetCheck())
	   state_com = 1;
   else
	   state_com = 0;
   cout<<"com: "<<state_com<<endl;
}
/*复选框消息响应函数，像素坐标*/
void CSRCtrlTerminalDlg::OnBnClickedCheckPix()
{
   CButton* pBtn = (CButton*)GetDlgItem(IDC_CHECK_PIX); 
   if(pBtn->GetCheck())
	   state_pix = 1;
   else
	   state_pix = 0;
   cout<<"pix: "<<state_pix<<endl;
}
/*计算角度余弦值功能函数*/
static double angle(cv::Point pt1, cv::Point pt2, cv::Point pt0)
{
   double dx1 = pt1.x - pt0.x;
   double dy1 = pt1.y - pt0.y;
   double dx2 = pt2.x - pt0.x;
   double dy2 = pt2.y - pt0.y;
   return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}
