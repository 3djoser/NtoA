#include "ntoa.h"

#include "ntoaStandardShader.h"
#include "utils.h"

NTOA::NTOA(Node* node) : RenderScene(node)
{
	//position 					= 0;
	speed 						= 1.0;
	doAbort 					= false;
	//
	m_buffer.m_node  			= this;
	//
	//Format & tmpFormat = (Format &)format();
	//AiNodeSetInt(options, "xres", node->formats.format()->width());
	//AiNodeSetInt(options, "yres", node->formats.format()->height());

	//m_buffer.init(tmpFormat.width(), tmpFormat.height());
	m_buffer.init(formats.format()->width(),formats.format()->height());
	//
	/*
	 *  Arnold params
	 */
	m_aa_samples 				= 0;
	m_diffuse_depth 			= 1;
	m_glossy_depth 				= 1;
	m_reflection_depth 			= 1;
	m_refraction_depth 			= 1;
	m_total_depth 				= 6;
	m_diffuse_samples 			= 1;
	m_sss_hemi_samples 			= 1;
	m_glossy_samples 			= 1;
	// CAMERA
	m_cam_dof 					= false;
	m_focal_distance 			= 1.0;
	m_aperture_size 		   	= 0.0;
	m_aperture_blades 		   	= 6;
	m_aperture_rotation 	   	= 0.0f;
	m_aperture_blade_curvature 	= 1.0f;
	m_aperture_aspect_ratio    	= 1.0f;
	// SKY
	m_sky_active     		   	= false;
    //m_sky_color      			= {1.0f, 1.0f, 1.0f };
	m_sky_intensity  		   	= 1.0f;
	m_sky_visibility 		   	= false;
};

// Destroying the Op should get rid of the parallel threads.
// Unfortunatly currently Nuke does not destroy one of the Ops on a
// deleted node, as it is saving it for Undo. This bug will be fixed
// in an upcoming version, so you should implement this:
NTOA::~NTOA()
{
};

void NTOA::translateOptions(NTOA * node)
{
	std::cout << " -> translate Options : " << node->formats.format()->width() << std::endl;
	AtNode *options = AiUniverseGetOptions();
	AiNodeSetInt(options, "xres", node->formats.format()->width());
	AiNodeSetInt(options, "yres", node->formats.format()->height());
	//
	AiNodeSetInt(options, "AA_samples", node->m_aa_samples);
	AiNodeSetInt(options, "GI_diffuse_depth"   , node->m_diffuse_depth);
	AiNodeSetInt(options, "GI_glossy_depth"    , node->m_glossy_depth);
	AiNodeSetInt(options, "GI_reflection_depth", node->m_reflection_depth);
	AiNodeSetInt(options, "GI_refraction_depth", node->m_refraction_depth);
	AiNodeSetInt(options, "GI_total_depth"     , node->m_total_depth);
	AiNodeSetInt(options, "GI_diffuse_samples" , node->m_diffuse_samples);
	AiNodeSetInt(options, "GI_sss_hemi_samples", node->m_sss_hemi_samples);
	AiNodeSetInt(options, "GI_glossy_samples"  , node->m_glossy_samples);

	// For now we create a gaussian filter node
	AtNode *filter = AiNode("gaussian_filter");
	AiNodeSetStr(filter, "name", "myfilter");

	// We create an NtoA display driver node
	AtNode *driver = AiNode("ntoa_drv");
	AiNodeSetStr(driver, "name", "mydriver");
	AiNodeSetPtr(driver, "data", &node->m_buffer);

	// assign the driver and filter to the main (beauty) AOV, which is called "RGB"
	AtArray *outputs_array = AiArrayAllocate(1, 1, AI_TYPE_STRING);
	AiArraySetStr(outputs_array, 0, "RGBA RGBA myfilter mydriver");
	AiNodeSetArray(options, "outputs", outputs_array);
};

void NTOA::translateSky(NTOA * node)
{
	if (node->m_sky_active)
	{
		AtNode *sky = AiNode("sky");
		AiNodeSetStr(sky, "name", "ntoaSky");
		AiNodeSetRGB(sky, "color", 1.0f, 1.0f, 1.0f );
		AiNodeSetFlt(sky, "intensity", node->m_sky_intensity);
		// Visibility options
		int visibility = 65535;

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
		AiNodeSetPtr(AiUniverseGetOptions(), "background", sky);
	}else{
		//AiNodeSetFlt(sky, "intensity", 0);
		//AiNodeSetPtr(options, "background", NULL);
	}
};

