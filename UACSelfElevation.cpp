#include "chgstg.h"

//
//   FUNCTION: IsUserInAdminGroup()
//
//   PURPOSE: The function checks whether the primary access token of the 
//   process belongs to user account that is a member of the local 
//   Administrators group, even if it currently is not elevated.
//
//   RETURN VALUE: Returns TRUE if the primary access token of the process 
//   belongs to user account that is a member of the local Administrators 
//   group. Returns FALSE if the token does not.
//
//   EXCEPTION: If this function fails, it throws a C++ DWORD exception which 
//   contains the Win32 error code of the failure.
//
//   EXAMPLE CALL:
//     try 
//     {
//         if (IsUserInAdminGroup())
//             wprintf (L"User is a member of the Administrators group\n");
//         else
//             wprintf (L"User is not a member of the Administrators group\n");
//     }
//     catch (DWORD dwError)
//     {
//         wprintf(L"IsUserInAdminGroup failed w/err %lu\n", dwError);
//     }
//
BOOL IsUserInAdminGroup()
{
	BOOL	fInAdminGroup	= FALSE;
	DWORD	dwError			= ERROR_SUCCESS,
			cbSize			= 0;
	HANDLE	hToken			= NULL,
			hTokenToCheck	= NULL;

    // Open the primary access token of the process for query and duplicate.
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_DUPLICATE, &hToken))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    // Determine whether system is running Windows Vista or later operating 
    // systems (major version >= 6) because they support linked tokens, but 
    // previous versions (major version < 6) do not.
    OSVERSIONINFO osver = { sizeof(osver) };
    if (!GetVersionEx(&osver))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    if (osver.dwMajorVersion >= 6)
    {
        // Running Windows Vista or later (major version >= 6). 
        // Determine token type: limited, elevated, or default. 
        TOKEN_ELEVATION_TYPE elevType;
        if (!GetTokenInformation(hToken, TokenElevationType, &elevType, sizeof(elevType), &cbSize))
        {
            dwError = GetLastError();
            goto Cleanup;
        }

        // If limited, get the linked elevated token for further check.
        if (TokenElevationTypeLimited == elevType)
        {
            if (!GetTokenInformation(hToken, TokenLinkedToken, &hTokenToCheck, 
                sizeof(hTokenToCheck), &cbSize))
            {
                dwError = GetLastError();
                goto Cleanup;
            }
        }
    }
    
    // CheckTokenMembership requires an impersonation token. If we just got a 
    // linked token, it already is an impersonation token.  If we did not get 
    // a linked token, duplicate the original into an impersonation token for 
    // CheckTokenMembership.
    if (!hTokenToCheck)
    {
        if (!DuplicateToken(hToken, SecurityIdentification, &hTokenToCheck))
        {
            dwError = GetLastError();
            goto Cleanup;
        }
    }

    // Create the SID corresponding to the Administrators group.
    BYTE adminSID[SECURITY_MAX_SID_SIZE];
    cbSize = sizeof(adminSID);
    if (!CreateWellKnownSid(WinBuiltinAdministratorsSid, NULL, &adminSID, &cbSize))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    // Check if the token to be checked contains admin SID.
    // http://msdn.microsoft.com/en-us/library/aa379596(VS.85).aspx:
    // To determine whether a SID is enabled in a token, that is, whether it 
    // has the SE_GROUP_ENABLED attribute, call CheckTokenMembership.
    if (!CheckTokenMembership(hTokenToCheck, &adminSID, &fInAdminGroup)) 
    {
        dwError = GetLastError();
        goto Cleanup;
    }

Cleanup:
    // Centralized cleanup for all allocated resources.
    if (hToken)
    {
        CloseHandle(hToken);
        hToken = NULL;
    }
    if (hTokenToCheck)
    {
        CloseHandle(hTokenToCheck);
        hTokenToCheck = NULL;
    }

    // Throw the error if something failed in the function.
    if (ERROR_SUCCESS != dwError)
    {
        throw dwError;
    }

    return fInAdminGroup;
}

