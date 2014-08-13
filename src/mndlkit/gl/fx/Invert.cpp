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
#include "Invert.h"

using namespace ci;

namespace mndl { namespace gl { namespace fx {

#define STRINGIFY(x) #x

const char *Invert::sInvertVertexShader = "#version 120\n"
	STRINGIFY(
		void main()
		{
			gl_Position =  gl_ModelViewProjectionMatrix * gl_Vertex;
			gl_TexCoord[ 0 ] = gl_MultiTexCoord0;
			gl_FrontColor = gl_Color;
		}
);

const char *Invert::sInvertFragmentShader = "#version 120\n"
	STRINGIFY(
		uniform sampler2D tex;

		uniform bool invert;
		uniform bool invertRgb;
		uniform bool invertAlpha;
		uniform float mixFactor;

		void main()
		{
			vec4 c0 = texture2D( tex, gl_TexCoord[ 0 ].st );
			vec4 c1 = c0;
			if ( invert )
			{
				c1.rgb = vec3( 1.0 ) - c1.rgb;
			}
			if ( invertRgb )
			{
				c1.rgb -= 2.0 * ( c1.rgb - dot( c1.rgb, vec3( 0.299, 0.587, 0.114 ) ) );
			}
			if ( invertAlpha )
			{
				c1.a -= 1.0 - c1.a;
			}
			gl_FragColor = mix( c0, c1, mixFactor );
		}
);

Invert::Invert( int w, int h )
{
	ci::gl::Fbo::Format format;
	format.setWrap( GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE );
	format.enableDepthBuffer( false );
	mOutputFbo = ci::gl::Fbo( w, h, format );

	mInvertShader = ci::gl::GlslProg::create( sInvertVertexShader, sInvertFragmentShader );
	mInvertShader->bind();
	mInvertShader->uniform( "tex", 0 );
	mInvertShader->unbind();
}

ci::gl::Texture & Invert::process( const ci::gl::Texture &source,
		bool invert, bool invertRgb, bool invertAlpha, float mixFactor )
{
	ci::gl::SaveFramebufferBinding bindingSaver;
	glPushAttrib( GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT | GL_VIEWPORT_BIT );

	ci::gl::pushMatrices();

	mOutputFbo.bindFramebuffer();

	ci::gl::setMatricesWindow( mOutputFbo.getSize(), false );
	ci::gl::setViewport( mOutputFbo.getBounds() );

	ci::gl::color( Color::white() );

	mInvertShader->bind();
	mInvertShader->uniform( "invert", invert );
	mInvertShader->uniform( "invertRgb", invertRgb );
	mInvertShader->uniform( "invertAlpha", invertAlpha );
	mInvertShader->uniform( "mixFactor", mixFactor );

	ci::gl::draw( source, mOutputFbo.getBounds() );

	mInvertShader->unbind();

	ci::gl::popMatrices();
	glPopAttrib();

	return mOutputFbo.getTexture();
}

} } } // namespace mndl::gl::fx

