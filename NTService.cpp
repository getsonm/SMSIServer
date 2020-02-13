// NTService.cpp
//
// Implementation of CNTService

#include "stdafx.h"
#include "NTService.h"

// static variables
CNTService* CNTService::m_pThis = NULL;

/*
'=====================================================================
'Objetivo:	Trata da inicialização da classe e da aplicação 
'			do Robo, seta parâmetros iniciais para instalação 
'			do robo como serviço do Windows.
'
'=====================================================================
*/
CNTService::CNTService(const char* szServiceName)
{
    // AVISO: Os limites do serviço, só podem ser definidos nessa classe.
    m_pThis = this;
    
    // Seta o nome do serivco e a versao em execucao
    strncpy(m_szServiceName, szServiceName, sizeof(m_szServiceName)-1);
    m_iMajorVersion = 1;
    m_iMinorVersion = 1;
    m_hEventSource = NULL;

    // seta o status inicial do robo para instalacao
    m_hServiceStatus = NULL;
    m_Status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    m_Status.dwCurrentState = SERVICE_STOPPED;
    m_Status.dwControlsAccepted = SERVICE_ACCEPT_PAUSE_CONTINUE | SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    m_Status.dwWin32ExitCode = 0;
    m_Status.dwServiceSpecificExitCode = 0;
    m_Status.dwCheckPoint = 0;
    m_Status.dwWaitHint = 0;
    m_bIsRunning = 0;
}

CNTService::~CNTService()
{
    if (m_hEventSource) {
        ::DeregisterEventSource(m_hEventSource);
    }
}

/*
'=====================================================================
'Objetivo:	Recebe na linha de comando, parâtros para 
'			operação interna do robo. Trata os parâmetros 
'			recebidos em arqv, referentes a instalação, 
'			remoção ou retorno dos status e versão de um 
'			serviço, esteja ele instalado ou não.
'
'Entrada	argc = quantidade de argumentos recebidos
'			argv = string para processamento do robo
'
'Retornos	True - os parâmetros recebidos foram reconhecidos
'			False - não reconheceu os parâmetros recebidos
'=====================================================================
*/
////////////////////////////////////////////////////////////////////////////////////////
// Default command line argument parsing
// Returns TRUE if it found an arg it recognised, FALSE if not
// Note: processing some arguments causes output to stdout to be generated.
BOOL CNTService::ParseStandardArgs(int argc, char* argv[])
{
	CString msg;
    
    if (argc <= 1) { return FALSE; }

    if (_stricmp(argv[1], "-v") == 0) 
	{
		msg.Format("%s Versao %d.%d\nServico %s instalado.\n", m_szServiceName, m_iMajorVersion, m_iMinorVersion, IsInstalled() ? "esta" : "nao");
		MessageBox(0, msg, "ROBO MONITORAMENTO [SMSI]", MB_ICONINFORMATION+MB_OK);
        return TRUE; // say we processed the argument
    } 
	else if (_stricmp(argv[1], "-i") == 0) 
	{
        // Request to install.
        if (IsInstalled()) 
		{
			msg.Format("%s esta instalado.\n", m_szServiceName);
			MessageBox(0, msg, "ROBO MONITORAMENTO [SMSI]", MB_ICONINFORMATION+MB_OK);
        } 
		else 
		{
            // Try and install the copy that's running
            if (Install()) 
			{
				msg.Format("%s instalado\n", m_szServiceName);
				MessageBox(0, msg, "ROBO MONITORAMENTO [SMSI]", MB_ICONINFORMATION+MB_OK);
            } 
			else 
			{
				msg.Format("%s erro na instalacao. Error %d\n", m_szServiceName, GetLastError());
				MessageBox(0, msg, "ROBO MONITORAMENTO [SMSI]", MB_ICONSTOP+MB_OK);
            }
        }
        return TRUE; // say we processed the argument
    } 
	else if (_stricmp(argv[1], "-u") == 0) 
	{
        // Request to uninstall.
        if (!IsInstalled()) 
		{
			msg.Format("%s nao instalado\n", m_szServiceName);
			MessageBox(0, msg, "ROBO MONITORAMENTO [SMSI]", MB_ICONINFORMATION+MB_OK);
        } 
		else 
		{
            // Try and remove the copy that's installed
            if (Uninstall()) 
			{
                // Get the executable file path
                char szFilePath[_MAX_PATH];
                ::GetModuleFileName(NULL, szFilePath, sizeof(szFilePath));
				msg.Format("%s removido. (Apague (%s).)\n",m_szServiceName, szFilePath);
				MessageBox(0, msg, "ROBO MONITORAMENTO [SMSI]", MB_ICONINFORMATION+MB_OK);
            } 
			else 
			{
				msg.Format("Nao foi removido %s. Error %d\n", m_szServiceName, GetLastError());
				MessageBox(0, msg, "ROBO MONITORAMENTO [SMSI]", MB_ICONINFORMATION+MB_OK);
            }
        }
        return TRUE; // say we processed the argument
    }
         
    return FALSE;
}

