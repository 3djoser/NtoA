#ifndef UTILS_H_
#define UTILS_H_

#include <ai.h>

#include "DDImage/Iop.h"

void mCpMat(DD::Image::Matrix4 & tmpMat,AtMatrix & mTmpMat)
{
	mTmpMat[0][0] = tmpMat.a00;
	mTmpMat[1][0] = tmpMat.a01;
	mTmpMat[2][0] = tmpMat.a02;
	mTmpMat[3][0] = tmpMat.a03;
	mTmpMat[0][1] = tmpMat.a10;
	mTmpMat[1][1] = tmpMat.a11;
	mTmpMat[2][1] = tmpMat.a12;
	mTmpMat[3][1] = tmpMat.a13;
	mTmpMat[0][2] = tmpMat.a20;
	mTmpMat[1][2] = tmpMat.a21;
	mTmpMat[2][2] = tmpMat.a22;
	mTmpMat[3][2] = tmpMat.a23;
	mTmpMat[0][3] = tmpMat.a30;
	mTmpMat[1][3] = tmpMat.a31;
	mTmpMat[2][3] = tmpMat.a32;
	mTmpMat[3][3] = tmpMat.a33;
};

#endif /* UTILS_H_ */
