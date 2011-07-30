// DisplayObject.cpp:  ActionScript DisplayObject class, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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


#ifdef HAVE_CONFIG_H
#include "gnashconfig.h" // USE_SWFTREE
#endif

#include "DisplayObject.h"

#include <utility>
#include <boost/assign/list_of.hpp>
#include <boost/bind.hpp>
#include <boost/logic/tribool.hpp>

#include "movie_root.h"
#include "MovieClip.h"
#include "VM.h" 
#include "fn_call.h"
#include "GnashException.h" 
#include "ExecutableCode.h"
#include "namedStrings.h"
#include "GnashEnums.h" 
#include "GnashNumeric.h"
#include "Global_as.h"
#include "Renderer.h"
#include "GnashAlgorithm.h"
#ifdef USE_SWFTREE
# include "tree.hh"
#endif

#undef set_invalidated

namespace gnash {

// Forward declarations.
namespace {
    /// Match blend modes.
    typedef std::map<DisplayObject::BlendMode, std::string> BlendModeMap;
    const BlendModeMap& getBlendModeMap();
    bool blendModeMatches(const BlendModeMap::value_type& val,
            const std::string& mode);
    
    typedef as_value(*Getter)(DisplayObject&);
    typedef void(*Setter)(DisplayObject&, const as_value&);
    typedef std::pair<Getter, Setter> GetterSetter;

    bool doSet(const ObjectURI& uri, DisplayObject& o, const as_value& val);
    bool doGet(const ObjectURI& uri, DisplayObject& o, as_value& val);
    const GetterSetter& getGetterSetterByIndex(size_t index);

    // NOTE: comparison will be case-insensitive
    const GetterSetter& getGetterSetterByURI(const ObjectURI& uri,
            string_table& st);

