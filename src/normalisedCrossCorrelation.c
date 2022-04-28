
//#include <iostream>
#include "normalisedCrossCorrelation.h"
#include <math.h>
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


//using namespace std;
//using namespace cv;
/*
void normalisedCrossCorrelation(const unsigned char* imL, const unsigned char* imR, short H, short W, int winSize, int maxSearchRange, Mat* imDisp, bool refL=true)
{traditional Normalised Cross Correlation
    short i=0;
    short W = imL->cols, H = imL->rows;
    short halfWinSize=(winSize-1)/2;
    short winx=(winSize-1);
    short dMin = 1;
    short dMax = 100;
    short jMin = winx+maxSearchRange;
    short jMax = W-winx-1;

    Parameters parameter;
    short cntI=1;
    short cntJ=1;

    short realIstr=0,realIend=0;

    double searchRange = 0.0;
    short searchWidth = 0;

    //---------Patameters
    parameter.winx = winx;
    parameter.realIStr = &realIstr;
    parameter.realIEnd = &realIend;
    parameter.jMin = &jMin;
    parameter.jMax = &jMax;
    parameter.shiftMin = &dMin;
    parameter.shiftMax = &dMax;
    //---------Fin Parameters

    dMin = 1;
    dMax = maxSearchRange;

    searchRange = 3.0;
    searchWidth = round(searchRange*0.5);


    cntI=1;

    if (!refL)
    {
      //Calculate Bottom Line Disparity
      i=H-1-halfWinSize;
      cntJ = 1;
      realIstr=i-halfWinSize;
      realIend=i-halfWinSize+winSize;
      funcLoopJ_L(imL, imR, imDisp, &parameter, i, &cntJ, &cntI);

      //Calculate Bottom-1 to 0 Line Disparity
      for (i=H-1-halfWinSize-1;i>=halfWinSize;i--)
      {

        cntJ = 1;
        realIstr=i-halfWinSize;
        realIend=i-halfWinSize+winSize;

        funcLoopJ_L(imL, imR, imDisp, &parameter, i, &cntJ);

      }
    }
    else
    {
      jMin = winx;
      jMax = W-winx-maxSearchRange-1;

      //Calculate Bottom Line Disparity
      i=H-1-halfWinSize;
      cntJ = 1;
      realIstr=i-halfWinSize;
      realIend=i-halfWinSize+winSize;
      funcLoopJ_R(imL, imR, imDisp, &parameter, i, &cntJ, &cntI);

      //Calculate Bottom-1 to 0 Line Disparity
      for (i=H-1-halfWinSize-1;i>=halfWinSize;i--)
      {

        cntJ = 1;
        realIstr=i-halfWinSize;
        realIend=i-halfWinSize+winSize;

        funcLoopJ_R(imL, imR, imDisp, &parameter, i, &cntJ);

      }
    }

}
*/
//Sub functions

void funcLoopJ_L(const unsigned char* imL, const unsigned char* imR, short H, short W, unsigned char* imDisp,struct Parameters* params, short i, short *loopInd1, short *loopInd2)
{
  short j,d = 0;
  short realJstr=0,realJend=0;
  double BlockR_SbtEndCol = 0;
  double corrScore,prevcorrScore;
  short bestMatchSoFar;
  short cntD = 1;
  short jMin = *(params->jMin);
  short jMax = *(params->jMax);
  short shiftMin = *(params->shiftMin);
  short shiftMax = *(params->shiftMax);
  short winx = params->winx;

  double BlockValRef = 0.0;
  double BlockValRef1stCol = 0.0;

  double TempTests = 0;

  params->realJStr = &realJstr;
  params->realJEnd = &realJend;
  params->d = &d;


  for (j=jMin;j<=jMax;j++)
  {

    prevcorrScore = 0.0;
    bestMatchSoFar = 0;
    realJstr=j-winx;
    realJend=j+winx+1;

    //-----------L^2
    if (*(loopInd1) == 1)
    {
      funcShiftSquare(imL, imL, H, W, params, &BlockValRef, &TempTests, &BlockValRef1stCol, loopInd1);
    }
    else
    {
      funcShiftSquare2(imL, imL, H, W, params, &BlockValRef, &TempTests, &BlockValRef1stCol);
    }
    //-----------Fin L^2

    cntD = 1;
    for (d=-shiftMin;d>=-shiftMax;d--)
    {

      //---------First time
      if ( cntD == 1 )
      {
        funcNccCore_L(imL, imR, H, W, params, &BlockR_SbtEndCol, &BlockValRef, &corrScore, &cntD);
      }
      else //---------not first time
      {
        funcNccCore_L2(imL, imR, H, W, params, &BlockR_SbtEndCol, &BlockValRef, &corrScore);
      }

      if ( corrScore > prevcorrScore )
      {

        prevcorrScore = corrScore;
        bestMatchSoFar = (int)-d;

      }

    }

    imDisp[(((H) - i) * (W)) + j] = bestMatchSoFar;

  }

  *(loopInd2) = 2;
}

