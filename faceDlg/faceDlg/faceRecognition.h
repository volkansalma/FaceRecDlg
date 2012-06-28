#ifndef FACERECOGNITON_H
#define FACERECOGNITON_H

#define THRES1 11 //öklid.
#define THRES2 0.88 //std. dev of gallery.

using namespace std;

template<class T> class Image{
private:
	IplImage* imgp;
public:
	Image(IplImage* img=0) {imgp=img;}
	~Image(){imgp=0;}
	void operator=(IplImage* img) {imgp=img;}
	inline T* operator[](const int rowIndx) {
		return ((T *)(imgp->imageData + rowIndx*imgp->widthStep));}
};

typedef struct{
	unsigned char b,g,r;
} RgbPixel;

typedef struct
{
	int id;
	CvPoint pixel;
	float pixelValueForImage[40];
}featureVector;


typedef Image<unsigned char>  BwImage;


class faceRecognition
{
public:
	faceRecognition(void);
	~faceRecognition(void);

	//void addFace(IplImage* faceImg,int id);
	void featureExtract(IplImage*);
	void saveFeaturesToFile(int,char*);
	int getMaxRecordNum(void);
	vector<featureVector> featureVectors;
	vector<featureVector>testFace;
	void getTestFaceVector(vector<featureVector>&,vector<featureVector>);
	int compareTwoVectors(vector<featureVector>&,vector<featureVector>&,int,int);
	vector<featureVector>registeredFaces;
	void loadGalleryFacesFromFile(char*,vector<featureVector>&);
	void getNameFromId(char*,int);

private:
	IplImage* gaborResponses[40];
	stack<int, vector<int> > IdStack;
	IplImage* input;
	CvGabor *gabor1[5][8];
	IplImage* featureMaps[40];
	

	//////////////////////////////////////////////////////////////////////////
	//featureExtractionFunctions

	void prepareFeatureMaps(void);
	void prepareGaborKernels(void);
	void gaborFilter(void);
	int maximumFilter(int,int,IplImage*);
	void featurePointLocalization(IplImage*, IplImage*);
	void createFeatureVector(void);
	int getPixelValueFromImage(IplImage*,int,int);
	bool pixelAlreadyRegistered(int,int);

	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	//faceMatching functions.

	similarVectors* similarVectorContainer[40];

	double calculateVectorSimilarity(featureVector&,featureVector&);
	void getNextVector(ifstream&,featureVector&);
	inline bool pixelDistanceAvailable(featureVector&,featureVector&,double);
	inline bool isContainerCreatedForThisIDBefore(int);
	inline similarVectors* createContainerForThisID(int,int,int);
	int idCreated[40];
	int resultId;


	//////////////////////////////////////////////////////////////////////////

};

#endif
