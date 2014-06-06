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

#include "utils.h"

NTOA::NTOA(Node* node) : RenderScene(node) {
	// BUFFER INIT
	m_buffer.m_node  			= this;
	m_buffer.init(formats.format()->width(),formats.format()->height());
	inRender                    = false;
	//
	/*
	 *  Arnold params
	 */
	// Sampling
	m_AA_samples                        = 1;
	m_GI_diffuse_samples                = 2;
	m_GI_glossy_samples                 = 2;
	m_GI_refraction_samples             = 2;
	m_GI_single_scatter_samples         = 5;
	m_sss_bssrdf_samples                = 0;
	m_volume_indirect_samples           = 0;
	// Clamping
	m_AA_sample_clamp_affects_aovs      = false;
	m_AA_sample_clamp                   = 10.0f;
	// Filter
	// Ray depth
	m_GI_total_depth                    = 10;
	m_GI_diffuse_depth                  = 1;
	m_GI_glossy_depth                   = 1;
	m_GI_reflection_depth               = 2;
	m_GI_refraction_depth               = 2;
	m_auto_transparency_depth           = 10;
	m_auto_transparency_threshold       = 0.99f;
	// Motion Blur
	m_ignore_motion_blur                = false;
	m_curved_motionblur                 = true;
	// Lights
	m_low_light_threshold               = 0.001f;
	// Textures
	m_texture_automip                   = true;
	m_texture_accept_unmipped           = true;
	m_texture_autotile                  = 64;
	m_texture_accept_untiled            = true;
	m_texture_max_memory_MB             = 1024;
	m_texture_max_open_files            = 512;
	m_texture_glossy_blur               = 0.015625f;
	m_texture_diffuse_blur              = 0.03125f;
	m_texture_sss_blur                  = 0.0078125f;
	m_texture_max_sharpen               = 1.5f;
	// Gamma correction
	m_light_gamma                       = 1.0f;
	m_shader_gamma                      = 1.0f;
	m_texture_gamma                     = 1.0f;
	// Render settings
//	ENUM          bucket_scanning                   = top
	m_bucket_size                       = 64;
	m_threads                           = 0;
	m_procedural_force_expand           = false;
	m_region_min_x                      = -1;
	m_region_min_y                      = -1;
	m_region_max_x                      = -1;
	m_region_max_y                      = -1;
	m_aspect_ratio                      = 1.0f;
	m_shadows_obey_light_linking        = false;
	m_enable_aov_composition            = false;
	// Search Paths
//	STRING        procedural_searchpath             = "";
//	STRING        shader_searchpath                 = "";
//	STRING        texture_searchpath                = "";
	// Licensing
	m_abort_on_license_fail             = false;
	m_skip_license_check                = false;
	// Diagnostics
	m_shader_nan_checks                 = false;
	m_abort_on_error                    = true;


	// Sampling
	m_volume_indirect_samples	= 1;
	m_lock_sampling_pattern		= false;
	// Clamping
	m_clamping_sample_value 	= false;
	// Filter
	m_filter_width				= 2.0f;
	// CAMERA
	m_cam_dof 					= false;
	m_focal_distance 			= 1.0f;
	m_aperture_size 		   	= 0.0f;
	m_aperture_blades 		   	= 6;
	m_aperture_rotation 	   	= 0.0f;
	m_aperture_blade_curvature 	= 1.0f;
	m_aperture_aspect_ratio    	= 1.0f;
	// SKY
	m_sky_active     		   	= false;
    m_sky_color[0] = m_sky_color[1] = m_sky_color[2] = 1.0f;
	m_sky_intensity  		   	= 1.0f;
	m_sky_castsShadows 		   	= false;
	m_sky_visibility 		   	= false;
	m_sky_visibleInReflections 	= false;
	m_sky_visibleInRefractions 	= false;
	m_sky_diffuse_visibility 	= false;
	m_sky_glossy_visibility 	= false;
}

NTOA::~NTOA() {
}

