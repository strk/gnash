// DisplayObject.cpp:  ActionScript DisplayObject class, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
// 


#ifdef HAVE_CONFIG_H
#include "gnashconfig.h" // USE_SWFTREE
#endif

#include "smart_ptr.h" // GNASH_USE_GC
#include "DisplayObject.h"
#include "movie_root.h"
#include "MovieClip.h"
#include "drag_state.h" // for do_mouse_drag (to be moved in movie_root)
#include "VM.h" // for do_mouse_drag (to be moved in movie_root)
#include "fn_call.h" // for shared ActionScript getter-setters
#include "GnashException.h" 
#include "ExecutableCode.h"
#include "namedStrings.h"
#include "gnash.h" // Quality
#include "GnashNumeric.h"
#include "Global_as.h"
#include "Renderer.h"

#ifdef USE_SWFTREE
# include "tree.hh"
#endif

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/bind.hpp>

#undef set_invalidated

namespace gnash
{

// Forward declarations.
namespace {
    /// Match blend modes.
    typedef std::map<DisplayObject::BlendMode, std::string> BlendModeMap;
    const BlendModeMap& getBlendModeMap();
    bool blendModeMatches(const BlendModeMap::value_type& val,
            const std::string& mode);
    
    typedef as_value(*Getter)(DisplayObject&);
    typedef std::map<string_table::key, Getter> Getters;
    typedef void(*Setter)(DisplayObject&, const as_value&);
    typedef std::map<string_table::key, Setter> Setters;

    const Getters displayObjectGetters();
    const Setters displayObjectSetters();

