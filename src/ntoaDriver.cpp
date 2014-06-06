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
 * ntoaDriver.cpp
 *
 *  Created on: Jan 20, 2011
 *      Author: ndu
 */

#include <time.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <sstream>

#include <memory>
#include <cstdarg>
#include <cstdio>
#include <limits.h>

#include <ai.h>

#include "ntoa.h"

#include "cBuffer.h"

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * ARNOLD PART
 *
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#define _gamma  (params[0].FLT )  /**< accessor for driver's gamma parameter */

AI_DRIVER_NODE_EXPORT_METHODS(ntoa_driver_std);

struct COutputDriverData
{
	uint  imageWidth;
	uint  imageHeight;
	float gamma;
	bool  rendering;
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
	AtRGBA*                   pixels;
	bool                      finished;
};

static COutputDriverData s_outputDriverData;

node_parameters {
	AiParameterFLT ("gamma", 1.0f);
	AiParameterPTR("data", NULL);
}

node_initialize {
	AiDriverInitialize(node, false, NULL);
}

driver_supports_pixel_type {
	switch (pixel_type) {
		case AI_TYPE_RGB:
		case AI_TYPE_RGBA:
			return true;
		default:
			return false;
	}
}

driver_extension {
	return NULL;
}

driver_open {
	AtParamValue *params = AiNodeGetParams(node);

	if (!s_outputDriverData.rendering) {
		s_outputDriverData.imageWidth  = display_window.maxx - display_window.minx + 1;
		s_outputDriverData.imageHeight = display_window.maxy - display_window.miny + 1;
		s_outputDriverData.gamma       = _gamma;
		s_outputDriverData.rendering   = true;
	}
}

driver_needs_bucket {
	return true;
}

driver_prepare_bucket {
   CDisplayUpdateMessage   msg;

   msg.msgType = MSG_BUCKET_PREPARE;
   msg.bucketRect.minx = bucket_xo;
   msg.bucketRect.miny = bucket_yo;
   msg.bucketRect.maxx = bucket_xo + bucket_size_x - 1;
   msg.bucketRect.maxy = bucket_yo + bucket_size_y - 1;
   msg.pixels          = NULL;
   msg.finished        = false;

}

driver_write_bucket {
	//std::cout << "driver_write_bucket-start" << std::endl;
	int          pixel_type;
	const void* bucket_data;

	// get the first AOV layer
	if (!AiOutputIteratorGetNext(iterator, NULL, &pixel_type, &bucket_data))
		return;

	cBuffer * m_buffer;
	m_buffer = static_cast <cBuffer *> (AiNodeGetPtr(node, "data"));

	int minx = bucket_xo;
	int miny = bucket_yo;
	int maxx = bucket_xo + bucket_size_x - 1;
	int maxy = bucket_yo + bucket_size_y - 1;

	// lock buffer
	m_buffer->m_mutex.lock();
	switch(pixel_type) {
		case AI_TYPE_RGB: {
			for (int j = miny; (j <= maxy); ++j) {
				for (int i = minx; (i <= maxx); ++i) {
					uint in_idx = (j-bucket_yo)*bucket_size_x + (i-bucket_xo);
					AtRGB  rgb = ((AtRGB* )bucket_data)[in_idx];

					AtRGBA* pixel = &m_buffer->get(i, (s_outputDriverData.imageHeight-1)-j);

					AiColorGamma(&rgb, s_outputDriverData.gamma);

					pixel->r = rgb.r;
					pixel->g = rgb.g;
					pixel->b = rgb.b;
					pixel->a = 0;
				}
			}
			break;
      }
      case AI_TYPE_RGBA: {
    	  for (int j = miny; (j <= maxy); ++j) {
    		  for (int i = minx; (i <= maxx); ++i) {
    			  uint in_idx = (j-bucket_yo)*bucket_size_x + (i-bucket_xo);
    			  AtRGBA  rgba = ((AtRGBA* )bucket_data)[in_idx];

    			  AtRGBA* pixel = &m_buffer->get(i, (s_outputDriverData.imageHeight-1)-j);

    			  AiRGBAGamma(&rgba, s_outputDriverData.gamma);

    			  pixel->r = rgba.r;
    			  pixel->g = rgba.g;
    			  pixel->b = rgba.b;
    			  pixel->a = rgba.a;
    		  }
    	  }
    	  break;
      }
	}
	m_buffer->m_mutex.unlock();
}

driver_close {
	CDisplayUpdateMessage msg;
	msg.finished = true;
	s_outputDriverData.rendering = false;
}

node_finish {
	// release the driver
	AiDriverDestroy(node);
}

node_update {
}

driver_process_bucket {
}
