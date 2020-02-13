// Servidores.cpp: implementation of the CServidores class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Servidores.h"
#include <windows.h>
#include <winbase.h>
#include <winsock.h>
#include <stdio.h>
#include <string.h>

typedef struct tagIPINFO
{
	u_char Ttl;				// tempo de vida
	u_char Tos;				// tipo de servico
	u_char IPFlags;			// flags para endereco IP
	u_char OptSize;			// tamanho de dados
	u_char FAR *Options;	// opcoes de dados no buffer
}IPINFO, *PIPINFO;

typedef struct tagICMPECHO
{
	u_long Source;			// endereco de origem
	u_long Status;			// status do endereco IP
	u_long RTTime;			// roda em milisegundos
	u_short DataSize;		// replica
	u_short Reserved;		// IP desconhecido
	void FAR *pData;		// para o buffer
	IPINFO	ipInfo;			// opcoes
}ICMPECHO, *PICMPECHO;

// ICMP.DLL onde estao a funcoes
HANDLE (WINAPI *pIcmpCreateFile) (VOID);

BOOL (WINAPI *pIcmpCloseHandle) (HANDLE);

DWORD (WINAPI *pIcmpSendEcho) (HANDLE,DWORD,LPVOID,WORD,PIPINFO,LPVOID,DWORD,DWORD);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CServidores::CServidores()
{

}

CServidores::~CServidores()
{

}

/*
'=====================================================================
'===		ChkServidor(LPCTSTR ip)
'
'Objetivo:	prepara o endereco ou ip de um servidor para ser testado
'
'Entrada.:	ip = emdereco IP do servidor
'
'Saida...:	nenhuma
'
'Retornos:	0    = sucesso
'			1    = ip ou nome nao recebido como parametro
'			<> 0 = servidor nao encontrado
'=====================================================================
*/
short CServidores::ChkServidor(LPCTSTR ip, char *err, u_short port)
{
	ASSERT( ip );
	m_Serv = ip;

	if( m_Serv.IsEmpty() ) return 1;

	return(TesteServidor(err, port));
}

// ****************************************************************************************** //

short CServidores::TesteServidor(char *err, u_short port)
{
	LPVOID	lpMsgBuf;
	struct	sockaddr_in Address;
	SOCKET	Socket = INVALID_SOCKET;
	WSADATA	WsaData;
	int		Error;
	struct hostent *   HostEntry;

	// WinSock WS2_32.DLL 
	Error = WSAStartup (0x101, &WsaData);
	if (Error == SOCKET_ERROR) 
    {
		FormatMessage
			( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &lpMsgBuf,
			0,
			NULL 
			);
	
		strcpy(err,(LPCTSTR)lpMsgBuf);
		
		LocalFree( lpMsgBuf );
		
		return 1;
	}


	if ( Socket == INVALID_SOCKET )
    {
        Socket = socket(AF_INET, SOCK_STREAM, 0);

        if (Socket == INVALID_SOCKET) 
        {
            FormatMessage
				( 
				FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				FORMAT_MESSAGE_FROM_SYSTEM | 
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				GetLastError(),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR) &lpMsgBuf,
				0,
				NULL 
				);
			
			strcpy(err,(LPCTSTR)lpMsgBuf);
			
			LocalFree( lpMsgBuf );

			WSACleanup();
			
			return 2;
        }
	}


	// Connect

    Address.sin_family = AF_INET;
    Address.sin_port = 0;
    Address.sin_addr.s_addr = INADDR_ANY;

    Error = bind(Socket, (struct sockaddr *) &Address, sizeof(Address));
    if (Error) 
	{
		FormatMessage
			( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &lpMsgBuf,
			0,
			NULL 
			);
		
		strcpy(err,(LPCTSTR)lpMsgBuf);
		
		LocalFree( lpMsgBuf );

		WSACleanup();
		
		return 3;
    }

    Address.sin_family = AF_INET;
    Address.sin_port = htons(port);
    Address.sin_addr.s_addr = inet_addr(m_Serv);

    if (Address.sin_addr.s_addr == -1) 
    {
            HostEntry = gethostbyname(m_Serv);
            if (HostEntry == NULL) 
            {
                FormatMessage
					( 
					FORMAT_MESSAGE_ALLOCATE_BUFFER | 
					FORMAT_MESSAGE_FROM_SYSTEM | 
					FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL,
					GetLastError(),
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					(LPTSTR) &lpMsgBuf,
					0,
					NULL 
					);
				
				strcpy(err,(LPCTSTR)lpMsgBuf);
				
				LocalFree( lpMsgBuf );

				WSACleanup();
				
				return 4;
            }
			else 
            {
                Address.sin_addr.s_addr = *((unsigned long *) HostEntry->h_addr);
            }
    }

    Error = connect(Socket, (struct sockaddr *) &Address, sizeof(Address));
    if (Error) 
    {
		FormatMessage
			( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &lpMsgBuf,
			0,
			NULL 
			);
		
		strcpy(err,(LPCTSTR)lpMsgBuf);
		
		LocalFree( lpMsgBuf );

		WSACleanup();
		
		return 5;
    }

	FormatMessage
			( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &lpMsgBuf,
			0,
			NULL 
			);

	strcpy(err,(LPCTSTR)lpMsgBuf);

	LocalFree( lpMsgBuf );

	WSACleanup();

	return 0;
}