    bool doSet(string_table::key prop, DisplayObject& o, const as_value& val);
    bool doGet(string_table::key prop, DisplayObject& o, as_value& val);
    string_table::key getPropertyByIndex(size_t index);
}

// Define static const members.
const int DisplayObject::lowerAccessibleBound;
const int DisplayObject::upperAccessibleBound;
const int DisplayObject::staticDepthOffset;
const int DisplayObject::removedDepthOffset;
const int DisplayObject::noClipDepthValue;

DisplayObject::DisplayObject(movie_root& mr, as_object* object,
        DisplayObject* parent)
    :
    _name(),
    _parent(parent),
    _object(object),
    _stage(mr),
    _xscale(100),
    _yscale(100),
    _rotation(0),
    _depth(0),
    _volume(100),
    _ratio(0),
    m_clip_depth(noClipDepthValue),
    _mask(0),
    _maskee(0),
    _blendMode(BLENDMODE_NORMAL),
    _visible(true),
    _scriptTransformed(false),
    _dynamicallyCreated(false),
    _unloaded(false),
    _destroyed(false),
    _invalidated(true),
    _child_invalidated(true)
{
    assert(m_old_invalidated_ranges.isNull());

    // This informs the core that the object is a DisplayObject.
    if (_object) _object->setDisplayObject(this);
}

as_object*
DisplayObject::object() const
{
    return _object;
}
    
bool
DisplayObject::unloaded() const
{
    return _unloaded;
}

void
DisplayObject::getLoadedMovie(Movie* extern_movie)
{
    LOG_ONCE(
    log_unimpl("loadMovie against a %s DisplayObject", typeName(*this))
    );

    // TODO: look at the MovieClip implementation, but most importantly
    //       test all the event handlers copies etc..

    UNUSED(extern_movie);
}

string_table::key
DisplayObject::getNextUnnamedInstanceName()
{
    assert(_object);
    movie_root& mr = getRoot(*_object);
	std::ostringstream ss;
	ss << "instance" << mr.nextUnnamedInstance();

    string_table& st = getStringTable(*_object);
	return st.find(ss.str());
}


SWFMatrix
DisplayObject::getWorldMatrix(bool includeRoot) const
{
	SWFMatrix m;
	if (_parent) {
	    m = _parent->getWorldMatrix(includeRoot);
	}
    if (_parent || includeRoot) m.concatenate(getMatrix());

	return m;
}

int
DisplayObject::getWorldVolume() const
{
	int volume=_volume;
	if (_parent != NULL)
	{
	    volume = int(volume*_parent->getVolume()/100.0);
	}

	return volume;
}

cxform
DisplayObject::get_world_cxform() const
{
	cxform	m;
	if (_parent != NULL)
	{
	    m = _parent->get_world_cxform();
	}
	m.concatenate(get_cxform());

	return m;
}


as_object*
DisplayObject::pathElement(string_table::key key)
{
    as_object* obj = getObject(this);
    if (!obj) return 0;

    string_table& st = stage().getVM().getStringTable();
    if (key == st.find("..")) return getObject(get_parent());
	if (key == st.find(".")) return obj;
    
    // The check is case-insensitive for SWF6 and below.
    if (equal(st, key, NSV::PROP_THIS, caseless(*obj))) {
        return obj;
    }
	return 0;
}

void 
DisplayObject::set_invalidated()
{
	set_invalidated("unknown", -1);
}

void
DisplayObject::set_invalidated(const char* debug_file, int debug_line)
{
	// Set the invalidated-flag of the parent. Note this does not mean that
	// the parent must re-draw itself, it just means that one of it's childs
	// needs to be re-drawn.
	if ( _parent ) _parent->set_child_invalidated(); 
  
	// Ok, at this point the instance will change it's
	// visual aspect after the
	// call to set_invalidated(). We save the *current*
	// position of the instance because this region must
	// be updated even (or first of all) if the DisplayObject
	// moves away from here.
	// 
	if ( ! _invalidated )
	{
		_invalidated = true;
		
		#ifdef DEBUG_SET_INVALIDATED
		log_debug("%p set_invalidated() of %s in %s:%d",
			(void*)this, get_name(), debug_file, debug_line);
		#else
		UNUSED(debug_file);
		UNUSED(debug_line);
		#endif
		
		// NOTE: the SnappingRanges instance used here is not initialized by the
		// GUI and therefore uses the default settings. This should not be a 
		// problem but special snapping ranges configuration done in gui.cpp
		// is ignored here... 
				
		m_old_invalidated_ranges.setNull();
		add_invalidated_bounds(m_old_invalidated_ranges, true);
	}

}

void
DisplayObject::add_invalidated_bounds(InvalidatedRanges& ranges, bool force)
{
    ranges.add(m_old_invalidated_ranges);
    if (visible() && (_invalidated||force))
    {
        SWFRect bounds;        
        bounds.expand_to_transformed_rect(getWorldMatrix(), getBounds());
        ranges.add(bounds.getRange());                        
    }        
}

void
DisplayObject::set_child_invalidated()
{
  if ( ! _child_invalidated ) 
  {
    _child_invalidated=true;
  	if ( _parent ) _parent->set_child_invalidated();
  } 
}

void
DisplayObject::extend_invalidated_bounds(const InvalidatedRanges& ranges)
{
	set_invalidated(__FILE__, __LINE__);
	m_old_invalidated_ranges.add(ranges);
}

as_value
DisplayObject::blendMode(const fn_call& fn)
{
    DisplayObject* ch = ensure<IsDisplayObject<> >(fn);

    // This is AS-correct, but doesn't do anything.
    // TODO: implement in the renderers!
    LOG_ONCE(log_unimpl(_("blendMode")));

    if (!fn.nargs)
    {
        // Getter
        BlendMode bm = ch->getBlendMode();

        /// If the blend mode is undefined, it doesn't return a string.
        if (bm == BLENDMODE_UNDEFINED) return as_value();

        std::ostringstream blendMode;
        blendMode << bm;
        return as_value(blendMode.str());
    }

    //
    // Setter
    //
    
    const as_value& bm = fn.arg(0);

    // Undefined argument sets blend mode to normal.
    if (bm.is_undefined()) {
        ch->setBlendMode(BLENDMODE_NORMAL);
        return as_value();
    }

    // Numeric argument.
    if (bm.is_number()) {
        double mode = bm.to_number();

        // Hardlight is the last known value. This also performs range checking
        // for float-to-int conversion.
        if (mode < 0 || mode > BLENDMODE_HARDLIGHT) {

            // An invalid numeric argument becomes undefined.
            ch->setBlendMode(BLENDMODE_UNDEFINED);
        }
        else {
            /// The extra static cast is required to keep OpenBSD happy.
            ch->setBlendMode(static_cast<BlendMode>(static_cast<int>(mode)));
        }
        return as_value();
    }

    // Other arguments use toString method.
    const std::string& mode = bm.to_string();

    const BlendModeMap& bmm = getBlendModeMap();
    BlendModeMap::const_iterator it = std::find_if(bmm.begin(), bmm.end(),
            boost::bind(blendModeMatches, _1, mode));

    if (it != bmm.end()) {
        ch->setBlendMode(it->first);
    }

    // An invalid string argument has no effect.

    return as_value();

}

void
DisplayObject::set_visible(bool visible)
{
    if (_visible != visible) set_invalidated(__FILE__, __LINE__);

    // Remove focus from this DisplayObject if it changes from visible to
    // invisible (see Selection.as).
    if (_visible && !visible) {
        assert(_object);
        movie_root& mr = getRoot(*_object);
        if (mr.getFocus() == this) {
            mr.setFocus(0);
        }
    }
    _visible = visible;      
}

void
DisplayObject::setWidth(double newwidth)
{
	const SWFRect& bounds = getBounds();
	const double oldwidth = bounds.width();
	assert(oldwidth >= 0); 

    const double xscale = oldwidth ? (newwidth / oldwidth) : 0; 
    const double rotation = _rotation * PI / 180.0;

    SWFMatrix m = getMatrix();
    const double yscale = m.get_y_scale(); 
    m.set_scale_rotation(xscale, yscale, rotation);
    setMatrix(m, true); 
}

as_value
getHeight(DisplayObject& o)
{
	SWFRect bounds = o.getBounds();
    const SWFMatrix m = o.getMatrix();
    m.transform(bounds);
    return twipsToPixels(bounds.height());      
}

void
setHeight(DisplayObject& o, const as_value& val)
{
    const double newheight = pixelsToTwips(val.to_number());
    if (newheight <= 0) {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Setting _height=%g of DisplayObject %s (%s)"),
                        newheight / 20, o.getTarget(), typeName(o));
        );
    }
    o.setHeight(newheight);
}

void
DisplayObject::setHeight(double newheight)
{
	const SWFRect& bounds = getBounds();

	const double oldheight = bounds.height();
	assert(oldheight >= 0); 

    const double yscale = oldheight ? (newheight / oldheight) : 0;
    const double rotation = _rotation * PI / 180.0;

    SWFMatrix m = getMatrix();
    const double xscale = m.get_x_scale();
    m.set_scale_rotation(xscale, yscale, rotation);
    setMatrix(m, true);
}

void
DisplayObject::setMatrix(const SWFMatrix& m, bool updateCache)
{

    if (m == m_matrix) return;

    //log_debug("setting SWFMatrix to: %s", m);
    set_invalidated(__FILE__, __LINE__);
    m_matrix = m;

    // don't update caches if SWFMatrix wasn't updated too
    if (updateCache) 
    {
        _xscale = m_matrix.get_x_scale() * 100.0;
        _yscale = m_matrix.get_y_scale() * 100.0;
        _rotation = m_matrix.get_rotation() * 180.0 / PI;
    }

}

