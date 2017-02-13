#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <commctrl.h>
#include "statuswh.h"
#include "resource.h"
#include "screen.h"
#include "tab32.h"
#include "timeconv.h"
#include "qrscoran.h"
#include "hfiles.h"
#include "disk32.h"
#include "errsupp.h"
#include	"cdrfiles.h"
#include	"messgeh.h"
extern	"C"	STATUS	statinf;
extern	"C"	DWORD	f_desc_10000;
extern	"C"	void outDigit (DWORD value);
extern int AddToString(LPSTR strtoadd,LPSTR strthatadd,int maxstr=1024);
extern int CharToInerger(LPSTR lp,int MaxN);
extern	"C"	DWORD n_nes_qrs;
extern	"C"	void qrs_inform ();	//ASM call only 
extern	"C"	void Sample2QRS();	//ASM call only
extern		int CharToInerger(LPSTR lp,int MaxN);
extern	HINSTANCE	hInst;
extern	"C"		HWND	hwndSys;

TimeDayNight	daynight={'1','0','.','0','0','.','0','0','-','2','0','.','0','0','.','0','0',
							'$',0x0d,0x0a,
							 '0','0','.','0','0','.','0','0','-','0','8','.','0','0','.','0','0',
							'$',0x0d,0x0a,'0'};

DWORD	DayStartSec,DayStopSec,NightStartSec,NightStopSec;
DWORD	PathDayTest=0,PathNightTest=0;
//Htime	DayStart,DayStop,NightStart,NightStop;
void UpdateToTime()
{
	DayStartSec=(CharToInerger(daynight.startday.clock,2)*3600)+(CharToInerger(daynight.startday.min,2)*60)+CharToInerger(daynight.startday.sec,2);
	DayStopSec=(CharToInerger(daynight.endday.clock,2)*3600)+(CharToInerger(daynight.endday.min,2)*60)+CharToInerger(daynight.endday.sec,2);
	NightStartSec=(CharToInerger(daynight.startnight.clock,2)*3600)+(CharToInerger(daynight.startnight.min,2)*60)+CharToInerger(daynight.startnight.sec,2);
	NightStopSec=(CharToInerger(daynight.endnight.clock,2)*3600)+(CharToInerger(daynight.endnight.min,2)*60)+CharToInerger(daynight.endnight.sec,2);
	if(DayStopSec==DayStartSec)	{	PathDayTest=0;	}
	else	{
	if(DayStopSec>DayStartSec)	{	PathDayTest=1;	}
	else						{	PathDayTest=2;	}	}

	if(NightStopSec==NightStartSec)	{	PathNightTest=0;	}
	else	{
	if(NightStopSec>NightStartSec)	{	PathNightTest=1;	}
	else							{	PathNightTest=2;	}	}


}
void LoadAndUpdateToTime()
{	FileRead(timdn_file,&daynight,sizeof(daynight),NULL,NULL);
	FileRead(timdnc_file,&daynight,sizeof(daynight),NULL,NULL);
	UpdateToTime();	}

void SaveTimeDayNight()
{	FileCreate((LPSTR)&timdn_file);
	FileWright((LPSTR)&timdn_file,&daynight,sizeof(daynight),NULL,NULL);		}

void SaveDefaultTimeDayNight()
{	FileCreate((LPSTR)&timdnc_file);
	FileWright((LPSTR)&timdnc_file,&daynight,sizeof(daynight),NULL,NULL);		}

BOOL	IsItDay(DWORD Sample)
{	Htime	time;
	Sample2NativTime(Sample,&time);
	return IsItDay(&time);	}

BOOL	IsItDay(LPHtime lpt)
{	DWORD	timesec;
	timesec=(lpt->clock*3600)+(lpt->min*60)+lpt->sec;
	switch(PathDayTest)
{	case 0:	return TRUE;	break;
	case 1:	if(timesec<DayStartSec)		{	return FALSE;	}
			if(timesec<=DayStopSec)		{	return TRUE;	}
			return FALSE;			break;
	case 2:	if(timesec>=DayStartSec)		{	return TRUE;	}
			if(timesec<=DayStopSec)			{	return TRUE;	}
			return FALSE;			break;
	default:	return FALSE;		break;	}}

