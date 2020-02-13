// mib.cpp : implementation file
//

#include "stdafx.h"
#include "smsiserver.h"
#include "mib.h"
#include <snmp.h>
#include <mgmtapi.h>
#include <conio.h>
#include <WINBASE.H>
#include "InitFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define POLLINGINTERVAL   3000U
#define NUMBEROFPOLLS     10U

HANDLE hTrdMibRec = NULL;
DWORD dwThreadId = 0;
BOOL gfPollForTraps = FALSE;
UINT guPollingInterval = POLLINGINTERVAL;
UINT guNumberOfPolls   = NUMBEROFPOLLS;
BOOL fExec;
BOOL sTrap;
BOOL ReceiveTrapsEventDriven( void );
COleDateTime tempoAgora;
COleDateTime tempoTrap;
BOOL ReceiveTrapsPollDriven( UINT, UINT );
void DisplayTrapResponsePDU( DWORD, AsnObjectIdentifier *, AsnNetworkAddress *, AsnInteger *, AsnInteger *, AsnTimeticks *, RFC1157VarBindList * );
void PrintOid( AsnObjectIdentifier * );
short DisplayVarBinds(  RFC1157VarBindList  * );
void PrintErrorMessage( char *, DWORD );
BYTE gbOperation       = 0;
COleDateTimeSpan spanElapsed;
LPVOID	lpMsgBuf;
char	msg[255];
char *AnyToStr( AsnObjectSyntax * );
short mibCall = 0;

char roboOID[32];
char oOID[32];			// object ID
char oidNome[32];
char oidValue[32];
char oidPorta[32];
char oidVersao[32];
char oidCmnidade[32];
char oidEnterprz[32];
char oidGenTrap[32];
char oidAgent[32];
char oidMsg[32];
short oidResp;
char ipAnt[32];
short MibStatus;

/* Generic trap strings */
static char *szGenericTraps[] =
{ 
    "coldStart(0)",
    "warmStart(1)",
    "linkDown(2)",
    "linkUp(3)",
    "authenticationFailure(4)",
    "egpNeighborLoss(5)",
    "enterpriseSpecific(6)"
};

/////////////////////////////////////////////////////////////////////////////
// CMib

CMib::CMib()
{
}

CMib::~CMib()
{
}


// Do not edit the following lines, which are needed by ClassWizard.
#if 0
BEGIN_MESSAGE_MAP(CMib, CSocket)
	//{{AFX_MSG_MAP(CMib)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
#endif	// 0

/////////////////////////////////////////////////////////////////////////////
// CMib member functions
/*
** Convert an AsnAny value to a string.
**
** Returns: A string representation of the variable data.
*/
char * AnyToStr( 
  AsnObjectSyntax  *sAny )       /* IN  AsnObjectSyntax object containing value */
{
    DWORD dwValue = 0;
    UINT uLen = 0;
    BYTE *puData = 0;
	char *pString = NULL;		// nome

    switch ( sAny->asnType )
    {
        case ASN_INTEGER:    
            pString = (char *) SnmpUtilMemAlloc( 33 );
            if ( pString )
               ltoa( sAny->asnValue.number, pString, 10 );
            break;
        case ASN_RFC1155_COUNTER:
            dwValue = sAny->asnValue.counter;
            goto Uint;
        case ASN_RFC1155_GAUGE:
            dwValue = sAny->asnValue.gauge;
            goto Uint;
        case ASN_RFC1155_TIMETICKS:
            dwValue = sAny->asnValue.ticks;
Uint:
            pString = (char *) SnmpUtilMemAlloc( 33 );
            if ( pString )
               ultoa( dwValue, pString, 10 );
            break;
        case ASN_OCTETSTRING:   /* Same as ASN_RFC1213_DISPSTRING */
            uLen = sAny->asnValue.string.length;
            puData = sAny->asnValue.string.stream;
            goto String;
        case ASN_SEQUENCE:      /* Same as ASN_SEQUENCEOF */
            uLen = sAny->asnValue.sequence.length;
            puData = sAny->asnValue.sequence.stream;
            goto String;
        case ASN_RFC1155_IPADDRESS:
        {
            if ( sAny->asnValue.address.length )
            {
                UINT i;
                char szBuf[17];

                uLen = sAny->asnValue.address.length;
                puData = sAny->asnValue.address.stream;

                pString = (char *) SnmpUtilMemAlloc( uLen * 4 );
                if ( pString )
                {
                    pString[0] = '\0';
    
                    for ( i = 0; i < uLen; i++ )
                    {
                        lstrcat( pString, itoa( puData[i], szBuf, 10 ) );    
                        if ( i < uLen-1 )
                            lstrcat( pString, "." );
                    }
                }
            }
            else
                pString = NULL;
            break;
        }
        case ASN_RFC1155_OPAQUE:
            if ( sAny->asnValue.arbitrary.length )
            {
                uLen = sAny->asnValue.arbitrary.length;
                puData = sAny->asnValue.arbitrary.stream;
String:
                pString = (char *) SnmpUtilMemAlloc( uLen + 1 );
                if ( pString )
                {
                    if ( sAny->asnValue.arbitrary.length )
                        strncpy( pString, (char *)puData, uLen );
                }
            }
            else
                pString = NULL;
            break;
        case ASN_OBJECTIDENTIFIER:
        {
            if ( sAny->asnValue.object.idLength )
            {
                pString = (char *) SnmpUtilMemAlloc( sAny->asnValue.object.idLength * 5 );
                if ( pString )
                {
                    UINT i;
                    char szBuf[17];
                    for ( i = 0; i < sAny->asnValue.object.idLength; i++ )
                    {
                        lstrcat( pString, itoa( sAny->asnValue.object.ids[i], szBuf, 10 ) );    
                        if ( i < sAny->asnValue.object.idLength-1 )
                            lstrcat( pString, "." );
                    }
                }
            }
            else
                pString = NULL;
            break;
        }
        default:             /* Unrecognised data type */
            return( FALSE );
    }
    return( pString );
}