void funcLoopJ_L2(const unsigned char* imL, const unsigned char* imR, short H, short W, unsigned char* imDisp,struct Parameters* params, short i, short *loopInd)
{
  short j,d = 0;
  short realJstr=0,realJend=0;
  double BlockR_SbtEndCol = 0;
  double corrScore,prevcorrScore;
  short bestMatchSoFar;
  short cntD = 1;
  short jMin = *(params->jMin);
  short jMax = *(params->jMax);
  short shiftMin = *(params->shiftMin);
  short shiftMax = *(params->shiftMax);
  short winx = params->winx;

  double BlockValRef = 0.0;
  double BlockValRef1stCol = 0.0;

  double TempTests = 0;

  params->realJStr = &realJstr;
  params->realJEnd = &realJend;
  params->d = &d;


  for (j=jMin;j<=jMax;j++)
  {

    prevcorrScore = 0.0;
    bestMatchSoFar = 0;
    realJstr=j-winx;
    realJend=j-winx+winx+winx+1;

    //-----------L^2
    if (*(loopInd) == 1)
    {
      funcShiftSquare(imL, imL, H, W, params, &BlockValRef, &TempTests, &BlockValRef1stCol, loopInd);
    }
    else
    {
      funcShiftSquare2(imL, imL, H, W, params, &BlockValRef, &TempTests, &BlockValRef1stCol);
    }
    //-----------Fin L^2

    cntD = 1;
    for (d=-shiftMin;d>=-shiftMax;d--)
    {

      //---------First time
      if ( cntD == 1 )
      {
        funcNccCore_L(imL, imR, H, W, params, &BlockR_SbtEndCol, &BlockValRef, &corrScore, &cntD);
      }
      else //---------not first time
      {
        funcNccCore_L2(imL, imR, H, W, params, &BlockR_SbtEndCol, &BlockValRef, &corrScore);
      }

      if ( corrScore > prevcorrScore )
      {

        prevcorrScore = corrScore;
        bestMatchSoFar = (int)-d;

      }

    }

    imDisp[(((H) - i) * (W)) + j] = bestMatchSoFar;

  }

}

