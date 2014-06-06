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

#ifndef NTOA_SHADOW_CATCHER_H_
#define NTOA_SHADOW_CATCHER_H_

#include <ai.h>

#include "DDImage/Iop.h"
#include "DDImage/Row.h"
#include "DDImage/Knobs.h"
#include "DDImage/Knob.h"
#include "DDImage/DDMath.h"

using namespace DD::Image;

class ntoaShadowCatcher : public DD::Image::Iop {
public:
	ntoaShadowCatcher(Node* node);

	AtNode * AiExport(ntoaShadowCatcher * shaderObject);

	int minimum_inputs() const;

	int maximum_inputs() const;

	void knobs(DD::Image::Knob_Callback f);

	void _validate(bool);

	void engine(int y, int xx, int r, DD::Image::ChannelMask channels, DD::Image::Row& row);

	const char* ntoaShadowCatcher::Class() const { return "ntoaShadowCatcher"; }

	const char* ntoaShadowCatcher::displayName() const { return "ntoaShadowCatcher"; }

	const char* ntoaShadowCatcher::node_help() const { return ""; }

public:
	static const Description desc;
	float color[4];
	float p_diffuse_weight;
};

#endif /* NTOA_SHADOW_CATCHER_H_ */
