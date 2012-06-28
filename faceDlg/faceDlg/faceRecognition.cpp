#include "faceRecognition.h"
#include "stdafx.h"

faceRecognition::faceRecognition(void)
{
	input = 0;
	for (int i= 0; i < 40; i++)
	{
		gaborResponses[i] = 0;
		featureMaps[i] = 0;
	}

	prepareGaborKernels();
	loadGalleryFacesFromFile("vsFaceDb.txt",registeredFaces);
	resultId = -1;

}
faceRecognition::~faceRecognition(void)
{
	

}
void faceRecognition::featureExtract(IplImage* img)
{
	if (!img)return;
	if (img->nChannels != 1) return ; //tek kanala çeviren fonk yazýlacak.

	IplImage* temp = cvCloneImage(img);
	if(input) cvReleaseImage(&input);
	input = cvCreateImage(cvSize(92,92),8,1); //50,50 idi 92,112

	cvResize(temp,input);
	cvReleaseImage(&temp);

	gaborFilter();

	prepareFeatureMaps();

	createFeatureVector();

}
void faceRecognition::prepareGaborKernels(void){

	double Sigma = PI;
	double F = 0.8 ;  // 0 ile 4 arasý deðiþecek. //0.8 paper

	int frequency = 2 ;

	for(int row = 0; row< 5; row++ )
	{
		double orientation = 3*PI/2;
		for(int column = 0; column<8; column++)
		{
			gabor1[row][column] = new CvGabor;
			gabor1[row][column]->Init(orientation , frequency , Sigma, F);

			orientation -= 0.125 * PI;

			//cout<<"F: "<<F<<" O:"<<orientation<<endl;
		}
		F += 0.1 ; //0.1 paper

	}

}
void faceRecognition::gaborFilter(void){

	for(int row = 0; row< 5; row++ )
	{
      		for(int column = 0; column<8; column++)
		{
			gaborResponses[(row*8) + column] =  cvCreateImage(cvSize(input->width,input->height), IPL_DEPTH_8U, 1);
			gabor1[row][column]->conv_img(input, gaborResponses[(row*8) + column], CV_GABOR_MAG);


			//show gabor kernel
			//int test = gabor1[row][column]->get_mask_width();
 			IplImage *kernel = cvCreateImage(cvSize(/*gabor1[row][column]->get_mask_width(), gabor1[row][column]->get_mask_width()*/15,15), IPL_DEPTH_8U, 1);
			kernel = gabor1[row][column]->get_image(CV_GABOR_REAL);

			//cvNamedWindow("Gabor Kernel", 1);
			//cvShowImage("Gabor Kernel", kernel);

			cvReleaseImage(&kernel);
			//show gabor kernel

			//show convolved result
			//cvNamedWindow("gabor");
			//cvShowImage("gabor",gaborResponses[(row*8) + column]);
			//cvNamedWindow("face");
			//cvShowImage("face",input);
			//cvWaitKey();
			//show convolved result


 		}

	}
}
int faceRecognition::maximumFilter(int x, int y, IplImage* in){
	CvPoint topLeft,topRight,bottomLeft,bottomRight;
	BwImage img(in); // tek kanal byte
	int maximum = 0;
	int windowSize = 14;

	if(x-windowSize > 0)
	{
		topLeft.x = x-windowSize ;
		bottomLeft.x = x-windowSize ;
	}

	else
	{
		topLeft.x = 0;
		bottomLeft.x = 0;
	}

	if(y-windowSize  > 0)
	{
		topLeft.y = y-windowSize ;
		topRight.y = y-windowSize ;
	}

	else
	{
		topLeft.y = 0;
		topRight.y = 0;
	}

	if(x+windowSize  < in->width)
	{
		topRight.x = x+windowSize ;
		bottomRight.x = x+windowSize ;
	}
	else
	{
		topRight.x = in->width;
		bottomRight.x = in->width;
	}

	if(y+windowSize  < in->height)
	{
		bottomLeft.y = y+windowSize ;
		bottomRight.y = y+windowSize ;
	}
	else
	{
		bottomLeft.y = in->height;
		bottomRight.y = in->height;
	}

	for(int y = topRight.y; y<= bottomRight.y; y++)
	{
		for(int x = topLeft.x; x<=topRight.x ; x++)
		{
			char pixelVal = img[y][x];
			if(  pixelVal  > maximum ) maximum = pixelVal ;
		}
	}

	return maximum;
}
void faceRecognition::featurePointLocalization(IplImage* sourceImg, IplImage* resultImage){
	CvScalar averageValue;
	double avg;
	int max = 0;

	averageValue = cvAvg(sourceImg);
	avg = averageValue.val[0];

	BwImage source(sourceImg); //tek kanal byte
	BwImage result(resultImage); //tek kanal gray
	int featureCnt = 0;

	long tick1 = GetTickCount();
	for(int x = 0; x < sourceImg->width; x++)
	{
		for(int y = 0; y < sourceImg->height; y++)
		{
			char pixelValue = source[y][x];
			if(pixelValue <= avg) continue;

				max = maximumFilter(x,y,sourceImg);
			
				if(pixelValue == max)
				{
					result[y][x] = 255;
					featureCnt++;
				}
		}
	}

	long tick2 = GetTickCount();

	long time = tick2 - tick1;

    //cout<<"Feature Cnt:"<<featureCnt<<endl;


}
void faceRecognition::prepareFeatureMaps(void){

	for(int cnt = 0; cnt < 40; cnt++ )
	{
			if(!featureMaps[cnt]) featureMaps[cnt] = cvCreateImage(cvGetSize(gaborResponses[cnt]),8,1);

			cvZero(featureMaps[cnt]);
			featurePointLocalization(gaborResponses[cnt],featureMaps[cnt]);

			//cvNamedWindow("features");
			//cvShowImage("features",featureMaps[cnt]);
		    //cvWaitKey();
	}
}
void faceRecognition::createFeatureVector(void)
{
	featureVector vector;
	featureVectors.clear();

	for (int i= 0; i< 40; i++)
	{
		BwImage img(featureMaps[i]);

		for(int w=0; w < featureMaps[i]->width; w++)
		{
			for(int h=0; h < featureMaps[i]->height; h++)
			{
				if(img[h][w])
				{
					if(pixelAlreadyRegistered(w,h)) continue;
					vector.pixel.x = w;
					vector.pixel.y = h;

					for(int img = 0; img < 40; img++)
					{
						vector.pixelValueForImage[img] = getPixelValueFromImage(gaborResponses[img],w,h)/255.0;
					}

					featureVectors.push_back(vector);

				}
			}
		}


	}

}
int faceRecognition::getPixelValueFromImage(IplImage* img, int w, int h)
{
	BwImage img_(img);
	return img_[h][w];
}