void NTOA::knobs(Knob_Callback f) {
	// SAMPLING
	BeginClosedGroup(f, "Sampling");
		Int_knob(f, &m_AA_samples, "m_AA_samples", "Camera (AA)");
		Int_knob(f, &m_GI_diffuse_samples, "m_GI_diffuse_samples", "Diffuse");
		Int_knob(f, &m_GI_glossy_samples, "m_GI_glossy_samples", "Glossy");
		Int_knob(f, &m_GI_refraction_samples, "m_GI_refraction_samples", "Refraction");
		Int_knob(f, &m_GI_single_scatter_samples, "m_GI_single_scatter_samples", "Single Scatter");
		Int_knob(f, &m_sss_bssrdf_samples, "m_sss_bssrdf_samples", "SSS");
		Int_knob(f, &m_volume_indirect_samples, "m_volume_indirect_samples", "Volume Indirect");
		//Bool_knob(f, &m_lock_sampling_pattern, "m_lock_sampling_pattern", "Lock Sampling Pattern");
		//SetFlags(f, DD::Image::Knob::STARTLINE);
		BeginClosedGroup(f, "Clamping");
			Bool_knob(f, &m_AA_sample_clamp_affects_aovs, "m_AA_sample_clamp_affects_aovs", "Clamp Sample Values");
			SetFlags(f, DD::Image::Knob::STARTLINE);
			Bool_knob(f, &m_AA_sample_clamp_affects_aovs, "m_AA_sample_clamp_affects_aovs", "Affect AOV's");
			SetFlags(f, DD::Image::Knob::STARTLINE);
			Float_knob(f, &m_AA_sample_clamp, "m_AA_sample_clamp", "Max Value");
		EndGroup(f);
		BeginClosedGroup(f, "Filter");
			Float_knob(f, &m_filter_width, "m_filter_width", "Width");
		EndGroup(f);
	EndGroup(f);
	Divider(f, "");
	// RAY DEPTH
	BeginClosedGroup(f, "Ray Depth");
		Int_knob(f, &m_GI_total_depth, "m_GI_total_depth", "Total");
		Int_knob(f, &m_GI_diffuse_depth, "m_GI_diffuse_depth", "Diffuse");
		Int_knob(f, &m_GI_glossy_depth, "m_GI_glossy_depth", "Glossy");
		Int_knob(f, &m_GI_reflection_depth, "m_GI_reflection_depth", "Reflection");
		Int_knob(f, &m_GI_refraction_depth, "m_GI_refraction_depth", "Refraction");
		Int_knob(f, &m_auto_transparency_depth, "m_auto_transparency_depth", "Transparency Depth");
		Float_knob(f, &m_auto_transparency_threshold, "m_auto_transparency_threshold", "Transparency Threshold");
	EndGroup(f);
	Divider(f, "");
	// ENVIRONMENT
	BeginClosedGroup(f, "Environment");
		BeginClosedGroup(f, "Sky");
			Bool_knob(f, &m_sky_active, "sky_active", "Active");
			Color_knob(f, (float*)&m_sky_color, "sky_color", "Color");
			SetFlags(f, DD::Image::Knob::STARTLINE);
			Float_knob(f, &m_sky_intensity, "sky_intensity", "Intensity");
			Bool_knob(f, &m_sky_castsShadows, "sky_castsShadows", "Casts shadows");
			SetFlags(f, DD::Image::Knob::STARTLINE);
			Bool_knob(f, &m_sky_visibility, "sky_visibility", "Visibility");
			Bool_knob(f, &m_sky_visibleInReflections, "visibleInReflections", "Visible in reflections");
			SetFlags(f, DD::Image::Knob::STARTLINE);
			Bool_knob(f, &m_sky_visibleInRefractions, "visibleInRefractions", "Visible in refractions");
			Bool_knob(f, &m_sky_diffuse_visibility, "diffuse_visibility", "Diffuse visibility");
			SetFlags(f, DD::Image::Knob::STARTLINE);
			Bool_knob(f, &m_sky_glossy_visibility, "glossy_visibility", "Glossy visibility");
		EndGroup(f);
	EndGroup(f);
	Divider(f, "");
	// MOTION BLUR
	BeginClosedGroup(f, "Motion Blur");
//		Bool_knob(f, &m_ignore_motion_blur, "m_ignore_motion_blur", "");
//		Bool_knob(f, &m_curved_motionblur, "m_curved_motionblur", "");
	EndGroup(f);
	Divider(f, "");
	// SSS
	BeginClosedGroup(f, "SSS");
	EndGroup(f);
	Divider(f, "");
	// LIGHTS
	BeginClosedGroup(f, "Lights");
		Float_knob(f, &m_low_light_threshold, "m_low_light_threshold", "Low Light Threshold");
	EndGroup(f);
	Divider(f, "");
	// TEXTURES
	BeginClosedGroup(f, "Textures");
		Bool_knob(f, &m_texture_automip, "m_texture_automip", "Texture Automip");
		//Bool_knob(f, &m_texture_accept_unmipped, "m_texture_accept_unmipped", "Texture Accept Unmipped");
		SetFlags(f, DD::Image::Knob::STARTLINE);
		Int_knob(f, &m_texture_autotile, "m_texture_autotile", "Texture Autotile");
		SetFlags(f, DD::Image::Knob::STARTLINE);
		Bool_knob(f, &m_texture_accept_untiled, "m_texture_accept_untiled", "Texture Accept Untiled");
		SetFlags(f, DD::Image::Knob::STARTLINE);
		Float_knob(f, &m_texture_max_memory_MB, "m_texture_max_memory_MB", "Texture Max Memory (MB)");
		SetFlags(f, DD::Image::Knob::STARTLINE);
		Int_knob(f, &m_texture_max_open_files, "m_texture_max_open_files", "Texture Max Open Files");
		Float_knob(f, &m_texture_glossy_blur, "m_texture_glossy_blur", "Texture Glossy Blur");
		Float_knob(f, &m_texture_diffuse_blur, "m_texture_diffuse_blur", "Texture Diffuse Blur");
		Float_knob(f, &m_texture_sss_blur, "m_texture_sss_blur", "Texture SSS Blur");
		Float_knob(f, &m_texture_max_sharpen, "m_texture_max_sharpen", "Texture Max Sharpen");
	EndGroup(f);
	Divider(f, "");
	// GAMMA CORRECTION
	BeginClosedGroup(f, "Gamma Correction");
		Float_knob(f, &m_light_gamma, "m_light_gamma", "Light");
		Float_knob(f, &m_shader_gamma, "m_shader_gamma", "Shader");
		Float_knob(f, &m_texture_gamma, "m_texture_gamma", "Texture");
	EndGroup(f);
	Divider(f, "");
	// SYSTEM
	BeginClosedGroup(f, "System");
		// RENDER SETTINGS
		BeginClosedGroup(f, "Render Settings");
			Format_knob(f, &formats, "format");
			Float_knob(f, &m_aspect_ratio, "m_aspect_ratio", "Aspect Ratio");
			//ENUM          bucket_scanning, "", "");
			Int_knob(f, &m_bucket_size, "m_bucket_size", "Bucket Size");
			Int_knob(f, &m_threads, "m_threads", "Threads");
			Bool_knob(f, &m_procedural_force_expand, "m_procedural_force_expand", "Force Procedural Expand");
			SetFlags(f, DD::Image::Knob::STARTLINE);
			//Bool_knob(f, &m_shadows_obey_light_linking, "m_shadows_obey_light_linking", "Shadows_obey_light_linking");
			//SetFlags(f, DD::Image::Knob::STARTLINE);
			Bool_knob(f, &m_enable_aov_composition, "m_enable_aov_composition", "Enable AOV Composition");
			SetFlags(f, DD::Image::Knob::STARTLINE);
		EndGroup(f);
		// SEARCH PATHS
		BeginClosedGroup(f, "Search Paths");
			//STRING        procedural_searchpath, "", "");
			//STRING        shader_searchpath, "", "");
			//STRING        texture_searchpath, "", "");
		EndGroup(f);
		// LICENSING
		BeginClosedGroup(f, "Licensing");
			Bool_knob(f, &m_abort_on_license_fail, "m_abort_on_license_fail", "Abort On License Fail");
			Bool_knob(f, &m_skip_license_check, "m_skip_license_check", "Skip License Check");
			SetFlags(f, DD::Image::Knob::STARTLINE);
		EndGroup(f);
		// DIAGNOSTICS
		BeginClosedGroup(f, "Diagnostics");
			Bool_knob(f, &m_shader_nan_checks, "m_shader_nan_checks", "Shader Nan Checks");
			Bool_knob(f, &m_abort_on_error, "m_abort_on_error", "Abort On Error");
			SetFlags(f, DD::Image::Knob::STARTLINE);
		EndGroup(f);
	EndGroup(f);

	//
	//File_knob(f, &m_assFile, "ass_file", "Ass file");
	// Camera
	Tab_knob(f, "Camera");
	Bool_knob(f, &m_cam_dof, "cam_dof", "Camera DOF");
	Float_knob(f, &m_focal_distance, "focal_distance", "Camera focal distance");
	Float_knob(f, &m_aperture_size, "aperture_size", "Camera aperture size");
	Int_knob(f, &m_aperture_blades, "aperture_blades", "Camera aperture blades");
	Float_knob(f, &m_aperture_rotation, "aperture_rotation", "Camera aperture rotation");
	Float_knob(f, &m_aperture_blade_curvature, "aperture_blade_curvature", "Camera aperture blade curvature");
	Float_knob(f, &m_aperture_aspect_ratio, "aperture_aspect_ratio", "Camera aperture aspect ratio");
}

