/*
'=======================================================================================
'= Arquivo: SMSIServerApp.cpp
'= Função:  de total disponibilidade dos servicos do robo, alem de se reportar para o
'=			SCM. Todas as funcionalidades operacionais do ROBO, estao nesse arquivo.
'=
'= Autor:   Getson Miranda
'= Data:    20/10/2003
'=======================================================================================
*/
// SMSIServerApp.cpp: implementation of the CSMSIServerApp class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SMSIServerApp.h"
#include "servidores.h"
#include "Testes.h"
#include "logfile.h"

/*char *cTarefa =
{
	"Atz. Parms. Geral",
	"Atz. Servidores"
};*/

/*
	Estrutura retem informacoes sobre os servidores
*/
struct servidor
{
	long cd_interno;
	COleDateTime proximaverificacao;
	COleDateTime inicio;
	COleDateTime fim;
	char ocorrencia;					// ' '= efetue teste; 
										// 'S'= servidor esta OK;
										// 'N'= servidor com erro na checagem
	char ip[20];
	char nm[20];
	char log[TAMCONN];
	HANDLE hThread;
}	pServidor[QTDMAXSERVS];

/*
	Paginas de Comparacao
*/
struct pag_comp
{
	long cd_interno;
	COleDateTime proximaverificacao;
	COleDateTime inicio;
	COleDateTime fim;
	char ocorrencia;					// ' '= efetue teste; 
										// 'S'= pagina esta OK;
										// 'N'= pagina com erro na checagem
	long servidor;
	char url[500];
	char nm[20];
	char log[TAMCONN];
	HANDLE hThread;
}	pPagComp[QTDMAXPAGCOMP];

/*
	Paginas de Teste
*/
struct pag_teste
{
	long cd_interno;
	COleDateTime proximaverificacao;
	COleDateTime inicio;
	COleDateTime fim;
	char ocorrencia;				// ' '= efetue teste; 
									// 'S'= pagina esta OK;
									// 'N'= pagina com erro na checagem
	long servidor;
	char url[500];
	char nm[20];
	char str[60];
	char log[TAMCONN];
	HANDLE hThread;
}	pPagTeste[QTDMAXPAGTESTE];

u_short porta = 80;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CSMSIServerApp::CSMSIServerApp()
{
	smsirun = 1; StDLLTrap = 1; AppStatus = 1;

	atuparm = 0;	// 0 = atualizar parametros gerais
					//							servidores
					//							Paginas de comparacao
					//							paginas de teste

	for(static int i=0;i<QTDMAXSERVS;i++)
	{
		pServidor[i].cd_interno = -1;
		pServidor[i].proximaverificacao = COleDateTime::GetCurrentTime();
		pServidor[i].inicio.m_dt = 0;
		pServidor[i].ocorrencia = ' ';
	}

	for(i=0;i<QTDMAXPAGCOMP;i++)
	{
		pPagComp[i].cd_interno = -1;
		pPagComp[i].inicio.m_dt = 0;
		pPagComp[i].proximaverificacao = COleDateTime::GetCurrentTime();
		pPagComp[i].ocorrencia = ' ';
	}

	for(i=0;i<QTDMAXPAGTESTE;i++)
	{
		pPagTeste[i].cd_interno = -1;
		pPagTeste[i].proximaverificacao = COleDateTime::GetCurrentTime();
		pPagTeste[i].inicio.m_dt = 0;
		pPagTeste[i].ocorrencia = ' ';
	}
}


CSMSIServerApp::~CSMSIServerApp()
{
}


/*
'=====================================================================
'=== AtuParmsGeral()
'
'Objetivo:	Carregar os parametros gerais de trabalho do Robo.
'
'Entrada.:	nenhuma
'
'Saida...:	0	= sucesso
			2	= nao carregou o Driver de Banco de Dados nos arquivo INI
			3	= não carregou o caminho do Banco de Dados nos arquivo INI
			1	= não carregou parametros gerais da tabela TRBCFSIS
			>10 = falha nas rotinas de banco de dados
'=====================================================================
*/
DWORD CSMSIServerApp::AtuParmsGeral()
{
	long p1=0,p2=0,p3=0,p4=0,p5=0,p6=0, p7=0, p8=0;
	CString p9, p10;
	DWORD ret = 0;

	if( oAdo.LeArqIni() )
		return oAdo.AdoStatus;

	if(oAdo.recParmGeral2(p1,p2,p3,p4,p5,p6,p7,p8,p9,p10))
		return oAdo.AdoStatus;

	nummaxthread		= p3; // nummaxthread, numero maximo de threads
	tempoverificacao	= p1; // tempoverificacao, tempo padrao de verificacao dos servicos
	tempoverificcerro	= p2; // tempoverificcerro, tempo de verificacao apos deteccao de erro
	tempotmoutserv		= p4; // tempotmoutserv, tempo de time out para thread do servidor
	tempotmoutpgcomp	= p5; // tempotmoutpgcomp, tempo de time out para as paginas de comparacao
	tempotmoutpgtst		= p6; // tempotmoutpgtst, tempo de time out para as paginas de testes do site
	tempoatualparm		= p7; // tempoatualparm, tempo para uma nova leitura do arquivo de inicializacao e banco de dados

	waitTrapVivo = p8;	// Quantidade em segundos para intervalo de envio de Traps, estou vivo.
	msgTrapVivo = p9;	// Mensagem a ser enviada na Trap, informando que o robo está vivo.
	msgTrapEnd = p10;	// Codigo da mensagem de Trap
	tmTrapVivo = COleDateTime::GetCurrentTime();
						// Ponto inicial para o envio de trap

	// Envio de Traps
	oIni.GetTRAP_VERSIONValue(ver);
	oIni.GetTRAP_COMUNITYValue(pub);
	oIni.GetTRAP_ENTERPRISEValue(ent);
	oIni.GetTRAP_GENTRAPValue(gen);
	oIni.GetTRAP_RECEIVERValue(receiv);
	oIni.GetTRAP_RECEIVER_PORTValue(port);

	return ret;
}



/*
'=====================================================================
'===		Processamento()
'
'Objetivo:	iniciar os servicos do robo, e chamado pela rotina de servico
'
'Entrada.:	nenhuma
'          
'=====================================================================
*/
// processamento de tarefas do robo
void CSMSIServerApp::Processamento()
{
	AtualizaParametros();
}



/*
'=====================================================================
'===		TrapToAtivo()
'
'Objetivo:	Envia trap estou ativo
'
'=====================================================================
*/
// processamento de tarefas do robo
VOID CSMSIServerApp::TrapToAtivo()
{
	COleDateTime hr_atual = COleDateTime::GetCurrentTime();

	COleDateTimeSpan spanElapsed = (hr_atual.m_dt - tmTrapVivo.m_dt );

	if(spanElapsed.GetTotalSeconds() >= waitTrapVivo) // Se eh o momento de enviar trap estou ativo
	{
		char tch[20];
		tch[0]='\0';
		char tch2[40];
		tch2[0]='\0';
		strcpy(tch, msgTrapEnd);
		strcpy(tch2, msgTrapVivo);
		SendTrap(tch,tch2);
		tmTrapVivo = COleDateTime::GetCurrentTime();
	}
}