// 
//   FUNCTION: IsRunAsAdmin()
//
//   PURPOSE: The function checks whether the current process is run as 
//   administrator. In other words, it dictates whether the primary access 
//   token of the process belongs to user account that is a member of the 
//   local Administrators group and it is elevated.
//
//   RETURN VALUE: Returns TRUE if the primary access token of the process 
//   belongs to user account that is a member of the local Administrators 
//   group and it is elevated. Returns FALSE if the token does not.
//
//   EXCEPTION: If this function fails, it throws a C++ DWORD exception which 
//   contains the Win32 error code of the failure.
//
//   EXAMPLE CALL:
//     try 
//     {
//         if (IsRunAsAdmin())
//             wprintf (L"Process is run as administrator\n");
//         else
//             wprintf (L"Process is not run as administrator\n");
//     }
//     catch (DWORD dwError)
//     {
//         wprintf(L"IsRunAsAdmin failed w/err %lu\n", dwError);
//     }
//
BOOL IsRunAsAdmin()
{
    BOOL fIsRunAsAdmin = FALSE;
    DWORD dwError = ERROR_SUCCESS;
    PSID pAdministratorsGroup = NULL;

    // Allocate and initialize a SID of the administrators group.
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    if (!AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &pAdministratorsGroup))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    // Determine whether the SID of administrators group is enabled in 
    // the primary access token of the process.
    if (!CheckTokenMembership(NULL, pAdministratorsGroup, &fIsRunAsAdmin))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

Cleanup:
    // Centralized cleanup for all allocated resources.
    if (pAdministratorsGroup)
    {
        FreeSid(pAdministratorsGroup);
        pAdministratorsGroup = NULL;
    }

    // Throw the error if something failed in the function.
    if (ERROR_SUCCESS != dwError)
    {
        throw dwError;
    }

    return fIsRunAsAdmin;
}

//
//   FUNCTION: IsProcessElevated()
//
//   PURPOSE: The function gets the elevation information of the current 
//   process. It dictates whether the process is elevated or not. Token 
//   elevation is only available on Windows Vista and newer operating 
//   systems, thus IsProcessElevated throws a C++ exception if it is called 
//   on systems prior to Windows Vista. It is not appropriate to use this 
//   function to determine whether a process is run as administartor.
//
//   RETURN VALUE: Returns TRUE if the process is elevated. Returns FALSE if 
//   it is not.
//
//   EXCEPTION: If this function fails, it throws a C++ DWORD exception 
//   which contains the Win32 error code of the failure. For example, if 
//   IsProcessElevated is called on systems prior to Windows Vista, the error 
//   code will be ERROR_INVALID_PARAMETER.
//
//   NOTE: TOKEN_INFORMATION_CLASS provides TokenElevationType to check the 
//   elevation type (TokenElevationTypeDefault / TokenElevationTypeLimited /
//   TokenElevationTypeFull) of the process. It is different from 
//   TokenElevation in that, when UAC is turned off, elevation type always 
//   returns TokenElevationTypeDefault even though the process is elevated 
//   (Integrity Level == High). In other words, it is not safe to say if the 
//   process is elevated based on elevation type. Instead, we should use 
//   TokenElevation.
//
//   EXAMPLE CALL:
//     try 
//     {
//         if (IsProcessElevated())
//             wprintf (L"Process is elevated\n");
//         else
//             wprintf (L"Process is not elevated\n");
//     }
//     catch (DWORD dwError)
//     {
//         wprintf(L"IsProcessElevated failed w/err %lu\n", dwError);
//     }
//
BOOL IsProcessElevated()
{
    BOOL fIsElevated = FALSE;
    DWORD dwError = ERROR_SUCCESS;
    HANDLE hToken = NULL;

    // Open the primary access token of the process with TOKEN_QUERY.
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    // Retrieve token elevation information.
    TOKEN_ELEVATION elevation;
    DWORD dwSize;
    if (!GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &dwSize))
    {
        // When the process is run on operating systems prior to Windows 
        // Vista, GetTokenInformation returns FALSE with the 
        // ERROR_INVALID_PARAMETER error code because TokenElevation is 
        // not supported on those operating systems.
        dwError = GetLastError();
        goto Cleanup;
    }

    fIsElevated = elevation.TokenIsElevated;

Cleanup:
    // Centralized cleanup for all allocated resources.
    if (hToken)
    {
        CloseHandle(hToken);
        hToken = NULL;
    }

    // Throw the error if something failed in the function.
    if (ERROR_SUCCESS != dwError)
    {
        throw dwError;
    }

    return fIsElevated;
}