void funcLoopJ_R(const unsigned char* imL, const unsigned char* imR, short H, short W, unsigned char* imDisp,struct Parameters* params, short i, short *loopInd1, short *loopInd2)
{
  short j,d = 0;
  short realJstr=0,realJend=0;
  double BlockR_SbtEndCol = 0;
  double corrScore,prevcorrScore;
  //uchar bestMatchSoFar;
  int bestMatchSoFar;
  short cntD = 1;
  short jMin = *(params->jMin);
  short jMax = *(params->jMax);
  short shiftMin = *(params->shiftMin);
  short shiftMax = *(params->shiftMax);
  short winx = params->winx;

  double BlockValRef = 0.0;
  double BlockValRef1stCol = 0.0;

  double TempTests = 0;

  params->realJStr = &realJstr;
  params->realJEnd = &realJend;
  params->d = &d;


  for (j=jMin;j<=jMax;j++)
  {

    prevcorrScore = 0.0;
    bestMatchSoFar = 0;
    realJstr=j-winx;
    realJend=j+winx+1;

    //-----------L^2
    if (*(loopInd1) == 1)
    {
      funcShiftSquare(imR, imR, H, W, params, &BlockValRef, &TempTests, &BlockValRef1stCol, loopInd1);
    }
    else
    {
      funcShiftSquare2(imR, imR, H, W, params, &BlockValRef, &TempTests, &BlockValRef1stCol);
    }
    //-----------Fin L^2

    cntD = 1;
    for (d=shiftMin;d<=shiftMax;d++)
    {

      //---------First time
      if ( cntD == 1 )
      {
        funcNccCore_R(imR, imL, H, W, params, &BlockR_SbtEndCol, &BlockValRef, &corrScore, &cntD);
      }
      else //---------not first time
      {
        funcNccCore_R2(imR, imL, H, W, params, &BlockR_SbtEndCol, &BlockValRef, &corrScore);
      }

      if ( corrScore > prevcorrScore )
      {

        prevcorrScore = corrScore;
        bestMatchSoFar = (int)d;
        //cout<<" "<<(int)d<<"="<<(int)bestMatchSoFar;

      }

    }

    //cout<<" "<<(int)bestMatchSoFar;
    //imDisp->at<uchar>(i,j) = bestMatchSoFar;
    imDisp[(((H) - i) * (W)) + j] = bestMatchSoFar;

  }

  //cout<<"T_T"<<endl;
  *(loopInd2) = 2;
}

void funcLoopJ_R2(const unsigned char* imL, const unsigned char* imR, short H, short W, unsigned char* imDisp,struct Parameters* params, short i, short *loopInd)
{
  short j,d = 0;
  short realJstr=0,realJend=0;
  double BlockR_SbtEndCol = 0;
  double corrScore,prevcorrScore;
  //uchar bestMatchSoFar;
  int bestMatchSoFar;
  short cntD = 1;
  short jMin = *(params->jMin);
  short jMax = *(params->jMax);
  short shiftMin = *(params->shiftMin);
  short shiftMax = *(params->shiftMax);
  short winx = params->winx;

  double BlockValRef = 0.0;
  double BlockValRef1stCol = 0.0;

  double TempTests = 0;

  params->realJStr = &realJstr;
  params->realJEnd = &realJend;
  params->d = &d;


  for (j=jMin;j<=jMax;j++)
  {

    prevcorrScore = 0.0;
    bestMatchSoFar = 0;
    realJstr=j-winx;
    realJend=j+winx+1;

    //-----------L^2
    if (*(loopInd) == 1)
    {
      funcShiftSquare(imR, imR, H, W, params, &BlockValRef, &TempTests, &BlockValRef1stCol, loopInd);
    }
    else
    {
      funcShiftSquare2(imR, imR, H, W, params, &BlockValRef, &TempTests, &BlockValRef1stCol);
    }
    //-----------Fin L^2

    cntD = 1;
    for (d=shiftMin;d<=shiftMax;d++)
    {

      //---------First time
      if ( cntD == 1 )
      {
        funcNccCore_R(imR, imL, H, W, params, &BlockR_SbtEndCol, &BlockValRef, &corrScore, &cntD);
      }
      else //---------not first time
      {
        funcNccCore_R2(imR, imL, H, W, params, &BlockR_SbtEndCol, &BlockValRef, &corrScore);
      }

      if ( corrScore > prevcorrScore )
      {

        prevcorrScore = corrScore;
        bestMatchSoFar = (int)d;
        //cout<<" "<<(int)d<<"="<<(int)bestMatchSoFar;

      }

    }

    //cout<<" "<<(int)bestMatchSoFar;
    imDisp[(((H) - i) * (W)) + j] = bestMatchSoFar;

  }
  //cout<<"T_T"<<endl;

}

