#if !defined(AFX_MIB_H__48BD7FDB_B3F3_4C9B_B722_0FD92920FC4E__INCLUDED_)
#define AFX_MIB_H__48BD7FDB_B3F3_4C9B_B722_0FD92920FC4E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// mib.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMib command target

class CMib : public CSocket
{
// Attributes
public:
	
// Operations
public:
	CMib();
	virtual ~CMib();

// Overrides
public:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMib)
	//}}AFX_VIRTUAL

	// Generated message map functions
	//{{AFX_MSG(CMib)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	short Start(char *);			// inicializa MIB
	int Stop();
	void RoboStatus( short );

// Implementation
protected:
	short RecebeTraps(char *);	// recupera Traps do Robo
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MIB_H__48BD7FDB_B3F3_4C9B_B722_0FD92920FC4E__INCLUDED_)
