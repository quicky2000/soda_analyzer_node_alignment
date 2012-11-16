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
#ifndef _NODE_H_
#define _NODE_H_

#include "osm_core_element.h"
#include <map>

namespace osm_diff_analyzer_node_alignment
{
  class way;

  class node
  {
  public:
    inline node(const osm_api_data_types::osm_object::t_osm_id & p_id,
                const std::string & p_user_name,
                const osm_api_data_types::osm_object::t_osm_id & p_user_id,
                const osm_api_data_types::osm_core_element::t_osm_version & p_version,
                const float & p_lat,
                const float & p_lon,
                bool p_in_changeset);
    void attach_to(const way & p_way);
    inline const osm_api_data_types::osm_object::t_osm_id & get_id(void)const;
    inline const float & get_lat(void)const;
    inline const float & get_lon(void)const;
    inline const osm_api_data_types::osm_core_element::t_osm_version & get_version(void)const;
  private:
    const osm_api_data_types::osm_object::t_osm_id m_id;
    const std::string m_user_name;
    const osm_api_data_types::osm_object::t_osm_id m_user_id;
    const osm_api_data_types::osm_core_element::t_osm_version m_version;
    float m_lat;
    float m_lon;
    bool m_in_changeset;
    std::map<osm_api_data_types::osm_object::t_osm_id,const way*> m_ways;
  };
  //----------------------------------------------------------------------------
  node::node(const osm_api_data_types::osm_object::t_osm_id & p_id,
             const std::string & p_user_name,
             const osm_api_data_types::osm_object::t_osm_id & p_user_id,
             const osm_api_data_types::osm_core_element::t_osm_version & p_version,
             const float & p_lat,
             const float & p_lon,
             bool p_in_changeset):
    m_id(p_id),
    m_user_name(p_user_name),
    m_user_id(p_user_id),
    m_version(p_version),
    m_lat(p_lat),
    m_lon(p_lon),
    m_in_changeset(p_in_changeset)
      {
      }

    //----------------------------------------------------------------------------
    const osm_api_data_types::osm_core_element::t_osm_version & node::get_version(void)const
      {
        return m_version;
      }

    //----------------------------------------------------------------------------
    const osm_api_data_types::osm_object::t_osm_id & node::get_id(void)const
      {
        return m_id;
      }
    //----------------------------------------------------------------------------
    const float & node::get_lat(void)const
      {
        return m_lat;
      }
    //----------------------------------------------------------------------------
    const float & node::get_lon(void)const
      {
        return m_lon;
      }
}

#endif // _NODE_H_
//EOF
