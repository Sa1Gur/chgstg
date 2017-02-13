#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include "disk32.h"
#include "statuswh.h"
#include "efi32.h"
#include "errsupp.h"
#include "screen.h"
#include "ssesppt.h"
#include "syslist.h"
extern	HINSTANCE hInst;
extern	"C" void outDigit (DWORD value);
extern		int CharToInerger(LPSTR lp,int MaxN);
LPBYTE	SeachForString(LPBYTE start,LPBYTE end,LPSTR string,int size);
int CountSRC(LPBYTE frombyte,int N);
extern char	Hospital[];
extern char	SerialNumber[];
extern int adc_res;
extern	"C"	BYTE	FastDma;
extern	"C"	DWORD	HDH_BUS;
extern	"C"	DWORD	CS_BUS;
extern	"C"	DWORD	UseVXDPhys;
extern	"C"	DWORD	ReqBoardSN;
DWORD	ReqBoardSN=-1;
DWORD	SYS=0;
DWORD	RegEnableMask=1;
DWORD	cdrv=2;
DWORD	PMSNR=0;	//default
DWORD	SetkaBlock=0;
DWORD	PredefinedCash=0;
DWORD	DriverTestQuerry=0;
BYTE	SynMode=0;
BYTE	SinglUndo=0;
BYTE	Blockw=FALSE;
BYTE	FastDma=0;
DWORD	HDH_BUS=0;
DWORD	CS_BUS=0;
DWORD	UseVXDPhys=0;
BOOL	hmwtest()
{	BYTE hmbuff[2048];	
	DWORD readdat[2];
	LPBYTE StartOfUser,StartOfSYS,StartOfSN,StartOfSRC,StartOfADC,StartOfAny,StartOfSNB;
	for (int i=0;i<sizeof(hmbuff);i++)	{hmbuff[i]=0;}

	if (FileRead("hmw.ini",(LPVOID)&hmbuff,sizeof(hmbuff),0,&readdat[0])==FALSE)
	{goto ExitFatalError;}
	
	StartOfUser=SeachForString(&hmbuff[0],&hmbuff[0]+sizeof(hmbuff),"USER:",5);

	StartOfSYS=SeachForString(&hmbuff[0],&hmbuff[0]+sizeof(hmbuff),"SYS:",4);
	StartOfSN=SeachForString(&hmbuff[0],&hmbuff[0]+sizeof(hmbuff),"SN:",3);
	StartOfSRC=SeachForString(&hmbuff[0],&hmbuff[0]+sizeof(hmbuff),"SRC:",4);
	StartOfSNB=SeachForString(&hmbuff[0],&hmbuff[0]+sizeof(hmbuff),"SNB:",4);
	
	if(StartOfSYS<StartOfUser)		{	goto ExitFatalError;	}
	if(StartOfSN<StartOfUser)		{	goto ExitFatalError;	}
	if(StartOfSRC<StartOfUser)		{	goto ExitFatalError;	}
	if(StartOfSNB>0)
	{	ReqBoardSN=CharToInerger((LPSTR)(StartOfSNB+4),10);
		if(StartOfSNB<StartOfUser)		{	goto ExitFatalError;	}}
	
	if((int)StartOfUser*(int)StartOfSYS*(int)StartOfSN*(int)StartOfSRC==0)	{	goto ExitFatalError;	}
	if(CountSRC(StartOfUser,StartOfSRC-StartOfUser)!=CharToInerger((LPSTR)(StartOfSRC+4),6))
	{
		
ExitFatalError:;	
	OutErrOk(NminidError);		return FALSE;	}

	StartOfAny=SeachForString(&hmbuff[0],&hmbuff[0]+sizeof(hmbuff),"SynkM:",6);
	if(StartOfAny!=0)	{SynMode=(BYTE)CharToInerger((LPSTR)(StartOfAny+6),2);}

	StartOfAny=SeachForString(&hmbuff[0],&hmbuff[0]+sizeof(hmbuff),"RTYPE:",6);
	if(StartOfAny!=0)	{RegEnableMask=(BYTE)CharToInerger((LPSTR)(StartOfAny+6),6);}

	StartOfAny=SeachForString(&hmbuff[0],&hmbuff[0]+sizeof(hmbuff),"UNDOSNG:",8);
	if(StartOfAny!=0)	{SinglUndo=(BYTE)CharToInerger((LPSTR)(StartOfAny+8),6);}

	StartOfAny=SeachForString(&hmbuff[0],&hmbuff[0]+sizeof(hmbuff),"CDRV:",5);
	if(StartOfAny!=0)	{cdrv=CharToInerger((LPSTR)(StartOfAny+5),3);		}
	
	StartOfAny=SeachForString(&hmbuff[0],&hmbuff[0]+sizeof(hmbuff),"PMSNR:",6);
	if(StartOfAny!=0)	{PMSNR=CharToInerger((LPSTR)(StartOfAny+6),6);		}

	StartOfAny=SeachForString(&hmbuff[0],&hmbuff[0]+sizeof(hmbuff),"GRIDBLOCK:",10);
	if(StartOfAny!=0)	{SetkaBlock=CharToInerger((LPSTR)(StartOfAny+10),6);		}

	StartOfAny=SeachForString(&hmbuff[0],&hmbuff[0]+sizeof(hmbuff),"CASHSIZE:",9);
	if(StartOfAny!=0)	{PredefinedCash=CharToInerger((LPSTR)(StartOfAny+9),6);		}

	StartOfAny=SeachForString(&hmbuff[0],&hmbuff[0]+sizeof(hmbuff),"RETRMODE:",9);
	if(StartOfAny!=0)	{retrmode=CharToInerger((LPSTR)(StartOfAny+9),6);		}

	StartOfAny=SeachForString(&hmbuff[0],&hmbuff[0]+sizeof(hmbuff),"BLOCKW",6);
	if(StartOfAny!=0)	{	Blockw=TRUE;		}

	StartOfAny=SeachForString(&hmbuff[0],&hmbuff[0]+sizeof(hmbuff),"FASTDMA:",8);
	if(StartOfAny!=0)	{	FastDma=CharToInerger((LPSTR)(StartOfAny+8),6);		}

	StartOfAny=SeachForString(&hmbuff[0],&hmbuff[0]+sizeof(hmbuff),"HDHBUS:",7);
	if(StartOfAny!=0)	{	HDH_BUS=CharToInerger((LPSTR)(StartOfAny+7),6);		}

	StartOfAny=SeachForString(&hmbuff[0],&hmbuff[0]+sizeof(hmbuff),"CSBUS:",6);
	if(StartOfAny!=0)	{	CS_BUS=CharToInerger((LPSTR)(StartOfAny+6),6);		}

	StartOfAny=SeachForString(&hmbuff[0],&hmbuff[0]+sizeof(hmbuff),"USEVXDP",7);
	if(StartOfAny!=0)	{	UseVXDPhys=TRUE;		}

	StartOfAny=SeachForString(&hmbuff[0],&hmbuff[0]+sizeof(hmbuff),"USESSE:",7);
	if(StartOfAny!=0)	{	UseSSE=CharToInerger((LPSTR)(StartOfAny+7),7);	}



	StartOfAny=SeachForString(&hmbuff[0],&hmbuff[0]+sizeof(hmbuff),"DTQ:",4);
	if(StartOfAny!=0)	{	DriverTestQuerry=CharToInerger((LPSTR)(StartOfAny+4),6);		}


//Load Hospital
	int k=0;
	for (i=(int)StartOfUser;i<(int)(&hmbuff[0]+sizeof(hmbuff)-1);i++)
	{	if (*(LPBYTE)i=='|')	goto ReadHospital;}	goto PassHospital;
ReadHospital:;	i+=1;
		for (i;i<(int)(&hmbuff[0]+sizeof(hmbuff)-1);i++)
	{	Hospital[k]=*(LPBYTE)i;		if(Hospital[k]=='$')	{	goto PassHospital;	}	k+=1;}
PassHospital:;	Hospital[k]=0;		OemToChar((LPSTR)&Hospital,(LPSTR)&Hospital);

//Load Serial Number
	k=0;
	for (i=(int)StartOfSN;i<(int)(&hmbuff[0]+sizeof(hmbuff)-1);i++)
	{	if (*(LPBYTE)i=='#')	goto ReadSN;}	goto PassSN;
ReadSN:;	i+=1;
		for (i;i<(int)(&hmbuff[0]+sizeof(hmbuff)-1);i++)
	{	SerialNumber[k]=*(LPBYTE)i;	if(SerialNumber[k]=='$')	{	goto PassSN;	}	k+=1;}
PassSN:;	SerialNumber[k]=0;		OemToChar((LPSTR)&SerialNumber,(LPSTR)&SerialNumber);

	StartOfADC=SeachForString(&hmbuff[0],&hmbuff[0]+sizeof(hmbuff),"ADC_R:",6);
	if(StartOfADC!=NULL)	{	adc_res=CharToInerger((LPSTR)(StartOfADC+6),3)-10;	}
	SYS=CharToInerger((LPSTR)(StartOfSYS+4),10);


	int sum=TestOnSYSbit(Elite_option)+TestOnSYSbit(WinStand_option)+TestOnSYSbit(WinLight_option);
	if(sum>1)			{	return FALSE;		}

	return TRUE;			}

