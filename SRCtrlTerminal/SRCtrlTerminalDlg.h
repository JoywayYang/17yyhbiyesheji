
// SRCtrlTerminalDlg.h : 头文件
//
#pragma once
#include "afxwin.h"
#include "mscomm1.h"
#define SLEEPTIME 300
//画实心圆的结构体
struct drawCircleData
	{
	int pu;int pv;//显示的像素坐标
	int r;int g;int b;//显示的颜色
	};
//显示文字的结构体信息
struct drawTextData
	{
	int ptx; int pty; //文字显示的坐标
	char ptinfo[20];
	};
//串口数据结构体信息
struct uartArr
	{
	int id;
	int wx;
	int wy;
	int wz;
	};
/*线程函数*/
UINT ThreadFunc(LPVOID lpParam); 
// CSRCtrlTerminalDlg 对话框
class CSRCtrlTerminalDlg : public CDialogEx
{
// 构造
public:
	CSRCtrlTerminalDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_SRCTRLTERMINAL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnComSend(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDrawBackGround(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDrawCircle(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDrawText(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
protected:
	//线程相关
    CWinThread* pThread;
private:
	CString m_ctrlCmd;
	CString m_datRec;
	CComboBox m_combLsCom;
	CComboBox m_combBaudrate;
public:
	CMscomm1 m_mscom;
	afx_msg void OnBnClickedButtonOpen();
	afx_msg void OnBnClickedButtonSendcmd();
	afx_msg void OnBnClickedButtonExit();
	DECLARE_EVENTSINK_MAP()
	void OnCommMscomm1();
	void MessageShowOnDialog(CString Message, bool clf);
	BOOL CSRCtrlTerminalDlg::PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedButtonScan();
	CString m_MessageShow;
	afx_msg void OnBnClickedButtonStart();
private:
	CComboBox m_camera;
public:
	afx_msg void OnBnClickedCheckCom();
	afx_msg void OnBnClickedCheckPix();
	};
