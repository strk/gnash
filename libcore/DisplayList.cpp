// dlist.cpp:  Display lists, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#include "smart_ptr.h" // GNASH_USE_GC
#include "DisplayList.h"
#include "log.h"
#include "render.h"
#include "StringPredicates.h"
#include "MovieClip.h"

#include <typeinfo>
#include <iostream>
#include <algorithm>
#include <stack>
#include <boost/bind.hpp>

namespace gnash {

class DepthEquals {
public:
  int _depth;

  DepthEquals(int depth)
    :
    _depth(depth)
  {}

  bool operator() (const DisplayItem& item) {
    if ( ! item.get() ) return false;
    return item->get_depth() == _depth;
  }
};

class DepthGreaterOrEqual {
public:
  int _depth;

  DepthGreaterOrEqual(int depth)
    :
    _depth(depth)
  {}

  bool operator() (const DisplayItem& item) {
    if ( ! item.get() ) return false;
    return item->get_depth() >= _depth;
  }
};

class NameEquals {
public:
  const std::string& _name;

  NameEquals(const std::string& name)
    :
    _name(name)
  {}

  bool operator() (const DisplayItem& item) {
    if ( ! item.get() ) return false;
    return item->get_name() == _name;
  }
};

class NameEqualsNoCase {

  StringNoCaseEqual noCaseEquals;

public:
  const std::string& _name;

  NameEqualsNoCase(const std::string& name)
    :
    _name(name)
  {}

