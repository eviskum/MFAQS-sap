// SAPEVENT.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "SAPEVENT.h"
#include <afxtempl.h>
#include "crfclass.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

/////////////////////////////////////////////////////////////////////////////
// CSAPEVENTApp

BEGIN_MESSAGE_MAP(CSAPEVENTApp, CWinApp)
	//{{AFX_MSG_MAP(CSAPEVENTApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSAPEVENTApp construction

CSAPEVENTApp::CSAPEVENTApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CSAPEVENTApp object

CSAPEVENTApp theApp;

#define DLLSUCCES	1
#define DLLERROR	2
#define LOGNORMAL	1
#define LOGERROR	2
#define LNORM( txt )	Log(txt, LOGNORMAL, FALSE);
#define LERR( txt )		Log(txt, LOGERROR, FALSE);
#define LJNORM( txt )	Log(txt, LOGNORMAL, TRUE);
#define LJERR( txt )	Log(txt, LOGERROR, TRUE);

#define ERROR 0
#define SUCCES 1

void Logon (CRfcConnection& Connection, LPCTSTR sapsys, LPCTSTR sapclnt);
void DumpErrorInfo (RFC_ERROR_INFO_EX & err);
int CallSap(LPCTSTR datafile);


HANDLE hLogEvent, hLogReady;
TCHAR log_txt[500];
int log_mode, log_job;
int filenumber = 1;

void Log(LPCTSTR txt, int mode, int job) {
	WaitForSingleObject(hLogReady, INFINITE);
	strcpy(log_txt, txt);
	log_mode = mode;
	log_job = job;
	SetEvent(hLogEvent);
}

extern "C" int PASCAL EXPORT DllGetLog(LPTSTR txt, int *mode, int *job)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	strcpy(txt, log_txt);
	*mode = log_mode;
	*job = log_job;
	return(0);
}

extern "C" int PASCAL EXPORT DllLogReg( HANDLE set_event, HANDLE set_ready )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	hLogEvent = set_event;
	hLogReady = set_ready;
	return(0);
}

int nodelete = TRUE;

extern "C" int PASCAL EXPORT DllWorker ( int argc, char *argv[] )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	LJNORM("SAPEVENT utility")

	if (argc != 1 && argc != 2) {
		LJERR("Forkert brug !!!! Korrekt brug: SAPEVENT trigfile <NODELETE>")
		return(DLLERROR);
	}

	if (argc == 1) nodelete = FALSE;
	else nodelete = TRUE;

	int retval = DLLERROR;
	Sleep(10000);
	if (CallSap((LPCTSTR) argv[0]) == SUCCES) retval = DLLSUCCES;

	return(retval);
}

RFC_FIELD_INFO FieldsBDC_T[] = {{"PROGRAM", 0, 0,   TYPC, 40, 0},
								{"DYNPRO", 1, 40,  TYPNUM, 4, 0},
								{"DYNBEGIN", 2, 44,  TYPC, 1, 0},
								{"FNAM", 3, 45,  TYPC, 132, 0},
								{"FVAL", 4, 177,  TYPC, 132, 0}} ;


RFC_FIELD_INFO FieldsMSG_S[] = {{"MSGTY", 0, 0,   TYPC, 1, 0},
								{"MSGID", 1, 1,  TYPC, 20, 0},
								{"MSGNO", 2, 21,  TYPC, 3, 0},
								{"MSSPC", 3, 24,  TYPC, 1, 0},
								{"MSGTX", 4, 25,  TYPC, 255, 0}} ;

class CRfcUser : public CRfcClientFunc
{
	//Constructor and destructor
public:
	CRfcUser  (CRfcConnection* pConnection);
	~CRfcUser (void) ;
	
	//Attributes
private:
	CRfcTableParam  *m_pTabBDC;
	CRfcSimpleParam *m_pSimTRANS;
	CRfcSimpleParam *m_pSimUPD;
	CRfcStructParam *m_pStrMSG;

	void ClearParams () ;

public:
	void SetEvent (LPCTSTR event, LPCTSTR parm) ;
	void PrintMsg();

};