/*
'=====================================================================
'===		CServidores::PingServidor()
'
'Objetivo:	efetua uma operacao de ping em um endereco ip recebido
'
'Entrada.:	nenhuma
'
'Saida...:	nenhuma
'
'Retornos:	2 = erro no acesso a ICMP.DLL para recuperar funcoes SDK 32
'			3 = erro no acesso a funcoes da dll ICMP.DLL
'			4 = problema nas conexoes de rede
'			5 = prolbema na versao das dll da WinSock
'			6 = erro da winsock na busca de servidores
'			0 = sucesso
'			9 = todos os servicos foram completados, porem nao
'				foi possivel localizar o servidor
'=====================================================================
*/
short CServidores::PingServidor(char *err)
{
	WSADATA wsaData;			// estrutura WinSocket WSADATA
	ICMPECHO icmpEcho;			// ICMP Echo reply buffer
	HANDLE hndlIcmp;			// handle para ICMP.DLL
	HANDLE hndlFile;		// handle para IcmpCreateFile()
    LPHOSTENT pHost;			// ponteiro para o host
    struct in_addr iaDest;		// estrutura para endereco internet
	DWORD *dwAddress;			// endereco IP
	IPINFO ipInfo;				// opcoes para endereco IP
	int nRet;					// codigo de retorno
	DWORD dwRet;				// return code
	int x;						// loop
	short sRet = 0;				// retorno do servico ICMP
	LPVOID lpMsgBuf;

	// abre a ICMP.DLL para recuperar funcoes SDK 32
	hndlIcmp = LoadLibrary("ICMP.DLL");
	if (hndlIcmp == NULL)
	{
		FormatMessage
			( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,
			0,
			NULL 
			);

		strcpy(err,(LPCTSTR)lpMsgBuf);

		LocalFree( lpMsgBuf );

		return 2;
	}

	// obtem um ponteiro para as funcoes da dll
	pIcmpCreateFile = (HANDLE (WINAPI *)(void)) GetProcAddress((HINSTANCE)hndlIcmp,"IcmpCreateFile");

	pIcmpCloseHandle = (BOOL (WINAPI *)(HANDLE)) GetProcAddress((HINSTANCE)hndlIcmp,"IcmpCloseHandle");

	pIcmpSendEcho = (DWORD (WINAPI *) (HANDLE,DWORD,LPVOID,WORD,PIPINFO,LPVOID,DWORD,DWORD)) GetProcAddress((HINSTANCE)hndlIcmp,"IcmpSendEcho");

	// checagem para saber se conseguiu ler todas funcoes
	if (pIcmpCreateFile == NULL		|| 
		pIcmpCloseHandle == NULL	||
		pIcmpSendEcho == NULL)
	{
		FreeLibrary((HINSTANCE)hndlIcmp);
		return 3;
	}

	// inicia servicos de busca de servidor
	nRet = WSAStartup(0x0101, &wsaData );
    if (nRet)
    {
        WSACleanup();
		FreeLibrary((HINSTANCE)hndlIcmp);        
		return 4;
    }

    // verifica a versao da winSock
    if (0x0101 != wsaData.wVersion)
    {
        WSACleanup();
		FreeLibrary((HINSTANCE)hndlIcmp);
		return 5;
    }

	// Lookup destination
    // inet_addr() determina se e para buscar com endereco IP ou pelo nome
    iaDest.s_addr = inet_addr(m_Serv);

    if (iaDest.s_addr == INADDR_NONE)
        pHost = gethostbyname(m_Serv);
    else
        pHost = gethostbyaddr((const char *)&iaDest, 
                        sizeof(struct in_addr), AF_INET);

	if (pHost == NULL)
	{
        WSACleanup();
		FreeLibrary((HINSTANCE)hndlIcmp);
		return 6;
	}

	// copia o endereco IP
	dwAddress = (DWORD *)(*pHost->h_addr_list);

	// *****************************************************************************************
		
	// forca ICMP ecoar 2 vezes para o host
	hndlFile = pIcmpCreateFile();
	for (x = 0; x < 2; x++)
	{
		// monta estrutura com os valores default
		ipInfo.Ttl = 255;
		ipInfo.Tos = 0;
		ipInfo.IPFlags = 0;
		ipInfo.OptSize = 0;
		ipInfo.Options = NULL;

		//icmpEcho.ipInfo.Ttl = 256;
		// recupera
		dwRet = pIcmpSendEcho(
			hndlFile,		// Handle from IcmpCreateFile()
			*dwAddress,		// Destination IP address
			NULL,			// Pointer to buffer to send
			0,				// Size of buffer in bytes
			&ipInfo,		// Request options
			&icmpEcho,		// Reply buffer
			sizeof(struct tagICMPECHO),
			1500);			// Time to wait in milliseconds
		// Print the results
		iaDest.s_addr = icmpEcho.Source;
		/*printf("\nReply from %s  Time=%ldms  TTL=%d",
				inet_ntoa(iaDest),
				icmpEcho.RTTime,
				icmpEcho.ipInfo.Ttl);*/

		if (icmpEcho.Status)
		{
			// Todos os servicos ICMP foram executados com sucesso, nao encontrou o servidor
			sRet = 9 ;
			break;
		}
	}

	// *****************************************************************************************

	// fecha tudo
	pIcmpCloseHandle(hndlFile);
	FreeLibrary((HINSTANCE)hndlIcmp);
	WSACleanup();

	return sRet;
}


