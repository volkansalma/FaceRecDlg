// Thread.cpp: implementation of the Thread class.
//
//////////////////////////////////////////////////////////////////////


//--- Includes
#include "StdAfx.h"
#include <Windows.h>
#include "Thread.h"
#include <assert.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//======================== static Thread Main function  =================
int Thread::ThreadFuncMain(Thread* pThread)
{
//	Seperate code section so that we can do __try unwinding
//

	int nReturn = 1;
	pThread->m_bThreadStarted = true;
	pThread->m_bThreadFinished = false;

	nReturn = pThread->Go();			// Run the Thread ...
	pThread->m_bThreadFinished = true;
	pThread->m_bThreadStarted = false;
	return nReturn;
}

//======================== static Thread function  =================
DWORD WINAPI Thread::ThreadFunc( LPVOID n ) 
{
	DWORD nReturn = ERROR_INVALID_DATA ;	// Failed, null pointer

	if( n ) 
	{
		Thread* pThread = (Thread *)n;
		nReturn = ThreadFuncMain(pThread);
	}

	return nReturn;
}

//=================================================================


//---------------------------------------------------------------------------------------------------
Thread::Thread(int nThreadPriority)
: m_bExitRequest(false)
, m_bThreadStarted(false)
, m_bThreadFinished(false)
, m_hThread(0)
, m_ThreadID(0)
, m_nThreadPriority(nThreadPriority)
, m_hWakeUp(INVALID_HANDLE_VALUE)
{
	m_hWakeUp = CreateEvent( 0, FALSE, FALSE, 0) ;
}


//---------------------------------------------------------------------------------------------------
Thread::~Thread()
{
	if(m_hWakeUp != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hWakeUp) ;
		m_hWakeUp = INVALID_HANDLE_VALUE ;
	}
	if( m_hThread )
	{
		CloseHandle( m_hThread ) ;
		m_hThread = 0 ;
	}
}

//---------------------------------------------------------------------------------------------------
bool Thread::SetThreadPriority(int nThreadPriority)
{
	bool bRes = true;

	//--- if thread started, set it
	if (m_hThread != 0)
	{
		bRes = (::SetThreadPriority(m_hThread, nThreadPriority) != FALSE );
	}
	m_nThreadPriority = nThreadPriority;	//remember it -> will get set if NOT yet started
	return bRes;
}


//---------------------------------------------------------------------------------------------------
UINT Thread::Go()
{
	return 1;		// Not overloaded!
}

//---------------------------------------------------------------------------------------------------
bool Thread::Start()
{
	if (!m_bThreadStarted)
	{
		m_bExitRequest = false;
		m_bThreadFinished = false;
		
		if( m_hThread )                // close an old handle first
		{
    		CloseHandle( m_hThread ) ;  
	    	m_hThread = 0 ;
		}
		m_hThread = CreateThread(0,0, &ThreadFunc, this, 0, &m_ThreadID);
		if ( (m_hThread != 0) && (::SetThreadPriority(m_hThread, m_nThreadPriority)) )
		{
			m_bThreadStarted = true;
		}
	}
	return m_bThreadStarted;
}

//---------------------------------------------------------------------------------------------------
bool Thread::Stop(DWORD dwTimeOutSec /* =5 */, bool /*bPostMessages*/)
{
	if (!m_bThreadStarted)
	{
		return true;
	}

	// Set the 'please exit' request flag & wake them up
	m_bExitRequest = true ;
	if(m_hWakeUp != INVALID_HANDLE_VALUE)
	{
		SetEvent(m_hWakeUp);
	}
	
	// Wait until they exit of their own volition
	int nMax = 100*dwTimeOutSec ;
	while(nMax > 0)
	{
		if(!IsThreadRunning()) break ;

		//if (false) //bPostMessages) : NEVER PROCCESS MSG LOOP. ThreadMsgs (with no window) dissapear...
		//{
		//	MSG msg;
		//	while (PeekMessage(&msg, NULL,  0, 0, PM_REMOVE)) 
		//	{ 
		//		TranslateMessage(&msg);
		//		DispatchMessage(&msg);
		//	}
		//}

		--nMax ;
		Sleep(10);
	} ;

    // NB: Can't call 'CloseHandle( m_hThread )' here, because IsThreadRunning() my be called after this method
    
	bool ok = true ;
	if (dwTimeOutSec && (nMax<1))
	{
		OutputDebugString(_T("Thread::Stop() timed out: "));
//		OutputDebugString(ToString().c_str()) ;
		OutputDebugString(_T("\n")) ;
		ok = false ;
	}

	return ok ;
}

//---------------------------------------------------------------------------------------------------
bool Thread::IsThreadRunning() const
{
	if (!m_bThreadStarted) return false ;	// It's not yet running
	if (m_bThreadFinished) return false ;	// It's already finished

	DWORD dwCode = 0 ;
	GetExitCodeThread(m_hThread, &dwCode);

	return (dwCode == STILL_ACTIVE);
}


//---------------------------------------------------------------------------------------------------
bool Thread::WakableSleep(DWORD dwMiliSec)
{
	bool rv = true ;

	if(m_hWakeUp != INVALID_HANDLE_VALUE)
	{
		DWORD ret = WaitForSingleObject( m_hWakeUp, dwMiliSec ) ;
		if( ret != WAIT_TIMEOUT )
		{
			rv = false ;			// must have been woken up early
		}
	} else
	{
		Sleep( dwMiliSec ) ;
	}

	return rv ;
}

//---------------------------------------------------------------------------------------------------
bool Thread::WakeUp()
{
	bool bRes = true;
	if(m_hWakeUp != INVALID_HANDLE_VALUE)
	{
		bRes = (SetEvent(m_hWakeUp) != FALSE);
	}

	return bRes;
}


