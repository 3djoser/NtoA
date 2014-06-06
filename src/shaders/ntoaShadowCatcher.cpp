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

#include "ntoaShadowCatcher.h"

ntoaShadowCatcher::ntoaShadowCatcher(Node* node)  : DD::Image::Iop(node) {
	color[0] = 1.0f;
	color[1] = 1.0f;
	color[2] = 1.0f;
	color[3] = 1.0f;
	p_diffuse_weight = 0.6f;
}

AtNode * ntoaShadowCatcher::AiExport(ntoaShadowCatcher * shaderObject) {
	// create a lambert shader
	AtNode *shaderS = AiNode("fb_ShadowMtd");
	AiNodeSetStr(shaderS, "name", shaderObject->node_name().c_str());
	AiNodeSetRGBA(shaderS, "color", shaderObject->color[0], shaderObject->color[1], shaderObject->color[2], shaderObject->color[3]);
	AiNodeSetFlt(shaderS, "p_diffuse_weight", shaderObject->p_diffuse_weight);
	return shaderS;
}

int ntoaShadowCatcher::minimum_inputs() const {
	return 0;
}

int ntoaShadowCatcher::maximum_inputs() const {
    return 0;
}

void ntoaShadowCatcher::knobs(DD::Image::Knob_Callback f) {
		Color_knob(f, (float*)&color, "color", "Color");
		ClearFlags(f, DD::Image::Knob::MAGNITUDE);
		Float_knob(f, &p_diffuse_weight, "p_diffuse_weight", "Weight");
}

void ntoaShadowCatcher::_validate(bool) {
	// Hash up knobs
	DD::Image::Hash knob_hash;
	knob_hash.reset();
	knob_hash.append(color[0]);
	knob_hash.append(color[1]);
	knob_hash.append(color[2]);
	knob_hash.append(p_diffuse_weight);
	append(knob_hash);
}

void ntoaShadowCatcher::engine(int y, int xx, int r, DD::Image::ChannelMask channels, DD::Image::Row& row) {
}

static DD::Image::Iop* constructor(Node* node) { return new ntoaShadowCatcher(node); }
const Iop::Description ntoaShadowCatcher::desc("ntoaShadowCatcher", "ntoaShadowCatcher", constructor);
