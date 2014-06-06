//
// Copyright 2014 Nicolas Dumay
//
// Licensed under the Apache License, Version 2.0 (the "Apache License")
// with the following modification; you may not use this file except in
// compliance with the Apache License and the following modification to it:
// Section 6. Trademarks. is deleted and replaced with:
//
// 6. Trademarks. This License does not grant permission to use the trade
//    names, trademarks, service marks, or product names of the Licensor
//    and its affiliates, except as required to comply with Section 4(c) of
//    the License and to reproduce the content of the NOTICE file.
//
// You may obtain a copy of the Apache License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the Apache License with the above modification is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the Apache License for the specific
// language governing permissions and limitations.

#ifndef UTILS_H_
#define UTILS_H_

#include <ai.h>

#include "DDImage/Iop.h"

void mCpMat(DD::Image::Matrix4 & tmpMat,AtMatrix & mTmpMat) {
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
}

#endif /* UTILS_H_ */