void
DisplayObject::set_event_handlers(const Events& copyfrom)
{
	for (Events::const_iterator it=copyfrom.begin(), itE=copyfrom.end();
			it != itE; ++it)
	{
		const event_id& ev = it->first;
		const BufferList& bufs = it->second;
		for (size_t i = 0, e = bufs.size(); i < e; ++i)
		{
			const action_buffer* buf = bufs[i];
			assert(buf);
			add_event_handler(ev, *buf);
		}	
	}
}

void
DisplayObject::add_event_handler(const event_id& id, const action_buffer& code)
{
	_event_handlers[id].push_back(&code);

	// todo: drop the DisplayObject as a listener
	//       if it gets no valid handlers for
	//       mouse or Key events.
}

std::auto_ptr<ExecutableCode>
DisplayObject::get_event_handler(const event_id& id) const
{
	std::auto_ptr<ExecutableCode> handler;

	Events::const_iterator it = _event_handlers.find(id);
	if ( it == _event_handlers.end() ) return handler;

#ifndef GNASH_USE_GC
	assert(get_ref_count() > 0);
#endif // GNASH_USE_GC
	DisplayObject* this_ptr = const_cast<DisplayObject*>(this);

	handler.reset( new EventCode(this_ptr, it->second) );
	return handler;
}

bool
DisplayObject::unload()
{

    const bool childHandler = unloadChildren();

	if (!_unloaded) {
		queueEvent(event_id::UNLOAD, movie_root::PRIORITY_DOACTION);
	}

    // Unregister this DisplayObject as mask and/or maskee.
    if (_maskee) _maskee->setMask(0);
    if (_mask) _mask->setMaskee(0);

	const bool hasEvent = hasEventHandler(event_id::UNLOAD) || childHandler;

    if (!hasEvent) {
        stage().removeQueuedConstructor(this);
    }

	_unloaded = true;

	return hasEvent;
}

void
DisplayObject::queueEvent(const event_id& id, int lvl)
{
    if (!_object) return;
	std::auto_ptr<ExecutableCode> event(new QueuedEvent(this, id));
	stage().pushAction(event, lvl);
}

bool
DisplayObject::hasEventHandler(const event_id& id) const
{
	Events::const_iterator it = _event_handlers.find(id);
	if (it != _event_handlers.end()) return true;

    if (!_object) return false;

    as_value tmp;
	if (_object->get_member(id.functionKey(), &tmp)) {
		return tmp.to_function();
	}
	return false;

}

/// Set the real and cached x scale.
//
/// Cached rotation and y scale are not updated.
void
DisplayObject::set_x_scale(double scale_percent)
{
    double xscale = scale_percent / 100.0;

    if (xscale != 0.0 && _xscale != 0.0)
    {
        if (scale_percent * _xscale < 0.0)
        {
            xscale = -std::abs(xscale);
        }
        else xscale = std::abs(xscale);
    }

    _xscale = scale_percent;

    // As per misc-ming.all/SWFMatrix_test.{c,swf}
    // we don't need to recompute the SWFMatrix from the 
    // caches.

	SWFMatrix m = getMatrix();

    m.set_x_scale(xscale);

    setMatrix(m); // we updated the cache ourselves

	transformedByScript(); 
}

/// Set the real and cached rotation.
//
/// Cached scale values are not updated.
void
DisplayObject::set_rotation(double rot)
{
	// Translate to the -180 .. 180 range
	rot = std::fmod(rot, 360.0);
	if (rot > 180.0) rot -= 360.0;
	else if (rot < -180.0) rot += 360.0;

	double rotation = rot * PI / 180.0;

    if (_xscale < 0 ) rotation += PI; 

	SWFMatrix m = getMatrix();
    m.set_rotation(rotation);

    // Update the matrix from the cached x scale to avoid accumulating
    // errors.
    // TODO: also update y scale? The x scale update is needed to keep
    // TextField correct; no tests for y scale.
    m.set_x_scale(std::abs(scaleX() / 100.0));
	setMatrix(m); // we update the cache ourselves

	_rotation = rot;

	transformedByScript(); 
}


/// Set the real and cached y scale.
//
/// Cached rotation and x scale are not updated.
void
DisplayObject::set_y_scale(double scale_percent)
{
    double yscale = scale_percent / 100.0;
    
    if (yscale != 0.0 && _yscale != 0.0)
    {
        if (scale_percent * _yscale < 0.0) yscale = -std::abs(yscale);
        else yscale = std::abs(yscale);
    }

	_yscale = scale_percent;

	SWFMatrix m = getMatrix();
    m.set_y_scale(yscale);
	setMatrix(m); // we updated the cache ourselves

	transformedByScript(); 
}


std::string
DisplayObject::getTargetPath() const
{
	// TODO: check what happens when this DisplayObject
	//       is a Movie loaded into another
	//       running movie.
	
	typedef std::vector<std::string> Path;
	Path path;

	// Build parents stack
	const DisplayObject* topLevel = 0;
	const DisplayObject* ch = this;

    string_table& st = getStringTable(*getObject(this));
	for (;;)
	{
		const DisplayObject* parent = ch->get_parent();

		// Don't push the _root name on the stack
		if (!parent) {
			topLevel = ch;
			break;
		}

		path.push_back(st.value(ch->get_name()));
		ch = parent;
	} 

	assert(topLevel);

	if (path.empty()) {
		if (&stage().getRootMovie() == this) return "/";
		std::stringstream ss;
		ss << "_level" << _depth-DisplayObject::staticDepthOffset;
		return ss.str();
	}

	// Build the target string from the parents stack
	std::string target;
	if (topLevel != &stage().getRootMovie()) {
		std::stringstream ss;
		ss << "_level" << 
            topLevel->get_depth() - DisplayObject::staticDepthOffset;
		target = ss.str();
	}
	for (Path::reverse_iterator it=path.rbegin(), itEnd=path.rend();
			it != itEnd; ++it) {
		target += "/" + *it; 
	}
	return target;
}