void funcShiftSquare(const unsigned char* imL, const unsigned char* imR, short H, short W,struct Parameters* params, double* BlockValue, double* BlockValue_SbtTop, double* Block1stCol, short* loopInd)
{
  short realJ=0,realI=0;

  short realIstr = *(params->realIStr);
  short realIend = *(params->realIEnd);
  short realJstr = *(params->realJStr);
  short realJend = *(params->realJEnd);

  double doubleBlock_SbtTop=0;
  double doubleBlockTopLeft=0;
  double doubleBlockTopRow=0;

  //Calculate 1,1 of the block
  realI = realIstr;
  realJ = realJstr;
  //doubleBlockTopLeft = inputI1->at<uchar>(realI,realJ) * inputI2->at<uchar>(realI,realJ);
  doubleBlockTopLeft = imL[(((H) - realI) * (W)) + realJ] * imR[(((H) - realI) * (W)) + realJ];

  //Calculate the first Row of the block
  realI = realIstr;
  doubleBlockTopRow = doubleBlockTopLeft;
  for (realJ=realJstr+1;realJ<realJend;realJ++)
  {
    //doubleBlockTopRow += inputI1->at<uchar>(realI,realJ) * inputI2->at<uchar>(realI,realJ);
	  doubleBlockTopRow += imL[(((H) - realI) * (W)) + realJ] * imR[(((H) - realI) * (W)) + realJ];
  }

  //Calculate the first Col (from 2nd Row) of the block
  realJ = realJstr;
  for (realI=realIstr+1;realI<realIend;realI++)
  {
    //doubleBlock_SbtTop += inputI1->at<uchar>(realI,realJ) * inputI2->at<uchar>(realI,realJ);
	  doubleBlock_SbtTop += imL[(((H) - realI) * (W)) + realJ] * imR[(((H) - realI) * (W)) + realJ];
  }

  //Total first Col Value
  //*(params->doubleTemp1stL) = doubleBlock_SbtTop + doubleBlockTopLeft;
  *(Block1stCol) = doubleBlock_SbtTop + doubleBlockTopLeft;

  //Calculate the 2nd Col to the End Col (from 2nd Row) of the block
  for (realJ=realJstr+1;realJ<realJend;realJ++)
  {
    for (realI=realIstr+1;realI<realIend;realI++)
    {
      //doubleBlock_SbtTop += inputI1->at<uchar>(realI,realJ) * inputI2->at<uchar>(realI,realJ);
    	doubleBlock_SbtTop += imL[(((H) - realI) * (W)) + realJ] * imR[(((H) - realI) * (W)) + realJ];
    }
  }

  //Total Block Value
  //*(params->doubleTempL) = doubleBlock_SbtTop + doubleBlockTopRow;
  *(BlockValue) = doubleBlock_SbtTop + doubleBlockTopRow;

  //params->doubleTempSbtTopArryL[j] = doubleBlock_SbtTop;
  *(BlockValue_SbtTop) = doubleBlock_SbtTop;

  //change First Loop indicator.
  *loopInd = 2;

}

