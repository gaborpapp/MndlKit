/*
 Copyright (C) 2014 Gabor Papp

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

typedef std::shared_ptr< class Invert > InvertRef;

class Invert
{
 public:
	static InvertRef create( int width, int height )
	{ return InvertRef( new Invert( width, height ) ); }

	static InvertRef create( const ci::Vec2i &size )
	{ return InvertRef( new Invert( size.x, size.y ) ); }

	ci::gl::Texture & process( const ci::gl::Texture &source,
			bool invert, bool invertRgb, bool invertAlpha, float mixFactor );

 protected:
	Invert( int width, int height );

	ci::gl::Fbo mOutputFbo;

	ci::gl::GlslProgRef mInvertShader;

	static const char *sInvertVertexShader;
	static const char *sInvertFragmentShader;
};

} } } // namespace mndl::gl::fx

