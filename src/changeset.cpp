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

#include "changeset.h"
#include "way.h"
#include "node.h"
#include "osm_way.h"
#include <sstream>
#include <limits>
#include <cmath>

namespace osm_diff_analyzer_node_alignment
{
  //----------------------------------------------------------------------------
  void changeset::add(const osm_api_data_types::osm_way & p_way)
  {

    // Create a simplified representation of way that will survive to diff end of life
    way * l_way = new way(p_way.get_id(),p_way.get_user(),p_way.get_user_id(),p_way.get_version(),true);
    m_ways.insert(std::map<osm_api_data_types::osm_object::t_osm_id,way*>::value_type(p_way.get_id(),l_way));
    const std::vector<osm_api_data_types::osm_object::t_osm_id> & l_node_refs = p_way.get_node_refs();
    l_way->set_node_refs(l_node_refs);
  }
  //----------------------------------------------------------------------------
  void changeset::add(const osm_api_data_types::osm_node & p_node)
  {
    node * l_node = new node(p_node.get_id(),p_node.get_user(),p_node.get_user_id(),p_node.get_version(),p_node.get_lat(),p_node.get_lon(),true);
    m_nodes.insert(std::map<osm_api_data_types::osm_object::t_osm_id,node*>::value_type(p_node.get_id(),l_node));
  }

  //----------------------------------------------------------------------------
  changeset::~changeset(void)
  {
    for(std::map<osm_api_data_types::osm_object::t_osm_id,way*>::iterator l_iter = m_ways.begin();
        l_iter != m_ways.end();
        ++l_iter)
      {
        delete l_iter->second;
      }
    for(std::map<osm_api_data_types::osm_object::t_osm_id,node*>::iterator l_iter = m_nodes.begin();
        l_iter != m_nodes.end();
        ++l_iter)
      {
        delete l_iter->second;
      }
  }
  //----------------------------------------------------------------------------
  void changeset::search_aligned_ways(std::ofstream & p_stream)
  {
    std::cout << "Search aligned ways in changeset " << m_id << std::endl ;
    // First check if modified ways has been aligned to eliminate a maximum of nodes to limite API
    // call that will be done later for each node to determine to which way it belongs
    // If a way has been aligned all its nodes will be removed and no more analyzed
    for(std::map<osm_api_data_types::osm_object::t_osm_id,way*>::iterator l_iter_way = m_ways.begin();
        l_iter_way != m_ways.end();
        ++l_iter_way)
      {
        check_way(l_iter_way->second->get_id(),l_iter_way->second->get_node_refs());
      }


    std::cout << "Search aligned nodes in changeset " << m_id << std::endl ;
    // For each node get ways
    while(m_nodes.size())
      {
        std::map<osm_api_data_types::osm_object::t_osm_id,node*>::iterator l_iter_node = m_nodes.begin();

        bool l_aligned = false;
        const std::vector<osm_api_data_types::osm_way*> * const l_ways = m_api->get_node_ways(l_iter_node->second->get_id());
        for(std::vector<osm_api_data_types::osm_way*>::const_iterator l_iter_way = l_ways->begin();
            l_iter_way != l_ways->end() && !l_aligned;
            ++l_iter_way)
          {
            if(!l_aligned && m_checked_ways.find((*l_iter_way)->get_id()) == m_checked_ways.end())
              {
                l_aligned = check_way((*l_iter_way)->get_id(),(*l_iter_way)->get_node_refs());
                m_checked_ways.insert((*l_iter_way)->get_id());
              }
            delete *l_iter_way;
          }
        if(!l_aligned)
          {
            delete l_iter_node->second;
            m_nodes.erase(l_iter_node);
          }

      }

  }