/*
'=====================================================================
'Objetivo:	Abre o serviço para testar se está instalado no SCM.
'
'Retornos	True - serviço instalado
'			False - não instalado
'=====================================================================
*/
////////////////////////////////////////////////////////////////////////////////////////
// Install/uninstall routines
// Test if the service is currently installed
BOOL CNTService::IsInstalled()
{
    BOOL bResult = FALSE;

    // Open the Service Control Manager
    SC_HANDLE hSCM = ::OpenSCManager(NULL, // local machine
                                     NULL, // ServicesActive database
                                     SC_MANAGER_ALL_ACCESS); // full access
    if (hSCM) {

        // Try to open the service
        SC_HANDLE hService = ::OpenService(hSCM,
                                           m_szServiceName,
                                           SERVICE_QUERY_CONFIG);
        if (hService) {
            bResult = TRUE;
            ::CloseServiceHandle(hService);
        }

        ::CloseServiceHandle(hSCM);
    }
    
    return bResult;
}

/*
'=====================================================================
'Objetivo:	Instala o robo como serviço do Windows.
'
'Retornos	True - serviço instalado
'			False - não foi instalado
'=====================================================================
*/
BOOL CNTService::Install()
{
    // Open the Service Control Manager
    SC_HANDLE hSCM = ::OpenSCManager(NULL, // local machine
                                     NULL, // ServicesActive database
                                     SC_MANAGER_ALL_ACCESS); // full access
    if (!hSCM) return FALSE;

    // Get the executable file path
    char szFilePath[_MAX_PATH];
    ::GetModuleFileName(NULL, szFilePath, sizeof(szFilePath));

    // Create the service
    SC_HANDLE hService = ::CreateService(hSCM,
                                         m_szServiceName,
                                         m_szServiceName,
                                         SERVICE_ALL_ACCESS,
                                         SERVICE_WIN32_OWN_PROCESS,
                                         SERVICE_AUTO_START,        // start condition
                                         SERVICE_ERROR_NORMAL,
                                         szFilePath,
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL);
    if (!hService) {
        ::CloseServiceHandle(hSCM);
        return FALSE;
    }

    char szKey[256];
    HKEY hKey = NULL;
    strcpy(szKey, "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\");

    strcat(szKey, m_szServiceName);
    if (::RegCreateKey(HKEY_LOCAL_MACHINE, szKey, &hKey) != ERROR_SUCCESS) {
        ::CloseServiceHandle(hService);
        ::CloseServiceHandle(hSCM);
        return FALSE;
    }

    // Add the Event ID message-file name to the 'EventMessageFile' subkey.
    ::RegSetValueEx(hKey,
                    "EventMessageFile",
                    0,
                    REG_EXPAND_SZ, 
                    (CONST BYTE*)szFilePath,
                    strlen(szFilePath) + 1);     

    // Set the supported types flags.
    DWORD dwData = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
    ::RegSetValueEx(hKey,
                    "TypesSupported",
                    0,
                    REG_DWORD,
                    (CONST BYTE*)&dwData,
                     sizeof(DWORD));
    ::RegCloseKey(hKey);

    LogEvent(EVENTLOG_INFORMATION_TYPE, EVMSG_INSTALLED, m_szServiceName);

    // tidy up
    ::CloseServiceHandle(hService);
    ::CloseServiceHandle(hSCM);
    return TRUE;
}


/*
'=====================================================================
'Objetivo:	Remove e exclui o robo como serviço do Windows.
'
'Retornos	True - removido
'			False - problema na remocao
'=====================================================================
*/
BOOL CNTService::Uninstall()
{
    // Open the Service Control Manager
    SC_HANDLE hSCM = ::OpenSCManager(NULL, // local machine
                                     NULL, // ServicesActive database
                                     SC_MANAGER_ALL_ACCESS); // full access
    if (!hSCM) return FALSE;

    BOOL bResult = FALSE;
    SC_HANDLE hService = ::OpenService(hSCM,
                                       m_szServiceName,
                                       DELETE);
    if (hService) {
        if (::DeleteService(hService)) {
            LogEvent(EVENTLOG_INFORMATION_TYPE, EVMSG_REMOVED, m_szServiceName);
            bResult = TRUE;
        } else {
            LogEvent(EVENTLOG_ERROR_TYPE, EVMSG_NOTREMOVED, m_szServiceName);
        }
        ::CloseServiceHandle(hService);
    }
    
    ::CloseServiceHandle(hSCM);
    return bResult;
}


/*
'=====================================================================
'Objetivo:	Grava no Event Viewer do Windows resultados 
'			internos de execução, instalação e parada do Robo.
'
'Entradas:	wType, determina o tipo de evento a ser logado
'			dwID, determina o evento
'			pszS1, ponteiro para o primeiro grupo de texto a ser gravado
'			pszS2, ponteiro para o segundo grupo de texto a ser gravado
'			pszS3, ponteiro para o terceiro grupo de texto a ser gravado
'
'=====================================================================
*/
///////////////////////////////////////////////////////////////////////////////////////
// Logging functions
// This function makes an entry into the application event log
void CNTService::LogEvent(WORD wType, DWORD dwID,
                          const char* pszS1,
                          const char* pszS2,
                          const char* pszS3)
{
    const char* ps[3];
    ps[0] = pszS1;
    ps[1] = pszS2;
    ps[2] = pszS3;

    int iStr = 0;
    for (int i = 0; i < 3; i++) {
        if (ps[i] != NULL) iStr++;
    }
        
    // Check the event source has been registered and if
    // not then register it now
    if (!m_hEventSource) {
        m_hEventSource = ::RegisterEventSource(NULL,  // local machine
                                               m_szServiceName); // source name
    }

    if (m_hEventSource) {
        ::ReportEvent(m_hEventSource,
                      wType,
                      0,
                      dwID,
                      NULL, // sid
                      iStr,
                      0,
                      ps,
                      NULL);
    }
}


/*
'=====================================================================
'Objetivo:	Inicia a execução das rotinas envolvidas no 
'			processamento, do Robo como um serviço do Windows. 
'			Para o controle de execução para o método Run da 
'			classe ou de suas herdeiras.
'
'Retornos	True - inicializado
'			False - erro na inicializacao
'
'=====================================================================
*/
//////////////////////////////////////////////////////////////////////////////////////////////
// Service startup and registration
BOOL CNTService::StartService()
{
    SERVICE_TABLE_ENTRY st[] = 
	{
        {m_szServiceName, ServiceMain},
        {NULL, NULL}
    };

    BOOL b = ::StartServiceCtrlDispatcher(st);
    return b;
}


/*
'=====================================================================
'
'Objetivo:	Função cadastrada no SCM para disparar a execução 
'			do Robo.
'
'=====================================================================
*/
// static member function (callback)
void CNTService::ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
    // Get a pointer to the C++ object
    CNTService* pService = m_pThis;
	
    // Register the control request handler
    pService->m_Status.dwCurrentState = SERVICE_RUNNING;

    pService->m_hServiceStatus = RegisterServiceCtrlHandler(
						pService->m_szServiceName,
						Handler);

    if (pService->m_hServiceStatus == NULL) 
	{
        pService->LogEvent(EVENTLOG_ERROR_TYPE, EVMSG_CTRLHANDLERNOTINSTALLED);
        return;
    }

    // Start the initialisation
    if (pService->Initialize()) 
	{
        // Do the real work. 
        // When the Run function returns, the service has stopped.
        pService->m_bIsRunning = 1;
        pService->m_Status.dwWin32ExitCode = 0;
        pService->m_Status.dwCheckPoint = 0;
        pService->m_Status.dwWaitHint = 0;
        pService->Run();
    }

	// Tell the service manager we are stopped
    pService->SetStatus(SERVICE_STOPPED);

}