/*
** Display an error string and the GetLastError message.
*/
void PrintErrorMessage( 
  char  *pString,            /* IN Error string to display */
  DWORD  dwError )            /* IN Value returned by GetLastError */
{
   char szMsg[128];

   if ( pString )
   {
      wsprintf( szMsg, "%s: %u", pString, dwError );
      puts( szMsg );
   }
   
   if ( FormatMessage( 
      FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_MAX_WIDTH_MASK,
      NULL, dwError,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      (LPTSTR) &szMsg, sizeof( szMsg ) - 1, NULL ) != 0 )
   {
      puts( szMsg );
   }
}



/*
** Display the contents of a RFC1157VarBindList object.
*/
short DisplayVarBinds(RFC1157VarBindList *pVarBinds)
{
    UINT i, j;
    // DWORD dwLength;
    char *pNome = NULL;		// nome
	char *pValue = NULL;
	char oUID[5];

    if ( pVarBinds == NULL )
        return 1;

    for ( i = 0; i < pVarBinds->len; i++ )
    {
        /* Name */ 
        SnmpMgrOidToStr( &pVarBinds->list[i].name, &pNome );
		strcpy(oidNome, pNome);

        SnmpUtilMemFree( pNome );
        pNome = NULL;

		oOID[0] = '\0';
		oUID[0] = '\0';
        for ( j = 0; j < pVarBinds->list[i].name.idLength; j++ )
        {
			sprintf(oUID, "%d%c", pVarBinds->list[i].name.ids[j], j < pVarBinds->list[i].name.idLength - 1 ? '.' : ' ' );
			strcat(oOID,oUID);
        }

        pValue = AnyToStr( &pVarBinds->list[i].value );
		
		if(!strcmp(pValue,"0"))
			exit(-1);

		if(pValue == NULL)
			pValue = "Sem Mensagem";
		oidValue[0]='\0';
		strcpy(oidValue, pValue);

        SnmpUtilMemFree( pValue );
        pValue = NULL;
    }

	short tam = strlen(roboOID);
	if( !memcmp(oOID, roboOID, tam) )
		return 0;
	else
		return 1;
}



/*
** Display an AsnObjectIdentifier OID value as a string with a newline.
*/
void PrintOid( 
  AsnObjectIdentifier *pName )  /* IN  */
{
    if ( pName )
    {
        UINT i;
        char szBuf[8];
        char szOid[80];

        szOid[0] = '\0';

        for ( i = 0; i < pName->idLength; i++ )
        {
            wsprintf( szBuf, "%u.", pName->ids[i] );
            lstrcat( szOid, szBuf );
        }
        szOid[lstrlen(szOid)-1] = '\0';
    
        // puts( szOid );

		oidEnterprz[0]='\0';
		strcpy(oidEnterprz, szOid);
    }
}

