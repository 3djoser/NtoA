//
// ArnoldRender.cpp
//
//  Created on: Apr 27, 2011
//      Author: ndu
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

#define RESX 2048
#define RESY 1556

static const char* const CLASS = "ArnoldRender";
static const char* const HELP  = "Arnold render node";

#include <time.h>
#include <sstream>
#include <cstdio>
#include <limits.h>

#include "DDImage/Op.h"
#include "DDImage/RenderScene.h"
#include "DDImage/Thread.h"
#include "DDImage/Format.h"
#include "DDImage/Row.h"
#include "DDImage/CameraOp.h"
#include "DDImage/Knobs.h"
#include "DDImage/GeoOp.h"
#include "DDImage/GeoInfo.h"

#include <ai.h>

using namespace DD::Image;

static void CheckForUpdate(unsigned, unsigned, void* d);
static void RenderLoop(unsigned index, unsigned nThreads, void* d);

void mCpMat(Matrix4 & tmpMat,AtMatrix & mTmpMat)
{
   mTmpMat[0][0] = tmpMat.a00;
   mTmpMat[1][0] = tmpMat.a01;
   mTmpMat[2][0] = tmpMat.a02;
   mTmpMat[3][0] = tmpMat.a03;
   mTmpMat[0][1] = tmpMat.a10;
   mTmpMat[1][1] = tmpMat.a11;
   mTmpMat[2][1] = tmpMat.a12;
   mTmpMat[3][1] = tmpMat.a13;
   mTmpMat[0][2] = tmpMat.a20;
   mTmpMat[1][2] = tmpMat.a21;
   mTmpMat[2][2] = tmpMat.a22;
   mTmpMat[3][2] = tmpMat.a23;
   mTmpMat[0][3] = tmpMat.a30;
   mTmpMat[1][3] = tmpMat.a31;
   mTmpMat[2][3] = tmpMat.a32;
   mTmpMat[3][3] = tmpMat.a33;
};

class ArnoldRender : public RenderScene
{
   public:
      bool needUpdate;
      bool killthreadUpdate;
      bool killthreadRender;
      bool initRender;
      bool startRender;
      bool endRender;
      bool inRender;
      // Buffer
		std::vector<AtRGBA> _data;
		unsigned int        _width;
		unsigned int        _height;
		Lock                _mutex; // mutex for locking the pixel buffer
		//
	   FormatPair          formats;
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

	  ArnoldRender(Node* node) : RenderScene(node)
	  {
			_width  = 0;
			_height = 0;
			m_light_count = 0;
			m_geo_count   = 0;
			/*
			 *  Arnold params
			 */
			m_aa_samples 					= 0;
			m_diffuse_depth 				= 1;
			m_glossy_depth 				= 1;
			m_reflection_depth 			= 1;
			m_refraction_depth 			= 1;
			m_total_depth 					= 6;
			m_diffuse_samples 			= 1;
			m_sss_hemi_samples 			= 1;
			m_glossy_samples 				= 1;
			// CAMERA
			m_cam_dof = false;
			m_focal_distance 				= 1.0;
			m_aperture_size 			   = 0.0;
			m_aperture_blades 		   = 6;
			m_aperture_rotation 		   = 0.0f;
			m_aperture_blade_curvature = 1.0f;
			m_aperture_aspect_ratio    = 1.0f;
			// SKY
			m_sky_active     				= false;
		   m_sky_color      				= {1.0f, 1.0f, 1.0f };
			m_sky_intensity  				= 1.0f;
			m_sky_visibility 				= false;

			//initBuffer(formats.format()->width(),formats.format()->height());
			//initBuffer(_width,_height);
	      killthreadUpdate = false;
	      killthreadRender = false;
	      inRender    = false;
		   needUpdate = false;
	      startRender = false;
	      endRender   = false;
	      initRender  = false;
	      Thread::spawn(::CheckForUpdate, 1, this);
	      Thread::spawn(::RenderLoop, 1, this);
	  }

	  // Destroying the Op should get rid of the parallel threads.
	  // Unfortunatly currently Nuke does not destroy one of the Ops on a
	  // deleted node, as it is saving it for Undo. This bug will be fixed
	  // in an upcoming version, so you should implement this:
	  ~ArnoldRender()
	  {
			std::cout << "NUKE : ~ArnoldRender()" << std::endl;
			while(inRender){
				while(AiRendering()){
					AiRenderAbort();
				}
				AiEnd();
			}
			killthreadUpdate = true;
			killthreadRender = true;
			std::cout << "wait 0" << std::endl;
			Thread::wait(this);
			std::cout << "wait 1" << std::endl;
			Thread::wait(this);
			std::cout << "wait 2" << std::endl;
	  }

	  CameraOp* m_getCam()
	  {
	  	return render_camera();
	  }

	  GeoOp* m_getGeo()
	  {
	  	return render_geo();
	  }

	  void flagForUpdate()
	  {
	  	//std::cout << "flagForUpdate-start" << std::endl;
	  	if ( hash_counter==UINT_MAX )
	  		hash_counter=0;
	  	else
	  		hash_counter++;
		  needUpdate = true;
	  };
	  // The hash value must change or Nuke will think the picture is the
	  // same. If you can't determine some id for the picture, you should
	  // use the current time or something.
	  void append(Hash& hash)
	  {
	  	hash.append(hash_counter);
	  };

