#include "stdafx.h"
#include "SAPINTF.h"
#include "crfclass.h"
#include "SAPSERV.h"
#include "SAPCONN.h"

CRfcMyApp::CRfcMyApp (CRfcConnection* pConnection)
          :CRfcServerApp (pConnection), m_bPrintWait(TRUE) {
}

CRfcMyApp::~CRfcMyApp() {
}

void CRfcMyApp::OnIdle() {
	Sleep(1000);
	if(m_bPrintWait)
	{
		LJNORM("Vi venter paa tilslutning...")
		m_bPrintWait = FALSE;
	}
}

void CRfcMyApp::EnableIdlePrint() {
	m_bPrintWait = TRUE;
}
