#include "stdafx.h"
#include "SAPINTF.h"
#include "crfclass.h"
#include "SAPSERV.h"

class CRfcDbGet:public CRfcServerFunc {
private:
    static RFC_FIELD_INFO  FieldsDEF[] ;
    static RFC_FIELD_INFO  FieldsCONTENTS[] ;
public:
	CRfcDbGet();
	CRfcDbGet(CRfcMyApp *pApp);
	~CRfcDbGet();
protected:
	CRfcSimpleParam* m_pSimDb       ;
	CRfcSimpleParam* m_pSimSelect   ;
	CRfcSimpleParam* m_pSimMaxRows  ;
	CRfcSimpleParam* m_pSimRows     ;
	CRfcSimpleParam* m_pSimFields   ;
	CRfcTableParam * m_pTabDef      ;
	CRfcTableParam * m_pTabContents ;
	CRfcMyApp      * m_pApp         ;
private:
	void ClearParams();
public:
	void Process(void);
};



RFC_FIELD_INFO  CRfcDbGet::FieldsDEF[] = {
	{"FIELDNAME", 0, 0, TYPC, 30, 0},
	{"TYPE", 1, 30, TYPC, 1, 0},
	{"LENGTH", 2, 31, TYPNUM, 6, 0},
	{"DECIMAL", 3, 37, TYPNUM, 6, 0}
} ;

RFC_FIELD_INFO  CRfcDbGet::FieldsCONTENTS[] = {
	{"NULL", 0, 0, TYPNUM, 1, 0},
	{"DATA", 1, 1, TYPC, 256, 0}
} ;


CRfcDbGet::CRfcDbGet (CRfcMyApp *pApp) : CRfcServerFunc ("DBGET"), m_pApp(pApp)
{
	try
	{
		m_pSimDb       = new CRfcSimpleParam ("DBINSTANCE", TYPC, 3);
		m_pSimSelect   = new CRfcSimpleParam ("DBSELECT", TYPC, 1000);
		m_pSimMaxRows  = new CRfcSimpleParam ("MAXROWS",  TYPINT, sizeof(RFC_INT));
		m_pSimRows     = new CRfcSimpleParam ("ROWS",  TYPINT, sizeof(RFC_INT));
		m_pSimFields   = new CRfcSimpleParam ("FIELDS",  TYPINT, sizeof(RFC_INT));

		m_pTabDef      = new CRfcTableParam  ("FIELDDEF", 43);
		m_pTabDef->AddFieldInfo (FieldsDEF,
						sizeof(FieldsDEF) / sizeof(RFC_FIELD_INFO)) ;

		m_pTabContents = new CRfcTableParam  ("CONTENTS",257) ;
		m_pTabContents->AddFieldInfo (FieldsCONTENTS, 
						sizeof(FieldsCONTENTS) / sizeof(RFC_FIELD_INFO)) ;

		AddImportParam (*m_pSimDb) ;
		AddImportParam (*m_pSimSelect) ;
		AddImportParam (*m_pSimMaxRows) ;
		AddExportParam (*m_pSimRows) ;
		AddExportParam (*m_pSimFields) ;
		AddTableParam  (*m_pTabDef) ;
		AddTableParam  (*m_pTabContents) ;
	}
	catch (char*)
	{
		ClearParams() ;
		throw ;
	}
}

CRfcDbGet::~CRfcDbGet()
{
    ClearParams() ;
}

void CRfcDbGet::ClearParams()
{
	if (m_pSimDb       != NULL) delete m_pSimDb ; 
	if (m_pSimSelect   != NULL) delete m_pSimSelect ;
	if (m_pSimMaxRows  != NULL) delete m_pSimMaxRows ;
	if (m_pSimRows     != NULL) delete m_pSimRows ;
	if (m_pSimFields   != NULL) delete m_pSimFields ;
	if (m_pTabDef      != NULL) delete m_pTabDef ;
	if (m_pTabContents != NULL) delete m_pTabContents ;
	m_pSimDb = NULL;
	m_pSimSelect = NULL;
	m_pSimMaxRows = NULL;
	m_pSimRows = NULL;
	m_pSimFields = NULL;
	m_pTabDef = NULL;
	m_pTabContents = NULL;
} 

void CRfcDbGet::Process()
{
/*	int nCount ;
	nCount = m_pTabContents->GetRowCount() ;
	m_pSimCount->Value() = nCount ;
	cout << (CSTR)m_pSimTitle->Value() << endl ;
	for (int nRow = 0; nRow < nCount; nRow++)  
	{
		cout << (CSTR)m_pTabContents->Cell(nRow, 0) << endl ;
	}
	cout << flush ; */
	m_pApp->EnableIdlePrint();
}