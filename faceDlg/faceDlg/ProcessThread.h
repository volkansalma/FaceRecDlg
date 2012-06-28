// CameraPlayerThread.h: interface for the CCameraPlayerThread class.
//
//////////////////////////////////////////////////////////////////////

#ifndef processthread_H
#define processthread_H


#define IMAGE_HANDLED_MESSAGE  (WM_APP + 1)

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Thread.h"
#include <queue>
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>

using namespace std;

#define FRAMES_TO_PROCESS_IN_A_SEC (20)

class ProcessThread: public Thread  
{

private:
	void findFaceAndDrawRect(IplImage* ,bool &willRecord);
	IplImage* getImageFromCam();
	

	CvCapture* capture;

	IplImage* faceImg;
	

	//face detect
	CvMemStorage* storage;
	CvHaarClassifierCascade* cascade; 
	CvHaarClassifierCascade* nested_cascade; 
	int use_nested_cascade; 

	IplImage *frame;
	IplImage *frame_copy;;
	IplImage *image;
	double scale;
	IplImage *gray, *small_img;
	IplImage* dlgfaceImage;
	//

	void* m_dlgPtr;


public:
	
	void End();
	void ProcessJob();
	void Initialize(HWND);
	
	HWND dialogHWND;
	DWORD GetLastUpdateTick();
	ProcessThread(int nThreadPriority = /*THREAD_PRIORITY_NORMAL*/THREAD_PRIORITY_ABOVE_NORMAL,void* dlgPtr = 0, IplImage *faceIm = 0);
    ~ProcessThread();

	bool willRecord;
	bool wasSuccesfullyRecorded;
	CRITICAL_SECTION	m_CriticalSectionForFaceImg;
	bool isValidImage(IplImage*);



protected:

	virtual UINT Go();
	volatile bool		m_bReadyForNewImage;
	volatile int		m_nCounter;
	CRITICAL_SECTION	m_CriticalSection;
	
	DWORD				m_dvLastUpdate;
	bool				m_running;
};



#endif // 
