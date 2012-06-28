// CameraPlayerThread.cpp: implementation of the CCameraPlayerThread class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ProcessThread.h"

using namespace std;



inline DWORD GETTICKCOUNTDIFFERENCE(DWORD first, DWORD second)
{
	return (((first) <= (second))?((second)-(first)):(UINT_MAX-(first)+(second)));
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ProcessThread::ProcessThread(int nThreadPriority,void *dlgPtr,IplImage *faceIm)
: Thread(nThreadPriority)
, m_bReadyForNewImage(false)
, m_nCounter(0)
, m_dvLastUpdate(0)
{
	InitializeCriticalSection(&m_CriticalSection);
	InitializeCriticalSection(&m_CriticalSectionForFaceImg);

	m_dlgPtr = dlgPtr;
	

	storage = 0;
	cascade = 0;
	nested_cascade = 0;
	use_nested_cascade = 0;

	frame = 0;
	frame_copy = 0;
	image = 0;

	cascade = (CvHaarClassifierCascade*)cvLoad("haarcascade_frontalface_default.xml", 0, 0, 0 );
	storage = cvCreateMemStorage(0);
	scale = 2;
	capture = cvCaptureFromCAM(0);


	frame_copy = cvCreateImage( cvSize(640,480),
		IPL_DEPTH_8U, 3 );	

	gray = cvCreateImage( cvSize(640,480), 8, 1 );
	
	small_img = cvCreateImage( cvSize( cvRound (640/scale),
		cvRound (480/scale)), 8, 1 );


	willRecord = false;
	wasSuccesfullyRecorded = false;

	dlgfaceImage = faceIm;
}

ProcessThread::~ProcessThread()
{
	cvReleaseCapture(&capture);
	DeleteCriticalSection(&m_CriticalSection);	
	DeleteCriticalSection(&m_CriticalSectionForFaceImg);
		
}

UINT ProcessThread::Go()
{

	
	DWORD	dwTickPrevious = GetTickCount();
	DWORD	dwDiff = 0;
	DWORD	dwCurrent = 0;
	DWORD	dwTimeToWait = (DWORD)(1000.0/FRAMES_TO_PROCESS_IN_A_SEC);

	while (!GetExitRequest())
	{
		dwCurrent = GetTickCount();
		dwDiff = GETTICKCOUNTDIFFERENCE(dwTickPrevious, dwCurrent);
		
		if(dwDiff < dwTimeToWait)
				{
					WakableSleep(dwTimeToWait - dwDiff);
					continue;
				}
				else dwTickPrevious = dwCurrent;
	
			ProcessJob();
	}
	return 0;
}

void ProcessThread::ProcessJob()
{
	IplImage* img = getImageFromCam();
	if(!isValidImage(img)) return;


	//Joblara bak. Yeni Job yoksa bul iþaretle yalnýzca.
	//enroll job u varsa. görüntüyü kaydet Dialoga msg push et.

	//HerZamanBulÝþaretle.
	findFaceAndDrawRect(img,willRecord);


}
void ProcessThread::findFaceAndDrawRect(IplImage* frame,bool &willRecord)
{
	 
	cvCopy( frame, frame_copy, 0 );

	 //detect and draw
	 IplImage* img = frame_copy;

	 int i = 0;
	 int j = 0;

   	 cvCvtColor( img, gray, CV_BGR2GRAY );
	 cvResize( gray, small_img, CV_INTER_LINEAR );
	 cvEqualizeHist( small_img, small_img );
	 cvClearMemStorage(storage);

	 
	 if( cascade )
	 {
		 CvSeq* faces = 0;
		 
		 faces = cvHaarDetectObjects( small_img, cascade, storage,
			 1.2,2, 0
			 |CV_HAAR_FIND_BIGGEST_OBJECT
			 //|CV_HAAR_DO_ROUGH_SEARCH
			 |CV_HAAR_DO_CANNY_PRUNING
			 //|CV_HAAR_SCALE_IMAGE
			 ,
			 cvSize(50,50));
			 
		
		 
		 if(faces)
		 {
			 CvRect* r = 0;
			 r = (CvRect*)cvGetSeqElem( faces, 0);

			 if(r)
			 {
				 CvMat small_img_roi;
				 CvPoint pointUp;
				 CvPoint pointDown;
				 CvScalar color = CV_RGB(0,255,0);
				 int width_ = 0;
				 int height_ = 0;

				 pointUp.x = cvRound((r->x)*scale);
				 pointUp.y = cvRound((r->y)*scale);
				 pointDown.x = cvRound((r->x + r->height)*scale);
				 pointDown.y = cvRound((r->y + r->height)*scale);

				 if(willRecord)
				 {
					IplImage* tempImg = cvCreateImage(cvSize(abs(pointUp.x - pointDown.x), abs(pointUp.y - pointDown.y)),8,3);
					IplImage* tempImgGray = cvCreateImage(cvSize(abs(pointUp.x - pointDown.x),abs(pointUp.y - pointDown.y)),8,1);
					
					if(isValidImage(tempImg) && isValidImage(tempImgGray))
					{
						cvSetImageROI(img,cvRect(min(pointUp.x,pointDown.x),min(pointUp.y,pointDown.y),tempImg->width,tempImg->height));
						cvCopy(img,tempImg);
						cvResetImageROI(img);
						cvCvtColor(tempImg,tempImgGray,CV_RGB2GRAY);

						EnterCriticalSection(&m_CriticalSectionForFaceImg);
						cvResize(tempImgGray,dlgfaceImage);
						cvEqualizeHist(dlgfaceImage,dlgfaceImage);
						willRecord = false;
						LeaveCriticalSection(&m_CriticalSectionForFaceImg);

						cvShowImage("catch",dlgfaceImage);
						
						
						cvReleaseImage(&tempImg);
						cvReleaseImage(&tempImgGray);
					}
					
					
					
				 }

				 cvRectangle(img,pointUp,pointDown,color,2);
			 }

		 }
	 }

	 cvShowImage( "face", img );
	 //

}

IplImage* ProcessThread::getImageFromCam()
{
	IplImage* img = 0; 
	
	if(!cvGrabFrame(capture))
	{         
		return 0;
	}
	
	img=cvRetrieveFrame(capture);           
	return img;
}

DWORD ProcessThread::GetLastUpdateTick()
{
	DWORD dv = 0;
	EnterCriticalSection(&m_CriticalSection);	
	dv = m_dvLastUpdate;
	LeaveCriticalSection(&m_CriticalSection);	
	return dv;
}

void ProcessThread::End()
{
}

void ProcessThread::Initialize(HWND in)
{
}

bool ProcessThread::isValidImage(IplImage* in)
{
	if(!in) return false;

	if(in->width <0 || in->width > 640) return false;
	if(in->height <0 || in->height> 480) return false;

	return true;
}

