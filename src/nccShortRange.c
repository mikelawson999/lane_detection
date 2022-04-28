
#include <iostream>
#include <stdio.h>
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
extern void  funcShiftSquare(const unsigned char* inputI1, const unsigned char* inputI2, short H, short W,struct Parameters* params, double* BlockValue, double* BlockValue_SbtTop, double* Block1stCol);
extern void  funcNccCore_L(const unsigned char* inputI1, const unsigned char* inputI2, short H, short W,struct Parameters* params, double* BlockValue_SbtEndCol, double* BlockRefVal, double* outCorrScore, short *cntD);
extern void  funcNccCore_L(const unsigned char* inputI1, const unsigned char* inputI2, short H, short W,struct Parameters* params, double* BlockValue_SbtEndCol, double* BlockRefVal, double* outCorrScore);
extern void  funcNccCore_R(const unsigned char* inputI1, const unsigned char* inputI2, short H, short W,struct Parameters* params, double* BlockValue_Sbt1stCol, double* BlockRefVal, double* outCorrScore, short *cntD);
extern void  funcNccCore_R(const unsigned char* inputI1, const unsigned char* inputI2, short H, short W,struct Parameters* params, double* BlockValue_Sbt1stCol, double* BlockRefVal, double* outCorrScore);
extern void  funcLoopJ_L(const unsigned char* inputI1, const unsigned char* inputI2, short H, short W, unsigned char* imDisp,struct Parameters* params, short i, short* loopInd1, short* loopInd2);
extern void  funcLoopJ_L(const unsigned char* inputI1, const unsigned char* inputI2, short H, short W, unsigned char* imDisp,struct Parameters* params, short i, short* loopInd);
extern void  funcLoopJ_R(const unsigned char* inputI1, const unsigned char* inputI2, short H, short W, unsigned char* imDisp,struct Parameters* params, short i, short* loopInd1, short* loopInd2);
extern void  funcLoopJ_R(const unsigned char* inputI1, const unsigned char* inputI2, short H, short W, unsigned char* imDisp,struct Parameters* params, short i, short* loopInd);

//using namespace std;
//using namespace cv;

void split_image(const unsigned char* in, unsigned char* imL, unsigned char* imR, short H, short W)
{
	int i;
	for(i = 0; i < H*W; i++)
	{
		imL[i] = in[i];
	}
	for(i =H*W; i < H*W*2 ; i++)
	{
		imR[i - (H*W)] = in[i];
	}
}

