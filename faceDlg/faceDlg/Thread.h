#if !defined(THREAD_H_INCLUDED_)
#define THREAD_H_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


//==================================================================================
class Thread
{
public:
	/** Get the status of the Exit Request flag.
	 * @return True=Exit has been requested, False=keep running. */
	bool GetExitRequest() const {return m_bExitRequest;}

	/** WakeUp this thread, get it out of WakableSleep()
	 * @return True=Ok, False=Error */
	bool WakeUp();

	/** Set the Thread Priority.
	 * @param int Priority: {THREAD_PRIORITY_IDLE, THREAD_PRIORITY_NORMAL etc. 
	 * @see <a href=ms-help://MS.VSCC/MS.MSDNVS/dllproc/prothred_9f5l.htm> MSDN </a> }
	 * @return True=Priority changed, False=Error changing priority.*/
	bool SetThreadPriority(int nThreadPriority);

	/** Is the thread (still) running 
	 * @return True=Still Running, False=Not yet Started or has now Stopped.*/
	bool IsThreadRunning() const;

	/** Ask the thread to exit.
	 * @param DWORD dwTimeOutSec. Stop timeout (0=set flag and return without waiting).
	 * @param bool bPostMessages. process message loop while waiting.
	 * @return True=Stopped, False=Not yet stopped as we timed out while waiting.
	 * @note overload this to stop any subordinate objects. It should not be neccassary for 
	 * Overload Stop implementations to call Sleep or WakeableSleep (WakeableSleep will reset the watchdog).
	 * Example implemention:<pre>
		bool Stop(DWORD dwTimeOutSec=5, bool bPostMessages=true)
		{
			Thread::Stop( 0, false ) ; //ask nicely (set flag)

			// Do specific processing...

			return Thread::Stop( dwTimeOutSec, bPostMessages ); //built in wait
		} 	
	 </pre>
	 */
	virtual bool Stop(DWORD dwTimeOutSec=5, bool bPostMessages=true);

	/** Start the thread.
	 * @return True=Running, False=Failed to start 
	 * @note probably don't overload this, try to start subordinate ojects in your own Go() */
	virtual bool Start();

	/** Constructor.
	 *	@param string_type const &strClassName. Thread class identificaion or name (10..20 meaningfull chars please).
	 *	@param int nThreadPriority. Thread priority to set to.
	 *  @note This name is seen by the User when the Watchdog detects a failure in the thread.*/
	Thread(int nThreadPriority = THREAD_PRIORITY_NORMAL);

	/** Default destructor.*/
	virtual ~Thread();

	/** Set the name of a Thread for the debugger.
	 *	@param nThreadID. Windows Thread id or -1=This thread.
	 *	@param string_type Thread name (only 9 significant narrow chars)*/
	static inline void SetThreadName( DWORD dwThread, LPCSTR szThreadName)
	{
		UNREFERENCED_PARAMETER(szThreadName);
		UNREFERENCED_PARAMETER(dwThread);
	}

protected:
	/** Thread control function. Override this to define thread behaviour. 
	 *	Should be of the form: <pre>
		UINT Go()
		{
			while (!GetExitRequest())
			{
				WakableSleep(nSeconds);	// This kicks the watchdog for you
				Kick( true ) ;			// YOU MUST CALL THIS ONCE EVERY 30 seconds if you don't call WakableSleep()
				Kick( false ) ;			// turn off the watchdog during long periods of inactivity
			}//while
			return 0;
		}
	 </pre>
	 *	@note Do not call this method directly, it is called from the base class.*/
	virtual UINT Go();

//--- Protected functions
	/** Sleep for at-least the specified time or until WakeUp() is called
	 * @param DWORD MiliSeconds, can be INFINATE.
	 * @return True=Slept for the specified period, False=Woken up early or Error
	 * @note This also kicks the watchdog for you, turning it off if you wait >30seconds.
	 * @note Do not call this method from an overloaded Stop() - it will kick the watchdog.
	 * @see WakeUp() */
	bool	WakableSleep(DWORD dwMiliSec);

	HANDLE			m_hWakeUp;					///< Blocking handle used by WakableSleep() & WakeUp()
private:
//--- Protected vars
	HANDLE			m_hThread;					///< Windows Thread Handle
	DWORD			m_ThreadID;					///< Windows Thread ID
	int				m_nThreadPriority;			///< Thread priority

private:
	static DWORD WINAPI ThreadFunc( LPVOID n ) ;
	static int ThreadFuncMain(Thread* pThread);
	volatile bool	m_bThreadFinished;			///< Thread done flag, True when the thread has exited
	volatile bool	m_bThreadStarted;			///< Thread running flag, True once the thread has stated
	volatile bool	m_bExitRequest;				///< Exit flag, this is set by Stop() to ask the thread to exit
};

#endif // !defined(THREAD_H_INCLUDED_)