BOOL	IsItNight(DWORD Sample)
{	Htime	time;
	Sample2NativTime(Sample,&time);
	return IsItNight(&time);	}


BOOL	IsItNight(LPHtime lpt)
{	DWORD	timesec;
	timesec=(lpt->clock*3600)+(lpt->min*60)+lpt->sec;
	switch(PathNightTest)
{	case 0:	return TRUE;	break;
	case 1:	if(timesec<NightStartSec)		{	return FALSE;	}
			if(timesec<=NightStopSec)		{	return TRUE;	}
			return FALSE;			break;
	case 2:	if(timesec>=NightStartSec)		{	return TRUE;	}
			if(timesec<=NightStopSec)			{	return TRUE;	}
			return FALSE;			break;
	default:	return FALSE;		break;	}}




BOOL DlgProc_OnInitDialogTimeSet(HWND hdlg,HWND hwndFocus,LPARAM lParam);
BOOL DlgProc_OnCommandDialogTimeSet(HWND hdlg,int id,HWND hwndCtl,UINT codeNotify);
BOOL APIENTRY DialogTimeSet	(HWND hdlg,UINT msg,WPARAM wParam,LPARAM lParam)
{		switch (msg)
	{	HANDLE_MSG(hdlg,WM_INITDIALOG,DlgProc_OnInitDialogTimeSet);
		HANDLE_MSG(hdlg,WM_COMMAND,DlgProc_OnCommandDialogTimeSet);
		default:	break;	}
		return FALSE;	}


void DayNightSetup()
{	DialogBox(hInst,MAKEINTRESOURCE(IDD_DIALOG28),hwndSys,(DLGPROC)DialogTimeSet);	}


