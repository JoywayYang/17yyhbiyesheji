
// SRCtrlTerminal.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CSRCtrlTerminalApp:
// �йش����ʵ�֣������ SRCtrlTerminal.cpp
//

class CSRCtrlTerminalApp : public CWinApp
{
public:
	CSRCtrlTerminalApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CSRCtrlTerminalApp theApp;