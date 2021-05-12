#include "stdafx.h"
#include "SAPINTF.h"
#include "SQLGET.h"
#include "SAPCONN.h"
#include <afxdb.h>
#include <fstream.h>


SqlFields *rootfield, *currentfield;

SqlFields::SqlFields() {
	length = 0;
	decimal = 0;
	next = NULL;
}

SqlData *rootdata, *currentdata;

SqlData::SqlData() {
	null = 0;
	next = NULL;
}

int no_fields;

int GetSqlData(LPCTSTR instance, LPCTSTR sql_select, int maxrows) {
	CDatabase SQL;

	CString dbinst;
	dbinst = instance;

	TRY {
		LJNORM("Connecter til SQL")
		dbinst.MakeUpper();
		if (dbinst == "DB2") SQL.Open("SQLPVSE", FALSE, FALSE, "ODBC;UID=gki;PWD=abk;", FALSE);
		else SQL.Open("MSSQL", FALSE, FALSE, "ODBC;DATABASE=warehouse;UID=dwhm;PWD=golf;", FALSE);
	}
	CATCH(CDBException, e) {
		LJERR(CString("FEJL i ODBC-connect: ") + e->m_strError)
		return(-1);
	}
	END_CATCH
	
	CRecordset SQL_Table(&SQL);
	LJNORM("  fetcher SQL recordset...")
	TRY {
		SQL_Table.Open(CRecordset::forwardOnly, sql_select);
	}
	CATCH(CDBException, e) {
		LJERR(CString("FEJL i SQL-fetch: ") + e->m_strError)
		return(-1);
	}
	END_CATCH

	no_fields = 0;
	short fields = SQL_Table.GetODBCFieldCount( );
	no_fields = fields;

	int records = 0;

	struct CODBCFieldInfo fieldstruct;
	
	rootdata = currentdata = NULL;
	rootfield = currentfield = NULL;

	if( !SQL_Table.IsBOF( ) ) {
		while ( !SQL_Table.IsEOF( ) && ((maxrows == 0) || (maxrows > records))) {
			for (short i = 0; i < fields; i++) {
				if (records == 0) {
					if (rootfield == NULL) rootfield = currentfield = new SqlFields;
					else currentfield = currentfield->next = new SqlFields;	

					SQL_Table.GetODBCFieldInfo(i, fieldstruct);
					currentfield->fieldname = fieldstruct.m_strName;
					switch (fieldstruct.m_nSQLType) {
					case 1: // CHAR
						currentfield->type = "C";
						currentfield->length = fieldstruct.m_nPrecision;
						break;
					case 3: // DEC
						currentfield->type = "N";
						currentfield->length = fieldstruct.m_nPrecision;
						currentfield->decimal = fieldstruct.m_nScale;
						break;
					case 4: // INTEGER
						currentfield->type = "I";
						break;
					case 5: // SMALLINT
						currentfield->type = "S";
						break;
					case 9: // DATE
						currentfield->type = "D";
						break;
					case 11: // TIMESTAMP
						currentfield->type = "T";
						break;
					default:
						currentfield->type = "U";
					}
				}
				if (rootdata == NULL) rootdata = currentdata = new SqlData;
				else currentdata = currentdata->next = new SqlData;
				CString bufferstring;
				SQL_Table.GetFieldValue(i, bufferstring);
				if (bufferstring.IsEmpty() == TRUE) {
					currentdata->null = 1;
				} else {
					currentdata->data = bufferstring;
					currentdata->null = 0;
				}
			}
			if ((records++ % 1000) == 0) printf(".");
			SQL_Table.MoveNext( );
		}
	}

	TCHAR tmpstr[1024];
	sprintf(tmpstr, "Har hentet %i records", records);
	LJNORM(tmpstr)
	SQL_Table.Close();
	SQL.Close();
	return(records);
}
