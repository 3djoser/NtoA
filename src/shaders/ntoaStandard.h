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

#ifndef NTOA_STANDARD_SHADER_H_
#define NTOA_STANDARD_SHADER_H_

#include <ai.h>

#include "DDImage/Iop.h"
#include "DDImage/Row.h"
#include "DDImage/Knobs.h"
#include "DDImage/Knob.h"
#include "DDImage/DDMath.h"

using namespace DD::Image;

static const char* const brdf_types[] = { "stretched_phong", "cook_torrance", "ward_duer", 0 };

class ntoaStandard : public DD::Image::Iop {
public:
	ntoaStandard(Node* node);

	AtNode * AiExport(ntoaStandard * shaderObject);

	int minimum_inputs() const;

	int maximum_inputs() const;

	void knobs(DD::Image::Knob_Callback f);

	void _validate(bool);

	void engine(int y, int xx, int r, DD::Image::ChannelMask channels, DD::Image::Row& row);

	const char* ntoaStandard::Class() const { return "ntoaStandard"; }

	const char* ntoaStandard::displayName() const { return "ntoaStandard"; }

	const char* ntoaStandard::node_help() const { return ""; }

public:
	static const Description desc;
	int   specular_brdf;
	float specular_roughness;
	float specular_anisotropy;
	float specular_rotation;
	float transmittance[3];
	float Kd;
	float Kd_color[3];
	float diffuse_roughness;
	float Ks;
	float Ks_color[3];
	float Phong_exponent;
	float Kr;
	float Kr_color[3];
	float reflection_exit_color[3];
	bool  reflection_exit_use_environment;
	float Kt;
	float Kt_color[3];
	float refraction_exit_color[3];
	bool  refraction_exit_use_environment;
	float IOR;
	float Kb;
	bool  Fresnel;
	float Krn;
	bool  specular_Fresnel;
	float Ksn;
	bool  Fresnel_affect_diff;
	float emission;
	float emission_color[3];
	float direct_specular;
	float indirect_specular;
	float direct_diffuse;
	float indirect_diffuse;
	bool  enable_glossy_caustics;
	bool  enable_reflective_caustics;
	bool  enable_refractive_caustics;
	bool  enable_internal_reflections;
	float Ksss;
	float Ksss_color[3];
	float sss_radius[3];
	float bounce_factor;
	float opacity[3];
};

#endif /* NTOA_STANDARD_SHADER_H_ */