	  void _validate(bool for_real)
	  {
			std::cout << "NUKE : Validate" << std::endl;
		   bool mDoRestart = false;

			info_.full_size_format(*formats.fullSizeFormat());
			info_.format(*formats.format());
			info_.channels(Mask_RGBA);
			info_.set(format());
         Format & tmpFormat = (Format &)format();
         if (_width!=tmpFormat.width() || _height!=tmpFormat.height() )
         {
            _width  = tmpFormat.width();
            _height = tmpFormat.height();
            initBuffer(_width,_height);
            mDoRestart    = true;
         }
			 // Hash up knobs that affect the UV attributes
			 Hash knob_hash;
			 knob_hash.reset();
			 // Hash rest of local knobs:
			 knob_hash.append(m_aa_samples);
			 knob_hash.append(m_diffuse_depth);
			 knob_hash.append(m_glossy_depth);
			 knob_hash.append(m_reflection_depth);
			 knob_hash.append(m_refraction_depth);
			 knob_hash.append(m_total_depth);
			 knob_hash.append(m_diffuse_samples);
			 knob_hash.append(m_sss_hemi_samples);
			 knob_hash.append(m_glossy_samples);
			// CAMERA
			 knob_hash.append(m_camera_count);
			 knob_hash.append(m_cam_dof);
			 knob_hash.append(m_focal_distance);
			 knob_hash.append(m_aperture_size);
			 knob_hash.append(m_aperture_blades);
			 knob_hash.append(m_aperture_rotation);
			 knob_hash.append(m_aperture_blade_curvature);
			 knob_hash.append(m_aperture_aspect_ratio);
			// SKY
			 knob_hash.append(m_sky_active);
			 knob_hash.append(m_sky_color[0]);
			 knob_hash.append(m_sky_color[1]);
			 knob_hash.append(m_sky_color[2]);
			 knob_hash.append(m_sky_intensity);
			 knob_hash.append(m_sky_visibility);


			// Our temporary refresh hash counter
			unsigned int tmpScene_hash_counter = knob_hash.getHash();

			// We validate the inputs
			if (input(0))
			{
				input(0)->validate(for_real);
				//std::cout << "+_validate input 0 " << input(0)->node_name() << " " << input(0)->Class() << std::endl;
			}
			if (input(1))
			{
				input(1)->validate(for_real);
				//std::cout << "+_validate input 1 " << input(1)->node_name() << " " << input(1)->Class() << std::endl;
			}
			if (input(2))
			{
				input(2)->validate(for_real);
				//std::cout << "+_validate input 2 " << input(2)->node_name() << " " << input(2)->Class() << std::endl;
			}
		   if (input(3))
		   {
		      input(3)->validate(for_real);
		      //std::cout << "+_validate input 3 " << input(3)->node_name() << " " << input(3)->Class() << std::endl;
		  }
			// On recupere la camera
			CameraOp* tmpCam;
			tmpCam = render_camera();
			if (tmpCam != 0)
			{
				// On la valide
				tmpCam->validate(for_real);
				Hash tmpHash;
				tmpHash = tmpCam->hash();
				tmpScene_hash_counter += tmpHash.getHash();
				//std::cout << tmpCam->Class() << " " << tmpCam->node_name() << " " << tmpCam->focal_length();
				//std::cout << " " << tmpCam->matrix() << std::endl;
				//std::cout << "tmpCam != 0" << std::endl;
				//mDoRestart    = true;
			}

			// On recupere la geo
			GeoOp*       tmpGeo;
			GeoInfo      m_geoInfos;
			GeometryList m_geoList;
			tmpGeo = m_getGeo();
			if (tmpGeo != 0)
			{
				// On la valide
				tmpGeo->validate(for_real);
				// variables
				Scene m_scene;
				//std::cout << tmpGeo->Class() << std::endl;
				// SCENE
				if       (strcmp(tmpGeo->Class(), "Scene") == 0)
				{
					// On recupere la scene
					tmpGeo->build_scene(m_scene);
					Scene * node = &m_scene;
					if (node!=0)
					{
						//node->validate(for_real);
						//std::cout << "gniiiiiiiiiiiiiiiiiiiiiiiiiiii" << std::endl;
						//node->evaluate_lights();
						// Pour chaque LightContext
						int tmpLightCount = 0;
						const unsigned n = node->lights.size();
						for (unsigned i = 0; i < n; i++) {
							LightContext& ltx = *(node->lights[i]);
							ltx.light()->validate(for_real);
							Hash tmpHash;
							tmpHash = ltx.light()->hash();
							tmpScene_hash_counter += tmpHash.getHash();
							//std::cout << ltx.light()->node_name() << " " << ltx.matrix(0) <<std::endl;
							tmpLightCount++;
						}
						// Changement du nombre de light, on relance le render
						if (m_light_count != tmpLightCount)
						{
							m_light_count = tmpLightCount;
							std::cout << "NUKE : Light(s) count differ. Restart need. " <<  m_light_count << " " << tmpLightCount << std::endl;
							mDoRestart    = true;
						}
						// Geo???
						int tmpGeoCount = 0;
						GeometryList * 	m_geoList;
						m_geoList = node->object_list();
						if (m_geoList!=0)
						{
							//std::cout << "RENDER : Object(s) in m_geoList : " << m_geoList->objects()<< std::endl;
							for (unsigned i = 0; i <m_geoList->objects(); i++) {
								GeoInfo & object = m_geoList->object(i);
								GeoOp * tmpGeoOp = object.source_geo;
								tmpGeoOp->validate(for_real);
								Hash tmpHash;
								/*
								tmpHash = tmpGeoOp->hash(Group_Points);
								tmpScene_hash_counter += tmpHash.getHash();
								tmpHash = tmpGeoOp->hash(Group_Matrix);
								tmpScene_hash_counter += tmpHash.getHash();
								*/
								tmpHash = tmpGeoOp->hash(0);
								tmpScene_hash_counter += tmpHash.getHash();
								tmpScene_hash_counter += object.material->hash().getHash();

								//std::cout << "GeoInfo material name : " << object.material->node_name()<< std::endl;
								tmpGeoCount++;
							}
						}
						// Changement du nombre de geo, on relance le render
					if (m_geo_count != tmpGeoCount)
					{
						std::cout << "NUKE : Object(s) count differ. Restart need. " << m_geo_count << " " << tmpGeoCount << std::endl;
					   m_geo_count = tmpGeoCount;
					   mDoRestart  = true;
					}

					}
				}/*else if (strcmp(tmpGeo->Class(), "ReadGeo2") == 0)
				{
					ReadGeo * node = reinterpret_cast<ReadGeo*> (tmpGeo);
					std::cout << node->filename() << std::endl;
				}else if (strcmp(tmpGeo->Class(), "Card2") == 0)
				{
					tmpGeo->build_scene(m_scene);
					tmpGeo->get_geometry(m_scene,m_geoList);
					std::cout << "Objects : " << m_geoList.objects() << std::endl;
					//m_geoInfos = m_geoList.object0();
					m_geoInfos.print_info(std::cout);
					//std::cout << "primitives  : " << m_geoInfos.primitives() << std::endl;
				}
				//Cube
				//Cylinder
				//Sphere
				//
				//
				//tmpGeo->print_info(std::cout);
				 */
				else
				{
					tmpGeo->validate(for_real);
				 //mDoRestart  = true;
				}
			}
			if (m_light_count!=0 && m_geo_count!=0)
			{
				//std::cout << "m_light_count : " << m_light_count << std::endl;
				//std::cout << "m_geo_count   : " << m_geo_count << std::endl;
				if (mDoRestart)
				{
					std::cout << "NUKE : request initRender" << std::endl;
					initRender = true;
				}
				else
				{
					//
					if (tmpScene_hash_counter != scene_hash_counter)
					{
						if (inRender)
						{
							std::cout << "NUKE : request interrupt" << std::endl;
							while(AiRendering())
							{
								AiRenderInterrupt();
							}
						}
						std::cout << "NUKE : request startRender" << std::endl;
						scene_hash_counter = tmpScene_hash_counter;
						startRender = true;
					}
				}
			}
	  }

