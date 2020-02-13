// InitFile.h: interface for the CInitFile class.
//
//////////////////////////////////////////////////////////////////////

/*
'=======================================================================================
'= Arquivo: 
'= Função:  
'= Autor:   Getson Miranda
'= Data:    21/10/2003
'=======================================================================================
*/

#if !defined(AFX_INITFILE_H__190D79A6_E340_4CC4_ACDF_E030493280D9__INCLUDED_)
#define AFX_INITFILE_H__190D79A6_E340_4CC4_ACDF_E030493280D9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CInitFile  
{
public:
	CInitFile(); // construtora
	virtual ~CInitFile(); // destrutora

	char nm[TAMPATHDEFAULT];						// caminho + nome arquivo INI

	/* retornos do arquivo INI */

	/* _-=-_ Base de Dados - Secao BASE_DADOS do arquivo INI _-=-_ */
	DWORD GetDRIVERValue( char * );					// Microsoft Access Driver (*.mdb)
	DWORD GetDBQValue( char * );						// C:/SMSISERVER/BD/RBDBD001.MDB
	DWORD GetUIDValue( char * );						//
	DWORD GetPWDValue( char * );						//
	DWORD GetOutrosConnectionStringValue( char * );	//
	DWORD GetConnectionTimeoutValue( char * );		// 600
	DWORD GetCommandTimeoutValue( char * );			// 2000

	/* _-=-_ Servidor do OpenView - Secao SNMP_HOST_INFO do arquivo INI _-=-_ */
	VOID GetTRAP_RECEIVER_PORTValue( char * );	// 162
	VOID GetTRAP_RECEIVERValue( char * );		// 127.0.0.1
	VOID GetTRAP_VERSIONValue( char * );		// 0
	VOID GetTRAP_COMUNITYValue( char * );		// public
	VOID GetTRAP_ENTERPRISEValue( char * );		// 1.3.6.1.4.1.105.2.2
	VOID GetTRAP_GENTRAPValue( char * );		// 6
	VOID GetTRAP_SPECTRAPValue( char * );		// 5

	/* _-=-_ Parametros Gerais - Secao GERAL do arquivo INI _-=-_ */
	VOID GetLOGDIRValue( char * );			// C:/SMSISERVER/LOG/

	VOID GetMibOID( char * );
	VOID GetMibPorta( char * );
	VOID GetMibVersao( char * );
	VOID GetMibComunidade( char * );
	VOID GetMibMsg( char * );

private:

	VOID GetArqIniApp();
};

#endif // !defined(AFX_INITFILE_H__190D79A6_E340_4CC4_ACDF_E030493280D9__INCLUDED_)