void funcShiftSquare2(const unsigned char* imL, const unsigned char* imR, short H, short W,struct Parameters* params, double* BlockValue, double* BlockValue_SbtTop, double* Block1stCol)
{
  short realJ=0,realI=0;

  short realIstr = *(params->realIStr);
  short realIend = *(params->realIEnd);
  short realJstr = *(params->realJStr);
  short realJend = *(params->realJEnd);

  double doubleBlockValueT=0;
  double doubleBlock1stCol=0;
  double doubleBlockTopRow=0;
  double doubleBlockEndCol=0;
  double doubleBlockCornerL[4]={0}; //0~3 TL TR BL BR

  //Top-Left
  realI = realIstr;
  realJ = realJstr;
  //doubleBlockCornerL[0]= inputI1->at<uchar>(realI,realJ) * inputI2->at<uchar>(realI,realJ);
  doubleBlockCornerL[0]= imL[(((H) - realI) * (W)) + realJ] * imR[(((H) - realI) * (W)) + realJ];
  //Top-Right
  realJ = realJend-1;
  //doubleBlockCornerL[1]= inputI1->at<uchar>(realI,realJ) * inputI2->at<uchar>(realI,realJ);
  doubleBlockCornerL[1]= imL[(((H) - realI) * (W)) + realJ] * imR[(((H) - realI) * (W)) + realJ];
  //Bottom-Right
  realI = realIend-1;
  //doubleBlockCornerL[3]= inputI1->at<uchar>(realI,realJ) * inputI2->at<uchar>(realI,realJ);
  doubleBlockCornerL[3]= imL[(((H) - realI) * (W)) + realJ] * imR[(((H) - realI) * (W)) + realJ];
  //Bottom-Left
  realJ = realJstr;
  //doubleBlockCornerL[2]= inputI1->at<uchar>(realI,realJ) * inputI2->at<uchar>(realI,realJ);
  doubleBlockCornerL[2]= imL[(((H) - realI) * (W)) + realJ] * imR[(((H) - realI) * (W)) + realJ];


  //Calculate the 1st Col (from 2nd Row to End-1 Row) of the block
  realJ = realJstr;
  for (realI=realIstr+1;realI<realIend-1;realI++)
  {
    //doubleBlock1stCol += inputI1->at<uchar>(realI,realJ) * inputI2->at<uchar>(realI,realJ);
	  doubleBlock1stCol += imL[(((H) - realI) * (W)) + realJ] * imR[(((H) - realI) * (W)) + realJ];
  }

  //Total Value of 1st Col (+TL&BL)
  doubleBlock1stCol += doubleBlockCornerL[0] + doubleBlockCornerL[2];

  //Calculate the End Col (from 2nd Row to End-1 Row) of the block
  realJ = realJend-1;
  for (realI=realIstr+1;realI<realIend-1;realI++)
  {
    //doubleBlockEndCol += inputI1->at<uchar>(realI,realJ) * inputI2->at<uchar>(realI,realJ);
	  doubleBlockEndCol += imL[(((H) - realI) * (W)) + realJ] * imR[(((H) - realI) * (W)) + realJ];
  }

  //Total Value of End Col (+TR&BR)
  doubleBlockEndCol += doubleBlockCornerL[1] + doubleBlockCornerL[3];

  //Total Block Value
  doubleBlockValueT = *(BlockValue) - *(Block1stCol) + doubleBlockEndCol;
  *(BlockValue) = doubleBlockValueT;

  //Update 1st Col Value
  *(Block1stCol) = doubleBlock1stCol;

  //Calculate the 1st Row (from 2nd Col to End-1 Col) of the block
  realI = realIstr;
  for (realJ=realJstr+1;realJ<realJend-1;realJ++)
  {
    //doubleBlockTopRow += inputI1->at<uchar>(realI,realJ) * inputI2->at<uchar>(realI,realJ);
    doubleBlockTopRow += imL[(((H) - realI) * (W)) + realJ] * imR[(((H) - realI) * (W)) + realJ];
  }

  //Total Value of 1st Row (+TL&TR)
  doubleBlockTopRow += doubleBlockCornerL[0] + doubleBlockCornerL[1];

  //params->doubleTempSbtTopArryL[j] = *(params->doubleTempL) - doubleBlockTopRow;
  *(BlockValue_SbtTop) = doubleBlockValueT - doubleBlockTopRow;

}

void funcNccCore_L(const unsigned char* imL, const unsigned char* imR, short H, short W,struct Parameters* params, double* BlockValue_SbtEndCol, double* BlockRefVal, double* outCorrScore, short *cntD)
{
  short realJ=0,realI=0;
  short d=*(params->d);

  short realIstr = *(params->realIStr);
  short realIend = *(params->realIEnd);
  short realJstr = *(params->realJStr);
  short realJend = *(params->realJEnd);

  double BlockValue1_New = *(BlockRefVal);
  double BlockValue2_New = 0;
  double BlockValue3_New = 0;

  //BlockValue 1st Col to End-1 Col
  for (realJ=realJstr;realJ<realJend-1;realJ++)
  {
    for (realI=realIstr;realI<realIend;realI++)
    {      
      //BlockValue2_New += inputI2->at<uchar>(realI,realJ+d) * inputI2->at<uchar>(realI,realJ+d);
      //BlockValue3_New += inputI1->at<uchar>(realI,realJ)   * inputI2->at<uchar>(realI,realJ+d);
    	BlockValue2_New += imR[(((H) - realI) * (W)) + realJ + d] * imR[(((H) - realI) * (W)) + realJ + d];
    	BlockValue3_New += imL[(((H) - realI) * (W)) + realJ] * imR[(((H) - realI) * (W)) + realJ + d];
    }
  }

  //BlockValue no End Col
  *(BlockValue_SbtEndCol) = BlockValue2_New;

  //BlockValue End-1 Col
  realJ=realJend-1;
  for (realI=realIstr;realI<realIend;realI++)
  {
    //BlockValue2_New += inputI2->at<uchar>(realI,realJ+d) * inputI2->at<uchar>(realI,realJ+d);
    //BlockValue3_New += inputI1->at<uchar>(realI,realJ)   * inputI2->at<uchar>(realI,realJ+d);
	  BlockValue2_New += imR[(((H) - realI) * (W)) + realJ + d] * imR[(((H) - realI) * (W)) + realJ + d];
	  BlockValue3_New += imL[(((H) - realI) * (W)) + realJ] * imR[(((H) - realI) * (W)) + realJ + d];
  }

  *(outCorrScore) = BlockValue3_New / ( sqrt( BlockValue1_New ) * sqrt( BlockValue2_New ) );

  *cntD = 2;

}