/*
'=====================================================================
'===		SendTrap( char *msg, char *msg1 )
'
'Objetivo:	Enviar trap
'
'Entrada.:	msg  = [1.3.6.1.4.1.105.0.4]
'			msg1 = mensagem a ser enviada
'
'Retorno.:	0 = sucesso
'           1 = erro no acesso a dll de traps do VB
'			2 = erro no acesso a funcao de trap na dll do VB
'			3 = erro no acesso ao arquivo de inicializacao
'=====================================================================
*/
DWORD CSMSIServerApp::SendTrap( char *msg, char *msg1 )
{
	HRESULT hresult_trap;
	CLSID clsid_trap;
	CoInitialize(NULL);
	hresult_trap = CLSIDFromProgID(OLESTR("TrapSNMP.clsTrapSNMP"),&clsid_trap);

	if(FAILED(hresult_trap))
	{
		EnviarLog(GetLastError(), "CSMSIServerApp::SendTrap 01","TrapSNMP.DLL ou MSWINSCK.OCX");
		StDLLTrap = 0;
		return 1;
	}
		
	_clsTrapSNMP *t;
		
	hresult_trap = CoCreateInstance(clsid_trap,NULL,CLSCTX_INPROC_SERVER,__uuidof(_clsTrapSNMP),(LPVOID *) &t);

	if(FAILED(hresult_trap))
	{
		EnviarLog(GetLastError(), "CSMSIServerApp::SendTrap 02","TrapSNMP.DLL ou MSWINSCK.OCX");
		StDLLTrap = 0;
		return 2;
	}

	t->PutTRAP_VERSION(ver);
	t->PutTRAP_COMUNITY(pub);
	t->PutTRAP_ENTERPRISE(ent);
	t->PutTRAP_GENTRAP(gen);
	t->PutTRAP_RECEIVER(receiv);
	t->PutTRAP_RECEIVER_PORT(port);

	try
	{
		t->EnviarTrap(msg, msg1);
	}
	catch(_com_error e)
	{
		DWORD err=e.Error();
		StDLLTrap = 0;
		EnviarLog(err, "CSMSIServerApp::SendTrap 03","TrapSNMP.DLL ou MSWINSCK.OCX");
	}

	t->Release ();
	
	CoUninitialize();

	return 0;
}

/*
'=====================================================================
'===		AtualizaParametros()
'
'Objetivo:	realiza uma atualizacao geral nas estruturas de servidores e paginas
'
'Entrada.:	nenhuma
'
'Retorno	nenhum
'=====================================================================
*/
void CSMSIServerApp::AtualizaParametros()
{
	TCHAR tch[TAMCONN];
	tch[0]='\0';
	DWORD ret = 0;
	long tempo = 0;

	switch(atuparm)
	{
		// Alimentar parametros dos sistema
	case 0: ret = AtuParmsGeral();
		if( ret )
		{
			sprintf(tch,"AtuParmsGeral() retornou [%d]",ret);
		}
		numdispthread = nummaxthread;
		atuparm++;
		break;
		
		// Alimentar parametros dos servidores
	case 1: ret = AlimentaDadosServs();
		if( ret )
		{
			sprintf(tch,"AlimentaDadosServs() retornou [%d]",ret);
			smsirun = 0;
		}
		atuparm++;
		break;
		
		// Alimentar parametros das paginas de comparacao
	case 2: ret = AlimentaPagsComp();
		if( ret )
		{
			sprintf(tch,"AlimentaPagsComp() retornou [%d]",ret);
		}
		atuparm++;
		break;
		
		// Alimentar parametros das paginas de teste
	case 3: ret = AlimentaPagsTestes();
		if( ret )
		{
			sprintf(tch,"AlimentaPagsTestes() retornou [%d]",ret);
		}
		ultatuparm = COleDateTime::GetCurrentTime();
		atuparm++;
		break;
		
	case 4: 
	default:
		Verificacao();
		break;
	}
}

/*
'=====================================================================
'===		AlimentaDadosServs()
'
'Objetivo:	Alimenta registros na estrutura de servidores
'
'Entrada.:	nenhuma
'
'Retorno	3 = Nao conseguiu abrir o banco de dados
'           0 = Sucesso na carga
'=====================================================================
*/
// leitura e carga das tabelas do banco de dados
DWORD CSMSIServerApp::AlimentaDadosServs()
{
	// Campos na tabela de servidores
	long	cd = 0;
	char	ips[20];
	char	nm[20];

	// carrega registro dos servidores
	long i = 0;
	long incluir = 0;

	DWORD ret = oAdo.AbreTabelaServidores();
	// nome de servidores e endrecos IP
	if( ret )
		return ret; // Nao conseguiu abrir o banco de dados

	ips[0]='\0';
	nm[0]='\0';

	while( oAdo.RegistroServidores2(cd,ips,nm) )
	{
		// leitura de registro da tabela de servidores
		//for(i=0;i<QTDMAXSERVS;i++)
		//{
		//	if(pServidor[i].cd_interno == cd)
		//	{
		//		incluir = 0;
		//		break;
		//	}
		//	else
		//	{
		//		incluir = 1;
		//		break;
		//	}
		//}

		//if(incluir)
		//{
		pServidor[i].cd_interno = cd;
		memcpy(pServidor[i].ip,ips,sizeof(ips));
		memcpy(pServidor[i].nm,nm,sizeof(nm));
			// pServidor[i].proximaverificacao = COleDateTime::GetCurrentTime();
			// pServidor[i].inicio.m_dt = 0;
		pServidor[i].fim.m_dt = 0;
			// pServidor[i].ocorrencia = ' ';
		//}

		ips[0]='\0';
		nm[0]='\0';
		i++;
	}

	return 0; // Sucesso
}

/*
'=====================================================================
'===		AlimentaPagsComp()
'
'Objetivo:	Alimenta registros na estrutura das paginas de comparacao
'
'Entrada.:	nenhuma
'
'Retorno	2 = Nao conseguiu abrir o banco de dados
'           0 = Sucesso na carga
'=====================================================================
*/
// leitura e carga das tabelas do banco de dados
DWORD CSMSIServerApp::AlimentaPagsComp()
{
	// Campos na tabela de servidores
	long cd = 0;
	long cd_serv = 0;
	char url[500];
	char nm[20];
	// carrega registro dos servidores
	long i = 0;
	long incluir = 0;

	// select total das paginas para comparacao
	DWORD ret = oAdo.AbreTabelaPgComp();
	if( ret )
	{
		for(i=0;i<QTDMAXPAGCOMP;i++)
			pPagComp[i].cd_interno = -1;

		return ret; // Nao conseguiu abrir o banco de dados
	}

	url[0]='\0';
	nm[0]='\0';

	while( oAdo.RgTbPagComp(cd, cd_serv, url, nm) )
	{	
		//for(i=0;i<QTDMAXPAGCOMP;i++)
		//{
		//	if(pPagComp[i].cd_interno == cd)
		//	{
		//		incluir = 0;
		//		break;
		//	}
		//	else
		//	{
		//		incluir = 1;
		//		break;
		//	}
		//}
		//if(incluir)
		//{
			// pPagComp[i].inicio.m_dt = 0;
			// pPagComp[i].proximaverificacao = COleDateTime::GetCurrentTime();
			pPagComp[i].cd_interno = cd;
			pPagComp[i].servidor = cd_serv;
			memcpy(pPagComp[i].url,url,sizeof(url));
			memcpy(pPagComp[i].nm,nm,sizeof(nm));
			pPagComp[i].fim.m_dt = 0;
			// pPagComp[i].ocorrencia = ' ';
		//}
		url[0]='\0'; nm[0]='\0'; cd = 0; cd_serv = 0;
		i++;
	}

	return 0; // Sucesso
}

