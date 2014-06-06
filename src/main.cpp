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

#include "ntoa.h"

// NtoA's Arnold driver and shadowCatcher shader
extern AtNodeMethods* ntoa_driver_std;
extern AtNodeMethods* fb_ShadowMtd;

enum SHADERS {
   MY_SHADER,
   MY_OTHER_SHADER
};

node_loader {
   switch (i) {
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

// NtoA Arnold renderer
static Iop* build(Node* node) { return new NTOA(node); }
const Iop::Description NTOA::d("ArnoldRender", "ArnoldRender", build);


