#define _WININET_
#define INTERNET_MAX_URL_LENGTH MAX_PATH

#define	IPLENGTH			16

#define NumberOfOptions		16
#define NumberOfLanguages	2

#define _WIN32_DCOM

#ifndef STATUS_SUCCESS

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' "\
"version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#pragma comment(lib, "powrprof.lib")
#pragma comment(lib, "UxTheme.lib")
#pragma comment(lib, "comctl32.lib")
#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)
#endif
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "wsock32.lib") 
#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib, "Netapi32.lib") 
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "Setupapi.lib")

#include <winsock2.h>
#include <iphlpapi.h>
#include <lm.h>
#include <stdio.h>
#include <iostream>
#include <comdef.h>
#include <Wbemidl.h>
#include <aclapi.h>
#include <ntsecapi.h>
#include <sddl.h>
#include <setupapi.h>

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <wingdi.h>
#include <objidl.h>
#include <io.h>
#include <shlobj.h>
#include <Winuser.h>
#include <Winreg.h>
#include <math.h>
#include <time.h> 
#include <string>
#include <string.h>
#include <tchar.h>
#include <shlwapi.h>
#include <TlHelp32.h>
#include "resource.h"
#include "disk32.h"
#include "syslist.h"
#include "PowrProf.h"
#include "commctrl.h"
#include "UxTheme.h"
#include "vssym32.h"
#include "fromDDK/netcfgx.h"
#include "fromDDK/netcfgn.h"
#include "fromDDK/devguid.h"

#include "UACSelfElevation.h"