std::string
DisplayObject::getTarget() const
{

	// TODO: check what happens when this DisplayObject
	//       is a Movie loaded into another
	//       running movie.
	
	typedef std::vector<std::string> Path;
	Path path;

	// Build parents stack
	const DisplayObject* ch = this;
    string_table& st = stage().getVM().getStringTable();
	for (;;)
	{
		const DisplayObject* parent = ch->get_parent();

		// Don't push the _root name on the stack
		if (!parent) {

			std::stringstream ss;
			if (!dynamic_cast<const Movie*>(ch)) {
				// must be an as-referenceable
				// DisplayObject created using 'new'
				// like, new MovieClip, new Video, new TextField...
				//log_debug("DisplayObject %p (%s) doesn't have a parent and "
                //        "is not a Movie", ch, typeName(*ch));
				ss << "<no parent, depth" << ch->get_depth() << ">";
				path.push_back(ss.str());
			}
			else {
				ss << "_level" <<
                    ch->get_depth() - DisplayObject::staticDepthOffset;
				path.push_back(ss.str());
			}
			break;
		}

		path.push_back(st.value(ch->get_name()));
		ch = parent;
	} 

	assert (!path.empty());

	// Build the target string from the parents stack
	std::string target;
	for (Path::const_reverse_iterator it=path.rbegin(), itEnd=path.rend();
			it != itEnd; ++it) {

		if (!target.empty()) target += ".";
		target += *it; 
	}

	return target;
}


void
DisplayObject::destroy()
{
	// in case we are destroyed without being unloaded first
	// see bug #21842
	_unloaded = true;

	/// we may destory a DisplayObject that's not unloaded.
	///(we don't have chance to unload it in current model,
    /// see new_child_in_unload_test.c)
	/// We don't destroy ourself twice, right ?

    if (_object) _object->clearProperties();

	assert(!_destroyed);
	_destroyed = true;
}

void
DisplayObject::markReachableResources() const
{
    markOwnResources();
    if (_object) _object->setReachable();
	if (_parent) _parent->setReachable();
	if (_mask) _mask->setReachable();
	if (_maskee) _maskee->setReachable();
}

/// Whether to use a hand cursor when the mouse is over this DisplayObject
//
/// This depends on the useHandCursor AS property, but:
/// 1. Only AS-referenceable objects may use a hand cursor (TODO: check
///    Video). 
/// 2. Only objects with a release event may use a hand cursor.
/// 3. The default value (if the property is not defined) is true.
bool
DisplayObject::allowHandCursor() const
{
    if (!getObject(this)) return false;

    if (!hasEventHandler(event_id::RELEASE)) return false;

    as_value val;
    if (!getObject(this)->get_member(NSV::PROP_USEHANDCURSOR, &val)) {
         return true;
    }
    return val.to_bool();
}

void
DisplayObject::setMask(DisplayObject* mask)
{
	if ( _mask == mask ) return;

    set_invalidated();

	// Backup this before setMaskee has a chance to change it..
	DisplayObject* prevMaskee = _maskee;

	// If we had a previous mask unregister with it
	if ( _mask && _mask != mask )
	{
		// the mask will call setMask(NULL) 
		// on any previously registered maskee
		// so we make sure to set our _mask to 
		// NULL before getting called again
		_mask->setMaskee(0);
	}

	// if we had a maskee, notify it to stop using
	// us as a mask
	if (prevMaskee) prevMaskee->setMask(0);

	// TODO: should we reset any original clip depth
	//       specified by PlaceObject tag ?
	set_clip_depth(noClipDepthValue); 
	_mask = mask;
	_maskee = 0;

	if (_mask) {
		/// Register as as masked by the mask
		_mask->setMaskee(this);
	}
}

void
DisplayObject::setMaskee(DisplayObject* maskee)
{
	if ( _maskee == maskee ) { return; }

	if (_maskee) {
		// We don't want the maskee to call setMaskee(null)
		// on us again
		_maskee->_mask = 0;
	}

	_maskee = maskee;

	if (!maskee)
	{
		// TODO: should we reset any original clip depth
		//       specified by PlaceObject tag ?
		set_clip_depth(noClipDepthValue);
	}
}


bool 
DisplayObject::boundsInClippingArea(Renderer& renderer) const 
{
  SWFRect mybounds = getBounds();
  getWorldMatrix().transform(mybounds);
  
  return renderer.bounds_in_clipping_area(mybounds.getRange());  
}