	  void knobs(Knob_Callback f)
	  {
	   Format_knob(f, &formats, "format");
	  	//File_knob(f, &m_assFile, "ass_file", "Ass file");
	  	Int_knob(f, &m_aa_samples, "aa_samples", "AA Samples");
	  	Int_knob(f, &m_diffuse_depth, "m_diffuse_depth", "GI diffuse depth");
	  	Int_knob(f, &m_glossy_depth, "m_glossy_depth", "GI glossy depth");
	  	Int_knob(f, &m_reflection_depth, "m_reflection_depth", "GI reflection depth");
	  	Int_knob(f, &m_refraction_depth, "m_refraction_depth", "GI refraction depth");
	  	Int_knob(f, &m_total_depth, "m_total_depth", "GI total depth");
	  	Int_knob(f, &m_diffuse_samples, "m_diffuse_samples", "GI diffuse samples");
	  	Int_knob(f, &m_sss_hemi_samples, "m_sss_hemi_samples", "GI sss hemi samples");
	  	Int_knob(f, &m_glossy_samples, "m_glossy_samples", "GI glossy samples");
	  	// Camera
	  	Tab_knob(f, "Camera");
	  	Bool_knob(f, &m_cam_dof, "cam_dof", "Camera DOF");
	  	Float_knob(f, &m_focal_distance, "focal_distance", "Camera focal distance");
	  	Float_knob(f, &m_aperture_size, "aperture_size", "Camera aperture size");
	  	Int_knob(f, &m_aperture_blades, "aperture_blades", "Camera aperture blades");
	  	Float_knob(f, &m_aperture_rotation, "aperture_rotation", "Camera aperture rotation");
	  	Float_knob(f, &m_aperture_blade_curvature, "aperture_blade_curvature", "Camera aperture blade curvature");
	  	Float_knob(f, &m_aperture_aspect_ratio, "aperture_aspect_ratio", "Camera aperture aspect ratio");
	  	// Sky
	  	Tab_knob(f, "Sky");
	  	Bool_knob(f, &m_sky_active, "sky_active", "Active");
	  	Color_knob(f, (float*)&m_sky_color, "sky_color", "Color");
	  	Float_knob(f, &m_sky_intensity, "sky_intensity", "Intensity");
	  	Bool_knob(f, &m_sky_visibility, "sky_visibility", "Visibility");
	  	//AiNodeSetRGB(sky, "color", 1.0f, 1.0f, 1.0f );*/
	  };

	  int knob_changed(Knob* knb)
	  {
	  	if (knb != NULL) {
	  		if       (knb->name() && strcmp(knb->name(), "format") == 0)
	  		{
	  			Format & tmpFormat = (Format &)format();
	  			//initBuffer(tmpFormat.width(),tmpFormat.height());
				_width  = tmpFormat.width();
				_height = tmpFormat.height();
				//initRender = true;
		  		return 1;
	  		}
			//startRender = true;
	  		return 1;
	  	}
	  	return 0;
	  };

	  void engine(int y, int x, int r, ChannelMask c, Row& out)
	  {
			//
			float *rOut = out.writable(Chan_Red)   + x;
			float *gOut = out.writable(Chan_Green) + x;
			float *bOut = out.writable(Chan_Blue)  + x;
			float *aOut = out.writable(Chan_Alpha) + x;
			//
			const float *END = rOut + (r - x);
			//
			unsigned int xxx = static_cast<unsigned int> (x);
			unsigned int yyy = static_cast<unsigned int> (y);
			// don't have a buffer yet
			_mutex.lock();
			if ( _width==0 && _height==0 )
			{
				while (rOut < END)
				{
					*rOut = *gOut = *bOut = *aOut = 0.25f;
					++rOut;
					++gOut;
					++bOut;
					++aOut;
					++xxx;

				}
			}
			else
			{
				while (rOut < END)
				{
					if ( xxx >= _width || yyy >= _height )
					{
						*rOut = *gOut = *bOut = *aOut = 0.f;
					}
					else
					{
						*rOut = get(xxx, yyy).r;
						*gOut = get(xxx, yyy).g;
						*bOut = get(xxx, yyy).b;
						*aOut = get(xxx, yyy).a;
					}
					++rOut;
					++gOut;
					++bOut;
					++aOut;
					++xxx;
				}
			}
			_mutex.unlock();
	  }

		void initBuffer(const unsigned int width, const unsigned int height)
		{
			_width  = width;
			_height = height;
			_mutex.lock();
			_data.clear();
			_data.resize(_width * _height);
			_mutex.unlock();
			std::cout << "NUKE : Buffer X Y : " << width << " " << height << std::endl;
		};

