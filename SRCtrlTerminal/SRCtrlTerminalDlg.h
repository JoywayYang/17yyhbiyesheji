
// SRCtrlTerminalDlg.h : ͷ�ļ�
//
#pragma once
#include "afxwin.h"
#include "mscomm1.h"
#define SLEEPTIME 300
//��ʵ��Բ�Ľṹ��
struct drawCircleData
	{
	int pu;int pv;//��ʾ����������
	int r;int g;int b;//��ʾ����ɫ
	};
//��ʾ���ֵĽṹ����Ϣ
struct drawTextData
	{
	int ptx; int pty; //������ʾ������
	char ptinfo[20];
	};
//�������ݽṹ����Ϣ
struct uartArr
	{
	int id;
	int wx;
	int wy;
	int wz;
	};
/*�̺߳���*/
UINT ThreadFunc(LPVOID lpParam); 
// CSRCtrlTerminalDlg �Ի���
class CSRCtrlTerminalDlg : public CDialogEx
{
// ����
public:
	CSRCtrlTerminalDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_SRCTRLTERMINAL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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
	//�߳����
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
