/*
 Copyright (C) 2013 Gabor Papp

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "cinder/Cinder.h"
#include "cinder/Vector.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"

namespace mndl { namespace gl { namespace fx {

typedef std::shared_ptr< class GaussianBlur > GaussianBlurRef;

class GaussianBlur
{
  public:
	static GaussianBlurRef create( const ci::Vec2i &size, ci::gl::Fbo::Format format = ci::gl::Fbo::Format() )
	{ return GaussianBlurRef( new GaussianBlur( size, format ) ); }

	static GaussianBlurRef create( int x, int y, ci::gl::Fbo::Format format = ci::gl::Fbo::Format() )
	{ return create( ci::Vec2i( x, y ), format ); }

	ci::gl::Texture & process( const ci::gl::Texture &source,
			int iterations = 1 );

  protected:
	GaussianBlur( const ci::Vec2i &size, ci::gl::Fbo::Format format );

	ci::gl::Fbo mFbo;

	ci::gl::GlslProg mGaussianBlurVPassShader;
	ci::gl::GlslProg mGaussianBlurHPassShader;

	static const char *sGaussianBlurHPassVertexShader;
	static const char *sGaussianBlurHPassFragmentShader;
	static const char *sGaussianBlurVPassVertexShader;
	static const char *sGaussianBlurVPassFragmentShader;
};

} } } // namespace mndl::gl::fx