int NTOA::knob_changed(Knob* knb) {
	if (knb != NULL) {
		stopArnold();
		return 1;
	}
	return 0;
}

void NTOA::startArnold() {
	stopArnold();
	// start an Arnold session
	AiBegin();
	inRender = true;
	AiNodeSetInt(AiUniverseGetOptions(), "threads", 6);
	AiNodeSetInt(AiUniverseGetOptions(), "preserve_scene_data", true);
	AiMsgSetLogFileName("ntoa.log");
	AiNodeEntryInstall(AI_NODE_DRIVER, AI_TYPE_RGBA, "ntoa_drv", "<built-in>", ntoa_driver_std, AI_VERSION);

	// shadow catcher
	AiNodeEntryInstall(AI_NODE_SHADER, AI_TYPE_RGBA, "fb_ShadowMtd", "<built-in>", fb_ShadowMtd, AI_VERSION);
}

void NTOA::stopArnold() {
	if (inRender) {
		std::cout << "Arnold stop requested" << std::endl;
		while(AiRendering()) {
			AiRenderAbort();
		}
		AiEnd();
		inRender = false;
		std::cout << "Arnold stopped!" << std::endl;
	}
}

void NTOA::translateOptions() {
	std::cout << " -> translate Options : " << std::endl;
	AtNode *options = AiUniverseGetOptions();
	//
	AiNodeSetInt(options, "xres", formats.format()->width());
	AiNodeSetInt(options, "yres", formats.format()->height());
	// Sampling
    AiNodeSetInt(options, "AA_samples", m_AA_samples );
    AiNodeSetInt(options, "GI_diffuse_samples", m_GI_diffuse_samples );
    AiNodeSetInt(options, "GI_glossy_samples", m_GI_glossy_samples );
    AiNodeSetInt(options, "GI_refraction_samples", m_GI_refraction_samples );
    AiNodeSetInt(options, "GI_single_scatter_samples", m_GI_single_scatter_samples );
    AiNodeSetInt(options, "sss_bssrdf_samples", m_sss_bssrdf_samples );
    AiNodeSetInt(options, "volume_indirect_samples", m_volume_indirect_samples );
    // Clamping
    AiNodeSetBool(options, "AA_sample_clamp_affects_aovs", m_AA_sample_clamp_affects_aovs );
    AiNodeSetFlt(options, "AA_sample_clamp", m_AA_sample_clamp );
    // Filter
    // Ray depth
    AiNodeSetInt(options, "GI_total_depth", m_GI_total_depth );
    AiNodeSetInt(options, "GI_diffuse_depth", m_GI_diffuse_depth );
    AiNodeSetInt(options, "GI_glossy_depth", m_GI_glossy_depth );
    AiNodeSetInt(options, "GI_reflection_depth", m_GI_reflection_depth );
    AiNodeSetInt(options, "GI_refraction_depth", m_GI_refraction_depth );
    AiNodeSetInt(options, "auto_transparency_depth", m_auto_transparency_depth );
    AiNodeSetFlt(options, "auto_transparency_threshold", m_auto_transparency_threshold );
    // Motion Blur
    AiNodeSetBool(options, "ignore_motion_blur", m_ignore_motion_blur );
    AiNodeSetBool(options, "curved_motionblur", m_curved_motionblur );
    // Lights
    AiNodeSetFlt(options, "low_light_threshold", m_low_light_threshold );
    // Textures
    AiNodeSetBool(options, "texture_automip", m_texture_automip );
    AiNodeSetBool(options, "texture_accept_unmipped", m_texture_accept_unmipped );
    AiNodeSetInt(options, "texture_autotile", m_texture_autotile );
    AiNodeSetBool(options, "texture_accept_untiled", m_texture_accept_untiled );
    AiNodeSetFlt(options, "texture_max_memory_MB", m_texture_max_memory_MB );
    AiNodeSetInt(options, "texture_max_open_files", m_texture_max_open_files );
    AiNodeSetFlt(options, "texture_glossy_blur", m_texture_glossy_blur );
    AiNodeSetFlt(options, "texture_diffuse_blur", m_texture_diffuse_blur );
    AiNodeSetFlt(options, "texture_sss_blur", m_texture_sss_blur );
    AiNodeSetFlt(options, "texture_max_sharpen", m_texture_max_sharpen );
    // Gamma correction
    AiNodeSetFlt(options, "light_gamma", m_light_gamma );
    AiNodeSetFlt(options, "shader_gamma", m_shader_gamma );
    AiNodeSetFlt(options, "texture_gamma", m_texture_gamma );
    // Render settings
//  ENUM          bucket_scanning", m_bucket_scanning );
    AiNodeSetInt(options, "bucket_size", m_bucket_size );
    AiNodeSetInt(options, "threads", m_threads );
    AiNodeSetBool(options, "procedural_force_expand", m_procedural_force_expand );
    AiNodeSetInt(options, "region_min_x", m_region_min_x );
    AiNodeSetInt(options, "region_min_y", m_region_min_y );
    AiNodeSetInt(options, "region_max_x", m_region_max_x );
    AiNodeSetInt(options, "region_max_y", m_region_max_y );
    AiNodeSetFlt(options, "aspect_ratio", m_aspect_ratio );
    AiNodeSetBool(options, "shadows_obey_light_linking", m_shadows_obey_light_linking );
    AiNodeSetBool(options, "enable_aov_composition", m_enable_aov_composition );
    // Search Paths
//  STRING        procedural_searchpath", m_procedural_searchpath );
//  STRING        shader_searchpath", m_shader_searchpath );
//  STRING        texture_searchpath", m_texture_searchpath );
    // Licensing
    AiNodeSetBool(options, "abort_on_license_fail", m_abort_on_license_fail );
    AiNodeSetBool(options, "skip_license_check", m_skip_license_check );
    // Diagnostics
    AiNodeSetBool(options, "shader_nan_checks", m_shader_nan_checks );
    AiNodeSetBool(options, "abort_on_error", m_abort_on_error );

	// For now we create a gaussian filter node
	AtNode *filter = AiNode("gaussian_filter");
	AiNodeSetStr(filter, "name", "myfilter");

	// We create an NtoA display driver node
	AtNode *driver = AiNode("ntoa_drv");
	AiNodeSetStr(driver, "name", "mydriver");
	AiNodeSetPtr(driver, "data", &m_buffer);

	// assign the driver and filter to the main (beauty) AOV, which is called "RGB"
	AtArray *outputs_array = AiArrayAllocate(1, 1, AI_TYPE_STRING);
	AiArraySetStr(outputs_array, 0, "RGBA RGBA myfilter mydriver");
	AiNodeSetArray(options, "outputs", outputs_array);
}

