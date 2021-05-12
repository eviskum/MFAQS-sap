#include "stdafx.h"
#include "SAPINTF.h"
#include "crfclass.h"
#include "SAPSERV.h"
#include "SAPDbGet.h"
#include "SQLGET.h"


RFC_FIELD_INFO  CRfcDbGet::FieldsDEF[] = {
	{"FIELDNAME", 0, 0, TYPC, 30, 0},
	{"FTYPE", 1, 30, TYPC, 1, 0},
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
	int retval, maxrows;
	CString sqlselect, instance;
	sqlselect = (CSTR) m_pSimSelect->Value();
	instance = (CSTR) m_pSimDb->Value();
	maxrows = m_pSimMaxRows->Value();
	retval = GetSqlData((LPCTSTR) instance, (LPCTSTR) sqlselect, maxrows);
	m_pSimRows->Value() = retval;
	if (retval >= 0) {
		m_pSimFields->Value() = no_fields;
		currentdata = rootdata;
		while (currentdata != NULL) {
			m_pTabContents->AppendInitializedRow();
			m_pTabContents->Field(0) = currentdata->null;
			m_pTabContents->Field(1) = (LPCTSTR) currentdata->data;
			rootdata = currentdata;
			currentdata = currentdata->next;
			delete rootdata;
		}
		currentfield = rootfield;
		while (currentfield != NULL) {
			m_pTabDef->AppendInitializedRow();
			m_pTabDef->Field(0) = (LPCTSTR) currentfield->fieldname;
			m_pTabDef->Field(1) = currentfield->type;
			m_pTabDef->Field(2) = currentfield->length;
			m_pTabDef->Field(3) = currentfield->decimal;
			rootfield = currentfield;
			currentfield = currentfield->next;
			delete rootfield;
		}
	}

	rootdata = NULL;
	rootfield = NULL;

	m_pApp->EnableIdlePrint();
}