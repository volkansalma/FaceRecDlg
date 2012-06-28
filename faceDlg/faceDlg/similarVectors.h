#ifndef SIMILARVECTORS_H
#define SIMILARVECTORS_H

#define ALPHA 0.48
#define BETA 0.52

class similarVectors
{
    public:
        similarVectors(int label,int testFeatureSize);
        virtual ~similarVectors();

        void addVector(float similarity);
        void voteMaxSimilarityID();
        float getMaxSimilarityValue();
        float getMeanOfSimilarities();
        float getOverAllsimilarity();
        int getLabel();
        int getVectorCount();
        void incrementRefFeatureFaceCount();

    protected:
    private:
        int label;
        int vectorCount;
        float maxSimilarity;
        float meanOfSimilarities;
        double totalSimilarity;
        int maxCount;
        int NOFFInRefFace;
        int testAllSize;
};

#endif // SIMILARVECTORS_H
