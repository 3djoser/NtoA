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

class ntoaStandard : public DD::Image::Iop
{
public:
	ntoaStandard(Node* node);

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
