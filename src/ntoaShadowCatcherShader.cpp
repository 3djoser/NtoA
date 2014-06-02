#include <stdio.h>
#include <string.h>

#include <ai.h>

#include "DDImage/Iop.h"

enum fb_ShadowShaderParams
{
	p_color,p_diffuse_weight
};

AI_SHADER_NODE_EXPORT_METHODS(fb_ShadowMtd);

node_parameters
{
	AiParameterRGBA( "color"            , 1.0f, 1.0f, 1.0f , 1.0f);
	AiParameterFLT( "p_diffuse_weight" , 0.5f);
	AiParameterPTR("data", NULL);
}

shader_evaluate
{
	DD::Image::Iop * backGround;
	backGround = static_cast <DD::Image::Iop *> (AiNodeGetPtr(node, "data"));

	AtRGBA  result  = AiShaderEvalParamRGBA(p_color);
	AtRGBA  color   = AiShaderEvalParamRGBA(p_color);
	float   dWeight = AiShaderEvalParamFlt(p_diffuse_weight);

	color.a       = AiLightsGetShadowMatte(sg).r;
   
	if (sg->Rt == AI_RAY_CAMERA)
	{
		if (backGround!=0)
		{
			backGround->request(DD::Image::Chan_Red, 0);
			result.r = backGround->at(sg->x, 480-sg->y, DD::Image::Chan_Red);
			backGround->request(DD::Image::Chan_Green, 0);
			result.g = backGround->at(sg->x, 480-sg->y, DD::Image::Chan_Green);
			backGround->request(DD::Image::Chan_Blue, 0);
			result.b = backGround->at(sg->x, 480-sg->y, DD::Image::Chan_Blue);
		}

		AtColor tmpInDiff = AiIndirectDiffuse( &sg->Nf, sg);
		result.r           *= ((color.r * AiLightsGetShadowMatte(sg).r) + tmpInDiff.r*dWeight)*color.a;
		result.g           *= ((color.g * AiLightsGetShadowMatte(sg).g) + tmpInDiff.g*dWeight)*color.a;
		result.b           *= ((color.b * AiLightsGetShadowMatte(sg).b) + tmpInDiff.b*dWeight)*color.a;
		result.a            = color.a;
		sg->out.RGBA    = result;
	} else {
		AtMatrix mat;
		AiWorldToScreenMatrix(AiUniverseGetCamera(), AiCameraGetShutterStart(), mat);
		AtPoint tmpP;
		AiM4PointByMatrixMult( &tmpP, mat, &sg->P );

		if ( tmpP.x > 0 )
		{
			sg->u = tmpP.x / 640.0f ;
		} else {
			sg->u  = 0;
			//tmpP.x = 0;
		}

		if ( tmpP.y > 0 )
		{
			sg->v = 1 - tmpP.y / 480.0f ;
		} else {
			sg->v = 0;
			tmpP.y = 0;
		}
		// do texture lookup
		if (backGround!=0)
		{
			//std::cout << "backGround                           " << backGround << std::endl;
			//std::cout << "backGround Class                     " << backGround->Class() << std::endl;
			//std::cout << "x : " << tmpP.x << " y : " << tmpP.y << std::endl;
			//std::cout << "Chan_Red                             " << Chan_Red << std::endl;
			//std::cout << "backGround->at(sg->x,sg->y,Chan_Red) " << backGround->at(sg->x,sg->y,Chan_Red) << std::endl;
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

node_initialize
{
}

node_update
{
}

node_finish
{
}