#ifdef USE_SWFTREE
DisplayObject::InfoTree::iterator
DisplayObject::getMovieInfo(InfoTree& tr, InfoTree::iterator it)
{
	const std::string yes = _("yes");
	const std::string no = _("no");

	it = tr.append_child(it, StringPair(getTarget(), typeName(*this)));

	std::ostringstream os;
	os << get_depth();
	tr.append_child(it, StringPair(_("Depth"), os.str()));

    /// Don't add if the DisplayObject has no ratio value
    if (get_ratio() >= 0)
    {
        os.str("");
        os << get_ratio();
        tr.append_child(it, StringPair(_("Ratio"), os.str()));
    }	    

    /// Don't add if it's not a real clipping depth
    if (int cd = get_clip_depth() != noClipDepthValue)
    {
		os.str("");
		if (_maskee) os << "Dynamic mask";
		else os << cd;

		tr.append_child(it, StringPair(_("Clipping depth"), os.str()));	    
    }

    os.str("");
    os << getBounds().width() << "x" << getBounds().height();
	tr.append_child(it, StringPair(_("Dimensions"), os.str()));	

	tr.append_child(it, StringPair(_("Dynamic"), isDynamic() ? yes : no));	
	tr.append_child(it, StringPair(_("Mask"), isMaskLayer() ? yes : no));	    
	tr.append_child(it, StringPair(_("Destroyed"), isDestroyed() ? yes : no));
	tr.append_child(it, StringPair(_("Unloaded"), unloaded() ? yes : no));
	
    os.str("");
    os << _blendMode;
    tr.append_child(it, StringPair(_("Blend mode"), os.str()));
#ifndef NDEBUG
    // This probably isn't interesting for non-developers
    tr.append_child(it, StringPair(_("Invalidated"), _invalidated ? yes : no));
    tr.append_child(it, StringPair(_("Child invalidated"),
                _child_invalidated ? yes : no));
#endif
	return it;
}
#endif

MovieClip*
DisplayObject::getAsRoot()
{
    return get_root();
}

void
setIndexedProperty(size_t index, DisplayObject& o, const as_value& val)
{
    string_table::key prop = getPropertyByIndex(index);
    if (!prop) return;
    doSet(prop, o, val);
}

void
getIndexedProperty(size_t index, DisplayObject& o, as_value& val)
{
    string_table::key prop = getPropertyByIndex(index);
    if (!prop) {
        val.set_undefined();
        return;
    }
    doGet(prop, o, val);
}


/// DisplayObject property lookup 
//
/// This function is only called on the first object in the inheritance chain
/// after the object's own properties have been checked.
/// In AS2, any DisplayObject marks the end of the inheritance chain for
/// lookups.
//
/// Lookup order:
//
/// 1. _level0.._level9
/// 2. Objects on the DisplayList of a MovieClip
/// 3. DisplayObject magic properties (_x, _y etc).
/// 4. MovieClips' TextField variables (this is probably not the best
///    way to do it, but as it is done like this, this must be called here.
///    It will cause an infinite recursion otherwise.
bool
getDisplayObjectProperty(DisplayObject& obj, string_table::key key,
        as_value& val)
{
    
    as_object* o = getObject(&obj);
    assert(o);

    string_table& st = getStringTable(*o);
    const std::string& propname = st.value(key);

    // Check _level0.._level9
    movie_root& mr = getRoot(*getObject(&obj));
    unsigned int levelno;
    if (isLevelTarget(getSWFVersion(*o), propname, levelno)) {
        MovieClip* mo = mr.getLevel(levelno);
        if (mo) {
            val = getObject(mo);
            return true;
        }
        return false;
    }
    
    MovieClip* mc = dynamic_cast<MovieClip*>(&obj);
    if (mc) {
        DisplayObject* ch = mc->getDisplayListObject(key);
        if (ch) {
           val = getObject(ch);
           return true;
        }
    }

    const string_table::key noCaseKey = st.noCase(key);

    // These properties have normal case-sensitivity.
    // They are tested to exist for TextField, MovieClip, and Button
    // but do not belong to the inheritance chain.
    switch (caseless(*o) ? noCaseKey : key)
    {
        default:
            break;
        case NSV::PROP_uROOT:
            if (getSWFVersion(*o) < 5) break;
            val = getObject(obj.getAsRoot());
            return true;
        case NSV::PROP_uGLOBAL:
            // TODO: clean up this mess.
            assert(getObject(&obj));
            if (getSWFVersion(*o) < 6) break;
            val = &getGlobal(*o);
            return true;
    }

    // These magic properties are case insensitive in all versions!
    if (doGet(noCaseKey, obj, val)) return true;

    // Check MovieClip such as TextField variables.
    // TODO: check if there's a better way to find these properties.
    if (mc && mc->getTextFieldVariables(key, val)) return true;

    return false;
}
    

bool
setDisplayObjectProperty(DisplayObject& obj, string_table::key key, 
        const as_value& val)
{
    // These magic properties are case insensitive in all versions!
    string_table& st = getStringTable(*getObject(&obj));
    return doSet(st.noCase(key), obj, val);
}

