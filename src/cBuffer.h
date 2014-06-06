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
 * cBuffer.h
 *
 *  Created on: Jan 20, 2011
 *      Author: ndu
 */

#ifndef CBUFFER_H_
#define CBUFFER_H_

#include <vector>

#include "DDImage/Thread.h"

#include <ai.h>

class NTOA;

class cBuffer {
public:
    cBuffer();

    void init(const unsigned int width, const unsigned int height);

    AtRGBA& get(unsigned int x, unsigned int y);

    const AtRGBA& get(unsigned int x, unsigned int y) const;

    const unsigned int size() const;

    // data
    std::vector<AtRGBA> _data;
    unsigned int        _width;
    unsigned int        _height;
    NTOA  *             m_node;  // the node
    DD::Image::Lock     m_mutex; // mutex for locking the pixel buffer
};

#endif /* CBUFFER_H_ */