/*
'=====================================================================
'
'Objetivo:	Altera o status de execução do robo.
'
'Entradas	dwState, status de execução do servico
'			 = 0 é para  parar
'			!= 0 continua executando
'
'=====================================================================
*/
///////////////////////////////////////////////////////////////////////////////////////////
// status functions
void CNTService::SetStatus(DWORD dwState)
{
    m_Status.dwCurrentState = dwState;
    ::SetServiceStatus(m_hServiceStatus, &m_Status);
}


/*
'=====================================================================
'
'Objetivo:	Altera o status do robo para executando.
'
'Retornos	True = sucesso
'			False = falha ao iniciar
'
'=====================================================================
*/
///////////////////////////////////////////////////////////////////////////////////////////
// Service initialization
BOOL CNTService::Initialize()
{
    // Start the initialization
    SetStatus(SERVICE_START_PENDING);
    
    // Perform the actual initialization
    BOOL bResult = OnInit(); 
    
    // Set final state
    m_Status.dwWin32ExitCode = GetLastError();
    m_Status.dwCheckPoint = 0;
    m_Status.dwWaitHint = 0;

    if (!bResult) 
	{
        LogEvent(EVENTLOG_ERROR_TYPE, EVMSG_FAILEDINIT);
        SetStatus(SERVICE_STOPPED);
        return FALSE;    
    }
    
    LogEvent(EVENTLOG_INFORMATION_TYPE, EVMSG_STARTED);

    SetStatus(SERVICE_RUNNING);

    return TRUE;
}


