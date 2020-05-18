#include "chgstg.h"

char optionsText[NumberOfOptions][NumberOfLanguages][256] = {
												{"Установить видеорежим",										"Set video mode to"},
												{"Сделать экранный шрифт крупным (120 точек/дюйм)",				"120 dpi"},
												{"Отключить заставку и энергосберегающий режим монитора",		"Switch Off Screen Saver And Energy Saving Mode"},
												{"Установить на Рабочий стол обои Astro7",						"Set Astro7 BackGround"},
												{"Режим быстрой прорисовки",									"Fast drawing"},
												{"Отключение автозапуска flash-накопителей",					"Switch Off Autorun"},
												{"Сервисный процесс",											"Service Proccess"},
												{"Изменить фон загрузки/выключения Windows",					"Switch Windows logon(off) background"},
												{"Отключить контроль учетной записи",							"Disable UAC"},
												{"Отключить обновление Windows",								"Disable Windows update"},
												{"Отключить защитник Windows",									"Disable Windows defender"},
												{"Отключить значок \"Центр поддержки\" из области уведомлений", "Disable \"Support center\" notifications"},
												{"Включить \"Гостя\" ",											"Enable \"Guest\" "},
												{"Задать настройки сети",										"Set network settings"},
												{"Расшарить папки",												"Share Folders"},
												{"Заменить шелл",												"Custom shell" }
};

char	optionsCommandPromt[NumberOfOptions] = { 'v', 'f', 'e', 'w', 'q', 'a', 'p', 'b', 'u', 'i', 'd', 'm', 'g', 'n', 's', 'c' };
bool	optionsAddData[NumberOfOptions] = {
													false,
													false,
													false,
													false,
													false,
													false,
													false,
													false,
													false,
													false,
													false,
													false,
													false,
													false,
													true,
													false
};

bool	options[NumberOfOptions] = { false, false, false, false, false, false, false, false, false, false, false };

#define	numbUSERS	2
#define	astro7		TEXT("astro7.bmp")//Windows Xp хочет .bmp
#define divSign		TEXT('|')

TCHAR	path[MAX_PATH] = TEXT("");
char	Interface[NumberOfLanguages][10] = { "Интерфейс",									"Interface" },
Connection[NumberOfLanguages][12] = { "Подключение",								"Connection" },
ConnectionsNotDetected[NumberOfLanguages][48] = { "Подключения по локальной сети не найдены",	"Local Connection Not Found" };
int		language = 0;//Выбираем язык для диалогов

char	NetworkConnectionName[MAX_PATH][256],
NetworkAdapterName[MAX_PATH][256];
short	NetworkConnectionIndex[256],
NetworkConnectionQuantity = 0;

#define		GraphicListSize			256
DEVMODE		hDevModlist[GraphicListSize];
DWORD		lDevModlist = 0;