BOOL DlgProc_OnInitDialogTimeSet(HWND hdlg,HWND hwndFocus,LPARAM lParam)
{	SendMessage(GetDlgItem(hdlg,IDC_SPIN1),UDM_SETRANGE,0,MAKELONG(23,0));
	SendMessage(GetDlgItem(hdlg,IDC_SPIN1),UDM_SETPOS,0,MAKELONG((short)CharToInerger(daynight.startday.clock,2),0));
  
	SendMessage(GetDlgItem(hdlg,IDC_SPIN2),UDM_SETRANGE,0,MAKELONG(59,0));
	SendMessage(GetDlgItem(hdlg,IDC_SPIN2),UDM_SETPOS,0,MAKELONG((short)CharToInerger(daynight.startday.min,2),0));

	SendMessage(GetDlgItem(hdlg,IDC_SPIN10),UDM_SETRANGE,0,MAKELONG(59,0));
	SendMessage(GetDlgItem(hdlg,IDC_SPIN10),UDM_SETPOS,0,MAKELONG((short)CharToInerger(daynight.startday.sec,2),0));

	SendMessage(GetDlgItem(hdlg,IDC_SPIN3),UDM_SETRANGE,0,MAKELONG(23,0));
	SendMessage(GetDlgItem(hdlg,IDC_SPIN3),UDM_SETPOS,0,MAKELONG((short)CharToInerger(daynight.endday.clock,2),0));
  
	SendMessage(GetDlgItem(hdlg,IDC_SPIN6),UDM_SETRANGE,0,MAKELONG(59,0));
	SendMessage(GetDlgItem(hdlg,IDC_SPIN6),UDM_SETPOS,0,MAKELONG((short)CharToInerger(daynight.endday.min,2),0));

	SendMessage(GetDlgItem(hdlg,IDC_SPIN12),UDM_SETRANGE,0,MAKELONG(59,0));
	SendMessage(GetDlgItem(hdlg,IDC_SPIN12),UDM_SETPOS,0,MAKELONG((short)CharToInerger(daynight.endday.sec,2),0));



	SendMessage(GetDlgItem(hdlg,IDC_SPIN14),UDM_SETRANGE,0,MAKELONG(23,0));
	SendMessage(GetDlgItem(hdlg,IDC_SPIN14),UDM_SETPOS,0,MAKELONG((short)CharToInerger(daynight.startnight.clock,2),0));
  
	SendMessage(GetDlgItem(hdlg,IDC_SPIN15),UDM_SETRANGE,0,MAKELONG(59,0));
	SendMessage(GetDlgItem(hdlg,IDC_SPIN15),UDM_SETPOS,0,MAKELONG((short)CharToInerger(daynight.startnight.min,2),0));

	SendMessage(GetDlgItem(hdlg,IDC_SPIN16),UDM_SETRANGE,0,MAKELONG(59,0));
	SendMessage(GetDlgItem(hdlg,IDC_SPIN16),UDM_SETPOS,0,MAKELONG((short)CharToInerger(daynight.startnight.sec,2),0));

	SendMessage(GetDlgItem(hdlg,IDC_SPIN17),UDM_SETRANGE,0,MAKELONG(23,0));
	SendMessage(GetDlgItem(hdlg,IDC_SPIN17),UDM_SETPOS,0,MAKELONG((short)CharToInerger(daynight.endnight.clock,2),0));
  
	SendMessage(GetDlgItem(hdlg,IDC_SPIN18),UDM_SETRANGE,0,MAKELONG(59,0));
	SendMessage(GetDlgItem(hdlg,IDC_SPIN18),UDM_SETPOS,0,MAKELONG((short)CharToInerger(daynight.endnight.min,2),0));

	SendMessage(GetDlgItem(hdlg,IDC_SPIN19),UDM_SETRANGE,0,MAKELONG(59,0));
	SendMessage(GetDlgItem(hdlg,IDC_SPIN19),UDM_SETPOS,0,MAKELONG((short)CharToInerger(daynight.endnight.sec,2),0));

	return TRUE;		}