/*
'=====================================================================
'===		AlimentaPagsTestes()
'
'Objetivo:	Alimenta registros na estrutura das paginas de teste
'
'Entrada.:	nenhuma
'
'Retorno	2 = Nao conseguiu abrir o banco de dados
'           0 = Sucesso na carga
'=====================================================================
*/
// leitura e carga das tabelas do banco de dados
DWORD CSMSIServerApp::AlimentaPagsTestes()
{
	// Campos na tabela de servidores
	long cd = 0;
	long cd_serv = 0;
	char url[500];
	char nm[20];

	// carrega registro dos servidores
	long i = 0;
	long incluir = 0;

	// select total das paginas para comparacao
	DWORD ret = oAdo.AbreTabelaPgTeste();
	if( ret )
	{
		for(i=0;i<QTDMAXPAGTESTE;i++)
			pPagTeste[i].cd_interno = -1;

		return ret; // Nao conseguiu abrir o banco de dados
	}

	url[0]='\0';
	nm[0]='\0';

	while( oAdo.RgTbPagTeste(cd, cd_serv, url, nm) )
	{
		// recupera um registro	
		//for(i=0;i<QTDMAXPAGTESTE;i++)
		//{
		//	if(pPagTeste[i].cd_interno == cd)
		//	{
		//		incluir = 0;
		//		break;
		//	}
		//	else
		//	{
		//		incluir = 1;
		//		break;
		//	}
		//}
		//if(incluir)
		//{
			pPagTeste[i].cd_interno = cd;
			pPagTeste[i].servidor = cd_serv;
			memcpy(pPagTeste[i].url,url,sizeof(url));
			memcpy(pPagTeste[i].nm,nm,sizeof(nm));
			// pPagTeste[i].proximaverificacao = COleDateTime::GetCurrentTime();
			// pPagTeste[i].inicio.m_dt = 0;
			pPagTeste[i].fim.m_dt = 0;
			// pPagTeste[i].ocorrencia = ' ';
		//}
		url[0]='\0';
		nm[0]='\0';
		i++;
	}

	return 0; // Sucesso
}


/*
'=====================================================================
'===		Threaded( LPVOID )
'
'Objetivo:	thread para teste de servidores
'
'Saida :	0 = sucesso
'			2 = erro no teste de ping do servidor
'=====================================================================
*/
DWORD WINAPI ThreadFunc( long &i ) 
{ 
	long ind = i;
	CServidores oServ;
	char err[TAMCONN];
	err[0] = '\0';
	long tam = 0;
	
	short ret = oServ.ChkServidor(pServidor[ind].ip, err, porta);

	if( ret )
	{
		if( ret < 3 )
			pServidor[ind].ocorrencia = 'X';	// Erro interno
		else
			pServidor[ind].ocorrencia = 'N';	// Erro no teste
	}
	else
		pServidor[ind].ocorrencia = 'S';		// Sucesso no teste

	tam = strlen(err);
	err[tam-2]='\0';
	strcpy(pServidor[ind].log,err);
	
    return ret; 
}


/*
'=====================================================================
'===		Threaded2( LPVOID )
'
'Objetivo:	thread para paginas de comparacao
'
'Saida :	0 = sucesso na comparacao
'			1 = erro na criacao de um objeto de banco de dados
'			2 = erro na criacao de um objeto de teste de comparacao
'			4 = erro na comparacao da pagina
'=====================================================================
*/
DWORD WINAPI ThreadFunc2( long &j ) 
{ 
	long ind = j;
	TCHAR memo[50000];
	TCHAR url[500];
	memo[0]='\0';
	url[0]='\0';
	char err[TAMCONN];
	err[0] = '\0';
	long tam = 0;
	long tam2;
	long reg = 0;
	CTestes oPg;
	CAdo *oAdo2;
	short ret = 1;

	oAdo2 = new CAdo();

	while( oAdo2->rgPgCmp(pPagComp[ind].cd_interno, memo,url, tam2, reg) )
	{
		ret = oPg.CmpPg(url, memo, tam2, err);

		if( !ret )
			break;

		reg++;
		memo[0]='\0';
		url[0]='\0';
		Sleep(1000);
	}

	if( ret )
		pPagComp[ind].ocorrencia = 'N';	// Erro
	else
		pPagComp[ind].ocorrencia = 'S';	// Sucesso

	tam = strlen(err);
	err[tam-2]='\0';
	strcpy(pPagComp[ind].log,err);

	delete oAdo2;

	return 1;
}

/*
'=====================================================================
'===		Threaded3( LPVOID )
'
'Objetivo:	thread para paginas de teste
'
'Saida :	0 = sucesso
'			2 = erro no teste da pagina
'=====================================================================
*/
DWORD WINAPI ThreadFunc3( long &j ) 
{ 
	long ind = j;
	TCHAR str[40];
	CTestes oPg;
	char err[TAMCONN];
	short ret = 1;
	int tam = 0;

	err[0]='\0';
	str[0]='\0';

	ret = oPg.TstPg(pPagTeste[ind].url, str, err);

	if( ret == 2 )
		pPagTeste[ind].ocorrencia = 'x'; // Erro interno
	else
	{
		if( ret )
			pPagTeste[ind].ocorrencia = 'N'; // Erro
		else
			pPagTeste[ind].ocorrencia = 'S'; // Sucesso
	}

	tam = strlen(err);
	err[tam-2]='\0';
	strcpy(pPagTeste[ind].log,err);
	strcpy(pPagTeste[ind].str,&str[1]);

    return 0; 
}



/*
'=====================================================================
'===		EnviarTrapServidor(long cd, char *msg)
'
'Objetivo:	Busca no banco de dados o nome do servidor e o codigo de mensagem de
'			trap padrao (1.3.6.1.4.1.105.0.4). Encontrando envia TRAP para o servidor
'			cadastrado no arquivo INI da aplicacao.
'			Metodo aplicado somente aos servidores do site
'
'Entrada :	cd = codigo interno do servidor
'			msg	=	"ERRGE" Erro generico
			msg	=	"TMOUT" Erro de time out
'			
'=====================================================================
*/
void CSMSIServerApp::EnviarTrapServidor(long cd, char *msg)
{
	char trp[32]; char nm[32];
	trp[0]='\0';
	nm[0]='\0';
	// nome de servidores e endrecos IP
	if( oAdo.tbTrapErroSrv(cd, msg, trp, nm) )
		return;
	// Campos na tabela de servidores
	char mensg[50];
	mensg[0]='\0';
	sprintf(mensg,"ERRO SERVIDOR %s",nm);
	SendTrap(trp,mensg);
}


