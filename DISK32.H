extern "C" HANDLE FileOpen (LPCTSTR);
extern "C" BOOL FileWright(LPCTSTR path,LPVOID buffer,DWORD limit,LPDWORD locres,DWORD start);
extern "C" BOOL FileWrightX(LPCTSTR path,LPVOID buffer,DWORD limit,LPDWORD locres,DWORD start);
extern "C" BOOL FileRead(LPCTSTR,LPVOID,DWORD,long,LPDWORD);
extern "C" DWORD File2Mem(LPCTSTR lpFileName,DWORD a_file,
	DWORD n_Byte);
extern "C" BOOL	OffFile2Mem(DWORD addr);
extern "C" void	OffAllFile2Mem ();
extern "C" BOOL FileCreate(LPCTSTR path);	
extern "C" DWORD DiskSize(int Disk);
extern "C" BOOL FileCopy(LPCTSTR pathFrom,LPCTSTR pathTo);
extern "C" BOOL CorrectToWright(LPSTR path);
extern "C" void CorrectThePath(LPSTR pathFrom,LPSTR pathTo);