LRESULT CALLBACK DialogSetSN(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL	Initial(HWND hdlg, HWND hwndFocus, LPARAM lParam);
BOOL	Menu(HWND hdlg, int id, HWND hwndCtl, UINT codeNotify);

enum {
	NORMAL = 0,
	TO_CMD = 1,
	FROM_CMD = 2
};
int		GlobalFunction(int fIndex, HWND hdlg, BYTE flag = NORMAL, TCHAR* strBuffer = NULL);

void	Registry_SetDword(HKEY Branch, char* KeyPath, char* Key, DWORD value);
void	Registry_GetString(HKEY Branch, char* KeyPath, char* Key, char resultString[MAX_PATH]);
void	Registry_SetString(HKEY Branch, char* KeyPath, char* Key, char valueString[MAX_PATH]);

int		APIENTRY _tWinMain(HINSTANCE	hInstance, HINSTANCE hPrevInstance, LPTSTR cmdLine, int nCmdShow)
{
	MSG		msg;
	HWND	hWnd = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, (DLGPROC)DialogSetSN);

	char lpszCmdLine[] = "-e|-w|-u";

	if (lpszCmdLine[0] == '-')
	{
		int clPos = 1;
		for (int i = 0; i < NumberOfOptions; i++)
		{
			if (lpszCmdLine[clPos] == optionsCommandPromt[i])
			{
				options[i] = true;
				SendMessage(GetDlgItem(hWnd, IDC_baseOpt + i), BM_SETCHECK, BST_CHECKED, 0L);

				clPos += 2;

				clPos += GlobalFunction(i, hWnd, FROM_CMD, &lpszCmdLine[clPos]);
				if (lpszCmdLine[clPos] = '-')
				{
					clPos++;
				}
			}
		}

		_sntprintf_s(path, MAX_PATH, _TRUNCATE, TEXT("%s"), cmdLine);
		SendMessage(hWnd, WM_COMMAND, IDOK, 0);
	}
	else
	{
		_sntprintf_s(path, MAX_PATH, _TRUNCATE, TEXT("%s"), lpszCmdLine);
		ShowWindow(hWnd, SW_SHOWNORMAL);
	}

	BOOL bRet;

	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
	{
		if (bRet == -1)
		{
			// Handle the error and possibly exit
			break;
		}
		else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return msg.wParam;
}

LRESULT CALLBACK DialogSetSN(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		HANDLE_MSG(hdlg, WM_INITDIALOG, Initial);
		HANDLE_MSG(hdlg, WM_COMMAND, Menu);
	case WM_CLOSE:
	{
		DestroyWindow(hdlg);
	}
	break;
	case WM_DESTROY:
	{
		PostQuitMessage(0);
	}
	break;
	default:
		break;
	}
	return FALSE;
}

BOOL Initial(HWND hdlg, HWND hwndFocus, LPARAM lParam)
{
	LANGID langID = GetSystemDefaultUILanguage();
	if ((LOBYTE(langID) != LANG_RUSSIAN) && (HIBYTE(langID) != SUBLANG_RUSSIAN_RUSSIA))
	{
		language++;
	}

	for (DWORD iModeNum = 0; iModeNum < GraphicListSize; iModeNum++)
	{
		if (EnumDisplaySettings(NULL, iModeNum, &hDevModlist[lDevModlist]))
		{
			lDevModlist++;
			for (int i = 0; i < lDevModlist - 1; i++)
			{
				if ((hDevModlist[i].dmPelsWidth == hDevModlist[lDevModlist - 1].dmPelsWidth) &&
					(hDevModlist[i].dmPelsHeight == hDevModlist[lDevModlist - 1].dmPelsHeight) &&
					(hDevModlist[i].dmBitsPerPel == hDevModlist[lDevModlist - 1].dmBitsPerPel))
				{
					lDevModlist--;
					break;
				}
				else
				{
					if (((hDevModlist[i].dmPelsWidth > hDevModlist[lDevModlist - 1].dmPelsWidth) &&
						(hDevModlist[i].dmPelsHeight >= hDevModlist[lDevModlist - 1].dmPelsHeight)) ||
						((hDevModlist[i].dmPelsWidth == hDevModlist[lDevModlist - 1].dmPelsWidth) &&
							(hDevModlist[i].dmPelsHeight == hDevModlist[lDevModlist - 1].dmPelsHeight) &&
							(hDevModlist[i].dmBitsPerPel > hDevModlist[lDevModlist - 1].dmBitsPerPel)))
					{
						DEVMODE tempDevMod = hDevModlist[i];
						hDevModlist[i] = hDevModlist[lDevModlist - 1];
						hDevModlist[lDevModlist - 1] = tempDevMod;
					}
				}
			}
		}
		else
		{
			break;
		}
	}
	for (int i = lDevModlist - 1; i >= 0; i--)
	{
		TCHAR textDevMode[1024] = TEXT("");
		sprintf_s(textDevMode, sizeof(textDevMode) - 1, TEXT("%dx%d %dbit"), hDevModlist[i].dmPelsWidth, hDevModlist[i].dmPelsHeight, hDevModlist[i].dmBitsPerPel);
		SendMessage(GetDlgItem(hdlg, IDC_chresDevModeEDIT), LB_ADDSTRING, 0, (LPARAM)textDevMode);
	}

	for (int i = 0; i < NumberOfOptions; i++)//Text Initialization
	{
		SendMessage(GetDlgItem(hdlg, IDC_chres + i), WM_SETTEXT, NULL, (LPARAM)optionsText[i][language]);
	}
	SendMessage(GetDlgItem(hdlg, IDC_chresDevModeEDIT), LB_SETCURSEL, NULL, 0);
	EnableWindow(GetDlgItem(hdlg, IDC_chresDevModeEDIT), FALSE);
	for (int i = 0; i < 6; i++)//Deactivate controls
	{
		EnableWindow(GetDlgItem(hdlg, IDC_netBase + i), FALSE);
	}
	for (int i = 0; i < 2; i++)//Deactivate controls
	{
		EnableWindow(GetDlgItem(hdlg, IDC_shareFolderBase + i), FALSE);
	}

	Button_SetElevationRequiredState(GetDlgItem(hdlg, IDOK), !IsProcessElevated());

	return TRUE;
}