CRfcUser::CRfcUser(CRfcConnection* pConnection)
		:CRfcClientFunc(pConnection, "RFC_CALL_TRANSACTION")   
{
	m_pTabBDC = NULL;   
	m_pSimTRANS = NULL;   
	m_pSimUPD = NULL;
	m_pStrMSG = NULL;

	try
	{
		m_pSimTRANS		= new CRfcSimpleParam ("TRANCODE", TYPC, 20) ;
        m_pSimUPD		= new CRfcSimpleParam ("UPDMODE", TYPC, 1) ;
		m_pStrMSG		= new CRfcStructParam ("MESSG", 280);
		m_pTabBDC		= new CRfcTableParam  ("BDCTABLE", 309) ;

		m_pTabBDC->AddFieldInfo (FieldsBDC_T, 
				sizeof(FieldsBDC_T) / sizeof(RFC_FIELD_INFO)) ;

		m_pStrMSG->AddFieldInfo (FieldsMSG_S, 
				sizeof(FieldsMSG_S) / sizeof(RFC_FIELD_INFO)) ;

		AddImportParam (*m_pSimTRANS);
		AddImportParam (*m_pSimUPD);
		AddExportParam (*m_pStrMSG);
		AddTableParam  (*m_pTabBDC);
		m_pTabBDC->Create();
	}
	catch (char*)  // catch memory 
	{ 
		ClearParams();
		throw; 
	}
}

void CRfcUser::ClearParams()
{
    if (m_pSimTRANS != NULL)		delete m_pSimTRANS ;
    if (m_pSimUPD != NULL)			delete m_pSimUPD ;
    if (m_pStrMSG != NULL)			delete m_pStrMSG ;
    if (m_pTabBDC != NULL)			delete m_pTabBDC ;
	m_pSimTRANS = NULL;
	m_pSimUPD = NULL;
	m_pStrMSG = NULL;
	m_pTabBDC = NULL;
}


CRfcUser::~CRfcUser()
{
   ClearParams() ;
}


void CRfcUser::SetEvent (LPCTSTR event, LPCTSTR parm)
{
	m_pSimTRANS->Value() = "SM64";
	m_pSimUPD->Value() = "X";

	m_pTabBDC->Clear();
	m_pTabBDC->AppendInitializedRow();
	m_pTabBDC->Field(0) = "SAPLBTCH"; m_pTabBDC->Field(1) = "1250"; m_pTabBDC->Field(2) = "X";
	m_pTabBDC->Field(3) = "BDC_CURSOR"; m_pTabBDC->Field(4) = "BTCH1250-PARAMETER";
	m_pTabBDC->AppendInitializedRow();
	m_pTabBDC->Field(0) = "SAPLBTCH"; m_pTabBDC->Field(1) = "1250"; m_pTabBDC->Field(2) = " ";
	m_pTabBDC->Field(3) = "BDC_OKCODE"; m_pTabBDC->Field(4) = "=RAIS";
	m_pTabBDC->AppendInitializedRow();
	m_pTabBDC->Field(0) = "SAPLBTCH"; m_pTabBDC->Field(1) = "1250"; m_pTabBDC->Field(2) = " ";
	m_pTabBDC->Field(3) = "BTCH1250-EVENTID"; m_pTabBDC->Field(4) = event;
	m_pTabBDC->AppendInitializedRow();
	m_pTabBDC->Field(0) = "SAPLBTCH"; m_pTabBDC->Field(1) = "1250"; m_pTabBDC->Field(2) = " ";
	m_pTabBDC->Field(3) = "BTCH1250-PARAMETER"; m_pTabBDC->Field(4) = parm;


	m_pTabBDC->AppendInitializedRow();
	m_pTabBDC->Field(0) = "SAPLBTCH"; m_pTabBDC->Field(1) = "1250"; m_pTabBDC->Field(2) = "X";
	m_pTabBDC->Field(3) = "BDC_CURSOR"; m_pTabBDC->Field(4) = "BTCH1250-PARAMETER";
	m_pTabBDC->AppendInitializedRow();
	m_pTabBDC->Field(0) = "SAPLBTCH"; m_pTabBDC->Field(1) = "1250"; m_pTabBDC->Field(2) = " ";
	m_pTabBDC->Field(3) = "BDC_OKCODE"; m_pTabBDC->Field(4) = "=BACK";
	m_pTabBDC->AppendInitializedRow();
	m_pTabBDC->Field(0) = "SAPLBTCH"; m_pTabBDC->Field(1) = "1250"; m_pTabBDC->Field(2) = " ";
	m_pTabBDC->Field(3) = "BTCH1250-EVENTID"; m_pTabBDC->Field(4) = event;
	m_pTabBDC->AppendInitializedRow();
	m_pTabBDC->Field(0) = "SAPLBTCH"; m_pTabBDC->Field(1) = "1250"; m_pTabBDC->Field(2) = " ";
	m_pTabBDC->Field(3) = "BTCH1250-PARAMETER"; m_pTabBDC->Field(4) = parm;
}

