//N of bit for SYS dword (stest.cpp)
#define HRV_option	0
//#define _option	1
#define xHRV_option		2
#define PSM_option		3
#define DB_option		4
#define LP_option		5
#define Scient_option	6
#define QT_option		7
#define xST_option		8

#define lTest_option	9	//Lead Imposition Test
#define ADTest_option	10	//AD analysis option
#define Elite_option	11	//It Draws Elite Picture instead of Expert
#define P_Extr_option	12	//Made for bakulevsk.
#define BPLAB_option	13	//BPLab Recoder support

#define SinglBP_option	14	//BPLab Recoder without ECG support


#define ShHrv_option	15	//Extended HRV fo SF
#define DebugInf_option	16	//Additional Debug Information (not to user)
#define MTWA_option		17	//MTWA

#define BLOCKW_option	28	//Prevent from using driver and recoders (used for workstation)
#define LightPic_option	29	//Light picture only version ( for special purpose, it incompatible with DOS sys bits)
#define WinStand_option	30	//Stand only version 
#define WinLight_option	31	//Light only version (it incompatible with DOS sys bits)
BOOL TestOnSYSbit(int bit);

#define CallIfOk(bit,proc)	if(TestOnSYSbit(bit)==FALSE)	{OutInfOk(NomdError);}	else	{proc;}


//Expert is default version!

//**************Light Version Limitation (in comparison with Expert )****************
//1. Supports only 2-chan standart ECG records.
//2. No Other Option !!! such as HRV, QT etc.
//3. Start Picture is Light Picture.


//**************Standart Version Limitation (in comparison with Expert )****************
//1. Not supports 12-chan  and BP_ECG records.
//2. Start Picture is Standart Picture


//************NON HOLTER SYS bits
#define ExEFI_option		1		// CHPS programm for stimulation