		AtRGBA& get(unsigned int x, unsigned int y)
		{
			unsigned int index = (_width * y) + x;
			return _data[index];
		};

		const AtRGBA& get(unsigned int x, unsigned int y) const
		{
			unsigned int index = (_width * y) + x;
			return _data[index];
		};

		const unsigned int size() const
		{
			return _data.size();
		};
	  const char* Class() const { return CLASS; }
	  const char* node_help() const { return HELP; }
	  static const Iop::Description d;

};
/*
 * Thread looking for updating nuke framebuffer
 */
static void CheckForUpdate(unsigned index, unsigned nThreads, void* d)
{
   while (!((ArnoldRender*)d)->killthreadUpdate) {
   	//if (((ArnoldRender*)d)->inRender)
   	//{
			//TODO: verifier que le buffer a ete update
			if (((ArnoldRender*)d)->needUpdate)
			{
				((ArnoldRender*)d)->asapUpdate();
				std::cout << "NUKE : asapUpdate requested" << std::endl;
				((ArnoldRender*)d)->needUpdate = false;
			}
   	//}
		usleep(0.25 * 1000000);
   }
}

static Iop* build(Node* node) { return new ArnoldRender(node); }
const Iop::Description ArnoldRender::d(CLASS, 0, build);

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * ARNOLD PART
 *
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
extern AtNodeMethods* ntoa_driver_std;
extern AtNodeMethods* fb_ShadowMtd;

class ntoaStandard : public Iop
{
public:
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


  ntoaStandard(Node* node);

  int minimum_inputs();
  int maximum_inputs();

  void knobs(Knob_Callback f);

  void _validate(bool);

  void engine(int y, int xx, int r, ChannelMask channels, Row& row);

