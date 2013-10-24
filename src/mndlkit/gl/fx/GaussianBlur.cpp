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

#include "cinder/app/App.h"
#include "cinder/gl/gl.h"

#include "GaussianBlur.h"

namespace mndl { namespace gl { namespace fx {

#define STRINGIFY(x) #x

const char *GaussianBlur::sGaussianBlurHPassVertexShader =
	STRINGIFY(
		uniform float width;

		varying vec4 texCoordsPos;
		varying vec4 texCoordsNeg;

		void main()
		{
			float invOffset1 = 1.3846153846 / width;
			float invOffset2 = 3.2307692308 / width;

			vec2 uv = gl_MultiTexCoord0.st;
			texCoordsPos = vec4( uv.x, uv.y, uv.x + invOffset1, uv.x + invOffset2 );
			texCoordsNeg = vec4( uv.x, uv.y, uv.x - invOffset1, uv.x - invOffset2 );

			gl_Position = ftransform();
		}
	);

const char *GaussianBlur::sGaussianBlurHPassFragmentShader =
	STRINGIFY(
			uniform sampler2D tex;

			varying vec4 texCoordsPos;
			varying vec4 texCoordsNeg;

			void main()
			{
				vec4 tc = texture2D( tex, texCoordsPos.xy ) * 0.2270270270;
				tc += texture2D( tex, texCoordsPos.zy ) * 0.3162162162;
				tc += texture2D( tex, texCoordsPos.wy ) * 0.0702702703;

				tc += texture2D( tex, texCoordsNeg.zy ) * 0.3162162162;
				tc += texture2D( tex, texCoordsNeg.wy ) * 0.0702702703;

				gl_FragColor = vec4( tc );
			}
	);

const char *GaussianBlur::sGaussianBlurVPassVertexShader =
	STRINGIFY(
		uniform float height;

		varying vec4 texCoordsPos;
		varying vec4 texCoordsNeg;

		void main()
		{
			float invOffset1 = 1.3846153846 / height;
			float invOffset2 = 3.2307692308 / height;

			vec2 uv = gl_MultiTexCoord0.st;
			texCoordsPos = vec4( uv.x, uv.y, uv.y + invOffset1, uv.y + invOffset2 );
			texCoordsNeg = vec4( uv.x, uv.y, uv.y - invOffset1, uv.y - invOffset2 );

			gl_Position = ftransform();
		}
	);

const char *GaussianBlur::sGaussianBlurVPassFragmentShader =
	STRINGIFY(
		uniform sampler2D tex;

		varying vec4 texCoordsPos;
		varying vec4 texCoordsNeg;

		void main()
		{
			vec4 tc = texture2D( tex, texCoordsPos.xy ) * 0.2270270270;
			tc += texture2D( tex, texCoordsPos.xz ) * 0.3162162162;
			tc += texture2D( tex, texCoordsPos.xw ) * 0.0702702703;

			tc += texture2D( tex, texCoordsNeg.xz ) * 0.3162162162;
			tc += texture2D( tex, texCoordsNeg.xw ) * 0.0702702703;

			gl_FragColor = vec4( tc );
		}
	);

GaussianBlur::GaussianBlur( const ci::Vec2i &size, ci::gl::Fbo::Format format )
{
	format.enableDepthBuffer( false );
	format.enableColorBuffer( true, 2 );
	mFbo = ci::gl::Fbo( size.x, size.y, format );

	try
	{
		mGaussianBlurHPassShader = ci::gl::GlslProg( sGaussianBlurHPassVertexShader,
													 sGaussianBlurHPassFragmentShader );
		mGaussianBlurVPassShader = ci::gl::GlslProg( sGaussianBlurVPassVertexShader,
													 sGaussianBlurVPassFragmentShader );
	}
	catch ( const ci::gl::GlslProgCompileExc &exc )
	{
		ci::app::console() << exc.what() << std::endl;
	}

	if ( mGaussianBlurHPassShader )
	{
		mGaussianBlurHPassShader.bind();
		mGaussianBlurHPassShader.uniform( "tex", 0 );
		mGaussianBlurHPassShader.uniform( "width", float( mFbo.getWidth() ) );
		mGaussianBlurHPassShader.unbind();
	}
	if ( mGaussianBlurVPassShader )
	{
		mGaussianBlurVPassShader.bind();
		mGaussianBlurVPassShader.uniform( "tex", 0 );
		mGaussianBlurVPassShader.uniform( "height", float( mFbo.getHeight() ) );
		mGaussianBlurVPassShader.unbind();
	}
}

ci::gl::Texture & GaussianBlur::process( const ci::gl::Texture &source, int iterations /* = 1 */ )
{
	ci::gl::SaveFramebufferBinding bindingSaver;
	glPushAttrib( GL_VIEWPORT_BIT );

	ci::gl::pushMatrices();

	mFbo.bindFramebuffer();
	ci::gl::setMatricesWindow( mFbo.getSize(), false );
	ci::gl::setViewport( mFbo.getBounds() );

	glDrawBuffer( GL_COLOR_ATTACHMENT0 );
	ci::gl::draw( source, mFbo.getBounds() );

	for ( int i = 0; i < iterations; i++ )
	{
		glDrawBuffer( GL_COLOR_ATTACHMENT1 );
		mGaussianBlurHPassShader.bind();
		ci::gl::draw( mFbo.getTexture( 0 ), mFbo.getBounds() );
		mGaussianBlurHPassShader.unbind();

		glDrawBuffer( GL_COLOR_ATTACHMENT0 );
		mGaussianBlurVPassShader.bind();
		ci::gl::draw( mFbo.getTexture( 1 ), mFbo.getBounds() );
		mGaussianBlurVPassShader.unbind();
	}

	mFbo.unbindFramebuffer();

	ci::gl::popMatrices();
	glPopAttrib();

	return mFbo.getTexture();
}

} } } // namespace mndl::gl::fx

