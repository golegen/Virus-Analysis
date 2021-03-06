
// AntivirusToolDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "AntivirusTool.h"
#include "AntivirusToolDlg.h"
#include "afxdialogex.h"
#include <TlHelp32.h>
#include <windows.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


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


// CAntivirusToolDlg 对话框



CAntivirusToolDlg::CAntivirusToolDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ANTIVIRUSTOOL_DIALOG, pParent)
	, m_Edit(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAntivirusToolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_Edit);
}

BEGIN_MESSAGE_MAP(CAntivirusToolDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CAntivirusToolDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CAntivirusToolDlg 消息处理程序

BOOL CAntivirusToolDlg::OnInitDialog()
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

void CAntivirusToolDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CAntivirusToolDlg::OnPaint()
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
HCURSOR CAntivirusToolDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


//1.在内存中查找病毒是否还存在
BOOL CAntivirusToolDlg::FindTargetProcess(char* pszProcessName, DWORD *dwPid)
{
	bool bFind = false;//是否查找成功
	//创建进程快照
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap==INVALID_HANDLE_VALUE)
	{
		return bFind;
	}
	PROCESSENTRY32 pe = { 0 };
	pe.dwSize = sizeof(pe);
	//开始查找第一个线程
	BOOL bRet = Process32First(hProcessSnap,&pe);
	while (bRet)
	{
		//如果进程名一致
		if (lstrcmp(pe.szExeFile,(LPCWSTR)pszProcessName)==0)
		{
			//输出PID
			*dwPid = pe.th32ProcessID;
			bFind = true;
			break;
		}
		bRet = Process32Next(hProcessSnap, &pe);
	}
	CloseHandle(hProcessSnap);
	return bFind;
}


//2.提升权限,访问一些受限制的系统资源
bool CAntivirusToolDlg::EnableDebugPrivilege(char * pszPrivilege)
{
	HANDLE hToken = INVALID_HANDLE_VALUE;
	LUID luid;
	TOKEN_PRIVILEGES tp;

	BOOL bRet = OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);
	if (bRet == FALSE)
	{
		return bRet;
	}

	bRet = LookupPrivilegeValue(NULL, (LPCWSTR)pszPrivilege, &luid);//这里需要进行类型转换,将pszPrivilege转换成LPCWSTR类型
	if (bRet == FALSE)
	{
		return bRet;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	bRet = AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);

	return bRet;
}

//计算CRC32值
DWORD CAntivirusToolDlg::CRC32(BYTE* ptr, DWORD Size)
{
	DWORD crcTable[256], crcTmp1;
	//动态生成SRC-32表
	for (int i = 0; i < 256; i++)
	{
		crcTmp1 = i;
		for (int j = 8; j >0; j--)
		{
			if (crcTmp1&1)
			{
				crcTmp1 = (crcTmp1 >> 1) ^ 0xEDB88320L;
			}
			else
			{
				crcTmp1 >>= 1;
			}
		}
		//计算CRC32值
		DWORD crcTmp2 = 0xFFFFFFFF;
		while (Size--)
		{
			crcTmp2 = ((crcTmp2 >> 8) & 0x00FFFFFF) ^ crcTable[(crcTmp2 ^ (*ptr)) & 0xFF];
			ptr++;
		}
		return (crcTmp2 ^ 0xFFFFFFFF);
	}
}


