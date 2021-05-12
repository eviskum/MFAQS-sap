#include "stdafx.h"
#include "SAPINTF.h"
#include "SAPCONN.h"
#include "SAPSERV.h"
#include "SAPDbGet.h"

HANDLE end_sap_event;

int connect = FALSE;
int stoploop = FALSE;
CRfcMyApp *ptr_MyApp = NULL;
CRfcConnection *ptr_Conn = NULL;

TCHAR arg[25];


LPFNLOGFUNC1 LogNorm;
LPFNLOGFUNC2 LogErr;

int workpart;

CWinThread *hSaploop;


extern "C" int PASCAL EXPORT DllLogReg(void *Func1, void *Func2) {
	LogNorm = (LPFNLOGFUNC1) Func1;
	LogErr = (LPFNLOGFUNC2) Func2;
	return(TRUE);
}


extern "C" int PASCAL EXPORT StopWork(void) {
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	cout << "Nedlukker SAP interface" << endl << flush;
	SetEvent(end_sap_event);
	if (connect == TRUE) {
		ptr_MyApp->RemoveServerFunction(0);
		ptr_Conn->Close();
	}

	stoploop = TRUE;

	DWORD ThreadStatus;
	do {
		GetExitCodeThread(hSaploop->m_hThread, &ThreadStatus);
		Sleep(500);
	} while (ThreadStatus == STILL_ACTIVE);
	delete hSaploop;

	delete end_sap_event;
	cout << "..SAP disconnected og interface nedlukket" << endl << flush;

	return(TRUE);
}

extern "C" int PASCAL EXPORT StartWork(int part, int argc, TCHAR *argv[]) {
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	workpart = part;

	if (argc != 2) {
		LJERR("FEJL: Forkert antal parametre")
		return(FALSE);
	}

	SECURITY_ATTRIBUTES signalatt;
	signalatt.nLength = sizeof(SECURITY_ATTRIBUTES);
	signalatt.lpSecurityDescriptor = NULL;
	signalatt.bInheritHandle = TRUE;
	end_sap_event = CreateEvent(&signalatt, TRUE, FALSE, NULL);

	connect = FALSE;
	stoploop = FALSE;
	ptr_MyApp = NULL;

	strcpy(arg, argv[1]);

	LJNORM("Starter SAP Interface work thread")
	hSaploop = AfxBeginThread(&ConnectSap, (LPVOID) arg);

	return(TRUE);
}

UINT ConnectSap( LPVOID pParam ) {
	LJNORM("..SAP Interface work thread initialiserer..")

	do {
		CRfcConnection	Connection;
		CRfcMyApp		MyApp(&Connection);
		CRfcDbGet		RfcDbGet(&MyApp);

		TCHAR* argv[10];
		TCHAR progname[] = "SAPINTF";

		argv[0] = progname;
		argv[1] = (LPTSTR) pParam;
		argv[2] = NULL;
		argv[3] = NULL;
		argv[4] = NULL;
		argv[5] = NULL;
		argv[6] = NULL;
		argv[7] = NULL;
		argv[8] = NULL;
		argv[9] = NULL;

	    try
		{
			ptr_Conn = &Connection;
			LJNORM(CString("..connecter til SAP (") + argv[1] + ")")
	        Connection.Accept (argv) ;

			LJNORM("..registrerer RFC-funktioner i SAP")
	        MyApp.AddServerFunction (RfcDbGet) ;
			ptr_MyApp = &MyApp;
			connect = TRUE;
  
			LJNORM("SAP interface work process er startet")
		    MyApp.Run() ;
	    }
		catch(RFC_ERROR_INFO_EX err)
	    {
			connect = FALSE;
			if (stoploop == FALSE) {
				char grpName[128];
				ErrorGroupName(err.group, grpName);
				LJERR(CString("RFC Error:\n") +
					"        Error Group: " + grpName + "\n" +
					"        Key: " + err.key + "\n" +
					"        Message: " + err.message + "\n")
			}
		}
		connect = FALSE;
	} while (WaitForSingleObject(end_sap_event, 300000) == WAIT_TIMEOUT);
	LJNORM("..SAP Interface work thread lukker")
	ExitThread(1);
	return(1);
}