void CRfcUser::PrintMsg() {
	char tmp[1024];
	strcpy(tmp, (* m_pStrMSG)[0]);
	strcat(tmp, " ");
	strcat(tmp, (* m_pStrMSG)[1]);
	strcat(tmp, " ");
	strcat(tmp, (* m_pStrMSG)[2]);
	strcat(tmp, " ");
	strcat(tmp, (* m_pStrMSG)[3]);
	strcat(tmp, " ");
	strcat(tmp, (* m_pStrMSG)[4]);
	LJNORM(tmp)
}




int CallSap(LPCTSTR datafile) {
	CRfcConnection   Connection ;	//Connection object
	char*            pException ;	//RFC call exceptions
    RFC_RC           rc;			//RFC return code
	int              status;

	
	CString sapsys, sapclnt, event, parm;
	CStdioFile cfile;
	CString txt;

	if (cfile.Open(datafile, CFile::modeRead | CFile::shareDenyWrite) == 0) {
		status = ERROR;
	} else {
		status = ERROR;
		if (cfile.ReadString(sapsys))
			if (cfile.ReadString(sapclnt))
				if (cfile.ReadString(event))
					if (cfile.ReadString(parm)) status = SUCCES;
		cfile.Close();
	}
	if (status == ERROR) return(ERROR);

	LJNORM("SAPEVENT: Trig-fil er indlaest og vi kalder SAP")

    CRfcUser    MyFunc(&Connection);

	try {
		Logon (Connection, (LPCTSTR) sapsys, (LPCTSTR) sapclnt) ;
	} catch (RFC_ERROR_INFO_EX err) {
		DumpErrorInfo (err) ;
		return ERROR;
	}

	LJNORM("SAPEVENT: Vi er nu logget paa SAP")

	MyFunc.SetEvent(event, parm);

    try {
       rc = MyFunc.CallReceive (pException) ;
    }
    catch (RFC_ERROR_INFO_EX err) {
        DumpErrorInfo (err) ;
		return ERROR;
    }

	int retval = ERROR;
    if (rc != RFC_OK) {
		TCHAR tmpstr[1024];
		sprintf(tmpstr, "SAPEVENT: RFC ret-code <> 0, %s", pException);
		LJERR(tmpstr)
    } else {
		MyFunc.PrintMsg();
		LJNORM("SAPEVENT: SAP er kaldt med succes")
		CString tmpstr(datafile);
		tmpstr.MakeUpper();
		if (nodelete == FALSE) { if (DeleteFile(datafile)) retval = SUCCES; }
		else retval = SUCCES;
	}

	Connection.Clear();

	return retval;
}

void Logon (CRfcConnection& Connection, LPCTSTR sapsys, LPCTSTR sapclnt) 
{
	RFC_USER_INFO    UserInfo;     //All user login info
	RFC_CONNECT_INFO ConnectInfo;  //R/3 system connection info
	char InputBuffer[MAX_LEN]; 


	strcpy(InputBuffer, sapsys);
	ConnectInfo.rstrDestination = InputBuffer  ;

	strcpy(InputBuffer, sapclnt);
	UserInfo.rstrClient = InputBuffer  ;

	strcpy(InputBuffer, "sysrfc");
	UserInfo.rstrUserName = InputBuffer ;

	strcpy(InputBuffer, "wazx88");
	UserInfo.rstrPassword = InputBuffer ;
    
	UserInfo.rstrLanguage = "DA";

	ConnectInfo.rfcMode             =  RFC_MODE_PARAMETER ;           
    
	Connection.SetUserInfo    (UserInfo);
	Connection.SetConnectInfo (ConnectInfo);
	Connection.SetTraceLevel  (0);

	Connection.SafeOpen () ;
}

void DumpErrorInfo (RFC_ERROR_INFO_EX & err)
{
	TCHAR strtmp[2048];
	char grpName[128];
	ErrorGroupName(err.group, grpName);
	strcpy(strtmp, "RFC Error:\n");
	strcat(strtmp, "Error Group: "); strcat(strtmp, grpName);
	strcat(strtmp, "\nKey: "); strcat(strtmp, err.key);
	strcat(strtmp, "\nMessage: "); strcat(strtmp, err.message);
	LJERR(strtmp)
}
