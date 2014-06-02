/*
 * cBuffer.cpp
 *
 *  Created on: Jan 20, 2011
 *      Author: ndu
 */
#include "cBuffer.h"


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * COMMON PART
 *
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
// cBuffer : The framebuffer for our node
cBuffer::cBuffer() :
	_width(0),
	_height(0)
{
}

void cBuffer::init(const unsigned int width, const unsigned int height)
{
	_width  = width;
	_height = height;
	m_mutex.lock();
	_data.clear();
	_data.resize(_width * _height);
	m_mutex.unlock();
	//std::cout << "cBuffer X Y : " << width << " " << height << std::endl;
}

AtRGBA& cBuffer::get(unsigned int x, unsigned int y)
{
	unsigned int index = (_width * y) + x;
	return _data[index];
}

const AtRGBA& cBuffer::get(unsigned int x, unsigned int y) const
{
	unsigned int index = (_width * y) + x;
	return _data[index];
}

const unsigned int cBuffer::size() const
{
	return _data.size();
}


