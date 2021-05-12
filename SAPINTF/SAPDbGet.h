#ifndef _SAPDBGET_H_
#define _SAPDBGET_H_

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

#endif
