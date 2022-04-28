/*
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/ 
 * 
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions 
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the   
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ti/sysbios/BIOS.h> 
#include <ti/sysbios/hal/Cache.h>

#include <math.h>
#include "ti/platform/platform.h"
#include "../inc/mcip_core.h"
#include "imglib.h"
#include "normalisedCrossCorrelation.h"

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
        //bestMatchSoFar = (int)-d;
        bestMatchSoFar = (unsigned char)-d;

      }

    }

    imDisp[((i) * (W)) + j] = bestMatchSoFar;

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
        //bestMatchSoFar = (int)-d;
        bestMatchSoFar = (unsigned char)-d;

      }

    }

    imDisp[((i) * (W)) + j] = bestMatchSoFar;

  }

}

void funcLoopJ_R(const unsigned char* imL, const unsigned char* imR, short H, short W, unsigned char* imDisp,struct Parameters* params, short i, short *loopInd1, short *loopInd2)
{
  short j,d = 0;
  short realJstr=0,realJend=0;
  double BlockR_SbtEndCol = 0;
  double corrScore,prevcorrScore;
  double output[100];
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
    BlockValRef = 29311;
    TempTests = 23737;
    BlockValRef1stCol = 2982;
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

	  //if ((i==367) && (j == 92))
      //{
      //	output[d] = corrScore;
	//	if (d == 100)
	//	{
	//		output[0] = corrScore;
	//	}
     // }
      if ( corrScore > prevcorrScore )
      {

        prevcorrScore = corrScore;
        bestMatchSoFar = (unsigned char)d;
        //cout<<" "<<(int)d<<"="<<(int)bestMatchSoFar;

      }

    }

    //cout<<" "<<(int)bestMatchSoFar;
    //imDisp->at<uchar>(i,j) = bestMatchSoFar;
    //if ((i==367) && (j < 100))
    //{
    //	output[j] = bestMatchSoFar;
    //}
    //if ((i==367) && (j == 100))
    //{
    //	output[0] = bestMatchSoFar;
    //}
    imDisp[((i) * (W)) + j] = bestMatchSoFar;

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
        bestMatchSoFar = (unsigned char)d;
        //cout<<" "<<(int)d<<"="<<(int)bestMatchSoFar;

      }

    }

    //cout<<" "<<(int)bestMatchSoFar;
    imDisp[((i) * (W)) + j] = bestMatchSoFar;

  }
  //cout<<"T_T"<<endl;

}

void funcShiftSquare(const unsigned char* imL, const unsigned char* imR, short H, short W,struct Parameters* params, double* BlockValue, double* BlockValue_SbtTop, double* Block1stCol, short* loopInd)
{
	unsigned char temp1,temp2,temp3,temp4;


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
  doubleBlockTopLeft = imL[(realI * (W)) + realJ] * imR[(realI * (W)) + realJ];
  temp1 =  imL[((realI+1) * (W)) + realJ + 1];
  	temp2 = imL[(realI * (W)) + realJ + 1];
  	temp3 = imL[((realI+1) * (W)) + realJ];
  	temp4 = imL[(realI * (W)) + realJ];
  //Calculate the first Row of the block
  realI = realIstr;
  doubleBlockTopRow = doubleBlockTopLeft;
  for (realJ=realJstr+1;realJ<realJend;realJ++)
  {
    //doubleBlockTopRow += inputI1->at<uchar>(realI,realJ) * inputI2->at<uchar>(realI,realJ);
	  doubleBlockTopRow += imL[(realI * (W)) + realJ] * imR[(realI * (W)) + realJ];
  }

  //Calculate the first Col (from 2nd Row) of the block
  realJ = realJstr;
  for (realI=realIstr+1;realI<realIend;realI++)
  {
    //doubleBlock_SbtTop += inputI1->at<uchar>(realI,realJ) * inputI2->at<uchar>(realI,realJ);
	  doubleBlock_SbtTop += imL[(realI * (W)) + realJ] * imR[(realI * (W)) + realJ];
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
    	doubleBlock_SbtTop += imL[(realI * (W)) + realJ] * imR[(realI * (W)) + realJ];
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
  doubleBlockCornerL[0]= imL[(realI * (W)) + realJ] * imR[(realI * (W)) + realJ];
  //Top-Right
  realJ = realJend-1;
  //doubleBlockCornerL[1]= inputI1->at<uchar>(realI,realJ) * inputI2->at<uchar>(realI,realJ);
  doubleBlockCornerL[1]= imL[(realI * (W)) + realJ] * imR[(realI * (W)) + realJ];
  //Bottom-Right
  realI = realIend-1;
  //doubleBlockCornerL[3]= inputI1->at<uchar>(realI,realJ) * inputI2->at<uchar>(realI,realJ);
  doubleBlockCornerL[3]= imL[(realI * (W)) + realJ] * imR[(realI * (W)) + realJ];
  //Bottom-Left
  realJ = realJstr;
  //doubleBlockCornerL[2]= inputI1->at<uchar>(realI,realJ) * inputI2->at<uchar>(realI,realJ);
  doubleBlockCornerL[2]= imL[(realI * (W)) + realJ] * imR[(realI * (W)) + realJ];


  //Calculate the 1st Col (from 2nd Row to End-1 Row) of the block
  realJ = realJstr;
  for (realI=realIstr+1;realI<realIend-1;realI++)
  {
    //doubleBlock1stCol += inputI1->at<uchar>(realI,realJ) * inputI2->at<uchar>(realI,realJ);
	  doubleBlock1stCol += imL[(realI * (W)) + realJ] * imR[(realI * (W)) + realJ];
  }

  //Total Value of 1st Col (+TL&BL)
  doubleBlock1stCol += doubleBlockCornerL[0] + doubleBlockCornerL[2];

  //Calculate the End Col (from 2nd Row to End-1 Row) of the block
  realJ = realJend-1;
  for (realI=realIstr+1;realI<realIend-1;realI++)
  {
    //doubleBlockEndCol += inputI1->at<uchar>(realI,realJ) * inputI2->at<uchar>(realI,realJ);
	  doubleBlockEndCol += imL[(realI * (W)) + realJ] * imR[(realI * (W)) + realJ];
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
    doubleBlockTopRow += imL[(realI * (W)) + realJ] * imR[(realI * (W)) + realJ];
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
    	BlockValue2_New += imR[(realI * (W)) + realJ + d] * imR[(realI * (W)) + realJ + d];
    	BlockValue3_New += imL[(realI * (W)) + realJ] * imR[(realI * (W)) + realJ + d];
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
	  BlockValue2_New += imR[(realI * (W)) + realJ + d] * imR[(realI * (W)) + realJ + d];
	  BlockValue3_New += imL[(realI * (W)) + realJ] * imR[(realI * (W)) + realJ + d];
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
    	BlockValue3_New += imL[(realI * (W)) + realJ] * imR[(realI * (W)) + realJ + d];
    }
  }

  //BlockValue 1st Col
  realJ = realJstr;
  for (realI=realIstr;realI<realIend;realI++)
  {
    //BlockValue2_1stCol  += inputI2->at<uchar>(realI,realJ+d) * inputI2->at<uchar>(realI,realJ+d);
	  BlockValue2_1stCol  += imR[(realI * (W)) + realJ + d] * imR[(realI * (W)) + realJ + d];
  }

  //BlockValue End Col
  realJ = realJend-1;
  for (realI=realIstr;realI<realIend;realI++)
  {
    //BlockValue2_EndCol += inputI2->at<uchar>(realI,realJ+d) * inputI2->at<uchar>(realI,realJ+d);
	  BlockValue2_EndCol += imR[(realI * (W)) + realJ + d] * imR[(realI * (W)) + realJ + d];
  }

  BlockValue2_New = *(BlockValue_SbtEndCol) + BlockValue2_1stCol;

  *(BlockValue_SbtEndCol) = BlockValue2_New - BlockValue2_EndCol;

  *(outCorrScore) = BlockValue3_New / ( sqrt( BlockValue1_New ) * sqrt( BlockValue2_New ) );

}

void funcNccCore_R(const unsigned char* imL, const unsigned char* imR, short H, short W,struct Parameters* params, double* BlockValue_Sbt1stCol, double* BlockRefVal, double* outCorrScore, short *cntD)
{
	unsigned char temp1,temp2,temp3,temp4;
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
    	if (d == 33)
    		{
    			temp1 = imR[(realI * (W)) + realJ + d];
        		temp2 = imR[(realI * (W)) + realJ + d];
        		temp3 = imL[(realI * (W)) + realJ];
        		temp4 = imR[(realI * (W)) + realJ + d];
    		}
      //BlockValue2_New += inputI2->at<uchar>(realI,realJ+d) * inputI2->at<uchar>(realI,realJ+d);
      //BlockValue3_New += inputI1->at<uchar>(realI,realJ)   * inputI2->at<uchar>(realI,realJ+d);
    	BlockValue2_New += imR[(realI * (W)) + realJ + d] * imR[(realI * (W)) + realJ + d];
    	BlockValue3_New += imL[(realI * (W)) + realJ] * imR[(realI * (W)) + realJ + d];
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
	  BlockValue2_New += imR[(realI * (W)) + realJ + d] * imR[(realI * (W)) + realJ + d];
	  BlockValue3_New += imL[(realI * (W)) + realJ] * imR[(realI * (W)) + realJ + d];
  }

  *(outCorrScore) = BlockValue3_New / ( sqrt( BlockValue1_New ) * sqrt( BlockValue2_New ) );

  *cntD = 2;

}

void funcNccCore_R2(const unsigned char* imL, const unsigned char* imR, short H, short W,struct Parameters* params, double* BlockValue_Sbt1stCol, double* BlockRefVal,  double* outCorrScore)
{
  short realJ=0,realI=0;
  short d=*(params->d);
  unsigned char temp1,temp2,temp3,temp4;
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
    	if (d == 33)
    		{
			temp1 = imR[(realI * (W)) + realJ + d];
    		temp2 = imR[(realI * (W)) + realJ + d];
    		temp3 = imL[(realI * (W)) + realJ];
    		temp4 = imR[(realI * (W)) + realJ + d];
    		}
    	BlockValue3_New += imL[(realI * (W)) + realJ] * imR[(realI * (W)) + realJ + d];

    }
  }

  //BlockValue 1st Col
  realJ = realJstr;
  for (realI=realIstr;realI<realIend;realI++)
  {
  	if (d == 33)
  		{
			temp1 = imR[(realI * (W)) + realJ + d];
  		temp2 = imR[(realI * (W)) + realJ + d];
  		temp3 = imL[(realI * (W)) + realJ];
  		temp4 = imR[(realI * (W)) + realJ + d];
  		}
    //BlockValue2_1stCol  += inputI2->at<uchar>(realI,realJ+d) * inputI2->at<uchar>(realI,realJ+d);
	  BlockValue2_1stCol  += imR[(realI * (W)) + realJ + d] * imR[(realI * (W)) + realJ + d];
  }

  //BlockValue End Col
  realJ = realJend-1;
  for (realI=realIstr;realI<realIend;realI++)
  {
    //BlockValue2_EndCol += inputI2->at<uchar>(realI,realJ+d) * inputI2->at<uchar>(realI,realJ+d);
	  BlockValue2_EndCol += imR[(realI * (W)) + realJ + d] * imR[(realI * (W)) + realJ + d];
  }

  BlockValue2_New = *(BlockValue_Sbt1stCol) + BlockValue2_EndCol;

  *(BlockValue_Sbt1stCol) = BlockValue2_New - BlockValue2_1stCol;

  *(outCorrScore) = BlockValue3_New / ( sqrt( BlockValue1_New ) * sqrt( BlockValue2_New ) );

}

void split_image(const unsigned char* in, unsigned char* imL, unsigned char* imR, short H, short W)
{
	int i;
	int crop_number = (W * ((H/2)-1));
	for(i = 0; i <= ((H/2)*W); i++)
	{
		imL[i] = in[i];
		imR[i] = in[i + crop_number];
	}
	//for(i =H*W; i < H*W*2 ; i++)
	//{
	//	imR[i - (H*W)] = in[i];
	//}
}

void nccShortRange(const unsigned char* imL, const unsigned char* imR, short H, short W, int winSize, int maxSearchRange, unsigned char* imDisp)
{//Main function to run for our proposed algorithm
    short i=0;
    int refl = 1;
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

    //short* nextRowEstDispariyCenters1;
    //short* nextRowEstDispariyCenters2;
    //short* nextRowEstDispariyCenters3;

    //nextRowEstDispariyCenters1 = new short [W];
    //nextRowEstDispariyCenters2 = new short [W];
    //nextRowEstDispariyCenters3 = new short [W];
    short	nextRowEstDispariyCenters1[1280];
    short	nextRowEstDispariyCenters2[1280];
    short	nextRowEstDispariyCenters3[1280];

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
    //searchWidth = searchRange*0.5;
    searchWidth = floor((searchRange*0.5) + 0.5);

    cntI=1;

    //if (!refL)
    if (refl == 0)
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
    	  nextRowEstDispariyCenters1[cnt+1] = imDisp[((i) * (W)) + cnt] - searchWidth;
    	  nextRowEstDispariyCenters2[cnt+1] = imDisp[((i) * (W)) + cnt + 1] - searchWidth;
    	  nextRowEstDispariyCenters3[cnt+1] = imDisp[((i) * (W)) + cnt + 2] - searchWidth;

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
          nextRowEstDispariyCenters1[cnt+1] = imDisp[((i) * (W)) + cnt] - searchWidth;
          nextRowEstDispariyCenters2[cnt+1] = imDisp[((i) * (W)) + cnt + 1] - searchWidth;
          nextRowEstDispariyCenters3[cnt+1] = imDisp[((i) * (W)) + cnt + 2] - searchWidth;
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
    	  nextRowEstDispariyCenters1[cnt+1] = imDisp[((i) * (W)) + cnt] - searchWidth;
    	  nextRowEstDispariyCenters2[cnt+1] = imDisp[((i) * (W)) + cnt + 1] - searchWidth;
    	  nextRowEstDispariyCenters3[cnt+1] = imDisp[((i) * (W)) + cnt + 2] - searchWidth;
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
        	 nextRowEstDispariyCenters1[cnt+1] = imDisp[((i) * (W)) + cnt] - searchWidth;
        	 nextRowEstDispariyCenters2[cnt+1] = imDisp[((i) * (W)) + cnt + 1] - searchWidth;
        	 nextRowEstDispariyCenters3[cnt+1] = imDisp[((i) * (W)) + cnt + 2] - searchWidth;
        }

      }// End for i
    }

    //delete [] nextRowEstDispariyCenters1;
    //delete [] nextRowEstDispariyCenters2;
    //delete [] nextRowEstDispariyCenters3;

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
      funcShiftSquare2(imL, imL, H, W, params, &BlockValRef, &TempTests, &BlockValRef1stCol);
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
        //bestMatchSoFar = (int)-d;
        bestMatchSoFar = (unsigned char)-d;
      }

      cntWhile++;

    }

    //imDisp->at<uchar>(i,j) = bestMatchSoFar;
    imDisp[((i) * (W)) + j] = bestMatchSoFar;



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
      funcShiftSquare2(imR, imR, H, W, params, &BlockValRef, &TempTests, &BlockValRef1stCol);
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
        //bestMatchSoFar = (int)d;
        bestMatchSoFar = (unsigned char)d;

      }

      cntWhile++;

    }

    //imDisp->at<uchar>(i,j) = bestMatchSoFar;
    imDisp[((i) * (W)) + j] = bestMatchSoFar;


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

  //if (loopRange>10)
  //{
   // cout<<"searchRange Cannot be greater than 10."<<endl;
  //}

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
    	BlockValue2_New += inputI2[(realI * (W)) + realJ + d] * inputI2[(realI * (W)) + realJ + d];
    	BlockValue3_New += inputI2[(realI * (W)) + realJ] * inputI2[(realI * (W)) + realJ];
    }
  }

  *(outCorrScore) = BlockValue3_New / ( sqrt( BlockValue1_New ) * sqrt ( BlockValue2_New ) );

}


/* This value is used as threshold on the Sobel output to get edge data. */
#define IMAGE_THRESHOLD_VALUE (0xfe)