//一键杀毒按钮
void CAntivirusToolDlg::OnBnClickedButton1()
{
	DWORD dwPid = 0;
	bool bRet = false;
	CString csTxt;
	//1.在内存中查找病毒是否存在 
	bRet = FindTargetProcess("spo0lsv.exe", &dwPid);
	if (bRet==true)
	{
		CString csTxt = _T("检查系统内存...\r\n");
		csTxt += _T("系统中存在病毒:spo0lsv.exe\r\n");
		csTxt += _T("准备进程查杀...\r\n");

		SetDlgItemText(IDC_LIST1, csTxt);
		//提升权限
		bRet = EnableDebugPrivilege((char*)SE_DEBUG_NAME);

		if (bRet==false)
		{
			csTxt += _T("提升权限失败\r\n");
		}
		else
		{
			csTxt += _T("提升权限成功\r\n");
		}
		SetDlgItemText(IDC_LIST1, csTxt);

		//获得进程句柄
		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);
		if (hProcess==INVALID_HANDLE_VALUE)
		{
			csTxt += _T("无法结束病毒进程\r\n");
			return;
		}
		//结束病毒进程
		bRet = TerminateProcess(hProcess, 0);
		if (bRet==FALSE)
		{
			csTxt += _T("无法结束病毒进程\r\n");
			return;
		}
		csTxt += _T("病毒进程已经结束\r\n");
		SetDlgItemText(IDC_LIST1, csTxt);
	}
	else
	{
		csTxt += _T("系统中不存在病毒:spo0lsv.exe\r\n");
	}
	//清理系统目录下的病毒文件
	Sleep(10);
	//查杀磁盘中是否存在病毒:spo0lsv.exe
	char szSysPath[MAX_PATH] = { 0 };
	//获取系统目录
	GetSystemDirectory((LPWSTR)szSysPath,MAX_PATH);
	//拼接出病毒路径
	lstrcat((LPWSTR)szSysPath, (LPCWSTR)"\\drivers\\spo0lsv.exe");
	csTxt += _T("正在扫描磁盘....\r\n");
	//查找文件是否存在
	if (GetFileAttributes((LPCWSTR)szSysPath) == INVALID_FILE_ATTRIBUTES)
	{
		csTxt += _T("病毒文件不存在\r\n");
	}
	else
	{
		//如果对应路径下存在目标文件 则应计算散列值再次确认 防止误杀
		csTxt += _T("正在计算散列值...\r\n");
		//获取文件句柄
		HANDLE hFile = CreateFile((LPCWSTR)szSysPath,GENERIC_READ,
		FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
		if (hFile==INVALID_HANDLE_VALUE)
		{
			AfxMessageBox(_T("创建文件失败！"));
			return;
		}
		//获得文件大小
		DWORD dwSize = GetFileSize(hFile, NULL);
		if (dwSize==INVALID_FILE_SIZE)
		{
			AfxMessageBox(_T("获取文件大小失败！"));
			return;
		}
		//根据文件大小申请空间
		BYTE* pFile = (BYTE*)malloc(dwSize);
		if (pFile==NULL)
		{
			AfxMessageBox(_T("申请空间失败失败！"));
			return;
		}
		DWORD dwNum = 0;
		//把文件读取到内存
		ReadFile(hFile, pFile, dwSize, &dwNum, NULL);
		//计算spo0lsv的散列值
		DWORD dwCrc32 = CRC32(pFile, dwSize);
		if (pFile!=NULL)
		{
			free(pFile);
			pFile = NULL;
		}
		CloseHandle(hFile);
		// 0x89240FCD是“熊猫烧香”病毒的散列值  E334747C
		if (dwCrc32!= 0xE334747C)
		{
			csTxt += L"病毒散列值校验失败！\r\n";
		}
		else
		{
			csTxt += L"病毒散列值校验成功 正在删除...\r\n";
			//去除文件的隐藏 系统和只读属性
			DWORD dwFileAttributes = GetFileAttributes(LPCWSTR(szSysPath));
			//~按位取反
			dwFileAttributes &= ~FILE_ATTRIBUTE_HIDDEN;
			dwFileAttributes &= ~FILE_ATTRIBUTE_SYSTEM;
			dwFileAttributes &= ~FILE_ATTRIBUTE_READONLY;
			//删除病毒文件
			bRet = DeleteFile((LPCWSTR)szSysPath);
			if (bRet)
			{
				csTxt += _T("病毒被删除\r\n");
			}
			else
			{
				csTxt += _T("病毒无法删除\r\n");
			}
		}
	}
	SetDlgItemText(IDC_LIST1, csTxt);
	Sleep(10);
	//删除每个盘符下的setup.exe和autorun.inf以及Desktop_.ini
	char szDriverString[MAXBYTE] = {0};
	char* pTmp = NULL;
	//获取字符串类型的驱动器列表
	GetLogicalDriveStrings(MAXBYTE,(LPWSTR)szDriverString);
	pTmp = szDriverString;
	while (*pTmp)
	{
		char szAutorunPath[MAX_PATH] = {0};
		char szSetupPath[MAX_PATH] = { 0 };
		//拼接路径
		lstrcat((LPWSTR)szAutorunPath,(LPCWSTR)pTmp);
		lstrcat((LPWSTR)szAutorunPath,(LPCWSTR)"autorun.inf");
		lstrcat((LPWSTR)szSetupPath,(LPCWSTR)pTmp);
		lstrcat((LPWSTR)szSetupPath,(LPCWSTR)"setup.exe");

		if (GetFileAttributes((LPCWSTR)szSetupPath)==INVALID_FILE_ATTRIBUTES)
		{
			csTxt += pTmp;
			csTxt += _T("setup.exe病毒文件不存在\r\n");
		}
		else
		{
			csTxt += pTmp;
			csTxt += _T("setup.exe病毒文件存在 正在校验哈希值\r\n");
			HANDLE hFile = CreateFile((LPCWSTR)szSetupPath,GENERIC_READ,
			FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
			if (hFile==INVALID_HANDLE_VALUE)
			{
				AfxMessageBox(_T("创建文件失败！"));
				return;
			}
			DWORD dwSize = GetFileSize(hFile, NULL);
			if (dwSize==INVALID_FILE_SIZE)
			{
				AfxMessageBox(_T("获取文件大小失败！"));
				return;
			}
			BYTE* pFile = (BYTE*)malloc(dwSize);
			if (pFile==NULL)
			{
				AfxMessageBox(_T("申请空间失败"));
				return;
			}
			DWORD dwNum = 0;
			ReadFile(hFile, pFile, dwSize, &dwNum, NULL);
			DWORD dwCrc32 = CRC32(pFile, dwSize);
			if (pFile!=NULL)
			{
				free(pFile);
				pFile = NULL;
			}
			CloseHandle(hFile);
			if (dwCrc32!= 0xE334747C)
			{
				csTxt += _T("哈希值校验失败!\r\n");
			}
			else
			{
				csTxt += _T("哈希值校验成功!正在删除...\r\n");
				//去除文件的隐藏 系统和只读属性
				DWORD dwFileAttributes = GetFileAttributes((LPCWSTR)szSetupPath);
				dwFileAttributes &= ~FILE_ATTRIBUTE_HIDDEN;
				dwFileAttributes &= ~FILE_ATTRIBUTE_SYSTEM;
				dwFileAttributes &= ~FILE_ATTRIBUTE_READONLY;
				SetFileAttributes((LPCWSTR)szSetupPath, dwFileAttributes);
				//删除setup.exe
				bRet = DeleteFile((LPCWSTR)szSetupPath);

				if (bRet)
				{
					csTxt += pTmp;
					csTxt += _T("setup.exe病毒已被删除..\r\n");
				}
				else
				{
					csTxt += pTmp;
					csTxt += _T("setup.exe病毒无法删除..\r\n");
				}
			}
		}
		//去除文件的隐藏 系统和只读属性
		DWORD dwFileAttributes = GetFileAttributes((LPCWSTR)szAutorunPath);
		dwFileAttributes &= ~FILE_ATTRIBUTE_HIDDEN;
		dwFileAttributes &= ~FILE_ATTRIBUTE_SYSTEM;
		dwFileAttributes &= ~FILE_ATTRIBUTE_READONLY;
		SetFileAttributes((LPCWSTR)szAutorunPath,dwFileAttributes);
		//删除autorun.inf
		bRet = DeleteFile((LPCWSTR)szAutorunPath);
		csTxt += pTmp;
		if (bRet)
		{
			csTxt += _T("autorun.inf被删除!\r\n");
		}
		else
		{
			csTxt += _T("autorun.inf无法删除!\r\n");
		}
		//删除Desktop_.ini
		Sleep(10);
		///////////////////////////////////////////////////////////////////
		//  修复注册表内容，删除病毒启动项并修复文件的隐藏显示
		///////////////////////////////////////////////////////////////////
		csTxt += _T("正在检查注册表...\r\n");
		SetDlgItemText(IDC_LIST1, csTxt);
		// 首先检查启动项
		char RegRun[] = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";
		HKEY hKeyHKCU = NULL;
		LONG lSize = MAXBYTE;
		char cData[MAXBYTE] = { 0 };

		long lRet = RegOpenKey(HKEY_CURRENT_USER, (LPCWSTR)RegRun, &hKeyHKCU);
		if (lRet == ERROR_SUCCESS)
		{
			lRet = RegQueryValueEx(hKeyHKCU, (LPCWSTR)"svcshare", NULL, NULL, (unsigned char *)cData, (unsigned long *)&lSize);
			if (lRet == ERROR_SUCCESS)
			{
				if (lstrcmp((LPCWSTR)cData, (LPCWSTR)"C:\\WINDOWS\\system32\\drivers\\spoclsv.exe") == 0)
				{
					csTxt += _T("注册表启动项中存在病毒信息\r\n");
				}

				lRet = RegDeleteValue(hKeyHKCU, (LPCWSTR)"svcshare");
				if (lRet == ERROR_SUCCESS)
				{
					csTxt += _T("注册表启动项中的病毒信息已删除！\r\n");
				}
				else
				{
					csTxt += _T("注册表启动项中的病毒信息无法删除\r\n");
				}
			}
			else
			{
				csTxt += _T("注册表启动项中不存在病毒信息\r\n");
			}
			RegCloseKey(hKeyHKCU);
		}
		else
		{
			csTxt += _T("注册表启动项信息读取失败\r\n");
		}
		// 接下来修复文件的隐藏显示，需要将CheckedValue的值设置为1
		char RegHide[] = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\\Folder\\Hidden\\SHOWALL";
		HKEY hKeyHKLM = NULL;
		DWORD dwFlag = 1;

		long lRetHide = RegOpenKey(HKEY_LOCAL_MACHINE, (LPCWSTR)RegHide, &hKeyHKLM);
		if (lRetHide == ERROR_SUCCESS)
		{
			csTxt += _T("检测注册表的文件隐藏选项...\r\n");
			if (ERROR_SUCCESS == RegSetValueEx(
				hKeyHKLM,             //subkey handle  
				(LPCWSTR)"CheckedValue",       //value name  
				0,                    //must be zero  
				REG_DWORD,            //value type  
				(CONST BYTE*)&dwFlag, //pointer to value data  
				4))                   //length of value data
			{
				csTxt += _T("注册表修复完毕！\r\n");
			}
			else
			{
				csTxt += _T("无法恢复注册表的文件隐藏选项\r\n");
			}
		}
		///////////////////////////////////////////////////////////////////
		// 病毒查杀完成
		///////////////////////////////////////////////////////////////////
		csTxt += _T("病毒查杀完成，请使用专业杀毒软件进行全面扫描！\r\n");
		SetDlgItemText(IDC_LIST1, csTxt);
	
	}
	
}