//
//   FUNCTION: GetProcessIntegrityLevel()
//
//   PURPOSE: The function gets the integrity level of the current process. 
//   Integrity level is only available on Windows Vista and newer operating 
//   systems, thus GetProcessIntegrityLevel throws a C++ exception if it is 
//   called on systems prior to Windows Vista.
//
//   RETURN VALUE: Returns the integrity level of the current process. It is 
//   usually one of these values:
//
//     SECURITY_MANDATORY_UNTRUSTED_RID (SID: S-1-16-0x0)
//     Means untrusted level. It is used by processes started by the 
//     Anonymous group. Blocks most write access. 
//
//     SECURITY_MANDATORY_LOW_RID (SID: S-1-16-0x1000)
//     Means low integrity level. It is used by Protected Mode Internet 
//     Explorer. Blocks write acess to most objects (such as files and 
//     registry keys) on the system. 
//
//     SECURITY_MANDATORY_MEDIUM_RID (SID: S-1-16-0x2000)
//     Means medium integrity level. It is used by normal applications 
//     being launched while UAC is enabled. 
//
//     SECURITY_MANDATORY_HIGH_RID (SID: S-1-16-0x3000)
//     Means high integrity level. It is used by administrative applications 
//     launched through elevation when UAC is enabled, or normal 
//     applications if UAC is disabled and the user is an administrator. 
//
//     SECURITY_MANDATORY_SYSTEM_RID (SID: S-1-16-0x4000)
//     Means system integrity level. It is used by services and other 
//     system-level applications (such as Wininit, Winlogon, Smss, etc.)  
//
//   EXCEPTION: If this function fails, it throws a C++ DWORD exception 
//   which contains the Win32 error code of the failure. For example, if 
//   GetProcessIntegrityLevel is called on systems prior to Windows Vista, 
//   the error code will be ERROR_INVALID_PARAMETER.
//
//   EXAMPLE CALL:
//     try 
//     {
//         DWORD dwIntegrityLevel = GetProcessIntegrityLevel();
//     }
//     catch (DWORD dwError)
//     {
//         wprintf(L"GetProcessIntegrityLevel failed w/err %lu\n", dwError);
//     }
//
DWORD GetProcessIntegrityLevel()
{
    DWORD dwIntegrityLevel = 0;
    DWORD dwError = ERROR_SUCCESS;
    HANDLE hToken = NULL;
    DWORD cbTokenIL = 0;
    PTOKEN_MANDATORY_LABEL pTokenIL = NULL;

    // Open the primary access token of the process with TOKEN_QUERY.
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    // Query the size of the token integrity level information. Note that 
    // we expect a FALSE result and the last error ERROR_INSUFFICIENT_BUFFER
    // from GetTokenInformation because we have given it a NULL buffer. On 
    // exit cbTokenIL will tell the size of the integrity level information.
    if (!GetTokenInformation(hToken, TokenIntegrityLevel, NULL, 0, &cbTokenIL))
    {
        if (ERROR_INSUFFICIENT_BUFFER != GetLastError())
        {
            // When the process is run on operating systems prior to Windows 
            // Vista, GetTokenInformation returns FALSE with the 
            // ERROR_INVALID_PARAMETER error code because TokenElevation 
            // is not supported on those operating systems.
            dwError = GetLastError();
            goto Cleanup;
        }
    }

    // Now we allocate a buffer for the integrity level information.
    pTokenIL = (TOKEN_MANDATORY_LABEL *)LocalAlloc(LPTR, cbTokenIL);
    if (pTokenIL == NULL)
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    // Retrieve token integrity level information.
    if (!GetTokenInformation(hToken, TokenIntegrityLevel, pTokenIL, cbTokenIL, &cbTokenIL))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    // Integrity Level SIDs are in the form of S-1-16-0xXXXX. (e.g. 
    // S-1-16-0x1000 stands for low integrity level SID). There is one and 
    // only one subauthority.
    dwIntegrityLevel = *GetSidSubAuthority(pTokenIL->Label.Sid, 0);

Cleanup:
    // Centralized cleanup for all allocated resources.
    if (hToken)
    {
        CloseHandle(hToken);
        hToken = NULL;
    }
    if (pTokenIL)
    {
        LocalFree(pTokenIL);
        pTokenIL = NULL;
        cbTokenIL = 0;
    }

    // Throw the error if something failed in the function.
    if (ERROR_SUCCESS != dwError)
    {
        throw dwError;
    }

    return dwIntegrityLevel;
}

//
//   FUNCTION: ReportError(LPWSTR, DWORD)
//
//   PURPOSE: Display an error dialog for the failure of a certain function.
//
//   PARAMETERS:
//   * pszFunction - the name of the function that failed.
//   * dwError - the Win32 error code. Its default value is the calling 
//   thread's last-error code value.
//
//   NOTE: The failing function must be immediately followed by the call of 
//   ReportError if you do not explicitly specify the dwError parameter of 
//   ReportError. This is to ensure that the calling thread's last-error code 
//   value is not overwritten by any calls of API between the failing 
//   function and ReportError.
//
void ReportError(LPCTSTR pszFunction, DWORD dwError)
{
    TCHAR szMessage[200];
    if (-1 != _stprintf_s(szMessage, ARRAYSIZE(szMessage), TEXT("%s failed w/err 0x%08lx"), pszFunction, dwError))
    {
        MessageBox(NULL, szMessage, TEXT("Error"), MB_ICONERROR);
    }
}