bool Evaluate(HWND hWnd)
{
	bool evaluate = true;

	BOOL fIsRunAsAdmin;

	try
	{
		fIsRunAsAdmin = IsRunAsAdmin();
	}
	catch (DWORD dwError)
	{
		ReportError(TEXT("IsRunAsAdmin"), dwError);
	}

	// Elevate the process if it is not run as administrator.
	if (!fIsRunAsAdmin)
	{
		TCHAR szPath[MAX_PATH];
		if (GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath)))
		{
			int nFoldersToShare = SendMessage(GetDlgItem(hWnd, IDC_shareFolderEDIT), CB_GETCOUNT, NULL, NULL);

			// Launch itself as administrator.
			SHELLEXECUTEINFO sei = { sizeof(sei) };
			sei.lpVerb = TEXT("runas");
			sei.lpFile = szPath;
			sei.hwnd = hWnd;
			sei.nShow = SW_NORMAL;
			sei.fMask = SEE_MASK_CLASSNAME;
			sei.lpParameters = new(TCHAR[(nFoldersToShare + 1) * MAX_PATH + NumberOfOptions * 3]);
			sei.lpClass = _T("exefile");

			int clPos = 0;
			for (int i = 0; i < NumberOfOptions; i++)
			{
				if (options[i])
				{
					clPos += _stprintf_s(&(TCHAR)sei.lpParameters[clPos], MAX_PATH, TEXT("-%c "), optionsCommandPromt[i]);
					clPos += GlobalFunction(i, hWnd, TO_CMD, &(TCHAR)sei.lpParameters[clPos]);
				}
			}

			if (clPos == 0)
			{
				sei.lpParameters = NULL;
			}

			if (ShellExecuteEx(&sei))
			{
				evaluate = false;
			}

			delete[] sei.lpParameters;
		}
	}

	return evaluate;
}

