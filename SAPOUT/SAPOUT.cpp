// SAPOUT.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "crfclass.h"
#include "SAPOUT.h"
#include <afxtempl.h>

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
// CSAPOUTApp

BEGIN_MESSAGE_MAP(CSAPOUTApp, CWinApp)
	//{{AFX_MSG_MAP(CSAPOUTApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSAPOUTApp construction

CSAPOUTApp::CSAPOUTApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CSAPOUTApp object

CSAPOUTApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CSAPOUTApp initialization

BOOL CSAPOUTApp::InitInstance()
{
	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	return TRUE;
}

#define DLLSUCCES	1
#define DLLERROR	2
#define LOGNORMAL	1
#define LOGERROR	2
#define LNORM( txt )	Log(txt, LOGNORMAL, FALSE);
#define LERR( txt )		Log(txt, LOGERROR, FALSE);
#define LJNORM( txt )	Log(txt, LOGNORMAL, TRUE);
#define LJERR( txt )	Log(txt, LOGERROR, TRUE);

HANDLE hLogEvent, hLogReady;
TCHAR log_txt[500];
int log_mode, log_job;


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



#define SUCCES 1
#define ERROR 0

class Config {
private:
	CString comment, SAPsys, client, odbc_id, odbc_connect,
		SAPtable, whereopt, sql_once, sql_insert;
	CArray<CString, CString> fields;
	int status;
public:
	Config(LPCTSTR conffile);
	int GetConfStatus();
	CString& GetSAPsys();
	CString& GetClient();
	CString& GetOdbc_id();
	CString& GetOdbc_connect();
	CString& GetSAPtable();
	CString& GetWhereopt();
	CString& GetSql_once();
	CString& GetSql_insert();
	int GetFields();
	CString& GetField(int idx);
};

class ExclList {
private:
	CArray<CString, CString> table;
	int field[1000];
	CArray<CString, CString> value;
	int status;
public:
	ExclList();
	int GetStatus();
	int Eval(LPCTSTR testtable, int testfield, LPCTSTR testvalue);
};

int CallSap(LPCTSTR conffile, LPCTSTR datafile);
void Logon (CRfcConnection& Connection, Config& conf);
void DumpErrorInfo (RFC_ERROR_INFO_EX & err);




extern "C" int PASCAL EXPORT DllWorker ( int argc, char *argv[] )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	LJNORM("Starter SAP tabel udtraek utility")

	if ((argc != 2) && (argc != 3)) {
		LJERR("Forkert brug !!!! Korrekt brug: SAPOUT conf.sql temp_fil.dat trigger")
		return(DLLERROR);
	}

	int retval = DLLERROR;

	if (CallSap(argv[0], argv[1]) == SUCCES) retval = DLLSUCCES;

	if (argc == 3) {
		TRY
		{
			CFile::Remove(argv[2]);
		}
		CATCH( CFileException, e )
		{
			LJERR("FEJL: kunne ikke slette trigger-fil")
			retval = DLLERROR;
		}
		END_CATCH
	}

	return(retval);
}




RFC_FIELD_INFO typeOfRFC_DB_FLD[] = {
  {"FIELDNAME", 0, 0, TYPC, 30, 0},
  {"OFFSET", 1, 30, TYPNUM, 6, 0},
  {"LENGTH", 2, 36, TYPNUM, 6, 0},
  {"TYPE", 3, 42, TYPC, 1, 0},
  {"FIELDTEXT", 4, 43, TYPC, 60, 0},
};

RFC_FIELD_INFO typeOfRFC_DB_OPT[] = {
  {"TEXT", 0, 0, TYPC, 72, 0},
};

RFC_FIELD_INFO typeOfTAB512[] = {
  {"WA", 0, 0, TYPC, 512, 0},
};





class CRfcUser : public CRfcClientFunc
{
	//Constructor and destructor
public:
	CRfcUser  (CRfcConnection* pConnection, Config& conf);
	~CRfcUser (void) ;
	
	//Attributes
private:
	CRfcSimpleParam *m_pQUERY_TABLE;
	CRfcSimpleParam *m_pDELIMITER;
	CRfcSimpleParam *m_pNO_DATA;
	CRfcSimpleParam *m_pROWSKIPS;
	CRfcSimpleParam *m_pROWCOUNT;