bool faceRecognition::pixelAlreadyRegistered(int x,int y)
{
	for (int i = 0; i < featureVectors.size(); i++)
	{
		if(featureVectors[i].pixel.x == x && featureVectors[i].pixel.y == y)
			return true;
	}

	return false;
}


void faceRecognition::saveFeaturesToFile(int id, char* isim)
{
    FILE* featureFile = 0; 
    featureFile = fopen("vsFaceDb.txt","a");

    FILE* fileNameID = 0;
    fileNameID  = fopen("fileNameID.txt","a");


    for (int i = 0 ; i < featureVectors.size(); i++)
	{
	    fprintf(featureFile,"%d ",id);
	    fprintf(featureFile,"%d ",featureVectors[i].pixel.x);
		fprintf(featureFile,"%d ",featureVectors[i].pixel.y);


		for (int item = 0; item < 40; item++)
		{
			fprintf(featureFile,"%f ",featureVectors[i].pixelValueForImage[item]);

		}
		fprintf(featureFile,"\n");
	}

	fclose(featureFile);

	fprintf(fileNameID,"%d %s\n",id,isim);
	fclose(fileNameID);

	
}
int faceRecognition::getMaxRecordNum(void)
{
	int maxNum = 0;
	FILE* fileNameID = 0;
	fileNameID  = fopen("fileNameID.txt","r");

	if(!fileNameID) return maxNum;
	int num;
	char isim[50];

	int returnVal = 0;
	
	do 
	{
		returnVal = fscanf(fileNameID,"%d",&num);
		fscanf(fileNameID,"%s",isim);
		if(num > maxNum) maxNum = num;
	} 
	while(returnVal != EOF);

	fclose(fileNameID);

	return maxNum;

}

