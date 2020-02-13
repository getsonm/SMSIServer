/*
'=======================================================================================
'= Arquivo: Testes.h
'= Função:  Arquivo de cabecalho para a classe CTestes
'= Autor:   Getson Miranda
'= Data:    28/10/2003
'=======================================================================================
*/
#if !defined(AFX_TESTES_H__06C5F2CF_9102_427A_8EA9_8809FA8DF997__INCLUDED_)
#define AFX_TESTES_H__06C5F2CF_9102_427A_8EA9_8809FA8DF997__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Testes.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTestes html view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif
#include <afxhtml.h>

class CTestes : public CHtmlView
{
protected:
	//CTestes();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CTestes)

// html Data
public:
	//{{AFX_DATA(CTestes)
		// NOTE: the ClassWizard will add data members here
	CTestes();           // protected constructor used by dynamic creation
	virtual ~CTestes();
	short CmpPg(char *, char *, long, char *);
	short TstPg(char *, char *, char *);
	short InvalidCotents(char *);
	//}}AFX_DATA

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTestes)
	public:
	virtual void OnFinalRelease();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//virtual ~CTestes();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CTestes)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// Generated OLE dispatch map functions
	//{{AFX_DISPATCH(CTestes)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTES_H__06C5F2CF_9102_427A_8EA9_8809FA8DF997__INCLUDED_)
