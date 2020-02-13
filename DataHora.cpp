// DataHora.cpp: implementation of the DataHora class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DataHora.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DataHora::DataHora()
{
}

DataHora::~DataHora()
{
}

/*
'=====================================================================
'=== Formata a variavel global da classe DT
'
'Objetivo:	Exclui um Agendamento e sua(s) Data(s)(Dia, Mes e Horario) no Sistema
'Entrada.:	fmt (formato)
'
'Valor		formato da variavel DT
'1			yyyymmdd
'2			yyyy/mm/dd hh:mm:ss
'3			yyyymmddhhmmss
'n          dd/mm/yyyy
'          
'Saida...: nao possui
'=====================================================================
*/
void DataHora::GetDtHora(short fmt)
{
	SYSTEMTIME tm;
	int ano, mes, dia, hor, min, seg;

	GetLocalTime(&tm);

	ano = tm.wYear;		// ano yyyy
	mes = tm.wMonth;	// mes mm
	dia = tm.wDay;		// dia dd
	hor = tm.wHour;		// hora hh
	min = tm.wMinute;	// minutos mm
	seg = tm.wSecond;	// segundos ss

	switch(fmt)
	{
		case 1: // yyyymmdd - para nome do arquivo de log do dia
			sprintf(DT,"%04d%02d%02d",ano, mes, dia);
			break;
		case 2: // yyyy/mm/dd hh:mm:ss - para primeira coluna de detalhe do arquivo
			sprintf(DT,"%02d/%02d/%04d %02d:%02d:%02d",
					dia,mes,ano,hor,min,seg);
			break;
		case 3: // yyyymmddhhmmss - para primeira coluna de detalhe do arquivo
			sprintf(DT,"%04d%02d%02d%02d%02d%02d",
					ano,mes,dia,hor,min,seg);
			break;
		default: // dd/mm/yyyy - retorno default
			sprintf(DT,"%02d/%02d/%04", dia, mes, ano);
			break;
	}
}