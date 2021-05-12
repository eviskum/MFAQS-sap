// SAPEVENT.h : main header file for the SAPEVENT DLL
//

#if !defined(AFX_SAPEVENT_H__E856A8FC_DA56_4A8E_86E1_14C03A66E23A__INCLUDED_)
#define AFX_SAPEVENT_H__E856A8FC_DA56_4A8E_86E1_14C03A66E23A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CSAPEVENTApp
// See SAPEVENT.cpp for the implementation of this class
//

class CSAPEVENTApp : public CWinApp
{
public:
	CSAPEVENTApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSAPEVENTApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CSAPEVENTApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SAPEVENT_H__E856A8FC_DA56_4A8E_86E1_14C03A66E23A__INCLUDED_)
