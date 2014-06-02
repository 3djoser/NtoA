#include <stdio.h>
#include <string.h>
#include <ai.h>
#include <ai_shaderglobals.h>
#include <ai_shaders.h>
#include <ai_shader_parameval.h>
#include <ai_shader_lights.h>

#include "DDImage/Iop.h"

using namespace DD::Image;

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
	Iop * backGround;
	backGround = static_cast <Iop *> (AiNodeGetPtr(node, "data"));


	AtRGBA  result  = AiShaderEvalParamRGBA(p_color);
   AtRGBA  color   = AiShaderEvalParamRGBA(p_color);
   float   dWeight = AiShaderEvalParamFlt(p_diffuse_weight);

   color.a       = AiLightsGetShadowMatte(sg).r;
   
   if (sg->Rt == AI_RAY_CAMERA)
   {
      if (backGround!=0)
      {
      	backGround->request(Chan_Red,0);
			result.r = backGround->at(sg->x,480-sg->y,Chan_Red);
      	backGround->request(Chan_Green,0);
			result.g = backGround->at(sg->x,480-sg->y,Chan_Green);
      	backGround->request(Chan_Blue,0);
			result.b = backGround->at(sg->x,480-sg->y,Chan_Blue);
      }
   	AtColor tmpInDiff = AiIndirectDiffuse( &sg->Nf, sg);
   	result.r           *= ((color.r * AiLightsGetShadowMatte(sg).r) + tmpInDiff.r*dWeight)*color.a;
   	result.g           *= ((color.g * AiLightsGetShadowMatte(sg).g) + tmpInDiff.g*dWeight)*color.a;
   	result.b           *= ((color.b * AiLightsGetShadowMatte(sg).b) + tmpInDiff.b*dWeight)*color.a;
   	result.a            = color.a;
      sg->out.RGBA    = result;
   }else
   {
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
      //AtTextureParams texparams;
      //AiTextureParamsSetDefaults(&texparams);
      // setup filter?
      //result   = AiTextureAccess(sg, "/home/ndu/workspace/ntoa/rue.tiff", &texparams);
      if (backGround!=0)
      {
			//std::cout << "backGround                           " << backGround << std::endl;
			//std::cout << "backGround Class                     " << backGround->Class() << std::endl;
			//std::cout << "x : " << tmpP.x << " y : " << tmpP.y << std::endl;
			//std::cout << "Chan_Red                             " << Chan_Red << std::endl;
			//std::cout << "backGround->at(sg->x,sg->y,Chan_Red) " << backGround->at(sg->x,sg->y,Chan_Red) << std::endl;
      	backGround->request(Chan_Red,0);
			result.r = backGround->at(tmpP.x,tmpP.y,Chan_Red);
      	backGround->request(Chan_Green,0);
			result.g = backGround->at(tmpP.x,tmpP.y,Chan_Green);
      	backGround->request(Chan_Blue,0);
			result.b = backGround->at(tmpP.x,tmpP.y,Chan_Blue);
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

