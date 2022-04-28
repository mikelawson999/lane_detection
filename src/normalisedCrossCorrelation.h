#ifndef NORMALISEDCROSSCORRELATION_H
#define NORMALISEDCROSSCORRELATION_H
//#include <opencv2/opencv.hpp>
#include <stdint.h>
#include "../inc/mcip_core.h"


//using namespace std;

//using namespace cv;

struct Parameters{
    short winx;
    short* d;
    short* realIStr;
    short* realIEnd;
    short* realJStr;
    short* realJEnd;
    short* jMin;
    short* jMax;
    short* shiftMin;
    short* shiftMax;
    short* nRDCpt1;
    short* nRDCpt2;
    short* nRDCpt3;    
    short* searchWidth;
    double* searchRange;  
} ;

extern void funcShiftSquare(const unsigned char* inputI1, const unsigned char* inputI2, short H, short W, struct Parameters* params, double* BlockValue, double* BlockValue_SbtTop, double* Block1stCol, short* loopInd);
extern void  funcShiftSquare2(const unsigned char* inputI1, const unsigned char* inputI2, short H, short W,struct Parameters* params, double* BlockValue, double* BlockValue_SbtTop, double* Block1stCol);
extern void  funcNccCore_L(const unsigned char* inputI1, const unsigned char* inputI2, short H, short W,struct Parameters* params, double* BlockValue_SbtEndCol, double* BlockRefVal, double* outCorrScore, short *cntD);
extern void  funcNccCore_L2(const unsigned char* inputI1, const unsigned char* inputI2, short H, short W,struct Parameters* params, double* BlockValue_SbtEndCol, double* BlockRefVal, double* outCorrScore);
extern void  funcNccCore_R(const unsigned char* inputI1, const unsigned char* inputI2, short H, short W,struct Parameters* params, double* BlockValue_Sbt1stCol, double* BlockRefVal, double* outCorrScore, short *cntD);
extern void  funcNccCore_R2(const unsigned char* inputI1, const unsigned char* inputI2, short H, short W,struct Parameters* params, double* BlockValue_Sbt1stCol, double* BlockRefVal, double* outCorrScore);
extern void  funcLoopJ_L(const unsigned char* inputI1, const unsigned char* inputI2, short H, short W, unsigned char* imDisp,struct Parameters* params, short i, short* loopInd1, short* loopInd2);
extern void  funcLoopJ_L2(const unsigned char* inputI1, const unsigned char* inputI2, short H, short W, unsigned char* imDisp,struct Parameters* params, short i, short* loopInd);
extern void  funcLoopJ_R(const unsigned char* inputI1, const unsigned char* inputI2, short H, short W, unsigned char* imDisp,struct Parameters* params, short i, short* loopInd1, short* loopInd2);
extern void  funcLoopJ_R2(const unsigned char* inputI1, const unsigned char* inputI2, short H, short W, unsigned char* imDisp,struct Parameters* params, short i, short* loopInd);

//void funcNccCoreFull(Mat* inputI1, Mat* inputI2, Parameters* params, double* BlockRefVal, double *outCorrScore);
extern void  funcNccCoreFull(const unsigned char* inputI1, const unsigned char* inputI2, short H, short W,struct Parameters* params, double* BlockRefVal, double* outCorrScore);

extern void  funcUniqRange(short* inPt1, short* inPt2, short* inPt3, short* uniqueRange, short* nOelement, short shiftMax, short searchRange);
//void funcLoopJQ_L(Mat* imL, Mat* imR, Mat* imDisp, Parameters* params, short i, short *cntJ);
extern void  funcLoopJQ_L(const unsigned char* imL, const unsigned char* imR, short H, short W, unsigned char* imDisp,struct Parameters* params, short i, short* cntJ);
//void funcLoopJQ_R(Mat* imL, Mat* imR, Mat* imDisp, Parameters* params, short i, short *cntJ);
extern void  funcLoopJQ_R(const unsigned char* imL, const unsigned char* imR, short H, short W, unsigned char* imDisp,struct Parameters* params, short i, short* cntJ);
extern void  quicksort(short* a,short l,short h);
extern void  split_image(const unsigned char* in, unsigned char* imL, unsigned char* imR, short W, short H);
#endif // NORMALISEDCROSSCORRELATION_H