BOOL DlgProc_OnCommandDialogTimeSet(HWND hdlg,int id,HWND hwndCtl,UINT codeNotify)
{	char Text[8];	int i;
	switch (id)
{	case IDCANCEL:		EndDialog(hdlg,-1);	return	TRUE;		break;
	case IDOK:

	i=GetWindowText(GetDlgItem(hdlg,IDC_EDIT1),Text,sizeof(Text));		
	i=CharToInerger(Text,i);	if(i>23)	{	OutErrOk(TimeXError);	return TRUE;	}
	sprintf(Text,"%.2d",i);	memccpy(daynight.startday.clock,Text,0,2);

	i=GetWindowText(GetDlgItem(hdlg,IDC_EDIT2),Text,sizeof(Text));		
	i=CharToInerger(Text,i);	if(i>60)	{	OutErrOk(TimeXError);	return TRUE;	}
	sprintf(Text,"%.2d",i);	memccpy(daynight.startday.min,Text,0,2);


	i=GetWindowText(GetDlgItem(hdlg,IDC_EDIT12),Text,sizeof(Text));		
	i=CharToInerger(Text,i);	if(i>60)	{	OutErrOk(TimeXError);	return TRUE;	}
	sprintf(Text,"%.2d",i);	memccpy(daynight.startday.sec,Text,0,2);



	i=GetWindowText(GetDlgItem(hdlg,IDC_EDIT4),Text,sizeof(Text));		
	i=CharToInerger(Text,i);	if(i>23)	{	OutErrOk(TimeXError);	return TRUE;	}
	sprintf(Text,"%.2d",i);	memccpy(daynight.endday.clock,Text,0,2);

	i=GetWindowText(GetDlgItem(hdlg,IDC_EDIT6),Text,sizeof(Text));		
	i=CharToInerger(Text,i);	if(i>59)	{	OutErrOk(TimeXError);	return TRUE;	}
	sprintf(Text,"%.2d",i);	memccpy(daynight.endday.min,Text,0,2);

	i=GetWindowText(GetDlgItem(hdlg,IDC_EDIT24),Text,sizeof(Text));		
	i=CharToInerger(Text,i);	if(i>59)	{	OutErrOk(TimeXError);	return TRUE;	}
	sprintf(Text,"%.2d",i);	memccpy(daynight.endday.sec,Text,0,2);



	i=GetWindowText(GetDlgItem(hdlg,IDC_EDIT26),Text,sizeof(Text));		
	i=CharToInerger(Text,i);	if(i>23)	{	OutErrOk(TimeXError);	return TRUE;	}
	sprintf(Text,"%.2d",i);	memccpy(daynight.startnight.clock,Text,0,2);

	i=GetWindowText(GetDlgItem(hdlg,IDC_EDIT27),Text,sizeof(Text));		
	i=CharToInerger(Text,i);	if(i>59)	{	OutErrOk(TimeXError);	return TRUE;	}
	sprintf(Text,"%.2d",i);	memccpy(daynight.startnight.min,Text,0,2);


	i=GetWindowText(GetDlgItem(hdlg,IDC_EDIT28),Text,sizeof(Text));		
	i=CharToInerger(Text,i);	if(i>59)	{	OutErrOk(TimeXError);	return TRUE;	}
	sprintf(Text,"%.2d",i);	memccpy(daynight.startnight.sec,Text,0,2);



	i=GetWindowText(GetDlgItem(hdlg,IDC_EDIT29),Text,sizeof(Text));		
	i=CharToInerger(Text,i);	if(i>23)	{	OutErrOk(TimeXError);	return TRUE;	}
	sprintf(Text,"%.2d",i);	memccpy(daynight.endnight.clock,Text,0,2);

	i=GetWindowText(GetDlgItem(hdlg,IDC_EDIT30),Text,sizeof(Text));		
	i=CharToInerger(Text,i);	if(i>59)	{	OutErrOk(TimeXError);	return TRUE;	}
	sprintf(Text,"%.2d",i);	memccpy(daynight.endnight.min,Text,0,2);

	i=GetWindowText(GetDlgItem(hdlg,IDC_EDIT31),Text,sizeof(Text));		
	i=CharToInerger(Text,i);	if(i>59)	{	OutErrOk(TimeXError);	return TRUE;	}
	sprintf(Text,"%.2d",i);	memccpy(daynight.endnight.sec,Text,0,2);




	SaveTimeDayNight();
	SaveDefaultTimeDayNight();
	UpdateToTime();
	SendMessage(HWND_BROADCAST,HWM_DAYNIGHTCHANGED,0,0);
	EndDialog(hdlg,0);	return	TRUE;	break;	

	default:	return FALSE;	break;}	}




void Second2HTime(DWORD Second,LPHtime lptime)
{	lptime->day=(BYTE)(Second/(3600*24));
	Second=Second-(((DWORD)lptime->day)*(3600*24));
	lptime->clock=(BYTE)(Second/3600);
	Second=Second-(((DWORD)lptime->clock)*3600);
	lptime->min=(BYTE)(Second/60);
	lptime->sec=(BYTE)(Second-(((DWORD)lptime->min)*60));}

void Second2NativTime(DWORD Second,LPHtime lpnative)
{	Htime	ht;
	Second2HTime(Second,&ht);
	FromStart2NativTime(&ht,lpnative);}

void FromStart2NativTime(LPHtime lpstart,LPHtime lpnative)
{	BYTE adder=0;
	*lpnative=*lpstart;
	lpnative->min=lpnative->min+statinf.time_min_s;
	if (lpnative->min>=60)	{adder=1; lpnative->min=lpnative->min-60;}
	lpnative->clock=lpnative->clock+statinf.time_clock_s+adder;

	if (lpnative->clock>=24)	{lpnative->clock=lpnative->clock-24;
	lpnative->day=1;}	}