/*********************************************************************************/
/* RGB to Y image convert routine                                                */
/* The conversion method used is Y = 0.299R + 0.587G + 0.114B                    */
/* (ITU-R Recommendation BT. 601-4)                                              */           
/*********************************************************************************/
static int convert_rgb_to_y (uint16_t bits_per_pixel,
		                  color_table_t * p_color_table,
						  uint8_t * pixel_array_rgb,
						  uint8_t * pixel_array_y,
						  uint32_t width, uint32_t height)
{
	int i;
	int pixel_size = bits_per_pixel / 8;

	if(pixel_size == 1) {
        if (p_color_table) {
    		for(i = 0; i < (width * height); i++) {
    			if (i > (width * height * 0.7))
    			{
    				pixel_array_y[i] = 1;
    			}
    			else
    			{
					pixel_array_y[i] = (uint8_t)(((double)p_color_table[pixel_array_rgb[i]].blue * 0.114)
											   + ((double)p_color_table[pixel_array_rgb[i]].green * 0.587)
    										   + ((double)p_color_table[pixel_array_rgb[i]].red * 0.299));
    			}
    		}
        } else {
    		printf("BPP 8 must have color table\n");
    		return -1;
        }
	} else if(pixel_size == 3) {
		for(i = 0; i < (width * height); i++) {

				pixel_array_y[i] = (uint8_t)(((double)pixel_array_rgb[3 * i] * 0.114)       /* Blue */
										   + ((double)pixel_array_rgb[3 * i + 1] * 0.587)   /* Green */
										   + ((double)pixel_array_rgb[3 * i + 2] * 0.299)); /* Red */


		}
	} else {
		printf("BPP %d not supported\n", bits_per_pixel);
		return -1;
	}

	return 0;
}



