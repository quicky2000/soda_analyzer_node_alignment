/*
  This file is part of osm_diff_analyzer_node_alignment, Openstreetmap
  diff analyzer based on CPP diff representation. It's aim is to survey
  ways edited and to generate an alert in case of node alignment
  Copyright (C) 2012  Julien Thevenon ( julien_thevenon at yahoo.fr )

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>
*/

#include "svg_report.h"
#include <limits>
#include <iostream>

namespace osm_diff_analyzer_node_alignment
{
  //----------------------------------------------------------------------------
  svg_report::svg_report(void):
    m_width(600),
    m_height(600),
    m_circle_size(5),
    m_min_lat(std::numeric_limits<double>::max()),
    m_max_lat(-(std::numeric_limits<double>::max()- 1 )),
    m_min_lon(std::numeric_limits<double>::max()),
    m_max_lon(-(std::numeric_limits<double>::max()- 1 ))
    {
    
    }

  //----------------------------------------------------------------------------
  void svg_report::update_xtrem_coordinates(const std::vector<std::pair<double,double> > & p_list)
  {
    for(std::vector<std::pair<double,double> >::const_iterator l_iter = p_list.begin() ;
        l_iter != p_list.end();
        ++l_iter)
      {
        if(l_iter->first > m_max_lat)
          {
            m_max_lat = l_iter->first;
          }
        if(l_iter->first < m_min_lat)
          {
            m_min_lat = l_iter->first;
          }
        if(l_iter->second > m_max_lon)
          {
            m_max_lon = l_iter->second;
          }
        if(l_iter->second < m_min_lon)
          {
            m_min_lon = l_iter->second;
          }
      }
  }

  //----------------------------------------------------------------------------
  void svg_report::adjust_xtrem_coordinates(const double & p_coef)
  {
    double l_diff_lat = m_max_lat - m_min_lat ;
    double l_diff_lon = m_max_lon - m_min_lon;
    m_min_lat = m_min_lat - l_diff_lat * p_coef;
    m_max_lat = m_max_lat + l_diff_lat * p_coef;
    m_min_lon = m_min_lon - l_diff_lon * p_coef;
    m_max_lon = m_max_lon + l_diff_lon * p_coef;
    
  }

  //----------------------------------------------------------------------------
  void svg_report::open(const std::string & p_file_name)
  {
    m_svg_file.open(p_file_name.c_str());
    if(!m_svg_file.is_open())
      {
        std::cout << "Error when creating SVG file \"" << p_file_name << "\"" << std::endl ;
      }
    m_svg_file << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << std::endl ;
    m_svg_file << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"" << m_width << "\" height=\"" << m_height << "\">" << std::endl ;
  }

  //----------------------------------------------------------------------------
  void svg_report::close(void)
  {
    m_svg_file << "</svg>" << std::endl ;
    m_svg_file.close();
  }

  //----------------------------------------------------------------------------
  void svg_report::draw_polyline(const std::vector<std::pair<double,double> > & p_list,
                                 const std::string & p_color,
                                 const uint32_t & p_supp)
  {
    double l_previous_x = 0.0;
    double l_previous_y = 0.0;
    for(std::vector<std::pair<double,double> >::const_iterator l_iter = p_list.begin() ;
        l_iter != p_list.end();
        ++l_iter)
      {
        double l_x = (( m_width - 1.0) * (l_iter->second -m_min_lon ))/ (m_max_lon - m_min_lon);
        double l_y = (( m_height - 1.0) * (m_max_lat - l_iter->first ))/ (m_max_lat - m_min_lat);
        if(l_previous_x != 0.0 && l_previous_y != 0.0)
          {
            m_svg_file << "<line x1=\""<< l_previous_x << "\" y1=\"" << l_previous_y << "\" x2=\"" << l_x << "\" y2=\"" << l_y << "\" stroke=\"" << p_color << "\" />" << std::endl ;
          }
        l_previous_x = l_x;
        l_previous_y = l_y;
        m_svg_file << "<circle cx=\""<< l_x << "\" cy=\"" << l_y << "\" r=\""<< (m_circle_size+p_supp) <<"\" fill=\"" << p_color << "\" />" << std::endl ;
      }
  }

}

//EOF