void NTOA::translateSky() {
	if (m_sky_active) {
		AtNode *sky = AiNode("sky");
		AiNodeSetStr(sky, "name", "ntoaSky");
		AiNodeSetRGB(sky, "color",m_sky_color[0], m_sky_color[1], m_sky_color[2]);

		AiNodeSetFlt(sky, "intensity", m_sky_intensity);
		// Visibility options
		int visibility = 65535;

		if (!m_sky_castsShadows) visibility &= ~AI_RAY_SHADOW;

		if (!m_sky_visibility) visibility &= ~AI_RAY_CAMERA;

		if (!m_sky_visibleInReflections) visibility &= ~AI_RAY_REFLECTED;

		if (!m_sky_visibleInRefractions) visibility &= ~AI_RAY_REFRACTED;

		if (!m_sky_diffuse_visibility) visibility &= ~AI_RAY_DIFFUSE;

		if (!m_sky_glossy_visibility) visibility &= ~AI_RAY_GLOSSY;

		AiNodeSetInt(sky, "visibility", visibility);
		AiNodeSetPtr(AiUniverseGetOptions(), "background", sky);
	}
}

void NTOA::translateScene() {
	std::cout << " -> Start : translateScene"<< std::endl;
	// Options
	translateOptions();

	// Sky
	translateSky();

	// Camera
	translateCamera(render_camera());

	// Now we process the geometry
	GeoOp*       tmpGeo;
	tmpGeo = render_geo();
	if (tmpGeo != 0) {
		if (strcmp(tmpGeo->Class(), "Scene") == 0) {
			// We get the 3d scene
			Scene m_scene;
			tmpGeo->build_scene(m_scene);
			Scene * nodeScene = &m_scene;
			if (nodeScene!=0) {
				// LIGHTS
				nodeScene->evaluate_lights();
				const unsigned n = nodeScene->lights.size();
				for (unsigned i = 0; i < n; i++) {
					LightContext& ltx = *(nodeScene->lights[i]);
					translateLight(ltx.light());
				}
				// GEOMETRY
				GeometryList * 	m_geoList;
				m_geoList = nodeScene->object_list();
				if (m_geoList!=0) {
					std::cout << "Object(s) to export : " << m_geoList->objects()<< std::endl;
					for (unsigned i = 0; i <m_geoList->objects(); i++) {
						GeoInfo & object =  m_geoList->object(i);
						translateGeo(object);
					}
				}
			}
		}
	}
}