/*
'=====================================================================
'===		AtualizaServidor(short idc)
'
'Objetivo:	atualiza o registro de um servidor de banco de dados, apos
'			a sua checagem
'
'Entrada.:	idc = indice do servidor na estrutura de servidores
'
'Saida...:	nenhuma
'
'Retornos:	nenhum
'
'=====================================================================
*/
void CSMSIServerApp::AtualizaServidor(long idc)
{
	char ip[20]; char nm[20];
	ip[0]='\0';
	nm[0]='\0';
	
	if( oAdo.tbUpdServ(pServidor[idc].cd_interno, nm, ip) )
	{
		pServidor[idc].cd_interno = -1;
		return;
	}
	
	memcpy(pServidor[idc].ip, ip, sizeof(ip));
	memcpy(pServidor[idc].nm, nm, sizeof(nm));
	pServidor[idc].inicio.m_dt = 0;
	pServidor[idc].fim.m_dt = 0;

	COleDateTime dt_atual;
	CTime tim = CTime::GetCurrentTime();
	int aano = tim.GetYear();
	int ames = tim.GetMonth();
	int adia = tim.GetDay();
	int ahor = tim.GetHour();
	int amin = tim.GetMinute();
	int aseg = tim.GetSecond();

	if(pServidor[idc].ocorrencia == 'N') // falha na checagem
	{
		tim += CTimeSpan(0,0,0,tempoverificcerro);
		int aaano = tim.GetYear();
		int aames = tim.GetMonth();
		int aadia = tim.GetDay();
		int aahor = tim.GetHour();
		int aamin = tim.GetMinute();
		int aaseg = tim.GetSecond();
		dt_atual.SetDateTime(aaano,aames,aadia,aahor,aamin,aaseg);
		pServidor[idc].proximaverificacao = dt_atual;
	}
	else
	{
		tim += CTimeSpan(0,0,0,tempoverificacao);
		int aaano = tim.GetYear();
		int aames = tim.GetMonth();
		int aadia = tim.GetDay();
		int aahor = tim.GetHour();
		int aamin = tim.GetMinute();
		int aaseg = tim.GetSecond();
		dt_atual.SetDateTime(aaano,aames,aadia,aahor,aamin,aaseg);
		pServidor[idc].proximaverificacao = dt_atual;
	}

	// pServidor[idc].ocorrencia = ' ';
}


/*
'=====================================================================
'===		EnviarTrapPagComp(long cd, char *msg)
'
'Objetivo:	Busca no banco de dados o nome do servidor e o codigo de mensagem de
'			trap padrao (1.3.6.1.4.1.105.0.4). Encontrando envia TRAP para o servidor
'			cadastrado no arquivo INI da aplicacao.
'			Metodo aplicado somente as paginas html de comparacao.
'
'Entrada :	cd = codigo interno do servidor
'			msg	=	"ERRGE" Erro generico
			msg	=	"TMOUT" Erro de time out
'			
'=====================================================================
*/
void CSMSIServerApp::EnviarTrapPagComp(long cd, char *msg)
{
	char trp[20]; 
	char nm[20];
	trp[0]='\0';
	nm[0]='\0';
	// nome de servidores e endrecos IP
	if( oAdo.tbTrapPgComp(cd, msg, trp, nm) )
		return;
	// Campos na tabela de servidores
	char mensg[50];
	mensg[0]='\0';
	sprintf(mensg,"ERRO PAG %.03d COMP SERV %s", cd, nm);
	SendTrap(trp,mensg);
}

/*
'=====================================================================
'===		EnviarTrapPagTeste(long cd, char *msg)
'
'Objetivo:	Busca no banco de dados o nome do servidor e o codigo de mensagem de
'			trap padrao (1.3.6.1.4.1.105.0.4). Encontrando envia TRAP para o servidor
'			cadastrado no arquivo INI da aplicacao.
'			Metodo aplicado somente para as paginas html de teste.
'
'Entrada :	cd = codigo interno do servidor
'			msg	=	"ERRGE" Erro generico
			msg	=	"TMOUT" Erro de time out
'			
'=====================================================================
*/
void CSMSIServerApp::EnviarTrapPagTeste(long i, char *msg, short trap)
{
	long ind = i;
	char trp[20]; char nm[20];
	trp[0]='\0'; nm[0]='\0';
	// nome de servidores e endrecos IP
	if( oAdo.tbTrapPgTeste(pPagTeste[ind].cd_interno, msg, trp, nm) )
		return;
	// Campos na tabela de servidores
	char mensg[50]; mensg[0]='\0';

	switch(trap)
	{
	case 1:
		sprintf(mensg,"ERRO PG %.03d TESTE SERV %s", pPagTeste[ind].cd_interno, nm);
		break;
	case 2:
		sprintf(mensg,"ERRO PG %.03d [%s] E SERV SQL", pPagTeste[ind].cd_interno, nm);
		break;
	default:
		sprintf(mensg,"ERRO PG %.03d TESTE SERV %s", pPagTeste[ind].cd_interno, nm);
		break;
	}

	SendTrap(trp, mensg);
}


/*
'=====================================================================
'
'Objetivo:	
'
'Entrada :	
'			
'=====================================================================
*/
VOID CSMSIServerApp::PingServidor(long &j)
{
	DWORD dwThreadId;

	pServidor[j].hThread = CreateThread( 
			NULL,                        // no security attributes 
			0,                           // use default stack size  
			(LPTHREAD_START_ROUTINE)ThreadFunc,                  // thread function 
			&j,			                 // argument to thread function 
			0,                           // use default creation flags 
			&dwThreadId);                // returns the thread identifier 
}


/*
'=====================================================================
'===		
'
'Objetivo:	
'			
'Entrada :	
'			
'=====================================================================
*/
VOID CSMSIServerApp::ComparaPagina(long &j)
{
	DWORD dwThreadId;

	pPagComp[j].hThread = CreateThread( 
			NULL,                        // no security attributes 
			0,                           // use default stack size  
			(LPTHREAD_START_ROUTINE)ThreadFunc2,                  // thread function 
			&j,			                 // argument to thread function 
			0,                           // use default creation flags 
			&dwThreadId);                // returns the thread identifier 
}


/*
'=====================================================================
'===		
'
'Objetivo:	
'
'Entrada :	
'			
'=====================================================================
*/
VOID CSMSIServerApp::TestaPagina(long &j)
{
	DWORD dwThreadId;
	
	pPagTeste[j].hThread = CreateThread( 
			NULL,                        // no security attributes 
			0,                           // use default stack size  
			(LPTHREAD_START_ROUTINE)ThreadFunc3,                  // thread function 
			&j,			                 // argument to thread function 
			0,                           // use default creation flags 
			&dwThreadId);                // returns the thread identifier 
}