BOOL Menu(HWND hdlg, int id, HWND hwndCtl, UINT codeNotify)
{
	for (int i = 0; i < NumberOfOptions; i++)
	{
		if (id == IDC_baseOpt + i)
		{
			options[i] = !options[i];
			switch (id)
			{
			case IDC_chres:
			{
				if (options[i])
				{
					EnableWindow(GetDlgItem(hdlg, IDC_chresDevModeEDIT), TRUE);
				}
				else
				{
					EnableWindow(GetDlgItem(hdlg, IDC_chresDevModeEDIT), FALSE);
				}
			}
			break;
			case IDC_net:
			{
				if (options[i])
				{
					for (int j = 0; j < 6; j++)
					{
						EnableWindow(GetDlgItem(hdlg, IDC_netBase + j), TRUE);
					}
				}
				else
				{
					for (int j = 0; j < 6; j++)
					{
						EnableWindow(GetDlgItem(hdlg, IDC_netBase + j), FALSE);
					}
				}
			}
			break;
			case IDC_shareFolder:
			{
				if (options[i])
				{
					for (int j = 0; j < 2; j++)
					{
						EnableWindow(GetDlgItem(hdlg, IDC_shareFolderBase + j), TRUE);
					}
				}
				else
				{
					for (int j = 0; j < 2; j++)
					{
						EnableWindow(GetDlgItem(hdlg, IDC_shareFolderBase + j), FALSE);
					}
				}
			}
			}
			break;
		}
	}

	switch (id)
	{
	case IDADD_shareFolder:
	{
		TCHAR	shareFolderPath[MAX_PATH] = TEXT(""),
			emptyText[MAX_PATH] = TEXT("");
		GetWindowText(GetDlgItem(hdlg, IDC_shareFolderEDIT), shareFolderPath, sizeof(shareFolderPath));
		memset(emptyText, 0, MAX_PATH - 1);

		if (_tcscmp(shareFolderPath, emptyText))
		{
			SendMessage(GetDlgItem(hdlg, IDC_shareFolderEDIT), CB_ADDSTRING, 0, (LPARAM)shareFolderPath);
		}
	}
	break;
	case IDOK:
	{
		for (int i = 0; i < NumberOfOptions; i++)
		{
			if (options[i])
			{
				if (Evaluate(hdlg))
				{
					GlobalFunction(i, hdlg);
				}
				else
				{
					break;//Уходим на повышение прав
				}
			}
		}

		DestroyWindow(hdlg);
		hdlg = NULL;
	}
	break;
	}

	return TRUE;
}

BOOL CreateDirectoryWithUserFullControlACL(LPCTSTR lpPath, PSECURITY_DESCRIPTOR	pSD)
{
	SECURITY_ATTRIBUTES		sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = pSD;
	sa.bInheritHandle = FALSE;

	if (CreateDirectory(lpPath, &sa))
	{
		return TRUE;
	}
	else
	{
		HANDLE hDir = CreateFile(lpPath, READ_CONTROL | WRITE_DAC, 0, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
		if (hDir == INVALID_HANDLE_VALUE)
		{
			return FALSE;
		}
		else
		{
			PSID pEveryoneSID = NULL;
			PACL pACL = NULL;
			EXPLICIT_ACCESS ea[1];
			SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;

			// Create a well-known SID for the Everyone group.
			AllocateAndInitializeSid(&SIDAuthWorld, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &pEveryoneSID);

			// Initialize an EXPLICIT_ACCESS structure for an ACE.
			// The ACE will allow Everyone read access to the key.
			ZeroMemory(&ea, 1 * sizeof(EXPLICIT_ACCESS));
			ea[0].grfAccessPermissions = GENERIC_ALL;
			ea[0].grfAccessMode = SET_ACCESS;
			ea[0].grfInheritance = CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE;
			ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
			ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
			ea[0].Trustee.ptstrName = (LPTSTR)pEveryoneSID;

			// Create a new ACL that contains the new ACEs.
			SetEntriesInAcl(1, ea, NULL, &pACL);

			// Initialize a security descriptor.
			PSECURITY_DESCRIPTOR pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);

			InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION);

			// Add the ACL to the security descriptor.
			SetSecurityDescriptorDacl(pSD, TRUE, pACL, FALSE);

			//Change the security attributes
			SetFileSecurity(lpPath, DACL_SECURITY_INFORMATION, pSD);

			if (pEveryoneSID)
				FreeSid(pEveryoneSID);
			if (pACL)
				LocalFree(pACL);
			if (pSD)
				LocalFree(pSD);

			return TRUE;
		}
	}
}