namespace {

as_value
getQuality(DisplayObject& o)
{
    movie_root& mr = getRoot(*getObject(&o));
    switch (mr.getQuality())
    {
        case QUALITY_BEST:
            return as_value("BEST");
        case QUALITY_HIGH:
            return as_value("HIGH");
        case QUALITY_MEDIUM:
            return as_value("MEDIUM");
        case QUALITY_LOW:
            return as_value("LOW");
    }

    return as_value();

}

void
setQuality(DisplayObject& o, const as_value& val)
{
    movie_root& mr = getRoot(*getObject(&o));

    if (!val.is_string()) return;

    const std::string& q = val.to_string();

    StringNoCaseEqual noCaseCompare;

    if (noCaseCompare(q, "BEST")) mr.setQuality(QUALITY_BEST);
    else if (noCaseCompare(q, "HIGH")) {
        mr.setQuality(QUALITY_HIGH);
    }
    else if (noCaseCompare(q, "MEDIUM")) {
        mr.setQuality(QUALITY_MEDIUM);
    }
    else if (noCaseCompare(q, "LOW")) {
            mr.setQuality(QUALITY_LOW);
    }

    return;
}

as_value
getURL(DisplayObject& o)
{
    return as_value(o.get_root()->url());
}

as_value
getHighQuality(DisplayObject& o)
{
    movie_root& mr = getRoot(*getObject(&o));
    switch (mr.getQuality())
    {
        case QUALITY_BEST:
            return as_value(2.0);
        case QUALITY_HIGH:
            return as_value(1.0);
        case QUALITY_MEDIUM:
        case QUALITY_LOW:
            return as_value(0.0);
    }
    return as_value();
}

void
setHighQuality(DisplayObject& o, const as_value& val)
{
    movie_root& mr = getRoot(*getObject(&o));

    const double q = val.to_number();

    if (q < 0) mr.setQuality(QUALITY_HIGH);
    else if (q > 2) mr.setQuality(QUALITY_BEST);
    else {
        int i = static_cast<int>(q);
        switch(i)
        {
            case 0:
                mr.setQuality(QUALITY_LOW);
                break;
            case 1:
                mr.setQuality(QUALITY_HIGH);
                break;
            case 2:
                mr.setQuality(QUALITY_BEST);
                break;
        }
    }

}

void
setY(DisplayObject& o, const as_value& val)
{

    const double newy = val.to_number();

    // NaN is skipped, Infinite isn't
    if (isNaN(newy))
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Attempt to set %s._y to %s "
            "(evaluating to number %g) refused"),
            o.getTarget(), val, newy);
        );
        return;
    }

    SWFMatrix m = o.getMatrix();
    // NOTE: infinite_to_zero is wrong here, see actionscript.all/setProperty.as
    m.set_y_translation(pixelsToTwips(infinite_to_zero(newy)));
    o.setMatrix(m); 
    o.transformedByScript();
}

as_value
getY(DisplayObject& o)
{
    SWFMatrix m = o.getMatrix();
    return twipsToPixels(m.get_y_translation());
}

void
setX(DisplayObject& o, const as_value& val)
{

    const double newx = val.to_number();

    // NaN is skipped, Infinite isn't
    if (isNaN(newx))
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Attempt to set %s._x to %s "
            "(evaluating to number %g) refused"),
            o.getTarget(), val, newx);
        );
        return;
    }

    SWFMatrix m = o.getMatrix();
    // NOTE: infinite_to_zero is wrong here, see actionscript.all/setProperty.as
    m.set_x_translation(pixelsToTwips(infinite_to_zero(newx)));
    o.setMatrix(m); 
    o.transformedByScript();
}

as_value
getX(DisplayObject& o)
{
    SWFMatrix m = o.getMatrix();
    return twipsToPixels(m.get_x_translation());
}

void
setScaleX(DisplayObject& o, const as_value& val)
{

    const double scale_percent = val.to_number();

    // NaN is skipped, Infinite is not, see actionscript.all/setProperty.as
    if (isNaN(scale_percent))
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Attempt to set %s._xscale to %s "
            "(evaluating to number %g) refused"),
            o.getTarget(), val, scale_percent);
        );
        return;
    }

    // input is in percent
    o.set_x_scale(scale_percent);

}

as_value
getScaleX(DisplayObject& o)
{
    return o.scaleX();
}

void
setScaleY(DisplayObject& o, const as_value& val)
{

    const double scale_percent = val.to_number();

    // NaN is skipped, Infinite is not, see actionscript.all/setProperty.as
    if (isNaN(scale_percent))
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Attempt to set %s._yscale to %s "
            "(evaluating to number %g) refused"),
            o.getTarget(), val, scale_percent);
        );
        return;
    }

    // input is in percent
    o.set_y_scale(scale_percent);

}

as_value
getScaleY(DisplayObject& o)
{
    return o.scaleY();
}

as_value
getVisible(DisplayObject& o)
{
    return o.visible();
}

void
setVisible(DisplayObject& o, const as_value& val)
{

    /// We cast to number and rely (mostly) on C++'s automatic
    /// cast to bool, as string "0" should be converted to
    /// its numeric equivalent, not interpreted as 'true', which
    /// SWF7+ does for strings.
    double d = val.to_number();

    // Infinite or NaN is skipped
    if (isInf(d) || isNaN(d)) {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Attempt to set %s._visible to %s "
            "(evaluating to number %g) refused"),
            o.getTarget(), val, d);
        );
        return;
    }

    o.set_visible(d);

    o.transformedByScript();
}

as_value
getAlpha(DisplayObject& o)
{
    return as_value(o.get_cxform().aa / 2.56);
}

void
setAlpha(DisplayObject& o, const as_value& val)
{

    // The new internal alpha value is input / 100.0 * 256.
    // We test for finiteness later, but the multiplication
    // won't make any difference.
    const double newAlpha = val.to_number() * 2.56;

    // NaN is skipped, Infinite is not, see actionscript.all/setProperty.as
    if (isNaN(newAlpha)) {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Attempt to set %s._alpha to %s "
            "(evaluating to number %g) refused"),
            o.getTarget(), val, newAlpha);
        );
        return;
    }

    cxform cx = o.get_cxform();

    // Overflows are *not* truncated, but set to -32768.
    if (newAlpha > std::numeric_limits<boost::int16_t>::max() ||
        newAlpha < std::numeric_limits<boost::int16_t>::min()) {
        cx.aa = std::numeric_limits<boost::int16_t>::min();
    }
    else {
        cx.aa = static_cast<boost::int16_t>(newAlpha);
    }

    o.set_cxform(cx);
    o.transformedByScript();  

}

as_value
getMouseX(DisplayObject& o)
{
	// Local coord of mouse IN PIXELS.
	boost::int32_t x, y;
	getRoot(*getObject(&o)).get_mouse_state(x, y);

	SWFMatrix m = o.getWorldMatrix();
    point a(pixelsToTwips(x), pixelsToTwips(y));
    
    m.invert().transform(a);
    return as_value(twipsToPixels(a.x));
}