  bool operator() (const DisplayItem& item)
  {
    if ( ! item.get() ) return false;
    return noCaseEquals(item->get_name(), _name);
  }
};

int
DisplayList::getNextHighestDepth() const
{
  testInvariant();

  int nexthighestdepth=0;
  for (const_iterator it = _charsByDepth.begin(),
      itEnd = _charsByDepth.end();
    it != itEnd; ++it)
  {
    character* ch = it->get();

    int chdepth = ch->get_depth();
    if ( chdepth >= nexthighestdepth )
    {
      nexthighestdepth = chdepth+1;
    }
  }
  return nexthighestdepth;
}

character*
DisplayList::get_character_at_depth(int depth)
{
  testInvariant();

  //GNASH_REPORT_FUNCTION;
  //dump();

  for (iterator it = _charsByDepth.begin(),
      itEnd = _charsByDepth.end();
    it != itEnd; ++it)
  {
    character* ch = it->get();

    // found
    if ( ch->get_depth() == depth ) return ch;

    // non-existent (chars are ordered by depth)
    if ( ch->get_depth() > depth ) return NULL;
  }

  return NULL;

}


character*
DisplayList::get_character_by_name(const std::string& name)
{
  testInvariant();

  const container_type::iterator e = _charsByDepth.end();

  container_type::const_iterator it = std::find_if(
      _charsByDepth.begin(), e,
      NameEquals(name));

  if ( it == e ) return NULL;
  
  return it->get();

}

character*
DisplayList::get_character_by_name_i(const std::string& name)
{
  testInvariant();

  const container_type::iterator e = _charsByDepth.end();

  container_type::const_iterator it = std::find_if(
      _charsByDepth.begin(), e,
      NameEqualsNoCase(name));

  if ( it == e ) return NULL;
  
  return it->get();
}

void
DisplayList::place_character(character* ch, int depth, as_object* initObj)
{
  assert(!ch->isUnloaded());
  ch->set_invalidated();
  ch->set_depth(depth);

  container_type::iterator it = std::find_if(
      _charsByDepth.begin(), _charsByDepth.end(),
      DepthGreaterOrEqual(depth));

  if ( it == _charsByDepth.end() || (*it)->get_depth() != depth )
  {
    // add the new char
    _charsByDepth.insert(it, DisplayItem(ch));
  }
  else
  {
    // remember bounds of old char
    InvalidatedRanges old_ranges; 
    (*it)->add_invalidated_bounds(old_ranges, true);  

    // make a copy (before replacing)
    boost::intrusive_ptr<character> oldCh = *it;

    // replace existing char (before calling unload!)
    *it = DisplayItem(ch);
  
    if ( oldCh->unload() )
    {
      // reinsert removed character if needed
      reinsertRemovedCharacter(oldCh);
    }
    else
    {
      oldCh->destroy();
    }
    
    // extend invalidated bounds
    ch->extend_invalidated_bounds(old_ranges);        
  }

  // Give life to this instance
  ch->stagePlacementCallback(initObj);

  testInvariant();
}

void
DisplayList::add(character* ch, bool replace)
{
  int depth = ch->get_depth();

  container_type::iterator it = std::find_if(
      _charsByDepth.begin(), _charsByDepth.end(),
      DepthGreaterOrEqual(depth));
  if ( it == _charsByDepth.end() || (*it)->get_depth() != depth )
  {
    _charsByDepth.insert(it, DisplayItem(ch));
  }
  else if ( replace )
  {
    *it = DisplayItem(ch);
  }

  testInvariant();
}

void
DisplayList::addAll(std::vector<character*>& chars, bool replace)
{
  testInvariant();

  for (std::vector<character*>::iterator it=chars.begin(),
      itEnd=chars.end();
      it != itEnd; ++it)
  {
    add(*it, replace);
  }

  testInvariant();
}

void
DisplayList::replace_character(
	character* ch, 
	int depth, 
	bool use_old_cxform,
	bool use_old_matrix)
{
  testInvariant();

  //GNASH_REPORT_FUNCTION;
  assert(!ch->isUnloaded());

  ch->set_invalidated();
  ch->set_depth(depth);

  // NOTE: currently, ::restart also cleans up all property, which include __proto__ !!
  //       For this reason I commented it out. Since no tests in the testsuite are failing
  //       I'm not sure what does this break. Udo: do you remember ? --strk;
  // ch->restart();

  container_type::iterator it = std::find_if(
      _charsByDepth.begin(), _charsByDepth.end(),
      DepthGreaterOrEqual(depth));

  DisplayItem di(ch);

  if ( it == _charsByDepth.end() || (*it)->get_depth() != depth )
  {
    _charsByDepth.insert(it, di);
  }
  else
  {
    // Make a copy (before replacing)
    boost::intrusive_ptr<character> oldch = *it;

    InvalidatedRanges old_ranges;
  
    if (use_old_cxform)
    {
      // Use the cxform from the old character.
      ch->set_cxform(oldch->get_cxform());
    }

    if (use_old_matrix)
    {
      // Use the SWFMatrix from the old character.
      ch->copyMatrix(*oldch); // copy SWFMatrix and caches
    }
    
    // remember bounds of old char
    oldch->add_invalidated_bounds(old_ranges, true);    

    // replace existing char (before calling unload)
    *it = di;

    // Unload old char
    if ( oldch->unload() )
    {
      // reinsert removed character if needed
      reinsertRemovedCharacter(oldch);
    }
    else
    {
      oldch->destroy();
    }
    
    // extend invalidated bounds
    // WARNING: when a new Button character is added,
    //          the invalidated bounds computation will likely
    //          be bogus, as the actual character shown is not instantiated
    //          until ::stagePlacementCallback for buttons (I'd say this is a bug in Button)
    //          UdoG, following ? 
    //
    ch->extend_invalidated_bounds(old_ranges);        

  }

  // Give life to this instance
  ch->stagePlacementCallback();

  testInvariant();
}
  
  
// Updates the transform properties of the character at
// the specified depth.
void
DisplayList::move_character(
  int depth,
  const cxform* color_xform,
  const SWFMatrix* mat,
  int* ratio,
  int* /* clip_depth */)
{
  testInvariant();

  //GNASH_REPORT_FUNCTION;
  //IF_VERBOSE_DEBUG(log_debug(_("dl::move(%d)"), depth));

  character* ch = get_character_at_depth(depth);
  if ( ! ch )
  {
    // FIXME, should this be log_aserror?
    IF_VERBOSE_MALFORMED_SWF(
    log_swferror(_("move_character() -- "
      "can't find object at depth %d"),
      depth);
    );
    return;
  }

  if ( ch->isUnloaded() )
  {
    log_error("Request to move an unloaded character");
    assert(!ch->isUnloaded());
  }

  // TODO: is sign of depth related to accepting anim moves ?
  if (ch->get_accept_anim_moves() == false)
  {
    // This character is rejecting anim moves.  This happens after it
    // has been manipulated by ActionScript.
    return;
  }

  if (color_xform)
  {
    ch->set_cxform(*color_xform);
  }
  if (mat)
  {
    ch->setMatrix(*mat, true); // update SWFMatrix caches
  }
  if(ratio)
  {
    ch->set_ratio(*ratio);
  }

  testInvariant();
}
  
  
// Removes the object at the specified depth.
void
DisplayList::remove_character(int depth)
{
  //GNASH_REPORT_FUNCTION;

  testInvariant();

  //log_debug(_("Before removing, list is:"));
  //dump();

#ifndef NDEBUG
  container_type::size_type size = _charsByDepth.size();
#endif

  // TODO: would it be legal to call remove_character with a depth
  //       in the "removed" zone ?
  // TODO: optimize to take by-depth order into account
  container_type::iterator it = std::find_if(
      _charsByDepth.begin(),
      _charsByDepth.end(),
      DepthEquals(depth));

  if ( it != _charsByDepth.end() )
  {
    // Make a copy (before erasing)
    boost::intrusive_ptr<character> oldCh = *it;

    // Erase (before callign unload)
    _charsByDepth.erase(it);

    if ( oldCh->unload() )
    {
      // reinsert removed character if needed
      // NOTE: could be optimized if we knew exactly how
      //       to handle the case in which the target depth
      //       (after the shift) is occupied already
      //
      reinsertRemovedCharacter(oldCh);
    }
    else
    {
      oldCh->destroy();
    }
  }

  assert(size >= _charsByDepth.size());

  testInvariant();

  //log_debug(_("Done removing, list is:"));
  //dump();
}

// TODO: take character by ref ?
void
DisplayList::swapDepths(character* ch1, int newdepth)
{
  testInvariant();

  if ( newdepth < character::staticDepthOffset )
  {
    IF_VERBOSE_ASCODING_ERRORS(
    log_aserror("%s.swapDepth(%d) : ignored call with target depth less then %d",
      ch1->getTarget(), newdepth, character::staticDepthOffset);
    );
    return;
  }

  int srcdepth = ch1->get_depth();

  // what if source char is at a lower depth ?
  assert(srcdepth >= character::staticDepthOffset);

  assert(srcdepth != newdepth);

  // TODO: optimize this scan by taking ch1 depth into account ?
  container_type::iterator it1 = std::find(_charsByDepth.begin(), _charsByDepth.end(), ch1);

  // upper bound ...
  container_type::iterator it2 = std::find_if(_charsByDepth.begin(), _charsByDepth.end(),
      DepthGreaterOrEqual(newdepth));

  if ( it1 == _charsByDepth.end() )
  {
    log_error("First argument to DisplayList::swapDepth() is NOT a character in the list. Call ignored.");
    return;
  }

  // Found another character at the given depth
  if ( it2 != _charsByDepth.end() && (*it2)->get_depth() == newdepth )
  {
    DisplayItem ch2 = *it2;

    ch2->set_depth(srcdepth);

    // TODO: we're not actually invalidated ourselves, rather our parent is...
    //       UdoG ? Want to verify this ?
    ch2->set_invalidated();

    // We won't accept static transforms after a depth swap.
    // See displaylist_depths_test6.swf for more info.
    ch2->transformedByScript();

    iter_swap(it1, it2);
  }

  // No character found at the given depth
  else
  {
    // Move the character to the new position
    // NOTE: insert *before* erasing, in case the list is
    //       the only referer of the ref-counted character
    _charsByDepth.insert(it2, ch1);
    _charsByDepth.erase(it1);
  }

  // don't change depth before the iter_swap case above, as
  // we'll need it to assign to the new character
  ch1->set_depth(newdepth);

  // TODO: we're not actually invalidated ourselves, rather our parent is...
  //       UdoG ? Want to verify this ?
  ch1->set_invalidated();

  // We won't accept static transforms after a depth swap.
  // See displaylist_depths_test6.swf for more info.
  ch1->transformedByScript();

  testInvariant();

}
  
bool
DisplayList::unload()
{
  //GNASH_REPORT_FUNCTION;

  testInvariant();

  // Should we start looking from beginNonRemoved ?
  // If I try, I get a failure in swfdec/gotoframe.swf
  for (iterator it = _charsByDepth.begin(), itEnd = _charsByDepth.end(); it != itEnd; )
  {
    // make a copy
    DisplayItem di = *it;

    // skip if already unloaded
    if ( di->isUnloaded() )
    {
      // TODO: call di->destroy(); ?
      ++it;
      continue;
    }

    if ( ! di->unload() ) // no event handler queued, we remove
    {
      //di->destroy(); // will be destroyed on next iteration, or by unload handler ? we don't want soft-ref to rebind here
      it = _charsByDepth.erase(it); 
    }
    else
    {
      ++it;
    }
  }

  testInvariant();

  return ! _charsByDepth.empty();

}


void
DisplayList::destroy()
{
  //GNASH_REPORT_FUNCTION;

  testInvariant();

  for (iterator it = _charsByDepth.begin(), itEnd = _charsByDepth.end(); it != itEnd; )
  {
    // make a copy
    DisplayItem di = *it;

    // skip if already unloaded
    if ( di->isDestroyed() )
    {
      ++it;
      continue;
    }

    di->destroy();
    it = _charsByDepth.erase(it); 
  }
  testInvariant();
}

// Display the referenced characters. Lower depths
// are obscured by higher depths.
void
DisplayList::display()
{
    testInvariant();

    //GNASH_REPORT_FUNCTION;
    std::stack<int> clipDepthStack;
    
    // We only display characters which are out of the "removed" zone (or should we check isUnloaded?)
    iterator it = beginNonRemoved(_charsByDepth);
    for(iterator endIt = _charsByDepth.end(); it != endIt; ++it)
    {
        character* ch = it->get();

        character* mask = ch->getMask();
        if ( mask && ch->get_visible() && ! mask->isUnloaded() )
        {
            render::begin_submit_mask();
            
            if (mask->boundsInClippingArea())
              mask->display();
            else
              mask->omit_display();
              
            render::end_submit_mask();
            
            if (ch->boundsInClippingArea())
              ch->display();
            else
              ch->omit_display();
              
            render::disable_mask();
            
            continue;
        }

        // Don't display dynamic masks
        if ( ch->isDynamicMask() )
        {
            continue;
        }

        assert(! ch->isUnloaded() ); // we don't advance unloaded chars

        // Check if this charater or any of its parents is a mask.
        // Characters acting as masks should always be rendered to the
        // mask buffer despite their visibility.
        //
        character * parent = ch->get_parent();
        bool renderAsMask = ch->isMaskLayer();
        while(!renderAsMask && parent)
        {
            renderAsMask = parent->isMaskLayer();
            parent = parent->get_parent();
        }
        
        // check for non-mask hiden characters
        if( !renderAsMask && (ch->get_visible() == false))
        {
            ch->omit_display();
            // Don't display non-mask hidden characters
            continue;
        }
    
        int depth = ch->get_depth();
        // Discard useless masks
        while(!clipDepthStack.empty() && (depth > clipDepthStack.top()))
        {
            clipDepthStack.pop();
            render::disable_mask();
        }

        // Push a new mask to the masks stack
    	if ( ch->isMaskLayer() ) // clipDepth != character::noClipDepthValue
        {
            int clipDepth = ch->get_clip_depth();
            clipDepthStack.push(clipDepth);
            render::begin_submit_mask();
        }
        
        if (ch->boundsInClippingArea())
          ch->display();        
        else
          ch->omit_display();
        
        // Notify the renderer that mask drawing has finished.
        if (ch->isMaskLayer())
        {
            render::end_submit_mask();
        }
    } //end of for

    // Discard any remaining masks
    while(!clipDepthStack.empty())
    {
        clipDepthStack.pop();
        render::disable_mask();
    }
    
    
}

void
DisplayList::omit_display()
{
  iterator it = beginNonRemoved(_charsByDepth);
  for(iterator endIt = _charsByDepth.end(); it != endIt; ++it) {
    character* ch = it->get();
    ch->omit_display();
  }
}

/*public*/
void
DisplayList::dump() const
{
  //testInvariant();

  int num=0;
  for( const_iterator it = _charsByDepth.begin(),
      endIt = _charsByDepth.end();
    it != endIt; ++it)
  {
    const DisplayItem& dobj = *it;
    log_debug(_("Item %d at depth %d (char id %d, name %s, type %s)"),
      num, dobj->get_depth(), dobj->get_id(),
      dobj->get_name(), typeName(*dobj));
    num++;
  }
}


void 
DisplayList::add_invalidated_bounds(InvalidatedRanges& ranges, bool force)
{
  testInvariant();
  
  /*
  This function generally has nothing else to do than calling the 
  add_invalidated_bounds() function of all items in the display list.
  However, special optimization is included for masks, which makes it 
  look a bit more complicated. We want to avoid that a masked character
  invalidates an area where the character is invisible because of it's
  mask (which is quite common). For example, think of a large bitmap that
  covers the entire stage and is masked by a very small circle in the
  middle of the stage (ie. it's only visible there). Changes in the 
  bitmap sprite would invalidate the whole stage even if only the small 
  masked portion in the middle really needs to be drawn again.
  
  So, like display(), we keep a stack of masks. Instead of drawing the
  mask we keep a separate list of InvalidatedRanges for the masks which
  later are intersected with the masked characters' InvalidatedRanges.
  
  The code is much based on the display() function, so some references
  in comments have been added for convenience.
  
  For a simpler implementation (that doesn't care about masks, but 
  still produces correct results) see CVS revision 1.96    

  TODO: review this function to take "dynamic" mask and maskees into account
  */
  
  std::stack<int> clipDepthStack; // same method used in display()
  std::stack<InvalidatedRanges> rangesStack;
  bool drawing_mask = false;

  iterator it = beginNonRemoved(_charsByDepth);
  for( iterator endIt = _charsByDepth.end(); it != endIt; ++it)
  {
    DisplayItem& dobj = *it;
    
#ifndef GNASH_USE_GC
    assert(dobj->get_ref_count() > 0);
#endif // ndef GNASH_USE_GC


    int depth = dobj->get_depth();    
    // Discard useless masks
    while (!clipDepthStack.empty() && (depth > clipDepthStack.top())) 
    {
      clipDepthStack.pop();

      // also add mask to ranges because mask itself may have changed
//      ranges.add(rangesStack.top());

      rangesStack.pop();  // disable_mask() equivalent
      
    }
    
    // Push a new mask to the masks stack
    if ( dobj->isMaskLayer() ) // clipDepth != character::noClipDepthValue
    {
      int clipDepth = dobj->get_clip_depth();
      clipDepthStack.push(clipDepth);
      
      drawing_mask = true; // begin_submit_mask equivalent
      
      if (rangesStack.empty())    
      {
        InvalidatedRanges item;
        rangesStack.push(item);
      } else {
        rangesStack.push(rangesStack.top()); // copy the top mask                          
      }
    }
    
    // ==> display() equivalent
    
    if (drawing_mask) {
    
      // --> The child is part of a mask, so add ranges to our 
      // mask ranges stack
      
      assert(!rangesStack.empty());
      dobj->add_invalidated_bounds(rangesStack.top(), true);          
      
      // need to call add_invalidated_bounds again because the previous
      // call needs force==true. Changes to the mask itself may also require
      // re-rendering of the mask area, so we have to add the mask itself
      // to the global ranges, but this time with normal "force" value...
      // As long the mask has not been invalidated and force==false this
      // call won't modify the "ranges" list.
      dobj->add_invalidated_bounds(ranges, force);
      
    } else {
      
      if (rangesStack.empty()) {
      
        // --> normal case for unmasked characters
        dobj->add_invalidated_bounds(ranges, force);
        
      } else {
      
        // --> character is masked, so intersect with "mask"
        
        // first get the ranges of the child in a separate list
        InvalidatedRanges childRanges;
        childRanges.inheritConfig(ranges);
        
        dobj->add_invalidated_bounds(childRanges, force);
        
        // then intersect ranges with topmost "mask"
        childRanges.intersect(rangesStack.top());
        
        // add result to the global ranges
        ranges.add(childRanges);
      
      }
      
    } // not drawing mask 
    
    // <== end of display() equivalent
    
    
    // Mask "drawing" has finished
    if (dobj->isMaskLayer())
    {
      drawing_mask = false; // end_submit_mask equivalent
    }
  }
  
  
}


/// This method is not in the header in the hope DisplayItemDepthLess
/// will be inlined by compiler

struct DisplayItemDepthLess {
  bool operator() (const DisplayItem& d1, const DisplayItem& d2)
  {
    return d1->get_depth() < d2->get_depth();
  }
};

void
DisplayList::sort()
{
  _charsByDepth.sort(DisplayItemDepthLess());
}

void
DisplayList::mergeDisplayList(DisplayList & newList)
{
    testInvariant();

    iterator itOld = beginNonRemoved(_charsByDepth);
    iterator itNew = beginNonRemoved(newList._charsByDepth);

    iterator itOldEnd = dlistTagsEffectivZoneEnd(_charsByDepth);
    iterator itNewEnd = newList._charsByDepth.end(); 
    assert(itNewEnd == dlistTagsEffectivZoneEnd(newList._charsByDepth) );

    // step1. 
    // starting scanning both lists.
    while( itOld != itOldEnd )
    {
        iterator itOldBackup = itOld;
        
        boost::intrusive_ptr<character> chOld = itOldBackup->get();
        int depthOld = chOld->get_depth();

        while( itNew != itNewEnd )
        {
            iterator itNewBackup = itNew;
            
            boost::intrusive_ptr<character> chNew = itNewBackup->get();
            int depthNew = chNew->get_depth();
            
            // depth in old list is occupied, and empty in new list.
            if( depthOld < depthNew )
            {
                itOld++;
                // unload the character if it's in static zone(-16384,0)
                if( depthOld < 0)
                {
                    _charsByDepth.erase(itOldBackup);

                     if ( chOld->unload() )
                    {
                        reinsertRemovedCharacter(chOld);
                    }
                    else 
                    {
                        chOld->destroy();
                    }
                }

                break;
            }
            // depth is occupied in both lists
            else if( depthOld == depthNew )
            {
                itOld++;
                itNew++;
				
                bool is_ratio_compatible = chOld->get_ratio() == chNew->get_ratio();
                if( !is_ratio_compatible || chOld->isDynamic() || !chOld->isActionScriptReferenceable() )
                {
                    // replace the character in old list with corresponding character in new list
                    _charsByDepth.insert(itOldBackup, *itNewBackup);
                    _charsByDepth.erase(itOldBackup);
                    
                    // unload the old character
                    if ( chOld->unload() )
                    {
                        reinsertRemovedCharacter(chOld);
                    } 
                    else 
                    {
                        chOld->destroy();
                    }
                }
                else
                {
                    newList._charsByDepth.erase(itNewBackup);

                    // replace the transformation SWFMatrix if the old character accepts 
					// static transformation.
                    if( chOld->get_accept_anim_moves() )
                    {
                        chOld->copyMatrix(*chNew); // copy SWFMatrix and caches 
                        chOld->set_cxform(chNew->get_cxform());
                    }
                    chNew->unload();
                    chNew->destroy();
                }

                break;
            }
            // depth in old list is empty, but occupied in new list.
            else 
            {
                itNew++;
                // add the new character to the old list.
                _charsByDepth.insert(itOldBackup, *itNewBackup );
            }
        }// end of while

        // break if finish scanning the new list
        if( itNew == itNewEnd )
        {
            break;
        }
    }// end of while

    // step2(only required if scanning of new list finished earlier in step1).
    // continue to scan the static zone of the old list.
    // unload remaining characters directly.
    while( (itOld != itOldEnd) && ((*itOld)->get_depth() < 0) )
    {
        boost::intrusive_ptr<character> chOld = itOld->get();

        itOld = _charsByDepth.erase(itOld);

        if ( chOld->unload() )
        {
            reinsertRemovedCharacter(chOld);
        }
        else 
        {
            chOld->destroy();
        }
    }

    // step3(only required if scanning of old list finished earlier in step1).
    // continue to scan the new list.
    // add remaining characters directly.
    if( itNew != itNewEnd )
    {
        _charsByDepth.insert(itOld, itNew, itNewEnd);
    }

    // step4.
    // Copy all unloaded characters from the new display list to the old display list, 
    // and clear the new display list
    for (itNew = newList._charsByDepth.begin(); itNew != itNewEnd; ++itNew)
    {
        boost::intrusive_ptr<character> chNew = itNew->get();
        int depthNew = chNew->get_depth();

        if( chNew->isUnloaded() )
        {
            iterator it = std::find_if(_charsByDepth.begin(), _charsByDepth.end(),
                    DepthGreaterOrEqual(depthNew));
            
            _charsByDepth.insert(it, *itNew);
        }
    }

    // clear the new display list after merge
    // ASSERT:
    // 	- Any element in newList._charsByDepth is either marked as unloaded
    //    or found in this list
#if GNASH_PARANOIA_LEVEL > 1
    for (iterator i=newList._charsByDepth.begin(), e=newList._charsByDepth.end(); i!=e; ++i)
    {
        character* ch = (*i).get();
        if ( ! ch->isUnloaded() )
	{
		iterator found = std::find(_charsByDepth.begin(), _charsByDepth.end(), ch);
		if ( found == _charsByDepth.end() )
		{
			log_error("mergeDisplayList: character %s (%s at depth %d [%d]) "
				"about to be discarded in given display list"
				" is not marked as unloaded and not found in the"
				" merged current displaylist",
				ch->getTarget(), typeName(*ch), ch->get_depth(),
				ch->get_depth()-character::staticDepthOffset);
			abort();
		}
	}
    }
#endif
    newList._charsByDepth.clear();

    testInvariant();
}



std::ostream& operator<< (std::ostream& os, const DisplayList& dl)
{
  os << "By depth: ";
  for (DisplayList::const_iterator it = dl._charsByDepth.begin(),
      itEnd = dl._charsByDepth.end();
      it != itEnd;
      ++it)
  {
    const DisplayItem& item = *it; 
    if ( it != dl._charsByDepth.begin() ) os << " | ";
    os << "ch id:" << item->get_id()
      << " name:" << item->get_name()
      << " depth:" << item->get_depth();
  }

  return os;
}

void
DisplayList::reinsertRemovedCharacter(boost::intrusive_ptr<character> ch)
{
  assert(ch->isUnloaded());
  testInvariant();

  // TODO: have this done by character::unload() instead ?
  int oldDepth = ch->get_depth();
  int newDepth = character::removedDepthOffset - oldDepth;
  ch->set_depth(newDepth);

  // TODO: optimize this by searching from the end(lowest depth).
  container_type::iterator it = std::find_if(
      _charsByDepth.begin(), _charsByDepth.end(),
      DepthGreaterOrEqual(newDepth));

  _charsByDepth.insert(it, DisplayItem(ch));

  testInvariant();
}

/*private static*/
DisplayList::iterator
DisplayList::beginNonRemoved(container_type& c)
{
  return std::find_if(c.begin(), c.end(),
      DepthGreaterOrEqual(character::removedDepthOffset - character::staticDepthOffset));
}

/*private static*/
DisplayList::const_iterator
DisplayList::beginNonRemoved(const container_type& c)
{
  return std::find_if(c.begin(), c.end(),
      DepthGreaterOrEqual(character::removedDepthOffset - character::staticDepthOffset));
}

/*private static*/
DisplayList::iterator
DisplayList::dlistTagsEffectivZoneEnd(container_type& c)
{
    return std::find_if(c.begin(), c.end(), 
               DepthGreaterOrEqual(0xffff + character::staticDepthOffset));
}

/*private static*/
DisplayList::const_iterator
DisplayList::dlistTagsEffectivZoneEnd(const container_type& c)
{
    return std::find_if(c.begin(), c.end(), 
               DepthGreaterOrEqual(0xffff + character::staticDepthOffset));
}

void
DisplayList::removeUnloaded()
{
  testInvariant();

#if 1
  // This is a list, so list::remove_if is quicker and cleaner than std::remove_if
  _charsByDepth.remove_if(boost::bind(&character::isUnloaded, _1));
#else
  iterator last = std::remove_if(_charsByDepth.begin(), _charsByDepth.end(), boost::bind(&character::isUnloaded, _1));
  _charsByDepth.erase(last, _charsByDepth.end());
#endif

  testInvariant();
}

bool
DisplayList::isSorted() const
{
  if ( _charsByDepth.empty() ) return true;

  const_iterator i=_charsByDepth.begin();
  int minDepth = (*i)->get_depth();
  ++i;
  for (const_iterator e=_charsByDepth.end(); i!=e; ++i)
  {
    int nextDepth = (*i)->get_depth();

    if ( nextDepth < minDepth ) return false;
    minDepth = nextDepth;
  }
  return true;
}

} // namespace gnash


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
