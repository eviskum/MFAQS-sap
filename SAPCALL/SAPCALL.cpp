// SAPCALL.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "SAPCALL.h"
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
// CSAPCALLApp

BEGIN_MESSAGE_MAP(CSAPCALLApp, CWinApp)
	//{{AFX_MSG_MAP(CSAPCALLApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSAPCALLApp construction

CSAPCALLApp::CSAPCALLApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CSAPCALLApp object

CSAPCALLApp theApp;

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

void Logon (CRfcConnection& Connection, LPCTSTR filename);
void DumpErrorInfo (RFC_ERROR_INFO_EX & err);
int CallSap(LPCTSTR datafile, LPCTSTR filenopath, int argc, char *argv[]);


HANDLE hLogEvent, hLogReady;
TCHAR log_txt[500];
int log_mode, log_job;
int filenumber = 1;
int seq_no = 0;

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

extern "C" int PASCAL EXPORT DllWorker ( int argc, char *argv[] )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	LJNORM("SAP CALL utility")

	if (argc != 1 && argc != 2 && argc != 3) {
		LJERR("Forkert brug !!!! Korrekt brug: SAPCALL fil <filekopi> <RENAME>")
		return(DLLERROR);
	}

	if (argc == 3 && strcmp(argv[2], "RENAME") != 0) {
		LJERR("Forkert brug !!!! Korrekt brug: SAPCALL fil <filekopi> <RENAME>")
		return(DLLERROR);
	}

	int retval = DLLERROR;
	CString filename, filenopath;

	CFileFind filefind;
	if (filefind.FindFile(argv[0])) {
		filefind.FindNextFile();
		filename = filefind.GetFilePath();
		filenopath = filefind.GetFileTitle();
		Sleep(10000);
		if (CallSap((LPCTSTR) filename, (LPCTSTR) filenopath, argc, argv) == SUCCES) retval = DLLSUCCES;
	}
	filefind.Close();

	return(retval);
}

RFC_FIELD_INFO typeOfTAB2048[] = {
  {"WA", 0, 0, TYPC, 2048, 0},
};

class CRfcUser : public CRfcClientFunc
{
	//Constructor and destructor
public:
	CRfcUser  (CRfcConnection* pConnection, LPCTSTR filename, CArray<CString, CString> &txtfile);
	~CRfcUser (void) ;
	
	//Attributes
private:
	CRfcSimpleParam *m_pFILENAME;

	void ClearParams () ;

public:
	CRfcTableParam *m_pDATA;

};

CRfcUser::CRfcUser(CRfcConnection* pConnection, LPCTSTR filename, CArray<CString, CString> &txtfile)
		:CRfcClientFunc(pConnection, "Z_CICS2SAP")   
{
	m_pFILENAME = NULL;
	m_pDATA = NULL;

	try
	{
		m_pFILENAME = new CRfcSimpleParam("FILENAME", TYPC, 128);
		m_pDATA = new CRfcTableParam ("DATA", 2048);
		m_pDATA->AddFieldInfo (typeOfTAB2048,
				sizeof(typeOfTAB2048) / sizeof(RFC_FIELD_INFO)) ;

		AddImportParam (*m_pFILENAME);
		AddTableParam (*m_pDATA);
		m_pDATA->Create();

		m_pFILENAME->Value() = filename;

		for (int i = 0; i < txtfile.GetSize(); i++) {
			m_pDATA->AppendInitializedRow();
			m_pDATA->Field("WA") = (LPCTSTR) txtfile[i];
		}
	}
	catch (char*)  // catch memory 
	{ 
		ClearParams();
		throw; 
	}
}

void CRfcUser::ClearParams()
{
    if (m_pFILENAME != NULL) delete m_pFILENAME; m_pFILENAME = NULL;
    if (m_pDATA != NULL) delete m_pDATA; m_pDATA = NULL;
}

CRfcUser::~CRfcUser()
{
   ClearParams() ;
}

CArray<CString, CString> txtfile;

int	ArchiveFile(LPCTSTR datafile) {
	TCHAR tmpstr[16];
	int idx = strlen(datafile)-1;
	while ((idx >= 0) && (datafile[idx] != '\\')) idx--;
	CString datfile = datafile;
	CString newdatfile = "SAPCALL_TRACE\\";
	if (idx >= 0) newdatfile += datfile.Mid(idx+1);
	else newdatfile += datfile;
	sprintf(tmpstr, ".%06d", seq_no++);
	newdatfile += tmpstr;
	
	if (CopyFile(datafile, (LPCTSTR) newdatfile, TRUE) == 0) {
		LJERR("FEJL: kunne ikke kopiere trace-fil")
		return ERROR;
	}
	return TRUE;
}



