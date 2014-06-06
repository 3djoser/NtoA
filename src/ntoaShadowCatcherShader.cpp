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

#include <stdio.h>
#include <string.h>

#include <ai.h>

#include "DDImage/Iop.h"

AI_SHADER_NODE_EXPORT_METHODS(fb_ShadowMtd);

enum fb_ShadowShaderParams {
	p_color,p_diffuse_weight
};

node_parameters {
	AiParameterRGBA( "color" , 1.0f, 1.0f, 1.0f , 1.0f);
	AiParameterFLT( "p_diffuse_weight" , 0.6f);
	AiParameterPTR("data", NULL);
}

shader_evaluate {
	DD::Image::Iop * backGround;
	backGround = static_cast <DD::Image::Iop *> (AiNodeGetPtr(node, "data"));

	AtRGBA  result; //  = AiShaderEvalParamRGBA(color);
	AtRGBA  color   = AiShaderEvalParamRGBA(p_color);
	float   dWeight = AiShaderEvalParamFlt(p_diffuse_weight);

	color.a       = AiLightsGetShadowMatte(sg).r;

	float XRES = AiNodeGetInt(AiUniverseGetOptions(), "xres");
	float YRES = AiNodeGetInt(AiUniverseGetOptions(), "yres");
	if (sg->Rt == AI_RAY_CAMERA) {
		if (backGround!=0) {
			backGround->request(DD::Image::Chan_Red, 0);
			result.r = backGround->at(sg->x, YRES-sg->y, DD::Image::Chan_Red);
			backGround->request(DD::Image::Chan_Green, 0);
			result.g = backGround->at(sg->x, YRES-sg->y, DD::Image::Chan_Green);
			backGround->request(DD::Image::Chan_Blue, 0);
			result.b = backGround->at(sg->x, YRES-sg->y, DD::Image::Chan_Blue);
		}

		AtColor tmpInDiff = AiIndirectDiffuse( &sg->Nf, sg)*color.a;
		result.r    *= ((color.r * AiLightsGetShadowMatte(sg).r) + tmpInDiff.r*dWeight);//*color.a;
		result.g    *= ((color.g * AiLightsGetShadowMatte(sg).g) + tmpInDiff.g*dWeight);//*color.a;
		result.b    *= ((color.b * AiLightsGetShadowMatte(sg).b) + tmpInDiff.b*dWeight);//*color.a;
		result.a     = color.a;
		sg->out.RGBA = result;
	} else {
		AtMatrix mat;
		AiWorldToScreenMatrix(AiUniverseGetCamera(), AiCameraGetShutterStart(), mat);
		AtPoint tmpP;
		AiM4PointByMatrixMult( &tmpP, mat, &sg->P );

		if ( tmpP.x > 0 ) {
			sg->u = tmpP.x / XRES ;
		} else {
			sg->u  = 0;
			//tmpP.x = 0;
		}

		if ( tmpP.y > 0 ) {
			sg->v = 1 - tmpP.y / YRES ;
		} else {
			sg->v = 0;
			tmpP.y = 0;
		}
		// do texture lookup
		if (backGround!=0) {
			backGround->request(DD::Image::Chan_Red,0);
			result.r = backGround->at(tmpP.x, tmpP.y, DD::Image::Chan_Red);
			backGround->request(DD::Image::Chan_Green,0);
			result.g = backGround->at(tmpP.x, tmpP.y, DD::Image::Chan_Green);
			backGround->request(DD::Image::Chan_Blue,0);
			result.b = backGround->at(tmpP.x, tmpP.y, DD::Image::Chan_Blue);
			//result.a = backGround->at(sg->x,sg->y,Chan_Alpha);
		}

		result.r *= 1-color.a;
		result.g *= 1-color.a;
		result.b *= 1-color.a;
		//result.a *= color.a;
		sg->out.RGBA    = result;
	}
}

node_initialize {
}

node_update {
}

node_finish {
}