void NTOA::translateScene(NTOA * node)
{
	GeoOp*       tmpGeo;
	tmpGeo = node->m_getGeo();
	if (tmpGeo != 0)
	{
		// variables
		Scene m_scene;
		std::cout << tmpGeo->Class() << std::endl;
		if       (strcmp(tmpGeo->Class(), "Scene") == 0)
		{
			// On recupere la scene
			tmpGeo->build_scene(m_scene);
			Scene * nodeScene = &m_scene;
			if (nodeScene!=0)
			{
				nodeScene->evaluate_lights();
				// Pour chaque LightContext
				const unsigned n = nodeScene->lights.size();
				for (unsigned i = 0; i < n; i++) {
					LightContext& ltx = *(nodeScene->lights[i]);
					//std::cout << ltx.light()->displayName() << " " << ltx.light()->matrix() <<std::endl;
					translateLight(ltx.light());
				}
				// Geo???
				GeometryList * 	m_geoList;
				m_geoList = nodeScene->object_list();
				//m_geoList->validate(for_real);
				if (m_geoList!=0)
				{
					std::cout << "Object(s) in m_geoList : " << m_geoList->objects()<< std::endl;
					for (unsigned i = 0; i <m_geoList->objects(); i++) {
						// L'objet
						GeoInfo & object =  m_geoList->object(i);
						translateGeo(object);
					}
				}
			}
		}
	}
};

void NTOA::translateCamera(CameraOp * nukeCamera)
{
	std::cout << " -> Start : translateCamera"<< std::endl;
	/* We create a pointer for an Arnold perspective camera
	AtNode *camera;
	// Do we already have a node for this camera?
	camera = AiNodeLookUpByName("ntoaCamera");
	if (camera = 0) camera = AiNode("persp_camera");*/

	AtNode *camera = AiNode("persp_camera");
	AiNodeSetStr(camera, "name", "ntoaCamera");
	AiNodeSetFlt(camera, "fov", (float)nukeCamera->focal_length());
	// position the camera (alternatively you can set 'matrix')
	Matrix4  tmpMat = nukeCamera->matrix();
	AtMatrix mTmpMat;
	mCpMat(tmpMat, mTmpMat);
	AiNodeSetMatrix(camera, "matrix", mTmpMat);
	/*
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
	*/
	// set the active camera (optional, since there is only one camera)
	AiNodeSetPtr(AiUniverseGetOptions(), "camera", camera);
};

void NTOA::translateLight(LightOp* nukeLight)
{
	std::cout << " -> Start : translateLight"<< std::endl;
	std::cout << nukeLight->Class() << std::endl;
	// create a point light source
	AtNode *light;
	// Do we already have a node for this light?
	light =	AiNodeLookUpByName(nukeLight->node_name().c_str());

	double lightType = 0.f;

	DD::Image::Knob* pType = nukeLight->knob("light_type");
	if (pType)
		lightType = pType->get_value(0);
	/* No create one
	if (light == 0)
	{
		std::cout << "RENDER LOOP : Create light " << nukeLight->node_name().c_str() << " * " << lightType << std::endl;

		if (lightType == 0.0f)
		{
			light = AiNode("distant_light");
		} else if (lightType == 1.0f)
		{
			light = AiNode("point_light");
		} else if (lightType == 2.0f)
		{
			light = AiNode("spot_light");
		}
	}*/
	// No create one
	if (light == 0)
	{
		std::cout << "RENDER LOOP : Create light " << nukeLight->node_name().c_str() << std::endl;
		if       (strcmp(nukeLight->Class(), "Light") == 0)
		{
			light = AiNode("point_light");
		} else if (strcmp(nukeLight->Class(), "DirectLight") == 0)
		{
			light = AiNode("distant_light");
		} else if (strcmp(nukeLight->Class(), "Spotlight") == 0)
		{
			light = AiNode("spot_light");
		}
	}
	AiNodeSetStr(light, "name", nukeLight->node_name().c_str());
	//AiNodeSetArray(camera, "matrix", AiArrayConvert(16, 1, AI_TYPE_FLOAT, (void *)node->m_getCam()->matrix().array(), true));
	Matrix4  tmpMat = nukeLight->matrix();
	AtMatrix mTmpMat;
	mCpMat(tmpMat, mTmpMat);
	AiNodeSetMatrix(light, "matrix", mTmpMat);
	//AiNodeSetFlt(light, "intensity", (float)nukeLight->intensity());
	AiNodeSetFlt(light, "intensity", 1);
	AiNodeSetFlt(light, "exposure", (float)nukeLight->intensity());
	Pixel & 	color = (Pixel &)nukeLight->color();
	//ChannelSet 	mChannels = color.channels;
	float * m_tmpColor;
	m_tmpColor = (float *)color.array();
	std::cout << "Size of color " << sizeof(m_tmpColor) <<std::endl;
	/*for (int ii=0;ii<sizeof(m_tmpColor);ii++){
		std::cout << "    - " << m_tmpColor[ii]<<std::endl;
	}*/
	AiNodeSetRGB(light, "color", m_tmpColor[1],m_tmpColor[2],m_tmpColor[3]);
	AiNodeSetBool(light, "cast_shadows", nukeLight->cast_shadows());
	AiNodeSetInt(light, "samples", nukeLight->samples());
	AiNodeSetFlt(light, "radius", nukeLight->sample_width());

    /*float * m_tmpColor;
    m_tmpColor = (float *)color.array();
    */
};