as_value
getMouseY(DisplayObject& o)
{
	// Local coord of mouse IN PIXELS.
	boost::int32_t x, y;
	getRoot(*getObject(&o)).get_mouse_state(x, y);

	SWFMatrix m = o.getWorldMatrix();
    point a(pixelsToTwips(x), pixelsToTwips(y));
    m.invert().transform(a);
    return as_value(twipsToPixels(a.y));
}

as_value
getRotation(DisplayObject& o)
{
    return o.rotation();
}


void
setRotation(DisplayObject& o, const as_value& val)
{

    // input is in degrees
    const double rotation_val = val.to_number();

    // NaN is skipped, Infinity isn't
    if (isNaN(rotation_val))
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Attempt to set %s._rotation to %s "
            "(evaluating to number %g) refused"),
            o.getTarget(), val, rotation_val);
        );
        return;
    }
    o.set_rotation(rotation_val);
}


as_value
getParent(DisplayObject& o)
{
    as_object* p = getObject(o.get_parent());
    return p ? p : as_value();
}

as_value
getTarget(DisplayObject& o)
{
    return o.getTargetPath();
}

as_value
getNameProperty(DisplayObject& o)
{
    string_table& st = getStringTable(*getObject(&o));
    const std::string& name = st.value(o.get_name());
    if (getSWFVersion(*getObject(&o)) < 6 && name.empty()) return as_value(); 
    return as_value(name);
}

void
setName(DisplayObject& o, const as_value& val)
{
    string_table& st = getStringTable(*getObject(&o));
    o.set_name(st.find(val.to_string().c_str()));
}

void
setSoundBufTime(DisplayObject& /*o*/, const as_value& /*val*/)
{
    LOG_ONCE(log_unimpl("_soundbuftime setting"));
}

as_value
getSoundBufTime(DisplayObject& /*o*/)
{
    return as_value(0.0);
}

as_value
getWidth(DisplayObject& o)
{
	SWFRect bounds = o.getBounds();
    const SWFMatrix& m = o.getMatrix();
    m.transform(bounds);
    return twipsToPixels(bounds.width());
}

void
setWidth(DisplayObject& o, const as_value& val)
{
    const double newwidth = pixelsToTwips(val.to_number());
    if (newwidth <= 0) {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Setting _width=%g of DisplayObject %s (%s)"),
            newwidth/20, o.getTarget(), typeName(o));
        );
    }
    o.setWidth(newwidth);
}

as_value
getFocusRect(DisplayObject& /*o*/)
{
    LOG_ONCE(log_unimpl("_focusrect"));
    return as_value(true);
}

void
setFocusRect(DisplayObject& /*o*/, const as_value& /*val*/)
{
    LOG_ONCE(log_unimpl("_focusrect setting"));
}

as_value
getDropTarget(DisplayObject& o)
{
    // This property only applies to MovieClips.
    MovieClip* mc = dynamic_cast<MovieClip*>(&o);
    if (!mc) return as_value();
    return as_value(mc->getDropTarget());
}

as_value
getCurrentFrame(DisplayObject& o)
{
    // This property only applies to MovieClips.
    MovieClip* mc = dynamic_cast<MovieClip*>(&o);
    if (!mc) return as_value();
    const int currframe =
        std::min(mc->get_loaded_frames(), mc->get_current_frame() + 1);
    return as_value(currframe);
}

as_value
getFramesLoaded(DisplayObject& o)
{
    // This property only applies to MovieClips.
    MovieClip* mc = dynamic_cast<MovieClip*>(&o);
    if (!mc) return as_value();
    return as_value(mc->get_frame_count());
}

as_value
getTotalFrames(DisplayObject& o)
{
    // This property only applies to MovieClips.
    MovieClip* mc = dynamic_cast<MovieClip*>(&o);
    if (!mc) return as_value();
    return as_value(mc->get_loaded_frames());
}


string_table::key
getPropertyByIndex(size_t index)
{

    // This is a magic number; defining it here makes sure that the
    // table is really this size.
    const size_t size = 22;

    if (index >= size) return 0;

    static const string_table::key props[size] = {
        NSV::PROP_uX,
        NSV::PROP_uY,
        NSV::PROP_uXSCALE,
        NSV::PROP_uYSCALE,
        NSV::PROP_uCURRENTFRAME,
        NSV::PROP_uTOTALFRAMES,
        NSV::PROP_uALPHA,
        NSV::PROP_uVISIBLE,
        NSV::PROP_uWIDTH,
        NSV::PROP_uHEIGHT,
        NSV::PROP_uROTATION, 
        NSV::PROP_uTARGET, 
        NSV::PROP_uFRAMESLOADED, 
        NSV::PROP_uNAME, 
        NSV::PROP_uDROPTARGET, 
        NSV::PROP_uURL, 
        NSV::PROP_uHIGHQUALITY, 
        NSV::PROP_uFOCUSRECT, 
        NSV::PROP_uSOUNDBUFTIME, 
        NSV::PROP_uQUALITY, 
        NSV::PROP_uXMOUSE, 
        NSV::PROP_uYMOUSE 
    };
    return props[index];
}

bool
doGet(string_table::key prop, DisplayObject& o, as_value& val)
{
    const Getters& getters = displayObjectGetters();
    const Getters::const_iterator it = getters.find(prop);
    if (it == getters.end()) return false;

    val = (*it->second)(o);
    return true;
}