  //----------------------------------------------------------------------------
  bool changeset::check_way(const osm_api_data_types::osm_object::t_osm_id & p_id,const std::vector<osm_api_data_types::osm_object::t_osm_id> & p_node_refs)
  {
    bool l_result = false;
    if(p_node_refs.size()> m_min_way_node_nb)
      {
        std::cout << "Check way : " << p_id << std::endl ;
        std::vector<node*> l_modified_nodes;
        std::vector<osm_api_data_types::osm_object::t_osm_id> l_unchanged_nodes;
        // Check if some existings nodes belong to this way
        for(std::vector<osm_api_data_types::osm_object::t_osm_id>::const_iterator l_way_node = p_node_refs.begin();
            l_way_node != p_node_refs.end();
            ++l_way_node)
          {
            std::map<osm_api_data_types::osm_object::t_osm_id,node*>::iterator l_node_iter = m_nodes.find(*l_way_node);
            if(l_node_iter != m_nodes.end())
              {
                l_modified_nodes.push_back(l_node_iter->second);
              }
            else
              {
                l_unchanged_nodes.push_back(*l_way_node);
              }
          }
        // check if more than coef % node has been modified : an abusive alignment modify almost every node except one
        float l_modif_rate = ((float)(l_modified_nodes.size())/((float)p_node_refs.size()));
        std::cout << "Way modif rate = " << l_modif_rate << std::endl ;
        std::cout << "nb modified nodes  = " <<  l_modified_nodes.size() << " vs nb nodes " << p_node_refs.size() << std::endl ;
        if(l_modified_nodes.size() == p_node_refs.size() - 2 || l_modif_rate > m_modif_rate_min_level)
          {
            // Check how many nodes has been moved by comparing with previous version of node
            // The check stop if the number of unmoved node is sufficiant to be sure that the modification rate will not be reached
            uint32_t l_nb_moved_node = l_modified_nodes.size();
            l_modif_rate = ((float)(l_nb_moved_node)/((float)p_node_refs.size()));
            std::vector<std::pair<double,double> > l_old_coordinates;
            std::vector<std::pair<double,double> > l_new_coordinates;
            std::cout << "Check how many nodes has been moved" << std::endl ;
            for(std::vector<node*>::const_iterator l_iter_node = l_modified_nodes.begin();
                l_iter_node != l_modified_nodes.end() && ( l_modif_rate > m_modif_rate_min_level || l_nb_moved_node >= p_node_refs.size() - 2 );
                ++l_iter_node)
              {
                const osm_api_data_types::osm_node * l_previous_node = m_api->get_node_version((*l_iter_node)->get_id(),(*l_iter_node)->get_version()-1);
                assert(l_previous_node);
                if(l_previous_node->get_lat() == (*l_iter_node)->get_lat() && l_previous_node->get_lon() == (*l_iter_node)->get_lon())
                  {
                    --l_nb_moved_node;
                    l_modif_rate = ((float)(l_nb_moved_node)/((float)p_node_refs.size()));
                    std::cout << "Recomputed moved rate : " << l_modif_rate << std::endl ;
                  }
                else
                  {
                    l_old_coordinates.push_back(std::pair<double,double>(l_previous_node->get_lat(),l_previous_node->get_lon()));
                    l_new_coordinates.push_back(std::pair<double,double>((*l_iter_node)->get_lat(),(*l_iter_node)->get_lon()));
                  }
                delete l_previous_node;
              }
            // Check if verification has been completed : sign of complete aligned way
            if(l_modif_rate > m_modif_rate_min_level || l_nb_moved_node >= p_node_refs.size() - 2 )
              {
                std::cout << "Getting unmodifed coordinates" << std::endl ;
                // Complete coordinates vectors with unchanged nodes
                for(std::vector<osm_api_data_types::osm_object::t_osm_id>::const_iterator l_node_iter = l_unchanged_nodes.begin();
                    l_node_iter != l_unchanged_nodes.end();
                    ++l_node_iter)
                  {
                    const osm_api_data_types::osm_node * l_previous_node = m_api->get_node_version(*l_node_iter);
                    l_new_coordinates.push_back(std::pair<double,double>(l_previous_node->get_lat(),l_previous_node->get_lon()));
                    l_old_coordinates.push_back(std::pair<double,double>(l_previous_node->get_lat(),l_previous_node->get_lon())); 
                  }
                std::cout << "Computing alignment" << std::endl ;
                double l_old_max_diff_square ;
                double l_old_result = moindre_carres(l_old_coordinates,l_old_max_diff_square);
                double l_new_max_diff_square ;
                double l_new_result = moindre_carres(l_new_coordinates,l_new_max_diff_square);
                std::cout << "Old = (" << l_old_result << "," << l_old_max_diff_square << ") New = (" << l_new_result << "," << l_new_max_diff_square << ")" << std::endl ;
                double l_alignment_modification_rate = ( l_new_result ? l_old_result / l_new_result : std::numeric_limits<double>::max());
                std::cout << "Alignment modification rate = " << l_alignment_modification_rate << std::endl ;
                double l_min_square_modification_rate = ( l_new_max_diff_square ? l_old_max_diff_square / l_new_max_diff_square : std::numeric_limits<double>::max());
		std::cout << "Min square modification rate = " << l_min_square_modification_rate << std::endl ;
                // Way has been aligned, remove node form analyzis queue to reduce API requests
                if(l_alignment_modification_rate > m_min_alignment_modification_rate && l_min_square_modification_rate  > m_min_alignment_modification_rate)
                  {
		    create_svg(p_id,l_old_coordinates,l_new_coordinates);
                    l_result = true;
                    std::stringstream l_id_stream;
                    l_id_stream << m_id;
                    std::stringstream l_way_id_stream;
                    l_way_id_stream << p_id;

                    std::string l_object_url;
                    m_api->get_object_browse_url(l_object_url,"way",p_id); 
                    std::string l_changeset_url;
                    m_api->get_object_browse_url(l_changeset_url,"changeset",m_id);
                    std::string l_user_url;
                    m_api->get_user_browse_url(l_user_url,m_user_id,m_user_name);
                    m_report << "<A HREF=\"" << l_object_url << "\">Way " << l_way_id_stream.str() << "</A> has been aligned by <A HREF=\"" << l_user_url << "\">" << m_user_name << "</A> in <A HREF=\"" << l_changeset_url << "\">Changeset " << l_id_stream.str() << "</A><BR>" << std::endl ;
                    for(std::vector<node*>::iterator l_iter = l_modified_nodes.begin();
                        l_iter != l_modified_nodes.end();
                        ++l_iter)
                      {
                    
                        m_nodes.erase((*l_iter)->get_id());
                        delete *l_iter;
                      }
                  }
              }
          }
      }
    return l_result;
  }

