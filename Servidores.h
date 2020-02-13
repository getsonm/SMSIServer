// Servidores.h: interface for the CServidores class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERVIDORES_H__72E0ED49_7C6A_4834_9480_CEF5FE68FC11__INCLUDED_)
#define AFX_SERVIDORES_H__72E0ED49_7C6A_4834_9480_CEF5FE68FC11__INCLUDED_

#include <afxsock.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CServidores : public CSocket  
{
public:
	CServidores();
	virtual ~CServidores();
	short ChkServidor(LPCTSTR,char*,u_short);

private:
	CString m_Serv;
	short PingServidor(char *);
	short TesteServidor(char *,u_short);
};

#endif // !defined(AFX_SERVIDORES_H__72E0ED49_7C6A_4834_9480_CEF5FE68FC11__INCLUDED_)