/*
	Efetua a verificacao do servidor SQL de uma pagina de Teste.
	Grava LOG com o retorno do SQL, se possivel.

	Entrada		i = indice na estrutura contendo informacoes sobre a pagina de teste

	Saida		nao possui

	Retorno		nenhum

  */

short CSMSIServerApp::ChkSrvSqlPgTst(long i)
{
	CString str_conn;
	long cd_serv = 0;
	long cd_conn = 0;
	char err[TAMCONN];

	err[0] = '\0';

	if(oAdo.recStrConnSQL(pPagTeste[i].cd_interno, str_conn, cd_serv, cd_conn))
		return 1;

	oAdo.ConnectSQLServ(str_conn, cd_serv, cd_conn, err);

	/*if( oAdo.ConnectSQLServ(str_conn, cd_serv, cd_conn, err) )
		pPagTeste[i].ocorrencia = 'N';
	else
		pPagTeste[i].ocorrencia = 'S';*/

	if( (strlen(err)) <= 0)
		strcpy(pPagTeste[i].log, "Erro na Comunicacao com Servidor SQL!");
	else
		strcpy(pPagTeste[i].log, err);

	EnviarLogTST(0, pPagTeste[i].url, NULL, i);

	return 0;
}


/*
'=====================================================================
'===		Verificacao()
'
'Objetivo:	Faz a verificacao dos servidores, paginas de comparacao e paginas de teste.
'			Em tres loops for para cada uma das tres tarefas.
'			O primeiro for tem a finalidade de criar as threads de servicos;
'			O segundo for finaliza as threads que foram criadas, e encerradas em
'			tempo normal;
'			O terceiro for encerra a thread por TimeOut, caso tenha ocorrido algum
'			problema, para que a thread nao tenha retornado.
'
'			A cada 1 minuto atualiza os parametros internos do robo que estao no banco
'			de dados, desviando a execucao.
'
'Entrada :	Nao possui
'			
'Retorno :	sempre 0 = verdadeiro
'=====================================================================
*/
VOID CSMSIServerApp::Verificacao()
{
	long i = 0;
	COleDateTime tempoAgora = COleDateTime::GetCurrentTime();
	COleDateTimeSpan spanElapsed;
	long tm = 0;

	switch(atuparm)
	{
		
		// Checagem de servidores
		
	case 4:
		
		if(numdispthread)
		{
			for(i=0;i<QTDMAXSERVS;i++)
			{
				if(pServidor[i].cd_interno != -1 )
				{
					if( tempoAgora.m_dt >= pServidor[i].proximaverificacao.m_dt )
					{
						numdispthread--;
						pServidor[i].inicio = COleDateTime::GetCurrentTime();
						PingServidor(i);
					}
				}
				Sleep(1000);
			}
		}
		
		atuparm++;
		break;
		
	case 5:
		
		// Verifica o encerramento normal das checagens dos servidores
		for(i=0;i<QTDMAXSERVS;i++)
		{
			if(pServidor[i].cd_interno != -1)
			{
				if( pServidor[i].inicio != 0 && pServidor[i].fim == 0 )
				{
					if(pServidor[i].ocorrencia != ' ') // pronto para proximo teste
					{
						CloseHandle(pServidor[i].hThread);

						if(pServidor[i].ocorrencia == 'N') // falhou no teste
							if(StDLLTrap)
								EnviarTrapServidor(pServidor[i].cd_interno, "ERRGE");
						
						pServidor[i].fim = COleDateTime::GetCurrentTime();
						spanElapsed = (pServidor[i].fim - pServidor[i].inicio);
						tm = spanElapsed.GetTotalSeconds();
						EnviarLogServ( tm, "ERRGE", i );
						Sleep(1000);	
						AtualizaServidor(i);
						numdispthread++;
					}
				}
			}
		}
		
		atuparm++;
		break;
		
	case 6:
		
		// encerra threads de checagem dos servidores por timeout
		for(i=0;i<QTDMAXSERVS;i++)
		{
			if(pServidor[i].cd_interno != -1)
			{
				if( pServidor[i].inicio != 0 &&
					pServidor[i].fim == 0 )
				{
					spanElapsed = (tempoAgora.m_dt - pServidor[i].inicio.m_dt);
					if(spanElapsed.GetTotalSeconds() > tempotmoutserv)
					{
						CloseHandle(pServidor[i].hThread);

						pServidor[i].fim = COleDateTime::GetCurrentTime();
						spanElapsed = (pServidor[i].fim - pServidor[i].inicio);
						tm = spanElapsed.GetTotalSeconds();
						pServidor[i].ocorrencia = 'N';			
						
						if(StDLLTrap)
							EnviarTrapServidor(pServidor[i].cd_interno, "TMOUT");

						strcpy(pServidor[i].log,"THREAD Encerrada TimeOut");

						EnviarLogServ( tm, "TMOUT", i);
						Sleep(1000);
						AtualizaServidor(i);

						numdispthread++;
					}
				}
			}
		}
		
		atuparm++;
		//atuparm = 100;
		break;
		
	case 7:
		
		// *************************************************************************************
		// PAGINAS DE COMPARACAO ***************************************************************
		// *************************************************************************************
		
		if(numdispthread)
		{
			for(i=0;i<QTDMAXPAGCOMP;i++)
			{
				if(pPagComp[i].cd_interno != -1 )
				{
					if( tempoAgora.m_dt >= pPagComp[i].proximaverificacao.m_dt )
					{
						if(ServidorCHK(pPagComp[i].servidor))
						{
							numdispthread--;
							pPagComp[i].inicio = COleDateTime::GetCurrentTime();
							ComparaPagina(i);
						}
					}
				}
				Sleep(1000);
			}
		}
		atuparm++;
		break;
		
	case 8:
		
		// mata as threads
		for(i=0;i<QTDMAXPAGCOMP;i++)
		{
			if(pPagComp[i].cd_interno != -1)
			{
				if( pPagComp[i].inicio.m_dt != 0 && pPagComp[i].fim.m_dt == 0 )
				{
					if(pPagComp[i].ocorrencia != ' ') // pronto para proximo teste
					{
						CloseHandle(pPagComp[i].hThread);

						pPagComp[i].fim = COleDateTime::GetCurrentTime();
						if(pPagComp[i].ocorrencia == 'N')
							if(StDLLTrap)
								EnviarTrapPagComp(pPagComp[i].cd_interno, "ERRGE");
						spanElapsed = (pPagComp[i].fim - pPagComp[i].inicio);
						tm = spanElapsed.GetTotalSeconds();
						EnviarLogPg(tm, pPagComp[i].url, "ERRGE", i);
						Sleep(1000);
						AtuPgComp(i);
						numdispthread++;
					}
				}
			}
		}
		
		atuparm++;
		break;
		
	case 9:
		
		// encerra threads por timeout
		for(i=0;i<QTDMAXPAGCOMP;i++)
		{
			if(pPagComp[i].cd_interno != -1)
			{
				
				if( pPagComp[i].inicio != 0 &&
					pPagComp[i].fim == 0 )
				{
					spanElapsed = (tempoAgora.m_dt - pPagComp[i].inicio.m_dt);
					if(spanElapsed.GetTotalSeconds() > tempotmoutpgcomp)
					{
						CloseHandle(pPagComp[i].hThread);

						pPagComp[i].fim = COleDateTime::GetCurrentTime();
						
						if(StDLLTrap)
							EnviarTrapPagComp(pPagComp[i].cd_interno, "TMOUT");
						
						spanElapsed = (pPagComp[i].fim - pPagComp[i].inicio);
						tm = spanElapsed.GetTotalSeconds();
						pPagComp[i].ocorrencia = 'N';
						
						strcpy(pPagComp[i].log,"THREAD Encerrada por TimeOut");
						EnviarLogPg(tm, pPagComp[i].url, "TMOUT", i);
						Sleep(1000);
						AtuPgComp(i);
						numdispthread++;
					}
				}
			}
		}
		
		atuparm++;
		//atuparm = 100;
		break;
		
	case 10:
		
		// *************************************************************************************
		// PAGINAS DE TESTE ********************************************************************
		// *************************************************************************************

		if(numdispthread)
		{
			for(i=0;i<QTDMAXPAGTESTE;i++)
			{
				if(pPagTeste[i].cd_interno != -1 )
				{
					if( tempoAgora.m_dt >= pPagTeste[i].proximaverificacao.m_dt )
					{
						if(ServidorCHK(pPagTeste[i].servidor))
						{
							numdispthread--;
							pPagTeste[i].inicio = COleDateTime::GetCurrentTime();
							TestaPagina(i);
						}
					}
				}
				Sleep(1000);
			}
		}
		
		atuparm++;
		break;
		
	case 11:
		
		// mata as threads
		for(i=0;i<QTDMAXPAGTESTE;i++)
		{
			if(pPagTeste[i].cd_interno != -1)
			{
				if( pPagTeste[i].inicio != 0 && pPagTeste[i].fim == 0 )
				{
					if(pPagTeste[i].ocorrencia != ' ') // pronto para proximo teste
					{
						CloseHandle(pPagTeste[i].hThread);

						pPagTeste[i].fim = COleDateTime::GetCurrentTime();
											
						spanElapsed = (pPagTeste[i].fim - pPagTeste[i].inicio);
						
						tm = spanElapsed.GetTotalSeconds();

						if(pPagTeste[i].ocorrencia == 'N')
						{
							if( ChkSrvSqlPgTst(i) )
							{
								strcpy(pPagTeste[i].log, "Nao foi possivel testar a pagina e a conexao com servidor SQL!");
								EnviarLogTST(tm, pPagTeste[i].url, "ERRGE", i);
								if(StDLLTrap)
									EnviarTrapPagTeste(i, "ERRGE", 2);
							}
							else
							{
								strcpy(pPagTeste[i].log, "Erro na pagina, servidor SQL esta OK!");
								EnviarLogTST(tm, pPagTeste[i].url, "ERRGE", i);
								if(StDLLTrap)
									EnviarTrapPagTeste(i, "ERRGE", 1);
							}
						}
						else
							EnviarLogTST(tm, pPagTeste[i].url, "ERRGE", i);

						AtuPgTeste(i);
						Sleep(1000);
						numdispthread++;
					}
				}
			}
		}
		
		atuparm++;
		break;
		
	case 12:
		
		// encerra threads por timeout
		for(i=0;i<QTDMAXPAGTESTE;i++)
		{
			if(pPagTeste[i].cd_interno != -1)
			{
				if( pPagTeste[i].inicio != 0 &&
					pPagTeste[i].fim == 0 )
				{
					spanElapsed = (tempoAgora.m_dt - pPagTeste[i].inicio.m_dt);
					if(spanElapsed.GetTotalSeconds() > tempotmoutpgtst)
					{
						CloseHandle(pPagTeste[i].hThread);

						pPagTeste[i].fim = COleDateTime::GetCurrentTime();
							
						spanElapsed = (pPagTeste[i].fim - pPagTeste[i].inicio);
						tm = spanElapsed.GetTotalSeconds();
						pPagTeste[i].ocorrencia = 'N';
						
						strcpy(pPagTeste[i].log,"THREAD Encerrada por TimeOut");
						EnviarLogTST(tm, pPagTeste[i].url, "TMOUT" , i);
						
						if( ChkSrvSqlPgTst(i) )
						{
							strcpy(pPagTeste[i].log, "Nao foi possivel o teste da pagina e a conexao com servidor SQL!");
							EnviarLogTST(tm, pPagTeste[i].url, "TMOUT", i);
							if(StDLLTrap)
								EnviarTrapPagTeste(i, "TMOUT", 2);
						}
						else
						{
							strcpy(pPagTeste[i].log, "Erro na pagina, mas o servidor SQL esta OK!");
							EnviarLogTST(tm, pPagTeste[i].url, "TMOUT", i);
							if(StDLLTrap)
								EnviarTrapPagTeste(i, "TMOUT", 1);
						}
						
						AtuPgTeste(i);
						Sleep(1000);
						numdispthread++;
					}
				}
			}
		}

	default:
		atuparm = 4;
		break;
	}

	TrapToAtivo();

	spanElapsed = (tempoAgora.m_dt - ultatuparm.m_dt );

	if(spanElapsed.GetTotalSeconds() >= tempoatualparm)
	{
		ultatuparm = COleDateTime::GetCurrentTime();

		atuparm = 0;
		StDLLTrap = 1;

		//if( (ultatuparm.GetHour() == 11 && ultatuparm.GetSecond() == 00) )
		//	atuparm = 99;
	}

}


