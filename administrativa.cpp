/*
'=======================================================================================
'= Arquivo: Administrativa.cpp
'= Função:  implementa a administracao de execucao do programa como
'=			um servico e todas as classes operacionais
'=			
'= Autor:   Getson Miranda
'= Data:    20/10/2003
'=======================================================================================
*/

// Administrativa.cpp: implementation of the CAdministrativa class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "Administrativa.h"
#include "SMSIServerApp.h"
#include "DataHora.h"
#include "LogFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CAdministrativa::CAdministrativa() 
: CNTService("ROBOSERVER")	// nome do servico no SCM
{							// herda todas as caracteristicas da classe NTService
	m_iStartParam = 0;
	m_iIncParam = 1;
	m_iState = m_iStartParam;
}

/*
'=====================================================================
'=== OnInit()
'
'Objetivo:	Registra o robo no banco de dados do Windows, se o Robo ja
'			estiver instalado, prepara para inicio de operacao.
'			
'Saida...:	Sempre TRUE
'=====================================================================
*/
BOOL CAdministrativa::OnInit()
{
	// leitura de parametros no registro
    // Abre a chave de registros
    // HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\<AppName>\Parameters
    HKEY hkey;
	char szKey[1024];
	strcpy(szKey, "SYSTEM\\CurrentControlSet\\Services\\");
	strcat(szKey, m_szServiceName);
	strcat(szKey, "\\Parameters");

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     szKey,
                     0,
                     KEY_QUERY_VALUE,
                     &hkey) == ERROR_SUCCESS) 
	{
        // Servico instalado
        DWORD dwType = 0;
        DWORD dwSize = sizeof(m_iStartParam);

        RegQueryValueEx(hkey,
                        "Start",
                        NULL,
                        &dwType,
                        (BYTE*)&m_iStartParam,
                        &dwSize);

        dwSize = sizeof(m_iIncParam);

        RegQueryValueEx(hkey,
                        "Inc",
                        NULL,
                        &dwType,
                        (BYTE*)&m_iIncParam,
                        &dwSize);

        RegCloseKey(hkey);
    }

	// determina que o servico esta em inicio de operacao
	m_iState = m_iStartParam;

	return TRUE;
}


/*
'=====================================================================
'===		Run()
'
'Objetivo:	Loop principal do programa, onde eh feita a atualalizacao
'			do Robo junto ao SCM. Dispara os servicos de checagem do
'			site atraves do metodo Processamento() da classe SMSIServerApp.
'
'=====================================================================
*/
void CAdministrativa::Run()
{
	char err[32];
	oMib.Start(err);

    while (m_bIsRunning)
	{

		CSMSIServerApp *oTst;
		oTst = new CSMSIServerApp();

		while(oTst->smsirun && m_bIsRunning)
		{
			m_iState += m_iIncParam;

			oTst->Processamento();

			oTst->AppStatus = m_bIsRunning;

			oMib.RoboStatus(oTst->AppStatus);

			Sleep(1000);

			while(m_bIsRunning == 2)
			{
				Sleep(1000);
				oMib.RoboStatus(oTst->AppStatus);
			}
		}

		if(oTst->smsirun == 0)
			break;

		delete oTst;
    }

	oMib.Stop();
}


/*
'=====================================================================
'===		OnUserControl(DWORD dwOpcode)
'
'Objetivo:	Salvar o status do Robo, no banco de registros do Windows.
'			Disponibiliza interface com usuario atraves do SCM.
'
'Entrada.:	dwOpcode = 1, usuario solicitou parada do servico
'
'Valor		true - parar os servicos
'			false - continuar executando os servicos
'          
'=====================================================================
*/
// Process user control requests
BOOL CAdministrativa::OnUserControl(DWORD dwOpcode)
{
    switch (dwOpcode) 
	{
    case SERVICE_CONTROL_USER + 0:

        // Salva status do servico no registro
        SaveStatus();
        return TRUE;

    default:
        break;
    }
    return FALSE; // nao instanciado
}


/*
'=====================================================================
'===		SaveStatus()
'
'Objetivo:	Salva o status do servico Robo, no banco de dados de registro
'			do Windows.
'
'=====================================================================
*/
// Save the current status in the registry
void CAdministrativa::SaveStatus()
{
    //DebugMsg("Saving current status");
    // Try opening the registry key:
    // HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\<AppName>\...
    HKEY hkey = NULL;
	char szKey[1024];
	strcpy(szKey, "SYSTEM\\CurrentControlSet\\Services\\");
	strcat(szKey, m_szServiceName);
	strcat(szKey, "\\Status");
    DWORD dwDisp;
	DWORD dwErr;
    //DebugMsg("Creating key: %s", szKey);
    
	dwErr = RegCreateKeyEx(	HKEY_LOCAL_MACHINE,
                           	szKey,
                   			0,
                   			"",
                   			REG_OPTION_NON_VOLATILE,
                   			KEY_WRITE,
                   			NULL,
                   			&hkey,
                   			&dwDisp);
	
	if (dwErr != ERROR_SUCCESS) 
	{
		//DebugMsg("Failed to create Status key (%lu)", dwErr);
		return;
	}	

    // Set the registry values
	//DebugMsg("Saving 'Current' as %ld", m_iState); 
    RegSetValueEx(hkey,
                  "Current",
                  0,
                  REG_DWORD,
                  (BYTE*)&m_iState,
                  sizeof(m_iState));

    // Finished with key
    RegCloseKey(hkey);
}
