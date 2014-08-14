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

 Based on the vvvv TextureFx PhotoFilm filter.
*/
#include "PhotoFilm.h"

using namespace ci;

namespace mndl { namespace gl { namespace fx {

#define STRINGIFY(x) #x

const char *PhotoFilm::sPhotoFilmVertexShader = "#version 120\n"
	STRINGIFY(
		void main()
		{
			gl_Position =  gl_ModelViewProjectionMatrix * gl_Vertex;
			gl_TexCoord[ 0 ] = gl_MultiTexCoord0;
			gl_FrontColor = gl_Color;
		}
);

const char *PhotoFilm::sPhotoFilmFragmentShader = "#version 120\n"
	STRINGIFY(
		uniform sampler2D tex;

		uniform float brightness;
		uniform float gamma;
		uniform vec3 colorA;
		uniform vec3 colorB;
		uniform float srcRgbAmount;
		uniform float srcRgbGamma;
		uniform float mixFactor;

		void main()
		{
			vec2 uv = gl_TexCoord[ 0 ].st;
			vec4 c = texture2D( tex, uv );
			float gray = dot( c.rgb, vec3( 0.299, 0.587, 0.114 ) );
			vec3 rgb = c.rgb - vec3( gray );
			c.rgb = gray * pow( 2.0, brightness ) *
							mix( mix( colorA, colorB, smoothstep( -0.4, 0.4, gray - 0.4 ) ),
								 1.0, pow( gray, 2.0 ) );
			c.rgb = c.rgb + sign( rgb ) * pow( abs( rgb ) * sqrt( 2.0 ) * srcRgbAmount,
											   pow( 2.0, srcRgbGamma ) );
			c.rgb = pow( c.rgb, pow( 2.0, gamma ) );
			c.rgb = lerp( c.rgb, texture2D( tex, uv ), mixFactor );
			gl_FragColor = c;
		}
);

PhotoFilm::PhotoFilm( int w, int h )
{
	ci::gl::Fbo::Format format;
	format.setWrap( GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE );
	format.enableDepthBuffer( false );
	mOutputFbo = ci::gl::Fbo( w, h, format );

	mPhotoFilmShader = ci::gl::GlslProg::create( sPhotoFilmVertexShader, sPhotoFilmFragmentShader );
	mPhotoFilmShader->bind();
	mPhotoFilmShader->uniform( "tex", 0 );
	mPhotoFilmShader->unbind();
}

ci::gl::Texture & PhotoFilm::process( const ci::gl::Texture &source, float brightness,
		float gamma,
		const ci::Color &colorA, const ci::Color &colorB, float srcRgbAmount,
		float srcRgbGamma, float mixFactor )
{
	ci::gl::SaveFramebufferBinding bindingSaver;
	glPushAttrib( GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT | GL_VIEWPORT_BIT );

	ci::gl::pushMatrices();

	mOutputFbo.bindFramebuffer();

	ci::gl::setMatricesWindow( mOutputFbo.getSize(), false );
	ci::gl::setViewport( mOutputFbo.getBounds() );

	ci::gl::color( Color::white() );

	mPhotoFilmShader->bind();
	mPhotoFilmShader->uniform( "brightness", brightness );
	mPhotoFilmShader->uniform( "gamma", gamma );
	mPhotoFilmShader->uniform( "colorA", colorA );
	mPhotoFilmShader->uniform( "colorB", colorB );
	mPhotoFilmShader->uniform( "srcRgbAmount", srcRgbAmount );
	mPhotoFilmShader->uniform( "srcRgbGamma", srcRgbGamma );
	mPhotoFilmShader->uniform( "mixFactor", mixFactor );

	ci::gl::draw( source, mOutputFbo.getBounds() );

	mPhotoFilmShader->unbind();

	ci::gl::popMatrices();
	glPopAttrib();

	return mOutputFbo.getTexture();
}

} } } // namespace mndl::gl::fx

