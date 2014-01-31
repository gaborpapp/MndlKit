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

#pragma once

#include "cinder/params/Params.h"
#include "cinder/Xml.h"
#include "cinder/Utilities.h"

#include <vector>
#include <string>

#include <boost/any.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

namespace mndl { namespace params {

typedef std::shared_ptr< class PInterfaceGl > PInterfaceGlRef;

class PInterfaceGl : public ci::params::InterfaceGl {
 public:
	static PInterfaceGlRef create( const std::string &title, const ci::Vec2i &size,
								   const ci::Vec2i &pos = ci::Vec2i::zero(),
								   const ci::ColorA &color = ci::ColorA( 0.3f, 0.3f, 0.3f, 0.4f ) );
	static PInterfaceGlRef create( ci::app::WindowRef window,
								   const std::string &title, const ci::Vec2i &size,
								   const ci::Vec2i &pos = ci::Vec2i::zero(),
								   const ci::ColorA &color = ci::ColorA( 0.3f, 0.3f, 0.3f, 0.4f ) );

	/** Add a persistent parameter for the window size, position and iconified status
	 * Persistent parameter will be initialized with saved value if found, or with
	 * supplied default otherwise
	 */
	void addPersistentSizeAndPosition();

	/** Add a persistent parameter to the window.  Persistent parameter will be
	 * initialized with saved value if found, or with supplied default
	 * otherwise
	 */
	template<typename T, typename TVAL>
	void addPersistentParam(const std::string& name, T* var, TVAL defVal,
			const std::string& optionsStr="", bool readOnly=false)
	{
		addParam(name,var,optionsStr,readOnly);
		std::string id = name2id(name);
		*var = getSettingsXml().hasChild( m_id + "/" + id )
			? getSettingsXml().getChild( m_id + "/" + id ).getValue((T)defVal)
			: (T)defVal;
		persistCallbacks().push_back(
				boost::bind( &PInterfaceGl::persistParam<T>, this, var, id ) );
	}

	void addPersistentParam(const std::string& name, ci::Color *var, const ci::Color &defVal,
			const std::string& optionsStr="", bool readOnly=false);
	void addPersistentParam(const std::string& name, ci::ColorA *var, const ci::ColorA &defVal,
			const std::string& optionsStr="", bool readOnly=false);

	template<typename T>
	void addPersistentParam(const std::string& name, ci::Vec3<T> *var, const ci::Vec3<T> &defVal,
			const std::string& optionsStr="", bool readOnly=false)
	{
		addParam(name,var,optionsStr,readOnly);
		const std::string id = name2id(name);
		var->x = getSettingsXml().hasChild( m_id + "/" + id + "_x" )
			? getSettingsXml().getChild( m_id + "/" + id + "_x" ).getValue(defVal.x)
			: defVal.x;
		var->y = getSettingsXml().hasChild( m_id + "/" + id + "_y" )
			? getSettingsXml().getChild( m_id + "/" + id + "_y" ).getValue(defVal.y)
			: defVal.y;
		var->z = getSettingsXml().hasChild( m_id + "/" + id + "_z" )
			? getSettingsXml().getChild( m_id + "/" + id + "_z" ).getValue(defVal.z)
			: defVal.z;
		persistCallbacks().push_back(
				boost::bind( &PInterfaceGl::persistParam<T>, this, &(var->x), id+"_x" ) );
		persistCallbacks().push_back(
				boost::bind( &PInterfaceGl::persistParam<T>, this, &(var->y), id+"_y" ) );
		persistCallbacks().push_back(
				boost::bind( &PInterfaceGl::persistParam<T>, this, &(var->z), id+"_z" ) );
	}

	template<typename T>
	void addPersistentParam(const std::string& name, ci::Quaternion<T> *var, const ci::Quaternion<T> &defVal,
			const std::string& optionsStr="", bool readOnly=false)
	{
		addParam(name,var,optionsStr,readOnly);
		const std::string id = name2id(name);
		var->v.x = getSettingsXml().hasChild( m_id + "/" + id + "_x" )
			? getSettingsXml().getChild( m_id + "/" + id + "_x" ).getValue(defVal.v.x)
			: defVal.v.x;
		var->v.y = getSettingsXml().hasChild( m_id + "/" + id + "_y" )
			? getSettingsXml().getChild( m_id + "/" + id +"_y" ).getValue(defVal.v.y)
			: defVal.v.y;
		var->v.z = getSettingsXml().hasChild( m_id + "/" + id + "_z" )
			? getSettingsXml().getChild( m_id + "/" + id + "_z" ).getValue(defVal.v.z)
			: defVal.v.z;
		var->w = getSettingsXml().hasChild( m_id + "/" + id + "_w" )
			? getSettingsXml().getChild( m_id + "/" + id + "_w" ).getValue(defVal.w)
			: defVal.w;
		persistCallbacks().push_back(
				boost::bind( &PInterfaceGl::persistParam<T>, this, &(var->v.x), id+"_x" ) );
		persistCallbacks().push_back(
				boost::bind( &PInterfaceGl::persistParam<T>, this, &(var->v.y), id+"_y" ) );
		persistCallbacks().push_back(
				boost::bind( &PInterfaceGl::persistParam<T>, this, &(var->v.z), id+"_z" ) );
		persistCallbacks().push_back(
				boost::bind( &PInterfaceGl::persistParam<T>, this, &(var->w), id+"_w" ) );
	}

