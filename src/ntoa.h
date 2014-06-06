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
/*
 * ntoa.h
 *
 *  Created on: Jan 20, 2011
 *      Author: ndu
 */

#ifndef NTOA_H_
#define NTOA_H_

#include <ai.h>

//#include "utils.h"

#include <time.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <sstream>

#include <memory>
#include <cstdarg>
#include <cstdio>
#include <limits.h>
#include <unistd.h>

#include "DDImage/Op.h"
#include "DDImage/Format.h"
#include "DDImage/Row.h"
#include "DDImage/Thread.h"
#include "DDImage/Knobs.h"
#include "DDImage/DDMath.h"
#include "DDImage/GeoOp.h"
#include "DDImage/Scene.h"
#include "DDImage/Iop.h"
#include "DDImage/RenderScene.h"
#include "DDImage/ReadGeo.h"
#include "DDImage/Pixel.h"

using namespace DD::Image;

#include "cBuffer.h"
#include "shaders/ntoaStandard.h"
#include "shaders/ntoaShadowCatcher.h"

extern AtNodeMethods* ntoa_driver_std;
extern AtNodeMethods* fb_ShadowMtd;

class NTOA : public RenderScene {
public:

	NTOA(Node* node);

	~NTOA();

	void knobs(Knob_Callback f);

	int knob_changed(Knob* knb);

	void startArnold();

	void stopArnold();

	void translateOptions();

	void translateSky();

	void translateScene();

	void translateCamera(CameraOp* nukeCamera);

	void translateLight(LightOp* nukeLight);

	void translateGeo(GeoInfo& object);

	void flagForUpdate();

	void increment();

	void append(Hash& hash);

	// Nuke call order
	void _validate(bool for_real);

	//void _request(int x, int y, int r, int t, ChannelMask channels, int count);

	void _open();

	void engine(int y, int xx, int r, ChannelMask channels, Row& row);

	//void _close();

	const char* Class() const { return "ArnoldRender"; }

	const char* displayName() const { return "NtoA"; }

	const char* node_help() const { return "Nuke to Arnold"; }

public:
	static const Iop::Description d;
	bool         inRender;
	bool         doAbort;
	Lock         m_mutex; // mutex for locking the pixel buffer
    cBuffer      m_buffer; // our pixel buffer
    FormatPair   formats;
	unsigned int hash_counter; // our refresh hash counter
	unsigned int scene_hash_counter; // our refresh hash counter
	/*
	//ENUM          AA_pattern                        = multi_jittered
	//ENUM          AA_motionblur_pattern             = jittered
	//ENUM          pin_threads                       = auto
	//ENUM          auto_transparency_mode            = always
	 */
    // Sampling
    int           m_AA_samples;
    int           m_GI_diffuse_samples;
    int           m_GI_glossy_samples;
    int           m_GI_refraction_samples;
    int           m_GI_single_scatter_samples;
    int           m_sss_bssrdf_samples;
    int           m_volume_indirect_samples;
	bool          m_lock_sampling_pattern;
    // Clamping
	bool          m_clamping_sample_value;
    bool          m_AA_sample_clamp_affects_aovs;
    float         m_AA_sample_clamp;
    // Filter
	float        m_filter_width;
    // Ray depth
    int           m_GI_total_depth;
    int           m_GI_diffuse_depth;
    int           m_GI_glossy_depth;
    int           m_GI_reflection_depth;
    int           m_GI_refraction_depth;
    int           m_auto_transparency_depth;
    float         m_auto_transparency_threshold;
    // Motion Blur
    bool          m_ignore_motion_blur;
    bool          m_curved_motionblur;
    // Lights
    float         m_low_light_threshold;
    // Textures
    bool          m_texture_automip;
    bool          m_texture_accept_unmipped;
    int           m_texture_autotile;
    bool          m_texture_accept_untiled;
    float         m_texture_max_memory_MB;
    int           m_texture_max_open_files;
    float         m_texture_glossy_blur;
    float         m_texture_diffuse_blur;
    float         m_texture_sss_blur;
    float         m_texture_max_sharpen;
    // Gamma correction
    float         m_light_gamma;
    float         m_shader_gamma;
    float         m_texture_gamma;
    // Render settings
    //ENUM          bucket_scanning;
    int           m_bucket_size;
    int           m_threads;
    bool          m_procedural_force_expand;
    int           m_region_min_x;
    int           m_region_min_y;
    int           m_region_max_x;
    int           m_region_max_y;
    float         m_aspect_ratio;
    bool          m_shadows_obey_light_linking;
    bool          m_enable_aov_composition;
    // Search Paths
	//STRING        procedural_searchpath;
	//STRING        shader_searchpath;
	//STRING        texture_searchpath;
    // Licensing
    bool          m_abort_on_license_fail;
    bool          m_skip_license_check;
    // Diagnostics
    bool          m_shader_nan_checks;
    bool          m_abort_on_error;
	// Sky
	bool         m_sky_active;
	float        m_sky_color[3];
	float        m_sky_intensity;
	bool         m_sky_castsShadows;
	bool         m_sky_visibility;
	bool         m_sky_visibleInReflections;
	bool         m_sky_visibleInRefractions;
	bool         m_sky_diffuse_visibility;
	bool         m_sky_glossy_visibility;
	// MISC
	const char * m_assFile;
	// CAMERA OVERRIDE
	int          m_camera_count;
	bool         m_cam_dof;
	float        m_focal_distance;
	float        m_aperture_size;
	int          m_aperture_blades;
	float        m_aperture_rotation;
	float        m_aperture_blade_curvature;
	float        m_aperture_aspect_ratio;
	// LIGHTS
	int          m_light_count;
	// GEOS
	int          m_geo_count;
};

#endif /* NTOA_H_ */