void nccShortRange(const unsigned char* imL, const unsigned char* imR, short H, short W, int winSize, int maxSearchRange, unsigned char* imDisp, bool refL=true)
{//Main function to run for our proposed algorithm
    short i=0;
    //short W = imL->cols, H = imL->rows;
    short halfWinSize=(winSize-1)/2;
    short winx=(winSize-1);
    short dMin = 1;
    short dMax = 100;
    short jMin = winx+maxSearchRange;
    short jMax = W-winx-1;

    struct Parameters parameter;
    short cntI=1;
    short cntJ=1;

    short realIstr=0,realIend=0;

    double searchRange = 0.0;
    short searchWidth = 0;
    short cnt=0;

    short* nextRowEstDispariyCenters1;
    short* nextRowEstDispariyCenters2;
    short* nextRowEstDispariyCenters3;

    nextRowEstDispariyCenters1 = new short [W];
    nextRowEstDispariyCenters2 = new short [W];
    nextRowEstDispariyCenters3 = new short [W];

    //---------Patameters
    parameter.winx = winx;
    parameter.realIStr = &realIstr;
    parameter.realIEnd = &realIend;
    parameter.jMin = &jMin;
    parameter.jMax = &jMax;
    parameter.shiftMin = &dMin;
    parameter.shiftMax = &dMax;
    parameter.nRDCpt1 = nextRowEstDispariyCenters1;
    parameter.nRDCpt2 = nextRowEstDispariyCenters2;
    parameter.nRDCpt3 = nextRowEstDispariyCenters3;
    parameter.searchRange = &searchRange;
    parameter.searchWidth = &searchWidth;
    //---------Fin Parameters

    dMin = 1;
    dMax = maxSearchRange;

    searchRange = 3.0;
    //searchWidth = round(searchRange*0.5);
    searchWidth = searchRange*0.5;

    cntI=1;

    if (!refL)
    {//Calculate disparity with the right image as the reference
      //Calculate Bottom Line Disparity
      i=H-1-halfWinSize;
      cntJ = 1;
      realIstr=i-halfWinSize;
      realIend=i-halfWinSize+winSize;
      funcLoopJ_L(imL, imR, H, W,imDisp, &parameter, i, &cntJ, &cntI);

      //Calculate new Disparity Center
      for (cnt=0;cnt<W-1;cnt++)
      {
        //nextRowEstDispariyCenters1[cnt+1] = imDisp->at<uchar>(i,cnt) - searchWidth;
        //nextRowEstDispariyCenters2[cnt+1] = imDisp->at<uchar>(i,cnt+1) - searchWidth;
        //nextRowEstDispariyCenters3[cnt+1] = imDisp->at<uchar>(i,cnt+2) - searchWidth;
    	  nextRowEstDispariyCenters1[cnt+1] = imDisp[(((H) - i) * (W)) + cnt] - searchWidth;
    	  nextRowEstDispariyCenters2[cnt+1] = imDisp[(((H) - i) * (W)) + cnt + 1] - searchWidth;
    	  nextRowEstDispariyCenters3[cnt+1] = imDisp[(((H) - i) * (W)) + cnt + 2] - searchWidth;

      }

      //Calculate Bottom-1 to 0 Line Disparity
      for (i=H-1-halfWinSize-1;i>=halfWinSize;i--)
      {

        cntJ = 1;

        realIstr=i-halfWinSize;
        realIend=i-halfWinSize+winSize;
        //================================================
        funcLoopJQ_L(imL, imR, H, W, imDisp, &parameter, i, &cntJ);
        //================================================

        //Calculate new Disparity Center
        for (cnt=0;cnt<W-1;cnt++)
        {
          nextRowEstDispariyCenters1[cnt+1] = imDisp[(((H) - i) * (W)) + cnt] - searchWidth;
          nextRowEstDispariyCenters2[cnt+1] = imDisp[(((H) - i) * (W)) + cnt + 1] - searchWidth;
          nextRowEstDispariyCenters3[cnt+1] = imDisp[(((H) - i) * (W)) + cnt + 2] - searchWidth;
        }

      }// End for i
    }
    else
    {//Calculate disparity with the left image as the reference
      jMin = winx;
      jMax = W-winx-maxSearchRange-1;

      //Calculate Bottom Line Disparity
      i=H-1-halfWinSize;
      cntJ = 1;
      realIstr=i-halfWinSize;
      realIend=i-halfWinSize+winSize;
      funcLoopJ_R(imL, imR, H, W,imDisp, &parameter, i, &cntJ, &cntI);

      //Calculate new Disparity Center
      for (cnt=0;cnt<W-1;cnt++)
      {
        //nextRowEstDispariyCenters1[cnt+1] = imDisp->at<uchar>(i,cnt) - searchWidth;
        //nextRowEstDispariyCenters2[cnt+1] = imDisp->at<uchar>(i,cnt+1) - searchWidth;
        //nextRowEstDispariyCenters3[cnt+1] = imDisp->at<uchar>(i,cnt+2) - searchWidth;
    	  nextRowEstDispariyCenters1[cnt+1] = imDisp[(((H) - i) * (W)) + cnt] - searchWidth;
    	  nextRowEstDispariyCenters2[cnt+1] = imDisp[(((H) - i) * (W)) + cnt + 1] - searchWidth;
    	  nextRowEstDispariyCenters3[cnt+1] = imDisp[(((H) - i) * (W)) + cnt + 2] - searchWidth;
      }

      //Calculate Bottom-1 to 0 Line Disparity
      for (i=H-1-halfWinSize-1;i>=halfWinSize;i--)
      {

        cntJ = 1;

        realIstr=i-halfWinSize;
        realIend=i-halfWinSize+winSize;
        //================================================
        funcLoopJQ_R(imL, imR, H, W, imDisp, &parameter, i, &cntJ);
        //================================================

        //Calculate new Disparity Center
        for (cnt=0;cnt<W-1;cnt++)
        {
          //nextRowEstDispariyCenters1[cnt+1] = imDisp->at<uchar>(i,cnt) - searchWidth;
          //nextRowEstDispariyCenters2[cnt+1] = imDisp->at<uchar>(i,cnt+1) - searchWidth;
          //nextRowEstDispariyCenters3[cnt+1] = imDisp->at<uchar>(i,cnt+2) - searchWidth;
        	 nextRowEstDispariyCenters1[cnt+1] = imDisp[(((H) - i) * (W)) + cnt] - searchWidth;
        	 nextRowEstDispariyCenters2[cnt+1] = imDisp[(((H) - i) * (W)) + cnt + 1] - searchWidth;
        	 nextRowEstDispariyCenters3[cnt+1] = imDisp[(((H) - i) * (W)) + cnt + 2] - searchWidth;
        }

      }// End for i
    }

    delete [] nextRowEstDispariyCenters1;
    delete [] nextRowEstDispariyCenters2;
    delete [] nextRowEstDispariyCenters3;

}

