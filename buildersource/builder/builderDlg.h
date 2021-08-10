
// builderDlg.h: 头文件
//

#pragma once


// CbuilderDlg 对话框
class CbuilderDlg : public CDialogEx
{
// 构造
public:
	CbuilderDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BUILDER_DIALOG };
#endif

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
	DECLARE_MESSAGE_MAP()

public:
	CString ServerAddr;
	CString ServerPort;
	CString ForwardPort;

	afx_msg void OnBnClickedGenerate();

	BOOL isx64;
	BOOL isx86;
	BOOL islinux;
	BOOL iswindows;
	BOOL isupx;
};
