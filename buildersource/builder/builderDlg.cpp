
// builderDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "builder.h"
#include "builderDlg.h"
#include "afxdialogex.h"
#include "Resource.h"
#include <windows.h>
#include<iostream>
#include <fstream>
#include <string>
#include<stdlib.h>
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//改参数默认值
//#define SERVER_ADDR	"\trootCmd.PersistentFlags().StringVarP(&ip, \"server_addr\", \"t\", \"%s\", \"server_addr\")"
//#define SERVER_PORT "\trootCmd.PersistentFlags().StringVarP(&port, \"server_port\", \"p\", \"%s\", \"server_port\")"
//#define SERVER_FORWARD_PORT "\trootCmd.PersistentFlags().StringVarP(&fport, \"server_forward_port\", \"f\", \"%s\", \"server_forward_port\")"

//改FileContent处
#define SERVER_ADDR	"  server_addr = ` + \"%s\" + `"
#define SERVER_PORT "  server_port = ` + \"%s\" + `"
#define SERVER_FORWARD_PORT "remote_port = ` + \"%s\" + `"


#define FRPCONFFILEPATH ".\\sub\\root.go"
#define FRPCOMPILEPATH ".\\frp\\cmd\\frpc\\main.go"


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CbuilderDlg 对话框



CbuilderDlg::CbuilderDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_BUILDER_DIALOG, pParent)
	, ServerAddr(_T("1.1.1.1"))
	, ServerPort(_T("80"))
	, ForwardPort(_T("8080"))
	, isx64(TRUE)
	, isx86(FALSE)
	, islinux(FALSE)
	, iswindows(FALSE)
	, isupx(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CbuilderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, SERVERADDR_EDIT1, ServerAddr);
	//DDV_MaxChars(pDX, ServerAddr, 255);
	DDX_Text(pDX, SERVERPORT_EDIT2, ServerPort);
	DDX_Text(pDX, FORWARDPORT_EDIT3, ForwardPort);

	DDX_Check(pDX, X64_CHECK1, isx64);
	DDX_Check(pDX, X86_CHECK2, isx86);
	DDX_Check(pDX, Windows_CHECK2, iswindows);
	DDX_Check(pDX, Linux_CHECK3, islinux);
	DDX_Check(pDX, Windows_CHECK4, isupx);
}

BEGIN_MESSAGE_MAP(CbuilderDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CbuilderDlg::OnBnClickedGenerate)	//click gerenate

END_MESSAGE_MAP()


// CbuilderDlg 消息处理程序

/*读取指定行*/
string ReadText(string filename, int line)
{
	ifstream fin;
	fin.open(filename, ios::in);
	string strVec[309];     //文本中总共有10行
	int i = 0;
	while (!fin.eof())
	{
		string inbuf;
		getline(fin, inbuf, '\n');
		strVec[i] = inbuf;
		i = i + 1;
	}
	return strVec[line - 1];
}

/*修改指定行*/
void ModifyLineData(char* fileName, int lineNum, string lineData)
{
	ifstream in;
	in.open(fileName);
	string strFileData = "";
	int line = 1;
	char tmpLineData[1024] = { 0 };
	while (in.getline(tmpLineData, sizeof(tmpLineData)))
	{
		if (line == lineNum)
		{
			strFileData += lineData;
			strFileData += "\n";
		}
		else
		{
			strFileData += tmpLineData;
			strFileData += "\n";
		}
		line++;
	}
	in.close();
	//写入文件
	ofstream out;
	out.open(fileName);
	out.flush();
	out << strFileData;
	out.close();
}


void CbuilderDlg::OnBnClickedGenerate() {

	char serveraddr[256];
	char serverport[256];
	char serverforwardport[256];

	UpdateData(TRUE);
	
	/*Convert CString to Char**/
	USES_CONVERSION;
	char* CharServerAddr = T2A(ServerAddr);
	char* CharServerPort = T2A(ServerPort);
	char* CharForwardPort = T2A(ForwardPort);

	/*format template*/
	snprintf(serveraddr, 128, SERVER_ADDR, CharServerAddr);
	snprintf(serverport, 128, SERVER_PORT, CharServerPort);
	snprintf(serverforwardport, 128, SERVER_FORWARD_PORT, CharForwardPort);
	
	/*Modify three lines*/
	ModifyLineData(FRPCONFFILEPATH, 83, serveraddr);
	ModifyLineData(FRPCONFFILEPATH, 84, serverport);
	ModifyLineData(FRPCONFFILEPATH, 90, serverforwardport);

	/*Set environment variable*/
	if (iswindows)
	{
		const char* goos = "GOOS=windows";
		_putenv(goos);

		if (isx64)
		{
			const char* goarch = "GOARCH=amd64";
			_putenv(goarch);

			CString outfilename = _T("Frpc_amd64.exe");
			CString gobuildcmd = _T("go build -trimpath -ldflags=\"-w -s\" -o ");
			gobuildcmd = gobuildcmd + outfilename;

			CStringA strA(gobuildcmd);
			LPCSTR ptr = strA;
			WinExec(ptr, SW_HIDE);
			if (isupx)
			{	
				Sleep(5000);
				WinExec("upx.exe Frpc_amd64.exe", SW_HIDE);
			}
		}
		else if (isx86)
		{
			const char* goarch = "GOARCH=386";
			_putenv(goarch);

			CString outfilename = _T("Frpc_386.exe");
			CString gobuildcmd = _T("go build -trimpath -ldflags=\"-w -s\" -o ");
			gobuildcmd = gobuildcmd + outfilename;

			CStringA strA(gobuildcmd);
			LPCSTR ptr = strA;
			WinExec(ptr, SW_HIDE);
			if (isupx)
			{
				Sleep(5000);
				WinExec("upx.exe Frpc_386.exe", SW_HIDE);
			}
		}
		else
		{
			MessageBoxA(NULL, "请选择其中一个!", "ERR", MB_OK);
			
		}
	}

	else if (islinux)
	{
		const char* goos = "GOOS=linux";
		_putenv(goos);

		if (isx64)
		{
			const char* goarch = "GOARCH=amd64";
			_putenv(goarch);

			CString outfilename = _T("Frpc_amd64");
			CString gobuildcmd = _T("go build -trimpath -ldflags=\"-w -s\" -o ");
			gobuildcmd = gobuildcmd + outfilename;

			CStringA strA(gobuildcmd);
			LPCSTR ptr = strA;
			WinExec(ptr, SW_HIDE);
			if (isupx)
			{
				Sleep(5000);
				WinExec("upx.exe Frpc_amd64", SW_HIDE);
			}
		}
		else if (isx86)
		{
			const char* goarch = "GOARCH=386";
			_putenv(goarch);

			CString outfilename = _T("Frpc_386");
			CString gobuildcmd = _T("go build -trimpath -ldflags=\"-w -s\" -o ");
			gobuildcmd = gobuildcmd + outfilename;

			CStringA strA(gobuildcmd);
			LPCSTR ptr = strA;
			WinExec(ptr, SW_HIDE);
			if (isupx)
			{
				Sleep(5000);
				WinExec("upx.exe Frpc_386", SW_HIDE);
			}
		}
		else
		{
			MessageBoxA(NULL, "请选择其中一个!", "ERR", MB_OK);
			
		}
	}
	else
	{
		MessageBoxA(NULL, "请选择其中一个!", "ERR", MB_OK);
	}

	//Sleep(3000);
	MessageBoxA(NULL, "Please Wait a few seconds!", "", MB_OK);
	UpdateData(false);
}

BOOL CbuilderDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CbuilderDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CbuilderDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CbuilderDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

