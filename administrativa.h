/*
'=======================================================================================
'= Arquivo: Administrativa.h
'= Função:  cabecalho para a classe de administracao do servico
'=
'= Autor:   Getson Miranda
'= Data:    20/10/2003
'=======================================================================================
*/

// Administrativa.h: interface for the CAdministrativa class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ADMINISTRATIVA_H__0894AC06_DB27_4A42_B2AE_82B603023D0F__INCLUDED_)
#define AFX_ADMINISTRATIVA_H__0894AC06_DB27_4A42_B2AE_82B603023D0F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ntservice.h"

class CAdministrativa : public CNTService
{
public:
	CAdministrativa();
	virtual BOOL OnInit();
    virtual void Run();
    virtual BOOL OnUserControl(DWORD dwOpcode);
    void SaveStatus();

	// Control parameters
	int m_iStartParam;
	int m_iIncParam;
	int m_iState;

};

#endif // !defined(AFX_ADMINISTRATIVA_H__0894AC06_DB27_4A42_B2AE_82B603023D0F__INCLUDED_)