/*********************************************************************************/
/* Process RGB image                                                             */
/*********************************************************************************/
void process_rgb (processing_info_t * p_info, int count, int number_of_slices, int lanes[20], int * vanishing_points)
{
	int num_iter = 3;
	int cur_iter;
	float maxzoomdegree;
	int vpx = 233;//p_info->width / 2;
	//vpx = 233;
	int vpy = p_info->height * (count +1);
	uint8_t * y = 0;
	uint8_t * y2 = 0;
	float pd = 0, id = 0;
	int * new_height;
	int * new_width;
	//int lanes[10];
	int i;
	//int * accumulator = 0;
	uint8_t * sobel = 0;
	for(i = 0; i < 20; i++)
	{
		lanes[i] = 0;
	}

    if (platform_get_coreid() != 0) {
        Cache_inv(p_info->rgb_in,
            (p_info->height * p_info->width * (p_info->bitspp / 8)), Cache_Type_ALL, FALSE);
        if (p_info->p_color_table) {
            Cache_inv(p_info->p_color_table, (sizeof(color_table_t) * 256), Cache_Type_ALL, FALSE);
        }
    }

    p_info->flag = -1;

    if(p_info->processing_type != edge_detection) {
        printf("Unsupported processing type %d\n", p_info->processing_type);
		goto close_n_exit;
    }

    if ((p_info->bitspp == 8) && (!p_info->p_color_table)) {
        y = p_info->rgb_in;
    } else {
    	if ((!p_info->scratch_buf_len[1]) || 
            ((p_info->width * p_info->height) > p_info->scratch_buf_len[1])) {
    		printf("process_rgb: Scratch buffer for Y not available: 0x%x\n",
                p_info->scratch_buf_len[1]);
    		goto close_n_exit;
    	}

        y = p_info->scratch_buf[1];
    	if (convert_rgb_to_y (p_info->bitspp, p_info->p_color_table,
    							  p_info->rgb_in, y,
    							  p_info->width, p_info->height) < 0) {
    		printf("Error in converting RGB to Y\n");
    		goto close_n_exit;
    	}
    }

	/* Run Sobel Filter */
	if ((!p_info->scratch_buf_len[0]) || 
            ((p_info->width * p_info->height) > p_info->scratch_buf_len[0])) {
		printf("Can't allocate memory for sobel\n");
		goto close_n_exit;
	}
    sobel = p_info->scratch_buf[0];
    y2 = p_info->scratch_buf[2];
    //grad = p_info->scratch_buf[2];
    //sobel2 = p_info->scratch_buf[3];

    //IMG_sobel_grad(y, count, p_info->width, p_info->height);
    //interceptfinder(grad, p_info->width, p_info->height, count);

    ///for(cur_iter = 0; cur_iter < 3; cur_iter++)
    ///{
    ///	if (cur_iter == 0)
    ///	{

    		//vpx = IMG_sobel_grad(y, sobel, count, p_info->width, p_info->height);
    		//vpx = vp_detect_2d(y, sobel, count, p_info->width, p_info->height);
    		//IMG_sobel_3x3_8(y, sobel, p_info->width, p_info->height);

    		//median_filter(sobel, y2, p_info->width, p_info->height);

    		//bilateral_filter(sobel, y2, 2, id, pd, p_info->width, p_info->height);
    		//IMG_thr_le2min_8(sobel, p_info->out,
    		//   						p_info->width, p_info->height, IMAGE_THRESHOLD_VALUE);
    ///		optimised_median_filter(y2, p_info->out, p_info->width, p_info->height);
    		//vpx = IMG_thresh_grad(sobel, p_info->out, count, p_info->width, p_info->height);
    ///	}
    ///	else
    ///	{
    ///		maxzoomdegree = 0.9;
    ///		select_area(vpx, vpy, num_iter, maxzoomdegree, cur_iter, p_info->height, p_info->width, &new_width, &new_height, y, y2);

    		//interpolate (y2, sobel, p_info->width, p_info->height, &new_width, &new_height);
    ///		choose_pixel(y2, sobel, p_info->width, p_info->height, &new_width, &new_height);
    		//unified_interpolate (y2, sobel, p_info->width, p_info->height, &new_width, &new_height, &V_start, &H_start);
    ///		IMG_sobel_3x3_8(sobel, y2, p_info->width, p_info->height);
    		//IMG_thr_le2min_8(sobel2, info_out2,
    		//				p_info->width, p_info->height, IMAGE_THRESHOLD_VALUE);

    ///		IMG_thr_le2min_8(y2, sobel,
    ///		  						p_info->width, p_info->height, IMAGE_THRESHOLD_VALUE);
    		//median_filter(sobel, y, p_info->width, p_info->height);
    		//IMG_median_3x3_16(y, p_info->width, sobel);

    ///		compare_iter(p_info->out, sobel, p_info->height, p_info->width);

    ///	}

    ///}

    //lane_tracking(lanes, p_info->out, number_of_slices, vpx, vpy, p_info->width, p_info->height);
    * vanishing_points = vpx;
    //p_info->out = y2;

    //IMG_sobel_grad(y, sobel, grad, p_info->width, p_info->height);

      //  		IMG_thr_le2min_8(sobel, p_info->out,
       // 						p_info->width, p_info->height, IMAGE_THRESHOLD_VALUE);
	/* Run Threshold Function */


	// new code written by me



	// new code written by me*/
    split_image(y, y2, sobel, p_info->height, p_info->width);
    unsigned char testL[100];
    	unsigned char testR[100];
    	for (i=0; i <100; i++)
    	{
    		testL[i] = y2[((i) * ( p_info->width)) + 0];
    	}
    		for (i=0; i <100; i++)
    	{
    		testR[i] = sobel[((i) * ( p_info->width)) + 0];
    	}
    //for (i = 0; i < ((p_info->height * p_info->width)); i++)
    //{
    //	p_info->out[i] = sobel[i];
    //}
    nccShortRange(y2, sobel, ((p_info->height) / 2), p_info->width, 5, 100, p_info->out);
	p_info->flag = 0;

    if (platform_get_coreid() != 0) {
        Cache_wb(p_info->out, (p_info->height * p_info->width), Cache_Type_ALL, FALSE);
    }

    if (platform_get_coreid() != 0) {
        Cache_inv(p_info->scratch_buf[0], p_info->scratch_buf_len[0], Cache_Type_ALL, FALSE);
        if (p_info->scratch_buf[1]) {
            Cache_inv(p_info->scratch_buf[1], p_info->scratch_buf_len[1], Cache_Type_ALL, FALSE); 
        }
        Cache_inv(p_info, sizeof(processing_info_t), Cache_Type_ALL, FALSE);
    }

close_n_exit:
    return;
}

