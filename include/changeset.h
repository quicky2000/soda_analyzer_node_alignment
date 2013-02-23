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
#ifndef _CHANGESET_H_
#define _CHANGESET_H_

#include "osm_api_data_types.h"
#include "node_alignment_common_api.h"
#include <string>
#include <fstream>
#include <set>
#include <map>

namespace osm_diff_analyzer_node_alignment
{
  class node;
  class way;
  class node_alignment_analyzer;
  class changeset
  {
  public:
    inline changeset(std::ofstream & p_report,
                     node_alignment_analyzer & p_analyzer,
                     const osm_api_data_types::osm_object::t_osm_id & p_id,
		     const std::string & p_user_name,
		     const osm_api_data_types::osm_object::t_osm_id & p_user_id);
    ~changeset(void);
    void add(const osm_api_data_types::osm_way & p_way);
    void add(const osm_api_data_types::osm_node & p_node);
    void search_aligned_ways(std::ofstream & p_stream);
    bool check_way(const osm_api_data_types::osm_object::t_osm_id & p_id,const std::vector<osm_api_data_types::osm_object::t_osm_id> & p_node_refs);
    inline static void set_api(node_alignment_common_api & p_api);
    inline static void set_modif_rate_min_level(const float & p_rate);
    inline static void set_min_alignment_modification_rate(const float & p_rate);
    inline static void set_min_way_node_nb(const float & p_rate);
    inline static const float & get_min_alignment_modification_rate(void);
    inline static const uint32_t & get_min_way_node_nb(void);
    inline static const float & get_modif_rate_min_level(void);
  private:
    void create_svg(const osm_api_data_types::osm_object::t_osm_id & p_id,
                    const std::vector<std::pair<double,double> > & p_old_list,
                    const std::vector<std::pair<double,double> > & p_new_list);
    void create_gpx(const std::string & p_file_name,
                    const std::vector<std::pair<double,double> > & p_points);
    
    std::ofstream & m_report;
    node_alignment_analyzer & m_analyzer;
    const osm_api_data_types::osm_object::t_osm_id m_id;
    const std::string m_user_name;
    const osm_api_data_types::osm_object::t_osm_id m_user_id;
    std::map<osm_api_data_types::osm_object::t_osm_id,way*> m_ways;
    std::map<osm_api_data_types::osm_object::t_osm_id,node*> m_nodes;
    std::set<osm_api_data_types::osm_object::t_osm_id> m_nodes_to_check;
    std::set<osm_api_data_types::osm_object::t_osm_id> m_checked_ways;

    static node_alignment_common_api * m_api; 

    static float m_modif_rate_min_level;
    static float m_min_alignment_modification_rate;
    static uint32_t m_min_way_node_nb;
  };
  //----------------------------------------------------------------------------
  changeset::changeset(std::ofstream & p_report,
                       node_alignment_analyzer & p_analyzer,
                       const osm_api_data_types::osm_object::t_osm_id & p_id,
                       const std::string & p_user_name,
                       const osm_api_data_types::osm_object::t_osm_id & p_user_id):
    m_report(p_report),
    m_analyzer(p_analyzer),
    m_id(p_id),
    m_user_name(p_user_name),
    m_user_id(p_user_id)
      {
      }

   //----------------------------------------------------------------------------
    void changeset::set_api(node_alignment_common_api & p_api)
    {
      m_api = & p_api;
    }

   //----------------------------------------------------------------------------
    void changeset::set_min_alignment_modification_rate(const float & p_rate)
    {
      m_min_alignment_modification_rate = p_rate ;
    }

   //----------------------------------------------------------------------------
    void changeset::set_min_way_node_nb(const float & p_min_way_node_nb)
    {
      m_min_way_node_nb = p_min_way_node_nb;
    }

   //----------------------------------------------------------------------------
    void changeset::set_modif_rate_min_level(const float & p_rate)
    {
      m_modif_rate_min_level = p_rate;
    }

   //----------------------------------------------------------------------------
    const float & changeset::get_min_alignment_modification_rate(void)
    {
      return m_min_alignment_modification_rate;
    }

   //----------------------------------------------------------------------------
    const uint32_t & changeset::get_min_way_node_nb(void)
    {
      return m_min_way_node_nb;
    }

   //----------------------------------------------------------------------------
    const float & changeset::get_modif_rate_min_level(void)
    {
      return m_modif_rate_min_level;
    }

}
#endif // _CHANGESET_H_