void NTOA::translateCamera(CameraOp * nukeCamera) {
	std::cout << " -> Start : translateCamera"<< std::endl;
	// We create a pointer for an Arnold perspective camera
	AtNode *camera;
	// Do we already have a node for this camera?
	camera = AiNodeLookUpByName("ntoaCamera");
	if (camera == 0) camera = AiNode("persp_camera");

	//AtNode *camera = AiNode("persp_camera");
	AiNodeSetStr(camera, "name", "ntoaCamera");
	AiNodeSetFlt(camera, "fov", (float)nukeCamera->focal_length());
	// position the camera (alternatively you can set 'matrix')
	Matrix4  tmpMat = nukeCamera->matrix();
	AtMatrix mTmpMat;
	mCpMat(tmpMat, mTmpMat);
	AiNodeSetMatrix(camera, "matrix", mTmpMat);

	if (m_cam_dof) {
		AiNodeSetFlt(camera, "focal_distance", (float)m_focal_distance);
		AiNodeSetFlt(camera, "aperture_size", (float)m_aperture_size);
		AiNodeSetInt(camera, "aperture_blades", (int)m_aperture_blades);
		AiNodeSetFlt(camera, "aperture_rotation", (float)m_aperture_size);
		AiNodeSetFlt(camera, "aperture_blade_curvature", (float)m_aperture_size);
		AiNodeSetFlt(camera, "aperture_aspect_ratio", (float)m_aperture_size);
	} else {
		AiNodeSetFlt(camera, "aperture_size", 0);
	}

	// set the active camera (optional, since there is only one camera)
	AiNodeSetPtr(AiUniverseGetOptions(), "camera", camera);
}