	//! Adds enumerated persistent parameter. The value corresponds to the indices of \a enumNames.
	void addPersistentParam(const std::string& name, std::vector<std::string> &enumNames, int* var, int defVal,
			const std::string& optionsStr="", bool readOnly=false);

	/** Adds presets to the bar. Presets containing all variables in
	 *  \a vars are stored and restored with an assigned label.
	 */
	void addPresets( std::vector< std::pair< std::string, boost::any > > &vars );

	//! Shows/hides all bars except help, which is always hidden if \a alwaysHideHelp is set.
	static void showAllParams( bool visible, bool alwaysHideHelp = true );

	//! Iconifies or deiconifies all bars. Hides help is \a alwaysHideHelp is set.
	static void maximizeAllParams( bool maximized = true, bool alwaysHideHelp = true );

	/** Loads persistent params. At the moment this only works when
	 * called at application start up, before creating persistent parameters.
	 */
	static void readSettings( const ci::DataSourceRef &source  );

	/** Save persistent params. */
	static void writeSettings( const ci::DataTargetRef &target );

protected:
	PInterfaceGl( const std::string &title, const ci::Vec2i &size,
				  const ci::Vec2i &pos = ci::Vec2i::zero(),
				  const ci::ColorA colorA = ci::ColorA( 0.3f, 0.3f, 0.3f, 0.4f ) );
	PInterfaceGl( ci::app::WindowRef window,
				  const std::string &title, const ci::Vec2i &size,
				  const ci::Vec2i &pos = ci::Vec2i::zero(),
				  const ci::ColorA colorA = ci::ColorA( 0.3f, 0.3f, 0.3f, 0.4f ) );

	std::string m_id;
	std::vector< boost::function< void() > > mPersistCallbacks;
	ci::XmlTree mRoot;

	// "manager"
	struct Manager
	{
		ci::XmlTree mSettingsXml;
		std::vector< std::weak_ptr< PInterfaceGl > > mBars;
	};

	static Manager& manager() {
		static Manager * m = new Manager();
		return *m;
	}

	static ci::XmlTree & getSettingsXml()
	{
		return manager().mSettingsXml;
	}

	static std::vector< std::weak_ptr< PInterfaceGl > >& getBars()
	{
		return manager().mBars;
	}

	std::vector< boost::function< void() > >& persistCallbacks()
	{
		return mPersistCallbacks;
	}

	ci::XmlTree& root()
	{
		return mRoot;
	}

	// save current size, position and iconified status value into an xml tree
	void persistSizeAndPosition();

	// save current parameter value into an xml tree
	template<typename T>
	void persistParam(T * var, const std::string& paramId)
	{
		std::vector< std::string > tokens = ci::split( paramId, "/" );
		std::string parentId = "";
		for ( auto it = tokens.cbegin(); it != tokens.cend(); ++it )
		{
			if ( !getXml().hasChild( parentId + "/" + *it ) )
			{
				if ( parentId == "" )
					getXml().push_back( ci::XmlTree( *it, "" ) );
				else
					getXml().getChild( parentId ).push_back( ci::XmlTree( *it, "" ) );
			}
			parentId += "/" + *it;
		}

		getXml().getChild(paramId).setValue(*var);
	}

	std::string colorToHex(const ci::ColorA &color);
	ci::ColorA hexToColor(const std::string &hex);

	void persistColor(ci::Color *var, const std::string& paramId);
	void persistColorA(ci::ColorA *var, const std::string& paramId);

	ci::XmlTree& getXml()
	{
		if (root().getTag() == "")
			root().setTag( m_id );
		return root();
	}

	// convert "some title" to SomeTitle so it can be used as XML tag
	static std::string name2id( const std::string& name );
};

} } // namespace mndl::params