/*
'=====================================================================
'===		ServidorCHK(long Serv)
'
'Objetivo:	Verifica se o servidor foi checado, se sua ocorrencia for
'			igual a S, faz a comparacao das paginas HTML e testa os
'			servicos do Site, 
'
'Entrada :	long Serv - codigo interno do servidor para o robo, esse
'			eh criado em tempo de execucao.
'          
'Retorno :	0 - Servidor esta OK
'			1 - Servidor nao disponivel, ignora os testes do site para esse servidor
'
'=====================================================================
*/
DWORD CSMSIServerApp::ServidorCHK(long Serv)
{
	int i;
	for(i=0;i<QTDMAXSERVS;i++)
	{
		if(pServidor[i].cd_interno == -1) // se o servidor eh invalido
			break;

		if(pServidor[i].cd_interno == Serv)
			if(pServidor[i].ocorrencia == 'S') // testou o servidor com sucesso
				return 1;
	}
	return 0;
}


/*
'=====================================================================
'===		AtuPgComp(short idc)
'
'Objetivo:	Preenche a estrutura interna com informacoes vitais sobre as
'			paginas de comparacao. 
'			Ataualiza as informacoes da pagina de comparacao no banco de
'			de dados, refente ao nome do servidor, codigo no banco de dados e,
'			e url da pagina, no servidor.
'			Verifica a ocorrencia da checagem da pagina, caso tenha ocorrido
'			algum problema, diminui o tempo de verificacao, conforme parametrizado,
'			do contrario ataualiza a pagina para o proximo momento de verificacao.
'
'Entrada :	Identificador atual interno da pagina de comparacao que acabou
'			de ser testada, independente do seu retorno.
'          
'=====================================================================
*/
void CSMSIServerApp::AtuPgComp(long idc)
{
	char nm[20];
	char url[500];
	long cd_serv;

	url[0]='\0';
	nm[0]='\0';
	
	if(oAdo.AtualizaPagComp(pPagComp[idc].cd_interno, cd_serv, nm, url))
	{
		pPagComp[idc].cd_interno = -1;
		return;
	}

	memcpy(pPagComp[idc].url, url, sizeof(url));
	memcpy(pPagComp[idc].nm, nm, sizeof(nm));
	pPagComp[idc].inicio.m_dt = 0;
	pPagComp[idc].fim.m_dt = 0;
	pPagComp[idc].servidor = cd_serv;

	COleDateTime dt_atual;
	CTime tim = CTime::GetCurrentTime();
	int aano = tim.GetYear();
	int ames = tim.GetMonth();
	int adia = tim.GetDay();
	int ahor = tim.GetHour();
	int amin = tim.GetMinute();
	int aseg = tim.GetSecond();

	if(pPagComp[idc].ocorrencia == 'N') // erro no teste atual
	{
		tim += CTimeSpan(0,0,0,tempoverificcerro);
		int aaano = tim.GetYear();
		int aames = tim.GetMonth();
		int aadia = tim.GetDay();
		int aahor = tim.GetHour();
		int aamin = tim.GetMinute();
		int aaseg = tim.GetSecond();
		dt_atual.SetDateTime(aaano,aames,aadia,aahor,aamin,aaseg);
		pPagComp[idc].proximaverificacao = dt_atual;
	}
	else
	{
		tim += CTimeSpan(0,0,0,tempoverificacao);
		int aaano = tim.GetYear();
		int aames = tim.GetMonth();
		int aadia = tim.GetDay();
		int aahor = tim.GetHour();
		int aamin = tim.GetMinute();
		int aaseg = tim.GetSecond();
		dt_atual.SetDateTime(aaano,aames,aadia,aahor,aamin,aaseg);
		pPagComp[idc].proximaverificacao = dt_atual;
	}

	pPagComp[idc].ocorrencia = ' ';
}