void faceRecognition::getTestFaceVector(vector<featureVector> &testAll, vector<featureVector> vectorFromFaceExtract)
{

	testAll.clear();
	for(int i = 0; i < vectorFromFaceExtract.size(); i++)
	{
		testAll.push_back(vectorFromFaceExtract[i]);
	}

	/*
	ifstream f1;
	f1.open(fileName,ios::in);
	featureVector test;

	while(!f1.eof())
	{
		getNextVector(f1,test);
		testAll.push_back(test);
	}
	*/

}
void faceRecognition::loadGalleryFacesFromFile(char* fileName,vector<featureVector> &galleryAll)
{
	ifstream f1;
	f1.open(fileName,ios::in);

	if(!f1) return;

	featureVector gallery;

	galleryAll.clear();
	while(!f1.eof())
	{
		getNextVector(f1,gallery);
		galleryAll.push_back(gallery);
	}

	f1.close();
}

int faceRecognition::compareTwoVectors(vector<featureVector> &testAll,vector<featureVector> &galleryAll,int th1,int th2)
{

	featureVector test;
	featureVector gallery;

	vector<similarVectors*>results;

	for(int i=0 ; i<=40; i++)
	{
		idCreated[i]=-1;
	}

	double maxSimilarity = 0;
	double meanOfSimilarity = 0;


	int galleryAllSize = galleryAll.size();
	int testAllSize = testAll.size();

	int classIndex = 0;
	int maxSimilarityId = 0;

	for(int t = 0; t < testAllSize ; t++)
	{
		maxSimilarity = 0;
		maxSimilarityId = -1;

		for(int g = 0; g < galleryAllSize  ; g++)
		{
			int id = galleryAll[g].id;

			if(!(isContainerCreatedForThisIDBefore(id)))
			{
				similarVectorContainer[classIndex] =
					createContainerForThisID(id,classIndex,testAllSize);
				classIndex++;
			}

			similarVectorContainer[idCreated[id]]->incrementRefFeatureFaceCount();

			if(!pixelDistanceAvailable(testAll[t],galleryAll[g],th1))
			{
				continue;
			}

			double similarity = calculateVectorSimilarity(testAll[t],galleryAll[g]);

			if(similarity < th2)
			{
				continue;
			}

			//similarVectorContainer[idCreated[id]]->addVector(similarity);

			if(similarity>maxSimilarity) //galeri içindeki max vektör.
			{
				maxSimilarity = similarity;
				maxSimilarityId = id;
			}

		}

		if(maxSimilarity != 0)
		{
			similarVectorContainer[idCreated[maxSimilarityId]]->addVector(maxSimilarity);
		}

	}

	double max_ = 0;
	int id_ = 0;

	for(int test = 1; test <= 40; test++ )
	{
		similarVectors* buffer;

		if(idCreated[test] != -1)
		{
			buffer = similarVectorContainer[idCreated[test]];


			/*
			cout<<buffer->getLabel()<<": "<<" MeanOfSim: "<<buffer->getMeanOfSimilarities()<<" VectorCnt: "
				<<buffer->getVectorCount()<<" OverAll: "<<buffer->getOverAllsimilarity()
				<<endl;
			*/

			//cout<<buffer->getLabel()<<": "<<" Similarity: "<<buffer->getOverAllsimilarity()<<endl;
			
			if(buffer->getOverAllsimilarity() > max_)
			{
				max_ = buffer->getOverAllsimilarity();
				id_ = test;
			}

			buffer->~similarVectors();
		}
	}
	//cout<<"**** Result: "<<id_<<" Score: "<<max_<<endl<<endl;

	return id_;
}

