#include "ntoaStandardShader.h"

ntoaStandard::ntoaStandard(Node* node)  : DD::Image::Iop(node)
{
	specular_brdf					  = 0;
	specular_roughness                = 0.467138f;
	specular_anisotropy               = 0.5f;
	specular_rotation                 = 0.0f;
	//	  transmittance                     = { 1.0f, 1.0f, 1.0f };
	Kd                                = 0.7f;
	//	  Kd_color                          = { 1.0f, 1.0f, 1.0f };
	diffuse_roughness                 = 0.0f;
	Ks                                = 0.0f;
	//Ks_color                          = { 1.0f, 1.0f, 1.0f };
	Phong_exponent                    = 10.0f;
	Kr                                = 0.0f;
	//Kr_color                          = { 1.0f, 1.0f, 1.0f };
	//reflection_exit_color             = { 0.0f, 0.0f, 0.0f };
	reflection_exit_use_environment   = false;
	Kt                                = 0.0f;
	//Kt_color                          = { 1.0f, 1.0f, 1.0f };
	//refraction_exit_color             = { 0.0f, 0.0f, 0.0f };
	refraction_exit_use_environment   = false;
	IOR                               = 1.0f;
	Kb                                = 0.0f;
	Fresnel                           = false;
	Krn                               = 0.0f;
	specular_Fresnel                  = false;
	Ksn                               = 0.0f;
	Fresnel_affect_diff               = true;
	emission                          = 0.0f;
	//emission_color                    = { 1.0f, 1.0f, 1.0f };
	direct_specular                   = 1.0f;
	indirect_specular                 = 1.0f;
	direct_diffuse                    = 1.0f;
	indirect_diffuse                  = 1.0f;
	enable_glossy_caustics            = false;
	enable_reflective_caustics        = false;
	enable_refractive_caustics        = false;
	enable_internal_reflections       = true;
	Ksss                              = 0.0f;
	//Ksss_color                        = { 1.0f, 1.0f, 1.0f };
	//sss_radius                        = { 0.1f, 0.1f, 0.1f };
	bounce_factor                     = 1.0f;
	opacity[0]                         = 1.0f; //{ 1.0f, 1.0f, 1.0f };
	opacity[1]                         = 1.0f; //{ 1.0f, 1.0f, 1.0f };
	opacity[2]                         = 1.0f; //{ 1.0f, 1.0f, 1.0f };
}

int ntoaStandard::minimum_inputs() const
{
	return 0;
}

int ntoaStandard::maximum_inputs() const
{
    return 0;
}

