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
#include "BrightnessContrast.h"

using namespace ci;

namespace mndl { namespace gl { namespace fx {

#define STRINGIFY(x) #x

const char *BrightnessContrast::sBrightnessContrastVertexShader = "#version 120\n"
	STRINGIFY(
		void main()
		{
			gl_Position =  gl_ModelViewProjectionMatrix * gl_Vertex;
			gl_TexCoord[ 0 ] = gl_MultiTexCoord0;
			gl_FrontColor = gl_Color;
		}
);

const char *BrightnessContrast::sBrightnessContrastFragmentShader = "#version 120\n"
	STRINGIFY(
		uniform sampler2D tex;

		uniform float contrast;
		uniform float brightness;

		void main()
		{
			vec4 color = texture2D( tex, gl_TexCoord[ 0 ].st );
			color.rgb /= color.a;
			color.rgb = ( ( color.rgb - 0.5 ) * max( contrast, 0. ) ) + 0.5 + brightness;
			color.rgb *= color.a;
			gl_FragColor = color;
		}
);

BrightnessContrast::BrightnessContrast( int w, int h )
{
	ci::gl::Fbo::Format format;
	format.setWrap( GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE );
	format.enableDepthBuffer( false );
	mOutputFbo = ci::gl::Fbo( w, h, format );

	mBrightnessContrastShader = ci::gl::GlslProg::create( sBrightnessContrastVertexShader, sBrightnessContrastFragmentShader );
	mBrightnessContrastShader->bind();
	mBrightnessContrastShader->uniform( "tex", 0 );
	mBrightnessContrastShader->unbind();
}

ci::gl::Texture & BrightnessContrast::process( const ci::gl::Texture &source, float brightness, float contrast )
{
	ci::gl::SaveFramebufferBinding bindingSaver;
	glPushAttrib( GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT | GL_VIEWPORT_BIT );

	ci::gl::pushMatrices();

	mOutputFbo.bindFramebuffer();

	ci::gl::setMatricesWindow( mOutputFbo.getSize(), false );
	ci::gl::setViewport( mOutputFbo.getBounds() );

	ci::gl::color( Color::white() );

	mBrightnessContrastShader->bind();
	mBrightnessContrastShader->uniform( "brightness", brightness );
	mBrightnessContrastShader->uniform( "contrast", contrast );

	ci::gl::draw( source, mOutputFbo.getBounds() );

	mBrightnessContrastShader->unbind();

	ci::gl::popMatrices();
	glPopAttrib();

	return mOutputFbo.getTexture();
}

} } } // namespace mndl::gl::fx

