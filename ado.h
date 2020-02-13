// ado.h: interface for the CAdo class.
//
//////////////////////////////////////////////////////////////////////

/*
'=======================================================================================
'= Arquivo: ado.h
'= Função:  arquivo de cabecalho para classe de tratamento do banco de dados
'= Autor:   Getson Miranda
'= Data:    21/10/2003
'=======================================================================================
*/

#if !defined(AFX_ADO_H__638C30F8_BB8E_491A_83D1_39349672C5DB__INCLUDED_)
#define AFX_ADO_H__638C30F8_BB8E_491A_83D1_39349672C5DB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CAdo
{

public:
	CAdo();				// construtor
	virtual ~CAdo();	// destrutor da classe

	short AdoStatus;

	VOID EnviarLog(DWORD, DWORD, char *, long, char, char *);

	short ConnectSQLServ(CString &, long, long, char *);

	DWORD recStrConnSQL(long, CString &, long &, long &);

	DWORD LeArqIni();

	DWORD RgTbPagComp(long &, long &, char *, char *);
	DWORD AbreTabelaPgComp();

	DWORD AbreTabelaPgTeste();
	DWORD RgTbPagTeste(long &, long &, char *, char *);

	DWORD AtualizaPagComp(long, long &, char *, char *);
	DWORD AtualizaPagTeste(long, long &, char *, char *);

	DWORD rgPgCmp(long, TCHAR *, TCHAR *, long &, long);

	DWORD tbTrapPgTeste(long , char *, char *, char *);

	// ******** Trap Pg Cmp
	DWORD tbTrapPgComp(long, char *, char *, char *);

	DWORD tbTrapErroSrv(long, char *, char *, char *);

	DWORD	tbUpdServ(long, char *, char *);

	// Abre o banco de dados e disponibiliza registros da tabela solicitada
	DWORD AbreTabelaServidores();

	// Devolve registros da tabela aberta
	DWORD RegistroServidores2(long &, char *, char *);

	// Parametros gerais
	DWORD recParmGeral2(long &,long &,long &,long &,long &,long &, long &, long &, CString &, CString &);

private:
	// testa existencia do banco de dados
	VOID dbStatus();

	char DRV[TAMCONN];
	char DBQ[TAMCONN];
	char UID[TAMCONN];
	char PWD[TAMCONN];
	char OCS[TAMCONN];
	char CMM[TAMCONN];
	char CTT[TAMCONN];
};

#endif // !defined(AFX_ADO_H__638C30F8_BB8E_491A_83D1_39349672C5DB__INCLUDED_)
