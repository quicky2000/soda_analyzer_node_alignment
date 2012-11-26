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
#ifndef _SVG_REPORT_H_
#define _SVG_REPORT_H_

#include <fstream>
#include <vector>
#include <inttypes.h>

namespace osm_diff_analyzer_node_alignment
{
  class svg_report
  {
  public:
    svg_report(void);
    void update_xtrem_coordinates(const std::vector<std::pair<double,double> > & p_list);
    void adjust_xtrem_coordinates(const double & p_coef);
    void open(const std::string & p_file_name);
    void draw_polyline(const std::vector<std::pair<double,double> > & p_list,
                       const std::string & p_color,
                       const uint32_t & p_supp);
    void close(void);
  private:
    uint32_t m_width;
    uint32_t m_height;
    uint32_t m_circle_size;
    double m_min_lat;
    double m_max_lat;
    double m_min_lon;
    double m_max_lon;
    std::ofstream m_svg_file;
  };
}
#endif // _SVG_REPORT_H_
//EOF