inline bool faceRecognition::pixelDistanceAvailable(featureVector &a,featureVector &b,double th1)
{
	if( ( sqrt(pow(double(a.pixel.x-b.pixel.x),2) + pow(double(a.pixel.y-b.pixel.y),2)) ) > th1)
		return false;

	return true;
}

void faceRecognition::getNextVector(ifstream &f1,featureVector &result)
{
	char line[1024];
	char valueEs[10];

	f1.getline(line,1024);

	sscanf(line,"%d %d %d %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
		&result.id,
		&result.pixel.x,
		&result.pixel.y,
		&result.pixelValueForImage[0],
		&result.pixelValueForImage[1],
		&result.pixelValueForImage[2],
		&result.pixelValueForImage[3],
		&result.pixelValueForImage[4],
		&result.pixelValueForImage[5],
		&result.pixelValueForImage[6],
		&result.pixelValueForImage[7],
		&result.pixelValueForImage[8],
		&result.pixelValueForImage[9],
		&result.pixelValueForImage[10],
		&result.pixelValueForImage[11],
		&result.pixelValueForImage[12],
		&result.pixelValueForImage[13],
		&result.pixelValueForImage[14],
		&result.pixelValueForImage[15],
		&result.pixelValueForImage[16],
		&result.pixelValueForImage[17],
		&result.pixelValueForImage[18],
		&result.pixelValueForImage[19],
		&result.pixelValueForImage[20],
		&result.pixelValueForImage[21],
		&result.pixelValueForImage[22],
		&result.pixelValueForImage[23],
		&result.pixelValueForImage[24],
		&result.pixelValueForImage[25],
		&result.pixelValueForImage[26],
		&result.pixelValueForImage[27],
		&result.pixelValueForImage[28],
		&result.pixelValueForImage[29],
		&result.pixelValueForImage[30],
		&result.pixelValueForImage[31],
		&result.pixelValueForImage[32],
		&result.pixelValueForImage[33],
		&result.pixelValueForImage[34],
		&result.pixelValueForImage[35],
		&result.pixelValueForImage[36],
		&result.pixelValueForImage[37],
		&result.pixelValueForImage[38],
		&result.pixelValueForImage[39]
	);

}

double faceRecognition::calculateVectorSimilarity(featureVector& k,featureVector& j)
{
	double pay = 0;
	double paydaToplamIc = 0;
	double paydaToplamDis = 0;
	double similarity = 0;

	for(int i = 0; i < 40; i++)
	{
		pay += k.pixelValueForImage[i] * j.pixelValueForImage[i];
		paydaToplamIc += pow(j.pixelValueForImage[i],2);
	}

	for(int i = 0; i <40; i++)
	{
		paydaToplamDis += pow(k.pixelValueForImage[i],2)*paydaToplamIc;
	}

	similarity = pay / sqrt(paydaToplamDis);

	return similarity;

}
inline bool faceRecognition::isContainerCreatedForThisIDBefore(int a)
{
	if(idCreated[a] == -1)
	{
		return false;
	}
	return true;
}
inline similarVectors* faceRecognition::createContainerForThisID(int a,int classIndex,int testFeatureSize)
{
	idCreated[a] = classIndex;
	return new similarVectors(a,testFeatureSize);
}

void faceRecognition::getNameFromId(char* name,int id)
{
	FILE* idFile = 0;
	idFile = fopen("fileNameID.txt","r");
	if(!idFile) return;

	int readId = -1;
	int fileResult = -1;
	char readName[50];

	do 
	{
		fileResult = fscanf(idFile,"%d %s\n", &readId,&readName);
		if (id == readId)
		{
			sprintf(name,"%s",readName);
			fclose(idFile);
			return;
		}

	} while (fileResult != EOF);
	
}