void ntoaStandard::knobs(DD::Image::Knob_Callback f)
{
	//
	BeginClosedGroup(f, "Diffuse");
		Color_knob(f, (float*)&Kd_color,               "Kd_color3",                        "Color");
		ClearFlags(f, DD::Image::Knob::MAGNITUDE);
		Float_knob(f, &Kd,                             "Kd",                              "Weight");
		Float_knob(f, &diffuse_roughness,              "diffuse_roughness",               "Roughness");
		Float_knob(f, &Kb,                             "Kb",                              "Backlight");
		Float_knob(f, &direct_diffuse,                 "direct_diffuse",                  "Direct diffuse");
		Float_knob(f, &indirect_diffuse,               "indirect_diffuse",                "Indirect diffuse");
		Bool_knob(f, &Fresnel_affect_diff,             "Fresnel_affect_diff",             "Fresnel affect diff");
		SetFlags(f, DD::Image::Knob::STARTLINE);
	EndGroup(f);
	//
	BeginClosedGroup(f, "Specular");
		Enumeration_knob(f, &specular_brdf, brdf_types,"specular_brdf",                   "Brdf");
		Color_knob(f, (float*)&Ks_color,               "Ks_color",                        "Color");
		ClearFlags(f, DD::Image::Knob::MAGNITUDE);
		Float_knob(f, &Ks,                             "Ks",                              "Weight");
		Float_knob(f, &Phong_exponent,                 "Phong_exponent",                  "Phong exponent");
		Float_knob(f, &specular_roughness,             "specular_roughness",              "Specular Roughness");
		Float_knob(f, &specular_anisotropy,            "specular_anisotropy",             "Specular Anisotropy");
		Float_knob(f, &specular_rotation,              "specular_rotation",               "Specular Rotation");
		Bool_knob(f, &specular_Fresnel,                "specular_Fresnel",                "Fresnel");
		SetFlags(f, DD::Image::Knob::STARTLINE);
		Float_knob(f, &Ksn,                            "Ksn",                             "Fresnel Coefficient");
		Float_knob(f, &direct_specular,                "direct_specular",                 "Direct specular");
		Float_knob(f, &indirect_specular,              "indirect_specular",               "Indirect specular");
	EndGroup(f);
	//
	BeginClosedGroup(f, "Reflection");
		Color_knob(f, (float*)&Kr_color,               "Kr_color",                        "Color");
		ClearFlags(f, DD::Image::Knob::MAGNITUDE);
		Float_knob(f, &Kr,                             "Kr",                              "Weight");
		Bool_knob(f, &Fresnel,                         "Fresnel",                         "Fresnel");
		SetFlags(f, DD::Image::Knob::STARTLINE);
		Float_knob(f, &Krn,                            "Krn",                             "Fresnel Coefficient");
		Color_knob(f, (float*)&reflection_exit_color,  "reflection_exit_color",           "Reflection exit color");
		ClearFlags(f, DD::Image::Knob::MAGNITUDE);
		Bool_knob(f, &reflection_exit_use_environment, "reflection_exit_use_environment", "Reflection exit use environment");
		SetFlags(f, DD::Image::Knob::STARTLINE);
	EndGroup(f);
	//
	BeginClosedGroup(f, "Refraction");
		Float_knob(f, &IOR,                            "IOR",                             "Index of Refraction");
		Color_knob(f, (float*)&Kt_color,               "Kt_color",                        "Color");
		ClearFlags(f, DD::Image::Knob::MAGNITUDE);
		Float_knob(f, &Kt,                             "Kt",                              "Transparency");
		Color_knob(f, (float*)&transmittance,          "transmittance",                   "Transmittance");
		ClearFlags(f, DD::Image::Knob::MAGNITUDE);
		Bool_knob(f, &refraction_exit_use_environment, "refraction_exit_use_environment", "Refraction exit use environment");
		SetFlags(f, DD::Image::Knob::STARTLINE);
		Color_knob(f, (float*)&refraction_exit_color,  "refraction_exit_color",           "Refraction exit color");
		ClearFlags(f, DD::Image::Knob::MAGNITUDE);
		Bool_knob(f, &enable_internal_reflections,     "enable_internal_reflections",     "Enable internal reflections");
		SetFlags(f, DD::Image::Knob::STARTLINE);
	EndGroup(f);
	//
	BeginClosedGroup(f, "SSS");
		Color_knob(f, (float*)&Ksss_color,             "Ksss_color",                      "Color");
		ClearFlags(f, DD::Image::Knob::MAGNITUDE);
		Float_knob(f, &Ksss,                           "Ksss",                            "Weight");
		Color_knob(f, (float*)&sss_radius,             "sss_radius",                      "Radius");
		ClearFlags(f, DD::Image::Knob::MAGNITUDE);
	EndGroup(f);
	//
	BeginClosedGroup(f, "Emission");
		Color_knob(f, (float*)&emission_color,         "emission_color",                  "Color");
		ClearFlags(f, DD::Image::Knob::MAGNITUDE);
		Float_knob(f, &emission,                       "emission",                        "Weight");
	EndGroup(f);
	//
	BeginClosedGroup(f, "Caustics");
		Bool_knob(f, &enable_glossy_caustics,          "enable_glossy_caustics",          "Enable glossy caustics");
		SetFlags(f, DD::Image::Knob::STARTLINE);
		Bool_knob(f, &enable_reflective_caustics,      "enable_reflective_caustics",      "Enable reflective caustics");
		SetFlags(f, DD::Image::Knob::STARTLINE);
		Bool_knob(f, &enable_refractive_caustics,      "enable_refractive_caustics",      "Enable refractive caustics");
		SetFlags(f, DD::Image::Knob::STARTLINE);
	EndGroup(f);
	//
	BeginClosedGroup(f, "Advanced");
		Float_knob(f, &bounce_factor,                  "bounce_factor",                   "Bounce factor");
		Color_knob(f, (float*)&opacity,                "opacity",                         "Opacity");
		ClearFlags(f, DD::Image::Knob::MAGNITUDE);
	EndGroup(f);
}