DWORD Sample2NativTime(DWORD N,LPHtime lpnative)
{	Htime htime={0,0,0,0};
	DWORD remainder=Samples2Time(N,&htime);
	FromStart2NativTime(&htime,lpnative);
	lpnative->SupDword2=N;
	return	remainder;}

DWORD NativTime2Sample	(LPHtime lpnative,DWORD msec)
{	DWORD result;
	double	v1000=1000.0,v10000=10000.0;
	int distsec=lpnative->sec
			+((lpnative->min-statinf.time_min_s)*60)
			+((lpnative->clock-statinf.time_clock_s)*3600)+
			+(lpnative->day*86400);
	_asm
	{		fild	msec
			fdiv	v1000

			fiadd	distsec
			fimul	f_desc_10000
			fdiv	v10000
			fistp	dword ptr result}
	return result;}

DWORD NativTime2SampleEx	(LPHtime lpnative,DWORD msec)
{	int center=lpnative->SupDword2;		//by sample
	if(center==0)
	{center=lpnative->SupDword;			//by qrs
	if(center!=0)
	{	n_nes_qrs=center;
	_asm	{	call qrs_inform
				jc AltCount
				mov eax,[esi]
				and eax,posqrsmask
				mov center,eax	}}
	else	{AltCount:;	center=NativTime2Sample(lpnative,msec);	}}
	return center;}

			
DWORD NativTime2Second	(LPHtime lpnative,DWORD msec)
{	return Sample2SecondC(NativTime2Sample	(lpnative,msec));	}



DWORD	Sample2SecondC	(DWORD i)
{		_asm	{	mov		eax,i
					call	Sample2Second
					mov i,eax	}
				return i;}

void	Sample2Second	()
{		_asm	{			mov edx,10000		//eax	-	samples
							mov ebx,f_desc_10000;
							mul edx
							push ecx
							cmp edx,ebx
							jb OkkTheTime
							sub eax,eax
							sub edx,edx
							mov ebx,1			
OkkTheTime:		
							mov ecx,ebx
							shr ecx,1
							div ebx				//eax	-	seconds
							cmp edx,ecx
							jl ni
							inc eax
ni:							pop ecx
}}

		
DWORD Samples2Time(DWORD sample,LPHtime lptime)
{	DWORD remainder;
_asm	{
	mov esi,lptime
	mov eax,sample
	mov edx,10000
	mul edx	
	mov ebx,f_desc_10000
	div ebx
	mov remainder,edx
	sub edx,edx
	mov ebx,3600*24
	div ebx
	mov [esi+3],al

	mov eax,edx
	sub edx,edx
	mov ebx,3600
	div ebx
	mov [esi+2],al
	
	mov eax,edx
	sub edx,edx
	mov ebx,60
	div ebx
	mov [esi+1],al
	mov [esi],dl		}
	return remainder/10;	}
		
void DrawTimeClockMin(HDC hdc,LPRECT rect,LPHtime lptime,long color,HGDIOBJ font)
{	char	timemes[6]="00.00";
	BYTE clock=lptime->clock, min=lptime->min;
_asm	{
	mov al,byte ptr clock
	sub ah,ah
	mov bh,10
	div bh
	add ax,'00'
	mov word ptr timemes,ax

	mov al,min
	sub ah,ah
	mov bh,10
	div bh
	add ax,'00'
	mov word ptr timemes+3,ax		}

	pict_in_Box	(hdc,(LPSTR)&timemes,rect,5,color,font);	}

void DrawTimeClockMinSec(HDC hdc,LPRECT rect,LPHtime lptime,long color,HGDIOBJ font)
{	char	timemes[9]="00.00.00";
	pict_in_Box	(hdc,(LPSTR)&timemes,rect,MakeHTimeString(lptime,(LPSTR)&timemes),color,font);	}


