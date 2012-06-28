#include "similarVectors.h"
#include "stdafx.h"

similarVectors::similarVectors(int inlabel,int testFeatureSize)
{
    //ctor

        label = inlabel;
        vectorCount = 0;

        maxSimilarity = 0;
        meanOfSimilarities = 0;
        maxCount = 0;
        NOFFInRefFace = 0;
        testAllSize = testFeatureSize;
		totalSimilarity = 0;

}

similarVectors::~similarVectors()
{
    //dtor
}
void similarVectors::addVector(float similarity)
{
     vectorCount++;
     if(similarity > maxSimilarity)
     {
        maxSimilarity = similarity;
     }
     totalSimilarity  +=  similarity;
}
float similarVectors::getMaxSimilarityValue()
{
    return maxSimilarity;
}
float similarVectors::getMeanOfSimilarities()
{
    return totalSimilarity/vectorCount;
}
float similarVectors::getOverAllsimilarity()
{
    if(vectorCount == 0)vectorCount = 1;
    return (ALPHA*getMeanOfSimilarities() + BETA*((double)vectorCount/((double)NOFFInRefFace/testAllSize)));
}
int similarVectors::getLabel()
{
    return label;
}
int similarVectors::getVectorCount()
{
    return vectorCount;
}


void similarVectors::incrementRefFeatureFaceCount()
{
    NOFFInRefFace++;
}
