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

class cBuffer{
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