void funcNccCore_L2(const unsigned char* imL, const unsigned char* imR, short H, short W,struct Parameters* params, double* BlockValue_SbtEndCol, double* BlockRefVal,  double* outCorrScore)
{
  short realJ=0,realI=0;
  short d=*(params->d);

  short realIstr = *(params->realIStr);
  short realIend = *(params->realIEnd);
  short realJstr = *(params->realJStr);
  short realJend = *(params->realJEnd);

  double BlockValue1_New = *(BlockRefVal);
  double BlockValue2_New = 0;
  double BlockValue2_1stCol = 0;
  double BlockValue2_EndCol = 0;
  double BlockValue3_New = 0;

  //Calculate BlockValue
  for (realJ=realJstr;realJ<realJend;realJ++)
  {
    for (realI=realIstr;realI<realIend;realI++)
    {
      //BlockValue3_New += inputI1->at<uchar>(realI,realJ) * inputI2->at<uchar>(realI,realJ+d);
    	BlockValue3_New += imL[(((H) - realI) * (W)) + realJ] * imR[(((H) - realI) * (W)) + realJ + d];
    }
  }  

  //BlockValue 1st Col
  realJ = realJstr;
  for (realI=realIstr;realI<realIend;realI++)
  {
    //BlockValue2_1stCol  += inputI2->at<uchar>(realI,realJ+d) * inputI2->at<uchar>(realI,realJ+d);
	  BlockValue2_1stCol  += imR[(((H) - realI) * (W)) + realJ + d] * imR[(((H) - realI) * (W)) + realJ + d];
  }

  //BlockValue End Col
  realJ = realJend-1;
  for (realI=realIstr;realI<realIend;realI++)
  {    
    //BlockValue2_EndCol += inputI2->at<uchar>(realI,realJ+d) * inputI2->at<uchar>(realI,realJ+d);
	  BlockValue2_EndCol += imR[(((H) - realI) * (W)) + realJ + d] * imR[(((H) - realI) * (W)) + realJ + d];
  }

  BlockValue2_New = *(BlockValue_SbtEndCol) + BlockValue2_1stCol;

  *(BlockValue_SbtEndCol) = BlockValue2_New - BlockValue2_EndCol;

  *(outCorrScore) = BlockValue3_New / ( sqrt( BlockValue1_New ) * sqrt( BlockValue2_New ) );

}