/*
'=====================================================================
'===		AtuPgTeste(short idc)
'
'Objetivo:	Preenche a estrutura interna com informacoes vitais sobre as
'			paginas de testes de servicos.
'			Ataualiza as informacoes da pagina de teste de acordo com o banco
'			de dados, refente ao nome do servidor, codigo no banco de dados e,
'			e url da pagina, no servidor.
'			Verifica a ocorrencia do teste para a pagina, caso tenha ocorrido
'			algum problema, diminui o tempo de verificacao, conforme parametrizado,
'			do contrario ataualiza a pagina para o proximo momento de verificacao.
'
'Entrada :	Identificador atual interno da pagina de comparacao que acabou
'			de ser testada, independente do seu retorno.
'          
'=====================================================================
*/
void CSMSIServerApp::AtuPgTeste(long idc)
{
	char nm[20];
	char url[500];
	long cd_serv;

	url[0]='\0';
	nm[0]='\0';

	if(oAdo.AtualizaPagTeste(pPagTeste[idc].cd_interno, cd_serv, nm, url))
	{
		pPagTeste[idc].cd_interno = -1;
		return;
	}

	memcpy(pPagTeste[idc].url, url, sizeof(url));
	memcpy(pPagTeste[idc].nm, nm, sizeof(nm));
	pPagTeste[idc].inicio.m_dt = 0;
	pPagTeste[idc].fim.m_dt = 0;
	pPagTeste[idc].servidor = cd_serv;

	COleDateTime dt_atual;
	CTime tim = CTime::GetCurrentTime();
	int aano = tim.GetYear();
	int ames = tim.GetMonth();
	int adia = tim.GetDay();
	int ahor = tim.GetHour();
	int amin = tim.GetMinute();
	int aseg = tim.GetSecond();

	if(pPagTeste[idc].ocorrencia == 'N') // erro no teste atual
	{
		tim += CTimeSpan(0,0,0,tempoverificcerro);
		int aaano = tim.GetYear();
		int aames = tim.GetMonth();
		int aadia = tim.GetDay();
		int aahor = tim.GetHour();
		int aamin = tim.GetMinute();
		int aaseg = tim.GetSecond();
		dt_atual.SetDateTime(aaano,aames,aadia,aahor,aamin,aaseg);
		pPagTeste[idc].proximaverificacao = dt_atual;
	}
	else
	{
		tim += CTimeSpan(0,0,0,tempoverificacao);
		int aaano = tim.GetYear();
		int aames = tim.GetMonth();
		int aadia = tim.GetDay();
		int aahor = tim.GetHour();
		int aamin = tim.GetMinute();
		int aaseg = tim.GetSecond();
		dt_atual.SetDateTime(aaano,aames,aadia,aahor,aamin,aaseg);
		pPagTeste[idc].proximaverificacao = dt_atual;
	}

	//pPagTeste[idc].ocorrencia = ' ';
}


/*
'=====================================================================
'
'Objetivo:	Grava log referente aos servidores. Informa no log a ocorrencia de checagem do
'			servidor testado.
'
'Entrada :	
'			
'=====================================================================
*/
void CSMSIServerApp::EnviarLogServ(float segs, char *cdtperr, long i)
{
	CLogFile *oLog;
	oLog = new CLogFile();
	if(!oLog)
		return;

	oDt.GetDtHora(2);

	if( (strlen(pServidor[i].log)) <= 0 )
		strcpy(pServidor[i].log, "Objeto nao encontrado!");

	if(pServidor[i].ocorrencia == 'X') // Erro classificado como interno
	{
		oLog->AppendText("%s ; %s ; %c ; %.2f ; %.05d ; %s ; %s ; %s",        oDt.DT, MSGINTERNAS,                     'N', segs, pServidor[i].cd_interno, pServidor[i].nm, pServidor[i].ip, pServidor[i].log);
	}
	else
	{
		if(pServidor[i].ocorrencia == 'N')
		{
			oLog->AppendText("%s ; %s ; %c ; %.2f ; %.05d ; %s ; %s ; %s %s", oDt.DT, MSGSERVIDOR, pServidor[i].ocorrencia, segs, pServidor[i].cd_interno, /* */ pServidor[i].nm, pServidor[i].ip, pServidor[i].log, cdtperr);
		}
		else
		{
			oLog->AppendText("%s ; %s ; %c ; %.2f ; %.05d ; %s ; %s ; %s"   , oDt.DT, MSGSERVIDOR, pServidor[i].ocorrencia, segs, pServidor[i].cd_interno, /* */ pServidor[i].nm, pServidor[i].ip, pServidor[i].log);
		}
	}
}