void NTOA::translateLight(LightOp* nukeLight) {
	std::cout << " -> Start : translateLight"<< std::endl;
	std::cout << nukeLight->Class() << std::endl;
	// create a point light source
	AtNode *light;
	// Do we already have a node for this light?
	light =	AiNodeLookUpByName(nukeLight->node_name().c_str());

	// No create one
	if (light == 0) {
		std::cout << "RENDER LOOP : Create light " << nukeLight->node_name().c_str() << std::endl;
		if       (strcmp(nukeLight->Class(), "Light") == 0) {
			light = AiNode("point_light");

		} else if (strcmp(nukeLight->Class(), "DirectLight") == 0) {
			light = AiNode("distant_light");

		} else if (strcmp(nukeLight->Class(), "Spotlight") == 0) {
			light = AiNode("spot_light");

		}
	}
	AiNodeSetStr(light, "name", nukeLight->node_name().c_str());
	Matrix4  tmpMat = nukeLight->matrix();
	AtMatrix mTmpMat;
	mCpMat(tmpMat, mTmpMat);
	AiNodeSetMatrix(light, "matrix", mTmpMat);
	//AiNodeSetFlt(light, "intensity", (float)nukeLight->intensity());
	AiNodeSetFlt(light, "intensity", 1);
	AiNodeSetFlt(light, "exposure", (float)nukeLight->intensity());
	Pixel & 	color = (Pixel &)nukeLight->color();
	float * m_tmpColor;
	m_tmpColor = (float *)color.array();
	std::cout << "Size of color " << sizeof(m_tmpColor) <<std::endl;
	/*for (int ii=0;ii<sizeof(m_tmpColor);ii++){
		std::cout << "    - " << m_tmpColor[ii]<<std::endl;
	}*/
	AiNodeSetRGB(light, "color", m_tmpColor[1],m_tmpColor[2],m_tmpColor[3]);
	AiNodeSetBool(light, "cast_shadows", nukeLight->cast_shadows());
	AiNodeSetInt(light, "samples", nukeLight->samples());
	if (strcmp(nukeLight->Class(), "DirectLight") == 0) {
		AiNodeSetFlt(light, "angle", nukeLight->sample_width());
	} else {
		AiNodeSetFlt(light, "radius", nukeLight->sample_width());
	}
}