int MakeHTimeString(LPHtime lptime,LPSTR lpstr)
{	BYTE clock=lptime->clock, min=lptime->min,sec=lptime->sec;
_asm	{
	mov edi,lpstr
	mov al,byte ptr clock
	sub ah,ah
	mov bh,10
	div bh
	add ax,'00'
	mov word ptr [edi],ax
	mov byte ptr [edi+2],'.'

	mov al,min
	sub ah,ah
	mov bh,10
	div bh
	add ax,'00'
	mov word ptr [edi+3],ax
	mov byte ptr [edi+5],'.'

	mov al,sec
	sub ah,ah
	mov bh,10
	div bh
	add ax,'00'
	mov word ptr [edi+6],ax
	mov byte ptr [edi+8],0
	}
	return 8;}


extern	char Month_th[13][16];
extern	char of_digit[],of_year[];

int MakeSuperDateString(int day,int month,int year,LPSTR lpstr)
{	if (day<0)			{day=0;}
	if (day>31)			{day=0;}
	if (month<1)		{month=0;}
	if (month>12)		{month=0;}
	int i=0;
	i=sprintf(lpstr+i,"%d-",day);
	i+=AddToString(lpstr+i,of_digit);
	i+=AddToString(lpstr+i," ");
	i+=AddToString(lpstr+i,(LPSTR)&Month_th[month][0]);
	i+=AddToString(lpstr+i," ");
	i+=sprintf(lpstr+i,"%d-",year);
	i+=AddToString(lpstr+i,of_year);
	return i;	}
	
int MakeStandartDateString(int day,int month,int year,LPSTR lpstr)
{	if (day<0)			{day=0;}
	if (day>31)			{day=0;}
	if (month<1)		{month=0;}
	if (month>12)		{month=0;}
	int i=0;
	if(day<10)	{	i+=sprintf(lpstr+i,"0");	}
	i+=sprintf(lpstr+i,"%d.",day);
	if(month<10)	{	i+=sprintf(lpstr+i,"0");	}
	i+=sprintf(lpstr+i,"%d.",month);
	i+=sprintf(lpstr+i,"%d",year);
	return i;	}

BOOL ReadDate(LPSTR lp,LPBYTE lpday,LPBYTE lpmonth,LPBYTE year_low,LPBYTE year_hi,int maxsize)
{	int k,i,day,month,year;
//seach first point

	for(i=0;i<maxsize;i++)
	{	if(*(lp+i)==0)	{return FALSE;}

	if(*(lp+i)=='.')	{goto point1found;}}	return FALSE;

point1found:;
	day=CharToInerger(lp,i);

	if (day>31)		{	return FALSE;	}

	i+=1;
	k=i;
//seach first point
	for(i;i<maxsize;i++)
	{	if(*(lp+i)==0)	{return FALSE;}
	if(*(lp+i)=='.')	{goto point2found;}}	return FALSE;

point2found:;
	month=CharToInerger(lp+k,i-k);
	if (month>12)		{	return FALSE;	}
	i+=1;
	year=CharToInerger(lp+i,maxsize-i);	
	*lpday=(BYTE)day;	*lpmonth=(BYTE)month;
_asm	{
		mov eax,year
		mov esi,year_low
		mov [esi],al
		mov esi,year_hi
		mov [esi],ah	}	return TRUE;}
BOOL	Sample2QRSc(DWORD sample)
{	_asm	{
	mov edi,sample
	call Sample2QRS
	jc PassQRS	}
	return TRUE;
PassQRS:;	return FALSE;	}


DWORD	GetLastTimeQrs()
{	LPQRSinf	Last;
	n_nes_qrs=max_n_qrs-1;
	
	_asm	{
		call qrs_inform
		jc passqrsinform
		mov Last,esi	}
		return Last->position;

passqrsinform:;
			  return -1;		}


