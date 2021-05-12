// SAPCALL.h : main header file for the SAPCALL DLL
//

#if !defined(AFX_SAPCALL_H__45C6FA8F_19B0_42EF_AEEF_428D7CBABDEA__INCLUDED_)
#define AFX_SAPCALL_H__45C6FA8F_19B0_42EF_AEEF_428D7CBABDEA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CSAPCALLApp
// See SAPCALL.cpp for the implementation of this class
//

class CSAPCALLApp : public CWinApp
{
public:
	CSAPCALLApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSAPCALLApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CSAPCALLApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SAPCALL_H__45C6FA8F_19B0_42EF_AEEF_428D7CBABDEA__INCLUDED_)