  const char* Class() const ;
  const char* displayName() const ;
  const char* node_help() const ;
  static const Description desc;

};
static void RenderLoop(unsigned index, unsigned nThreads, void* d)
{
	ArnoldRender * node = static_cast<ArnoldRender*> (d);

   while (!node->killthreadRender) {
		// Options
		AtNode *options;
   	if (node->initRender)
   	{
			std::cout << "RENDER LOOP : Init render" << std::endl;
   		//
   		while(node->inRender){
   			while(AiRendering()){
   				AiRenderAbort();
   			}
   			AiEnd();
   		}
			// Start an Arnold session
			AiBegin();
			// Options
			options = AiUniverseGetOptions();
			AiNodeSetInt(options, "threads", 6);
			AiNodeSetInt(options, "preserve_scene_data", true);
			AiMsgSetLogFileName("scene1.log");
			AiNodeInstall(AI_NODE_DRIVER, AI_TYPE_RGBA, "ntoa_drv", "<built-in>", ntoa_driver_std, AI_VERSION);
			AiNodeInstall(AI_NODE_SHADER, AI_TYPE_RGBA, "fb_ShadowMtd", "<built-in>", fb_ShadowMtd, AI_VERSION);
			std::cout << "RENDER LOOP : Render format " << node->_width << " " << node->_height << std::endl;
			//node->initBuffer(node->_width,node->_height);
			AiNodeSetInt(options, "xres", node->_width);
			AiNodeSetInt(options, "yres", node->_height);

			// create a gaussian filter node
			AtNode *filter = AiNode("gaussian_filter");
			AiNodeSetStr(filter, "name", "myfilter");

			// create an output driver node
			AtNode *driver = AiNode("ntoa_drv");
			AiNodeSetStr(driver, "name", "mydriver");
			AiNodeSetPtr(driver, "data", node);

			// assign the driver and filter to the main (beauty) AOV, which is called "RGB"
			AtArray *outputs_array = AiArrayAllocate(1, 1, AI_TYPE_STRING);
			AiArraySetStr(outputs_array, 0, "RGBA RGBA myfilter mydriver");
			AiNodeSetArray(options, "outputs", outputs_array);

			// No Init
			node->initRender  = false;
			node->startRender = true;
			node->endRender   = false;
			std::cout << "RENDER LOOP : Init render done!" << std::endl;
   	}
   	if (node->startRender)
   	{
			std::cout << "RENDER LOOP : Start render" << std::endl;
			node->inRender = true;
			// Options
			options = AiUniverseGetOptions();
			// We create a pointer for an arnold light node
			AtNode *camera;
			// Do we already have a node for this light?
			camera =	AiNodeLookUpByName("ntoaCamera");
			// No create one
			if (camera == 0)
			{
				std::cout << "RENDER LOOP : Create camera" << std::endl;
				camera = AiNode("persp_camera");
				AiNodeSetStr(camera, "name", "ntoaCamera");
				// set the active camera (optional, since there is only one camera)
				AiNodeSetPtr(options, "camera", camera);
			}
			// Transform
			Matrix4  tmpMat = node->m_getCam()->matrix();
			AtMatrix mTmpMat;
			mCpMat(tmpMat, mTmpMat);
			AiNodeSetMatrix(camera, "matrix", mTmpMat);
			// Dof
			AiNodeSetFlt(camera, "fov", (float)node->m_getCam()->focal_length());
			if (node->m_cam_dof)
			{
				AiNodeSetFlt(camera, "focal_distance", (float)node->m_focal_distance);
				AiNodeSetFlt(camera, "aperture_size", (float)node->m_aperture_size);
			}else{
				AiNodeSetFlt(camera, "focal_distance", 1);
				AiNodeSetFlt(camera, "aperture_size", 0);
			}
			AiNodeSetInt(camera, "aperture_blades", (int)node->m_aperture_blades);
			AiNodeSetFlt(camera, "aperture_rotation", (float)node->m_aperture_size);
			AiNodeSetFlt(camera, "aperture_blade_curvature", (float)node->m_aperture_size);
			AiNodeSetFlt(camera, "aperture_aspect_ratio", (float)node->m_aperture_size);

			// Options
			AtNode *options = AiUniverseGetOptions();
			// get the global options node and set some options
			AiNodeSetInt(options, "AA_samples", node->m_aa_samples);
			AiNodeSetInt(options, "GI_diffuse_depth"   , node->m_diffuse_depth);
			AiNodeSetInt(options, "GI_glossy_depth"    , node->m_glossy_depth);
			AiNodeSetInt(options, "GI_reflection_depth", node->m_reflection_depth);
			AiNodeSetInt(options, "GI_refraction_depth", node->m_refraction_depth);
			AiNodeSetInt(options, "GI_total_depth"     , node->m_total_depth);
			AiNodeSetInt(options, "GI_diffuse_samples" , node->m_diffuse_samples);
			AiNodeSetInt(options, "GI_sss_hemi_samples", node->m_sss_hemi_samples);
			AiNodeSetInt(options, "GI_glossy_samples"  , node->m_glossy_samples);

			// Sky
			// We create a pointer for an arnold sky node
			AtNode *sky;
			// Do we already have a node for this sky?
			sky =	AiNodeLookUpByName("ntoaSky");
			// No create one
			if (sky == 0)
			{
				std::cout << "RENDER LOOP : Create sky" << std::endl;
				sky = AiNode("sky");
				AiNodeSetStr(sky, "name", "ntoaSky");
			}
			AiNodeSetRGB(sky, "color", 1.0f, 1.0f, 1.0f );
			if (node->m_sky_active)
			{
				AiNodeSetFlt(sky, "intensity", node->m_sky_intensity);
				// Visibility options
				AtInt visibility = 65535;

				//if (!fnDagNode.findPlug("castsShadows").asBool())
				//   visibility &= ~AI_RAY_SHADOW;

				if (!node->m_sky_visibility) visibility &= ~AI_RAY_CAMERA;

				//if (!fnDagNode.findPlug("visibleInReflections").asBool())
				//   visibility &= ~AI_RAY_REFLECTED;

				//if (!fnDagNode.findPlug("visibleInRefractions").asBool())
				//   visibility &= ~AI_RAY_REFRACTED;

				/*if (customAttributes)
				{
				 if (!fnDagNode.findPlug("diffuse_visibility").asBool())
					visibility &= ~AI_RAY_DIFFUSE;

				 if (!fnDagNode.findPlug("glossy_visibility").asBool())
					visibility &= ~AI_RAY_GLOSSY;
				}*/

				AiNodeSetInt(sky, "visibility", visibility);
				AiNodeSetPtr(options, "background", sky);
			}else{
				AiNodeSetFlt(sky, "intensity", 0);
				AiNodeSetPtr(options, "background", NULL);
			}

			// On recupere la geo
			GeoOp*       tmpGeo;
			GeoInfo      m_geoInfos;
			//GeometryList m_geoList;
			tmpGeo = node->m_getGeo();
			if (tmpGeo != 0)
			{
				// variables
				Scene m_scene;
				//std::cout << tmpGeo->Class() << std::endl;
				if       (strcmp(tmpGeo->Class(), "Scene") == 0)
				{
					// On recupere la scene
					tmpGeo->build_scene(m_scene);
					Scene * nodeScene = &m_scene;
					if (nodeScene!=0)
					{
						//// LIGHTS
						//nodeScene->evaluate_lights();
						// Pour chaque LightContext
						const unsigned n = nodeScene->lights.size();
						for (unsigned i = 0; i < n; i++) {
							LightContext& ltx = *(nodeScene->lights[i]);
							//std::cout << ltx.light()->displayName() << " " << ltx.light()->matrix().array() <<std::endl;
							//std::cout << ltx.light()->Class() << std::endl;

							// We create a pointer for an arnold light node
							AtNode *light;
							// Do we already have a node for this light?
							light =	AiNodeLookUpByName(ltx.light()->node_name().c_str());
							// No create one
							if (light == 0)
							{
								std::cout << "RENDER LOOP : Create light " << ltx.light()->node_name().c_str() << std::endl;
								if       (strcmp(ltx.light()->Class(), "Light") == 0)
								{
									light = AiNode("point_light");
								} else if (strcmp(ltx.light()->Class(), "DirectLight") == 0)
								{
									light = AiNode("distant_light");
								} else if (strcmp(ltx.light()->Class(), "Spotlight") == 0)
								{
									light = AiNode("spot_light");
								}
								AiNodeSetStr(light, "name", ltx.light()->node_name().c_str());

							}
							// Set / Update matrix
							Matrix4  tmpMat = ltx.light()->matrix().array();
							AtMatrix mTmpMat;
							mCpMat(tmpMat, mTmpMat);
							AiNodeSetMatrix(light, "matrix", mTmpMat);
							// Set / Update intensity
							AiNodeSetFlt(light, "intensity", (float)ltx.light()->intensity());
							// Set / Update color
							Pixel & 	color = (Pixel &)ltx.light()->color();
							float * m_tmpColor;
							m_tmpColor = (float *)color.array();
							/*
							std::cout << "Size of color " << sizeof(m_tmpColor) <<std::endl;
							for (int ii=0;ii<sizeof(m_tmpColor);ii++){
								std::cout << "    - " << m_tmpColor[ii]<<std::endl;
							}
							*/
							AiNodeSetRGB(light, "color", m_tmpColor[1],m_tmpColor[2],m_tmpColor[3]);
							// Set / Update shadow
							AiNodeSetBool(light, "cast_shadows", ltx.light()->cast_shadows());
							AiNodeSetFlt(light, "radius", ltx.light()->sample_width());
							AiNodeSetInt(light, "samples", ltx.light()->samples());
						}

						//// GEO
						GeometryList * 	m_geoList;
						m_geoList = nodeScene->object_list();
						//m_geoList->validate(for_real);
						if (m_geoList!=0)
						{
							//std::cout << "Object(s) in m_geoList : " << m_geoList->objects()<< std::endl;
							for (unsigned i = 0; i <m_geoList->objects(); i++) {
								// L'objet
								GeoInfo & object =  m_geoList->object(i);
								// Nuke geo node
								GeoOp * tmpGeoOp = object.source_geo;
								//  We create a pointer for an arnold polymesh node
								AtNode * tmpPoly;
								// Do we already have a node for this geo?
								tmpPoly = AiNodeLookUpByName(tmpGeoOp->node_name().c_str());
								// No create one
								if (tmpPoly == 0)
								{
									std::cout << "RENDER LOOP : Create geo " << tmpGeoOp->node_name().c_str() << std::endl;
									tmpPoly = AiNode("polymesh");
									// Le nom de l'objet
									std::string nodeName = tmpGeoOp->node_name();
									AiNodeSetStr(tmpPoly, "name", nodeName.c_str());
									//std::cout << "GeoInfo object name : " << tmpGeoOp->node_name()<< std::endl;
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
									for (int faceIndex = 0;faceIndex<nFaces;faceIndex++)
									{
											const DD::Image::Primitive * prim =  object.primitive(faceIndex);
											int nVertices = prim->vertices();
											//nNormIndicesPerFace.push_back( nVertices );
											for (int verticeIndex = 0 ; verticeIndex < nVertices ; verticeIndex++)
											{
													//unsigned vi = vertIndices[faceIndex];
													//unsigned ni = n_group_type == Group_Points ? vi : prim->vertex_offset() + faceIndex;
													 // get vertex normal
													//const Vector3& n = norm->normal(ni);
													 //normVec.push_back( n  );
											}
									}
									/*
									std::cout << "startRender----------------------"<< std::endl;
									std::cout << "nFaces              : " << nFaces << std::endl;
									std::cout << "nVertIndicesPerFace : " << nVertIndicesPerFace.size() << std::endl;
									std::cout << "vertIndices         : " << vertIndices.size() << std::endl;
									std::cout << "ptVec               : " << ptVec.size() << std::endl;
									*/
									// Vertex par face
									AtArray *nsidesTmp = AiArrayAllocate((AtInt)nVertIndicesPerFace.size(), 1,AI_TYPE_UINT);
									for(AtUInt i = 0; (i < nVertIndicesPerFace.size()); i++)
										AiArraySetUInt(nsidesTmp, i, nVertIndicesPerFace[i]);
									AiNodeSetArray(tmpPoly, "nsides", nsidesTmp);
									// Vertex index par face
									AtArray *vidxsTmp = AiArrayAllocate((AtInt)vertIndices.size(), 1,AI_TYPE_UINT);
									for(AtUInt i = 0; (i < vertIndices.size()); i++)
										AiArraySetUInt(vidxsTmp, i, vertIndices[i]);
									AiNodeSetArray(tmpPoly, "vidxs", vidxsTmp);
									AtArray *normIdxTmp = AiArrayAllocate((AtInt)normIndices.size(), 1,AI_TYPE_UINT);
									for(AtUInt i = 0; (i < normIndices.size()); i++)
										AiArraySetUInt(normIdxTmp, i, normIndices[i]);
									AiNodeSetArray(tmpPoly, "nidxs", normIdxTmp);
									// Vertex
									AtArray *pntTmp = AiArrayAllocate((AtInt)ptVec.size(), 1,AI_TYPE_POINT);
									for(AtUInt i = 0; (i < ptVec.size()); i++){
										AtPoint tmpPnt = AtPoint();
										tmpPnt.x = ptVec[i].x;
										tmpPnt.y = ptVec[i].y;
										tmpPnt.z = ptVec[i].z;
										AiArraySetPnt(pntTmp, i, tmpPnt);
									}
									AiNodeSetArray(tmpPoly, "vlist", pntTmp);
									/* Normal
									AtArray *normTmp = AiArrayAllocate((AtInt)normVec.size(), 1,AI_TYPE_VECTOR);
									for(AtUInt i = 0; (i < ptVec.size()); i++){
										AtVector tmpPnt = AtVector();
										tmpPnt.x = normVec[i].x;
										tmpPnt.y = normVec[i].y;
										tmpPnt.z = normVec[i].z;
										AiArraySetVec(normTmp, i, tmpPnt);
									}
									AiNodeSetArray(tmpPoly, "nlist", normTmp);*/
									// Smoothing
									AiNodeSetBool(tmpPoly, "smoothing", true);

									//  We create a pointer for an arnold shader node
									AtNode * shader;
									// Do we have an AiStandard ?
									std::cout << "RENDER LOOP : Found material " << object.material->Class()<< std::endl;
									// Yes!
									if (strcmp(object.material->Class(), "ntoaStandard") == 0)
									{
										ntoaStandard * shaderObject = static_cast<ntoaStandard*> (object.material);
										// Do we already have a node for this shader?
										shader = AiNodeLookUpByName(object.material->node_name().c_str());
										// No create one
										if (shader == 0)
										{
											shader = AiNode("standard");
											AiNodeSetStr(shader, "name", object.material->node_name().c_str());
											AiNodeSetPtr(tmpPoly, "shader", shader);
										}
										AiNodeSetFlt(shader, "specular_roughness", object.material->knob("specular_roughness")->get_value());
										AiNodeSetFlt(shader, "specular_anisotropy", object.material->knob("specular_anisotropy")->get_value());
										AiNodeSetFlt(shader, "specular_rotation", object.material->knob("specular_rotation")->get_value());
										AiNodeSetRGB(shader, "transmittance", shaderObject->transmittance[0], shaderObject->transmittance[1], shaderObject->transmittance[2]);
										AiNodeSetFlt(shader, "Kd", object.material->knob("Kd")->get_value());
										AiNodeSetRGB(shader, "Kd_color", shaderObject->Kd_color[0], shaderObject->Kd_color[1], shaderObject->Kd_color[2]);
										AiNodeSetFlt(shader, "diffuse_roughness", object.material->knob("diffuse_roughness")->get_value());
										AiNodeSetFlt(shader, "Ks", object.material->knob("Ks")->get_value());
										AiNodeSetRGB(shader, "Ks_color", shaderObject->Ks_color[0], shaderObject->Ks_color[1], shaderObject->Ks_color[2]);
										AiNodeSetFlt(shader, "Phong_exponent", object.material->knob("Phong_exponent")->get_value());
										AiNodeSetFlt(shader, "Kr", object.material->knob("Kr")->get_value());
										AiNodeSetRGB(shader, "Kr_color", shaderObject->Kr_color[0], shaderObject->Kr_color[1], shaderObject->Kr_color[2]);
										AiNodeSetRGB(shader, "reflection_exit_color", shaderObject->reflection_exit_color[0], shaderObject->reflection_exit_color[1], shaderObject->reflection_exit_color[2]);
										AiNodeSetBool(shader, "reflection_exit_use_environment", object.material->knob("reflection_exit_use_environment")->get_value());
										AiNodeSetFlt(shader, "Kt", object.material->knob("Kt")->get_value());
										AiNodeSetRGB(shader, "Kt_color", shaderObject->Kt_color[0], shaderObject->Kt_color[1], shaderObject->Kt_color[2]);
										AiNodeSetRGB(shader, "refraction_exit_color", shaderObject->refraction_exit_color[0], shaderObject->refraction_exit_color[1], shaderObject->refraction_exit_color[2]);
										AiNodeSetBool(shader, "refraction_exit_use_environment", object.material->knob("refraction_exit_use_environment")->get_value());
										AiNodeSetFlt(shader, "IOR", object.material->knob("IOR")->get_value());
										AiNodeSetFlt(shader, "Kb", object.material->knob("Kb")->get_value());
										AiNodeSetBool(shader, "Fresnel", object.material->knob("Fresnel")->get_value());
										AiNodeSetFlt(shader, "Krn", object.material->knob("Krn")->get_value());
										AiNodeSetBool(shader, "specular_Fresnel", object.material->knob("specular_Fresnel")->get_value());
										AiNodeSetFlt(shader, "Ksn", object.material->knob("Ksn")->get_value());
										AiNodeSetBool(shader, "Fresnel_affect_diff", object.material->knob("Fresnel_affect_diff")->get_value());
										AiNodeSetFlt(shader, "emission", object.material->knob("emission")->get_value());
										AiNodeSetRGB(shader, "emission_color", shaderObject->emission_color[0], shaderObject->emission_color[1], shaderObject->emission_color[2]);
										AiNodeSetFlt(shader, "direct_specular", object.material->knob("direct_specular")->get_value());
										AiNodeSetFlt(shader, "indirect_specular", object.material->knob("indirect_specular")->get_value());
										AiNodeSetFlt(shader, "direct_diffuse", object.material->knob("direct_diffuse")->get_value());
										AiNodeSetFlt(shader, "indirect_diffuse", object.material->knob("indirect_diffuse")->get_value());
										AiNodeSetBool(shader, "enable_glossy_caustics", object.material->knob("enable_glossy_caustics")->get_value());
										AiNodeSetBool(shader, "enable_reflective_caustics", object.material->knob("enable_reflective_caustics")->get_value());
										AiNodeSetBool(shader, "enable_refractive_caustics", object.material->knob("enable_refractive_caustics")->get_value());
										AiNodeSetBool(shader, "enable_internal_reflections", object.material->knob("enable_internal_reflections")->get_value());
										AiNodeSetFlt(shader, "Ksss", object.material->knob("Ksss")->get_value());
										AiNodeSetRGB(shader, "Ksss_color", shaderObject->Ksss_color[0], shaderObject->Ksss_color[1], shaderObject->Ksss_color[2]);
										AiNodeSetRGB(shader, "sss_radius", shaderObject->sss_radius[0], shaderObject->sss_radius[1], shaderObject->sss_radius[2]);
										AiNodeSetFlt(shader, "bounce_factor", object.material->knob("bounce_factor")->get_value());
										AiNodeSetRGB(shader, "opacity", shaderObject->opacity[0], shaderObject->opacity[1], shaderObject->opacity[2]);
										if ( object.material->knob("specular_brdf")->get_value() == 0)
										{
											AiNodeSetStr(shader, "specular_brdf", "stretched_phong");
										}else if( object.material->knob("specular_brdf")->get_value() == 1)
										{
											AiNodeSetStr(shader, "specular_brdf", "cook_torrance");
										}else if( object.material->knob("specular_brdf")->get_value() == 2)
										{
											AiNodeSetStr(shader, "specular_brdf", "ward_duer");
										}
									}
									// NO
									else
									{
										// Do we already have a node for this shader?
										shader = AiNodeLookUpByName(object.material->node_name().c_str());
										// No create one
										if (shader == 0)
										{
											shader = AiNode("fb_ShadowMtd");
											AiNodeSetStr(shader, "name", object.material->node_name().c_str());
											AiNodeSetPtr(tmpPoly, "shader", shader);
											AiNodeSetPtr(shader, "data", static_cast <Iop *> (node)->input(0));
											AiNodeSetPtr(tmpPoly, "shader", shader);
											AiNodeSetRGBA(shader, "color", 0.0f, 0.0f, 0.0f , 1.0f);
										}
									}
								}
								// Set / Update Transform
			               Matrix4  tmpMat = object.matrix.array();
			               AtMatrix mTmpMat;
			               mCpMat(tmpMat, mTmpMat);
			               AiNodeSetMatrix(tmpPoly, "matrix", mTmpMat);
							}
						}
					}
				}
			}

			//// Render
		   const AtInt num_aa_samples = node->m_aa_samples;
		   bool isProgressive = true;
		   if (num_aa_samples<0)
		   {
		   	isProgressive = false;
		   }
		   AtInt init_progressive_samples = isProgressive ? -3 : num_aa_samples;
		   AtUInt prog_passes = isProgressive ? ((-init_progressive_samples) + 2) : 1;

		   AtULong ai_status(AI_SUCCESS);
		   for (AtUInt i = 0; (i < prog_passes); ++i)
		   {
		      AtInt sampling = i + init_progressive_samples;
		      if (i + 1 == prog_passes) sampling = num_aa_samples;

		      AiNodeSetInt(AiUniverseGetOptions(), "AA_samples", sampling);
		      // Begin a render!
				//AiASSWrite("/home/ndu/scene.ass", AI_NODE_ALL, FALSE);AI_INTERRUPT
		      ai_status = AiRender(AI_RENDER_MODE_CAMERA);

				if (ai_status==AI_SUCCESS)
				{
					std::cout << "RENDER LOOP : Render done!" << std::endl;
					node->startRender = false;
				}
				if (ai_status==AI_ABORT)
				{
					std::cout << "RENDER LOOP : Render Abort!" << std::endl;
					node->startRender = false;
					break;
				}
				if (ai_status==AI_INTERRUPT)
				{
					std::cout << "RENDER LOOP : Render Interrupt!" << std::endl;
					node->startRender = true;
					break;
				}
		   }
			//
			node->flagForUpdate();
			node->inRender = false;

   	}
		if (node->endRender)
		{
				std::cout << "END RENDER" << std::endl;
				// at this point we can shut down Arnold
				AiEnd();
				//node->initRender = true;

		}
		// Nothing to do ? Take a little nap
		usleep(0.1 * 1000000);
   }
}

