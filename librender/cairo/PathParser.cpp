// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
//

#include <functional>
#include <iterator>
#include <map>

#include "PathParser.h"
#include "log.h"

namespace gnash
{

const point&
UnivocalPath::startPoint() const
{
  return _fill_type == FILL_LEFT ? _path->ap : _path->m_edges.back().ap;
}

const point&
UnivocalPath::endPoint() const
{
  return _fill_type == FILL_LEFT ? _path->m_edges.back().ap : _path->ap;
}

PathParser::PathParser(const std::vector<Path>& paths, size_t numstyles)
: _paths(paths),
  _num_styles(numstyles),
  _shape_origin(0, 0),
  _cur_endpoint(0, 0)
{}

void
PathParser::run(const SWFCxForm& cx, const SWFMatrix& /*mat*/)
{
  // Since we frequently remove an element from the front or the back, we use
  // a double ended queue here.
  typedef std::deque<UnivocalPath> UniPathList;

  std::vector<UniPathList> unipathvec(_num_styles);

  for (const Path& path : _paths) {
  
    if (path.empty()) {
      continue;
    }

    int leftfill = path.getLeftFill();
    if (leftfill) {
      unipathvec[leftfill-1].emplace_front(&path, UnivocalPath::FILL_LEFT);
    }

    int rightfill = path.getRightFill();
    if (rightfill) {
      unipathvec[rightfill-1].emplace_front(&path, UnivocalPath::FILL_RIGHT);
    }
  }

  for (size_t i = 0; i < _num_styles; ++i) {

    start_shapes(i+1, cx);
    UniPathList& path_list = unipathvec[i];
      
    while (!path_list.empty()) {

      if (closed_shape()) {
        reset_shape(path_list.front());
        path_list.pop_front();
      }

      UniPathList::iterator it = emitConnecting(path_list);
     
      if (it == path_list.end()) {
        if (!closed_shape()) {
            log_error(_("path not closed!"));
          _cur_endpoint = _shape_origin;
        }
      } else {
        path_list.erase(it);
      }

    }
    end_shapes(i+1);
  }

}

std::deque<UnivocalPath>::iterator
PathParser::emitConnecting(std::deque<UnivocalPath>& paths)
{
  std::deque<UnivocalPath>::iterator it = paths.begin(),
                                     end = paths.end();
  while (it != end) {

    if ((*it).startPoint() == _cur_endpoint) {
      break;
    }

    ++it;
  }

  if (it != end) {
    append(*it);
  }
  return it;
}

void
PathParser::append(const UnivocalPath& append_path)
{
  const std::vector<Edge>& edges = append_path._path->m_edges;

  if (append_path._fill_type == UnivocalPath::FILL_LEFT) {

    std::for_each(edges.begin(), edges.end(), std::bind(&PathParser::line_to,
        this, std::placeholders::_1));
  } else {

    for (std::vector<Edge>::const_reverse_iterator prev = edges.rbegin(),
         it = std::next(prev), end = edges.rend(); it != end; ++it, ++prev) {
      if ((*prev).straight()) {
        lineTo((*it).ap);
      } else {
        line_to(Edge((*prev).cp, (*it).ap));
      }
    }

    line_to(Edge(edges.front().cp, append_path.endPoint()));
  }
    
  _cur_endpoint = append_path.endPoint();
}

void
PathParser::line_to(const Edge& curve)
{
  if (curve.straight()) {
    lineTo(curve.ap);
  } else {
    curveTo(curve);
  }
}

void
PathParser::start_shapes(int fill_style, const SWFCxForm& cx)
{
  prepareFill(fill_style, cx);
}

void
PathParser::end_shapes(int fill_style)
{
  terminateFill(fill_style);
}

void
PathParser::reset_shape(const UnivocalPath& append_path)
{
  fillShape();
  
  _shape_origin = append_path.startPoint();

  moveTo(_shape_origin);

  append(append_path);
}

bool
PathParser::closed_shape()
{
  return _cur_endpoint == _shape_origin;
}


} // namespace gnash
