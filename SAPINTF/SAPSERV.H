#ifndef _SAPSERV_H_
#define _SAPSERV_H_

#include "stdafx.h"
#include "SAPINTF.h"
#include "crfclass.h"

class CRfcMyApp:public CRfcServerApp {
public:
    CRfcMyApp(CRfcConnection* pConnection = NULL) ;
    ~CRfcMyApp() ;
private:
    BOOL m_bPrintWait;
public:
    void OnIdle();
    void EnableIdlePrint();
};

#endif