int CallSap(LPCTSTR datafile, LPCTSTR filenopath, int argc, char *argv[]) {
	CRfcConnection   Connection ;	//Connection object
	char*            pException ;	//RFC call exceptions
    RFC_RC           rc;			//RFC return code
	int              status;

	txtfile.RemoveAll();

	CStdioFile cfile;
	CString txt;

	if (cfile.Open(datafile, CFile::modeRead | CFile::shareDenyWrite) == 0) {
		status = ERROR;
	} else {
		while(cfile.ReadString(txt)) txtfile.Add(txt);
		cfile.Close();
		status = SUCCES;
	}
	if (status == ERROR) return(ERROR);

	LJNORM("SAPCALL: Filen er indlaest og vi kalder SAP")

    CRfcUser    MyFunc(&Connection, filenopath, txtfile);

	try {
		Logon (Connection, datafile) ;
	} catch (RFC_ERROR_INFO_EX err) {
		DumpErrorInfo (err) ;
		return ERROR;
	}

	LJNORM("SAPCALL: Vi er nu logget paa SAP")

    try {
       rc = MyFunc.CallReceive (pException) ;
    }
    catch (RFC_ERROR_INFO_EX err) {
        DumpErrorInfo (err) ;
		return ERROR;
    }

	int retval = ERROR;
	ArchiveFile(datafile);
    if (rc != RFC_OK) {
		TCHAR tmpstr[1024];
		sprintf(tmpstr, "SAPCALL: RFC ret-code <> 0, %s", pException);
		LJERR(tmpstr)
    } else {
		LJNORM("SAPCALL: SAP er kaldt med succes")
		if (argc == 3 || (argc == 2 && strcmp(argv[1], "RENAME") != 0)) {
			CString TargetFile(argv[1]);
			if (TargetFile[TargetFile.GetLength()-1] != '\\') TargetFile += '\\';
			if ((argc == 2 && strcmp(argv[1], "RENAME") == 0) || (argc == 3)){
				CString file(filenopath);
				int length = file.GetLength();
				if (file.Find('.') < 0) { file += '.'; length++; }
				else while (file[length-1] != '.') length--;
				TargetFile += file.Left(length) + (CTime::GetCurrentTime()).Format("%Y%m%d%H%M%S");
			} else TargetFile += filenopath;
			if (CopyFile(datafile, (LPCTSTR) TargetFile, TRUE) == 0) {
				LJERR("FEJL: kunne ikke kopiere fil")
				Connection.Clear();
				return retval;
			}
		}
		if ((argc == 2 && strcmp(argv[1], "RENAME") == 0) || (argc == 3)){
			TRY
			{
				CString file(datafile);
				int length = file.GetLength();
				if (file.Find('.') < 0) { file += '.'; length++; }
				else while (file[length-1] != '.') length--;
				CString newfilename = file.Left(length) + (CTime::GetCurrentTime()).Format("%Y%m%d%H%M%S");
				CFile::Rename(file, newfilename);
				retval = SUCCES;
			}
			CATCH( CFileException, e )
			{
				LJERR("Kunne ikke rename' VL-valuta-fil")
			}
			END_CATCH
		} else {
			if (DeleteFile(datafile)) retval = SUCCES;
		}
	}

	Connection.Clear();

	return retval;
}

void Logon (CRfcConnection& Connection, LPCTSTR filename) 
{
	RFC_USER_INFO    UserInfo;     //All user login info
	RFC_CONNECT_INFO ConnectInfo;  //R/3 system connection info
	char InputBuffer[MAX_LEN]; 

	CString tmpstr(filename);

	if (tmpstr.Find("UDV202") >= 0) {
		strcpy(InputBuffer, "UDV");
		ConnectInfo.rstrDestination = InputBuffer  ;
		strcpy(InputBuffer, "202");
		UserInfo.rstrClient = InputBuffer  ;
	} else if (tmpstr.Find("UDV300") >= 0) {
		strcpy(InputBuffer, "UDV");
		ConnectInfo.rstrDestination = InputBuffer  ;
		strcpy(InputBuffer, "300");
		UserInfo.rstrClient = InputBuffer  ;
	} else if (tmpstr.Find("UDV302") >= 0) {
		strcpy(InputBuffer, "UDV");
		ConnectInfo.rstrDestination = InputBuffer  ;
		strcpy(InputBuffer, "302");
		UserInfo.rstrClient = InputBuffer  ;
	} else if (tmpstr.Find("UDV601") >= 0) {
		strcpy(InputBuffer, "UDV");
		ConnectInfo.rstrDestination = InputBuffer  ;
		strcpy(InputBuffer, "601");
		UserInfo.rstrClient = InputBuffer  ;
	} else if (tmpstr.Find("QAS202") >= 0) {
		strcpy(InputBuffer, "QAS");
		ConnectInfo.rstrDestination = InputBuffer  ;
		strcpy(InputBuffer, "202");
		UserInfo.rstrClient = InputBuffer  ;
	} else if (tmpstr.Find("QAS310") >= 0) {
		strcpy(InputBuffer, "QAS");
		ConnectInfo.rstrDestination = InputBuffer  ;
		strcpy(InputBuffer, "310");
		UserInfo.rstrClient = InputBuffer  ;
	} else {
		strcpy(InputBuffer, "PRD");
		ConnectInfo.rstrDestination = InputBuffer  ;
		strcpy(InputBuffer, "202");
		UserInfo.rstrClient = InputBuffer  ;
	}

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