void NTOA::translateGeo(GeoInfo& object) {
	// Le node
	GeoOp * tmpGeoOp = object.source_geo;

	std::cout << " -> Start : translateGeo : "<< tmpGeoOp->node_name() << std::endl;

	/* Do we already have a node for this geo?
	AtNode *tmpPoly;
	tmpPoly = AiNodeLookUpByName(tmpGeoOp->node_name().c_str());
	if (tmpPoly == 0) {*/
		// create a sphere geometric primitive
		AtNode *tmpPoly = AiNode("polymesh");
		// Name
		std::string nodeName = tmpGeoOp->node_name();
		AiNodeSetStr(tmpPoly, "name", nodeName.c_str());

		int nFaces = object.primitives();
		std::vector<int> nVertIndicesPerFace;
		std::vector<int> vertIndices;
		std::vector<int> nNormIndicesPerFace;
		std::vector<int> normIndices;
		std::vector<Vector3> normVec;
		//
		const PointList* geoPoints = object.point_list();
		int numPoints = geoPoints->size();
		//
		std::vector<Vector3> ptVec;
		ptVec.reserve(numPoints);
		normVec.reserve(numPoints);
		//
		for (int i=0;i<numPoints;i++) {
			   const Vector3& v = (*geoPoints)[i];
			   //Vector4 worldSpace = object.matrix*v;
			   //ptVec.push_back( Vector3( worldSpace.x ,  worldSpace.y ,  worldSpace.z ));
			   ptVec.push_back( v );
		}
		/*
		// Normal
	    const AttribContext* N_ref = object.get_attribcontext("N");
	    const Attribute*      norm = N_ref ? N_ref->attribute : NULL;

	    if(!norm){
	      std::cout << "Missing \"N\" channel from geometry" << std::endl;
	      return;
	    }
	    // we have two possibility:
	    // the uv coordinate are stored in Group_Points or in Group_Vertices way
	    // the same for normal
	    GroupType n_group_type = N_ref->group;    // normal group type
	    // sanity check
	    assert(n_group_type == Group_Points || n_group_type == Group_Vertices);
		 */
		// On recupere les indices de face
		for (int faceIndex = 0;faceIndex<nFaces;faceIndex++)
		{
			   const DD::Image::Primitive * prim =  object.primitive(faceIndex);
			   int nVertices = prim->vertices();
			   nVertIndicesPerFace.push_back( nVertices );
			   for (int verticeIndex = 0 ; verticeIndex < nVertices ; verticeIndex++)
			   {
					   vertIndices.push_back( prim->vertex(verticeIndex)  );
					   //normIndices.push_back( prim->vertex(verticeIndex)  );
			   }
		}
		//
		for (int faceIndex = 0;faceIndex<nFaces;faceIndex++) {
				const DD::Image::Primitive * prim =  object.primitive(faceIndex);
			   int nVertices = prim->vertices();
			   //nNormIndicesPerFace.push_back( nVertices );
			   for (int verticeIndex = 0 ; verticeIndex < nVertices ; verticeIndex++) {
					   unsigned vi = vertIndices[faceIndex];
					   //unsigned ni = n_group_type == Group_Points ? vi : prim->vertex_offset() + faceIndex;
				       // get vertex normal
					   //const Vector3& n = norm->normal(ni);
				       //normVec.push_back( n  );
			   }
		}
		std::cout << "\tnFaces              : " << nFaces << std::endl;
		std::cout << "\tnVertIndicesPerFace : " << nVertIndicesPerFace.size() << std::endl;
		std::cout << "\tvertIndices         : " << vertIndices.size() << std::endl;
		std::cout << "\tptVec               : " << ptVec.size() << std::endl;

		// Vertex par face
		AtArray *nsidesTmp = AiArrayAllocate((int)nVertIndicesPerFace.size(), 1,AI_TYPE_UINT);
		for(uint i = 0; (i < nVertIndicesPerFace.size()); i++)
			AiArraySetUInt(nsidesTmp, i, nVertIndicesPerFace[i]);
		AiNodeSetArray(tmpPoly, "nsides", nsidesTmp);
		// Vertex index par face
		AtArray *vidxsTmp = AiArrayAllocate((int)vertIndices.size(), 1,AI_TYPE_UINT);
		for(uint i = 0; (i < vertIndices.size()); i++)
			AiArraySetUInt(vidxsTmp, i, vertIndices[i]);
		AiNodeSetArray(tmpPoly, "vidxs", vidxsTmp);
		AtArray *normIdxTmp = AiArrayAllocate((int)normIndices.size(), 1,AI_TYPE_UINT);
		for(uint i = 0; (i < normIndices.size()); i++)
			AiArraySetUInt(normIdxTmp, i, normIndices[i]);
		AiNodeSetArray(tmpPoly, "nidxs", normIdxTmp);
		// Vertex
		AtArray *pntTmp = AiArrayAllocate((int)ptVec.size(), 1,AI_TYPE_POINT);
		for(uint i = 0; (i < ptVec.size()); i++) {
			AtPoint tmpPnt = AtPoint();
			tmpPnt.x = ptVec[i].x;
			tmpPnt.y = ptVec[i].y;
			tmpPnt.z = ptVec[i].z;
			AiArraySetPnt(pntTmp, i, tmpPnt);
		}
		AiNodeSetArray(tmpPoly, "vlist", pntTmp);
		/* Normal
		AtArray *normTmp = AiArrayAllocate((int)normVec.size(), 1,AI_TYPE_VECTOR);
		for(uint i = 0; (i < ptVec.size()); i++){
			AtVector tmpPnt = AtVector();
			tmpPnt.x = normVec[i].x;
			tmpPnt.y = normVec[i].y;
			tmpPnt.z = normVec[i].z;
			AiArraySetVec(normTmp, i, tmpPnt);
		}
		AiNodeSetArray(tmpPoly, "nlist", normTmp);*/
		// Smoothing
		AiNodeSetBool(tmpPoly, "smoothing", true);
	//}
	// Transform
	Matrix4  tmpMat = object.matrix;
	AtMatrix mTmpMat;
	mCpMat(tmpMat, mTmpMat);
	AiNodeSetMatrix(tmpPoly, "matrix", mTmpMat);


	// We export the shader for the geometry
	std::cout << "\tMaterial : " << object.material->Class()<< std::endl;
	// Is it a NtoA standard?
	if (strcmp(object.material->Class(), "ntoaStandard") == 0) {
		ntoaStandard * shaderObject = reinterpret_cast<ntoaStandard*> (object.material);
		AiNodeSetPtr(tmpPoly, "shader", shaderObject->AiExport(shaderObject));
	}
	if (strcmp(object.material->Class(), "ntoaShadowCatcher") == 0) {
		// No so we use the shadow catcher
		ntoaShadowCatcher * shaderObject = reinterpret_cast<ntoaShadowCatcher*> (object.material);
		AtNode * shader = shaderObject->AiExport(shaderObject);
		AiNodeSetPtr(shader, "data", static_cast <Iop *> (this)->input(0));
		AiNodeSetPtr(tmpPoly, "shader", shader);
	}
}

void NTOA::flagForUpdate() {
	if ( hash_counter==UINT_MAX )
		hash_counter=0;
	else
		hash_counter++;
	asapUpdate();
	usleep( 0.1 * 1000000 );
}

// The hash value must change or Nuke will think the picture is the same.
void NTOA::append(Hash& hash) {
	hash.append(hash_counter);
}