void ntoaStandard::_validate(bool)
{
	// Hash up knobs
	DD::Image::Hash knob_hash;
	knob_hash.reset();
	knob_hash.append(specular_brdf);
	knob_hash.append(specular_roughness);
	knob_hash.append(specular_anisotropy);
	knob_hash.append(specular_rotation);
	knob_hash.append(transmittance[0]);
	knob_hash.append(transmittance[1]);
	knob_hash.append(transmittance[2]);
	knob_hash.append(Kd);
	knob_hash.append(Kd_color[0]);
	knob_hash.append(Kd_color[1]);
	knob_hash.append(Kd_color[2]);
	knob_hash.append(diffuse_roughness);
	knob_hash.append(Ks);
	knob_hash.append(Ks_color[0]);
	knob_hash.append(Ks_color[1]);
	knob_hash.append(Ks_color[2]);
	knob_hash.append(Phong_exponent);
	knob_hash.append(Kr);
	knob_hash.append(Kr_color[0]);
	knob_hash.append(Kr_color[1]);
	knob_hash.append(Kr_color[2]);
	knob_hash.append(reflection_exit_color[0]);
	knob_hash.append(reflection_exit_color[1]);
	knob_hash.append(reflection_exit_color[2]);
	knob_hash.append(reflection_exit_use_environment);
	knob_hash.append(Kt);
	knob_hash.append(Kt_color[0]);
	knob_hash.append(Kt_color[1]);
	knob_hash.append(Kt_color[2]);
	knob_hash.append(refraction_exit_color[0]);
	knob_hash.append(refraction_exit_color[1]);
	knob_hash.append(refraction_exit_color[2]);
	knob_hash.append(refraction_exit_use_environment);
	knob_hash.append(IOR);
	knob_hash.append(Kb);
	knob_hash.append(Fresnel);
	knob_hash.append(Krn);
	knob_hash.append(specular_Fresnel);
	knob_hash.append(Ksn);
	knob_hash.append(Fresnel_affect_diff);
	knob_hash.append(emission);
	knob_hash.append(emission_color[0]);
	knob_hash.append(emission_color[1]);
	knob_hash.append(emission_color[2]);
	knob_hash.append(direct_specular);
	knob_hash.append(indirect_specular);
	knob_hash.append(direct_diffuse);
	knob_hash.append(indirect_diffuse);
	knob_hash.append(enable_glossy_caustics);
	knob_hash.append(enable_reflective_caustics);
	knob_hash.append(enable_refractive_caustics);
	knob_hash.append(enable_internal_reflections);
	knob_hash.append(Ksss);
	knob_hash.append(Ksss_color[0]);
	knob_hash.append(Ksss_color[1]);
	knob_hash.append(Ksss_color[2]);
	knob_hash.append(sss_radius[0]);
	knob_hash.append(sss_radius[1]);
	knob_hash.append(sss_radius[2]);
	knob_hash.append(bounce_factor);
	knob_hash.append(opacity[0]);
	knob_hash.append(opacity[1]);
	knob_hash.append(opacity[2]);
	append(knob_hash);
}

void ntoaStandard::engine(int y, int xx, int r, DD::Image::ChannelMask channels, DD::Image::Row& row)
{
}

