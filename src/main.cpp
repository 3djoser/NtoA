#include "ntoa.h"
#include "ntoaStandardShader.h"
//#include "ntoaDriver.h"
//#include "ntoaShadowCatcherShader.h"


extern AtNodeMethods* ntoa_driver_std;
extern AtNodeMethods* fb_ShadowMtd;

enum SHADERS
{
   MY_SHADER,
   MY_OTHER_SHADER
};

node_loader
{
   switch (i)
   {
      case MY_SHADER:
         node->methods     = (AtNodeMethods*) ntoa_driver_std;
         node->output_type = AI_TYPE_RGB;
         node->name        = "ntoa_drv";
         node->node_type   = AI_NODE_DRIVER;
      break;

      case MY_OTHER_SHADER:
         node->methods     = (AtNodeMethods*) fb_ShadowMtd;
         node->output_type = AI_TYPE_RGBA;
         node->name        = "fb_ShadowMtd";
         node->node_type   = AI_NODE_SHADER;
			strcpy(node->version, AI_VERSION);
			return true;
      break;

      default:
         return false;
   }

   sprintf(node->version, AI_VERSION);
   return true;
}

// nuke builder stuff
static Iop* build(Node* node) { return new NTOA(node); }
const Iop::Description NTOA::d("ArnoldRender", "ArnoldRender", build);

static DD::Image::Iop* constructor(Node* node) { return new ntoaStandard(node); }
const Iop::Description ntoaStandard::desc("ntoaStandard", "ntoaStandard", constructor);