#define _gamma  (params[0].FLT )  /**< accessor for driver's gamma parameter */

AI_DRIVER_NODE_EXPORT_METHODS(ntoa_driver_std);

struct COutputDriverData
{
   AtUInt    imageWidth, imageHeight;
   float     gamma;
   AtBoolean rendering;
};

enum EDisplayUpdateMessageType
{
   MSG_BUCKET_PREPARE,
   MSG_BUCKET_UPDATE,
   MSG_IMAGE_UPDATE
};

// This struct holds the data for a display update message
struct CDisplayUpdateMessage
{
   EDisplayUpdateMessageType msgType;
   AtBBox2                   bucketRect;
   AtRGBA*                 pixels;
   bool                      finished;
};

static COutputDriverData                       s_outputDriverData;

node_parameters
{
   AiParameterFLT ("gamma", 1.0f);
   AiParameterPTR("data", NULL);
}

node_initialize
{
   AiDriverInitialize(node, FALSE, NULL);
}

node_update
{
}

driver_supports_pixel_type
{
   switch (pixel_type)
   {
      case AI_TYPE_RGB:
      case AI_TYPE_RGBA:
         return TRUE;
      default:
         return FALSE;
   }
}

driver_extension
{
   return NULL;
}

driver_open
{
   AtParamValue *params = node->params;

   if (!s_outputDriverData.rendering)
   {
      s_outputDriverData.imageWidth  = display_window.maxx - display_window.minx + 1;
      s_outputDriverData.imageHeight = display_window.maxy - display_window.miny + 1;
      s_outputDriverData.gamma       = _gamma;
      s_outputDriverData.rendering   = TRUE;
   }
}

