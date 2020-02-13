// ado.cpp: implementation of the CAdo class.
//
//////////////////////////////////////////////////////////////////////

/*
'=======================================================================================
'= Arquivo: ado.cpp
'= Função:  tratamento do banco de dados
'= Autor:   Getson Miranda
'= Data:    21/10/2003
'=======================================================================================
*/

#include "stdafx.h"
#include "ado.h"
#include <adoid.h>
#include <adoint.h>
#include "InitFile.h"
#include "LogFile.h"
#include "DataHora.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#import "c:\program files\common files\system\ado\msado15.dll" rename ( "EOF", "adoEOF" ) /* BANCO DE DADOS*/
//#import "C:\Arquivos de programas\Arquivos comuns\System\ado\msado15.dll" rename ( "EOF", "adoEOF" ) /* BANCO DE DADOS*/ /* lykiene */

// estrutura para inicializacao do objeto recordset
// _variant_t vValue;

ADODB::_ConnectionPtr con = NULL;	// conexao generica com banco de dados
ADODB::Recordset21Ptr rec = NULL;	// conexao generica com banco de dados
ADODB::_ConnectionPtr con1 = NULL; 	// conexao com banco de dados atravez de threads
ADODB::Recordset21Ptr rec1 = NULL;	// conexao com banco de dados atravez de threads
ADODB::_ConnectionPtr con6 = NULL;
ADODB::Recordset21Ptr rec6 = NULL;
ADODB::_ConnectionPtr con7 = NULL;
ADODB::Recordset21Ptr rec7 = NULL;

/*
'=====================================================================
'===		CAdo::CAdo()
'
'Objetivo:	Inicializar a classe de banco de dados, no momento em que 
'			ela é instanciada, na primeira execução do robo, ou chamadas 
'			feita pela thread que trata a página de comparação. Efetua 
'			uma leitura inicial ao arquivo INI, para leitura dos parâmetros 
'			de conexão com o Banco de Dados. Também inicializa interface de OLE, 
'			para leitura de registros entre a camada do banco de dados e as 
'			variáveis do Robo.
'
'=====================================================================
*/
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CAdo::CAdo()
{
	LeArqIni();			// Leitura de parametros de conexao do arquivo INI

	CoInitialize(NULL); // OLE

	AdoStatus = 1;
}

/*
'=====================================================================
'===		CAdo::~CAdo()
'
'Objetivo:	É chamado no momento de descarte da classe, para finalizar 
'			a interface aberta entre o banco de dados e aplicação, 
'			encerrando a interface OLE.
'
'=====================================================================
*/
CAdo::~CAdo()
{
	CoUninitialize();	// OLE
}

/*
'=====================================================================
'=== LeArqIni()
'
'Objetivo:	Efetua leitura dos parametros de conexao do banco de dados
'			no arquivo de inicializacao.
'
'Entrada.:	nenhuma
'
'Saida...:	grava parametros de conexao de banco de dados em variaveis da classe
'			DRV = DRIVER [Microsoft Access Driver (*.mdb)]
'			DBQ = CAMINHO DO BD [C:\ROBO\BD\RBDBD001.MDB]
'			UID = usuario
'			PWD = senha
'			OCS = Outros Connection String
'			CMM = 600
'			CTT = 2000
'=====================================================================
*/
DWORD CAdo::LeArqIni()
{
	CInitFile oIniDB;	// Classe do arquivo de inicializacao

	DRV[0]='\0';
	DBQ[0]='\0';
	UID[0]='\0';
	PWD[0]='\0';
	OCS[0]='\0';
	CMM[0]='\0';
	CTT[0]='\0';

	if(oIniDB.GetDRIVERValue( DRV ))
	{
		EnviarLog(GetLastError(), 1, "CAdo::LeArqIni oIniDB.GetDRIVERValue", 0, 'N', MSGARQINI);
		return AdoStatus;
	}

	if(oIniDB.GetDBQValue( DBQ ))
	{
		EnviarLog(GetLastError(), 1, "CAdo::LeArqIni oIniDB.GetDBQValue", 0, 'N', MSGARQINI);
		return AdoStatus;
	}

	return 0;
}


/*
'=====================================================================
'=== Tabela de Servidores
'
'Objetivo:	recupera id, nome e endero ip de um servidor a ser testado 
'
'Entrada.:	nenhuma
'          
'Saida...:	
'
'Retornos:	0 sucesso
'			1 nao foi possivel criar uma conexao para acesso ao banco de dados
'			2	falha na conexao com o banco de dados
'				senha ou usuario invalido
'				caminho do banco de dados invalido
'				algum parametro no arquivo ini esta incorreto
'			3 nao conseguiu acesso a tabela ou nao ha registros
'			4 erro na requery
'=====================================================================
*/
DWORD CAdo::AbreTabelaServidores()
{
	HRESULT hr = S_OK;

	hr = con1.CreateInstance(__uuidof(ADODB::Connection));
	if(hr != S_OK)
	{
		EnviarLog(GetLastError(), 0, "CAdo::AbreTabelaServidores - ADODB::Connection", 0, 'N', MSGSERVIDOR);
		return AdoStatus;
	}

	dbStatus();
	
	char *strcon;
	strcon = (char*)malloc(TAMCONN);
	if(strcon == NULL)
	{
		EnviarLog(GetLastError(), 0, "CAdo::AbreTabelaServidores - Alocando memoria para strcon", 0, 'N', MSGSERVIDOR);
		return AdoStatus;
	}
	strcon[0]='\0';

	sprintf(strcon,"DRIVER=%s;DBQ=%s",DRV,DBQ);

	try
	{
		hr = con1->Open(strcon,"","",0);
	}
	catch(_com_error e)
	{
		TCHAR err[255];
		strcpy(err, e.ErrorMessage());
		EnviarLog(4313, 1, "CAdo::AbreTabelaServidores 01", 0, 'N', MSGSERVIDOR);
		return AdoStatus;
	}

	if(hr != S_OK)
	{
		EnviarLog(GetLastError(), 0, "CAdo::AbreTabelaServidores - Sem acesso Tabelas BD", 0, 'N', MSGSERVIDOR);
		return AdoStatus;
	}

	char strCmd[] = "SELECT TRBSERVR.COD_SERVIDOR, TRBSERVR.NOM_SERVIDOR, TRBSERVR.NUM_IP_SERVIDOR FROM TRBSERVR;";

	VARIANT *vRecord = NULL;

	rec1 = con1->Execute(strCmd,vRecord,1);

	if(rec1->adoEOF)
	{
		EnviarLog(GetLastError(), 0, "CAdo::AbreTabelaServidores - Sem retorno de registro", 0, 'N', MSGSERVIDOR);
		rec1->Close();
		con1->Close();
		return AdoStatus;
	}

	rec1->MoveFirst();

	free(strcon);

	return 0;
}

/*
'=====================================================================
'===		RegistroServidores2(long &cd, char *ip, char *nm)
'
'Objetivo:	Recupera informacoes iniciais sobre os servidores para teste
'
'Entrada.:	cd = codigo referente ao servidor na tabela de servidores
'    e		ip = emdereco IP do servidor
'Saida...:	nm = nome do servidor
'
'Retornos:	0 = final de query
'			1 = um registro foi lido
'=====================================================================
*/
// Servidores
DWORD CAdo::RegistroServidores2(long &cd, char *ip, char *nm)
{
	CString p1, p2;

 	if(rec1->adoEOF)
	{
		rec1->Close();
		con1->Close();
		return 0;
	}

	ADODB::Field20Ptr nome;

	_variant_t vValue;

	nome = rec1->Fields->GetItem("COD_SERVIDOR");
	vValue = nome->Value;
	cd = V_I2(&vValue); 

	// ip do servidor
	nome = rec1->Fields->GetItem("NUM_IP_SERVIDOR");
	vValue = nome->Value;
	p1 = V_BSTR(&vValue);
	strcpy(ip, p1);

	// nome do servidor
	nome = rec1->Fields->GetItem("NOM_SERVIDOR");
	vValue = nome->Value;
	p2 = V_BSTR(&vValue);
	strcpy(nm, p2);
	
	rec1->MoveNext();

	return 1;
}