/*
'=====================================================================
'
'Objetivo:	Loop de execução do serviço.
'
'=====================================================================
*/
///////////////////////////////////////////////////////////////////////////////////////////////
// main function to do the real work of the service
// This function performs the main work of the service. 
// When this function returns the service has stopped.
void CNTService::Run()
{
    while (m_bIsRunning) 
	{
        Sleep(1000);
    }
}


/*
'=====================================================================
'
'Objetivo:	Altera o Handle de execução do robo.
'
'Entradas	dwOpcode = opçao de execução.
'
'=====================================================================
*/
//////////////////////////////////////////////////////////////////////////////////////
// Control request handlers
// static member function (callback) to handle commands from the
// service control manager
void CNTService::Handler(DWORD dwOpcode)
{
    // Get a pointer to the object
    CNTService* pService = m_pThis;
    
    switch (dwOpcode) 
	{
    case SERVICE_CONTROL_STOP:	// 1
        pService->SetStatus(SERVICE_STOP_PENDING);
        pService->OnStop();		// Finalizar threads (TODAS)
        pService->LogEvent(EVENTLOG_INFORMATION_TYPE, EVMSG_STOPPED);
		pService->m_bIsRunning = 0;
        break;

    case SERVICE_CONTROL_PAUSE: // 2
		pService->SetStatus(SERVICE_PAUSE_PENDING);
        pService->OnPause();
        pService->LogEvent(EVENTLOG_WARNING_TYPE, SERVICE_PAUSED);
		pService->SetStatus(SERVICE_PAUSED);
		pService->m_bIsRunning = 2;
        break;

    case SERVICE_CONTROL_CONTINUE: // 3
        pService->OnContinue();
		pService->LogEvent(EVENTLOG_INFORMATION_TYPE, SERVICE_RUNNING);
		pService->SetStatus(SERVICE_RUNNING);
		pService->m_bIsRunning = 1;
        break;

    case SERVICE_CONTROL_INTERROGATE: // 4
        pService->OnInterrogate();
        break;

    case SERVICE_CONTROL_SHUTDOWN: // 5
        pService->OnShutdown();
		pService->m_bIsRunning = 1;
        break;

	case SERVICE_PAUSED:
		pService->SetStatus(SERVICE_PAUSED);
		pService->OnPaused();
		pService->LogEvent(EVENTLOG_WARNING_TYPE, SERVICE_ACCEPT_SHUTDOWN);
		pService->m_bIsRunning = 2;
		break;

    default:
        if (dwOpcode >= SERVICE_CONTROL_USER) 
		{
            if (!pService->OnUserControl(dwOpcode)) 
			{
                pService->LogEvent(EVENTLOG_ERROR_TYPE, EVMSG_BADREQUEST);
            }
        } 
		else 
		{
            pService->LogEvent(EVENTLOG_ERROR_TYPE, EVMSG_BADREQUEST);
        }
        break;
    }

    ::SetServiceStatus(pService->m_hServiceStatus, &pService->m_Status);
}
        
// Called when the service is first initialized
BOOL CNTService::OnInit()
{
	return TRUE;
}

// Called when the service control manager wants to stop the service
void CNTService::OnStop()
{
}

// called when the service is interrogated
void CNTService::OnInterrogate()
{
}

// called when the service is paused
void CNTService::OnPause()
{
}

// called when the service is continued
void CNTService::OnContinue()
{
	//char err[32];
	//oMib.Start(err);
}

// called when the service is shut down
void CNTService::OnShutdown()
{
}

// called when the service gets a user control message
BOOL CNTService::OnUserControl(DWORD dwOpcode)
{
    return FALSE; // say not handled
}

// parado
void CNTService::OnPaused()
{
}

////////////////////////////////////////////////////////////////////////////////////////////
// Debugging support

/*void CNTService::DebugMsg(const char* pszFormat, ...)
{
    char buf[1024];
    sprintf(buf, "[%s](%lu): ", m_szServiceName, GetCurrentThreadId());
	va_list arglist;
	va_start(arglist, pszFormat);
    vsprintf(&buf[strlen(buf)], pszFormat, arglist);
	va_end(arglist);
    strcat(buf, "\n");
    OutputDebugString(buf);
}*/
