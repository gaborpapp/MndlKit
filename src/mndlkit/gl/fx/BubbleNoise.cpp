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

 Based on the vvvv TextureFx BubbleNoise filter.
*/
#include "cinder/app/App.h"

#include "BubbleNoise.h"

using namespace ci;

namespace mndl { namespace gl { namespace fx {

#define STRINGIFY(x) #x

const char *BubbleNoise::sBubbleNoiseVertexShader = "#version 120\n"
	STRINGIFY(
		void main()
		{
			gl_Position =  gl_ModelViewProjectionMatrix * gl_Vertex;
			gl_TexCoord[ 0 ] = gl_MultiTexCoord0;
			gl_FrontColor = gl_Color;
		}
);

const char *BubbleNoise::sBubbleNoiseFragmentShader = "#version 120\n"
	STRINGIFY(
		uniform sampler2D tex;

		uniform float time;
		uniform float freq;
		uniform float contrast;
		uniform vec4 colorBubble;
		uniform vec2 offset;

		vec3 mod289(vec3 x)
		{
			return x - floor(x * (1.0 / 289.0)) * 289.0;
		}

		vec4 mod289(vec4 x)
		{
			return x - floor(x * (1.0 / 289.0)) * 289.0;
		}

		vec4 permute(vec4 x)
		{
			return mod289(((x*34.0) + 1.0)*x);
		}

		vec4 taylorInvSqrt(vec4 r)
		{
			return 1.79284291400159 - 0.85373472095314 * r;
		}

		vec3 fade(vec3 t) {
			return t*t*t*(t*(t*6.0 - 15.0) + 10.0);
		}

		// Classic Perlin noise
		float cnoise(vec3 P)
		{
			vec3 Pi0 = floor(P); // Integer part for indexing
			vec3 Pi1 = Pi0 + vec3(1.0); // Integer part + 1
			Pi0 = mod289(Pi0);
			Pi1 = mod289(Pi1);
			vec3 Pf0 = fract(P); // Fractional part for interpolation
			vec3 Pf1 = Pf0 - vec3(1.0); // Fractional part - 1.0
			vec4 ix = vec4(Pi0.x, Pi1.x, Pi0.x, Pi1.x);
			vec4 iy = vec4(Pi0.yy, Pi1.yy);
			vec4 iz0 = Pi0.zzzz;
			vec4 iz1 = Pi1.zzzz;

			vec4 ixy = permute(permute(ix) + iy);
			vec4 ixy0 = permute(ixy + iz0);
			vec4 ixy1 = permute(ixy + iz1);

			vec4 gx0 = ixy0 * (1.0 / 7.0);
			vec4 gy0 = fract(floor(gx0) * (1.0 / 7.0)) - 0.5;
			gx0 = fract(gx0);
			vec4 gz0 = vec4(0.5) - abs(gx0) - abs(gy0);
			vec4 sz0 = step(gz0, vec4(0.0));
			gx0 -= sz0 * (step(0.0, gx0) - 0.5);
			gy0 -= sz0 * (step(0.0, gy0) - 0.5);

			vec4 gx1 = ixy1 * (1.0 / 7.0);
			vec4 gy1 = fract(floor(gx1) * (1.0 / 7.0)) - 0.5;
			gx1 = fract(gx1);
			vec4 gz1 = vec4(0.5) - abs(gx1) - abs(gy1);
			vec4 sz1 = step(gz1, vec4(0.0));
			gx1 -= sz1 * (step(0.0, gx1) - 0.5);
			gy1 -= sz1 * (step(0.0, gy1) - 0.5);

			vec3 g000 = vec3(gx0.x, gy0.x, gz0.x);
			vec3 g100 = vec3(gx0.y, gy0.y, gz0.y);
			vec3 g010 = vec3(gx0.z, gy0.z, gz0.z);
			vec3 g110 = vec3(gx0.w, gy0.w, gz0.w);
			vec3 g001 = vec3(gx1.x, gy1.x, gz1.x);
			vec3 g101 = vec3(gx1.y, gy1.y, gz1.y);
			vec3 g011 = vec3(gx1.z, gy1.z, gz1.z);
			vec3 g111 = vec3(gx1.w, gy1.w, gz1.w);

			vec4 norm0 = taylorInvSqrt(vec4(dot(g000, g000), dot(g010, g010), dot(g100, g100), dot(g110, g110)));
			g000 *= norm0.x;
			g010 *= norm0.y;
			g100 *= norm0.z;
			g110 *= norm0.w;
			vec4 norm1 = taylorInvSqrt(vec4(dot(g001, g001), dot(g011, g011), dot(g101, g101), dot(g111, g111)));
			g001 *= norm1.x;
			g011 *= norm1.y;
			g101 *= norm1.z;
			g111 *= norm1.w;

			float n000 = dot(g000, Pf0);
			float n100 = dot(g100, vec3(Pf1.x, Pf0.yz));
			float n010 = dot(g010, vec3(Pf0.x, Pf1.y, Pf0.z));
			float n110 = dot(g110, vec3(Pf1.xy, Pf0.z));
			float n001 = dot(g001, vec3(Pf0.xy, Pf1.z));
			float n101 = dot(g101, vec3(Pf1.x, Pf0.y, Pf1.z));
			float n011 = dot(g011, vec3(Pf0.x, Pf1.yz));
			float n111 = dot(g111, Pf1);

			vec3 fade_xyz = fade(Pf0);
			vec4 n_z = mix(vec4(n000, n100, n010, n110), vec4(n001, n101, n011, n111), fade_xyz.z);
			vec2 n_yz = mix(n_z.xy, n_z.zw, fade_xyz.y);
			float n_xyz = mix(n_yz.x, n_yz.y, fade_xyz.x);
			return 2.2 * n_xyz;
		}

		float surface3(vec3 coord)
		{
			float frequency = freq;
			float n = contrast / 2;
			n += 1.0  * abs(cnoise(coord * frequency));
			n += 0.5  * abs(cnoise(coord * frequency * 2.0));
			n += 0.25 * abs(cnoise(coord * frequency * 4.0));
			return n;
		}

		void main()
		{
			vec2 position = gl_TexCoord[ 0 ].st;
			vec4 c = texture2D( tex, position );
			vec2 offsetSize = vec2( 1.0, 1.0 ) / offset;
			float n = surface3(vec3(position.x               , position.y               , time * 0.1)) * 0.2 +
			          surface3(vec3(position.x - offsetSize.x, position.y               , time * 0.1)) * 0.2 +
			          surface3(vec3(position.x + offsetSize.x, position.y               , time * 0.1)) * 0.2 +
			          surface3(vec3(position.x               , position.y - offsetSize.y, time * 0.1)) * 0.2 +
			          surface3(vec3(position.x               , position.y + offsetSize.y, time * 0.1)) * 0.2;

			gl_FragColor = mix( colorBubble, c, n );
		}
);

BubbleNoise::BubbleNoise( int w, int h )
{
	ci::gl::Fbo::Format format;
	format.setWrap( GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE );
	format.enableDepthBuffer( false );
	mOutputFbo = ci::gl::Fbo( w, h, format );

	try
	{
		mBubbleNoiseShader = ci::gl::GlslProg::create( sBubbleNoiseVertexShader, sBubbleNoiseFragmentShader );
		mBubbleNoiseShader->bind();
		mBubbleNoiseShader->uniform( "tex", 0 );
		mBubbleNoiseShader->unbind();
	}
	catch( const ci::gl::GlslProgCompileExc &exc )
	{
		ci::app::console() << exc.what() << std::endl;
	}
}

ci::gl::Texture & BubbleNoise::process( const ci::gl::Texture &source, float time, float freq, float contrast, const ci::ColorA& colorBubble, const ci::Vec2f& offset )
{
	ci::gl::SaveFramebufferBinding bindingSaver;
	glPushAttrib( GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT | GL_VIEWPORT_BIT );

	ci::gl::pushMatrices();

	mOutputFbo.bindFramebuffer();

	ci::gl::setMatricesWindow( mOutputFbo.getSize(), false );
	ci::gl::setViewport( mOutputFbo.getBounds() );

	ci::gl::color( Color::white() );

	mBubbleNoiseShader->bind();

	mBubbleNoiseShader->uniform( "time", time );
	mBubbleNoiseShader->uniform( "freq", freq );
	mBubbleNoiseShader->uniform( "contrast", contrast );
	mBubbleNoiseShader->uniform( "color", colorBubble );
	mBubbleNoiseShader->uniform( "offset", offset );

	ci::gl::draw( source, mOutputFbo.getBounds() );

	mBubbleNoiseShader->unbind();

	ci::gl::popMatrices();
	glPopAttrib();

	return mOutputFbo.getTexture();
}

} } } // namespace mndl::gl::fx

