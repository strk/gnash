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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
//

#include <boost/utility.hpp>
#include "PathParser.h"
#include <deque>
#include <map>
#include <boost/bind.hpp>

namespace gnash
{

const Point2d<int>&
UnivocalPath::startPoint() const
{
  return _fill_type == FILL_LEFT ? _path->ap : _path->m_edges.back().ap;
}

const Point2d<int>&
UnivocalPath::endPoint() const
{
  return _fill_type == FILL_LEFT ? _path->m_edges.back().ap : _path->ap;
}



PathParser::PathParser(const std::vector<path>& paths, size_t numstyles)
: _paths(paths),
  _num_styles(numstyles),
  _shape_origin(0, 0),
  _cur_endpoint(0, 0)
{}

void
PathParser::run(const cxform& cx, const matrix& mat)
{
  typedef std::deque<UnivocalPath> UniPathList;

  std::vector<UniPathList> unipathvec(_num_styles);

  for (size_t i = 0; i < _paths.size(); ++i) {
  
    if (_paths[i].is_empty()) {
      continue;
    }

    int leftfill = _paths[i].getLeftFill();
    if (leftfill) {
      unipathvec[leftfill-1].push_front(UnivocalPath(&_paths[i], UnivocalPath::FILL_LEFT));
    }

    int rightfill = _paths[i].getRightFill();
    if (rightfill) {
      unipathvec[rightfill-1].push_front(UnivocalPath(&_paths[i], UnivocalPath::FILL_RIGHT));
    }
  }

  for (size_t i = 0; i < _num_styles; ++i) {

    start_shapes(i+1, cx, mat);
    UniPathList& path_list = unipathvec[i];
      
    while (!path_list.empty()) {

      if (closed_shape()) {
        reset_shape(path_list.front());
        path_list.pop_front();
      }

      UniPathList::iterator it = std::find_if(path_list.begin(),
        path_list.end(), boost::bind(&PathParser::emitConnecting, this, _1));
     
      if (it == path_list.end()) {
        if (!closed_shape()) {
          std::cout << "error: path not closed!" << std::endl;
          _cur_endpoint = _shape_origin;
        }
      } else {
        path_list.erase(it);
      }

    }
    end_shapes(i+1);
  }

}

bool
PathParser::emitConnecting(const UnivocalPath& subject)
{
  if (subject.startPoint() != _cur_endpoint) {
    return false;
  }

  append(subject);

  return true;
}

void
PathParser::append(const UnivocalPath& append_path)
{
  const std::vector< Edge<int> >& edges = append_path._path->m_edges;

  if (append_path._fill_type == UnivocalPath::FILL_LEFT) {

    std::for_each(edges.begin(), edges.end(), boost::bind(&PathParser::line_to,
                                                          this, _1));
    _cur_endpoint = edges.back().ap;

  } else {

    for (std::vector<edge>::const_reverse_iterator prev = edges.rbegin(),
         it = boost::next(prev), end = edges.rend(); it != end; ++it, ++prev) {
      if ((*prev).isStraight()) {
        lineTo((*it).ap);
      } else {
        line_to(Edge<int>((*prev).cp, (*it).ap));
      }
    }
    
    _cur_endpoint = append_path._path->ap;
    line_to(Edge<int>(edges.front().cp, _cur_endpoint));

  }
}

void
PathParser::line_to(const Edge<int>& curve)
{
  if (curve.isStraight()) {
    lineTo(curve.ap);
  } else {
    curveTo(curve);
  }
}

void
PathParser::start_shapes(int fill_style, const cxform& cx, const matrix& mat)
{
  prepareFill(fill_style, cx, mat);
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
