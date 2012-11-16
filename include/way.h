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
#ifndef _WAY_H_
#define _WAY_H_

#include "osm_core_element.h"
#include <map>
#include <vector>

namespace osm_diff_analyzer_node_alignment
{
  class node;

  class way
  {
  public:
    inline way(const osm_api_data_types::osm_object::t_osm_id & p_id,
               const std::string & p_user_name,
               const osm_api_data_types::osm_object::t_osm_id & p_user_id,
               const osm_api_data_types::osm_core_element::t_osm_version & p_version,
               bool p_in_changeset);
    inline void set_node_refs(const std::vector<osm_api_data_types::osm_object::t_osm_id> & p_node_refs);
    inline const std::vector<osm_api_data_types::osm_object::t_osm_id> & get_node_refs(void)const;
    inline const osm_api_data_types::osm_object::t_osm_id & get_id(void)const;
    inline bool is_checked(void)const;
    inline void set_checked(void);
  private:
    const osm_api_data_types::osm_object::t_osm_id m_id;
    const std::string m_user_name;
    const osm_api_data_types::osm_object::t_osm_id m_user_id;
    const osm_api_data_types::osm_core_element::t_osm_version m_version;
    bool m_in_changeset;
    bool m_checked;
    std::map<osm_api_data_types::osm_object::t_osm_id,node*> m_nodes;
    std::vector<osm_api_data_types::osm_object::t_osm_id> m_ordered_nodes;
      
  };
  //----------------------------------------------------------------------------
  const std::vector<osm_api_data_types::osm_object::t_osm_id> & way::get_node_refs(void)const
    {
      return m_ordered_nodes; 
    }

  //----------------------------------------------------------------------------
  way::way(const osm_api_data_types::osm_object::t_osm_id & p_id,
           const std::string & p_user_name,
           const osm_api_data_types::osm_object::t_osm_id & p_user_id,
           const osm_api_data_types::osm_core_element::t_osm_version & p_version,
           bool p_in_changeset):
    m_id(p_id),
    m_user_name(p_user_name),
    m_user_id(p_user_id),
    m_version(p_version),
    m_in_changeset(p_in_changeset),
    m_checked(false)
      {
      }

    //----------------------------------------------------------------------------
    void way::set_node_refs(const std::vector<osm_api_data_types::osm_object::t_osm_id> & p_node_refs)
    {
      m_ordered_nodes = p_node_refs;
    }

    //----------------------------------------------------------------------------
    const osm_api_data_types::osm_object::t_osm_id & way::get_id(void)const
      {
        return m_id;
      }

    //----------------------------------------------------------------------------
    bool way::is_checked(void)const
    {
      return m_checked;
    }

    //----------------------------------------------------------------------------
    void way::set_checked(void)
    {
      m_checked = true;
    }
}

#endif // _WAY_H_
//EOF