void funcNccCore_R(const unsigned char* imL, const unsigned char* imR, short H, short W,struct Parameters* params, double* BlockValue_Sbt1stCol, double* BlockRefVal, double* outCorrScore, short *cntD)
{
  short realJ=0,realI=0;
  short d=*(params->d);

  short realIstr = *(params->realIStr);
  short realIend = *(params->realIEnd);
  short realJstr = *(params->realJStr);
  short realJend = *(params->realJEnd);

  double BlockValue1_New = *(BlockRefVal);
  double BlockValue2_New = 0;
  double BlockValue3_New = 0;

  //BlockValue 2nd Col to End Col
  for (realJ=realJstr+1;realJ<realJend;realJ++)
  {
    for (realI=realIstr;realI<realIend;realI++)
    {
      //BlockValue2_New += inputI2->at<uchar>(realI,realJ+d) * inputI2->at<uchar>(realI,realJ+d);
      //BlockValue3_New += inputI1->at<uchar>(realI,realJ)   * inputI2->at<uchar>(realI,realJ+d);
    	BlockValue2_New += imR[(((H) - realI) * (W)) + realJ + d] * imR[(((H) - realI) * (W)) + realJ + d];
    	BlockValue3_New += imL[(((H) - realI) * (W)) + realJ] * imR[(((H) - realI) * (W)) + realJ + d];
    }
  }

  //BlockValue no End Col
  *(BlockValue_Sbt1stCol) = BlockValue2_New;

  //BlockValue 1st Col
  realJ=realJstr;
  for (realI=realIstr;realI<realIend;realI++)
  {
    //BlockValue2_New += inputI2->at<uchar>(realI,realJ+d) * inputI2->at<uchar>(realI,realJ+d);
    //BlockValue3_New += inputI1->at<uchar>(realI,realJ)   * inputI2->at<uchar>(realI,realJ+d);
	  BlockValue2_New += imR[(((H) - realI) * (W)) + realJ + d] * imR[(((H) - realI) * (W)) + realJ + d];
	  BlockValue3_New += imL[(((H) - realI) * (W)) + realJ] * imR[(((H) - realI) * (W)) + realJ + d];
  }

  *(outCorrScore) = BlockValue3_New / ( sqrt( BlockValue1_New ) * sqrt( BlockValue2_New ) );

  *cntD = 2;

}

void funcNccCore_R2(const unsigned char* imL, const unsigned char* imR, short H, short W,struct Parameters* params, double* BlockValue_Sbt1stCol, double* BlockRefVal,  double* outCorrScore)
{
  short realJ=0,realI=0;
  short d=*(params->d);

  short realIstr = *(params->realIStr);
  short realIend = *(params->realIEnd);
  short realJstr = *(params->realJStr);
  short realJend = *(params->realJEnd);

  double BlockValue1_New = *(BlockRefVal);
  double BlockValue2_New = 0;
  double BlockValue2_1stCol = 0;
  double BlockValue2_EndCol = 0;
  double BlockValue3_New = 0;

  //Calculate BlockValue
  for (realJ=realJstr;realJ<realJend;realJ++)
  {
    for (realI=realIstr;realI<realIend;realI++)
    {
      //BlockValue3_New += inputI1->at<uchar>(realI,realJ) * inputI2->at<uchar>(realI,realJ+d);
    	BlockValue3_New += imL[(((H) - realI) * (W)) + realJ] * imR[(((H) - realI) * (W)) + realJ + d];

    }
  }

  //BlockValue 1st Col
  realJ = realJstr;
  for (realI=realIstr;realI<realIend;realI++)
  {
    //BlockValue2_1stCol  += inputI2->at<uchar>(realI,realJ+d) * inputI2->at<uchar>(realI,realJ+d);
	  BlockValue2_1stCol  += imR[(((H) - realI) * (W)) + realJ + d] * imR[(((H) - realI) * (W)) + realJ + d];
  }

  //BlockValue End Col
  realJ = realJend-1;
  for (realI=realIstr;realI<realIend;realI++)
  {
    //BlockValue2_EndCol += inputI2->at<uchar>(realI,realJ+d) * inputI2->at<uchar>(realI,realJ+d);
	  BlockValue2_EndCol += imR[(((H) - realI) * (W)) + realJ + d] * imR[(((H) - realI) * (W)) + realJ + d];
  }

  BlockValue2_New = *(BlockValue_Sbt1stCol) + BlockValue2_EndCol;

  *(BlockValue_Sbt1stCol) = BlockValue2_New - BlockValue2_1stCol;

  *(outCorrScore) = BlockValue3_New / ( sqrt( BlockValue1_New ) * sqrt( BlockValue2_New ) );

}
