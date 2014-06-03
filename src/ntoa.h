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
//#include "DDImage/GroupType.h"

using namespace DD::Image;

#include "cBuffer.h"

extern AtNodeMethods* ntoa_driver_std;
extern AtNodeMethods* fb_ShadowMtd;

class NTOA : public RenderScene
{
public:

	NTOA(Node* node);

	~NTOA();

	void translateOptions(NTOA * node);

	void translateSky(NTOA * node);

	void translateScene(NTOA * node);

	void translateCamera(CameraOp* nukeCamera);

	void translateLight(LightOp* nukeLight);

	void translateGeo(GeoInfo& object);

	void flagForUpdate();

	void increment();

	void append(Hash& hash);

	CameraOp* m_getCam();

	GeoOp* m_getGeo();

	void mAbort();

	// Nuke call order
	void _validate(bool for_real);

	//void _request(int x, int y, int r, int t, ChannelMask channels, int count);

	void _open();

	void engine(int y, int xx, int r, ChannelMask channels, Row& row);

	//void _close();

	void knobs(Knob_Callback f);

	int knob_changed(Knob* knb);

	const char* Class() const { return "ArnoldRender"; }

	const char* displayName() const { return "ntoa"; }

	const char* node_help() const { return "Nuke to Arnold"; }

public:
	static const Iop::Description d;
	unsigned     position;
	double       speed;
	bool         doAbort;
	Lock         m_mutex; // mutex for locking the pixel buffer
    cBuffer      m_buffer; // our pixel buffer
    FormatPair   formats;
	unsigned int hash_counter; // our refresh hash counter
	unsigned int scene_hash_counter; // our refresh hash counter
	// ARNOLD GLOBALS
	const char * m_assFile;
	int          m_aa_samples;
	int          m_diffuse_depth;
	int          m_glossy_depth;
	int          m_reflection_depth;
	int          m_refraction_depth;
	int          m_total_depth;
	int          m_diffuse_samples;
	int          m_sss_hemi_samples;
	int          m_glossy_samples;
	// CAMERA OVERRIDE
	int          m_camera_count;
	bool         m_cam_dof;
	float        m_focal_distance;
	float        m_aperture_size;
	int          m_aperture_blades;
	float        m_aperture_rotation;
	float        m_aperture_blade_curvature;
	float        m_aperture_aspect_ratio;
	// SKY
	bool         m_sky_active;
	float        m_sky_color[3];
	float        m_sky_intensity;
	bool         m_sky_visibility;
	// LIGHTS
	int          m_light_count;
	// GEOS
	int          m_geo_count;
};

#endif /* NTOA_H_ */
