#ifndef _SAPCONN_H_
#define _SAPCONN_H_

#include "stdafx.h"
#include "SAPINTF.h"

typedef void (CALLBACK* LPFNLOGFUNC1)(int, LPCTSTR);
typedef void (CALLBACK* LPFNLOGFUNC2)(int, LPCTSTR);

extern LPFNLOGFUNC1 LogNorm;
extern LPFNLOGFUNC2 LogErr;

extern int workpart;

#define LJNORM( txt )	LogNorm(workpart, txt);
#define LJERR( txt )	LogErr(workpart, txt);

// void ConnectSap(int argc, TCHAR* argv[]);
extern "C" int PASCAL EXPORT DllLogReg(void *Func1, void *Func2);
extern "C" int PASCAL EXPORT StopWork(void);
extern "C" int PASCAL EXPORT StartWork(int part, int argc, TCHAR *argv[]);

UINT ConnectSap( LPVOID pParam /* int argc, TCHAR* argv[]*/ );

#endif