///////////////////////
//DEVMODE.dmFields = DM_POSITION | DM_PELSWIDTH | DM_PELSHEIGHT;
//DEVMODE.dmPelsWidth = DEVMODE.dmPelsHeight = 0

void DetachDisplay()
{
	BOOL            FoundSecondaryDisp = FALSE;
	DWORD           DispNum = 0;
	DISPLAY_DEVICE  DisplayDevice;
	LONG            Result;
	TCHAR           szTemp[200];
	int             i = 0;
	DEVMODE   defaultMode;

	// initialize DisplayDevice
	ZeroMemory(&DisplayDevice, sizeof(DisplayDevice));
	DisplayDevice.cb = sizeof(DisplayDevice);

	// get all display devices
	while (EnumDisplayDevices(NULL, DispNum, &DisplayDevice, 0))
	{
		ZeroMemory(&defaultMode, sizeof(DEVMODE));
		defaultMode.dmSize = sizeof(DEVMODE);
		if (!EnumDisplaySettings((LPSTR)DisplayDevice.DeviceName, ENUM_REGISTRY_SETTINGS, &defaultMode))
			OutputDebugString("Store default failed\n");

		if ((DisplayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) && !(DisplayDevice.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE))
		{
			DEVMODE    DevMode;
			ZeroMemory(&DevMode, sizeof(DevMode));
			DevMode.dmSize = sizeof(DevMode);
			DevMode.dmFields = DM_POSITION | DM_PELSWIDTH | DM_PELSHEIGHT;
			DevMode.dmPelsWidth = DevMode.dmPelsHeight = 0;

			Result = ChangeDisplaySettingsEx((LPSTR)DisplayDevice.DeviceName, &DevMode, NULL, CDS_UPDATEREGISTRY, NULL);
		}

		// Reinit DisplayDevice just to be extra clean

		ZeroMemory(&DisplayDevice, sizeof(DisplayDevice));
		DisplayDevice.cb = sizeof(DisplayDevice);
		DispNum++;
	} // end while for all display devices
}

