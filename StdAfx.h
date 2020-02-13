/*
'=======================================================================================
'= Arquivo: stdafx.h
'= Função:  disponiblizar api's, defines etc. . .
'= Autor:   Getson Miranda
'= Data:    24/10/2003
'=======================================================================================
*/

// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__C39D99BE_4ADB_4B92_A767_3862032A90E0__INCLUDED_)
#define AFX_STDAFX_H__C39D99BE_4ADB_4B92_A767_3862032A90E0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#define _WIN32_WINNT 0x0500

#include <afx.h>
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <iostream>

// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
#import "TrapSNMP.dll" 
using namespace TrapSNMP;

#include <winsvc.h>		// SCM - Windows Service Manager
#include <afxinet.h>	// HTML - Paginas de comparacao e teste
#include <afxsock.h>
#include "global.h"		// DEFINES e FUNCOES globais para o sistema

#endif // !defined(AFX_STDAFX_H__C39D99BE_4ADB_4B92_A767_3862032A90E0__INCLUDED_)