/// Do the actual setProperty
//
/// Return true if the property is a DisplayObject property, regardless of
/// whether it was successfully set or not.
//
/// @param prop     The property to search for. Note that all special
///                 properties are lower-case, so for a caseless check
///                 it is sufficient for prop to be caseless.
bool
doSet(string_table::key prop, DisplayObject& o, const as_value& val)
{
    const Setters& setters = displayObjectSetters();
    const Setters::const_iterator it = setters.find(prop);
    if (it == setters.end()) return false;

    const Setter s = it->second;

    // Read-only.
    if (!s) return true;
    
    if (val.is_undefined() || val.is_null()) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Attempt to set property to %s, refused"),
                o.getTarget(), val);
        );
        return true;
    }

    (*s)(o, val);
    return true;
}

const Getters
displayObjectGetters()
{
    static const Getters getters = boost::assign::map_list_of
        (NSV::PROP_uX, &getX)
        (NSV::PROP_uY, &getY)
        (NSV::PROP_uXSCALE, &getScaleX)
        (NSV::PROP_uYSCALE, &getScaleY)
        (NSV::PROP_uROTATION, &getRotation)
        (NSV::PROP_uHIGHQUALITY, &getHighQuality)
        (NSV::PROP_uQUALITY, &getQuality)
        (NSV::PROP_uALPHA, &getAlpha)
        (NSV::PROP_uWIDTH, &getWidth)
        (NSV::PROP_uURL, &getURL)
        (NSV::PROP_uHEIGHT, &getHeight)
        (NSV::PROP_uNAME, &getNameProperty)
        (NSV::PROP_uVISIBLE, &getVisible)
        (NSV::PROP_uSOUNDBUFTIME, &getSoundBufTime)
        (NSV::PROP_uFOCUSRECT, &getFocusRect)
        (NSV::PROP_uDROPTARGET, &getDropTarget)
        (NSV::PROP_uCURRENTFRAME, &getCurrentFrame)
        (NSV::PROP_uFRAMESLOADED, &getFramesLoaded)
        (NSV::PROP_uTOTALFRAMES, &getTotalFrames)
        (NSV::PROP_uPARENT, &getParent)
        (NSV::PROP_uTARGET, &getTarget)
        (NSV::PROP_uXMOUSE, &getMouseX)
        (NSV::PROP_uYMOUSE, &getMouseY);
    return getters;
}

const Setters
displayObjectSetters()
{
    const Setter n = 0;

    static const Setters setters = boost::assign::map_list_of
        (NSV::PROP_uX, &setX)
        (NSV::PROP_uY, &setY)
        (NSV::PROP_uXSCALE, &setScaleX)
        (NSV::PROP_uYSCALE, &setScaleY)
        (NSV::PROP_uROTATION, &setRotation)
        (NSV::PROP_uHIGHQUALITY, &setHighQuality)
        (NSV::PROP_uQUALITY, &setQuality)
        (NSV::PROP_uALPHA, &setAlpha)
        (NSV::PROP_uWIDTH, &setWidth)
        (NSV::PROP_uHEIGHT, &setHeight)
        (NSV::PROP_uNAME, &setName)
        (NSV::PROP_uVISIBLE, &setVisible)
        (NSV::PROP_uSOUNDBUFTIME, &setSoundBufTime)
        (NSV::PROP_uFOCUSRECT, &setFocusRect)
        (NSV::PROP_uDROPTARGET, n)
        (NSV::PROP_uCURRENTFRAME, n)
        (NSV::PROP_uFRAMESLOADED, n)
        (NSV::PROP_uTOTALFRAMES, n)
        (NSV::PROP_uPARENT, n)
        (NSV::PROP_uURL, n)
        (NSV::PROP_uTARGET, n)
        (NSV::PROP_uXMOUSE, n)
        (NSV::PROP_uYMOUSE, n);
    return setters;
}


const BlendModeMap&
getBlendModeMap()
{
    /// BLENDMODE_UNDEFINED has no matching string in AS. It is included
    /// here for logging purposes.
    static const BlendModeMap bm = boost::assign::map_list_of
        (DisplayObject::BLENDMODE_UNDEFINED, "undefined")
        (DisplayObject::BLENDMODE_NORMAL, "normal")
        (DisplayObject::BLENDMODE_LAYER, "layer")
        (DisplayObject::BLENDMODE_MULTIPLY, "multiply")
        (DisplayObject::BLENDMODE_SCREEN, "screen")
        (DisplayObject::BLENDMODE_LIGHTEN, "lighten")
        (DisplayObject::BLENDMODE_DARKEN, "darken")
        (DisplayObject::BLENDMODE_DIFFERENCE, "difference")
        (DisplayObject::BLENDMODE_ADD, "add")
        (DisplayObject::BLENDMODE_SUBTRACT, "subtract")
        (DisplayObject::BLENDMODE_INVERT, "invert")
        (DisplayObject::BLENDMODE_ALPHA, "alpha")
        (DisplayObject::BLENDMODE_ERASE, "erase")
        (DisplayObject::BLENDMODE_OVERLAY, "overlay")
        (DisplayObject::BLENDMODE_HARDLIGHT, "hardlight");

    return bm;
}

// Match a blend mode to its string.
bool
blendModeMatches(const BlendModeMap::value_type& val, const std::string& mode)
{
    /// The match must be case-sensitive.
    if (mode.empty()) return false;
    return (val.second == mode);
}

}

std::ostream&
operator<<(std::ostream& o, DisplayObject::BlendMode bm)
{
    const BlendModeMap& bmm = getBlendModeMap();
    return (o << bmm.find(bm)->second);
}


} // namespace gnash

// local variables:
// mode: c++
// indent-tabs-mode: t
// end:
