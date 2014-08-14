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

typedef std::shared_ptr< class PhotoFilm > PhotoFilmRef;

class PhotoFilm
{
 public:
	static PhotoFilmRef create( int width, int height )
	{ return PhotoFilmRef( new PhotoFilm( width, height ) ); }

	static PhotoFilmRef create( const ci::Vec2i &size )
	{ return PhotoFilmRef( new PhotoFilm( size.x, size.y ) ); }

	ci::gl::Texture & process( const ci::gl::Texture &source, float brightness,
		float gamma, const ci::Color &colorA, const ci::Color &colorB,
		float srcRgbAmount, float srcRgbGamma, float mixFactor );

 protected:
	PhotoFilm( int width, int height );

	ci::gl::Fbo mOutputFbo;

	ci::gl::GlslProgRef mPhotoFilmShader;

	static const char *sPhotoFilmVertexShader;
	static const char *sPhotoFilmFragmentShader;
};

} } } // namespace mndl::gl::fx

