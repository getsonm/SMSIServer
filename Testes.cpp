// Testes.cpp : implementation file
//
/*
'=======================================================================================
'= Arquivo: Testes.cpp
'= Função:  Contem as rotinas de comparacao de paginas e teste
'= Autor:   Getson Miranda
'= Data:    28/10/2003
'=======================================================================================
*/
#include "stdafx.h"
#include "smsiserver.h"
#include "Testes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTestes

IMPLEMENT_DYNCREATE(CTestes, CHtmlView)

CTestes::CTestes()
{
	EnableAutomation();
	//{{AFX_DATA_INIT(CTestes)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CTestes::~CTestes()
{
}

void CTestes::OnFinalRelease()
{
	// When the last reference for an automation object is released
	// OnFinalRelease is called.  The base class will automatically
	// deletes the object.  Add additional cleanup required for your
	// object before calling the base class.

	CHtmlView::OnFinalRelease();
}

void CTestes::DoDataExchange(CDataExchange* pDX)
{
	CHtmlView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTestes)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTestes, CHtmlView)
	//{{AFX_MSG_MAP(CTestes)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CTestes, CHtmlView)
	//{{AFX_DISPATCH_MAP(CTestes)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Note: we add support for IID_ITestes to support typesafe binding
//  from VBA.  This IID must match the GUID that is attached to the 
//  dispinterface in the .ODL file.

// {6D2D2EE6-20F8-40EF-897E-5FD5AD083FB3}
static const IID IID_ITestes =
{ 0x6d2d2ee6, 0x20f8, 0x40ef, { 0x89, 0x7e, 0x5f, 0xd5, 0xad, 0x8, 0x3f, 0xb3 } };

BEGIN_INTERFACE_MAP(CTestes, CHtmlView)
	INTERFACE_PART(CTestes, IID_ITestes, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTestes diagnostics

#ifdef _DEBUG
void CTestes::AssertValid() const
{
	CHtmlView::AssertValid();
}

void CTestes::Dump(CDumpContext& dc) const
{
	CHtmlView::Dump(dc);
}
#endif //_DEBUG

/*
'=====================================================================
'=== CmpPg(char *ds_url, char *ds_memo)
'
'Objetivo:	
'
'Entrada.:	char *ds_url = URL a ser aberta incluindo a pagina para comparacao
'			char *ds_memo = pagina recuperada do banco de dados para teste
'
'Retornos:	 0 = sucesso
'			#0 = erro interno nao fez nenhuma comparacao ou,
'				 as paginas sao diferentes.
'
'=====================================================================
*/
short CTestes::CmpPg(char *ds_url, char *ds_memo, long tam, char *err)
{
	CInternetSession *oIS;
	CStdioFile *oArq;
	char *str;

	str = (char*)malloc( 50000 );
	oArq = new CStdioFile();
	oIS = new CInternetSession();

	if(!oIS || !oArq || (str == NULL))
		return GetLastError();

	oIS->AssertValid();

	try
	{
		oArq = oIS->OpenURL(ds_url);
	}
	catch(CException *e)
	{
		/*HRESULT hr = e.Error();
		char* szErr = ""; 
		itoa(hr, szErr, 16); 
		CString tempString = "CTestes::CmpPg-01 "; 
		tempString += szErr;
		strcpy(err, tempString/*(LPCTSTR)lpMsgBuf*//*);*/
		TCHAR   szCause[255];
		CString strFormatted;
		e->GetErrorMessage(szCause, 255);
		strFormatted = "CTestes::CmpPg-01 ";
		strFormatted += szCause;
		strcpy(err, strFormatted/*(LPCTSTR)lpMsgBuf*/);
		return 0; // Interrompe a rotina
	}


	try
	{
		oArq->Read(str,50000);
	}
	catch(CFileException *e)
	{
		TCHAR   szCause[255];
		CString strFormatted;
		e->GetErrorMessage(szCause, 255);
		strFormatted = "CTestes::CmpPg-02 ";
		strFormatted += szCause;
		strcpy(err, strFormatted/*(LPCTSTR)lpMsgBuf*/);
		return 0; // Interrompe a rotina
	}

	long x = memcmp(ds_memo,str,tam);

	// *************************************************
	LPVOID	lpMsgBuf;
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
	// *************************************************

	free(str);
	oArq->Close();
	delete oArq;
	oIS->Close();
	delete oIS;

	return x;
}

/*
'=====================================================================
'=== CmpPg(char *ds_url, char *ds_memo)
'
'Objetivo:	
'
'Entrada.:	char *ds_url = URL a ser aberta incluindo a pagina para comparacao
'			char *ds_memo = pagina recuperada do banco de dados para teste
'
'Retornos:	 0  = sucesso
'			#0  = nao realizou o teste, ou encontrou erro
'			      na pagina.
'
'=====================================================================
*/
short CTestes::TstPg(char *url, char *ret, char *err)
{
	CInternetSession *oIS;	// MFC para conexao HTTP
	CStdioFile *oArq;		// Retorno da pagina Html, PHP, ASP ...
	long x = 0;					// Retorno do metodo
	TCHAR ch[40];			// Ocorrencia no teste do servico

	oIS = new CInternetSession(); // Conexao com a NET
	oArq = new CStdioFile(); // Objeto que recebera o retorno da pagina testada
	
	if(!oIS || !oArq)
		return 2;

	oIS->AssertValid(); // Analisa o status de conexao com a NET

	try
	{
		oArq = oIS->OpenURL(url); // conecta-se a pagina
	}
	catch(/*_com_error e*/ CException *e)
	{
		TCHAR   szCause[255];
		CString strFormatted;
		e->GetErrorMessage(szCause, 255);
		strFormatted = "CTestes::TstPg-01 ";
		strFormatted += szCause;
		strcpy(err, strFormatted/*(LPCTSTR)lpMsgBuf*/);
		return 0; // Interrompe a rotina
		//HRESULT hr = e.Error();
		//char* szErr = ""; 
		//itoa(hr, szErr, 16); 
		//CString tempString = "CTestes::TstPg-01 "; 
		//tempString += szErr;
		//strcpy(err, tempString/*(LPCTSTR)lpMsgBuf*/);
		//return 0; // Interrompe a rotina
	}

	ch[0]='\0';

	try
	{
		oArq->Read(ch,40); // recebe o retorno da pagina de servico
	}
	catch(CFileException *e)
	{
		TCHAR   szCause[255];
		CString strFormatted;
		e->GetErrorMessage(szCause, 255);
		strFormatted = "CTestes::TstPg-02 ";
		strFormatted += szCause;
		strcpy(err, strFormatted/*(LPCTSTR)lpMsgBuf*/);
		return 0; // Interrompe a rotina
	}

	char c;
	for(int k=0;k<40;k++) // elimina caracteres invalidos do retorno
	{
		c = ch[k]; 
		if( ch[k] <= 31 )
		{
			ch[k]='\0';
			break;
		}
	}

	strcpy(ret,ch); // retorno para log e trap

	// *************************************************
	LPVOID	lpMsgBuf;
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
	// *************************************************

	if(ret[0] == 'N')	// status do teste realizado
		x = 1;			// S = sucesso
						// N = fracasso
	oArq->Close();
	delete oArq;

	oIS->Close();
	delete oIS;

	return x;
}


short CTestes::InvalidCotents(char *str)
{
	char *pdest;
	char err404[] = "HTTP 404";

	pdest = strstr( str, err404 );
	if( pdest != NULL )
	{
		SetLastError(2);
		return 1;
	}

	
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CTestes message handlers
