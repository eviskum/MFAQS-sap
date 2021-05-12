#ifndef _SQLGET_H_
#define _SQLGET_H_

class SqlFields {
public:
	CString fieldname;
	CString type;
	int length;
	int decimal;
	SqlFields *next;
	SqlFields();
};

class SqlData {
public:
	CString data;
	int null;
	SqlData *next;
	SqlData();
};

extern SqlFields *rootfield, *currentfield;
extern SqlData *rootdata, *currentdata;
extern int no_fields;

int GetSqlData(LPCTSTR instance, LPCTSTR sql_select, int maxrows);


#endif
