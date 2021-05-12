// DLLLOAD.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "DLLLOAD.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// The one and only application object

CWinApp theApp;

using namespace std;


typedef void (CALLBACK* LPFNDLLFUNC1)(void);
typedef void (CALLBACK* LPFNDLLFUNC2)(void);

LPFNDLLFUNC1 StopWork;
LPFNDLLFUNC2 StartWork;

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		cerr << _T("Fatal Error: MFC initialization failed") << endl;
		nRetCode = 1;
	}
	else
	{
		// TODO: code your application's behavior here.

		HINSTANCE hDll;

		hDll = AfxLoadLibrary("SAPINTF.DLL");
		if (hDll != NULL) {
			StopWork = (LPFNDLLFUNC1)GetProcAddress(hDll, "StopWork");
			StartWork = (LPFNDLLFUNC2)GetProcAddress(hDll, "StartWork");
		} else {
			cout << "Kunne ikke loade dll" << endl << flush;
			exit(1);
		}

//		cout << argc << endl << flush;
//		cout << argv[0] << endl << flush;
//		cout << argv[1] << endl << flush;


		cout << "--------- vi starter" << endl << flush;
		StartWork();
		cout << "--------- started" << endl << flush;
		Sleep(15000);
		cout << "--------- vi stopper" << endl << flush;
		StopWork();
		cout << "--------- vi er stoppet" << endl << flush;
	}

	return nRetCode;
}


