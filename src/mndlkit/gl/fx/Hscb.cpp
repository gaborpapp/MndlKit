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
#include "cinder/app/App.h"

#include "Hscb.h"

using namespace ci;

namespace mndl { namespace gl { namespace fx {

#define STRINGIFY(x) #x

const char *Hscb::sHscbVertexShader = "#version 120\n"
	STRINGIFY(
		void main()
		{
			gl_Position =  gl_ModelViewProjectionMatrix * gl_Vertex;
			gl_TexCoord[ 0 ] = gl_MultiTexCoord0;
			gl_FrontColor = gl_Color;
		}
);

const char *Hscb::sHscbFragmentShader = "#version 120\n"
	STRINGIFY(
		uniform sampler2D tex;

		uniform float hue; // [0..1]
		uniform float saturation; // [0..n]
		uniform float brightness; // [0..1]
		uniform float contrast; // [0..n]

		// http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
		vec3 rgb2hsv( vec3 c )
		{
			vec4 K = vec4( 0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0 );
			vec4 p = c.g < c.b ? vec4( c.bg, K.wz ) : vec4( c.gb, K.xy );
			vec4 q = c.r < p.x ? vec4( p.xyw, c.r ) : vec4( c.r, p.yzx );

			float d = q.x - min( q.w, q.y );
			float e = 1.0e-10;
			return vec3( abs( q.z + ( q.w - q.y ) / ( 6.0 * d + e ) ), d / ( q.x + e ), q.x );
		}

		vec3 hsv2rgb( vec3 c )
		{
			vec4 K = vec4( 1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0 );
			vec3 p = abs( fract( c.xxx + K.xyz ) * 6.0 - K.www );
			return c.z * mix( K.xxx, clamp( p - K.xxx, 0.0, 1.0 ), c.y );
		}

		void main()
		{
			vec4 c = texture2D( tex, gl_TexCoord[ 0 ].st );
			vec3 col = c.rgb;
			vec3 hsv = rgb2hsv( col );
			hsv.x = mod( hsv.x + hue, 1.0 );
			hsv.y *= saturation;
			hsv.y = clamp( hsv.y, 0.0, 1.0 );
			col = hsv2rgb( hsv );
			col = ( col - 0.5 ) * contrast + 0.5 + brightness;
			col = clamp( col, 0.0, 1.0 );
			gl_FragColor = vec4( col, c.a );
		}
);

Hscb::Hscb( int w, int h )
{
	ci::gl::Fbo::Format format;
	format.setWrap( GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE );
	format.enableDepthBuffer( false );
	mOutputFbo = ci::gl::Fbo( w, h, format );

	try
	{
		mHscbShader = ci::gl::GlslProg::create( sHscbVertexShader, sHscbFragmentShader );
		mHscbShader->bind();
		mHscbShader->uniform( "tex", 0 );
		mHscbShader->unbind();
	}
	catch ( const ci::gl::GlslProgCompileExc &exc )
	{
		ci::app::console() << exc.what() << std::endl;
	}
}

ci::gl::Texture & Hscb::process( const ci::gl::Texture &source,
								 float hue, float saturation,
								 float contrast, float brightness )
{
	ci::gl::SaveFramebufferBinding bindingSaver;
	glPushAttrib( GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT | GL_VIEWPORT_BIT );

	ci::gl::pushMatrices();

	mOutputFbo.bindFramebuffer();

	ci::gl::setMatricesWindow( mOutputFbo.getSize(), false );
	ci::gl::setViewport( mOutputFbo.getBounds() );

	ci::gl::color( Color::white() );

	mHscbShader->bind();
	mHscbShader->uniform( "hue", hue );
	mHscbShader->uniform( "saturation", saturation );
	mHscbShader->uniform( "contrast", contrast );
	mHscbShader->uniform( "brightness", brightness );

	ci::gl::draw( source, mOutputFbo.getBounds() );

	mHscbShader->unbind();

	ci::gl::popMatrices();
	glPopAttrib();

	return mOutputFbo.getTexture();
}

} } } // namespace mndl::gl::fx