  void changeset::create_svg(const osm_api_data_types::osm_object::t_osm_id & p_id,
                             const std::vector<std::pair<double,double> > & p_old_list,
                             const std::vector<std::pair<double,double> > & p_new_list)
  {
    std::stringstream l_id_stream;
    l_id_stream << p_id;
    std::string l_file_name = "way_"+l_id_stream.str()+".svg";
    uint32_t l_width = 600;
    uint32_t l_height = 600;
    uint32_t l_circle_size = 5;
    double l_min_lat = std::numeric_limits<double>::max();
    double l_max_lat = -(std::numeric_limits<double>::max()- 1 );
    double l_min_lon = std::numeric_limits<double>::max();
    double l_max_lon = -(std::numeric_limits<double>::max()- 1 );
    for(std::vector<std::pair<double,double> >::const_iterator l_iter = p_old_list.begin() ;
        l_iter != p_old_list.end();
        ++l_iter)
      {
        if(l_iter->first > l_max_lat)
          {
            l_max_lat = l_iter->first;
          }
        if(l_iter->first < l_min_lat)
          {
            l_min_lat = l_iter->first;
          }
        if(l_iter->second > l_max_lon)
          {
            l_max_lon = l_iter->second;
          }
        if(l_iter->second < l_min_lon)
          {
            l_min_lon = l_iter->second;
          }
      }
    for(std::vector<std::pair<double,double> >::const_iterator l_iter = p_new_list.begin() ;
        l_iter != p_new_list.end();
        ++l_iter)
      {
        if(l_iter->first > l_max_lat)
          {
            l_max_lat = l_iter->first;
          }
        if(l_iter->first < l_min_lat)
          {
            l_min_lat = l_iter->first;
          }
        if(l_iter->second > l_max_lon)
          {
            l_max_lon = l_iter->second;
          }
        if(l_iter->second < l_min_lon)
          {
            l_min_lon = l_iter->second;
          }
      }
    double l_diff_lat = l_max_lat - l_min_lat ;
    double l_diff_lon = l_max_lon - l_min_lon;
    std::cout << "l_min_lat " << l_min_lat << std::endl ;
    l_min_lat = l_min_lat - l_diff_lat * 0.1;
    std::cout << "l_min_lat " << l_min_lat << std::endl ;
    std::cout << "l_max_lat " << l_max_lat << std::endl ;
    l_max_lat = l_max_lat + l_diff_lat * 0.1;
    std::cout << "l_max_lat " << l_max_lat << std::endl ;
    std::cout << "l_min_lon " << l_min_lon << std::endl ;
    l_min_lon = l_min_lon - l_diff_lon * 0.1;
    std::cout << "l_min_lon " << l_min_lon << std::endl ;
    std::cout << "l_max_lon " << l_max_lon << std::endl ;
    l_max_lon = l_max_lon + l_diff_lon * 0.1;
    std::cout << "l_max_lon " << l_max_lon << std::endl ;
  
    std::ofstream l_svg_file(l_file_name.c_str());
    l_svg_file << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << std::endl ;
    l_svg_file << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"" << l_width << "\" height=\"" << l_height << "\">" << std::endl ;
    for(std::vector<std::pair<double,double> >::const_iterator l_iter = p_old_list.begin() ;
        l_iter != p_old_list.end();
        ++l_iter)
      {
        double l_x = (( l_width - 1.0) * (l_iter->second -l_min_lon ))/ (l_max_lon - l_min_lon);
        double l_y = (( l_height - 1.0) * (l_max_lat - l_iter->first ))/ (l_max_lat - l_min_lat);
        l_svg_file << "<circle cx=\""<< l_x << "\" cy=\"" << l_y << "\" r=\""<< (l_circle_size +1 ) <<"\" fill=\"red\" />" << std::endl ;
      }
    for(std::vector<std::pair<double,double> >::const_iterator l_iter = p_new_list.begin() ;
        l_iter != p_new_list.end();
        ++l_iter)
      {
        double l_x = (( l_width - 1.0) * (l_iter->second -l_min_lon ))/ (l_max_lon - l_min_lon);
        double l_y = (( l_height - 1.0) * (l_max_lat - l_iter->first ))/ (l_max_lat - l_min_lat);
        l_svg_file << "<circle cx=\""<< l_x << "\" cy=\"" << l_y << "\" r=\""<< l_circle_size <<"\" fill=\"blue\" />" << std::endl ;
      }
    l_svg_file << "</svg>" << std::endl ;
    l_svg_file.close();

  }