void NTOA::_validate(bool for_real) {
	std::cout << "+_validate" << std::endl;
	info_.full_size_format(*formats.fullSizeFormat());
	info_.format(*formats.format());
	info_.channels(Mask_RGBA);
	info_.set(format());

	unsigned int tmpScene_hash_counter = 0; // our refresh hash counter

   /* Validate the image input:
   if (input(0))
   {
      input(0)->validate(for_real);
      std::cout << "+_validate input 0 " << input(0)->node_name() << " " << input(0)->Class() << std::endl;
   }
   if (input(1))
   {
      input(1)->validate(for_real);
      std::cout << "+_validate input 1 " << input(1)->node_name() << " " << input(1)->Class() << std::endl;
   }
   if (input(2))
   {
      input(2)->validate(for_real);
      std::cout << "+_validate input 2 " << input(2)->node_name() << " " << input(2)->Class() << std::endl;
  }
   if (input(3))
   {
      input(3)->validate(for_real);
      std::cout << "+_validate input 3 " << input(3)->node_name() << " " << input(3)->Class() << std::endl;
  }*/


	bool mDoRestart = false;
	// On recupere la camera
	CameraOp* tmpCam;
	tmpCam = render_camera();
	if (tmpCam != 0) {
		// On la valide
		tmpCam->validate(for_real);
		Hash tmpHash;
		tmpHash = tmpCam->hash();
		tmpScene_hash_counter = tmpHash.getHash();
		//std::cout << tmpCam->Class() << " " << tmpCam->node_name() << " " << tmpCam->focal_length();
		//std::cout << " " << tmpCam->matrix() << std::endl;
		//std::cout << "tmpCam != 0" << std::endl;
		//mDoRestart    = true;
	}
	// On recupere la geo
	GeoOp*       tmpGeo;
	GeoInfo      m_geoInfos;
	GeometryList m_geoList;
	tmpGeo = render_geo();
	if (tmpGeo != 0) {
		// On la valide
		tmpGeo->validate(for_real);
		// variables
		Scene m_scene;
		//std::cout << tmpGeo->Class() << std::endl;
		// SCENE
		if (strcmp(tmpGeo->Class(), "Scene") == 0) {
			// On recupere la scene
			tmpGeo->build_scene(m_scene);
			Scene * node = &m_scene;
			if (node!=NULL) {
				node->evaluate_lights();
				// Pour chaque LightContext
				int tmpLightCount = 0;
				const unsigned n = node->lights.size();
				for (unsigned i = 0; i < n; i++) {
					LightContext& ltx = *(node->lights[i]);
					ltx.light()->validate(for_real);
					Hash tmpHash;
					tmpHash = ltx.light()->hash();
					tmpScene_hash_counter += tmpHash.getHash();
				}
				// Geo???
				GeometryList * 	m_geoList;
				m_geoList = node->object_list();
				int tmpGeoCount = 0;
				if (m_geoList!=0) {
					//std::cout << "Object(s) in m_geoList : " << m_geoList->objects()<< std::endl;
					for (unsigned i = 0; i <m_geoList->objects(); i++) {
						GeoInfo & object = m_geoList->object(i);
						GeoOp * tmpGeoOp = object.source_geo;
						tmpGeoOp->validate(for_real);
						Hash tmpHash;
						tmpHash = tmpGeoOp->hash(Group_Points);
						tmpScene_hash_counter += tmpHash.getHash();
						tmpHash = tmpGeoOp->hash(Group_Matrix);
						tmpScene_hash_counter += tmpHash.getHash();
						tmpScene_hash_counter += object.material->hash().getHash();
					}
				}
			}
		} else {
			tmpGeo->validate(for_real);
		}
	}
	//
	if (tmpScene_hash_counter != scene_hash_counter) {
		scene_hash_counter = tmpScene_hash_counter;
	}
}

void NTOA::_open() {
	std::cout << " -> Start : _open"<< std::endl;
	// Init buffer
	m_buffer.init(formats.format()->width(),formats.format()->height());
	// start an Arnold session, now all call to Arnold API is safe
	startArnold();

	// We now process the scene graph
	translateScene();

	//AiASSWrite("NtoA.ass", AI_NODE_ALL, false);
	AiRender(AI_RENDER_MODE_CAMERA);

	// at this point we can shut down Arnold
	stopArnold();
	//flagForUpdate();

}

void NTOA::engine(int y, int xx, int r, ChannelMask channels, Row& row)
{
	//
	float *rOut = row.writable(Chan_Red)   + xx;
	float *gOut = row.writable(Chan_Green) + xx;
	float *bOut = row.writable(Chan_Blue)  + xx;
	float *aOut = row.writable(Chan_Alpha) + xx;
	//
	const float *END = rOut + (r - xx);
	//
	unsigned int xxx = static_cast<unsigned int> (xx);
	unsigned int yyy = static_cast<unsigned int> (y);
	// don't have a buffer yet
	m_buffer.m_mutex.lock();
	if ( m_buffer._width==0 && m_buffer._height==0 ) {
		while (rOut < END) {
			*rOut = *gOut = *bOut = *aOut = 0.25f;
			++rOut;
			++gOut;
			++bOut;
			++aOut;
			++xxx;

		}
	} else {
		while (rOut < END) {
			if ( xxx >= m_buffer._width || yyy >= m_buffer._height ) {
				*rOut = *gOut = *bOut = *aOut = 0.f;
			} else {
				*rOut = m_buffer.get(xxx, yyy).r;
				*gOut = m_buffer.get(xxx, yyy).g;
				*bOut = m_buffer.get(xxx, yyy).b;
				*aOut = m_buffer.get(xxx, yyy).a;
			}
			++rOut;
			++gOut;
			++bOut;
			++aOut;
			++xxx;
		}
	}
	m_buffer.m_mutex.unlock();
}