void NTOA::translateGeo(GeoInfo& object)
{
	// Le node
	GeoOp * tmpGeoOp = object.source_geo;

	std::cout << " -> Start : translateGeo : "<< tmpGeoOp->node_name() << std::endl;

	// Do we already have a node for this geo?
	//AtNode *tmpPoly;
	//tmpPoly = AiNodeLookUpByName(tmpGeoOp->node_name().c_str());
	//if (tmpPoly == 0) {
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
		for (int faceIndex = 0;faceIndex<nFaces;faceIndex++)
		{
				const DD::Image::Primitive * prim =  object.primitive(faceIndex);
			   int nVertices = prim->vertices();
			   //nNormIndicesPerFace.push_back( nVertices );
			   for (int verticeIndex = 0 ; verticeIndex < nVertices ; verticeIndex++)
			   {
					   unsigned vi = vertIndices[faceIndex];
					   //unsigned ni = n_group_type == Group_Points ? vi : prim->vertex_offset() + faceIndex;
				       // get vertex normal
					   //const Vector3& n = norm->normal(ni);
				       //normVec.push_back( n  );
			   }
		}
		std::cout << "nFaces              : " << nFaces << std::endl;
		std::cout << "nVertIndicesPerFace : " << nVertIndicesPerFace.size() << std::endl;
		std::cout << "vertIndices         : " << vertIndices.size() << std::endl;
		std::cout << "ptVec               : " << ptVec.size() << std::endl;

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
		for(uint i = 0; (i < ptVec.size()); i++){
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
	std::cout << "GeoInfo material class : " << object.material->Class()<< std::endl;
	// Is it a NtoA standard?
	if (strcmp(object.material->Class(), "ntoaStandard") == 0)
	{
		ntoaStandard * shaderObject = reinterpret_cast<ntoaStandard*> (object.material);
		// create a lambert shader
		AtNode *shaderS = AiNode("standard");
		AiNodeSetStr(shaderS, "name", object.material->node_name().c_str());
		AiNodeSetFlt(shaderS, "specular_roughness", object.material->knob("specular_roughness")->get_value());
		AiNodeSetFlt(shaderS, "specular_anisotropy", object.material->knob("specular_anisotropy")->get_value());
		AiNodeSetFlt(shaderS, "specular_rotation", object.material->knob("specular_rotation")->get_value());
		AiNodeSetRGB(shaderS, "transmittance", shaderObject->transmittance[0], shaderObject->transmittance[1], shaderObject->transmittance[2]);
		AiNodeSetFlt(shaderS, "Kd", object.material->knob("Kd")->get_value());
		AiNodeSetRGB(shaderS, "Kd_color", shaderObject->Kd_color[0], shaderObject->Kd_color[1], shaderObject->Kd_color[2]);
		AiNodeSetFlt(shaderS, "diffuse_roughness", object.material->knob("diffuse_roughness")->get_value());
		AiNodeSetFlt(shaderS, "Ks", object.material->knob("Ks")->get_value());
		AiNodeSetRGB(shaderS, "Ks_color", shaderObject->Ks_color[0], shaderObject->Ks_color[1], shaderObject->Ks_color[2]);
		AiNodeSetFlt(shaderS, "Phong_exponent", object.material->knob("Phong_exponent")->get_value());
		AiNodeSetFlt(shaderS, "Kr", object.material->knob("Kr")->get_value());
		AiNodeSetRGB(shaderS, "Kr_color", shaderObject->Kr_color[0], shaderObject->Kr_color[1], shaderObject->Kr_color[2]);
		AiNodeSetRGB(shaderS, "reflection_exit_color", shaderObject->reflection_exit_color[0], shaderObject->reflection_exit_color[1], shaderObject->reflection_exit_color[2]);
		AiNodeSetBool(shaderS, "reflection_exit_use_environment", object.material->knob("reflection_exit_use_environment")->get_value());
		AiNodeSetFlt(shaderS, "Kt", object.material->knob("Kt")->get_value());
		AiNodeSetRGB(shaderS, "Kt_color", shaderObject->Kt_color[0], shaderObject->Kt_color[1], shaderObject->Kt_color[2]);
		AiNodeSetRGB(shaderS, "refraction_exit_color", shaderObject->refraction_exit_color[0], shaderObject->refraction_exit_color[1], shaderObject->refraction_exit_color[2]);
		AiNodeSetBool(shaderS, "refraction_exit_use_environment", object.material->knob("refraction_exit_use_environment")->get_value());
		AiNodeSetFlt(shaderS, "IOR", object.material->knob("IOR")->get_value());
		AiNodeSetFlt(shaderS, "Kb", object.material->knob("Kb")->get_value());
		AiNodeSetBool(shaderS, "Fresnel", object.material->knob("Fresnel")->get_value());
		AiNodeSetFlt(shaderS, "Krn", object.material->knob("Krn")->get_value());
		AiNodeSetBool(shaderS, "specular_Fresnel", object.material->knob("specular_Fresnel")->get_value());
		AiNodeSetFlt(shaderS, "Ksn", object.material->knob("Ksn")->get_value());
		AiNodeSetBool(shaderS, "Fresnel_affect_diff", object.material->knob("Fresnel_affect_diff")->get_value());
		AiNodeSetFlt(shaderS, "emission", object.material->knob("emission")->get_value());
		AiNodeSetRGB(shaderS, "emission_color", shaderObject->emission_color[0], shaderObject->emission_color[1], shaderObject->emission_color[2]);
		AiNodeSetFlt(shaderS, "direct_specular", object.material->knob("direct_specular")->get_value());
		AiNodeSetFlt(shaderS, "indirect_specular", object.material->knob("indirect_specular")->get_value());
		AiNodeSetFlt(shaderS, "direct_diffuse", object.material->knob("direct_diffuse")->get_value());
		AiNodeSetFlt(shaderS, "indirect_diffuse", object.material->knob("indirect_diffuse")->get_value());
		AiNodeSetBool(shaderS, "enable_glossy_caustics", object.material->knob("enable_glossy_caustics")->get_value());
		AiNodeSetBool(shaderS, "enable_reflective_caustics", object.material->knob("enable_reflective_caustics")->get_value());
		AiNodeSetBool(shaderS, "enable_refractive_caustics", object.material->knob("enable_refractive_caustics")->get_value());
		AiNodeSetBool(shaderS, "enable_internal_reflections", object.material->knob("enable_internal_reflections")->get_value());
		AiNodeSetFlt(shaderS, "Ksss", object.material->knob("Ksss")->get_value());
		AiNodeSetRGB(shaderS, "Ksss_color", shaderObject->Ksss_color[0], shaderObject->Ksss_color[1], shaderObject->Ksss_color[2]);
		AiNodeSetRGB(shaderS, "sss_radius", shaderObject->sss_radius[0], shaderObject->sss_radius[1], shaderObject->sss_radius[2]);
		AiNodeSetFlt(shaderS, "bounce_factor", object.material->knob("bounce_factor")->get_value());
		AiNodeSetRGB(shaderS, "opacity", shaderObject->opacity[0], shaderObject->opacity[1], shaderObject->opacity[2]);
		if ( object.material->knob("specular_brdf")->get_value() == 0)
		{
			AiNodeSetStr(shaderS, "specular_brdf", "stretched_phong");
		}else if( object.material->knob("specular_brdf")->get_value() == 1)
		{
			AiNodeSetStr(shaderS, "specular_brdf", "cook_torrance");
		}else if( object.material->knob("specular_brdf")->get_value() == 2)
		{
			AiNodeSetStr(shaderS, "specular_brdf", "ward_duer");
		}
		AiNodeSetPtr(tmpPoly, "shader", shaderS);
	} else {
		// No so we use the shadow catcher
		AiNodeSetPtr(tmpPoly, "shader", AiNodeLookUpByName("shadowCatcher"));
	}
}

void NTOA::flagForUpdate()
{
	//std::cout << "flagForUpdate-start" << std::endl;
	if ( hash_counter==UINT_MAX )
		hash_counter=0;
	else
		hash_counter++;
	asapUpdate();
	usleep( 0.1 * 1000000 );
	//std::cout << "flagForUpdate-end" << std::endl;
};

// The hash value must change or Nuke will think the picture is the
// same. If you can't determine some id for the picture, you should
// use the current time or something.
void NTOA::append(Hash& hash)
{
	hash.append(hash_counter);
};

/* This is the code for the render thread. This thread is used only to run the AiRender() process outside of the main thread.
unsigned int RenderThread(AtVoid* data)
{
	std::cout << "RenderThread-start" << std::endl;
	AiRender(AI_RENDER_MODE_CAMERA);
	//usleep(unsigned(10 * 1000000));
	std::cout << "RenderThread-end" << std::endl;
};*/

CameraOp* NTOA::m_getCam()
{
	return render_camera();
}

GeoOp* NTOA::m_getGeo()
{
	return render_geo();
}

void NTOA::mAbort()
{
	std::cout << "mAbort-start" << std::endl;
	while(AiRendering()){
		AiRenderAbort();
	}
	AiEnd();
};

void NTOA::_validate(bool for_real)
{
	std::cout << "+_validate" << std::endl;
	info_.full_size_format(*formats.fullSizeFormat());
	info_.format(*formats.format());
	info_.channels(Mask_RGBA);
	info_.set(format());
	//
	//inRender = false;
	//mAbort();

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
	if (tmpCam != 0)
	{
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
			if (node!=NULL)
			{
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
					//std::cout << ltx.light()->node_name() << " " << ltx.matrix(0) <<std::endl;
					//tmpLightCount++;
				}
				// Changement du nombre de light, on relance le render
				/*if (m_light_count != tmpLightCount)
				{
					m_light_count = tmpLightCount;
					std::cout << "m_light_count != tmpLightCount" << std::endl;
					mDoRestart    = true;
				}*/
				// Geo???
				GeometryList * 	m_geoList;
				m_geoList = node->object_list();
				int tmpGeoCount = 0;
				if (m_geoList!=0)
				{
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

						//std::cout << "GeoInfo material name : " << object.material->node_name()<< std::endl;
						//tmpGeoCount++;
					}
				}
				// Changement du nombre de geo, on relance le render
			/*if (m_geo_count != tmpGeoCount)
			{
			   m_geo_count = tmpGeoCount;
			   mDoRestart  = true;
			}*/

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
	//
	if (tmpScene_hash_counter != scene_hash_counter)
	{
		scene_hash_counter = tmpScene_hash_counter;
		//mAbort();
	}
}

void NTOA::_open()
{
	std::cout << " -> Start : _open"<< std::endl;
	//node->inRender = true;
	m_buffer.init(formats.format()->width(),formats.format()->height());
	// start an Arnold session
	AiBegin();
	//AiLoadPlugins("/home/ndu/workspace/ntoaShadowCatcher/Debug");
	AiNodeSetInt(AiUniverseGetOptions(), "threads", 6);
	AiNodeSetInt(AiUniverseGetOptions(), "preserve_scene_data", true);
	AiMsgSetLogFileName("scene1.log");
	AiNodeEntryInstall(AI_NODE_DRIVER, AI_TYPE_RGBA, "ntoa_drv", "<built-in>", ntoa_driver_std, AI_VERSION);

	// shadow catcher
	AiNodeEntryInstall(AI_NODE_SHADER, AI_TYPE_RGBA, "fb_ShadowMtd", "<built-in>", fb_ShadowMtd, AI_VERSION);
	AtNode *shader = AiNode("fb_ShadowMtd");
	AiNodeSetStr(shader, "name", "shadowCatcher");
	AiNodeSetRGBA(shader, "color", 0.0f, 0.0f, 0.0f , 1.0f);
	AiNodeSetPtr(shader, "data", static_cast <Iop *> (this)->input(0));

	// Options
	translateOptions(this);

	// Sky
	translateSky(this);

	// create a perspective camera
	translateCamera(render_camera());

	// We now process the scene graph
	translateScene(this);

	//AiASSWrite("/home/ndu/scene.ass", AI_NODE_ALL, false);
	AiRender(AI_RENDER_MODE_CAMERA);
	// at this point we can shut down Arnold
	AiEnd();
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
	if ( m_buffer._width==0 && m_buffer._height==0 )
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
			if ( xxx >= m_buffer._width || yyy >= m_buffer._height )
			{
				*rOut = *gOut = *bOut = *aOut = 0.f;
			}
			else
			{
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
};

void NTOA::knobs(Knob_Callback f)
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

//mAbort();
int NTOA::knob_changed(Knob* knb)
{
	if (knb != NULL) {
		if       (knb->name() && strcmp(knb->name(), "format") == 0)
		{
			//m_mutex.lock();
			//Format & tmpFormat = (Format &)format();
			//m_buffer.init(tmpFormat.width(),tmpFormat.height());
			//m_buffer.init(1920,1080);
			//m_mutex.unlock();
			mAbort();
			return 1;
		}else if (knb->name() && strcmp(knb->name(), "ass_file") == 0)
		{
			mAbort();
			return 1;
		}else if (knb->name() && strcmp(knb->name(), "aa_samples") == 0)
		{
			mAbort();
			return 1;
		}else if (knb->name() && strcmp(knb->name(), "m_diffuse_depth") == 0)
		{
			mAbort();
			return 1;
		}else if (knb->name() && strcmp(knb->name(), "m_glossy_depth") == 0)
		{
			mAbort();
			return 1;
		}else if (knb->name() && strcmp(knb->name(), "m_reflection_depth") == 0)
		{
			mAbort();
			return 1;
		}else if (knb->name() && strcmp(knb->name(), "m_refraction_depth") == 0)
		{
			mAbort();
			return 1;
		}else if (knb->name() && strcmp(knb->name(), "m_total_depth") == 0)
		{
			mAbort();
			return 1;
		}else if (knb->name() && strcmp(knb->name(), "m_diffuse_samples") == 0)
		{
			mAbort();
			return 1;
		}else if (knb->name() && strcmp(knb->name(), "m_sss_hemi_samples") == 0)
		{
			mAbort();
			return 1;
		}else if (knb->name() && strcmp(knb->name(), "m_glossy_samples") == 0)
		{
			mAbort();
			return 1;
		}else if (knb->name() && strcmp(knb->name(), "cam_dof") == 0)
		{
			//if (knob("focal_distance") != 0)           knob("focal_distance")->enable(m_cam_dof);
			//if (knob("aperture_size") != 0)            knob("aperture_size")->enable(m_cam_dof);
			//if (knob("aperture_blades") != 0)          knob("aperture_blades")->enable(m_cam_dof);
			//if (knob("aperture_rotation") != 0)        knob("aperture_rotation")->enable(m_cam_dof);
			//if (knob("aperture_blade_curvature") != 0) knob("aperture_blade_curvature")->enable(m_cam_dof);
			//if (knob("aperture_aspect_ratio") != 0)    knob("aperture_aspect_ratio")->enable(m_cam_dof);
			//std::cout << " " << render_geo()->scene() << std::endl;
			//render(0,0,this);
			//Thread::spawn(::render, 1, this);
			//AiASSWrite("/3d/ndumay/scene2.ass", AI_NODE_ALL, false);
			//mAbort();
			return 1;
		}

		//inRender = true;
		//return 1;
		//startRender(0,0,this);
		//return 1;
	}
	return 0;
};