/*
'=====================================================================
'===		EnviarLogPg(long cd_int, char ocr, char *servs, float segs, char *nm, char *ip, char *cdtperr)
'
'Objetivo:	Grava log referente as paginas de comparacao.
'
'Entrada :	cd_int = codigo interno do servidor
'			ocr = (S)im ou (N)ao, ocorrencia na verificacao
'			servs = MSGPAG_COMP = LOG P/ PAGINA DE COMPARACAO
'			segs = tempo de execucao
'			nm = nome do servidor
'			ip = codigo ip do servidor
'			cdtperr = "ERRGE" erro generico ou,
'					  "TMOUT" erro por time out
'			
'=====================================================================
*/
void CSMSIServerApp::EnviarLogPg(float segs, char *ip, char *cdtperr, long i)
{
	CLogFile *oLog;
	oLog = new CLogFile();
	if(!oLog)
		return;

	oDt.GetDtHora(2);

	if( (strlen(pPagComp[i].log)) <= 0 )
		strcpy(pPagComp[i].log, "Objeto nao encontrado!");

	if(pPagComp[i].ocorrencia == 'X')
	{
		oLog->AppendText("%s ; %s ; %c ; %.2f ; %.05d ; %s ; %s ; %s %s", oDt.DT, MSGINTERNAS, 'N', segs, pPagComp[i].cd_interno, pPagComp[i].nm, ip, pPagComp[i].log, cdtperr);
	}
	else
	{
		if(pPagComp[i].ocorrencia == 'S')
			oLog->AppendText("%s ; %s ; %c ; %.2f ; %.05d ; %s ; %s ; %s - %s", oDt.DT, MSGPAG_COMP, pPagComp[i].ocorrencia, segs, pPagComp[i].cd_interno, pPagComp[i].nm, ip, "Sucesso na comparacao!", pPagComp[i].log);
		else
			oLog->AppendText("%s ; %s ; %c ; %.2f ; %.05d ; %s ; %s ; %s - %s - %s", oDt.DT, MSGPAG_COMP, pPagComp[i].ocorrencia, segs, pPagComp[i].cd_interno, pPagComp[i].nm, ip, "Erro na comparacao!", pPagComp[i].log, cdtperr);
	}

}


/*
'=====================================================================
'===		EnviarLogTST(long cd_int, char ocr, char *servs, float segs, char *nm, char *ip, char *cdtperr, char *ret)
'
'Objetivo:	Grava log referente as paginas de teste.
'
'Entrada :	cd_int = codigo interno do servidor
'			ocr = (S)im ou (N)ao, ocorrencia na verificacao
'			servs = MSGPAG_TEST = LOG P/ PAGINA DE TESTES
'			segs = tempo de execucao
'			nm = nome do servidor
'			ip = codigo ip do servidor
'			cdtperr = "ERRGE" erro generico ou,
'					  "TMOUT" erro por time out
'			
'=====================================================================
*/
void CSMSIServerApp::EnviarLogTST(float segs, char *ip, char *cdtperr, long i)
{
	CLogFile *oLog;
	oLog = new CLogFile();
	if(!oLog)
		return;

	oDt.GetDtHora(2);

	if( (strlen(pPagTeste[i].log)) <= 0 )
		strcpy(pPagTeste[i].log, "Objeto nao encontrado!");

	if(pPagTeste[i].ocorrencia == 'X')
		oLog->AppendText("%s ; %s ; %c ; %.2f ; %.05d ; %s ; %s ; %s",                 oDt.DT, MSGINTERNAS, pPagTeste[i].ocorrencia, segs, pPagTeste[i].cd_interno, pPagTeste[i].nm, ip, pPagTeste[i].log);
	else
	{
		if(pPagTeste[i].ocorrencia == 'N')
		{
			oLog->AppendText("%s ; %s ; %c ; %.2f ; %.05d ; %s ; %s ; %s %s %s %s",    oDt.DT, MSGPAG_TEST, pPagTeste[i].ocorrencia, segs, pPagTeste[i].cd_interno, /* */ pPagTeste[i].nm, ip, "Erro no teste.", pPagTeste[i].log, cdtperr, pPagTeste[i].str);
		}
		else
		{
			oLog->AppendText("%s ; %s ; %c ; %.2f ; %.05d ; %s ; %s ; %s %s %s",         oDt.DT, MSGPAG_TEST, pPagTeste[i].ocorrencia, segs, pPagTeste[i].cd_interno, /* */ pPagTeste[i].nm, ip, "Sucesso no teste!", pPagTeste[i].log, pPagTeste[i].str);
		}
	}
}


/*
'=====================================================================
'
'Objetivo:	Finalizar a verificação dos serviços.
'
'=====================================================================
*/
VOID CSMSIServerApp::CloseApp()
{
	short i;

	try
	{
		for(i=0;i<QTDMAXSERVS;i++)
		{
			CloseHandle(pServidor[i].hThread);
		}
	}
	catch(CNotSupportedException e)
	{
		TCHAR err[255];
		e.GetErrorMessage(err,255);
		EnviarLog(GetLastError(), "CSMSIServerApp::CloseApp 01","Finalizando Threads!");
	}

	try
	{
		for(i=0;i<QTDMAXPAGCOMP;i++)
		{
			CloseHandle(pPagComp[i].hThread);
			
		}
	}
	catch(CNotSupportedException e)
	{
		TCHAR err[255];
		e.GetErrorMessage(err,255);
		EnviarLog(GetLastError(), "CSMSIServerApp::CloseApp 02","Finalizando Threads!");
	}
	
	try
	{
		for(i=0;i<QTDMAXPAGTESTE;i++)
		{
			CloseHandle(pPagTeste[i].hThread);
			
		}
	}
	catch(CNotSupportedException e)
	{
		TCHAR err[255];
		e.GetErrorMessage(err,255);
		EnviarLog(GetLastError(), "CSMSIServerApp::CloseApp 03","Finalizando Threads!");
	}
}

// *********************************************************************************** //

VOID CSMSIServerApp::EnviarLog(DWORD lsterr, char *rot, char *csz)
{
	CLogFile oLog;

	DataHora oDt;

	LPVOID	lpMsgBuf;

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

	char ch[TAMCONN];
	char err[TAMCONN];

	strcpy(err,(LPCTSTR)lpMsgBuf);

	int tam = strlen(err);

	err[tam-2] = '\0';

	oDt.GetDtHora(2);

	sprintf(ch, "%s ; %s ; %c ; %.2f ; 00000 ; ",  oDt.DT, MSGINTERNAS, 'N', 0 );

	strcat(ch, csz);
	strcat(ch, " ; ");
	strcat(ch, rot);
	strcat(ch, " ; ");
	strcat(ch, err);

	oLog.AppendText(ch);

	LocalFree( lpMsgBuf );
}