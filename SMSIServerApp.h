/*
'=======================================================================================
'= Arquivo: SMSIServerApp.h
'= Função:  Arquivo de cabecalho para a classe CSMSIServer, que trata de todas as operacoes
'=			do Robo, incluindo retornar ao Service Control Manager (SCM).
'=
'= Autor:   Getson Miranda
'= Data:    20/10/2003
'=======================================================================================
*/
// SMSIServerApp.h: interface for the CSMSIServerApp class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SMSISERVERAPP_H__0C16288D_5425_4850_AD54_4BE2552D6983__INCLUDED_)
#define AFX_SMSISERVERAPP_H__0C16288D_5425_4850_AD54_4BE2552D6983__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GLOBAL.H"
#include "ado.h"
#include "InitFile.h"
#include "DataHora.h"

/*
'=====================================================================
'=== CSMSIServerApp
'
'Objetivo:	Declarar os membros, metodos e variaveis da classe, principal da
'			aplicacao.
'
'Entrada.:	Nao possui, basta apenas uma chamada ao metodo ::Processamento()
'			desta classe para o robo inicializar as tarefas. Detalhar adequadamente
'			o arquivo INI, incluindo o caminho do banco de dados.
'			Procura:
'						o arquivo ini robo.ini e,
'						banco de dados RBDBD001.MDB
'          
'Saida...:	0 - saida com sucesso
'			1 - encerramento anormal do robo
'
'Retornos:	nao possui
'
'=====================================================================
*/
class CSMSIServerApp  
{
public:

	CSMSIServerApp();			// construtor da classe

	virtual ~CSMSIServerApp();	// destrutor

	long smsirun;				// ******

	long AppStatus;

	void Processamento();		// inicia e controla a execucao das tarefas do robo

	VOID CloseApp();

private:

	short ChkSrvSqlPgTst(long);

	// Trap estou ativo . . .
	CString msgTrapVivo;		// BD  mensagem informando Robo vivo
	CString msgTrapEnd;			// BD  msg trap
	COleDateTime tmTrapVivo;	// INT tempo do ultimo envio de trap
	long waitTrapVivo;			// BD  intervalo em segundo, envio de trap Vivo
	VOID TrapToAtivo();			// INT envia trap estou ativo

	VOID EnviarLog(DWORD, char *, char *);

	DWORD AtuParmsGeral();					// atualiza parametros gerais

	void Verificacao();					// tratamento e verificacao de todos os servicos

	DWORD ServidorCHK(long);				// determina se o servidor eh valido para trabalhar com as paginas HTML

	void AtuPgComp(long);					// atauliza as informacoes das paginas de comparacao

	void AtuPgTeste(long);					// atualiza as informacoes das paginas de teste na tabela interna do robo

	// Traps
	void EnviarTrapPagTeste(long, char *, short)	// envia trap referente as paginas de teste
		;
	void EnviarTrapServidor(long, char *);	// envia trap referente aos servidores

	void EnviarTrapPagComp(long, char *);	// envia trap referente as paginas de comparacao

	// Logs
	void EnviarLogTST(float, char *, char *, long);	// Grava log das paginas de teste

	void EnviarLogServ(float, char *, long);				// Grava log dos servidores

	void	EnviarLogPg(float, char *, char *, long);			// Grava log das paginas de comparacao

	DWORD	LeituraParametros();
	DWORD	SendTrap( char *, char * );
	void	AtualizaServidor(long);
	DWORD	CHKServidores();			// ping servidores endereco ip ou nome
	void	AtualizaParametros();
	DWORD	AlimentaDadosServs();
	DWORD	AlimentaPagsComp();
	DWORD	AlimentaPagsTestes();

	VOID	PingServidor(long &);
	VOID	ComparaPagina(long &);
	VOID	TestaPagina(long &);
	
	CAdo		oAdo;						// banco de dados

	CInitFile	oIni;						// arquivo INI

	DataHora	oDt;						// formatos de data e hora

	char ver[50];							// parametro para dll trap
	char pub[50];							// parametro para dll trap
	char ent[50];							// parametro para dll trap
	char gen[50];							// parametro para dll trap
	char receiv[50];						// parametro para dll trap
	char port[50];							// parametro para dll trap

	long nummaxthread;
	long numdispthread;
	long tempoverificacao;
	long tempoverificcerro;
	long tempotmoutserv;
	long tempotmoutpgcomp;
	long tempotmoutpgtst;
	long tempoatualparm;
	COleDateTime ultatuparm;

	long atuparm;

	short StDLLTrap;
};

#endif // !defined(AFX_SMSISERVERAPP_H__0C16288D_5425_4850_AD54_4BE2552D6983__INCLUDED_)
