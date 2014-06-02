/*
 * ntoa.h
 *
 *  Created on: Jan 20, 2011
 *      Author: ndu
 */

#ifndef NTOA_H_
#define NTOA_H_

#define RESX 640
#define RESY 480

#include <ai.h>


#include <time.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <sstream>

#include <memory>
#include <cstdarg>
#include <cstdio>
#include <limits.h>

// class name
static const char* const CLASS = "ArnoldRender";
static const char* const HELP = "Nuke to Arnold.";

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

class NTOA : public RenderScene
{
public:

	unsigned position;
	double speed;
	bool killthread;
	bool doAbort;
	bool inRender;
	Lock          m_mutex; // mutex for locking the pixel buffer
    cBuffer       m_buffer; // our pixel buffer
    FormatPair    formats;
	unsigned int hash_counter; // our refresh hash counter
	unsigned int scene_hash_counter; // our refresh hash counter
	/*
	 *  Arnold params
	 */
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
	// CAMERA
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

	NTOA(Node* node);
  // Destroying the Op should get rid of the parallel threads.
  // Unfortunatly currently Nuke does not destroy one of the Ops on a
  // deleted node, as it is saving it for Undo. This bug will be fixed
  // in an upcoming version, so you should implement this:
  ~NTOA();
  void flagForUpdate();
  void increment();
  // The hash value must change or Nuke will think the picture is the
  // same. If you can't determine some id for the picture, you should
  // use the current time or something.
  void append(Hash& hash);
  // This is the code for the render thread. This thread is used only to run the AiRender() process outside of the main thread.
  //static unsigned int RenderThread(AtVoid* data);
  CameraOp* m_getCam();
  GeoOp* m_getGeo();
  void _validate(bool for_real);
  void mAbort();
  static void  mRender(unsigned index, unsigned nthreads, void* data);
  void engine(int y, int xx, int r, ChannelMask channels, Row& row);
  void knobs(Knob_Callback f);

  int knob_changed(Knob* knb);

	const char* displayName() const { return "ntoa"; }

	  const char* Class() const { return CLASS; }
	  const char* node_help() const { return HELP; }
	  static const Iop::Description d;
};


#endif /* NTOA_H_ */
