// DataHora.h: interface for the DataHora class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DATAHORA_H__193CC06A_3F9B_47BD_AB32_80AA65E38CE5__INCLUDED_)
#define AFX_DATAHORA_H__193CC06A_3F9B_47BD_AB32_80AA65E38CE5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class DataHora  
{
public:
	DataHora();
	virtual ~DataHora();
	char DT[19]; // data ou hora atual do sistema operacional
	void GetDtHora(short fmt);
};

#endif // !defined(AFX_DATAHORA_H__193CC06A_3F9B_47BD_AB32_80AA65E38CE5__INCLUDED_)