/*
  *
  *
  *
*/
void SendTrap()
{
	HRESULT hresult_trap;
	CLSID clsid_trap;
	_clsTrapSNMP *t;
	
	CoInitialize(NULL);

	hresult_trap = CLSIDFromProgID(OLESTR("TrapSNMP.clsTrapSNMP"),&clsid_trap);

	if(FAILED(hresult_trap))
	{
		return;
	}
		
	hresult_trap = CoCreateInstance(clsid_trap,NULL,CLSCTX_INPROC_SERVER,__uuidof(_clsTrapSNMP),(LPVOID *) &t);

	if(FAILED(hresult_trap))
	{
		return;
	}

	t->PutTRAP_VERSION(oidVersao);
	t->PutTRAP_COMUNITY(oidCmnidade);
	t->PutTRAP_ENTERPRISE(oidEnterprz);
	t->PutTRAP_GENTRAP("6");
	t->PutTRAP_RECEIVER(oidAgent);
	t->PutTRAP_RECEIVER_PORT(oidPorta);

	char mensg[32]; mensg[0] = '\0';
	sprintf(mensg, "R:%s ", oidMsg);
	strcpy(&mensg[7], "| ");
	strcpy(&mensg[8], oidValue);

	if( (strlen(mensg)) > 30 )
		mensg[31]='\0';

	try
	{
		t->EnviarTrap(roboOID, mensg);
	}
	catch(_com_error e)
	{
		DWORD err=e.Error();
	}

	t->Release ();
	
	CoUninitialize();

	sTrap = TRUE;
}

/*
** Display the information in a TrapReponse PDU and its variable bindings.
*/
void DisplayTrapResponsePDU(
  DWORD                dwTrapCount,     /* IN Count of traps received */
  AsnObjectIdentifier *pEnterprise,     /* IN TrapResponse PDU enterprises OID */
  AsnIPAddress        *pIPAddress,      /* IN TrapResponse PDU IpAddress value */
  AsnInteger          *plGenericTrap,   /* IN TrapResponse PDU generic trap value */
  AsnInteger          *plSpecificTrap,  /* IN TrapResponse PDU speciifc trap value */
  AsnTimeticks        *pdwTimeStamp,    /* IN TrapResponse PDU time stamp */
  RFC1157VarBindList  *pVarBinds )      /* IN Variable bindings list */
{
	if( !DisplayVarBinds( pVarBinds ) )		// Responder trap
	{
		PrintOid( pEnterprise );
		sprintf(oidAgent, "%u.%u.%u.%u", pIPAddress->stream[0], pIPAddress->stream[1], pIPAddress->stream[2], pIPAddress->stream[3] );
		SendTrap();
	}
}



