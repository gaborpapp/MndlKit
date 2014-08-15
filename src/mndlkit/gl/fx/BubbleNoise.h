/*
 Copyright (C) 2014 Botond Gabor Barna, Gabor Papp

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <memory>

#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Vector.h"

namespace mndl { namespace gl { namespace fx {

typedef std::shared_ptr< class BubbleNoise > BubbleNoiseRef;

class BubbleNoise
{
 public:
	static BubbleNoiseRef create( int width, int height )
	{ return BubbleNoiseRef( new BubbleNoise( width, height ) ); }

	static BubbleNoiseRef create(const ci::Vec2i &size)
	{ return BubbleNoiseRef( new BubbleNoise( size.x, size.y ) ); }

	ci::gl::Texture & process( const ci::gl::Texture &source,
							   float time, float freq, float contrast,
							   const ci::ColorA& colorBubble, const ci::Vec2f& offset );

 protected:
	BubbleNoise( int width, int height );

	ci::gl::Fbo mOutputFbo;

	ci::gl::GlslProgRef mBubbleNoiseShader;

	static const char *sBubbleNoiseVertexShader;
	static const char *sBubbleNoiseFragmentShader;
};

} } } // namespace mndl::gl::fx