BOOL TestOnSYSbit(int bit)
{	_asm	{	mov eax,bit
				bt SYS,eax
				jc Ok		}
				return FALSE;
Ok:;			return TRUE;}



int CountSRC(LPBYTE frombyte,int N)
{	int srcx;
	_asm	{
	mov esi,frombyte
	mov ecx,N
	sub ebx,ebx
	sub edx,edx	
count_s:;
	mov al,[esi]
	shl al,1
	jc loc1
	inc ebx
	dec edx
loc1:;
	inc edx
	shl al,1
	jc loc2
	inc ebx
	dec edx
loc2:;
	inc edx
	shl al,1
	jc loc3
	inc ebx
	dec edx
loc3:;
	inc edx
	shl al,1
	jc loc4
	inc ebx
	dec edx
loc4:;
	inc edx
	shl al,1
	jc loc5
	inc ebx
	dec edx
loc5:;
	inc edx
	shl al,1
	jc loc6
	inc ebx
	dec edx
loc6:;
	inc edx
	shl al,1
	jc loc7
	inc ebx
	dec edx
loc7:;
	inc edx
	shl al,1
	jc loc8
	inc ebx
	dec edx
loc8:;
	inc edx
	inc si
	loop count_s
	movzx eax,bx
	movzx ebx,dx
	mul ebx
	mov srcx,eax	}
	return srcx;		}



LPBYTE	SeachForString(LPBYTE start,LPBYTE end,LPSTR string,int size)
{	int i,k=0;
nextk:;	for (i=0;i<size;i++)
	{	if(*(start+k+i)!=*(string+i))	{goto NextByte;}}
	return start+k;
NextByte:;	if (start+k+size>=end)	{	return NULL;	}
		k+=1;	goto nextk;	}