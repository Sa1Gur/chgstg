#include "chgstg.h"

char optionsText[NumberOfOptions][NumberOfLanguages][256] =	{
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

char	optionsCommandPromt[NumberOfOptions]	= {'v', 'f', 'e', 'w', 'q', 'a', 'p', 'b', 'u', 'i', 'd', 'm', 'g', 'n', 's', 'c'};
bool	optionsAddData[NumberOfOptions]			= {
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

bool	options[NumberOfOptions] = {false, false, false, false, false, false, false, false, false, false, false};

#define	numbUSERS	2
#define	astro7		TEXT("astro7.bmp")//Windows Xp хочет .bmp
#define divSign		TEXT('|')

TCHAR	path[MAX_PATH]									= TEXT("");
char	Interface[NumberOfLanguages][10]				= {"Интерфейс",									"Interface"},
		Connection[NumberOfLanguages][12]				= {"Подключение",								"Connection"},
		ConnectionsNotDetected[NumberOfLanguages][48]	= {"Подключения по локальной сети не найдены",	"Local Connection Not Found"};
int		language = 0;//Выбираем язык для диалогов

char	NetworkConnectionName[MAX_PATH][256],
		NetworkAdapterName[MAX_PATH][256];
short	NetworkConnectionIndex[256],
		NetworkConnectionQuantity	= 0;

#define		GraphicListSize			256
DEVMODE		hDevModlist[GraphicListSize];
DWORD		lDevModlist				= 0;

LRESULT CALLBACK DialogSetSN(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL	Initial(HWND hdlg, HWND hwndFocus, LPARAM lParam);
void	NetInit(HWND hdlg, HWND hwndFocus, LPARAM lParam);
BOOL	Menu(HWND hdlg, int id, HWND hwndCtl, UINT codeNotify);

enum	{
			NORMAL		= 0,
			TO_CMD		= 1,
			FROM_CMD	= 2
		};
int		GlobalFunction(int fIndex, HWND hdlg, BYTE flag = NORMAL, TCHAR *strBuffer = NULL);

void	Registry_SetDword(HKEY Branch,	char* KeyPath, char* Key, DWORD value);
void	Registry_GetString(HKEY Branch, char* KeyPath, char* Key, char resultString[MAX_PATH]);
void	Registry_SetString(HKEY Branch, char* KeyPath, char* Key, char valueString[MAX_PATH]);

BOOL ListProcessModules(DWORD dwPID, TCHAR ProccessName[MAX_PATH])
{
	HANDLE			hModuleSnap	= INVALID_HANDLE_VALUE;
	HANDLE			expro		= NULL;
	MODULEENTRY32	me32;
  
	hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);
	if (hModuleSnap == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	else
	{
		me32.dwSize = sizeof(MODULEENTRY32);
	
		if (!Module32First(hModuleSnap, &me32))
		{
			CloseHandle( hModuleSnap );
			return FALSE;
		}
		else
		{
			do
			{
				if (StrCmpI(me32.szModule, ProccessName) == 0)
				{
					expro = OpenProcess(1, TRUE, me32.th32ProcessID);
					TerminateProcess(expro, -9);
				}
			}
			while(Module32Next(hModuleSnap, &me32));
	
			CloseHandle(hModuleSnap);
			return(TRUE);
		}
	}
}

BOOL GetProcessList(TCHAR ProccessName[MAX_PATH])
{
	HANDLE			hProcessSnap,
					hProcess;
	PROCESSENTRY32	pe32;
	DWORD			dwPriorityClass;
	
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0 );
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	else
	{
		pe32.dwSize = sizeof(PROCESSENTRY32);
		
		if (!Process32First(hProcessSnap, &pe32))
		{
			CloseHandle(hProcessSnap);
			return FALSE;
		}
		else
		{			
			do
			{
				dwPriorityClass = 0;
				hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
				
				if (hProcess != NULL)
				{
					dwPriorityClass = GetPriorityClass(hProcess);
					
					if (!dwPriorityClass)
					{
						CloseHandle(hProcess);
					}
				}
				
				ListProcessModules(pe32.th32ProcessID, ProccessName);
			}
			while (Process32Next(hProcessSnap, &pe32));
			
			CloseHandle(hProcessSnap);
		}
	}
	return TRUE;
}

int		APIENTRY _tWinMain(HINSTANCE	hInstance, HINSTANCE hPrevInstance, LPTSTR lpszCmdLine, int nCmdShow)
{	
	MSG		msg;
	HWND	hWnd = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, (DLGPROC)DialogSetSN);

	//lpszCmdLine = new char[1024];
	//SetDlgItemText(hWnd, IDC_customShell, "explorer.exe");//C:\\APOLYECG\\adminRunner_.exe
	//sprintf(lpszCmdLine, "-c|explorer.exe|");//C:\\APOLYECG\\adminRunner_.exe
	//MessageBox(0, lpszCmdLine, 0, 0);

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

		SendMessage(hWnd, WM_COMMAND, IDOK, 0);
		//ShowWindow(hWnd, SW_SHOWNORMAL);
	}
	else
	{
		_sntprintf_s(path, MAX_PATH, _TRUNCATE, TEXT("%s"), lpszCmdLine);
		ShowWindow(hWnd, SW_SHOWNORMAL);
		//InitCommonControls();
		//DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, (DLGPROC)DialogSetSN);
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
		HANDLE_MSG(hdlg, WM_INITDIALOG,	Initial);
		HANDLE_MSG(hdlg, WM_COMMAND,	Menu);
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
						DEVMODE tempDevMod				= hDevModlist[i];
						hDevModlist[i]					= hDevModlist[lDevModlist - 1];
						hDevModlist[lDevModlist - 1]	= tempDevMod;
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

	NetInit(hdlg, hwndFocus, lParam);

	for(int i = 0; i < NumberOfOptions; i++)//Text Initialization
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

	OSVERSIONINFO osver = { sizeof(osver) };
    if (!GetVersionEx(&osver))
    {
        DWORD dwError = GetLastError();
    }
	else
	{
		if (osver.dwMajorVersion >= 6)
		{
			Button_SetElevationRequiredState(GetDlgItem(hdlg, IDOK), !IsProcessElevated());
		}
	}

	return TRUE;
}

void NetInit(HWND hdlg, HWND hwndFocus, LPARAM lParam)
{
	{
		char	OutTextInfo[4048];
		int		iOutTextInfo = 0,
				SizeTextInfo = 4048;
		{
			FIXED_INFO *pFixedInfo;
			IP_ADDR_STRING *pIPAddr;

			ULONG ulOutBufLen;
			DWORD dwRetVal;
		
			pFixedInfo = (FIXED_INFO *) malloc(sizeof (FIXED_INFO));
			ulOutBufLen = sizeof (FIXED_INFO);

			if (GetNetworkParams(pFixedInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
			{
				free(pFixedInfo);
				pFixedInfo = (FIXED_INFO *) malloc(ulOutBufLen);
				if (pFixedInfo == NULL)
				{
					iOutTextInfo += sprintf_s(&OutTextInfo[iOutTextInfo], SizeTextInfo - iOutTextInfo, "Error allocating memory needed to call GetNetworkParams\n");
				}
			}

			if (dwRetVal = GetNetworkParams(pFixedInfo, &ulOutBufLen) != NO_ERROR)
			{
				iOutTextInfo += sprintf_s(&OutTextInfo[iOutTextInfo], SizeTextInfo - iOutTextInfo, "GetNetworkParams failed with error %d\n", dwRetVal);
			}
			else
			{		
				SendMessage(GetDlgItem(hdlg, IDC_nameEDIT), WM_SETTEXT, NULL, (LPARAM)pFixedInfo->HostName);
			
				////WORKGROUP || DOMAIN
				NETSETUP_JOIN_STATUS	nJS;
				LPWSTR *WPCworkGroupText;
				NetApiBufferAllocate(MAX_DOMAIN_NAME_LEN, (LPVOID *) &WPCworkGroupText);
				NetGetJoinInformation(NULL, WPCworkGroupText, &nJS);
				if ((nJS == NetSetupWorkgroupName) || (nJS == NetSetupDomainName))
				{
					char PCworkGroupText[MAX_DOMAIN_NAME_LEN] = "";
					sprintf_s(PCworkGroupText, sizeof(PCworkGroupText), "%S", *WPCworkGroupText);
					SendMessage(GetDlgItem(hdlg, IDC_workGroupEDIT), WM_SETTEXT, NULL, (LPARAM)PCworkGroupText);
				}
				NetApiBufferFree(WPCworkGroupText);
				/////////

				//iOutTextInfo += sprintf_s(&OutTextInfo[iOutTextInfo], SizeTextInfo - iOutTextInfo, "\tHost Name: %s\n", pFixedInfo->HostName);
				//iOutTextInfo += sprintf_s(&OutTextInfo[iOutTextInfo], SizeTextInfo - iOutTextInfo, "\tDomain Name: %s\n", pFixedInfo->DomainName);
				//iOutTextInfo += sprintf_s(&OutTextInfo[iOutTextInfo], SizeTextInfo - iOutTextInfo, "\tDNS Servers:\n");
				//iOutTextInfo += sprintf_s(&OutTextInfo[iOutTextInfo], SizeTextInfo - iOutTextInfo, "\t\t%s\n", pFixedInfo->DnsServerList.IpAddress.String);

				pIPAddr = pFixedInfo->DnsServerList.Next;
				while (pIPAddr)
				{
					//iOutTextInfo += sprintf_s(&OutTextInfo[iOutTextInfo], SizeTextInfo - iOutTextInfo, "\t\t%s\n", pIPAddr->IpAddress.String);
					pIPAddr = pIPAddr->Next;
				}

				//iOutTextInfo += sprintf_s(&OutTextInfo[iOutTextInfo], SizeTextInfo - iOutTextInfo, "\tNode Type: ");
				switch (pFixedInfo->NodeType) {
				case 1:
					//iOutTextInfo += sprintf_s(&OutTextInfo[iOutTextInfo], SizeTextInfo - iOutTextInfo, "%s\n", "Broadcast");
					break;
				case 2:
					//iOutTextInfo += sprintf_s(&OutTextInfo[iOutTextInfo], SizeTextInfo - iOutTextInfo, "%s\n", "Peer to peer");
					break;
				case 4:
					//iOutTextInfo += sprintf_s(&OutTextInfo[iOutTextInfo], SizeTextInfo - iOutTextInfo, "%s\n", "Mixed");
					break;
				case 8:
					//iOutTextInfo += sprintf_s(&OutTextInfo[iOutTextInfo], SizeTextInfo - iOutTextInfo, "%s\n", "Hybrid");
					break;
				default:
					//iOutTextInfo += sprintf_s(&OutTextInfo[iOutTextInfo], SizeTextInfo - iOutTextInfo, "\n");
					break;
				}

				//iOutTextInfo += sprintf_s(&OutTextInfo[iOutTextInfo], SizeTextInfo - iOutTextInfo, "\tNetBIOS Scope ID: %s\n", pFixedInfo->ScopeId);

				if (pFixedInfo->EnableRouting)
				{
					//iOutTextInfo += sprintf_s(&OutTextInfo[iOutTextInfo], SizeTextInfo - iOutTextInfo, "\tIP Routing Enabled: Yes\n");
				}
				else
				{
					//iOutTextInfo += sprintf_s(&OutTextInfo[iOutTextInfo], SizeTextInfo - iOutTextInfo, "\tIP Routing Enabled: No\n");
				}

				if (pFixedInfo->EnableProxy)
				{
					//iOutTextInfo += sprintf_s(&OutTextInfo[iOutTextInfo], SizeTextInfo - iOutTextInfo, "\tWINS Proxy Enabled: Yes\n");
				}
				else
				{
					//iOutTextInfo += sprintf_s(&OutTextInfo[iOutTextInfo], SizeTextInfo - iOutTextInfo, "\tWINS Proxy Enabled: No\n");
				}

				if (pFixedInfo->EnableDns)
				{
					//iOutTextInfo += sprintf_s(&OutTextInfo[iOutTextInfo], SizeTextInfo - iOutTextInfo, "\tNetBIOS Resolution Uses DNS: Yes\n");
				}
				else
				{
					//iOutTextInfo += sprintf_s(&OutTextInfo[iOutTextInfo], SizeTextInfo - iOutTextInfo, "\tNetBIOS Resolution Uses DNS: No\n");
				}
			}
		
			if (pFixedInfo)
			{
				free(pFixedInfo);
				pFixedInfo = NULL;
			}
		}
		
		{
			IP_ADAPTER_INFO  *pAdapterInfo;
			ULONG            ulOutBufLen;
			DWORD            dwRetVal;
		
			pAdapterInfo = (IP_ADAPTER_INFO *) malloc( sizeof(IP_ADAPTER_INFO) );
			ulOutBufLen = sizeof(IP_ADAPTER_INFO);

			if (GetAdaptersInfo( pAdapterInfo, &ulOutBufLen) != ERROR_SUCCESS)
			{
				free (pAdapterInfo);
				pAdapterInfo = (IP_ADAPTER_INFO *) malloc ( ulOutBufLen );
			}

			if ((dwRetVal = GetAdaptersInfo( pAdapterInfo, &ulOutBufLen)) != ERROR_SUCCESS)
			{
				iOutTextInfo += sprintf_s(&OutTextInfo[iOutTextInfo], SizeTextInfo - iOutTextInfo, "GetAdaptersInfo call failed with %d\n", dwRetVal);
			}
			else
			{
				PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
				while (pAdapter)
				{
					memmove_s(NetworkAdapterName[NetworkConnectionQuantity], MAX_PATH, pAdapter->AdapterName, MAX_PATH);
					NetworkConnectionIndex[NetworkConnectionQuantity]	= pAdapter->Index;
					char	resultString[MAX_PATH]	= "",
							KeyPath[MAX_PATH * 2]	= "";
					sprintf_s(KeyPath, sizeof(KeyPath), "SYSTEM\\CurrentControlSet\\Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\%s\\Connection", pAdapter->AdapterName);
					Registry_GetString(HKEY_LOCAL_MACHINE, KeyPath, "Name", resultString);
				
					memmove_s(NetworkConnectionName[NetworkConnectionQuantity], MAX_PATH, resultString, MAX_PATH);
					SendMessage(GetDlgItem(hdlg, IDC_netEDIT), LB_ADDSTRING, 0, (LPARAM)NetworkConnectionName[NetworkConnectionQuantity]);
					NetworkConnectionQuantity++;

					iOutTextInfo += sprintf_s(&OutTextInfo[iOutTextInfo], SizeTextInfo - iOutTextInfo, "Adapter Name: %s\n", pAdapter->AdapterName);
					iOutTextInfo += sprintf_s(&OutTextInfo[iOutTextInfo], SizeTextInfo - iOutTextInfo, "Adapter Desc: %s\n", pAdapter->Description);
					iOutTextInfo += sprintf_s(&OutTextInfo[iOutTextInfo], SizeTextInfo - iOutTextInfo, "Adapter Index: %d\n", pAdapter->Index);
					iOutTextInfo += sprintf_s(&OutTextInfo[iOutTextInfo], SizeTextInfo - iOutTextInfo, "\tAdapter Addr: \t");
			
					for (UINT i = 0; i < pAdapter->AddressLength; i++)
					{
						if (i == (pAdapter->AddressLength - 1))
						{
							iOutTextInfo += sprintf_s(&OutTextInfo[iOutTextInfo], SizeTextInfo - iOutTextInfo, "%.2X\n",(int)pAdapter->Address[i]);
						}
						else
						{
							iOutTextInfo += sprintf_s(&OutTextInfo[iOutTextInfo], SizeTextInfo - iOutTextInfo, "%.2X-",(int)pAdapter->Address[i]);
						}
					}
					iOutTextInfo += sprintf_s(&OutTextInfo[iOutTextInfo], SizeTextInfo - iOutTextInfo, "IP Address: %s\n", pAdapter->IpAddressList.IpAddress.String);
					iOutTextInfo += sprintf_s(&OutTextInfo[iOutTextInfo], SizeTextInfo - iOutTextInfo, "IP Mask: %s\n", pAdapter->IpAddressList.IpMask.String);
					iOutTextInfo += sprintf_s(&OutTextInfo[iOutTextInfo], SizeTextInfo - iOutTextInfo, "\tGateway: \t%s\n", pAdapter->GatewayList.IpAddress.String);
					iOutTextInfo += sprintf_s(&OutTextInfo[iOutTextInfo], SizeTextInfo - iOutTextInfo, "\t***\n");
			
					if (pAdapter->DhcpEnabled)
					{
						iOutTextInfo += sprintf_s(&OutTextInfo[iOutTextInfo], SizeTextInfo - iOutTextInfo, "\tDHCP Enabled: Yes\n");
						iOutTextInfo += sprintf_s(&OutTextInfo[iOutTextInfo], SizeTextInfo - iOutTextInfo, "\t\tDHCP Server: \t%s\n", pAdapter->DhcpServer.IpAddress.String);
					}
					else
					{
						iOutTextInfo += sprintf_s(&OutTextInfo[iOutTextInfo], SizeTextInfo - iOutTextInfo, "\tDHCP Enabled: No\n");
					}
			
					pAdapter = pAdapter->Next;
				}
			}
		
			if (pAdapterInfo)
			{
				free(pAdapterInfo);
			}
		}

		{
			ULONG				ulOutBufLen;
			DWORD				dwRetVal;

			IP_INTERFACE_INFO*	pInterfaceInfo;
			
			pInterfaceInfo = (IP_INTERFACE_INFO *) malloc(sizeof (IP_INTERFACE_INFO));
			ulOutBufLen = sizeof(IP_INTERFACE_INFO);

			if (GetInterfaceInfo(pInterfaceInfo, &ulOutBufLen) == ERROR_INSUFFICIENT_BUFFER)
			{
				free(pInterfaceInfo);
				pInterfaceInfo = (IP_INTERFACE_INFO *) malloc(ulOutBufLen);
			}
			
			if ((dwRetVal = GetInterfaceInfo(pInterfaceInfo, &ulOutBufLen)) != NO_ERROR)
			{
				iOutTextInfo += sprintf_s(&OutTextInfo[iOutTextInfo], SizeTextInfo - iOutTextInfo, "  GetInterfaceInfo failed with error: %d\n", dwRetVal);
			}
			else
			{
				iOutTextInfo += sprintf_s(&OutTextInfo[iOutTextInfo], SizeTextInfo - iOutTextInfo, "  GetInterfaceInfo succeeded.\n");
			
				iOutTextInfo += sprintf_s(&OutTextInfo[iOutTextInfo], SizeTextInfo - iOutTextInfo, "  Num Adapters: %ld\n\n", pInterfaceInfo->NumAdapters);
			
				for (unsigned int i = 0; i < (unsigned int) pInterfaceInfo->NumAdapters; i++)
				{
					iOutTextInfo += sprintf_s(&OutTextInfo[iOutTextInfo], SizeTextInfo - iOutTextInfo, "  Adapter Index[%d]: %ld\n", i, pInterfaceInfo->Adapter[i].Index);
					iOutTextInfo += sprintf_s(&OutTextInfo[iOutTextInfo], SizeTextInfo - iOutTextInfo, "  Adapter Name[%d]:  %ws\n\n", i, pInterfaceInfo->Adapter[i].Name);
				}
			}
			
			if (pInterfaceInfo)
			{
				free(pInterfaceInfo);
				pInterfaceInfo = NULL;
			}
		}

		{
			MIB_IPADDRTABLE  *pIPAddrTable;
			DWORD            dwSize = 0;
			DWORD            dwRetVal;
			
			pIPAddrTable = (MIB_IPADDRTABLE*) malloc(sizeof(MIB_IPADDRTABLE));
			
			if (GetIpAddrTable(pIPAddrTable, &dwSize, 0) == ERROR_INSUFFICIENT_BUFFER)
			{
				free( pIPAddrTable );
				pIPAddrTable = (MIB_IPADDRTABLE *) malloc ( dwSize );
			}
			
			if ((dwRetVal = GetIpAddrTable(pIPAddrTable, &dwSize, 0 )) != NO_ERROR)
			{
				iOutTextInfo += sprintf_s(&OutTextInfo[iOutTextInfo], SizeTextInfo - iOutTextInfo, "GetIpAddrTable call failed with %d\n", dwRetVal);
			}
			else
			{
				for (int i = 0; i < pIPAddrTable->dwNumEntries; i++)
				{
					iOutTextInfo += sprintf_s(&OutTextInfo[iOutTextInfo], SizeTextInfo - iOutTextInfo, "IP Address:         %ld\n", pIPAddrTable->table[i].dwAddr);
					iOutTextInfo += sprintf_s(&OutTextInfo[iOutTextInfo], SizeTextInfo - iOutTextInfo, "IP Mask:            %ld\n", pIPAddrTable->table[i].dwMask);
					iOutTextInfo += sprintf_s(&OutTextInfo[iOutTextInfo], SizeTextInfo - iOutTextInfo, "IF Index:           %ld\n", pIPAddrTable->table[i].dwIndex);
					iOutTextInfo += sprintf_s(&OutTextInfo[iOutTextInfo], SizeTextInfo - iOutTextInfo, "Broadcast Addr:     %ld\n", pIPAddrTable->table[i].dwBCastAddr);
					iOutTextInfo += sprintf_s(&OutTextInfo[iOutTextInfo], SizeTextInfo - iOutTextInfo, "Re-assembly size:   %ld\n", pIPAddrTable->table[i].dwReasmSize);
				}
			}
			
			if (pIPAddrTable)
			{
				free(pIPAddrTable);
			}
		}
	}	

	if (NetworkConnectionQuantity != 0)
	{
		SendMessage(GetDlgItem(hdlg, IDC_netEDIT), LB_SETCURSEL, NetworkConnectionQuantity - 1, 0);

		for (int i = 0; i < NetworkConnectionQuantity; i++)
		{
			if (strstr(NetworkConnectionName[i], Connection[language]))
			{
				SendMessage(GetDlgItem(hdlg, IDC_netEDIT), LB_SETCURSEL, i, 0);
				break;
			}
		}
	}
	else
	{
		SendMessage(GetDlgItem(hdlg, IDC_netEDIT), LB_ADDSTRING, 0, (LPARAM)ConnectionsNotDetected[language]);
	}
	SendMessage(GetDlgItem(hdlg, IDC_netIPADDRESS),		WM_SETTEXT, NULL, (LPARAM)"192.168.2.110");
	SendMessage(GetDlgItem(hdlg, IDC_netSUBNETMASK),	WM_SETTEXT, NULL, (LPARAM)"255.255.0.0");
	SendMessage(GetDlgItem(hdlg, IDC_netIPGATEWAY),		WM_SETTEXT, NULL, (LPARAM)"192.168.0.1");

	SetDlgItemText(hdlg, IDC_customShell, "C:\\APOLYECG\\adminRunner_.exe");

	char	resultString[MAX_PATH] = "";
	Registry_GetString(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon", "Shell", resultString);
	if (!strcmp(resultString, "explorer.exe"))
		SetDlgItemText(hdlg, IDC_customShell, "C:\\APOLYECG\\adminRunner_.exe");
	else
		SetDlgItemText(hdlg, IDC_customShell, "explorer.exe");
	//Registry_SetString(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon", "Shell", customPath);//explorer.exe
}

bool Evaluate(HWND hWnd)
{
	bool evaluate	= true;

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
			SHELLEXECUTEINFO sei	= {sizeof(sei)};
			sei.lpVerb				= TEXT("runas");
			sei.lpFile				= szPath;
			sei.hwnd				= hWnd;
			sei.nShow				= SW_NORMAL;
			sei.fMask				= SEE_MASK_CLASSNAME;
			sei.lpParameters		= new(TCHAR[(nFoldersToShare + 1) * MAX_PATH + NumberOfOptions * 3]);
			sei.lpClass				= _T("exefile");
			
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
			
			if (!ShellExecuteEx(&sei))
			{
				DWORD dwError = GetLastError();
				if (dwError == ERROR_CANCELLED)
				{
					// The user refused to allow privileges elevation.
					// Do nothing ...
				}
			}
			else
			{
				evaluate = false;
				//EndDialog(hWnd, TRUE);  // Quit itself
			}

			delete[] sei.lpParameters;
		}
	}
	else
	{
		//Мы уже под администратором
		//MessageBox(hWnd, TEXT("The process is running as administrator"), TEXT("UAC"), MB_OK);
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
			TCHAR	shareFolderPath[MAX_PATH]	= TEXT(""),
					emptyText[MAX_PATH]			= TEXT("");
			GetWindowText(GetDlgItem(hdlg, IDC_shareFolderEDIT), shareFolderPath, sizeof(shareFolderPath));
			memset(emptyText, 0, MAX_PATH - 1);
		
			if (_tcscmp(shareFolderPath, emptyText))
			{
				SendMessage(GetDlgItem(hdlg, IDC_shareFolderEDIT), CB_ADDSTRING,  0, (LPARAM)shareFolderPath);
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
	sa.nLength				= sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = pSD;
	sa.bInheritHandle		= FALSE;
	
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
			//SetSecurityInfo(hDir, SE_FILE_OBJECT, OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION, NULL, NULL, NULL, NULL);

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
			ea[0].grfInheritance= CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE;
			ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
			ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
			ea[0].Trustee.ptstrName  = (LPTSTR) pEveryoneSID;
			
			// Create a new ACL that contains the new ACEs.
			SetEntriesInAcl(1, ea, NULL, &pACL);
			
			// Initialize a security descriptor.
			PSECURITY_DESCRIPTOR pSD = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH); 

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
		if ( !EnumDisplaySettings((LPSTR)DisplayDevice.DeviceName, ENUM_REGISTRY_SETTINGS, &defaultMode) )
			OutputDebugString("Store default failed\n");

        if ((DisplayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) && !(DisplayDevice.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE))
        {
			DEVMODE    DevMode;
			ZeroMemory(&DevMode, sizeof(DevMode));
			DevMode.dmSize = sizeof(DevMode);
			DevMode.dmFields = DM_POSITION | DM_PELSWIDTH | DM_PELSHEIGHT;//DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_POSITION | DM_DISPLAYFREQUENCY | DM_DISPLAYFLAGS;
			DevMode.dmPelsWidth = DevMode.dmPelsHeight = 0;

            Result = ChangeDisplaySettingsEx((LPSTR)DisplayDevice.DeviceName, &DevMode, NULL, CDS_UPDATEREGISTRY, NULL);

            //The code below shows how to re-attach the secondary displays to the desktop

            //ChangeDisplaySettingsEx((LPSTR)DisplayDevice.DeviceName,
            //                       &defaultMode,
            //                       NULL,
            //                       CDS_UPDATEREGISTRY,
            //                       NULL);

		}

        // Reinit DisplayDevice just to be extra clean

        ZeroMemory(&DisplayDevice, sizeof(DisplayDevice));
        DisplayDevice.cb = sizeof(DisplayDevice);
        DispNum++;
	} // end while for all display devices
}

int GlobalFunction(int fIndex, HWND hdlg, BYTE flag, TCHAR *strBuffer)
{
	int retVal = 0;

	switch(fIndex)
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
						ZeroMemory(&dm,		sizeof(DEVMODE));
						dm.dmSize			= sizeof(DEVMODE);
						dm.dmBitsPerPel		= hDevModlist[lDevModlist - nDevMode - 1].dmBitsPerPel;	//Глубина цвета
						dm.dmPelsWidth		= hDevModlist[lDevModlist - nDevMode - 1].dmPelsWidth;	//Ширина монитора (в пикселях)
						dm.dmPelsHeight		= hDevModlist[lDevModlist - nDevMode - 1].dmPelsHeight;	//Высота монитора (в пикселях)
						dm.dmFields			= DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;

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
	
						typedef DWORD  (WINAPI* MYPROC)(HANDLE, LPWSTR);
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
						str +="PowerSetActiveScheme res = ";
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

						pp.user.VideoTimeoutAc		= 0;
						pp.user.VideoTimeoutDc		= 0;
						pp.user.SpindownTimeoutAc	= 0;
						pp.user.SpindownTimeoutDc	= 0;
						pp.user.IdleTimeoutAc		= 0;
						pp.user.IdleTimeoutDc		= 0;

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
					if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_MYPICTURES|CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, szPath)))
					{
						PathAppend(szPath, astro7);//WindowsXP хочет bmp
					}
					CopyFile(astro7, szPath, FALSE);
					Registry_SetString(HKEY_CURRENT_USER, "Control Panel\\Desktop", "WallpaperStyle",	"2");
					Registry_SetString(HKEY_CURRENT_USER, "Control Panel\\Desktop", "Wallpaper",		szPath);
					SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, szPath,  SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE);
				}
				break;
			}
		}
		break;
	case 4://void QuickDraw()
		{
			switch (flag)
			{
			case TO_CMD:
				break;
			case FROM_CMD:
				break;
			case NORMAL:
				{
					WCHAR   wszWallpaper[MAX_PATH];
					HRESULT	hr;
					IActiveDesktop *pActiveDesktop;
	
					CoInitialize(NULL);
					hr = CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC_SERVER, IID_IActiveDesktop, (void**)&pActiveDesktop);
					hr = pActiveDesktop->GetWallpaper(wszWallpaper, MAX_PATH, 0);
	
					Registry_SetDword(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\VisualEffects", "VisualFXSetting", 2);
					
					SystemParametersInfo(SPI_SETFONTSMOOTHING, FALSE, 0, SPIF_SENDCHANGE);
					SystemParametersInfo(SPI_SETCOMBOBOXANIMATION, 0, (PVOID)FALSE, SPIF_SENDCHANGE);
					SystemParametersInfo(SPI_SETGRADIENTCAPTIONS, 0, (PVOID)FALSE, SPIF_SENDCHANGE);
					SystemParametersInfo(SPI_SETHOTTRACKING, 0, (PVOID)FALSE, SPIF_SENDCHANGE);
					SystemParametersInfo(SPI_SETLISTBOXSMOOTHSCROLLING, 0, (PVOID)FALSE, SPIF_SENDCHANGE);
					SystemParametersInfo(SPI_SETMENUANIMATION, 0, (PVOID)FALSE, SPIF_SENDCHANGE);
					SystemParametersInfo(SPI_SETUIEFFECTS, 0, (PVOID)FALSE, SPIF_SENDCHANGE);
	
					pActiveDesktop->ApplyChanges(AD_APPLY_ALL);
					CoUninitialize();
	
					EnableTheming(FALSE);
				}
				break;
			}
		}
		break;
	case 5://void Autorun()
		{
			switch (flag)
			{
			case TO_CMD:
				break;
			case FROM_CMD:
				break;
			case NORMAL:
				{
					Registry_SetDword(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\policies\\Explorer", "NoDriveTypeAutoRun", 149);//параметр запрещающий запуск флешнакопителей
				}
				break;
			}
		}
		break;
	case 6://void AstroServiceProcess()
		{	
			switch (flag)
			{
			case TO_CMD:
				{
					retVal += _stprintf_s(&strBuffer[retVal], MAX_PATH, TEXT("%s%c"), path, divSign);
					retVal++;
				}
				break;
			case FROM_CMD:
				{
					int iPos = retVal;
					while ((strBuffer[iPos] != divSign))
					{
						iPos++;
					}

					_sntprintf_s(path, iPos - retVal + 1, _TRUNCATE, TEXT("%s"), &strBuffer[retVal]);
										
					retVal = iPos + 1;
				}
				break;
			case NORMAL:
				{
					TCHAR	INSTpath[MAX_PATH] = TEXT("");

					sprintf_s(INSTpath, MAX_PATH, "%s\\AstrocardServiceProcess.exe", path);
					GetProcessList(_T("AstrocardServiceProcess.exe"));
					
#ifndef WIN64

					CopyFile("ASP\\AstrocardServiceProcess.exe", INSTpath, FALSE);

#else
					CopyFile("ASP64\\AstrocardServiceProcess.exe", INSTpath, FALSE);

#endif

					Registry_SetString(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", "Astrocard Service Process", INSTpath);

					sprintf_s(INSTpath, MAX_PATH, "%s\\astrodll.dll", path);

#ifndef WIN64

					CopyFile("ASP\\astrodll.dll", INSTpath, FALSE);

#else

					CopyFile("ASP64\\astrodll.dll", INSTpath, FALSE);

#endif
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
					if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_WINDOWS|CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, szPath)))
					{
						PathAppend(szPath, TEXT("\\System32\\oobe\\info"));
						CreateDirectory(szPath, NULL);
						PathAppend(szPath, TEXT("\\backgrounds"));
						CreateDirectory(szPath, NULL);
						PathAppend(szPath, TEXT("\\backgroundDefault.jpg"));
					}
					CopyFile("astroWload.jpg", szPath, FALSE);
	
					SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, szPath,  SPIF_UPDATEINIFILE);
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

					if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_WINDOWS|CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, szPath)))
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
	case 9://void WUdisable()
		{
			switch (flag)
			{
			case TO_CMD:
				break;
			case FROM_CMD:
				break;
			case NORMAL:
				{
					Registry_SetDword(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate\\Auto Update", "AUOptions", 1);
				}
				break;
			}
		}
		break;
	case 10:
		{
			//Disable Windows Defender
			switch (flag)
			{
			case TO_CMD:
				break;
			case FROM_CMD:
				break;
			case NORMAL:
				{
					Registry_SetDword(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\Windows Defender", "DisableAntiSpyware", 1);
				}
				break;
			}
		}
		break;	
	case 11:
		{
			//Отключить значок "Центр поддержки" из области уведомлений  
			switch (flag)
			{
			case TO_CMD:
				break;
			case FROM_CMD:
				break;
			case NORMAL:
				{
					Registry_SetDword(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer", "HideSCAHealth", 1);
				}
				break;
			}
		}
		break;
	case 12://Включить Гостя
		{
			switch (flag)
			{
			case TO_CMD:
				break;
			case FROM_CMD:
				break;
			case NORMAL:
				{
					PSID pSid = NULL;

					TCHAR computerName[MAX_COMPUTERNAME_LENGTH + 1];
					DWORD compNameSize = MAX_COMPUTERNAME_LENGTH + 1;
					BOOL result = FALSE;
			
					if (GetComputerName(computerName, &compNameSize))
					{
						// Получим SID локальной машины
						PSID pSidLocalMashine	= NULL;

						LPTSTR domainName;
						DWORD cbSid = 0;
						SID_NAME_USE sidType;
						DWORD cchDomainName = 0;
				
						LookupAccountName(NULL, computerName, 0, &cbSid, NULL, &cchDomainName, &sidType);
				
						pSidLocalMashine = LocalAlloc(LMEM_FIXED, cbSid);
						domainName = new TCHAR[cchDomainName];
				
						if (LookupAccountName(NULL, computerName, pSidLocalMashine, &cbSid, domainName, &cchDomainName, &sidType))
						{
							SID_IDENTIFIER_AUTHORITY identAuth = SECURITY_NT_AUTHORITY;
					
							ULONG ssaCount = *GetSidSubAuthorityCount(pSidLocalMashine);
					
							if (ssaCount == 4)
							{
								DWORD ssa0 = *GetSidSubAuthority(pSidLocalMashine, 0);
								DWORD ssa1 = *GetSidSubAuthority(pSidLocalMashine, 1);
								DWORD ssa2 = *GetSidSubAuthority(pSidLocalMashine, 2);
								DWORD ssa3 = *GetSidSubAuthority(pSidLocalMashine, 3);
						
								// И соберем из него SID "Гостя"
								if (AllocateAndInitializeSid(&identAuth, 5, ssa0, ssa1, ssa2, ssa3, DOMAIN_USER_RID_GUEST, NULL, NULL, NULL, &pSid))
								{
									result = TRUE;

									LPTSTR	pSidStr = NULL;
							
									if (!(ConvertSidToStringSid(pSid, &pSidStr)))
									{
									}
									else
									{
										LSA_OBJECT_ATTRIBUTES ObjectAttributes;
										NTSTATUS ntsResult;
										LSA_HANDLE lsaHandle;
								
										ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes));
								
										ntsResult = LsaOpenPolicy(NULL, &ObjectAttributes, POLICY_ALL_ACCESS, &lsaHandle);
										if (ntsResult != STATUS_SUCCESS)
										{
											wprintf(L"OpenPolicy returned %lu\n", LsaNtStatusToWinError(ntsResult));
											//return NULL;
										}
										else
										{
											if (lsaHandle != NULL)
											{
												LSA_UNICODE_STRING lusUserRights;
												lusUserRights.Buffer = L"SeDenyInteractiveLogonRight";//SE_DENY_INTERACTIVE_LOGON_NAME;
												lusUserRights.Length = wcslen(L"SeDenyInteractiveLogonRight") * sizeof(WCHAR);
												lusUserRights.MaximumLength = (lusUserRights.Length + 1) * sizeof(WCHAR);

												NTSTATUS ret = LsaRemoveAccountRights(lsaHandle, pSid, FALSE, &lusUserRights, 1);
												if (ret != STATUS_SUCCESS)
												{
													printf("LsaRemoveAccountRights error: %lu\n", LsaNtStatusToWinError(ret));
												}

												lusUserRights.Buffer = L"SeInteractiveLogonRight";//SE_INTERACTIVE_LOGON_NAME;
												lusUserRights.Length = wcslen(L"SeInteractiveLogonRight") * sizeof(WCHAR);
												lusUserRights.MaximumLength = (lusUserRights.Length + 1) * sizeof(WCHAR);

												ret = LsaAddAccountRights(lsaHandle, pSid, &lusUserRights, 1);
												if (ret != STATUS_SUCCESS)
												{
													printf("LsaAddAccountRights error: %lu\n", LsaNtStatusToWinError(ret));
												}
										
												LPWSTR	userName	= 0;
												LPWSTR	domainName	= 0;
												DWORD	cchName		= 0;
												DWORD	cchDomain	= 0;
												SID_NAME_USE use;

												LookupAccountSidW(NULL, pSid, userName, &cchName, domainName, &cchDomain, &use);
		
												userName	= new WCHAR[cchName];
												domainName	= new WCHAR[cchDomain];
										
												if (LookupAccountSidW(NULL, pSid, userName, &cchName, domainName, &cchDomain, &use))
												{
													USER_INFO_1008	userInfo;
													DWORD			parm_err,
													level = 1008;			
											
													userInfo.usri1008_flags = UF_SCRIPT|UF_PASSWD_NOTREQD|UF_DONT_EXPIRE_PASSWD;
													//userInfo.usri1008_flags = UF_SCRIPT|UF_ACCOUNTDISABLE;//отключение записи
													ret = NetUserSetInfo(NULL, userName, level, (LPBYTE)&userInfo, &parm_err);
													if (ret != NERR_Success)
													{
														printf("NetUserSetInfo failed. Code 0x%.8X. Error 0x%.8X.\n", ret, parm_err);
													}
												}
												delete [] userName;
												delete [] domainName;	
										
												lusUserRights.Buffer		= L"SeDenyNetworkLogonRight";//SE_DENY_NETWORK_LOGON_NAME;
												lusUserRights.Length		= wcslen(L"SeDenyNetworkLogonRight") * sizeof(WCHAR);
												lusUserRights.MaximumLength = (lusUserRights.Length + 1) * sizeof(WCHAR);
								
												ret = LsaRemoveAccountRights(lsaHandle, pSid, FALSE, &lusUserRights, 1);
												if (ret != STATUS_SUCCESS)
												{
													printf("LsaRemoveAccountRights failed with %lu\n", LsaNtStatusToWinError(ret));
												}
												else
												{
													printf("Guest has been enabled.\n");
												}
											}
									
											FreeSid(pSidLocalMashine);									
										}

										LsaClose(lsaHandle);
									}

									LocalFree(pSidStr);
								}
							}
						}
					}

					FreeSid(pSid);
		
					//////////////////////AutoLogon
					if ((int)ShellExecute(NULL, "open", "control", "userpasswords2", NULL, SW_SHOWNORMAL) <= 32)//Проверка на ошибки
					{				
						//MessageBox(NULL, TEXT("Unable to run control userpasswords2"), TEXT("Error"), NULL);//In embedded we dont need that
					}
				}
				break;
			}
		}
		break;
	case 13://void SetIP()
		{
			char	commandLine[MAX_PATH]					= "",
					ipText[IPLENGTH]						= "",
					subnetText[IPLENGTH]					= "",
					gatewayText[IPLENGTH]					= "",
					PCnameText[MAX_HOSTNAME_LEN]			= "",
					PCworkGroupText[MAX_DOMAIN_NAME_LEN]	= "";
			
			switch (flag)
			{
			case TO_CMD:
				{
					retVal += _stprintf_s(&strBuffer[retVal], MAX_PATH, TEXT("%d%c"), SendMessage(GetDlgItem(hdlg, IDC_netEDIT), LB_GETCURSEL, 0, 0), divSign);
					retVal += GetWindowText(GetDlgItem(hdlg, IDC_netIPADDRESS),		&strBuffer[retVal],	sizeof(ipText));
					(TCHAR)strBuffer[retVal] = divSign;
					retVal++;
					retVal += GetWindowText(GetDlgItem(hdlg, IDC_netSUBNETMASK),	&strBuffer[retVal],	sizeof(subnetText));
					(TCHAR)strBuffer[retVal] = divSign;
					retVal++;
					retVal += GetWindowText(GetDlgItem(hdlg, IDC_netIPGATEWAY),		&strBuffer[retVal],	sizeof(gatewayText));
					(TCHAR)strBuffer[retVal] = divSign;
					retVal++;
					retVal += GetWindowText(GetDlgItem(hdlg, IDC_nameEDIT),			&strBuffer[retVal],	sizeof(PCnameText));
					(TCHAR)strBuffer[retVal] = divSign;
					retVal++;
					retVal += GetWindowText(GetDlgItem(hdlg, IDC_workGroupEDIT),	&strBuffer[retVal],	sizeof(PCworkGroupText));
					(TCHAR)strBuffer[retVal] = divSign;
					retVal++;
				}
				break;
			case FROM_CMD:
				{
					SendMessage(GetDlgItem(hdlg, IDC_netEDIT), LB_SETCURSEL, atoi(strBuffer), 0);

					while ((strBuffer[retVal] != divSign))
					{
						retVal++;
					}
					retVal++;
					
					int iPos = retVal;
					while ((strBuffer[iPos] != divSign))
					{
						iPos++;
					}
					
					_sntprintf_s(ipText, iPos - retVal + 1, _TRUNCATE, TEXT("%s"), &strBuffer[retVal]);
					SetWindowText(GetDlgItem(hdlg, IDC_netIPADDRESS),	ipText);
					retVal = iPos + 1;

					iPos = retVal;
					while ((strBuffer[iPos] != divSign))
					{
						iPos++;
					}

					_sntprintf_s(subnetText, iPos - retVal + 1, _TRUNCATE, TEXT("%s"), &strBuffer[retVal]);
					SetWindowText(GetDlgItem(hdlg, IDC_netSUBNETMASK),	subnetText);
					retVal = iPos + 1;

					iPos = retVal;
					while ((strBuffer[iPos] != divSign))
					{
						iPos++;
					}

					_sntprintf_s(gatewayText, iPos - retVal + 1, _TRUNCATE, TEXT("%s"), &strBuffer[retVal]);
					SetWindowText(GetDlgItem(hdlg, IDC_netIPGATEWAY),	gatewayText);
					retVal = iPos + 1;

					iPos = retVal;
					while ((strBuffer[iPos] != divSign))
					{
						iPos++;
					}

					_sntprintf_s(PCnameText, iPos - retVal + 1, _TRUNCATE, TEXT("%s"), &strBuffer[retVal]);
					SetWindowText(GetDlgItem(hdlg, IDC_nameEDIT),	PCnameText);
					retVal = iPos + 1;

					iPos = retVal;
					while ((strBuffer[iPos] != divSign))
					{
						iPos++;
					}
					
					_sntprintf_s(PCworkGroupText, iPos - retVal + 1, _TRUNCATE, TEXT("%s"), &strBuffer[retVal]);
					SetWindowText(GetDlgItem(hdlg, IDC_workGroupEDIT),	PCworkGroupText);
					retVal = iPos + 1;
				}
				break;
			case NORMAL:
				{
					GetWindowText(GetDlgItem(hdlg, IDC_netIPADDRESS),	ipText,			sizeof(ipText));
					GetWindowText(GetDlgItem(hdlg, IDC_netSUBNETMASK),	subnetText,		sizeof(subnetText));
					GetWindowText(GetDlgItem(hdlg, IDC_netIPGATEWAY),	gatewayText,	sizeof(gatewayText));

					wchar_t		WipText[IPLENGTH]		= L"",
								WsubnetText[IPLENGTH]	= L"",
								WgatewayText[IPLENGTH]	= L"";;

					swprintf(WipText,		L"%S", ipText);
					swprintf(WsubnetText,	L"%S", subnetText);
					swprintf(WgatewayText,	L"%S", gatewayText);

					/////////////////////Set IP via Win32_NetworkAdapterConfiguration Class
					HRESULT hres = CoInitializeEx(0, COINIT_MULTITHREADED),
							hres2;
		
					// Step 1: --------------------------------------------------
					// Initialize COM. ------------------------------------------
			
					if (FAILED(hres))
					{
						//"Failed to initialize COM library. Error code = 0x"
					}
					else
					{
						// Step 2: --------------------------------------------------
						// Set general COM security levels --------------------------
						// Note: If you are using Windows 2000, you need to specify -
						// the default authentication credentials for a user by using
						// a SOLE_AUTHENTICATION_LIST structure in the pAuthList ----
						// parameter of CoInitializeSecurity ------------------------
			
						hres = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT,  RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
			
						if (FAILED(hres))
						{
							//"Failed to initialize security. Error code = 0x" 
						}
						else
						{
							// Step 3: ---------------------------------------------------
							// Obtain the initial locator to WMI -------------------------
				
							IWbemLocator *pLoc = NULL;
							hres = CoCreateInstance(CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID *) &pLoc);
				
							if (FAILED(hres))
							{
								//"Failed to create IWbemLocator object." Err code = 0x"
							}
							else
							{
								// Step 4: -----------------------------------------------------
								// Connect to WMI through the IWbemLocator::ConnectServer method
					
								IWbemServices *pSvc = NULL;
					
								// Connect to the root\cimv2 namespace with
								// the current user and obtain pointer pSvc
								// to make IWbemServices calls.
								hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc);
					
								if (FAILED(hres))
								{
									//"Could not connect. Error code = 0x" 
								}
								else
								{
									//"Connected to ROOT\\CIMV2 WMI namespace" << endl;

									// Step 5: --------------------------------------------------
									// Set security levels on the proxy -------------------------
						
									hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
						
									if (FAILED(hres))
									{
										//"Could not set proxy blanket. Error code = 0x"
									}
									else
									{
										// Step 6: --------------------------------------------------
										// Use the IWbemServices pointer to make requests of WMI ----
										BSTR	ClassName	= SysAllocString(L"Win32_NetWorkAdapterConfiguration"),
												ip			= SysAllocString(WipText),
												mask		= SysAllocString(WsubnetText),
												gateway		= SysAllocString(WgatewayText);
										IWbemClassObject	*pClass = NULL;

										hres = pSvc->GetObject(ClassName, 0, NULL, &pClass, NULL);

										if (FAILED(hres))
										{
											//"Query for network adapter failed. Error code = 0x"
										}
										else
										{
											BSTR	MethodName		= SysAllocString(L"EnableStatic"),
													MethodGateways	= SysAllocString(L"SetGateways");
									
											IWbemClassObject	*pInParamsDefinition	= NULL,
																*pInParamsGateways		= NULL;
											hres = pClass->GetMethod(MethodName, 0,  &pInParamsDefinition, NULL);
											hres2 = pClass->GetMethod(MethodGateways, 0,  &pInParamsGateways, NULL);
							
											if (FAILED(hres) || FAILED(hres2))
											{
												//"Query for EnableStatic method failed. Error code = 0x"
											}
											else
											{
												IWbemClassObject	*pClassInstance = NULL,
																	*pClassGateways = NULL;
												hres	= pInParamsDefinition->SpawnInstance(0, &pClassInstance);
												hres2	= pInParamsGateways->SpawnInstance(0,	&pClassGateways);
							
												//IP && SubNetMask
												long index[] = {0};
							
												SAFEARRAY *ip_list		= SafeArrayCreateVector(VT_BSTR, 0, 1);
												SafeArrayPutElement(ip_list, index, ip);
							
												SAFEARRAY *mask_list	= SafeArrayCreateVector(VT_BSTR, 0, 1);
												SafeArrayPutElement(mask_list, index, mask);
							
												VARIANT arg1;
												arg1.vt = VT_ARRAY|VT_BSTR;
												arg1.parray = ip_list;
												hres = pClassInstance->Put(bstr_t("IPAddress"), 0, &arg1, 0);
							
												VARIANT arg2;
												arg2.vt = VT_ARRAY|VT_BSTR;
												arg2.parray = mask_list;
												hres = pClassInstance->Put(bstr_t("SubNetMask"), 0, &arg2, 0);
							
												///Gateways
												long index2[]={0};
							
												SAFEARRAY *gateways_list	= SafeArrayCreateVector(VT_BSTR, 0, 1);
												SafeArrayPutElement(gateways_list, index2, gateway);
							
												SAFEARRAY *costmetric_list	= SafeArrayCreateVector(VT_UINT, 0, 1);
												unsigned int I = 1; 
												SafeArrayPutElement(costmetric_list, index2, &I);
							
												VARIANT arg3;
												arg3.vt = VT_ARRAY|VT_BSTR;
												arg3.parray = gateways_list;
												hres = pClassGateways->Put(bstr_t("DefaultIPGateway"), 0, &arg3, 0);
							
												VARIANT arg4;
												arg4.vt = VT_ARRAY|VT_UINT;
												arg4.parray = costmetric_list;
												hres = pClassGateways->Put(bstr_t("GatewayCostMetric"), 0, &arg4, 0);
												////////////////

												IEnumWbemClassObject* pEnumerator = NULL;
												hres = pSvc->CreateInstanceEnum(ClassName, WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);
							
												if (FAILED(hres))
												{
													//"Enum for network adapter failed. Error code = 0x"
												}
												else
												{
													// Step 7: -------------------------------------------------
													// Get the data from the query in step 6 -------------------
								
													IWbemClassObject	*pclsObj = NULL,
																		*pOutInst = NULL;
													ULONG uReturn = 0;
													VARIANT pathVar;
 
													while (pEnumerator)
													{
														HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
									
														if (0 == uReturn)
														{
															break;
														}
											
														VARIANT desc;
														VARIANT id;
														VARIANT dhcp;
														VARIANT ipEnabled;
 
														BSTR DHCP = SysAllocString(L"DHCPEnabled");
														pclsObj->Get(DHCP, 0, &dhcp, 0, 0);
														SysFreeString(DHCP);
												
														BSTR IPEnable = SysAllocString(L"IPEnabled");
														pclsObj->Get(IPEnable, 0, &ipEnabled, 0, 0);
														SysFreeString(IPEnable);
												
														BSTR Description = SysAllocString(L"SettingID");//Description");
														pclsObj->Get(Description, 0, &desc, 0, 0);
														SysFreeString(Description);
												
														BSTR AdapterId = SysAllocString(L"InterfaceIndex");//Не поддерживается в Windows XP
														pclsObj->Get(AdapterId, 0, &id, 0, 0);
														SysFreeString(AdapterId);
												
														char curNetworkAdapterName[MAX_PATH] = "";

														sprintf(curNetworkAdapterName, "%S", desc.bstrVal);
												
														//if (NetworkConnectionIndex[SendMessage(GetDlgItem(hdlg, IDC_netEDIT), LB_GETCURSEL, 0, 0)] == V_I4(&id))//Не работает в Windows XP
														if (strcmp(curNetworkAdapterName, NetworkAdapterName[SendMessage(GetDlgItem(hdlg, IDC_netEDIT), LB_GETCURSEL, 0, 0)]) == 0)
														{
															// now we have the instance of the actual network card we want to modify
															// let's query it's PATH property so that we can call the method on it.
															BSTR PropName = SysAllocString(L"__PATH");
															pclsObj->Get(PropName, 0, &pathVar, 0, 0);
															BSTR InstancePath = pathVar.bstrVal;
												
															// finally we get to call the actuall method we want (EnableStatic())
															hr = pSvc->ExecMethod(InstancePath, MethodName, 0, NULL, pClassInstance, &pOutInst, NULL);
															long ret = 0;
									
															if (hr != WBEM_S_NO_ERROR)
															{
																BSTR Text;
																hr = pOutInst->GetObjectText(0, &Text);
																SysFreeString(Text);
															}
															else
															{
																// Get the EnableStatic method return value
																VARIANT ret_value;
																BSTR	strReturnValue = SysAllocString(L"ReturnValue");
																hr = pOutInst->Get(strReturnValue, 0, &ret_value, 0, 0);
																long	ret = V_I4(&ret_value);
																VariantClear(&ret_value);

																hr = pSvc->ExecMethod(InstancePath, MethodGateways, 0, NULL, pClassGateways, &pOutInst, NULL);

																if (hr != WBEM_S_NO_ERROR)
																{
																	BSTR Text;
																	hr = pOutInst->GetObjectText(0, &Text);
																	SysFreeString(Text);
																}
																else
																{
																	// Get the SetGateaway method return value
																	VARIANT ret_value;
																	BSTR	strReturnValue = SysAllocString(L"ReturnValue");
																	hr = pOutInst->Get(strReturnValue, 0, &ret_value, 0, 0);
																	long	ret = V_I4(&ret_value);
																	SysFreeString(strReturnValue);
																	VariantClear(&ret_value);
																}
															}
															SysFreeString(PropName);
														}

														VariantClear(&desc);
														VariantClear(&id);
														VariantClear(&dhcp);
														VariantClear(&ipEnabled);
													}
													VariantClear(&pathVar);								
													pOutInst->Release();
													pclsObj->Release();
												}
									
												VariantClear(&arg1);
												VariantClear(&arg2);
												VariantClear(&arg3);
												VariantClear(&arg4);

												pEnumerator->Release();
												pClassInstance->Release(),
												pClassGateways->Release();
											}
											pInParamsDefinition->Release();
											pInParamsGateways->Release();
											SysFreeString(MethodName);
											SysFreeString(MethodGateways);
										}
										pClass->Release();
										
										SysFreeString(ip);
										SysFreeString(mask);
										SysFreeString(gateway);
										SysFreeString(ClassName);
									}
									pSvc->Release();
								}
								pLoc->Release();
							}
						}
						CoUninitialize();
					}
					/////////////////////////////////////////		
					///Change IP by netsh
					//sprintf_s(commandLine, MAX_PATH, "interface ip set address \"%s\" static %s %s %s 1", NetworkConnectionName[SendMessage(GetDlgItem(hdlg, IDC_netEDIT), LB_GETCURSEL, 0, 0)], ipText, subnetText, getawayText);
					//Везде, кроме XP можно и нужно писать не ip, а ipv4
					//Везде, кроме XP можно и нужно ссылаться на индекс подключения, а не на его имя (не NetworkConnectionName, а NetworkConnectionIndex)
					//ShellExecute(NULL, "open", "netsh.exe", commandLine, NULL, SW_HIDE);

					/////////NAME && WORKGROUP
					GetWindowText(GetDlgItem(hdlg, IDC_nameEDIT), PCnameText, sizeof(PCnameText));

					SetComputerNameEx(ComputerNamePhysicalDnsHostname, PCnameText);

					GetWindowText(GetDlgItem(hdlg, IDC_workGroupEDIT), PCworkGroupText, sizeof(PCworkGroupText));
					wchar_t		WPCworkGroupText[MAX_PATH]	= L"";
					swprintf(WPCworkGroupText, L"%S", PCworkGroupText);
					NetJoinDomain(NULL, WPCworkGroupText, NULL, NULL, NULL, NULL);
				}
				break;
			}
		}
		break;
	case 14://void ShareFolder()
		{
			switch (flag)
			{
			case TO_CMD:
				{
					int nFoldersToShare = SendMessage(GetDlgItem(hdlg, IDC_shareFolderEDIT), CB_GETCOUNT, NULL, NULL);
					retVal += _stprintf_s(&strBuffer[retVal], MAX_PATH, TEXT("%d%c"), nFoldersToShare, divSign);
					for (int i = 0; i < nFoldersToShare; i++)
					{
						retVal += SendMessage(GetDlgItem(hdlg, IDC_shareFolderEDIT), CB_GETLBTEXT, i, (LPARAM)&strBuffer[retVal]);
						(TCHAR)strBuffer[retVal] = divSign;
						retVal++;
					}
				}
				break;
			case FROM_CMD:
				{
					int nFoldersToShare = atoi(strBuffer),
						nPos = 1;
					while ((strBuffer[nPos] != divSign))
					{
						nPos++;
					}
					retVal = nPos + 1;
					for (int i = 0; i < nFoldersToShare; i++)
					{
						int iPos = retVal;
						while ((strBuffer[iPos] != divSign))
						{
							iPos++;
						}
						TCHAR	shareFolderPath[MAX_PATH] = TEXT("");
						_sntprintf_s(shareFolderPath, iPos - retVal + 1, _TRUNCATE, TEXT("%s"), &strBuffer[retVal]);
						SendMessage(GetDlgItem(hdlg, IDC_shareFolderEDIT), CB_ADDSTRING,  0, (LPARAM)shareFolderPath);
						retVal = iPos + 1;
					}
				}
				break;
			case NORMAL:
				{
					HRESULT				hr;
					INetCfg*			cfg;
					INetCfgLock*		lock;
					LPWSTR				szLockedBy;
					BOOL				bSetup;
					INetCfgClassSetup*	ncClassSetup;
					INetCfgComponent*	ncc;
					OBO_TOKEN			oboToken;
			
					CoInitialize(NULL);

					hr = CoCreateInstance(CLSID_CNetCfg, NULL, CLSCTX_INPROC_SERVER, IID_INetCfg, (void **)&cfg);
					if (hr == S_OK)
					{
						hr = cfg->QueryInterface(IID_INetCfgLock, (void **)&lock);
						if (hr == S_OK)
						{
							hr = lock->AcquireWriteLock(500, L"Astrocard", &szLockedBy);
							if (hr == S_OK)
							{
								hr = cfg->Initialize(NULL);
								if (hr == S_OK)
								{
									WCHAR	szPath[MAX_PATH],
											szInfNameAfterCopy[MAX_PATH];			
			
									swprintf_s(szPath, MAX_PATH, L"c:\\windows\\inf\\netserv.inf");
							
									bSetup = SetupCopyOEMInfW(szPath, NULL, SPOST_PATH, 0, szInfNameAfterCopy, MAX_PATH, NULL, NULL);
									if (bSetup == TRUE)
									{
										hr = cfg->QueryNetCfgClass(&GUID_DEVCLASS_NETSERVICE, IID_INetCfgClassSetup, (void**)&ncClassSetup);
										if (hr == S_OK)
										{
											ZeroMemory(&oboToken, sizeof(oboToken));
											oboToken.Type = OBO_USER;
									
											hr = ncClassSetup->Install(L"ms_server"/*NETCFG_SERVICE_CID_MS_SERVER*/, &oboToken, NSF_POSTSYSINSTALL, 0, NULL, NULL, &ncc);
											printf("Install procedure returned code 0x%.8X\n", hr);
											hr = cfg->Apply();
									
											ncClassSetup->Release();
										}
										else
										{
											char errorText[256] = "";
											sprintf(errorText, "QueryNetCfgClass failed with 0x%.8X\n", hr);
											MessageBox(0, errorText, 0, 0);

										}
									}
									else
									{
										char errorText[256] = "";
										sprintf(errorText, "SetupCopyOEMInf failed with code 0x%.8X\n", GetLastError());
										MessageBox(0, errorText, 0, 0);
										hr = S_FALSE;
									}
							
									cfg->Uninitialize();
								}
								else
								{
									char errorText[256] = "";
									sprintf(errorText, "Initialize failed with 0x%.8X\n", hr);
									MessageBox(0, errorText, 0, 0);
								}
						
								lock->ReleaseWriteLock();
							}
							else
							{
								char errorText[256] = "";
								sprintf(errorText, "AcquireWriteLock failed with 0x%.8X\n", hr);
								MessageBox(0, errorText, 0, 0);
							}
					
							lock->Release();
						}
						else
						{
							char errorText[256] = "";
							sprintf(errorText, "Query INetCfgLock failed with 0x%.8X\n", hr);
							MessageBox(0, errorText, 0, 0);
						}
				
						cfg->Release();
					}
					else
					{
						char errorText[256] = "";
						sprintf(errorText, "CoCreateInstance failed with 0x%.8X\n", hr);
						MessageBox(0, errorText, "Share_Folder", 0);
					}

					CoUninitialize();
					
					///////////////Create SECURITY_DESCRIPTOR
					PSECURITY_DESCRIPTOR	pSD;
					PSID	pEveryoneSID	= NULL,
							pAdminSID		= NULL,
							pGuestSID		= NULL;
					PACL	pACL			= NULL;
					EXPLICIT_ACCESS ea[numbUSERS];
					ZeroMemory(&ea, numbUSERS * sizeof(EXPLICIT_ACCESS));
					SID_IDENTIFIER_AUTHORITY	SIDAuthWorld	= SECURITY_WORLD_SID_AUTHORITY,
												SIDAuthNT		= SECURITY_NT_AUTHORITY,
												SIDGuest		= SECURITY_NT_AUTHORITY;
					if (AllocateAndInitializeSid(&SIDAuthWorld, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &pEveryoneSID))
					{
						ea[0].grfAccessPermissions	= GENERIC_ALL;
						ea[0].grfAccessMode			= SET_ACCESS;
						ea[0].grfInheritance		= NO_INHERITANCE;
						ea[0].Trustee.TrusteeForm	= TRUSTEE_IS_SID;
						ea[0].Trustee.TrusteeType	= TRUSTEE_IS_WELL_KNOWN_GROUP;//TRUSTEE_IS_GROUP;//TRUSTEE_IS_WELL_KNOWN_GROUP;
						ea[0].Trustee.ptstrName		= (LPTSTR)pEveryoneSID;
					}					
					if (AllocateAndInitializeSid(&SIDAuthNT, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &pAdminSID))
					{
						ea[1].grfAccessPermissions	= GENERIC_ALL;
						ea[1].grfAccessMode			= SET_ACCESS;
						ea[1].grfInheritance		= NO_INHERITANCE;
						ea[1].Trustee.TrusteeForm	= TRUSTEE_IS_SID;
						ea[1].Trustee.TrusteeType	= TRUSTEE_IS_GROUP;
						ea[1].Trustee.ptstrName		= (LPTSTR)pAdminSID;
					}					
					
					// Create a new ACL that contains the new ACEs.
					DWORD dwRes = SetEntriesInAcl(numbUSERS, ea, NULL, &pACL);

					// Initialize a security descriptor.
					pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
					if (NULL == pSD)
					{
						char	errorText[64] = "";
						sprintf_s(errorText, sizeof(errorText) - 1, "LocalAlloc Error %u\n", GetLastError());
						MessageBox(NULL, errorText, "Error", NULL);
					}
		
					if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
					{
						char	errorText[64] = "";
						sprintf_s(errorText, sizeof(errorText) - 1, "InitializeSecurityDescriptor Error %u\n", GetLastError());
						MessageBox(NULL, errorText, "Error", NULL);
					}
		
					if (!SetSecurityDescriptorDacl(pSD, TRUE, pACL, FALSE))
					{
						char	errorText[64] = "";
						sprintf_s(errorText, sizeof(errorText) - 1, "SetSecurityDescriptorDacl Error %u\n", GetLastError());
						MessageBox(NULL, errorText, "Error", NULL);
					}
		
					if (!IsValidSecurityDescriptor(pSD))
					{
						pSD = NULL;
					}
					/////////////////

					int nFoldersToShare = SendMessage(GetDlgItem(hdlg, IDC_shareFolderEDIT), CB_GETCOUNT, NULL, NULL);
					for (int i = 0; i < nFoldersToShare; i++)
					{
						char	shareFolderPath[MAX_PATH] = "";
				
						int shareFolderLen = SendMessage(GetDlgItem(hdlg, IDC_shareFolderEDIT), CB_GETLBTEXT, i, (LPARAM)&shareFolderPath);
												
						wchar_t		WshareFolderPath[MAX_PATH]	= L"",
									Wnetname[MAX_PATH]			= L"";

						for (int j = 0; j < MAX_PATH - 1; j++)
						{
							if (shareFolderPath[j] == '/')
							{
								shareFolderPath[j] = '\\';//разворачиваем слеши
							}
						}

						if (shareFolderPath[shareFolderLen - 1] == '\\')//Путь к папка не должен заканчиваться на слешь
						{
							sprintf(&shareFolderPath[shareFolderLen - 1], "");
						}			
			
						swprintf(WshareFolderPath,	L"%S", shareFolderPath);
						swprintf(Wnetname,			L"%S", shareFolderPath);

						char *Tnetname = new char[MAX_PATH];
						Tnetname =  strstr(shareFolderPath, "\\");

						while (Tnetname)
						{
							swprintf(Wnetname, L"%S", &Tnetname[1]);
							Tnetname =  strstr(&Tnetname[1], "\\");
						}
				
						NET_API_STATUS		res;
						SHARE_INFO_502		inf;
						DWORD				parm_err;
												
						if (CreateDirectoryWithUserFullControlACL(shareFolderPath, pSD))//Создадим директорию и раздадим ей права
						{
							memset(&inf, 0, sizeof(inf));								//Обнуляем структуру
							inf.shi502_netname				= (LPWSTR)Wnetname;			//Имя шаринга
							inf.shi502_type					= STYPE_DISKTREE;			//расшариваем папку
							inf.shi502_remark				= L"Shared_By_Astro";		//Примечание. Хорошо наблюдается из консоли 'net share'
							inf.shi502_permissions			= ACCESS_ALL;				//Доступ на чтение и исполнение
							inf.shi502_max_uses				= 4;						//Ограничение на макс. кол-во подключеных юзеров
							inf.shi502_path					= (LPWSTR)WshareFolderPath; //Собственно папка, которую расшариваем
							inf.shi502_passwd				= NULL;						//Без пароля
							inf.shi502_security_descriptor	= pSD;						//Права
							res = NetShareAdd(NULL, 502, (LPBYTE)&inf, &parm_err); 

							if (res != NERR_Success)
							{
								if (res == NERR_DuplicateShare)
								{
									res = NetShareSetInfo(NULL, inf.shi502_netname, 502, (LPBYTE)&inf, &parm_err);
									if (res != NERR_Success)
									{
										char textError1[16] = "";
										sprintf_s(textError1, MAX_PATH, "%s error %d\r\nparm_err %d", shareFolderPath, res, parm_err);
										MessageBox(0, textError1, 0, 0);//SHARE_NETNAME_PARMNUM
									}
								}
								else
								{
									char textError[16] = "";
									sprintf_s(textError, MAX_PATH, "%s error %d\r\nparm_err %d", shareFolderPath, res, parm_err);
									MessageBox(0, textError, 0, 0);
								}
							}
						}
						else
						{
							char textError[16] = "";
							sprintf_s(textError, MAX_PATH, "%s error %d", shareFolderPath, GetLastError());
							MessageBox(0, textError, 0, 0);
						}
					}

					if (pEveryoneSID) 
						FreeSid(pEveryoneSID);
					if (pAdminSID) 
						FreeSid(pAdminSID);
					if (pACL) 
						LocalFree(pACL);
					if (pSD) 
						LocalFree(pSD);
				}
				break;
			}
		}
		break;
	case 15://void CustomShell()
		{
			char	commandLine[MAX_PATH] = "",
					customPath[MAX_PATH] = "";

			switch (flag)
			{
			case TO_CMD:
				{
					retVal += _stprintf_s(&strBuffer[retVal], MAX_PATH, TEXT("%d%c"), SendMessage(GetDlgItem(hdlg, IDC_customShell), LB_GETCURSEL, 0, 0), divSign);
					retVal += GetWindowText(GetDlgItem(hdlg, IDC_customShell), &strBuffer[retVal], sizeof(customPath));
					(TCHAR)strBuffer[retVal] = divSign;
					retVal++;					
				}
				break;
			case FROM_CMD:
				{
					int iPos = retVal;
					while ((strBuffer[iPos] != divSign))
					{
						iPos++;
					}

					_sntprintf_s(customPath, iPos - retVal + 1, _TRUNCATE, TEXT("%s"), &strBuffer[retVal]);
					SetDlgItemText(hdlg, IDC_customShell, customPath);
					retVal = iPos + 1;					
				}
				break;
			case NORMAL:
				{
					GetDlgItemText(hdlg, IDC_customShell, customPath, MAX_PATH);
					//MessageBox(0, customPath, 0, 0);
					Registry_SetString(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon", "Shell", customPath);

					Registry_SetDword(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", "DisableChangePassword",		1);
					Registry_SetDword(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", "DisableLockWorkstation",	1);
					Registry_SetDword(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", "DisableTaskMgr",			1);
				}
				break;
			}
		}
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
	RegSetValueEx(hkey, Key, NULL, REG_SZ, (unsigned char*)valueString,	valueStringL);
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