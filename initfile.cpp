// InitFile.cpp: implementation of the CInitFile class.
//
//////////////////////////////////////////////////////////////////////

/*
'=======================================================================================
'			Leitura de seqüências e chaves no arquivo Robo.ini
'
'= Arquivo: InitFile.cpp
'= Função:  Implementa a leitura do arquivo de inicialização, Robo.ini
'= Autor:   Getson Miranda
'= Data:    21/10/2003
'=======================================================================================
*/

#include "stdafx.h"
#include "InitFile.h"

/*
'=====================================================================
'===		
'
'Objetivo:	
'
'Entrada.:	nenhuma
'          
'Saida...:	
'
'Retornos:	
'=====================================================================
*/
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CInitFile::CInitFile()
{
	GetArqIniApp();
}
// *****************************************************************************************
CInitFile::~CInitFile()
{
}

/*
'=====================================================================
'===		
'
'Objetivo:	Retorna em TRAP_RECEIVER_PORT o número da porta, 
'			obtido no arquivo INI, para envio de Trap.
'
'Entrada.:	TRAP_RECEIVER_PORT
'
'Retornos:	TRAP_RECEIVER_PORT
'=====================================================================
*/
VOID CInitFile::GetTRAP_RECEIVER_PORTValue( char * TRAP_RECEIVER_PORT )
{
	char ch[TAMPATHDEFAULT];

	GetPrivateProfileString(	"SNMP_HOST_INFO",
								"TRAP_RECEIVER_PORT",
								NULL,
								ch,
								sizeof(ch),
								nm );

	memcpy(TRAP_RECEIVER_PORT, ch, sizeof(ch));
}

/*
'=====================================================================
'===		
'
'Objetivo:	Retorna em TRAP_RECEIVER o número do endereço IP 
'			do servidor, obtido no arquivo INI, para o envio 
'			de Traps.
'
'Entrada.:	TRAP_RECEIVER
'
'Retornos:	TRAP_RECEIVER
'=====================================================================
*/
VOID CInitFile::GetTRAP_RECEIVERValue( char * TRAP_RECEIVER )
{
	char ch[TAMPATHDEFAULT];

	GetPrivateProfileString(	"SNMP_HOST_INFO",
								"TRAP_RECEIVER",
								NULL,
								ch,
								sizeof(ch),
								nm );

	memcpy(TRAP_RECEIVER, ch, sizeof(ch));
}

/*
'=====================================================================
'===		
'
'Objetivo:	Retorna em TRAP_VERSION o número da versão da Trap
'			obtido no arquivo INI.
'	
'Entrada.:	TRAP_VERSION
'          
'Retornos:	TRAP_VERSION
'=====================================================================
*/
VOID CInitFile::GetTRAP_VERSIONValue( char * TRAP_VERSION )
{
	char ch[TAMCONN];

	GetPrivateProfileString(	"SNMP_HOST_INFO",
								"TRAP_VERSION",
								NULL,
								ch,
								sizeof(ch),
								nm );

	memcpy(TRAP_VERSION, ch, sizeof(ch));
}

/*
'=====================================================================
'===		
'
'Objetivo:	Retorna em TRAP_COMUNITY a comunidade da Trap.
'
'Entrada.:	TRAP_COMUNITY
'
'Retornos:	TRAP_COMUNITY
'=====================================================================
*/
VOID CInitFile::GetTRAP_COMUNITYValue( char * TRAP_COMUNITY )
{
	char ch[TAMPATHDEFAULT];

	GetPrivateProfileString(	"SNMP_HOST_INFO",
								"TRAP_COMUNITY",
								NULL,
								ch,
								sizeof(ch),
								nm );

	memcpy(TRAP_COMUNITY, ch, sizeof(ch));
}

/*
'=====================================================================
'===		
'
'Objetivo:	Retorna em TRAP_ENTERPRISE o código corporativo da instituição, obtido no arquivo INI
'
'Entrada.:	TRAP_ENTERPRISE
'          
'Retornos:	TRAP_ENTERPRISE
'=====================================================================
*/
VOID CInitFile::GetTRAP_ENTERPRISEValue( char * TRAP_ENTERPRISE )
{
	char ch[TAMPATHDEFAULT];

	GetPrivateProfileString(	"SNMP_HOST_INFO",
								"TRAP_ENTERPRISE",
								NULL,
								ch,
								sizeof(ch),
								nm );

	memcpy(TRAP_ENTERPRISE, ch, sizeof(ch));
}