//Sub functions
void funcLoopJQ_L(const unsigned char* imL, const unsigned char* imR, short H, short W, unsigned char* imDisp,struct Parameters* params, short i, short* cntJ)
{
  short j,d = 0;
  short realJstr=0,realJend=0;
  //double BlockR_SbtEndCol = 0;
  double corrScore,prevcorrScore;
  short bestMatchSoFar;
  //int cntD = 1;
  short jMin = *(params->jMin);
  short jMax = *(params->jMax);
  //int shiftMin = *(params->shiftMin);
  short shiftMax = *(params->shiftMax);
  short winx = params->winx;

  short nOelement=0;
  short uniqueRange[15]={0};

  double BlockValRef = 0.0;
  double BlockValRef1stCol = 0.0;

  double TempTests = 0;

  params->realJStr = &realJstr;
  params->realJEnd = &realJend;
  params->d = &d;

  for (j=jMin;j<=jMax;j++)
  {

    nOelement=0;
    funcUniqRange((params->nRDCpt1+j), (params->nRDCpt2+j), (params->nRDCpt3+j), uniqueRange, &nOelement, shiftMax, (short)*(params->searchRange) );

    prevcorrScore = 0.0;
    bestMatchSoFar = 0;
    realJstr=j-winx;
    realJend=j-winx+winx+winx+1;

    //Calculate sum(L^2)
    if (*(cntJ) == 1)
    {
      funcShiftSquare(imL, imL, H, W, params, &BlockValRef, &TempTests, &BlockValRef1stCol, cntJ);
    }
    else
    {
      funcShiftSquare(imL, imL, H, W, params, &BlockValRef, &TempTests, &BlockValRef1stCol);
    }


    //cntD = 1;

    short cntWhile=0;

    while (cntWhile<nOelement)
    {
      d=-uniqueRange[cntWhile];

      funcNccCoreFull(imL, imR, H, W, params, &BlockValRef, &corrScore);

      if ( corrScore > prevcorrScore )
      {

        prevcorrScore = corrScore;
        //bestMatchSoFar = (uchar)-d;
        bestMatchSoFar = (int)-d;

      }

      cntWhile++;

    }

    //imDisp->at<uchar>(i,j) = bestMatchSoFar;
    imDisp[(((H) - i) * (W)) + j] = bestMatchSoFar;



  }// End for j

}

void funcLoopJQ_R(const unsigned char* imL, const unsigned char* imR, short H, short W, unsigned char* imDisp,struct Parameters* params, short i, short* cntJ)
{
  short j,d = 0;
  short realJstr=0,realJend=0;

  double corrScore,prevcorrScore;
  short bestMatchSoFar;

  short jMin = *(params->jMin);
  short jMax = *(params->jMax);
  //int shiftMin = *(params->shiftMin);
  short shiftMax = *(params->shiftMax);
  short winx = params->winx;

  short nOelement=0;
  short uniqueRange[15]={0};

  double TempTests = 0;

  double BlockValRef = 0.0;
  double BlockValRef1stCol = 0.0;

  params->realJStr = &realJstr;
  params->realJEnd = &realJend;
  params->d = &d;

  for (j=jMin;j<=jMax;j++)
  {

    nOelement=0;
    funcUniqRange((params->nRDCpt1+j), (params->nRDCpt2+j), (params->nRDCpt3+j), uniqueRange, &nOelement, shiftMax, (short)*(params->searchRange) );

    prevcorrScore = 0.0;
    bestMatchSoFar = 0;
    realJstr=j-winx;
    realJend=j+winx+1;

    //Calculate sum(L^2)
    if (*(cntJ) == 1)
    {
      funcShiftSquare(imR, imR, H, W, params, &BlockValRef, &TempTests, &BlockValRef1stCol, cntJ);
    }
    else
    {
      funcShiftSquare(imR, imR, H, W, params, &BlockValRef, &TempTests, &BlockValRef1stCol);
    }


    //cntD = 1;

    short cntWhile=0;

    while (cntWhile<nOelement)
    {
      d=uniqueRange[cntWhile];

      funcNccCoreFull(imR, imL, H, W, params, &BlockValRef, &corrScore);

      if ( corrScore > prevcorrScore )
      {

        prevcorrScore = corrScore;
        //bestMatchSoFar = (uchar)d;
        bestMatchSoFar = (int)d;

      }

      cntWhile++;

    }

    //imDisp->at<uchar>(i,j) = bestMatchSoFar;
    imDisp[(((H) - i) * (W)) + j] = bestMatchSoFar;


  }// End for j

}