int GlobalFunction(int fIndex, HWND hdlg, BYTE flag, TCHAR* strBuffer)
{
	int retVal = 0;

	switch (fIndex)
	{

	case 0://void Resolution(int hWidth, int hHeight, int hBitsPerPixel)
	{
		switch (flag)
		{
		case TO_CMD:
		{
			retVal += _stprintf_s(&strBuffer[retVal], MAX_PATH, TEXT("%d%c"), SendMessage(GetDlgItem(hdlg, IDC_chresDevModeEDIT), LB_GETCURSEL, NULL, NULL), divSign);
		}
		break;
		case FROM_CMD:
		{
			SendMessage(GetDlgItem(hdlg, IDC_chresDevModeEDIT), LB_SETCURSEL, atoi(strBuffer), 0);

			while ((strBuffer[retVal] != divSign))
			{
				retVal++;
			}
		}
		break;
		case NORMAL:
		{
			//DetachDisplay();
			int nDevMode = SendMessage(GetDlgItem(hdlg, IDC_chresDevModeEDIT), LB_GETCURSEL, NULL, NULL);

			if (nDevMode != -1)
			{
				DEVMODE dm;
				ZeroMemory(&dm, sizeof(DEVMODE));
				dm.dmSize = sizeof(DEVMODE);
				dm.dmBitsPerPel = hDevModlist[lDevModlist - nDevMode - 1].dmBitsPerPel;	//Глубина цвета
				dm.dmPelsWidth = hDevModlist[lDevModlist - nDevMode - 1].dmPelsWidth;	//Ширина монитора (в пикселях)
				dm.dmPelsHeight = hDevModlist[lDevModlist - nDevMode - 1].dmPelsHeight;	//Высота монитора (в пикселях)
				dm.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;

				LONG result = ChangeDisplaySettings(&dm, CDS_UPDATEREGISTRY);
				if (result != DISP_CHANGE_SUCCESSFUL)										//Сам процесс изменения конфигурации графики
				{
					TCHAR	errorText[256] = TEXT("");
					_stprintf_s(errorText, sizeof(errorText) - 1, TEXT("ChangeDisplaySettings Error %d\n"), result);
					MessageBox(NULL, errorText, TEXT("Error"), NULL);
				}
			}
			else
			{
				MessageBox(NULL, TEXT("VideoMode do not selected!"), TEXT("Error"), NULL);
			}
		}
		break;
		}
	}
	break;
	case 1://void ScreenFont()
	{
		switch (flag)
		{
		case TO_CMD:
			break;
		case FROM_CMD:
			break;
		case NORMAL:
		{
			HDC DC = GetDC(0);
			HMODULE hModule = LoadLibrary("syssetup.dll");

			if (GetDeviceCaps(DC, LOGPIXELSX) != 120)
			{
				ReleaseDC(0, DC);

				typedef DWORD(WINAPI* MYPROC)(HANDLE, LPWSTR);
				MYPROC ProcAdd = (MYPROC)GetProcAddress(hModule, "SetupChangeFontSize");
				DWORD dwRetVal = (ProcAdd)(NULL, L"120");

				FreeLibrary(hModule);
			}
		}
		break;
		}
	}
	break;
	case 2://void Power()
	{
		switch (flag)
		{
		case TO_CMD:
			break;
		case FROM_CMD:
			break;
		case NORMAL:
		{
			HANDLE hLog = CreateFile("log_ChngStg.txt", GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_ALWAYS, 0, 0);

			OSVERSIONINFO osver = { sizeof(osver) };
			if (GetVersionEx(&osver) && osver.dwMajorVersion >= 6)
			{
				DWORD dRes = PowerSettingAccessCheck(ACCESS_ACTIVE_SCHEME, NULL);// GUID *PowerGuid);
				std::string str("PowerSettingAccessCheck res = ");
				char buffer[65];
				str += _itot(dRes, buffer, 10); //ERROR_ACCESS_DISABLED_BY_POLICY
				str += _T("\r\n");

				dRes = PowerSetActiveScheme(0, &GUID_MIN_POWER_SAVINGS);//GUID_MIN_POWER_SAVINGS, which corresponds to the Maximum Performance power scheme and its corresponding personality
				str += "PowerSetActiveScheme res = ";
				str += _itot(dRes, buffer, 10);
				str += _T("\r\n");
				WriteFile(hLog, &str[0], str.size(), 0, 0);

			}
			else
			{
				unsigned int ps;
				POWER_POLICY pp;

				SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, 0, NULL, 0);

				GetActivePwrScheme(&ps);
				ReadPwrScheme(ps, &pp);

				pp.user.VideoTimeoutAc = 0;
				pp.user.VideoTimeoutDc = 0;
				pp.user.SpindownTimeoutAc = 0;
				pp.user.SpindownTimeoutDc = 0;
				pp.user.IdleTimeoutAc = 0;
				pp.user.IdleTimeoutDc = 0;

				SetActivePwrScheme(ps, 0, &pp);
			}
			CloseHandle(hLog);
		}
		break;
		}
	}
	break;
	case 3://void Wall()
	{
		switch (flag)
		{
		case TO_CMD:
			break;
		case FROM_CMD:
			break;
		case NORMAL:
		{
			TCHAR	szPath[MAX_PATH];
			if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_MYPICTURES | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, szPath)))
			{
				PathAppend(szPath, astro7);//WindowsXP хочет bmp
			}
			CopyFile(astro7, szPath, FALSE);
			Registry_SetString(HKEY_CURRENT_USER, "Control Panel\\Desktop", "WallpaperStyle", "2");
			Registry_SetString(HKEY_CURRENT_USER, "Control Panel\\Desktop", "Wallpaper", szPath);
			SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, szPath, SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE);
		}
		break;
		}
	}
	break;	
	case 7://void BackGround()
	{
		switch (flag)
		{
		case TO_CMD:
			break;
		case FROM_CMD:
			break;
		case NORMAL:
		{
			Registry_SetDword(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Authentication\\LogonUI\\Background", "OEMBackground", 1);

			TCHAR	szPath[MAX_PATH];
			if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_WINDOWS | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, szPath)))
			{
				PathAppend(szPath, TEXT("\\System32\\oobe\\info"));
				CreateDirectory(szPath, NULL);
				PathAppend(szPath, TEXT("\\backgrounds"));
				CreateDirectory(szPath, NULL);
				PathAppend(szPath, TEXT("\\backgroundDefault.jpg"));
			}
			CopyFile("astroWload.jpg", szPath, FALSE);

			SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, szPath, SPIF_UPDATEINIFILE);
		}
		break;
		}
	}
	break;
	case 8://void UACdisable()
	{
		switch (flag)
		{
		case TO_CMD:
			break;
		case FROM_CMD:
			break;
		case NORMAL:
		{
			Registry_SetDword(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", "EnableLUA", 0);
			break;
			//We will try not to use GUI 

			TCHAR	szPath[MAX_PATH];

			if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_WINDOWS | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, szPath)))
			{
				PathAppend(szPath, TEXT("\\System32\\UserAccountControlSettings.exe"));

				HANDLE hUAC = CreateFile(szPath, GENERIC_EXECUTE, NULL, 0, OPEN_EXISTING, 0, 0);

				if (hUAC != INVALID_HANDLE_VALUE)
				{
					ShellExecute(NULL, "open", "UserAccountControlSettings.exe", NULL, NULL, SW_SHOWNORMAL);
				}
				else
				{
					Registry_SetDword(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", "EnableLUA", 0);
				}

				CloseHandle(hUAC);
			}
			break;
		}
		}
	}
	break;	
	break;
	default:
		break;
	}

	return retVal;
}