  double changeset::moindre_carres(const std::vector<std::pair<double,double> > & p_list, double & p_max_alignment_square)
  {
    p_max_alignment_square = 0;
    double l_sum = 0;
    double l_average_x = 0;
    double l_average_y = 0;
    for(std::vector<std::pair<double,double> >::const_iterator l_iter = p_list.begin();
        l_iter != p_list.end();
        ++l_iter)
      {
        l_average_x += l_iter->first;
        l_average_y += l_iter->second;
      }
    l_average_x = l_average_x / p_list.size(); 
    l_average_y = l_average_y / p_list.size(); 
    std::cout << "Average x = " << l_average_x << std::endl ;
    std::cout << "Average y = " << l_average_y << std::endl ;
  
    // a compuation
    double l_a = 0;
    double l_num = 0;
    double l_den = 0;
    for(std::vector<std::pair<double,double> >::const_iterator l_iter = p_list.begin();
        l_iter != p_list.end();
        ++l_iter)
      {
        l_num += (l_iter->first - l_average_x) * (l_iter->second - l_average_x);
        l_den += (l_iter->first - l_average_x) * (l_iter->first - l_average_x);
      }  
    if(l_den)
      {
        l_a = l_num / l_den;
        double l_b = l_average_y - l_a * l_average_x;
        for(std::vector<std::pair<double,double> >::const_iterator l_iter = p_list.begin();
            l_iter != p_list.end();
            ++l_iter)
          {
            double l_diff = (l_iter->second - l_a * l_iter->first - l_b);
            double l_diff_square = l_diff * l_diff;
            std::cout << "Max = " << p_max_alignment_square << " " << l_diff_square << std::endl ;
            if(p_max_alignment_square < l_diff_square)
              {
                p_max_alignment_square = l_diff_square;
              }
            l_sum += l_diff_square; 
          }      
      }
    else
      {
        std::cout << "Droite verticale" << std::endl ;
      }
    return l_sum;
  }


  float changeset::m_modif_rate_min_level = 0.9;
  float changeset::m_min_alignment_modification_rate = 100;
  node_alignment_analyzer_common_api * changeset::m_api = NULL;
  uint32_t changeset::m_min_way_node_nb = 2;
}
//EOF