/*
'=====================================================================
'===		
'
'Objetivo:	Retorna em TRAP_GENTRAP um código genérico para 
'			trap.
'
'Entrada.:	TRAP_GENTRAP
'          
'Retornos:	TRAP_GENTRAP
'=====================================================================
*/
VOID CInitFile::GetTRAP_GENTRAPValue( char * TRAP_GENTRAP )
{
	char ch[TAMPATHDEFAULT];

	GetPrivateProfileString(	"SNMP_HOST_INFO",
								"TRAP_GENTRAP",
								NULL,
								ch,
								sizeof(ch),
								nm );

	memcpy(TRAP_GENTRAP, ch, sizeof(ch));
}


/*
'=====================================================================
'===		
'
'Objetivo:	Retorna em TRAP_SPECTRAP código específico para 
'			trap, obtido no arquivo INI.
'
'Entrada.:	TRAP_SPECTRAP
'          
'Retornos:	TRAP_SPECTRAP
'=====================================================================
*/
VOID CInitFile::GetTRAP_SPECTRAPValue( char * TRAP_SPECTRAP )
{
	char ch[TAMPATHDEFAULT];

	GetPrivateProfileString(	"SNMP_HOST_INFO",
								"TRAP_SPECTRAP",
								NULL,
								ch,
								sizeof(ch),
								nm );

	memcpy(TRAP_SPECTRAP, ch, sizeof(ch));
}



/*
'=====================================================================
'===		
'
'Objetivo:	Retorna em DRIVER o driver de conexão para acesso 
'			ao banco de dados.
'
'Entrada.:	DRIVER
'
'Retornos:	DRIVER
'=====================================================================
*/
DWORD CInitFile::GetDRIVERValue( char * DRIVER )
{
	char ch[TAMCONN];

	GetPrivateProfileString(	"BASE_DADOS",
								"DRIVER",
								NULL,
								ch,
								sizeof(ch),
								nm );

	strcpy(DRIVER, ch);

	if(!strlen(DRIVER))
		return 1;
	else
		return 0;
}

/*
'=====================================================================
'===		
'
'Objetivo:	Retorna em DBQ o caminho ou string para acesso ao 
'			banco de dados ou, suas tabelas
'
'Entrada.:	DBQ
'          
'Retornos:	DBQ
'=====================================================================
*/
DWORD CInitFile::GetDBQValue( char * DBQ )
{
	char ch[TAMCONN];

	GetPrivateProfileString(	"BASE_DADOS",
								"DBQ",
								NULL,
								ch,
								sizeof(ch),
								nm );

	strcpy(DBQ, ch);

	if(!strlen(DBQ))
		return 1;
	else
		return 0;
}

/*
'=====================================================================
'===		
'
'Objetivo:	Retorna em UID o usuário para conexão com o banco 
'			de dados.
'
'Entrada.:	UID
'
'Retornos:	UID
'=====================================================================
*/
DWORD CInitFile::GetUIDValue( char * UID )
{
	char ch[TAMCONN];

	GetPrivateProfileString(	"BASE_DADOS",
								"UID",
								NULL,
								ch,
								sizeof(ch),
								nm );

	strcpy(UID, ch);

	if(!strlen(UID))
		return 1;
	else
		return 0;
}

/*
'=====================================================================
'===		
'
'Objetivo:	Retorna em PWD a senha do usuário para conexão com
'			o banco de dados.
'
'Entrada.:	PWD
'
'Retornos:	PWD
'=====================================================================
*/
DWORD CInitFile::GetPWDValue( char * PWD )
{
	char ch[TAMCONN];

	GetPrivateProfileString(	"BASE_DADOS",
								"PWD",
								NULL,
								ch,
								sizeof(ch),
								nm );

	strcpy(PWD, ch);

	if(!strlen(PWD))
		return 1;
	else
		return 0;
}

/*
'=====================================================================
'===		
'
'Objetivo:	Retorna em OutrosConnectionString uma string 
'			complementar para conexão com o banco de dados.
'
'Entrada.:	OutrosConnectionString
'
'Retornos:	OutrosConnectionString
'=====================================================================
*/
DWORD CInitFile::GetOutrosConnectionStringValue( char * OutrosConnectionString )
{
	char ch[TAMCONN];

	GetPrivateProfileString(	"BASE_DADOS",
								"OutrosConnectionString",
								NULL,
								ch,
								sizeof(ch),
								nm );

	strcpy(OutrosConnectionString, ch);

	if(!strlen(OutrosConnectionString))
		return 1;
	else
		return 0;
}

