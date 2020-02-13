/*
'=======================================================================================
'= Arquivo: SMSIServer.cpp
'= Função:  arquivo principal do programa - disponibiliza a funcao
'=			_tmain() - MFC
'= Autor:   Getson Miranda
'= Data:    20/10/2003
'=======================================================================================
*/
// SMSIServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "SMSIServer.h"
#include "Administrativa.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// The one and only application object

// Aplicacao com acesso a MFC
CWinApp theApp;

// Suporte ao standard ::in ::out objetos do c++
using namespace std;

/*
'=====================================================================
'=== inicializador dos servicos da aplicacao
'
'Objetivo:	trabalhar com os parametros para execucao como servico do windows
'			
'Entrada.:	-i instala o programa como servico do windows
'			-u remove o programa com servico do windows, nao apaga
'			   o arquivo executavel
'			-v devolve a versao do servico em execucao, quando esse
'			   ja estiver instalado
'          
'Saida...:	no prompt de comando pode exibir
'			informa se o servico esta instalado ou nao
'			informa a versao do serviso em execuacao, caso ja steja instalado
'			informa se o servico foi desinstalado com sucesso
'			Analisar o arquivo de LOG
'=====================================================================
*/
int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		MessageBox(0, "*** Fatal Error: MFC initialization failed! *** For a console application, which does not use the MFC-supplied WinMain function, you must call AfxWinInit directly to initialize MFC. O programa de monitoramento nao pode ser executado, nesta maquina!", "ROBO - MFC", MB_ICONERROR+MB_OK);
		return 1;
	}

	/*
		A classe CAdministrativa, dispara a execucao do
		Robo como um servico do Windows. Ficando em estado
		de Loop no metodo Run() da classe.
	*/
	CAdministrativa oAdm; // Cria um objeto da classe CAdministrativa

	if( !oAdm.ParseStandardArgs(argc, argv) ) // Analisa os parametros de linha de comando
	{
		oAdm.StartService(); // funcao da classe registrada no SCM para execucao do servico Robo

		return oAdm.m_Status.dwWin32ExitCode; // 0 sucesso
	}

	return 0;
}
