/*
 Copyright (c) 2010, The Barbarian Group
 All rights reserved.

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and
	the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
	the following disclaimer in the documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
*/

#include "cinder/app/App.h"
#include "mndlkit/params/PParams.h"
#include "cinder/Filesystem.h"
#include "cinder/Utilities.h"

#include "AntTweakBar.h"

#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>

#include <ctype.h>

using namespace std;
using namespace ci;

namespace mndl { namespace params {

PInterfaceGl::PInterfaceGl( const std::string &title, const ci::Vec2i &size, const ci::Vec2i &pos /* = Vec2i::zero() */,
							const ci::ColorA colorA /* = ColorA( 0.3f, 0.3f, 0.3f, 0.4f ) */ ) :
	ci::params::InterfaceGl( title, size, colorA ), m_id( name2id( title ) )
{
	TwSetCurrentWindow( mTwWindowId );
	if ( pos != ci::Vec2i::zero() )
		TwSetParam( mBar.get(), NULL, "position", TW_PARAM_INT32, 2, pos.ptr() );
}

PInterfaceGl::PInterfaceGl( ci::app::WindowRef window,
							const std::string &title, const ci::Vec2i &size, const ci::Vec2i &pos /* = Vec2i::zero() */,
							const ci::ColorA colorA /* = ColorA( 0.3f, 0.3f, 0.3f, 0.4f ) */ ) :
	ci::params::InterfaceGl( window, title, size, colorA ), m_id( name2id( title ) )
{
	TwSetCurrentWindow( mTwWindowId );
	if ( pos != ci::Vec2i::zero() )
		TwSetParam( mBar.get(), NULL, "position", TW_PARAM_INT32, 2, pos.ptr() );
}

PInterfaceGlRef PInterfaceGl::create( const std::string &title, const Vec2i &size,
									  const ci::Vec2i &pos /* = ci::Vec2i::zero() */,
									  const ColorA &color /* = ColorA( 0.3f, 0.3f, 0.3f, 0.4f ) */ )
{
	PInterfaceGlRef barRef = PInterfaceGlRef( new PInterfaceGl( title, size, pos, color ) );
	getBars().push_back( barRef );
	return barRef;
}

PInterfaceGlRef PInterfaceGl::create( ci::app::WindowRef window,
									  const std::string &title, const Vec2i &size,
									  const ci::Vec2i &pos /* = ci::Vec2i::zero() */,
									  const ColorA &color /* = ColorA( 0.3f, 0.3f, 0.3f, 0.4f ) */ )
{
	PInterfaceGlRef barRef = PInterfaceGlRef( new PInterfaceGl( window, title, size, pos, color ) );
	getBars().push_back( barRef );
	return barRef;
}

std::string PInterfaceGl::name2id( const std::string& name )
{
	std::string id = "";
	enum State { START, APPEND, UPCASE };
	State state(START);

	BOOST_FOREACH(char c, name) {
		switch(state) {
			case START:
				if (isalpha(c)) {
					id += c;
					state = APPEND;
				} else if (isdigit(c)) {
					id = "_" + string( 1, c );
					state = APPEND;
				}
				break;
			case APPEND:
				if (isalnum(c)) {
					id += c;
				} else {
					state = UPCASE;
				}
				break;
			case UPCASE:
				if (islower(c)) {
					id += toupper(c);
					state = APPEND;
				} else if (isalnum(c)) {
					id += c;
					state = APPEND;
				}
				break;
		}
	}
	return id;
}

void PInterfaceGl::readSettings( const ci::DataSourceRef &source  )
{
	try
	{
		getSettingsXml() = XmlTree( source );
	}
	catch ( ... )
	{ }
}

void PInterfaceGl::writeSettings( const ci::DataTargetRef &target )
{
	ci::XmlTree doc = ci::XmlTree::createDoc();
	for ( std::weak_ptr< PInterfaceGl > barWeak : getBars() )
	{
		auto bar = barWeak.lock();
		if ( !bar )
		{
			continue;
		}

		BOOST_FOREACH( boost::function< void() > f, bar->mPersistCallbacks )
		{
			f();
		}
		doc.push_back( bar->mRoot );
	}
	doc.write( target );
}

void PInterfaceGl::addPersistentSizeAndPosition()
{
	TwSetCurrentWindow( mTwWindowId );

	int size[2];
	TwGetParam( mBar.get(), NULL, "size", TW_PARAM_INT32, 2, size );

	std::string idW = m_id + "/" + name2id("width");
	size[0] = getSettingsXml().hasChild(idW)
		? getSettingsXml().getChild(idW).getValue((int)size[0])
		: size[0];

	std::string idH = m_id + "/" + name2id("height");
	size[1] = getSettingsXml().hasChild(idH)
		? getSettingsXml().getChild(idH).getValue((int)size[1])
		: size[1];

	TwSetParam( mBar.get(), NULL, "size", TW_PARAM_INT32, 2, size );

	int pos[2];
	TwGetParam( mBar.get(), NULL, "position", TW_PARAM_INT32, 2, pos );

	std::string idX = m_id + "/" + name2id("posx");
	pos[0] = getSettingsXml().hasChild(idX)
		? getSettingsXml().getChild(idX).getValue((float)pos[0])
		: pos[0];

	std::string idY = m_id + "/" + name2id("posy");
	pos[1] = getSettingsXml().hasChild(idY)
		? getSettingsXml().getChild(idY).getValue((float)pos[1])
		: pos[1];

	TwSetParam( mBar.get(), NULL, "position", TW_PARAM_INT32, 2, pos );

	int icon;
	TwGetParam( mBar.get(), NULL, "iconified", TW_PARAM_INT32, 1, &icon );

	std::string idIcon = m_id + "/" + name2id("icon");
	icon = getSettingsXml().hasChild(idIcon)
		? getSettingsXml().getChild(idIcon).getValue((int)icon)
		: icon;

	TwSetParam( mBar.get(), NULL, "iconified", TW_PARAM_INT32, 1, &icon );

	int valuesWidth;
	TwGetParam( mBar.get(), NULL, "valueswidth", TW_PARAM_INT32, 1, &valuesWidth );

	std::string idValuesWidth = m_id + "/" + name2id("valueswidth");
	valuesWidth = getSettingsXml().hasChild(idValuesWidth)
		? getSettingsXml().getChild(idValuesWidth).getValue((int)valuesWidth)
		: valuesWidth;

	TwSetParam( mBar.get(), NULL, "valueswidth", TW_PARAM_INT32, 1, &valuesWidth );

	persistCallbacks().push_back(
			boost::bind( &PInterfaceGl::persistSizeAndPosition, this) );
}

void PInterfaceGl::persistSizeAndPosition()
{
	TwSetCurrentWindow( mTwWindowId );

	int size[2];
	TwGetParam( mBar.get(), NULL, "size", TW_PARAM_INT32, 2, size );

	std::string idW = name2id("width");
	if (!getXml().hasChild(idW))
		getXml().push_back(XmlTree(idW,""));
	getXml().getChild(idW).setValue(size[0]);

	std::string idH = name2id("height");
	if (!getXml().hasChild(idH))
		getXml().push_back(XmlTree(idH,""));
	getXml().getChild(idH).setValue(size[1]);

	int pos[2];
	TwGetParam( mBar.get(), NULL, "position", TW_PARAM_INT32, 2, pos );

	std::string idX = name2id("posx");
	if (!getXml().hasChild(idX))
		getXml().push_back(XmlTree(idX,""));
	getXml().getChild(idX).setValue(pos[0]);

	std::string idY = name2id("posy");
	if (!getXml().hasChild(idY))
		getXml().push_back(XmlTree(idY,""));
	getXml().getChild(idY).setValue(pos[1]);

	int icon;
	TwGetParam( mBar.get(), NULL, "iconified", TW_PARAM_INT32, 1, &icon );

	std::string idIcon = name2id("icon");
	if (!getXml().hasChild(idIcon))
		getXml().push_back(XmlTree(idIcon,""));
	getXml().getChild(idIcon).setValue(icon);

	int valuesWidth;
	TwGetParam( mBar.get(), NULL, "valueswidth", TW_PARAM_INT32, 1, &valuesWidth );
	std::string idValuesWidth = name2id("valueswidth");
	if (!getXml().hasChild(idValuesWidth))
		getXml().push_back(XmlTree(idValuesWidth,""));
	getXml().getChild(idValuesWidth).setValue(valuesWidth);
}

//! Adds enumerated persistent parameter. The value corresponds to the indices of \a enumNames.
void PInterfaceGl::addPersistentParam(const std::string& name, std::vector<std::string> &enumNames,
		int* var, int defVal, const std::string& optionsStr, bool readOnly)
{
	addParam(name, enumNames, var, optionsStr, readOnly);
	std::string id = name2id(name);
	*var = getSettingsXml().hasChild( m_id + "/" + id )
		? getSettingsXml().getChild( m_id + "/" + id ).getValue(defVal)
		: defVal;
	persistCallbacks().push_back(
			boost::bind( &PInterfaceGl::persistParam<int>, this, var, id ) );
}

void PInterfaceGl::showAllParams( bool visible, bool alwaysHideHelp /* = true */ )
{
	int windowId = 0;

	while ( TwWindowExists( windowId ) )
	{
		TwSetCurrentWindow( windowId );
		int barCount = TwGetBarCount();

		int32_t visibleInt = visible ? 1 : 0;
		for ( int i = 0; i < barCount; ++i )
		{
			TwBar *bar = TwGetBarByIndex( i );
			TwSetParam( bar, NULL, "visible", TW_PARAM_INT32, 1, &visibleInt );
		}

		windowId++;
	}

	if ( alwaysHideHelp )
		TwDefine( "TW_HELP visible=false" );
}

void PInterfaceGl::maximizeAllParams( bool maximized /* = true */, bool alwaysHideHelp /* = true */ )
{
	int windowId = 0;

	while ( TwWindowExists( windowId ) )
	{
		TwSetCurrentWindow( windowId );
		int barCount = TwGetBarCount();

		int32_t maximizedInt = maximized ? 0 : 1;
		for ( int i = 0; i < barCount; ++i )
		{
			TwBar *bar = TwGetBarByIndex( i );
			TwSetParam( bar, NULL, "iconified", TW_PARAM_INT32, 1, &maximizedInt );
		}

		windowId++;
	}

	if ( alwaysHideHelp )
		TwDefine( "TW_HELP visible=false" );
}

// string-color onversion by Paul Houx
std::string PInterfaceGl::colorToHex(const ci::ColorA &color)
{
	unsigned int a = ((unsigned int) (color.a * 255) & 0xFF) << 24;
	unsigned int r = ((unsigned int) (color.r * 255) & 0xFF) << 16;
	unsigned int g = ((unsigned int) (color.g * 255) & 0xFF) << 8;
	unsigned int b = ((unsigned int) (color.b * 255) & 0xFF);

	unsigned int value = a + r + g + b;

	std::stringstream clr;
	clr << std::hex << value;

	return clr.str();
}

ci::ColorA PInterfaceGl::hexToColor(const std::string &hex)
{
	std::stringstream converter(hex);
	unsigned int value;
	converter >> std::hex >> value;

	float a = ((value >> 24) & 0xFF) / 255.0f;
	float r = ((value >> 16) & 0xFF) / 255.0f;
	float g = ((value >> 8) & 0xFF) / 255.0f;
	float b = ((value) & 0xFF) / 255.0f;

	return ci::ColorA(r, g, b, a);
}

void PInterfaceGl::addPersistentParam(const std::string& name, ci::ColorA *var, const ci::ColorA &defVal,
		const std::string& optionsStr, bool readOnly)
{
	addParam(name,var,optionsStr,readOnly);
	const std::string id = name2id(name);
	std::string colorStr = getSettingsXml().hasChild( m_id + "/" + id )
			? getSettingsXml().getChild( m_id + "/" + id ).getValue(colorToHex(defVal))
			: colorToHex(defVal);
	*var = hexToColor( colorStr );
	persistCallbacks().push_back(
		boost::bind( &PInterfaceGl::persistColorA, this, var, id ) );
}

void PInterfaceGl::persistColorA(ci::ColorA *var, const std::string& paramId)
{
	if (!getXml().hasChild(paramId))
		getXml().push_back(XmlTree(paramId,""));
	getXml().getChild(paramId).setValue( colorToHex( *var ) );
}

void PInterfaceGl::addPersistentParam(const std::string& name, ci::Color *var, const ci::Color &defVal,
		const std::string& optionsStr, bool readOnly)
{
	addParam(name,var,optionsStr,readOnly);
	const std::string id = name2id(name);
	std::string colorStr = getSettingsXml().hasChild( m_id + "/" + id )
			? getSettingsXml().getChild( m_id + "/" + id ).getValue(colorToHex(defVal))
			: colorToHex(defVal);
	ColorA ca = hexToColor( colorStr );
	*var = Color( ca.r, ca.g, ca.b );
	persistCallbacks().push_back(
		boost::bind( &PInterfaceGl::persistColor, this, var, id ) );
}

void PInterfaceGl::persistColor(ci::Color *var, const std::string& paramId)
{
	if (!getXml().hasChild(paramId))
		getXml().push_back(XmlTree(paramId,""));
	getXml().getChild(paramId).setValue( colorToHex( *var ) );
}

} } // namespace mndl::params