// 
//   FUNCTION: OnInitDialog(HWND, HWND, LPARAM)
//
//   PURPOSE: Process the WM_INITDIALOG message. Check and display the 
//   "run as administrator" status, the elevation information, and the 
//   integrity level of the current process.
//
BOOL OnInitDialog(HWND hWnd, HWND hwndFocus, LPARAM lParam)
{
    // Get and display whether the primary access token of the process 
    // belongs to user account that is a member of the local Administrators 
    // group even if it currently is not elevated (IsUserInAdminGroup).
    HWND hInAdminGroupLabel = GetDlgItem(hWnd, IDC_INADMINGROUP_STATIC);
    try
    {
        BOOL const fInAdminGroup = IsUserInAdminGroup();
        SetWindowText(hInAdminGroupLabel, fInAdminGroup ? TEXT("True") : TEXT("False"));
    }
    catch (DWORD dwError)
    {
        SetWindowText(hInAdminGroupLabel, TEXT("N/A"));
        ReportError(TEXT("IsUserInAdminGroup"), dwError);
    }

    // Get and display whether the process is run as administrator or not 
    // (IsRunAsAdmin).
    HWND hIsRunAsAdminLabel = GetDlgItem(hWnd, IDC_ISRUNASADMIN_STATIC);
    try
    {
        BOOL const fIsRunAsAdmin = IsRunAsAdmin();
        SetWindowText(hIsRunAsAdminLabel, fIsRunAsAdmin ? TEXT("True") : TEXT("False"));
    }
    catch (DWORD dwError)
    {
        SetWindowText(hIsRunAsAdminLabel, TEXT("N/A"));
        ReportError(TEXT("IsRunAsAdmin"), dwError);
    }
    
    // Get and display the process elevation information (IsProcessElevated) 
    // and integrity level (GetProcessIntegrityLevel). The information is not 
    // available on operating systems prior to Windows Vista.

    HWND	hIsElevatedLabel	= GetDlgItem(hWnd, IDC_ISELEVATED_STATIC),
			hILLabel			= GetDlgItem(hWnd, IDC_IL_STATIC);

    OSVERSIONINFO osver = { sizeof(osver) };
    if (GetVersionEx(&osver) && osver.dwMajorVersion >= 6)
    {
        // Running Windows Vista or later (major version >= 6).

        try
        {
            // Get and display the process elevation information.
            BOOL const fIsElevated = IsProcessElevated();
            SetWindowText(hIsElevatedLabel, fIsElevated ? TEXT("True") : TEXT("False"));

            // Update the Self-elevate button to show the UAC shield icon on 
            // the UI if the process is not elevated. The 
            // Button_SetElevationRequiredState macro (declared in Commctrl.h) 
            // is used to show or hide the shield icon in a button. You can 
            // also get the shield directly as an icon by calling 
            // SHGetStockIconInfo with SIID_SHIELD as the parameter.
            HWND hElevateBtn = GetDlgItem(hWnd, IDC_ELEVATE_BN);
            Button_SetElevationRequiredState(hElevateBtn, !fIsElevated);
        }
        catch (DWORD dwError)
        {
            SetWindowText(hIsElevatedLabel, TEXT("N/A"));
            ReportError(TEXT("IsProcessElevated"), dwError);
        }

        try
        {
            // Get and display the process integrity level.
            DWORD const dwIntegrityLevel = GetProcessIntegrityLevel();
            switch (dwIntegrityLevel)
            {
            case SECURITY_MANDATORY_UNTRUSTED_RID:	SetWindowText(hILLabel, TEXT("Untrusted"));	break;
            case SECURITY_MANDATORY_LOW_RID:		SetWindowText(hILLabel, TEXT("Low"));		break;
            case SECURITY_MANDATORY_MEDIUM_RID:		SetWindowText(hILLabel, TEXT("Medium"));	break;
            case SECURITY_MANDATORY_HIGH_RID:		SetWindowText(hILLabel, TEXT("High"));		break;
            case SECURITY_MANDATORY_SYSTEM_RID:		SetWindowText(hILLabel, TEXT("System"));	break;
            default:								SetWindowText(hILLabel, TEXT("Unknown"));	break;
            }
        }
        catch (DWORD dwError)
        {
            SetWindowText(hILLabel, TEXT("N/A"));
			ReportError(TEXT("GetProcessIntegrityLevel"), dwError);
        }
    }
    else
    {
        SetWindowText(hIsElevatedLabel,	TEXT("N/A"));
        SetWindowText(hILLabel,			TEXT("N/A"));
    }

    return TRUE;
}