	void ClearParams () ;

public:
	CRfcTableParam *m_pOPTIONS;
	CRfcTableParam *m_pFIELDS;
	CRfcTableParam *m_pDATA;

};

CRfcUser::CRfcUser(CRfcConnection* pConnection, Config& conf)
		:CRfcClientFunc(pConnection, "RFC_READ_TABLE")   
{
	m_pOPTIONS = NULL;
	m_pFIELDS = NULL;
	m_pDATA = NULL;
	m_pQUERY_TABLE = NULL;
	m_pDELIMITER = NULL;
	m_pNO_DATA = NULL;
	m_pROWSKIPS = NULL;
	m_pROWCOUNT = NULL;

	try
	{
		m_pQUERY_TABLE = new CRfcSimpleParam("QUERY_TABLE", TYPC, 30);
		m_pDELIMITER = new CRfcSimpleParam("DELIMITER", TYPC, 1);
		m_pNO_DATA = new CRfcSimpleParam("NO_DATA", TYPC, 1);
		m_pROWSKIPS = new CRfcSimpleParam("ROWSKIPS", TYPINT);
		m_pROWCOUNT = new CRfcSimpleParam("ROWCOUNT", TYPINT);

		m_pQUERY_TABLE->Value() = (LPCTSTR) conf.GetSAPtable();
		m_pDELIMITER->Value() = ",";
		m_pNO_DATA->Value() = " ";
		m_pROWSKIPS->Value() = 0;
		m_pROWCOUNT->Value() = 0;
		
		m_pOPTIONS = new CRfcTableParam ("OPTIONS", 72);
		m_pOPTIONS->AddFieldInfo (typeOfRFC_DB_OPT,
				sizeof(typeOfRFC_DB_OPT) / sizeof(RFC_FIELD_INFO)) ;

		m_pFIELDS = new CRfcTableParam ("FIELDS", 103);
		m_pFIELDS->AddFieldInfo (typeOfRFC_DB_FLD,
				sizeof(typeOfRFC_DB_FLD) / sizeof(RFC_FIELD_INFO)) ;

		m_pDATA = new CRfcTableParam ("DATA", 512);
		m_pDATA->AddFieldInfo (typeOfTAB512,
				sizeof(typeOfTAB512) / sizeof(RFC_FIELD_INFO)) ;

		AddImportParam (*m_pQUERY_TABLE);
		AddImportParam (*m_pDELIMITER);
		AddImportParam (*m_pNO_DATA);
		AddImportParam (*m_pROWSKIPS);
		AddImportParam (*m_pROWCOUNT);
		AddTableParam (*m_pOPTIONS);
		AddTableParam (*m_pFIELDS);
		AddTableParam (*m_pDATA);
		m_pOPTIONS->Create();
		m_pFIELDS->Create();
		m_pDATA->Create();

		for (int i = 0; i < conf.GetFields(); i++) {
			m_pFIELDS->AppendInitializedRow();
			m_pFIELDS->Field("FIELDNAME") = (LPCTSTR) conf.GetField(i);
		}
		if (conf.GetWhereopt() > " ") {
			m_pOPTIONS->AppendInitializedRow();
			m_pOPTIONS->Field("TEXT") = (LPCTSTR) conf.GetWhereopt();
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
    if (m_pOPTIONS != NULL) delete m_pOPTIONS; m_pOPTIONS = NULL;
    if (m_pFIELDS != NULL) delete m_pFIELDS; m_pFIELDS = NULL;
    if (m_pDATA != NULL) delete m_pDATA; m_pDATA = NULL;
	if (m_pQUERY_TABLE != NULL) delete m_pQUERY_TABLE; m_pQUERY_TABLE = NULL;
	if (m_pDELIMITER != NULL) delete m_pDELIMITER; m_pDELIMITER = NULL;
	if (m_pNO_DATA != NULL) delete m_pNO_DATA; m_pNO_DATA = NULL;
	if (m_pROWSKIPS != NULL) delete m_pROWSKIPS; m_pROWSKIPS = NULL;
	if (m_pROWCOUNT != NULL) delete m_pROWCOUNT; m_pROWCOUNT = NULL;
}

CRfcUser::~CRfcUser()
{
   ClearParams() ;
}


char tmpstr[20000];

int CallSap(LPCTSTR conffile, LPCTSTR datafile) {
	CRfcConnection   Connection ;	//Connection object
	char*            pException ;	//RFC call exceptions
    RFC_RC           rc;			//RFC return code 

	Config		conf(conffile);
	if (conf.GetConfStatus() == ERROR) {
		LJERR("Kunne ikke laese config-fil")
		return 0;
	}
	ExclList excllist;
	if (excllist.GetStatus() == ERROR) {
		LJERR("Kunne ikke laese excllist-fil")
		return 0;
	}

    CRfcUser    MyFunc(&Connection, conf);

	try {
		Logon (Connection, conf) ;
	} catch (RFC_ERROR_INFO_EX err) {
		DumpErrorInfo (err) ;
		return 0;
	}

	CString logtxt("Henter data fra ");
	logtxt += (LPCTSTR) conf.GetSAPsys();
	logtxt += ",";
	logtxt += (LPCTSTR) conf.GetClient();
	logtxt += ",";
	logtxt += (LPCTSTR) conf.GetSAPtable();
	logtxt += ".... ";
	LJNORM((LPCTSTR) logtxt)
    try {
       rc = MyFunc.CallReceive (pException) ;
    }
    catch (RFC_ERROR_INFO_EX err) {
        DumpErrorInfo (err) ;
		Connection.Clear();
		return 0;
    }

	CStdioFile datfile;
	if (datfile.Open(datafile, CFile::modeCreate | CFile::modeReadWrite) == 0) {
		LJERR("FEJL: kunne ikke aabne output fil")
		Connection.Clear();
		return 0;
	}
	CString dataval, tmp, sql_insert;
	int offset, length;
	sql_insert = conf.GetSql_insert();
    if (rc != RFC_OK) {
		logtxt = "RFC ret-code <> 0 ";
		logtxt += pException;
		LJERR((LPCTSTR) logtxt)
    } else {
		int userec = TRUE;
		int droppedrecords = 0;
		TCHAR logtxt2[1024];
		sprintf(logtxt2, "%u records hentet", MyFunc.m_pDATA->GetRowCount());
		LJNORM(logtxt2)
		for (int i = 0; i < MyFunc.m_pDATA->GetRowCount(); i++) {
			userec = TRUE;
			dataval = (const char *) MyFunc.m_pDATA->Cell(i,0);
			int k = 0, l = 0;
			for (int j = 0; j < MyFunc.m_pFIELDS->GetRowCount(); j++) {
				offset = (RFC_INT) MyFunc.m_pFIELDS->Cell(j,1);
				length = (RFC_INT) MyFunc.m_pFIELDS->Cell(j,2);
				tmp = dataval.Mid(offset, length);
				for(;(sql_insert[l] != '%' && sql_insert[l] != '$' &&
					l < (int) strlen(sql_insert)); l++, k++)
					tmpstr[k] = sql_insert[l];
				tmpstr[k] = '\0'; l++;
				if (sql_insert[l-1] == '$') {
					tmpstr[k++] = '\'';
					tmpstr[k] = '\0';
				} else {
					if (tmp.Find('-') >= 0) {
						tmp.Replace('-', ' ');
						tmp.TrimLeft();
						tmp.Insert(0, '-');
					}
				}
				if (excllist.Eval(conffile, j, tmp) == FALSE) userec = FALSE;
				strcat(tmpstr, tmp);
				for (unsigned int m = k; m < strlen(tmpstr); m++) if (tmpstr[m] == '\'') tmpstr[m] = '´';
				k = strlen(tmpstr);
				if (sql_insert[l-1] == '$') {
					tmpstr[k++] = '\'';
					tmpstr[k] = '\0';
				}
			}
			while (l < sql_insert.GetLength()) tmpstr[k++] = sql_insert[l++];
			tmpstr[k] = '\n';
			tmpstr[k+1] = '\0';
			if (userec)	datfile.WriteString(tmpstr);
			else {
				droppedrecords++;
//				sprintf(logtxt2, "record dropped");
//				LJNORM(logtxt2)
			}
//			cout << (LPCTSTR) insert << endl << flush;
		}
		sprintf(logtxt2, "%d records dropped", droppedrecords);
		LJNORM(logtxt2)
	}
	datfile.Close();

	Connection.Clear();

	return SUCCES;
}


void Logon (CRfcConnection& Connection, Config& conf) 
{
	RFC_USER_INFO    UserInfo;     //All user login info
	RFC_CONNECT_INFO ConnectInfo;  //R/3 system connection info
	char InputBuffer[MAX_LEN]; 

	strcpy(InputBuffer, (LPCTSTR) conf.GetSAPsys());
	ConnectInfo.rstrDestination = InputBuffer  ;
	
	strcpy(InputBuffer, (LPCTSTR) conf.GetClient());
	UserInfo.rstrClient = InputBuffer  ;

	strcpy(InputBuffer, "sysrfc");
	UserInfo.rstrUserName = InputBuffer ;

	strcpy(InputBuffer, "wazx88");
	UserInfo.rstrPassword = InputBuffer ;
    
	UserInfo.rstrLanguage = "E";

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

Config::Config(LPCTSTR conffile) {
	CStdioFile cfile;
	CString fielddata;

	if (cfile.Open(conffile, CFile::modeRead | CFile::shareDenyWrite) == 0) {
		status = ERROR;
	} else {
		cfile.ReadString(comment);
		cfile.ReadString(SAPsys);
		cfile.ReadString(client);
		cfile.ReadString(odbc_id);
		cfile.ReadString(odbc_connect);
		cfile.ReadString(SAPtable);
		cfile.ReadString(sql_once);
		cfile.ReadString(whereopt);
		cfile.ReadString(fielddata);
		cfile.ReadString(sql_insert);
		cfile.Close();
		status = SUCCES;
	}
	fielddata.TrimLeft(); fielddata.TrimRight();
	if (fielddata > " ") {
		int idx = 0;
		while ((idx = fielddata.Find(" ", 0)) >= 0) {
			fields.Add(fielddata.Left(idx));
			fielddata.Delete(0,idx+1);
		}
		fields.Add(fielddata);
	}
}

int Config::GetConfStatus() {
	return(status);
}

CString& Config::GetSAPsys() {
	return(SAPsys);
}

CString& Config::GetClient() {
	return(client);
}

CString& Config::GetOdbc_id() {
	return(odbc_id);
}

CString& Config::GetOdbc_connect() {
	return(odbc_connect);
}

CString& Config::GetSAPtable() {
	return(SAPtable);
}

CString& Config::GetWhereopt() {
	return(whereopt);
}

CString& Config::GetSql_once() {
	return(sql_once);
}

CString& Config::GetSql_insert() {
	return(sql_insert);
}

int Config::GetFields() {
	return(fields.GetSize());
}

CString& Config::GetField(int idx){
	return(fields[idx]);
}


ExclList::ExclList() {
	CStdioFile cfile;
	CString fielddata;

	if (cfile.Open("SAP\\excllist", CFile::modeRead | CFile::shareDenyWrite) == 0) {
		status = ERROR;
	} else {
		int x = 0;
		while (cfile.ReadString(fielddata)) {
			fielddata.TrimLeft(); fielddata.TrimRight(); fielddata += " ";
			int count = 0;
			int idx = 0;
			while ((idx = fielddata.Find(" ", 0)) >= 0) {
				if (count == 0) table.Add(fielddata.Left(idx));
				if (count == 1) field[x] = atoi((LPCTSTR) fielddata.Left(idx));
				if (count == 2) value.Add(fielddata.Left(idx));
				fielddata.Delete(0,idx+1);
				count++;
			}
			if (count == 0) table.Add(fielddata.Left(idx));
			if (count == 1) field[x] = atoi((LPCTSTR) fielddata.Left(idx));
			if (count == 2) value.Add(fielddata.Left(idx));
			x++;
		}
		cfile.Close();
		status = SUCCES;
	}
}

int ExclList::Eval(LPCTSTR testtable, int testfield, LPCTSTR testvalue) {
	for (int i = 0; i < table.GetSize(); i++) {
		CString tmp(testtable);
		if (tmp.Find((LPCTSTR) table[i]) >= 0) {
			if (field[i] == testfield) {
				if (value[i] == testvalue) return(FALSE);
			}
		}
	}
	return(TRUE);
}

int ExclList::GetStatus() {
	return(status);
}