    // Convenience function to create a const URI-to-function map
    template<typename Map> const Map getURIMap(
            const typename Map::key_compare& cmp);
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
    GcResource(mr.gc()),
    _name(),
    _parent(parent),
    _object(object),
    _stage(mr),
    _xscale(100),
    _yscale(100),
    _rotation(0),
    _depth(0),
    _focusRect(parent ? boost::tribool(boost::indeterminate) :
                        boost::tribool(true)),
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

ObjectURI
DisplayObject::getNextUnnamedInstanceName()
{
    assert(_object);
    movie_root& mr = stage();

    std::ostringstream ss;
    ss << "instance" << mr.nextUnnamedInstance();

    VM& vm = mr.getVM();
    return getURI(vm, ss.str(), true);
}


int
DisplayObject::getWorldVolume() const
{
    int volume = _volume;
    if (_parent) {
        volume = int(volume*_parent->getVolume()/100.0);
    }

    return volume;
}


as_object*
DisplayObject::pathElement(const ObjectURI& uri)
{
    as_object* obj = getObject(this);
    if (!obj) return 0;

    string_table::key key = getName(uri);

    string_table& st = stage().getVM().getStringTable();

    // TODO: put ".." and "." in namedStrings
    if (key == st.find("..")) return getObject(parent());
    if (key == st.find(".")) return obj;
    
    // The check is case-insensitive for SWF6 and below.
    // TODO: cache ObjectURI(NSV::PROP_THIS) [as many others...]
    if (ObjectURI::CaseEquals(st, caseless(*obj))
            (uri, ObjectURI(NSV::PROP_THIS))) {
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
            (void*)this, getTarget(), debug_file, debug_line);
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
        bounds.expand_to_transformed_rect(getWorldMatrix(*this), getBounds());
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
        double mode = toNumber(bm, getVM(fn));

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
        movie_root& mr = stage();
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

    SWFMatrix m = getMatrix(*this);
    const double yscale = m.get_y_scale(); 
    m.set_scale_rotation(xscale, yscale, rotation);
    setMatrix(m, true); 
}

as_value
getHeight(DisplayObject& o)
{
    SWFRect bounds = o.getBounds();
    const SWFMatrix m = getMatrix(o);
    m.transform(bounds);
    return twipsToPixels(bounds.height());      
}

void
setHeight(DisplayObject& o, const as_value& val)
{
    const double newheight = pixelsToTwips(toNumber(val, getVM(*getObject(&o))));
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

    SWFMatrix m = getMatrix(*this);
    const double xscale = m.get_x_scale();
    m.set_scale_rotation(xscale, yscale, rotation);
    setMatrix(m, true);
}

void
DisplayObject::setMatrix(const SWFMatrix& m, bool updateCache)
{

    if (m == _transform.matrix) return;

    set_invalidated(__FILE__, __LINE__);
    _transform.matrix = m;

    // don't update caches if SWFMatrix wasn't updated too
    if (updateCache) {
        _xscale = _transform.matrix.get_x_scale() * 100.0;
        _yscale = _transform.matrix.get_y_scale() * 100.0;
        _rotation = _transform.matrix.get_rotation() * 180.0 / PI;
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
}

std::auto_ptr<ExecutableCode>
DisplayObject::get_event_handler(const event_id& id) const
{
    std::auto_ptr<ExecutableCode> handler;

    Events::const_iterator it = _event_handlers.find(id);
    if ( it == _event_handlers.end() ) return handler;

    DisplayObject* this_ptr = const_cast<DisplayObject*>(this);

    handler.reset( new EventCode(this_ptr, it->second) );
    return handler;
}

bool
DisplayObject::unload()
{
    const bool childHandler = unloadChildren();

    // Unregister this DisplayObject as mask and/or maskee.
    if (_maskee) _maskee->setMask(0);
    if (_mask) _mask->setMaskee(0);

    const bool hasEvent =
        hasEventHandler(event_id(event_id::UNLOAD)) || childHandler;

    if (!hasEvent) {
        stage().removeQueuedConstructor(this);
    }

    _unloaded = true;

    return hasEvent;
}

bool
DisplayObject::hasEventHandler(const event_id& id) const
{
    Events::const_iterator it = _event_handlers.find(id);
    if (it != _event_handlers.end()) return true;

    if (!_object) return false;

    // Don't check resolve! Also don't check if it's a function, as
    // the swfdec testsuite (onUnload-prototype.as) shows that it
    // doesn't matter.
    if (Property* prop = _object->findProperty(id.functionURI())) {
        return prop; 
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

    SWFMatrix m = getMatrix(*this);

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

    if (_xscale < 0) rotation += PI; 

    SWFMatrix m = getMatrix(*this);
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

    SWFMatrix m = getMatrix(*this);
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
        const DisplayObject* parent = ch->parent();

        // Don't push the _root name on the stack
        if (!parent) {
            topLevel = ch;
            break;
        }

        path.push_back(ch->get_name().toString(st));
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
    for (;;) {

        const DisplayObject* parent = ch->parent();

        // Don't push the _root name on the stack
        if (!parent) {

            std::stringstream ss;
            if (!dynamic_cast<const Movie*>(ch)) {
                // must be an as-referenceable
                // DisplayObject created using 'new'
                // like, new MovieClip, new Video, new TextField...
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

        path.push_back(ch->get_name().toString(st));
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
///    CANNOT CONFIRM THE ABOVE, SEE ButtonEventsTest.swf in misc-ming.all
/// 3. The default value (if the property is not defined) is true.
bool
DisplayObject::allowHandCursor() const
{
    as_object* obj = getObject(this);
    if (!obj) return false;

    // Checking for RELEASE breaks ButtonEventsTest.
    // I guess such an event would influence wheter or not this
    // character would become an active one, despite hand cursor
    //if (!hasEventHandler(event_id::RELEASE)) return false;

    as_value val;
    if (!obj->get_member(NSV::PROP_USEHANDCURSOR, &val)) {
         return true;
    }
    return toBool(val, getVM(*obj));
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
    getWorldMatrix(*this).transform(mybounds);
  
    return renderer.bounds_in_clipping_area(mybounds.getRange());  
}

#ifdef USE_SWFTREE
DisplayObject::InfoTree::iterator
DisplayObject::getMovieInfo(InfoTree& tr, InfoTree::iterator it)
{
    const std::string yes = _("yes");
    const std::string no = _("no");

    it = tr.append_child(it, std::make_pair(getTarget(), typeName(*this)));

    std::ostringstream os;
    os << get_depth();
    tr.append_child(it, std::make_pair(_("Depth"), os.str()));

    /// Don't add if the DisplayObject has no ratio value
    if (get_ratio() > 0) {
        os.str("");
        os << get_ratio();
        tr.append_child(it, std::make_pair(_("Ratio"), os.str()));
    }        

    /// Don't add if it's not a real clipping depth
    const int cd = get_clip_depth();
    if (cd != noClipDepthValue) {
        os.str("");
        if (_maskee) os << "Dynamic mask";
        else os << cd;

        tr.append_child(it, std::make_pair(_("Clipping depth"), os.str()));        
    }

    os.str("");
    os << getBounds().width() << "x" << getBounds().height();
    tr.append_child(it, std::make_pair(_("Dimensions"), os.str()));    

    tr.append_child(it, std::make_pair(_("Dynamic"), isDynamic() ? yes : no));    
    tr.append_child(it, std::make_pair(_("Mask"), isMaskLayer() ? yes : no));        
    tr.append_child(it, std::make_pair(_("Destroyed"),
                isDestroyed() ? yes : no));
    tr.append_child(it, std::make_pair(_("Unloaded"), unloaded() ? yes : no));
    
    os.str("");
    os << _blendMode;
    tr.append_child(it, std::make_pair(_("Blend mode"), os.str()));
#ifndef NDEBUG
    // This probably isn't interesting for non-developers
    tr.append_child(it, std::make_pair(_("Invalidated"),
                _invalidated ? yes : no));
    tr.append_child(it, std::make_pair(_("Child invalidated"),
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
    const Setter s = getGetterSetterByIndex(index).second;
    if (!s) return; // read-only (warn?)

    if (val.is_undefined() || val.is_null()) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Attempt to set property to %s, refused"),
                o.getTarget(), val);
        );
        return;
    }

    (*s)(o, val);
}

void
getIndexedProperty(size_t index, DisplayObject& o, as_value& val)
{
    const Getter s = getGetterSetterByIndex(index).first;
    if (!s) {
        val.set_undefined();
        return;
    }
    val = (*s)(o);
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
getDisplayObjectProperty(DisplayObject& obj, const ObjectURI& uri,
        as_value& val)
{
    
    as_object* o = getObject(&obj);
    assert(o);

    string_table& st = getStringTable(*o);
    const std::string& propname = uri.toString(st);

    // Check _level0.._level9
    unsigned int levelno;
    if (isLevelTarget(getSWFVersion(*o), propname, levelno)) {
        movie_root& mr = getRoot(*getObject(&obj));
        MovieClip* mo = mr.getLevel(levelno);
        if (mo) {
            val = getObject(mo);
            return true;
        }
        return false;
    }
    
    MovieClip* mc = obj.to_movie();
    if (mc) {
        DisplayObject* ch = mc->getDisplayListObject(uri);
        if (ch) {
           val = getObject(ch);
           return true;
        }
    }

    const string_table::key noCaseKey = uri.noCase(st);

    // These properties have normal case-sensitivity.
    // They are tested to exist for TextField, MovieClip, and Button
    // but do not belong to the inheritance chain.
    switch (caseless(*o) ? noCaseKey : getName(uri))
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
    if (doGet(uri, obj, val)) return true;

    // Check MovieClip such as TextField variables.
    // TODO: check if there's a better way to find these properties.
    if (mc && mc->getTextFieldVariables(uri, val)) return true;

    return false;
}
    

bool
setDisplayObjectProperty(DisplayObject& obj, const ObjectURI& uri,
        const as_value& val)
{
    // These magic properties are case insensitive in all versions!
    return doSet(uri, obj, val);
}
    
DisplayObject::MaskRenderer::MaskRenderer(Renderer& r, const DisplayObject& o)
    :
    _renderer(r),
    _mask(o.visible() && o.getMask() && !o.getMask()->unloaded() ? o.getMask()
                                                                 : 0)
{
    if (!_mask) return;

    _renderer.begin_submit_mask();
    DisplayObject* p = _mask->parent();
    const Transform tr = p ?
        Transform(getWorldMatrix(*p), getWorldCxForm(*p)) : Transform(); 
    _mask->display(_renderer, tr);
    _renderer.end_submit_mask();
}

DisplayObject::MaskRenderer::~MaskRenderer()
{
    if (_mask) _renderer.disable_mask();
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

    const double q = toNumber(val, getVM(*getObject(&o)));

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

    const double newy = toNumber(val, getVM(*getObject(&o)));

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

    SWFMatrix m = getMatrix(o);
    // NOTE: infinite_to_zero is wrong here, see actionscript.all/setProperty.as
    m.set_y_translation(pixelsToTwips(infinite_to_zero(newy)));
    o.setMatrix(m); 
    o.transformedByScript();
}

as_value
getY(DisplayObject& o)
{
    const SWFMatrix m = getMatrix(o);
    return twipsToPixels(m.get_y_translation());
}

void
setX(DisplayObject& o, const as_value& val)
{

    const double newx = toNumber(val, getVM(*getObject(&o)));

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

    SWFMatrix m = getMatrix(o);
    // NOTE: infinite_to_zero is wrong here, see actionscript.all/setProperty.as
    m.set_x_translation(pixelsToTwips(infinite_to_zero(newx)));
    o.setMatrix(m); 
    o.transformedByScript();
}

as_value
getX(DisplayObject& o)
{
    const SWFMatrix m = getMatrix(o);
    return twipsToPixels(m.get_x_translation());
}

void
setScaleX(DisplayObject& o, const as_value& val)
{

    const double scale_percent = toNumber(val, getVM(*getObject(&o)));

    // NaN is skipped, Infinite is not, see actionscript.all/setProperty.as
    if (isNaN(scale_percent)) {
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

    const double scale_percent = toNumber(val, getVM(*getObject(&o)));

    // NaN is skipped, Infinite is not, see actionscript.all/setProperty.as
    if (isNaN(scale_percent)) {
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
    const double d = toNumber(val, getVM(*getObject(&o)));

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
    return as_value(getCxForm(o).aa / 2.56);
}

void
setAlpha(DisplayObject& o, const as_value& val)
{

    // The new internal alpha value is input / 100.0 * 256.
    // We test for finiteness later, but the multiplication
    // won't make any difference.
    const double newAlpha = toNumber(val, getVM(*getObject(&o))) * 2.56;

    // NaN is skipped, Infinite is not, see actionscript.all/setProperty.as
    if (isNaN(newAlpha)) {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Attempt to set %s._alpha to %s "
            "(evaluating to number %g) refused"),
            o.getTarget(), val, newAlpha);
        );
        return;
    }

    SWFCxForm cx = getCxForm(o);

    // Overflows are *not* truncated, but set to -32768.
    if (newAlpha > std::numeric_limits<boost::int16_t>::max() ||
        newAlpha < std::numeric_limits<boost::int16_t>::min()) {
        cx.aa = std::numeric_limits<boost::int16_t>::min();
    }
    else {
        cx.aa = static_cast<boost::int16_t>(newAlpha);
    }

    o.setCxForm(cx);
    o.transformedByScript();  

}

as_value
getMouseX(DisplayObject& o)
{
    // Local coord of mouse IN PIXELS.
    boost::int32_t x, y;
    boost::tie(x, y) = getRoot(*getObject(&o)).mousePosition();

    SWFMatrix m = getWorldMatrix(o);
    point a(pixelsToTwips(x), pixelsToTwips(y));
    
    m.invert().transform(a);
    return as_value(twipsToPixels(a.x));
}

as_value
getMouseY(DisplayObject& o)
{
    // Local coord of mouse IN PIXELS.
    boost::int32_t x, y;
    boost::tie(x, y) = getRoot(*getObject(&o)).mousePosition();

    SWFMatrix m = getWorldMatrix(o);
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
    const double rotation_val = toNumber(val, getVM(*getObject(&o)));

    // NaN is skipped, Infinity isn't
    if (isNaN(rotation_val)) {
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
    as_object* p = getObject(o.parent());
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
    const std::string& name = o.get_name().toString(st);
    if (getSWFVersion(*getObject(&o)) < 6 && name.empty()) return as_value(); 
    return as_value(name);
}

void
setName(DisplayObject& o, const as_value& val)
{
    o.set_name(getURI(getVM(*getObject(&o)), val.to_string()));
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
    const SWFMatrix& m = getMatrix(o);
    m.transform(bounds);
    return twipsToPixels(bounds.width());
}

void
setWidth(DisplayObject& o, const as_value& val)
{
    const double newwidth = pixelsToTwips(toNumber(val, getVM(*getObject(&o))));
    if (newwidth <= 0) {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Setting _width=%g of DisplayObject %s (%s)"),
            newwidth/20, o.getTarget(), typeName(o));
        );
    }
    o.setWidth(newwidth);
}

as_value
getFocusRect(DisplayObject& o)
{
    LOG_ONCE(log_unimpl("_focusrect"));

    const boost::tribool fr = o.focusRect();
    if (boost::indeterminate(fr)) {
        as_value null;
        null.set_null();
        return as_value(null);
    }
    const bool ret = static_cast<bool>(fr);
    if (getSWFVersion(*getObject(&o)) == 5) {
        return as_value(static_cast<double>(ret));
    }
    return as_value(ret);
}

void
setFocusRect(DisplayObject& o, const as_value& val)
{
    LOG_ONCE(log_unimpl("_focusrect"));

    VM& vm = getVM(*getObject(&o));
    if (!o.parent()) {
        const double d = toNumber(val, vm);
        if (isNaN(d)) return;
        o.focusRect(d);
        return;
    }
    o.focusRect(toBool(val, vm));
}

as_value
getDropTarget(DisplayObject& o)
{
    // This property only applies to MovieClips.
    MovieClip* mc = o.to_movie();
    if (!mc) return as_value();
    return as_value(mc->getDropTarget());
}

as_value
getCurrentFrame(DisplayObject& o)
{
    // This property only applies to MovieClips.
    MovieClip* mc = o.to_movie();
    if (!mc) return as_value();
    const int currframe =
        std::min(mc->get_loaded_frames(), mc->get_current_frame() + 1);
    return as_value(currframe);
}

as_value
getFramesLoaded(DisplayObject& o)
{
    // This property only applies to MovieClips.
    MovieClip* mc = o.to_movie();
    if (!mc) return as_value();
    return as_value(mc->get_loaded_frames());
}

as_value
getTotalFrames(DisplayObject& o)
{
    // This property only applies to MovieClips.
    MovieClip* mc = o.to_movie();
    if (!mc) return as_value();
    return as_value(mc->get_frame_count());
}


/// @param uri     The property to search for. Note that all special
///                properties are lower-case.
///
/// NOTE that all properties have getters so you can recognize a 
/// 'not-found' condition by checking .first = 0
const GetterSetter&
getGetterSetterByURI(const ObjectURI& uri, string_table& st)
{
    typedef std::map<ObjectURI, GetterSetter, ObjectURI::CaseLessThan> 
        GetterSetters;

    static const GetterSetters gs =
        getURIMap<GetterSetters>(ObjectURI::CaseLessThan(st, true));

    const GetterSetters::const_iterator it = gs.find(uri);

    if (it == gs.end()) {
        static const GetterSetter none(0, 0);
        return none;
    }

    return it->second;
}


const GetterSetter&
getGetterSetterByIndex(size_t index)
{
    const Setter n = 0;

    static const GetterSetter props[] = {
        GetterSetter(&getX, &setX),
        GetterSetter(&getY, &setY),
        GetterSetter(&getScaleX, &setScaleX),
        GetterSetter(&getScaleY, &setScaleY),

        GetterSetter(&getCurrentFrame, n),
        GetterSetter(&getTotalFrames, n),
        GetterSetter(&getAlpha, &setAlpha),
        GetterSetter(&getVisible, &setVisible),

        GetterSetter(&getWidth, &setWidth),
        GetterSetter(&getHeight, &setHeight),
        GetterSetter(&getRotation, &setRotation),
        GetterSetter(&getTarget, n),

        GetterSetter(&getFramesLoaded, n),
        GetterSetter(&getNameProperty, &setName),
        GetterSetter(&getDropTarget, n),
        GetterSetter(&getURL, n),

        GetterSetter(&getHighQuality, &setHighQuality),
        GetterSetter(&getFocusRect, &setFocusRect),
        GetterSetter(&getSoundBufTime, &setSoundBufTime),
        GetterSetter(&getQuality, &setQuality),

        GetterSetter(&getMouseX, n),
        GetterSetter(&getMouseY, n)

    };

    if (index >= arraySize(props)) {
        const Getter ng = 0;
        static const GetterSetter none(ng, n);
        return none;
    }

    return props[index];
}


bool
doGet(const ObjectURI& uri, DisplayObject& o, as_value& val)
{
    string_table& st = getStringTable(*getObject(&o));
    const Getter s = getGetterSetterByURI(uri, st).first;
    if (!s) return false;

    val = (*s)(o);
    return true;
}


/// Do the actual setProperty
//
/// Return true if the property is a DisplayObject property, regardless of
/// whether it was successfully set or not.
//
/// @param uri     The property to search for. Note that all special
///                properties are lower-case, so for a caseless check
///                it is sufficient for prop to be caseless.
bool
doSet(const ObjectURI& uri, DisplayObject& o, const as_value& val)
{
    string_table& st = getStringTable(*getObject(&o));

    const GetterSetter gs = getGetterSetterByURI(uri, st);
     
    // not found (all props have getters)
    if (!gs.first) return false;

    const Setter s = gs.second;

    // read-only (TODO: aserror ?)
    if (!s) return true;

    if (val.is_undefined() || val.is_null()) {
        IF_VERBOSE_ASCODING_ERRORS(
            // TODO: add property  name to this log...
            log_aserror(_("Attempt to set property to %s, refused"),
                o.getTarget(), val);
        );
        return true;
    }

    (*s)(o, val);
    return true;
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

/// Return a const map of property URI to function.
//
/// This function takes advantage of NRVO to allow the map to
/// be constructed in the caller.
template<typename Map>
const Map
getURIMap(const typename Map::key_compare& cmp)
{
    const Setter n = 0;

    Map ret(cmp);
    ret.insert(std::make_pair(NSV::PROP_uX, GetterSetter(&getX, &setX)));
    ret.insert(std::make_pair(NSV::PROP_uY, GetterSetter(&getY, &setY)));
    ret.insert(std::make_pair(NSV::PROP_uXSCALE,
                GetterSetter(&getScaleX, &setScaleX)));
    ret.insert(std::make_pair(NSV::PROP_uYSCALE,
                GetterSetter(&getScaleY, &setScaleY)));
    ret.insert(std::make_pair(NSV::PROP_uROTATION,
                GetterSetter(&getRotation, &setRotation)));
    ret.insert(std::make_pair(NSV::PROP_uHIGHQUALITY,
                GetterSetter(&getHighQuality, &setHighQuality)));
    ret.insert(std::make_pair(NSV::PROP_uQUALITY,
                GetterSetter(&getQuality, &setQuality)));
    ret.insert(std::make_pair(NSV::PROP_uALPHA,
                GetterSetter(&getAlpha, &setAlpha)));
    ret.insert(std::make_pair(NSV::PROP_uWIDTH,
                GetterSetter(&getWidth, &setWidth)));
    ret.insert(std::make_pair(NSV::PROP_uHEIGHT,
                GetterSetter(&getHeight, &setHeight)));
    ret.insert(std::make_pair(NSV::PROP_uNAME,
                GetterSetter(&getNameProperty, &setName)));
    ret.insert(std::make_pair(NSV::PROP_uVISIBLE,
                GetterSetter(&getVisible, &setVisible)));
    ret.insert(std::make_pair(NSV::PROP_uSOUNDBUFTIME,
                GetterSetter(&getSoundBufTime, &setSoundBufTime)));
    ret.insert(std::make_pair(NSV::PROP_uFOCUSRECT,
                GetterSetter(&getFocusRect, &setFocusRect)));
    ret.insert(std::make_pair(NSV::PROP_uDROPTARGET,
                GetterSetter(&getDropTarget, n)));
    ret.insert(std::make_pair(NSV::PROP_uCURRENTFRAME,
                GetterSetter(&getCurrentFrame, n)));
    ret.insert(std::make_pair(NSV::PROP_uFRAMESLOADED,
                GetterSetter(&getFramesLoaded, n)));
    ret.insert(std::make_pair(NSV::PROP_uTOTALFRAMES,
                GetterSetter(&getTotalFrames, n)));
    ret.insert(std::make_pair(NSV::PROP_uURL, GetterSetter(&getURL, n)));
    ret.insert(std::make_pair(NSV::PROP_uTARGET, GetterSetter(&getTarget, n)));
    ret.insert(std::make_pair(NSV::PROP_uXMOUSE, GetterSetter(&getMouseX, n)));
    ret.insert(std::make_pair(NSV::PROP_uYMOUSE, GetterSetter(&getMouseY, n)));
    ret.insert(std::make_pair(NSV::PROP_uPARENT, GetterSetter(&getParent, n)));
    return ret;
}

} // anonymous namespace

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