/*
'=====================================================================
'===		TRBSERVR - tbUpdServ(long cd, char *nm, char *ip)
'
'Objetivo:	atualiza dado dos servidores, onde o tempo e determinado por tempoatualparm, 
'			tempo para uma nova leitura do arquivo de inicializacao e banco de dados.
'
'Entrada.:	cd = codigo do servidor na tabela de servidores
'   e       nm = nome do servidor
'Saida...:	ip = endereco Ip do servidor
'
'Retornos:	0  = sucesso, registro lido com sucesso
'			10 = problema de conexao com o banco de dados
'			11 = problema de conexao com a tabela de servidores
'			1  = nenhum registro foi recuperado
'=====================================================================
*/
DWORD CAdo::tbUpdServ(long cd, char *nm, char *ip)
{
	HRESULT hr = S_OK;
	ADODB::_ConnectionPtr con5 = NULL; 	// conexao com banco de dados atravez de threads
	ADODB::Recordset21Ptr rec5 = NULL;	// conexao com banco de dados atravez de threads

	hr = con5.CreateInstance(__uuidof(ADODB::Connection));
	if(hr != S_OK)
	{
		EnviarLog(GetLastError(), 0, "CAdo::tbUpdServ - ADODB::Connection", cd, 'N', MSGSERVIDOR);
		return AdoStatus;
	}

	dbStatus();
	
	char *strcon;
	strcon = (char*)malloc(TAMCONN);
	if(strcon == NULL)
	{
		EnviarLog(GetLastError(), 0, "CAdo::tbUpdServ - Alocando memoria para strcon", cd, 'N', MSGSERVIDOR);
		return AdoStatus;
	}
	strcon[0]='\0';

	sprintf(strcon,"DRIVER=%s;DBQ=%s",DRV,DBQ);

	try
	{
		hr = con5->Open(strcon,"","",0);
	}
	catch(_com_error e)
	{
		TCHAR err[255];
		strcpy(err, e.ErrorMessage());
		EnviarLog(4313, 1, "CAdo::tbUpdServ 02", cd, 'N', MSGSERVIDOR);
		return AdoStatus;
	}

	if(hr != S_OK)
	{
		EnviarLog(GetLastError(), 0, "CAdo::tbUpdServ - Sem acesso Tabelas BD", cd, 'N', MSGSERVIDOR);
		return AdoStatus;
	}

	char *strCmd;
	strCmd = (char*)malloc(512);
	strCmd[0]='\0';

	sprintf(strCmd,"SELECT COD_SERVIDOR, NOM_SERVIDOR, NUM_IP_SERVIDOR FROM TRBSERVR WHERE (((COD_SERVIDOR)=%d));", cd);

	VARIANT *vRecord = NULL;

	rec5 = con5->Execute(strCmd,vRecord,1);

	if(rec5->adoEOF)
	{
		EnviarLog(GetLastError(), 0, "CAdo::tbUpdServ - Sem retorno de registro", cd, 'N', MSGSERVIDOR);
		rec5->Close();
		con5->Close();
		return AdoStatus;
	}

	ADODB::Field20Ptr nome;

	_variant_t vValue;

	CString p1, p2;

	// nome do servidor
	nome = rec5->Fields->GetItem("NOM_SERVIDOR");
	vValue = nome->Value;
	p1 = V_BSTR(&vValue);
	strcpy(nm, p1);

	// ip do servidor
	nome = rec5->Fields->GetItem("NUM_IP_SERVIDOR");
	vValue = nome->Value;
	p2 = V_BSTR(&vValue);
	strcpy(ip, p2);

	rec5->Close();
	con5->Close();

	free(strcon);
	free(strCmd);

	return 0;
}



/*
'=====================================================================
'===		TRBSVRPG - AbreTabelaPgComp()
'
'Objetivo:	Realiza abertura da tabela de paginas de comparacao do site
'
'Entrada.:	nenhuma
'          
'Saida...:	nenhuma
'
'Retornos:	0 sucesso
'			10 = problema de conexao com o banco de dados
'			11 = problema de conexao com a tabela de servidores
'=====================================================================
*/
DWORD CAdo::AbreTabelaPgComp()
{
	HRESULT hr = S_OK;

	hr = con1.CreateInstance(__uuidof(ADODB::Connection));
	if(hr != S_OK)
	{
		EnviarLog(GetLastError(), 0, "CAdo::AbreTabelaPgComp - ADODB::Connection", 0, 'N', MSGPAG_COMP);
		return AdoStatus;
	}

	dbStatus();
	
	char *strcon;
	strcon = (char*)malloc(TAMCONN);
	if(strcon == NULL)
	{
		EnviarLog(GetLastError(), 0, "CAdo::AbreTabelaPgComp - Alocando memoria para strcon", 0, 'N', MSGPAG_COMP);;
		return AdoStatus;
	}
	strcon[0]='\0';

	sprintf(strcon,"DRIVER=%s;DBQ=%s",DRV,DBQ);

	try
	{
		hr = con1->Open(strcon,"","",0);
	}
	catch(_com_error e)
	{
		TCHAR err[255];
		strcpy(err, e.ErrorMessage());
		EnviarLog(4313, 1, "CAdo::AbreTabelaPgComp 03", 0, 'N', MSGPAG_COMP);
		return AdoStatus;
	}

	if(hr != S_OK)
	{
		EnviarLog(GetLastError(), 0, "CAdo::AbreTabelaPgComp - Sem acesso Tabelas BD", 0, 'N', MSGPAG_COMP);
		return AdoStatus;
	}

	char strCmd[] = "SELECT TRBSERVR.NOM_SERVIDOR, TRBSVRPG.DSC_URL_PAGINA_SVR, TRBSVRPG.COD_SERVIDOR,	TRBSVRPG.COD_PAGINA_SVR	FROM TRBSERVR INNER JOIN TRBSVRPG ON TRBSERVR.COD_SERVIDOR = TRBSVRPG.COD_SERVIDOR;";

	VARIANT *vRecord = NULL;

	rec1 = con1->Execute(strCmd,vRecord,1);

	if(rec1->adoEOF)
	{
		EnviarLog(GetLastError(), 0, "CAdo::AbreTabelaPgComp - Sem retorno de registro", 0, 'N', MSGPAG_COMP);
		rec1->Close();
		con1->Close();
		return AdoStatus;
	}

	rec1->MoveFirst();

	free(strcon);

	return 0;
}

/*
'=====================================================================
'===	TRBSVRPG - RgTbPagComp(long &cd, long &cd_serv, char *url, char *nm)
'
'Objetivo:	retornar registros das paginas de comparcao
'
'Entrada.:	cd = codigo da pagina na tabela de paginas de comparacao
'   e       cd_serv = codigo do servidor na tabela de servidores
'Saida...:	url = url para recuperacao da pagina
'			nm = nome do servidor
'
'Retornos:	0 = nao ha registros para leitura
'			1 = foi lido um registro com sucesso
'=====================================================================
*/
DWORD CAdo::RgTbPagComp(long &cd, long &cd_serv, char *url, char *nm)
{
 	if(rec1->adoEOF)
	{
		rec1->Close();
		con1->Close();
		return 0;
	}

	ADODB::Field20Ptr nome;

	_variant_t vValue;

	// codigo do servidor
	nome = rec1->Fields->GetItem("COD_SERVIDOR");
	vValue = nome->Value;
	cd_serv = V_I2(&vValue); 

	// codigo da pagina
	nome = rec1->Fields->GetItem("COD_PAGINA_SVR");
	vValue = nome->Value;
	cd = V_I2(&vValue); 

	CString p1, p2;

	// nome do servidor
	nome = rec1->Fields->GetItem("NOM_SERVIDOR");
	vValue = nome->Value;
	p1 = V_BSTR(&vValue);
	strcpy(nm,p1);

	// url
	nome = rec1->Fields->GetItem("DSC_URL_PAGINA_SVR");
	vValue = nome->Value;
	p2 = V_BSTR(&vValue);
	strcpy(url,p2);
	
	rec1->MoveNext();

	return 1;
}


/*
'=====================================================================
'===		TRBPGTST - AbreTabelaPgTeste()
'
'Objetivo:	Realiza abertura da tabela de paginas de teste do site
'
'Entrada.:	nenhuma
'          
'Saida...:	nenhuma
'
'Retornos:	0 sucesso
'			10 = problema instanciando o banco de dados
'			11 = problema de conexao com o banco de dados ou a tabela
'			12 = registro(s) nao encontrado(s)
'=====================================================================
*/
DWORD CAdo::AbreTabelaPgTeste()
{
	HRESULT hr = S_OK;

	hr = con1.CreateInstance(__uuidof(ADODB::Connection));
	if(hr != S_OK)
	{
		EnviarLog(GetLastError(), 0, "CAdo::AbreTabelaPgTeste - ADODB::Connection", 0, 'N', MSGPAG_TEST);
		return AdoStatus;
	}

	dbStatus();
	
	char *strcon;
	strcon = (char*)malloc(TAMCONN);
	if(strcon == NULL)
	{
		EnviarLog(GetLastError(), 0, "CAdo::AbreTabelaPgTeste - Alocando memoria para strcon", 0, 'N', MSGPAG_TEST);
		return AdoStatus;
	}
	strcon[0]='\0';

	sprintf(strcon,"DRIVER=%s;DBQ=%s",DRV,DBQ);

	try
	{
		hr = con1->Open(strcon,"","",0);
	}
	catch(_com_error e)
	{
		TCHAR err[255];
		strcpy(err, e.ErrorMessage());
		EnviarLog(4313, 1, "CAdo::AbreTabelaPgTeste 04", 0, 'N', MSGPAG_TEST);
		return AdoStatus;
	}
	
	if(hr != S_OK)
	{
		EnviarLog(GetLastError(), 0, "CAdo::AbreTabelaPgTeste - Sem acesso Tabelas BD", 0, 'N', MSGPAG_TEST);
		return AdoStatus;
	}

	char strCmd[] = "SELECT TRBPGTST.COD_PAGINA_TESTE, TRBSERVR.COD_SERVIDOR, TRBPGTST.DSC_URL_PAGINA_TST, TRBSERVR.NOM_SERVIDOR FROM TRBSERVR INNER JOIN TRBPGTST ON TRBSERVR.COD_SERVIDOR = TRBPGTST.COD_SERVIDOR;";

	VARIANT *vRecord = NULL;

	rec1 = con1->Execute(strCmd,vRecord,1);

	if(rec1->adoEOF)
	{
		EnviarLog(GetLastError(), 0, "CAdo::AbreTabelaPgTeste - Sem retorno de registro", 0, 'N', MSGPAG_TEST);
		rec1->Close();
		con1->Close();
		return AdoStatus;
	}

	rec1->MoveFirst();

	free(strcon);

	return 0;
}

/*
'=====================================================================
'===		TRBPGTST - RgTbPagTeste(long &cd, long &cd_serv, char *url, char *nm)
'
'Objetivo:	retornar registros das paginas de comparcao
'
'Entrada.:	cd = codigo da pagina na tabela de paginas de comparacao
'   e       cd_serv = codigo do servidor na tabela de servidores
'Saida...:	url = url para recuperacao da pagina
'			nm = nome do servidor
'
'Retornos:	0 = nao ha registros para leitura
'			1 = foi lido um registro com sucesso
'=====================================================================
*/
DWORD CAdo::RgTbPagTeste(long &cd, long &cd_serv, char *url, char *nm)
{
 	if(rec1->adoEOF)
	{
		rec1->Close();
		con1->Close();
		return 0;
	}

	ADODB::Field20Ptr nome;

	_variant_t vValue;

	// codigo do servidor
	nome = rec1->Fields->GetItem("COD_SERVIDOR");
	vValue = nome->Value;
	cd_serv = V_I2(&vValue); 

	// codigo da pagina
	nome = rec1->Fields->GetItem("COD_PAGINA_TESTE");
	vValue = nome->Value;
	cd = V_I2(&vValue); 

	CString p1, p2;

	// nome do servidor
	nome = rec1->Fields->GetItem("NOM_SERVIDOR");
	vValue = nome->Value;
	p1 = V_BSTR(&vValue);
	strcpy(nm,p1);

	// url
	nome = rec1->Fields->GetItem("DSC_URL_PAGINA_TST");
	vValue = nome->Value;
	p2 = V_BSTR(&vValue);
	strcpy(url,p2);
	
	rec1->MoveNext();

	return 1;
}


/*
'=====================================================================
'===		TRBSVRPG - AtualizaPagComp(long cd, long &cd_serv, char *nm, char *url)
'
'Objetivo:	atualizar os registros de uma pagina de comparacao, conforme tempo determinado por,
'			tempoatualparm, tempo para uma nova leitura do arquivo de inicializacao e banco 
'			de dados
'
'Entrada.:	cd = codigo da pagina a ser recuperada na tabela
'
'Saida...:	url = url para recuperacao da pagina
'			nm = nome do servidor
'			cd_serv = codigo do servidor na tabela de servidores
'
'Retornos:	0  = um registro foi recuperado com sucesso
'			1  = nao ha registros para leitura referente a pagina
'			10 = problema de conexao com o banco de dados
'			11 = problema de conexao com a tabela de servidores
'=====================================================================
*/
DWORD CAdo::AtualizaPagComp(long cd, long &cd_serv, char *nm, char *url)
{
	HRESULT hr = S_OK;

	hr = con.CreateInstance(__uuidof(ADODB::Connection));
	if(hr != S_OK)
	{
		EnviarLog(GetLastError(), 0, "CAdo::AtualizaPagComp - ADODB::Connection", cd, 'N', MSGPAG_COMP);
		return AdoStatus;
	}

	dbStatus();
	
	char *strcon;
	strcon = (char*)malloc(TAMCONN);
	if(strcon == NULL)
	{
		EnviarLog(GetLastError(), 0, "CAdo::AtualizaPagComp - Alocando memoria para strcon", cd, 'N', MSGPAG_COMP);
		return AdoStatus;
	}
	strcon[0]='\0';

	sprintf(strcon,"DRIVER=%s;DBQ=%s",DRV,DBQ);

	try
	{
		hr = con->Open(strcon,"","",0);
	}
	catch(_com_error e)
	{
		TCHAR err[255];
		strcpy(err, e.ErrorMessage());
		EnviarLog(4313, 1, "CAdo::AtualizaPagComp 05", cd, 'N', MSGPAG_COMP);
		return AdoStatus;
	}

	if(hr != S_OK)
	{
		EnviarLog(GetLastError(), 0, "CAdo::AtualizaPagComp - Sem acesso Tabelas BD", cd, 'N', MSGPAG_COMP);
		return AdoStatus;
	}

	char *strCmd;
	strCmd = (char*)malloc(512);
	strCmd[0]='\0';

	sprintf(strCmd,"SELECT TRBSVRPG.COD_SERVIDOR, TRBSERVR.NOM_SERVIDOR, TRBSVRPG.DSC_URL_PAGINA_SVR FROM (TRBSERVR INNER JOIN TRBSVRPG ON TRBSERVR.COD_SERVIDOR = TRBSVRPG.COD_SERVIDOR) INNER JOIN TRBSPGCP ON TRBSVRPG.COD_PAGINA_SVR = TRBSPGCP.COD_PAGINA_SVR WHERE (((TRBSVRPG.COD_PAGINA_SVR)=%d));",cd);

	VARIANT *vRecord = NULL;

	rec = con->Execute(strCmd,vRecord,1);

	if(rec->adoEOF)
	{
		EnviarLog(GetLastError(), 0, "CAdo::AtualizaPagComp - Sem retorno de registro", cd, 'N', MSGPAG_COMP);
		rec->Close();
		con->Close();
		return AdoStatus;
	}

	ADODB::Field20Ptr nome;

	_variant_t vValue;

	// codigo do servidor
	nome = rec->Fields->GetItem("COD_SERVIDOR");
	vValue = nome->Value;
	cd_serv = V_I2(&vValue);

	CString p1, p2;

	// nome do servidor
	nome = rec->Fields->GetItem("NOM_SERVIDOR");
	vValue = nome->Value;
	p1 = V_BSTR(&vValue);
	strcpy(nm,p1);

	// ip do servidor
	nome = rec->Fields->GetItem("DSC_URL_PAGINA_SVR");
	vValue = nome->Value;
	p2 = V_BSTR(&vValue);
	strcpy(url, p2);

	rec->Close();
	con->Close();

	free(strcon);
	free(strCmd);

	return 0;
}


/*
'=====================================================================
'===		TRBSVRPG - AtualizaPagTeste(long cd, long &cd_serv, char *nm, char *url)
'
'Objetivo:	atualizar os registros de uma pagina de teste, conforme tempo determinado por,
'			tempoatualparm, tempo para uma nova leitura do arquivo de inicializacao e banco 
'			de dados
'
'Entrada.:	cd = codigo da pagina a ser recuperada na tabela
'
'Saida...:	url = url para recuperacao da pagina
'			nm = nome do servidor
'			cd_serv = codigo do servidor na tabela de servidores
'
'Retornos:	0  = um registro foi recuperado com sucesso
'			1  = nao ha registros para leitura referente a pagina
'			10 = problema de conexao com o banco de dados
'			11 = problema de conexao com a tabela de servidores
'=====================================================================
*/
DWORD CAdo::AtualizaPagTeste(long cd, long &cd_serv, char *nm, char *url)
{
	HRESULT hr = S_OK;

	hr = con.CreateInstance(__uuidof(ADODB::Connection));
	if(hr != S_OK)
	{
		EnviarLog(GetLastError(), 0, "CAdo::AtualizaPagTeste - ADODB::Connection", cd, 'N', MSGPAG_TEST);
		return AdoStatus;
	}

	dbStatus();
	
	char *strcon;
	strcon = (char*)malloc(TAMCONN);
	if(strcon == NULL)
	{
		EnviarLog(GetLastError(), 0, "CAdo::AtualizaPagTeste - Alocando memoria para strcon", cd, 'N', MSGPAG_TEST);
		return AdoStatus;
	}
	strcon[0]='\0';

	sprintf(strcon,"DRIVER=%s;DBQ=%s",DRV,DBQ);

	try
	{
		hr = con->Open(strcon,"","",0);
	}
	catch(_com_error e)
	{
		TCHAR err[255];
		strcpy(err, e.ErrorMessage());
		EnviarLog(4313, 1, "CAdo::AtualizaPagTeste 06", cd, 'N', MSGPAG_TEST);
		return AdoStatus;
	}

	if(hr != S_OK)
	{
		EnviarLog(GetLastError(), 0, "CAdo::AtualizaPagTeste - Sem acesso Tabelas BD", cd, 'N', MSGPAG_TEST);
		return AdoStatus;
	}

	char *strCmd;
	strCmd = (char*)malloc(512);
	strCmd[0]='\0';

	sprintf(strCmd,"SELECT TRBSERVR.COD_SERVIDOR, TRBSERVR.NOM_SERVIDOR, TRBPGTST.DSC_URL_PAGINA_TST FROM TRBSERVR INNER JOIN TRBPGTST ON TRBSERVR.COD_SERVIDOR = TRBPGTST.COD_SERVIDOR WHERE (((TRBPGTST.COD_PAGINA_TESTE)=%d));",cd);

	VARIANT *vRecord = NULL;

	rec = con->Execute(strCmd,vRecord,1);

	if(rec->adoEOF)
	{
		EnviarLog(GetLastError(), 0, "CAdo::AtualizaPagTeste - Sem retorno de registro", cd, 'N', MSGPAG_TEST);
		rec->Close();
		con->Close();
		return AdoStatus;
	}

	ADODB::Field20Ptr nome;

	_variant_t vValue;

	// codigo do servidor
	nome = rec->Fields->GetItem("COD_SERVIDOR");
	vValue = nome->Value;
	cd_serv = V_I2(&vValue);

	CString p1, p2;

	// nome do servidor
	nome = rec->Fields->GetItem("NOM_SERVIDOR");
	vValue = nome->Value;
	p1 = V_BSTR(&vValue);
	strcpy(nm,p1);

	// ip do servidor
	nome = rec->Fields->GetItem("DSC_URL_PAGINA_TST");
	vValue = nome->Value;
	p2 = V_BSTR(&vValue);
	strcpy(url, p2);

	rec->Close();
	con->Close();

	free(strcon);
	free(strCmd);

	return 0;
}


/*
'=====================================================================
'===		tbTrapErroSrv(long cd, char *msg, char *trp, char *nm)
'
'Objetivo:	recupera mensagem de trap para um servidor
'
'Entrada.:	cd = codigo do servidor
'
'Saida...:	msg	=	"ERRGE" Erro generico ou,
'					"TMOUT" Erro de time out
'			trp = codigo da mensagem de trap [1.3.6.1.4.1.105.0.4]
'			nm  = nome do servidor
'
'Retornos:	0  = um registro foi recuperado com sucesso
'			1  = nao ha registros para leitura referente a pagina
'			10 = problema de conexao com o banco de dados
'			11 = problema de conexao com a tabela de servidores
'=====================================================================
*/
DWORD CAdo::tbTrapErroSrv(long cd, char *msg, char *trp, char *nm)
{
	HRESULT hr = S_OK;
	ADODB::_ConnectionPtr con4 = NULL; 	// conexao com banco de dados atravez de threads
	ADODB::Recordset21Ptr rec4 = NULL;	// conexao com banco de dados atravez de threads

	hr = con4.CreateInstance(__uuidof(ADODB::Connection));
	if(hr != S_OK)
	{
		EnviarLog(GetLastError(), 0, "CAdo::tbTrapErroSrv - ADODB::Connection", cd, 'N', MSGTRAPSERV);
		return AdoStatus;
	}

	dbStatus();
	
	char *strcon;
	strcon = (char*)malloc(TAMCONN);
	if(strcon == NULL)
	{
		EnviarLog(GetLastError(), 0, "CAdo::tbTrapErroSrv - Alocando memoria para strcon", cd, 'N', MSGTRAPSERV);
		return AdoStatus;
	}
	strcon[0]='\0';

	sprintf(strcon,"DRIVER=%s;DBQ=%s",DRV,DBQ);

	try
	{
		hr = con4->Open(strcon,"","",0);
	}
	catch(_com_error e)
	{
		TCHAR err[255];
		strcpy(err, e.ErrorMessage());
		EnviarLog(4313, 1, "CAdo::tbTrapErroSrv 07", cd, 'N', MSGTRAPSERV);
		return AdoStatus;
	}

	if(hr != S_OK)
	{
		EnviarLog(GetLastError(), 0, "CAdo::tbTrapErroSrv - Sem acesso Tabelas BD", cd, 'N', MSGTRAPSERV);
		return AdoStatus;
	}

	char *strCmd;
	strCmd = (char*)malloc(512);
	strCmd[0]='\0';

	sprintf(strCmd,"SELECT TRBTRAPS.COD_MENSAGEM_TRAP, TRBSERVR.NOM_SERVIDOR FROM TRBSERVR INNER JOIN (TRBTRAPS INNER JOIN TRBSVRER ON TRBTRAPS.COD_TRAP = TRBSVRER.COD_TRAP) ON TRBSERVR.COD_SERVIDOR = TRBSVRER.COD_SERVIDOR WHERE (((TRBSVRER.COD_SERVIDOR)=%d) AND ((TRBSVRER.COD_TIPO_ERRO)='%s'));", cd, msg);

	VARIANT *vRecord = NULL;

	rec4 = con4->Execute(strCmd,vRecord,1);

	if(rec4->adoEOF)
	{
		rec4->Close();
		con4->Close();
		return 15;
	}

	ADODB::Field20Ptr nome;

	_variant_t vValue;

	CString p1, p2;

	// codigo da trap
	nome = rec4->Fields->GetItem("COD_MENSAGEM_TRAP");
	vValue = nome->Value;
	p1 = V_BSTR(&vValue);
	strcpy(trp,p1);

	// nome do servidor
	nome = rec4->Fields->GetItem("NOM_SERVIDOR");
	vValue = nome->Value;
	p2 = V_BSTR(&vValue);
	strcpy(nm, p2);

	rec4->Close();
	con4->Close();

	free(strcon);
	free(strCmd);

	return 0;
}

/*
'=====================================================================
'===		tbTrapPgComp(long cd, char *msg, char *trp, char *nm)
'
'Objetivo:	recupera mensagem de trap para uma pagina de comparacao
'
'Entrada.:	cd = codigo do servidor
'
'Saida...:	msg	=	"ERRGE" Erro generico ou,
'					"TMOUT" Erro de time out
'			trp = codigo da mensagem de trap [1.3.6.1.4.1.105.0.4]
'			nm  = nome do servidor
'
'Retornos:	0  = um registro foi recuperado com sucesso
'			1  = nao ha registros para leitura referente a pagina
'			10 = problema de conexao com o banco de dados
'			11 = problema de conexao com a tabela de servidores
'=====================================================================
*/
DWORD CAdo::tbTrapPgComp(long cd, char *msg, char *trp, char *nm)
{
	HRESULT hr = S_OK;

	hr = con.CreateInstance(__uuidof(ADODB::Connection));
	if(hr != S_OK)
	{
		EnviarLog(GetLastError(), 0, "CAdo::tbTrapPgComp - ADODB::Connection", cd, 'N', MSGTRAPPCMP);
		return AdoStatus;
	}

	dbStatus();
	
	char *strcon;
	strcon = (char*)malloc(TAMCONN);
	if(strcon == NULL)
	{
		EnviarLog(GetLastError(), 0, "CAdo::tbTrapPgComp - Alocando memoria para strcon", cd, 'N', MSGTRAPPCMP);
		return AdoStatus;
	}
	strcon[0]='\0';

	sprintf(strcon,"DRIVER=%s;DBQ=%s",DRV,DBQ);

	try
	{
		hr = con->Open(strcon,"","",0);
	}
	catch(_com_error e)
	{
		TCHAR err[255];
		strcpy(err, e.ErrorMessage());
		EnviarLog(4313, 1, "CAdo::tbTrapPgComp 08", cd, 'N', MSGTRAPPCMP);
		return AdoStatus;
	}

	if(hr != S_OK)
	{
		EnviarLog(GetLastError(), 0, "CAdo::tbTrapPgComp - Sem acesso Tabelas BD", cd, 'N', MSGTRAPPCMP);
		return AdoStatus;
	}

	char *strCmd;
	strCmd = (char*)malloc(512);
	strCmd[0]='\0';

	sprintf(strCmd,"SELECT TRBTRAPS.COD_MENSAGEM_TRAP, TRBSERVR.NOM_SERVIDOR FROM TRBSERVR INNER JOIN (TRBTRAPS INNER JOIN (TRBSVRPG INNER JOIN TRBSPGER ON TRBSVRPG.COD_PAGINA_SVR = TRBSPGER.COD_PAGINA_SVR) ON TRBTRAPS.COD_TRAP = TRBSPGER.COD_TRAP) ON TRBSERVR.COD_SERVIDOR = TRBSVRPG.COD_SERVIDOR WHERE (((TRBSPGER.COD_PAGINA_SVR)=%d) AND ((TRBSPGER.COD_TIPO_ERRO)='%s'));", cd, msg);

	VARIANT *vRecord = NULL;

	rec = con->Execute(strCmd,vRecord,1);

	if(rec->adoEOF)
	{
		EnviarLog(GetLastError(), 0, "CAdo::tbTrapPgComp - Sem retorno de registro", cd, 'N', MSGTRAPPCMP);
		rec->Close();
		con->Close();
		return AdoStatus;
	}

	ADODB::Field20Ptr nome;

	_variant_t vValue;

	CString p1, p2;

	// codigo da trap
	nome = rec->Fields->GetItem("COD_MENSAGEM_TRAP");
	vValue = nome->Value;
	p1 = V_BSTR(&vValue);
	strcpy(trp,p1);

	// nome do servidor
	nome = rec->Fields->GetItem("NOM_SERVIDOR");
	vValue = nome->Value;
	p2 = V_BSTR(&vValue);
	strcpy(nm, p2);

	rec->Close();
	con->Close();

	free(strcon);
	free(strCmd);

	return 0;
}


/*
'=====================================================================
'===		tbTrapPgTeste(long cd, char *msg, char *trp, char *nm)
'
'Objetivo:	recupera mensagem de trap para uma pagina de teste
'
'Entrada.:	cd = codigo do servidor
'
'Saida...:	msg	=	"ERRGE" Erro generico ou,
'					"TMOUT" Erro de time out
'			trp = codigo da mensagem de trap [1.3.6.1.4.1.105.0.4]
'			nm  = nome do servidor
'
'Retornos:	0  = um registro foi recuperado com sucesso
'			1  = nao ha registros para leitura referente a pagina
'			10 = problema de conexao com o banco de dados
'			11 = problema de conexao com a tabela de servidores
'=====================================================================
*/
DWORD CAdo::tbTrapPgTeste(long cd, char *msg, char *trp, char *nm)
{
	HRESULT hr = S_OK;

	hr = con.CreateInstance(__uuidof(ADODB::Connection));
	if(hr != S_OK)
	{
		EnviarLog(GetLastError(), 0, "CAdo::tbTrapPgTeste - ADODB::Connection", cd, 'N', MSGTRAPPTST);
		return AdoStatus;
	}

	dbStatus();
	
	char *strcon;
	strcon = (char*)malloc(TAMCONN);
	if(strcon == NULL)
	{
		EnviarLog(GetLastError(), 0, "CAdo::tbTrapPgTeste - Alocando memoria para strcon", cd, 'N', MSGTRAPPTST);
		return AdoStatus;
	}
	strcon[0]='\0';

	sprintf(strcon,"DRIVER=%s;DBQ=%s",DRV,DBQ);

	try
	{
		hr = con->Open(strcon,"","",0);
	}
	catch(_com_error e)
	{
		TCHAR err[255];
		strcpy(err, e.ErrorMessage());
		EnviarLog(4313, 1, "CAdo::tbTrapPgTeste 09", cd, 'N', MSGTRAPPTST);
		return AdoStatus;
	}

	if(hr != S_OK)
	{
		EnviarLog(GetLastError(), 0, "CAdo::tbTrapPgTeste - Sem acesso Tabelas BD", cd, 'N', MSGTRAPPTST);
		return AdoStatus;
	}

	char *strCmd;
	strCmd = (char*)malloc(512);
	strCmd[0]='\0';

	sprintf(strCmd,"SELECT TRBTRAPS.COD_MENSAGEM_TRAP, TRBSERVR.NOM_SERVIDOR FROM TRBTRAPS INNER JOIN (TRBSERVR INNER JOIN (TRBPGTST INNER JOIN TRBPTEER ON TRBPGTST.COD_PAGINA_TESTE = TRBPTEER.COD_PAGINA_TESTE) ON TRBSERVR.COD_SERVIDOR = TRBPGTST.COD_SERVIDOR) ON TRBTRAPS.COD_TRAP = TRBPTEER.COD_TRAP WHERE (((TRBPTEER.COD_PAGINA_TESTE)=%d) AND ((TRBPTEER.COD_TIPO_ERRO)='%s'));", cd, msg);

	VARIANT *vRecord = NULL;

	rec = con->Execute(strCmd,vRecord,1);

	if(rec->adoEOF)
	{
		EnviarLog(GetLastError(), 0, "CAdo::tbTrapPgTeste - Sem retorno de registro", cd, 'N', MSGTRAPPTST);
		rec->Close();
		con->Close();
		return AdoStatus;
	}

	ADODB::Field20Ptr nome;

	_variant_t vValue;

	CString p1, p2;

	// codigo da trap
	nome = rec->Fields->GetItem("COD_MENSAGEM_TRAP");
	vValue = nome->Value;
	p1 = V_BSTR(&vValue);
	strcpy(trp,p1);

	// nome do servidor
	nome = rec->Fields->GetItem("NOM_SERVIDOR");
	vValue = nome->Value;
	p2 = V_BSTR(&vValue);
	strcpy(nm, p2);

	rec->Close();
	con->Close();

	free(strcon);
	free(strCmd);

	return 0;
}


/*
'=====================================================================
'===		rgPgCmp(long cd, char *txt, char *url, long &tam, long reg)
'
'Objetivo:	recuperar url e texto de uma pagina para comparacao
'
'Entrada.:	cd  = codigo da pagina de comparacao
'			reg = numero do registro a ser devolvido pelo metodo
'
'Saida...:	txt	= texto da pagina a ser comparado com a url
'			url = url para recuperacao da pagina
'			tam = tamanho da pagina a ser comparada
'
'Retornos:	0   = um registro foi recuperado com sucesso
'			1   = nao ha registros para leitura referente a pagina
'			10  = problema de conexao com o banco de dados
'			11  = problema de conexao com a tabela de servidores
'=====================================================================
*/ /* ****** */
DWORD CAdo::rgPgCmp(long cd, TCHAR *txt, TCHAR *url, long &tam, long reg)
{
	HRESULT hr = S_OK;
	ADODB::_ConnectionPtr con2 = NULL;
	ADODB::Recordset21Ptr rec2 = NULL;
	CString p1, p2;

	hr = con2.CreateInstance(__uuidof(ADODB::Connection));
	if(hr != S_OK)
	{
		EnviarLog(GetLastError(), 0, "CAdo::rgPgCmp - ADODB::Connection", cd, 'N', MSGPAG_COMP);
		return AdoStatus;
	}

	dbStatus();
	
	char *strcon;
	strcon = (char*)malloc(TAMCONN);
	if(strcon == NULL)
	{
		EnviarLog(GetLastError(), 0, "CAdo::rgPgCmp - Alocando memoria para strcon", cd, 'N', MSGPAG_COMP);
		return AdoStatus;
	}
	strcon[0]='\0';

	sprintf(strcon,"DRIVER=%s;DBQ=%s",DRV,DBQ);

	try
	{
		hr = con2->Open(strcon,"","",0);
	}
	catch(_com_error e)
	{
		TCHAR err[255];
		strcpy(err, e.ErrorMessage());
		EnviarLog(4313, 1, "CAdo::rgPgCmp 10", cd, 'N', MSGPAG_COMP);
		return AdoStatus;
	}

	if(hr != S_OK)
	{
		EnviarLog(GetLastError(), 0, "CAdo::rgPgCmp - Sem acesso Tabelas BD", cd, 'N', MSGPAG_COMP);
		return AdoStatus;
	}

	char *strCmd;
	strCmd = (char*)malloc(512);
	strCmd[0]='\0';

	sprintf(strCmd,"SELECT TRBSPGCP.DSC_MDL_PGN_CMP, TRBSVRPG.DSC_URL_PAGINA_SVR, Len(TRBSPGCP.DSC_MDL_PGN_CMP) AS TAMANHO FROM TRBSVRPG INNER JOIN TRBSPGCP ON TRBSVRPG.COD_PAGINA_SVR = TRBSPGCP.COD_PAGINA_SVR WHERE (((TRBSPGCP.COD_PAGINA_SVR)=%d));", cd);

	VARIANT *vRecord = NULL;
    
	rec2 = con2->Execute(strCmd,vRecord,1);

	ADODB::Field20Ptr nome;

	_variant_t vValue;

	while(!rec2->adoEOF)
	{
		if(reg == 0)
		{
			// codigo do servidor
			nome = rec2->Fields->GetItem("TAMANHO");
			vValue = nome->Value;
			tam = V_I2(&vValue); 

			// nome do servidor
			nome = rec2->Fields->GetItem("DSC_URL_PAGINA_SVR");
			vValue = nome->Value;
			p1 = V_BSTR(&vValue);
			strcpy(url, p1);

			// campo memo
			nome = rec2->Fields->GetItem("DSC_MDL_PGN_CMP");
			vValue = nome->Value;
			p2 = V_BSTR(&vValue);
			strcpy(txt, p2);
			
			rec2->Close();
			con2->Close();
			return 1;
		}
		
		rec2->MoveNext();
		reg--;
	}
	
	rec2->Close();
	con2->Close();

	free(strcon);
	free(strCmd);
	
	return 0;
}

/*
'=====================================================================
'===		recParmGeral2(long &p1,long &p2,long &p3,long &p4,long &p5,long &p6)
'
'Objetivo:	Leitura dos parametros gerais do sistema na tabela TRBCFSIS
'
'Entrada.:	p1 = tempoverificacao, tempo padrao de verificacao dos servicos
'   e		p2 = tempoverificcerro, tempo de verificacao apos deteccao de erro
'Saida...:	p3 = nummaxthread, numero maximo de threads
'			p4 = tempotmoutserv, tempo de time out para thread do servidor
'			p5 = tempotmoutpgcomp, tempo de time out para as paginas de comparacao
'			p6 = tempotmoutpgtst, tempo de time out para as paginas de testes do site
'			p7 = tempoatualparm, tempo para uma nova leitura do arquivo de inicializacao e banco de dados
'
'=====================================================================
*/
DWORD CAdo::recParmGeral2(long &p1,long &p2,long &p3,long &p4,long &p5,long &p6, long &p7, long &p8, CString &p9, CString &p10)
{
	HRESULT hr = S_OK; // retem o resultado da ultima operacao
	ADODB::_ConnectionPtr con3 = NULL; 
	ADODB::Recordset21Ptr rec3 = NULL;
	
	// cria uma instancia com o banco de dados
	hr = con3.CreateInstance(__uuidof(ADODB::Connection));
	if(hr != S_OK)
	{
		EnviarLog(GetLastError(), 0, "CAdo::recParmGeral2 - ADODB::Connection", 0, 'N', MSGPARMGERA);
		return AdoStatus;
	}

	dbStatus();
	
	char *strcon;
	strcon = (char*)malloc(TAMCONN);
	if(strcon == NULL)
	{
		EnviarLog(GetLastError(), 0, "CAdo::recParmGeral2 - Alocando memoria para strcon", 0, 'N', MSGPARMGERA);
		return AdoStatus;
	}
	strcon[0]='\0';

	sprintf(strcon,"DRIVER=%s;DBQ=%s",DRV,DBQ);
	
	try
	{
		hr = con3->Open(strcon,"","",0);
	}
	catch(_com_error e)
	{
		TCHAR err[255];
		strcpy(err, e.ErrorMessage());
		EnviarLog(4313, 1, "CAdo::recParmGeral2 11", 0, 'N', MSGPARMGERA);
		return AdoStatus;
	}  

	if(hr != S_OK)
	{
		EnviarLog(GetLastError(), 0, "CAdo::recParmGeral2 - Sem acesso Tabelas BD", 0, 'N', MSGPARMGERA);
		return AdoStatus;
	}

	char strCmd[] = "SELECT QTD_SEGUNDO_VRF, QTD_SGD_VRF_ERRO, NUM_MAXIMO_THREAD, QTD_SGD_TMT_SVR, QTD_SGD_TMT_SPG, QTD_SGD_TMT_PTE, QTD_SGD_ATU_PAR, QTD_SEG_TRP_VVO, DSC_MSG_TRP_ATV, COD_MENSAGE_TRP FROM TRBCFSIS;";
	
	VARIANT *vRecord = NULL;

	rec3 = con3->Execute(strCmd,vRecord,1);

	if(rec3->adoEOF)
	{
		EnviarLog(GetLastError(), 0, "CAdo::recParmGeral2 - Sem retorno de registro", 0, 'N', MSGPARMGERA);
		rec3->Close();
		con3->Close();
		return AdoStatus;
	}

	ADODB::Field20Ptr nome;

	_variant_t vValue;

	nome = rec3->Fields->GetItem("QTD_SEGUNDO_VRF");
	vValue = nome->Value;
	p1 = V_I2(&vValue); 
	if(p1<=0)
	{
		EnviarLog(GetLastError(), 0, "CAdo::recParmGeral2 - Valor invalido QTD_SEGUNDO_VRF tabela TRBCFSIS", 0, 'N', MSGPARMGERA);
		p1 = 120;
	}
	
	nome = rec3->Fields->GetItem("QTD_SGD_VRF_ERRO");
	vValue = nome->Value;
	p2 = V_I2(&vValue); 
	if(p2<=0)
	{
		EnviarLog(GetLastError(), 0, "CAdo::recParmGeral2 - Valor invalido QTD_SGD_VRF_ERRO tabela TRBCFSIS", 0, 'N', MSGPARMGERA);
		p2 = 60;
	}

	nome = rec3->Fields->GetItem("NUM_MAXIMO_THREAD");
	vValue = nome->Value;
	p3 = V_I2(&vValue);
	if(p3<=0)
	{
		EnviarLog(GetLastError(), 0, "CAdo::recParmGeral2 - Valor invalido NUM_MAXIMO_THREAD tabela TRBCFSIS", 0, 'N', MSGPARMGERA);
		p3 = 12;
	}

	nome = rec3->Fields->GetItem("QTD_SGD_TMT_SVR");
	vValue = nome->Value;
	p4 = V_I2(&vValue); 
	if(p4<=0)
	{
		EnviarLog(GetLastError(), 0, "CAdo::recParmGeral2 - Valor invalido QTD_SGD_TMT_SVR tabela TRBCFSIS", 0, 'N', MSGPARMGERA);
		p4 = 30;
	}

	nome = rec3->Fields->GetItem("QTD_SGD_TMT_SPG");
	vValue = nome->Value;
	p5 = V_I2(&vValue); 
	if(p5<=0)
	{
		EnviarLog(GetLastError(), 0, "CAdo::recParmGeral2 - Valor invalido QTD_SGD_TMT_SPG tabela TRBCFSIS", 0, 'N', MSGPARMGERA);
		p5=30;
	}

	nome = rec3->Fields->GetItem("QTD_SGD_TMT_PTE");
	vValue = nome->Value;
	p6 = V_I2(&vValue); 
	if(p6<=0)
	{
		EnviarLog(GetLastError(), 0, "CAdo::recParmGeral2 - Valor invalido QTD_SGD_TMT_PTE tabela TRBCFSIS", 0, 'N', MSGPARMGERA);
		p6=30;
	}

	nome = rec3->Fields->GetItem("QTD_SGD_ATU_PAR");
	vValue = nome->Value;
	p7 = V_I2(&vValue); 
	if(p7<=0)
	{
		EnviarLog(GetLastError(), 0, "CAdo::recParmGeral2 - Valor invalido QTD_SGD_ATU_PAR tabela TRBCFSIS", 0, 'N', MSGPARMGERA);
		p7=600;
	}

	nome = rec3->Fields->GetItem("QTD_SEG_TRP_VVO");
	vValue = nome->Value;
	p8 = V_I2(&vValue); 
	if(p8<=0)
	{
		EnviarLog(GetLastError(), 0, "CAdo::recParmGeral2 - Valor invalido QTD_SEG_TRP_VVO tabela TRBCFSIS", 0, 'N', MSGPARMGERA);
		p8 = 60;
	}

	nome = rec3->Fields->GetItem("DSC_MSG_TRP_ATV");
	vValue = nome->Value;
	p9 = V_BSTR(&vValue);
	if( p9.IsEmpty() )
	{
		EnviarLog(GetLastError(), 0, "CAdo::recParmGeral2 - Valor invalido DSC_MSG_TRP_ATV tabela TRBCFSIS", 0, 'N', MSGPARMGERA);
		p9 = "ATIVO";
	}

	nome = rec3->Fields->GetItem("COD_MENSAGE_TRP");
	vValue = nome->Value;
	p10 = V_BSTR(&vValue);
	if( p10.IsEmpty() )
	{
		EnviarLog(GetLastError(), 0, "CAdo::recParmGeral2 - Valor invalido COD_MENSAGE_TRP tabela TRBCFSIS", 0, 'N', MSGPARMGERA);
		p10 = "";
	}

	rec3->Close();
	con3->Close();

	free(strcon);

	return 0;
}


/*
	Acessar um servidor SQL.

	Entrada		String de Conexao

	Saida		&dthr = Data e Hora recuperadas do Servidor SQL
					    dd/mm/yyyy-hh:mm:ss

	retorno		 0 = Sucesso
				!0 = Falha na conexao
*/
short CAdo::ConnectSQLServ(CString &strcon, long cd_serv, long cd_conn, char *err)
{
	HRESULT hr = S_OK;
	ADODB::_ConnectionPtr con7 = NULL; 
	ADODB::Recordset21Ptr rec7 = NULL;
	char strconn[TAMCONN];
	strcpy(strconn, strcon);
	
	// cria uma instancia com o banco de dados
	hr = con7.CreateInstance(__uuidof(ADODB::Connection));
	if(hr != S_OK)
	{
		EnviarLog(GetLastError(), 0, "CAdo::ConnectSQLServ - ADODB::Connection Erro na Pagina Teste e na conexao com Servidor SQL", cd_serv, 'N', MSGSQLSERV);
		return AdoStatus;
	}

	try
	{
		hr = con7->Open(strconn,"","",0);
	}
	catch(_com_error e)
	{
		TCHAR err[255];
		strcpy(err, e.ErrorMessage());
		EnviarLog(4313, 1, "CAdo::ConnectSQLServ 13 Erro na Pagina Teste e na conexao com Servidor SQL", cd_serv, 'N', MSGSQLSERV);
		return AdoStatus;
	}

	if(hr != S_OK)
	{
		EnviarLog(GetLastError(), 0, "CAdo::ConnectSQLServ - Sem acesso Tabelas BD Erro na Pagina Teste e na conexao com Servidor SQL", cd_serv, 'N', MSGSQLSERV);
		return AdoStatus;
	}

	char strCmd[] = "SELECT GETDATE() DATA;";
	
	VARIANT *vRecord = NULL;
	char dh[TAMCONN];

	rec7 = con7->Execute(strCmd,vRecord,1);

	if(rec7->adoEOF)
	{
		rec7->Close();
		con7->Close();
		sprintf(err,"Erro na Pagina Teste e na conexao com Servidor SQL %.02d, StrCon %.02d ", cd_serv, cd_conn);
		return 1;
	}

	ADODB::Field20Ptr nome;

	_variant_t vValue;

	nome = rec7->Fields->GetItem("DATA");
	vValue = nome->Value;
	COleDateTime dt = V_DATE(&vValue);

	sprintf(dh,"Servidor SQL %.02d Con %.02d - retorno %.02d/%.02d/%d-%.02d:%.02d:%.02d", cd_serv, cd_conn,
			dt.GetDay(),dt.GetMonth(),dt.GetYear(),dt.GetHour(),dt.GetMinute(),dt.GetSecond());
	
	strcpy(err, "SQL OK! ");
	strcat(err, dh);
	
	rec7->Close();
	con7->Close();

	return 0; // sucesso
}


// *********************************************************************************** //

VOID CAdo::dbStatus()
{
	FILE *fp;

	if( (fp = fopen(DBQ,"r")) == NULL )
	{
		EnviarLog(GetLastError(), 1, "CAdo::dbStatus - Banco Dados esta invalido", 0, 'N', MSGACCESS);
		return;
	}

	fclose(fp);
}

// *********************************************************************************** //

VOID CAdo::EnviarLog(DWORD lsterr, DWORD stp, char *rot, long cd, char ocr, char *tpmsg)
{
	CLogFile oLog;
	DataHora oDt;
	LPVOID	lpMsgBuf;
	char ch[TAMCONN]; ch[0]='\0';

	FormatMessage
			( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			lsterr,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &lpMsgBuf,
			0,
			NULL 
			);

	strcpy(ch,(LPCTSTR)lpMsgBuf);
	int tam = strlen(ch);
	ch[tam-2]='\0';

	char msg[255]; msg[0]='\0';

	if(stp)
		AdoStatus = 2;

	sprintf(msg, "0.00 ; %0.5d ; ALERTA ; %s ; %s", cd, rot, ch);
	
	oDt.GetDtHora(2);

	oLog.AppendText("%s ; %s ; %c ; %s", oDt.DT, tpmsg, ocr, msg);

	LocalFree( lpMsgBuf );
}


/*
'=====================================================================
'=====================================================================
*/
DWORD CAdo::recStrConnSQL(long cd_pg, CString &str_conn, long &cd_serv, long &cd_conn)
{
	HRESULT hr = S_OK; // retem o resultado da ultima operacao
	ADODB::_ConnectionPtr con3 = NULL; 
	ADODB::Recordset21Ptr rec3 = NULL;
	
	// cria uma instancia com o banco de dados
	hr = con3.CreateInstance(__uuidof(ADODB::Connection));
	if(hr != S_OK)
	{
		EnviarLog(GetLastError(), 0, "CAdo::recStrConnSQL - ADODB::Connection", cd_pg, 'N', MSGSQLSERV);
		return AdoStatus;
	}

	dbStatus();
	
	char *strcon;
	strcon = (char*)malloc(TAMCONN);
	if(strcon == NULL)
	{
		EnviarLog(GetLastError(), 0, "CAdo::recStrConnSQL - Alocando memoria para strcon", cd_pg, 'N', MSGSQLSERV);
		return AdoStatus;
	}
	strcon[0]='\0';

	sprintf(strcon,"DRIVER=%s;DBQ=%s",DRV,DBQ);
	
	try
	{
		hr = con3->Open(strcon,"","",0);
	}
	catch(_com_error e)
	{
		TCHAR err[255];
		strcpy(err, e.ErrorMessage());
		EnviarLog(4313, 1, "CAdo::recStrConnSQL 14", cd_pg, 'N', MSGSQLSERV);
		return AdoStatus;
	}  

	if(hr != S_OK)
	{
		EnviarLog(GetLastError(), 0, "CAdo::recStrConnSQL - Sem acesso Tabelas BD", cd_pg, 'N', MSGSQLSERV);
		return AdoStatus;
	}

	char *strCmd;
	strCmd = (char*)malloc(512);
	strCmd[0]='\0';
	sprintf(strCmd, "SELECT COD_SERVIDOR, COD_ST_CONN, DS_ST_CONN FROM TRBSVSQL WHERE (((COD_PAGINA_TESTE)=%d));", cd_pg);
	
	VARIANT *vRecord = NULL;

	rec3 = con3->Execute(strCmd,vRecord,1);

	if(rec3->adoEOF)
	{
		EnviarLog(GetLastError(), 0, "CAdo::recStrConnSQL - Sem retorno de registro", cd_pg, 'N', MSGSQLSERV);
		rec3->Close();
		con3->Close();
		return AdoStatus;
	}

	ADODB::Field20Ptr nome;

	_variant_t vValue;

	nome = rec3->Fields->GetItem("COD_SERVIDOR");
	vValue = nome->Value;
	cd_serv = V_I2(&vValue); 
	if(cd_serv<=0)
	{
		EnviarLog(GetLastError(), 0, "CAdo::recStrConnSQL - invalido COD_SERVIDOR tabela TRBSVSQL", cd_pg, 'N', MSGSQLSERV);
		cd_serv = 0;
	}

	nome = rec3->Fields->GetItem("COD_ST_CONN");
	vValue = nome->Value;
	cd_conn = V_I2(&vValue); 
	if(cd_conn<=0)
	{
		EnviarLog(GetLastError(), 0, "CAdo::recStrConnSQL - invalido COD_ST_CONN tabela TRBSVSQL", cd_pg, 'N', MSGSQLSERV);
		cd_conn = 0;
	}
		
	nome = rec3->Fields->GetItem("DS_ST_CONN");
	vValue = nome->Value;
	str_conn = V_BSTR(&vValue);
	if( str_conn.IsEmpty() )
	{
		EnviarLog(GetLastError(), 0, "CAdo::recStrConnSQL - invalido DS_ST_CONN tabela TRBSVSQL", cd_pg, 'N', MSGSQLSERV);
		return AdoStatus;
	}

	rec3->Close();
	con3->Close();

	free(strcon);
	free(strCmd);

	return 0;
}