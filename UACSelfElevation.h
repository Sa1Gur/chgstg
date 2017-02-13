BOOL IsUserInAdminGroup();
BOOL IsRunAsAdmin();
BOOL IsProcessElevated();
DWORD GetProcessIntegrityLevel();
void ReportError(LPCTSTR pszFunction, DWORD dwError = GetLastError());