/*
'=====================================================================
'===		
'
'Objetivo:	Retorna em ConnectionTimeout o tempo de espera da 
'			resposta do servidor, para conexão com o banco de 
'			dados.
'
'Entrada.:	ConnectionTimeout
'
'Retornos:	ConnectionTimeout
'=====================================================================
*/
DWORD CInitFile::GetConnectionTimeoutValue( char * ConnectionTimeout )
{
	char ch[TAMCONN];

	GetPrivateProfileString(	"BASE_DADOS",
								"ConnectionTimeout",
								NULL,
								ch,
								sizeof(ch),
								nm );

	strcpy(ConnectionTimeout, ch);

	if(!strlen(ConnectionTimeout))
		return 1;
	else
		return 0;
}

/*
'=====================================================================
'===		
'
'Objetivo:	Retorna em CommandTimeout comando para execução, 
'			em caso de time out.
'
'Entrada.:	CommandTimeout
'
'Retornos:	CommandTimeout
'=====================================================================
*/
DWORD CInitFile::GetCommandTimeoutValue( char * CommandTimeout )
{
	char ch[TAMCONN];

	GetPrivateProfileString(	"BASE_DADOS",
								"CommandTimeout",
								NULL,
								ch,
								sizeof(ch),
								nm );

	strcpy(CommandTimeout, ch);

	if(!strlen(CommandTimeout))
		return 1;
	else
		return 0;
}


/*
'=====================================================================
'===		
'
'Objetivo:	Retorna em LOGDIR o diretório para gravação dos 
'			arquivos diários de LOG.
'
'Entrada.:	LOGDIR
'
'Retornos:	LOGDIR
'=====================================================================
*/
VOID CInitFile::GetLOGDIRValue( char * LOGDIR )
{
	char ch[50];

	GetPrivateProfileString(	"GERAL",
								"LOGDIR",
								NULL,
								ch,
								sizeof(ch),
								nm );

	memcpy(LOGDIR, ch, sizeof(ch));
}


/*
'=====================================================================
'===		caminho da aplicacao robo + nome do arquivo ini
'
'Objetivo:	recupera o diretorio de execucao do robo
'			acrescenta uma barra \ e o nome do arquivo INI
'
'=====================================================================
*/
VOID CInitFile::GetArqIniApp()
{
	char szFilePath[_MAX_PATH];					// buffer

	//GetLOGDIRValue( szFilePath );

	::GetModuleFileName(NULL, szFilePath, sizeof(szFilePath));

	long tam = strlen(szFilePath);

	tam -= 14;

	strcpy(&szFilePath[tam],NOMEARQINI);	// acrescenta o nome do arquivo ini

	strcpy(nm,szFilePath);					// retorna o nome do arquivo ini
}


/*
*
*
*
*/
VOID CInitFile::GetMibOID( char *OID )
{
	char ch[32];

	GetPrivateProfileString(	"MIB",
								"OID",
								NULL,
								ch,
								sizeof(ch),
								nm );

	memcpy(OID, ch, sizeof(ch));
}


/*
*
*
*
*/
VOID CInitFile::GetMibPorta( char *PORTA )
{
	char ch[32];

	GetPrivateProfileString(	"MIB",
								"PORTA",
								NULL,
								ch,
								sizeof(ch),
								nm );

	memcpy(PORTA, ch, sizeof(ch));
}


/*
*
*
*
*/
VOID CInitFile::GetMibVersao( char *VERSAO )
{
	char ch[32];

	GetPrivateProfileString(	"MIB",
								"VERSAO",
								NULL,
								ch,
								sizeof(ch),
								nm );

	memcpy(VERSAO, ch, sizeof(ch));
}



/*
*
*
*
*/
VOID CInitFile::GetMibComunidade( char *CMNDD )
{
	char ch[32];

	GetPrivateProfileString(	"MIB",
								"COMUNIDADE",
								NULL,
								ch,
								sizeof(ch),
								nm );

	memcpy(CMNDD, ch, sizeof(ch));
}



/*
*
*
*
*/
VOID CInitFile::GetMibMsg( char *MSG )
{
	char ch[32];

	GetPrivateProfileString(	"MIB",
								"MENSAGEM",
								NULL,
								ch,
								sizeof(ch),
								nm );

	memcpy(MSG, ch, sizeof(ch));
}