void Registry_SetDword(HKEY Branch, char* KeyPath, char* Key, DWORD value)
{
	HKEY	hkey;
	DWORD	error,
		dwDisposition;
	TCHAR	classname[] = TEXT(" ");

	error = RegOpenKeyEx(Branch, KeyPath, NULL, KEY_SET_VALUE, &hkey);
	if (error != ERROR_SUCCESS)
	{
		error = RegCreateKeyEx(Branch, KeyPath, 0, classname, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &dwDisposition);
	}
	RegSetValueEx(hkey, Key, NULL, REG_DWORD, (unsigned char*)&value, 4);
	RegCloseKey(hkey);
}

void Registry_SetString(HKEY Branch, char* KeyPath, char* Key, char valueString[MAX_PATH])
{
	HKEY	hkey;
	DWORD	error,
		dwDisposition;
	TCHAR	classname[] = TEXT(" ");

	error = RegOpenKeyEx(Branch, KeyPath, NULL, KEY_SET_VALUE, &hkey);
	if (error != ERROR_SUCCESS)
	{
		RegCreateKeyEx(Branch, KeyPath, 0, classname, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &dwDisposition);
	}
	int valueStringL = strlen(valueString);
	RegSetValueEx(hkey, Key, NULL, REG_SZ, (unsigned char*)valueString, valueStringL);
	RegCloseKey(hkey);
}

void Registry_GetString(HKEY Branch, char* KeyPath, char* Key, char resultString[MAX_PATH])
{
	HKEY	hkey;
	DWORD	error,
		sizeValue = MAX_PATH;
	TCHAR	classname[] = TEXT(" ");

	error = RegOpenKeyEx(Branch, KeyPath, NULL, KEY_READ, &hkey);
	if (error == ERROR_SUCCESS)
	{
		error = RegQueryValueEx(hkey, Key, NULL, NULL, (LPBYTE)resultString, &sizeValue);
	}
	RegCloseKey(hkey);
}