/*
** Receive traps using the event-driven mechanism of the SNMP Trap Service.
**
** Returns: TRUE if polling concluded without error, otherwise FALSE.
*/
BOOL ReceiveTrapsEventDriven( void )
{
    BOOL fResult = TRUE;

	fExec = TRUE;

    HANDLE hTrapEvent;

    static char *pListening = "\nSnmpTool: Listening for traps. Hit Escape to exit...";

	CInitFile oIni;
	roboOID[0] = '\0';
	oIni.GetMibOID(roboOID);
	if( !strlen(roboOID) )
	{
		SetLastError(1751);
		return false;
	}
	
	oidPorta[0] = '\0';
	oIni.GetMibPorta( oidPorta );
	if( !strlen(oidPorta) )
		strcpy(oidPorta, "162");
	
	oidVersao[0] = '\0';
	oIni.GetMibVersao( oidVersao );
	if( !strlen(oidVersao) )
		strcpy(oidVersao, "0");
	
	oidCmnidade[0] = '\0';
	oIni.GetMibComunidade( oidCmnidade );
	if( !strlen(oidCmnidade) )
		strcpy(oidCmnidade, "public");

	/* Start listening for trap events */
    if ( SnmpMgrTrapListen( &hTrapEvent ) == FALSE )
    {
        DWORD dwError = GetLastError();
        if ( dwError != SNMP_MGMTAPI_TRAP_DUPINIT )
        {
            goto ReceiveTrapsEventDrivenFailed;
            fResult = FALSE;
        }
    }
    else
    {
        DWORD dwTrapCount = 0;
        DWORD dwResult;
		
        while (fExec)
        {
			switch(MibStatus)
			{
			case 0: // Stop
				strcpy(oidMsg, "PARAD");		
				break;
			case 1: // Run
				strcpy(oidMsg, "ATIVO");
				break;
			case 2: // Parando
			case 3: // Continue
			case 4: // Respondendo
			case 5: // Shutdown
				strcpy(oidMsg, "PAUSA");
				break;
			default: // Desconhecido
				strcpy(oidMsg, "DESC.");
				break;
			}

            /* Receiving a trap will set an event signal */
            dwResult = WaitForSingleObject( hTrapEvent, 1000 );
			
            if ( dwResult != WAIT_FAILED )
            {
                
				if ( dwResult != WAIT_TIMEOUT )
                {    
                    AsnInteger          TrapGeneric;
                    AsnInteger          TrapSpecific;
                    AsnTimeticks        TrapTimeStamp;
                    AsnIPAddress        TrapIpAddress;
                    AsnObjectIdentifier TrapEnterprise;
                    RFC1157VarBindList  TrapVarBinds;
					
                    /* Initialize the variable bindings list */
                    TrapVarBinds.list = (RFC1157VarBind *) SnmpUtilMemAlloc( sizeof( RFC1157VarBind ) );

                    if ( TrapVarBinds.list == NULL )
                    {
                        // PrintErrorMessage("ERROR: SnmpUtilMemAlloc: ", GetLastError() );
                        goto ReceiveTrapsEventDrivenFailed;
                        fResult = FALSE;
						fExec = FALSE;
                    }
					
                    /* ******************************************************************* */

                    while ( SnmpMgrGetTrap( &TrapEnterprise, &TrapIpAddress, 
						    &TrapGeneric, &TrapSpecific, &TrapTimeStamp, &TrapVarBinds ) == TRUE )
                    {
					    
						dwTrapCount++;
						
						DisplayTrapResponsePDU( dwTrapCount, &TrapEnterprise, &TrapIpAddress, &TrapGeneric, 
								&TrapSpecific, &TrapTimeStamp, &TrapVarBinds );
						
                        if ( TrapIpAddress.dynamic == TRUE )
                            SnmpUtilMemFree( TrapIpAddress.stream );
						
                        SnmpUtilOidFree( &TrapEnterprise );
						
						Sleep(1000);

						//if( sTrap == TRUE )
						//	fExec = FALSE;
                    }

					/* ******************************************************************* */

                    SnmpUtilVarBindListFree( &TrapVarBinds );
				
                    if ( ( dwResult = GetLastError() ) != 0 )
                    {
                        if ( dwResult != SNMP_MGMTAPI_NOTRAPS )
                            fExec = FALSE;
                    }
					
                    if ( ResetEvent( hTrapEvent ) == 0 )
					{
						fExec = FALSE;
						CloseHandle(hTrapEvent);
					}
                }
				
				if(dwTrapCount == 500)
					dwTrapCount = 0;

            }
            else
            {
                goto ReceiveTrapsEventDrivenFailed;
                fResult = FALSE;
				fExec = FALSE;
            }
        } // while !!!
    }

CloseHandle(hTrapEvent);

ReceiveTrapsEventDrivenFailed:
	return( fResult );
}


DWORD WINAPI ThreadRecepcaoTrap( LPVOID lpParam ) 
{ 
	DWORD r = 0;
	msg[0] = '\0';

	while(1)
	{
		if ( ReceiveTrapsEventDriven() == FALSE )
		{
			r = -2;
			break;
		}
		sTrap = FALSE;
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
	
	strcpy(msg,(LPCTSTR)lpMsgBuf);
		
	LocalFree( lpMsgBuf );

	return r;
}


short CMib::RecebeTraps(char *err)
{
	hTrdMibRec = CreateThread( 
			NULL,
			0,
			ThreadRecepcaoTrap,
			0,
			0,
			&dwThreadId);

	if(hTrdMibRec = NULL)
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

	Sleep(5000);

	strcpy(err,msg);

	// tempoTrap = COleDateTime::GetCurrentTime();

	return 0;
}


short CMib::Start(char *err)
{
	oidResp = 0;
	return(RecebeTraps(err));
}


int CMib::Stop()
{
	unsigned int ExitCode = 1;	// False

	CloseHandle(hTrdMibRec);

	return ExitCode; // Contem o status de finalizacao da Thread
}

void CMib::RoboStatus(short st)
{
	MibStatus = st;
}