driver_prepare_bucket
{
   CDisplayUpdateMessage   msg;

   msg.msgType = MSG_BUCKET_PREPARE;
   msg.bucketRect.minx = bucket_xo;
   msg.bucketRect.miny = bucket_yo;
   msg.bucketRect.maxx = bucket_xo + bucket_size_x - 1;
   msg.bucketRect.maxy = bucket_yo + bucket_size_y - 1;
   msg.pixels          = NULL;
   msg.finished        = false;

}

driver_write_bucket
{
	//std::cout << "driver_write_bucket-start" << std::endl;
   AtInt          pixel_type;
   const AtVoid* bucket_data;

   // get the first AOV layer
   if (!AiOutputIteratorGetNext(iterator, NULL, &pixel_type, &bucket_data))
      return;

   ArnoldRender * m_node;
   m_node = reinterpret_cast <ArnoldRender *> (AiNodeGetPtr(node, "data"));
   //AtRGBA* pixels = new AtRGBA[bucket_size_x * bucket_size_y];

   AtInt minx = bucket_xo;
   AtInt miny = bucket_yo;
   AtInt maxx = bucket_xo + bucket_size_x - 1;
   AtInt maxy = bucket_yo + bucket_size_y - 1;

   // lock buffer
   m_node->_mutex.lock();
   switch(pixel_type)
   {
      case AI_TYPE_RGB:
      {
         for (AtInt j = miny; (j <= maxy); ++j)
         {
            for (AtInt i = minx; (i <= maxx); ++i)
            {
               AtUInt in_idx = (j-bucket_yo)*bucket_size_x + (i-bucket_xo);
               AtRGB  rgb = ((AtRGB* )bucket_data)[in_idx];

               AtRGBA* pixel = &m_node->get(i, (RESY-1)-j);

               AiColorGamma(&rgb, s_outputDriverData.gamma);

               pixel->r = rgb.r;
               pixel->g = rgb.g;
               pixel->b = rgb.b;
               pixel->a = 0;

            }
         }
         break;
      }
      case AI_TYPE_RGBA:
      {
         for (AtInt j = miny; (j <= maxy); ++j)
         {
            for (AtInt i = minx; (i <= maxx); ++i)
            {
               AtUInt in_idx = (j-bucket_yo)*bucket_size_x + (i-bucket_xo);
               AtRGBA  rgba = ((AtRGBA* )bucket_data)[in_idx];

                AtRGBA* pixel = &m_node->get(i, (RESY-1)-j);

               AiRGBAGamma(&rgba, s_outputDriverData.gamma);

					//std::cout << "//////////////////////////////////////////////+" << pixel << std::endl;
               pixel->r = rgba.r;
               pixel->g = rgba.g;
               pixel->b = rgba.b;
               pixel->a = rgba.a;

             }
         }
         break;
      }
   }
   m_node->_mutex.unlock();
   //m_node->m_node.m_hash++;
   //m_node->m_node.hash-append(m_node->m_node->m_hash)
   if (m_node!=NULL)
   {
	   m_node->flagForUpdate();
   }
	//std::cout << "driver_write_bucket-end" << std::endl;

}

driver_close
{
    CDisplayUpdateMessage msg;

    msg.finished = true;

    s_outputDriverData.rendering = FALSE;
}

node_finish
{
    // release the driver
    AiDriverDestroy(node);
}

enum SHADERS
{
   MY_SHADER,
   MY_OTHER_SHADER
};

node_loader
{
   switch (i)
   {
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
			return TRUE;
      break;

      default:
         return FALSE;
   }

   sprintf(node->version, AI_VERSION);
   return TRUE;
}