void funcUniqRange(short* inPt1, short* inPt2, short* inPt3, short* uniqueRange, short* nOelement, short shiftMax, short searchRange)
{
  short loopRange = (short)searchRange;
  short rangePoints[15]={0};
  short uniqueRangeTemp[15]={0};
  short tempSort[3]={0};
  short arryLength = (loopRange+1)*3;
  short maxVal = 0;
  short minVal = 32767;
  short cnt = 0;
  short cnt1 = 1;
  short cnt2 = 0;
  short saveCnt = 0;
  short arryStr = 0;

  if (loopRange>10)
  {
    cout<<"searchRange Cannot be greater than 10."<<endl;
  }

  tempSort[0] = *(inPt1);
  tempSort[1] = *(inPt2);
  tempSort[2] = *(inPt3);

  for (cnt=0;cnt<loopRange+1;cnt++)
  {
    rangePoints[cnt] = tempSort[0]+ cnt;
    rangePoints[cnt+loopRange+1] = tempSort[1]+ cnt;
    rangePoints[cnt+loopRange+loopRange+2] = tempSort[2]+ cnt;
  }
/*
  for (int izz=0;izz<30;izz++)
  {
    cout<<" "<<rangePoints[izz];
  }
  cout<<endl;
*/

/*
  short passCnt = 0;
  short temp = 0;
  cnt1=1;
  //Bubble Sort
  while (cnt1>0)
  {
    passCnt = 0;
    for (int cnt2=0;cnt2<arryLength-1;cnt2++)
    {
      if (rangePoints[cnt2] > rangePoints[cnt2+1])
      {
        temp = rangePoints[cnt2+1];
        rangePoints[cnt2+1] = rangePoints[cnt2];
        rangePoints[cnt2] = temp;
        passCnt++;
        //totalCnt++;
      }
    }
    cnt1 = passCnt;
  }
  //Fin Bubble Sort
*/

  quicksort( rangePoints, 0, arryLength-1);

  minVal = rangePoints[0];
  if (minVal<2)
  {
    minVal = 2;
  }
  maxVal = rangePoints[arryLength-1];
  if (maxVal>shiftMax)
  {
    maxVal = shiftMax;
  }

  cnt1 = minVal;
  while (cnt1<=maxVal)
  {
    for (cnt2=arryStr;cnt2<arryLength;cnt2++)
    {
      if (rangePoints[cnt2] == cnt1)
      {
        uniqueRangeTemp[saveCnt] = cnt1;
        saveCnt++;
        arryStr = cnt2+1;
        cnt2=arryLength;
      }
    }
    cnt1++;
  }

  *(nOelement) = saveCnt;

  for (cnt=0;cnt<saveCnt+1;cnt++)
  {
    uniqueRange[cnt] = uniqueRangeTemp[cnt];
  }

  //*************Fin Find Unique search range
}

void quicksort(short* a,short l,short h)
{
  if (l>=h)
  {
    return ;
  }

  short j,i,key;

  i=l;
  j=h;
  key=a[i];

  while(i<j)
  {
    while ( i<j && a[j]>key )
    {
      j--;
    }
    if (i<j)
    {
      a[i++]=a[j];
    }
    while ( i<j && a[i]<key )
    {
      i++;
    }
    if (i<j)
    {
      a[j--]=a[i];
    }
  }
  a[i]=key;

  if ( l < i-1 )
  {
    quicksort(a,l,i-1);
  }
  if ( i+1 < h )
  {
    quicksort(a,i+1,h);
  }

}

void funcNccCoreFull(const unsigned char* inputI1, const unsigned char* inputI2, short H, short W,struct Parameters* params, double* BlockRefVal, double* outCorrScore)
{
  int realJ=0,realI=0;
  int d=*(params->d);

  int realIstr = *(params->realIStr);
  int realIend = *(params->realIEnd);
  int realJstr = *(params->realJStr);
  int realJend = *(params->realJEnd);

  double BlockValue1_New = *(BlockRefVal);
  double BlockValue2_New = 0;
  double BlockValue3_New = 0;

  //BlockValue 1st Col to End-1 Col
  for (realJ=realJstr;realJ<realJend;realJ++)
  {
    for (realI=realIstr;realI<realIend;realI++)
    {
      //BlockValue2_New += inputI2->at<uchar>(realI,realJ+d) * inputI2->at<uchar>(realI,realJ+d);
      //BlockValue3_New += inputI1->at<uchar>(realI,realJ)   * inputI2->at<uchar>(realI,realJ+d);
    	BlockValue2_New += inputI2[(((H) - realI) * (W)) + realJ + d] * inputI2[(((H) - realI) * (W)) + realJ + d];
    	BlockValue3_New += inputI2[(((H) - realI) * (W)) + realJ] * inputI2[(((H) - realI) * (W)) + realJ];
    }
  }

  *(outCorrScore) = BlockValue3_New / ( sqrt( BlockValue1_New ) * sqrt ( BlockValue2_New ) );

}

